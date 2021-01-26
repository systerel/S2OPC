/*
 * Licensed to Systerel under one or more contributor license
 * agreements. See the NOTICE file distributed with this work
 * for additional information regarding copyright ownership.
 * Systerel licenses this file to you under the Apache
 * License, Version 2.0 (the "License"); you may not use this
 * file except in compliance with the License. You may obtain
 * a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

/// @file sopc_interrupttimer.c

#include <assert.h>

#include "sopc_interrupttimer.h"

/// @brief Sync status of an interrupt timer workspace
/// @brief ORDER of those status is important. INITIALIZED SHALL BE THE LAST STATUS !!!
/// @brief This status is used to verify initialization and the using of the workspace without blocking algorithm usage.
typedef enum E_INTERRUPT_TIMER_SYNC_STATUS
{
    E_INTERRUPT_TIMER_SYNC_STATUS_NOT_INITIALIZED, ///< Workspace is created and not initialized
    E_INTERRUPT_TIMER_SYNC_STATUS_INITIALIZING,    ///< Initializing on going...
    E_INTERRUPT_TIMER_SYNC_STATUS_DEINITIALIZING,  ///< Resetting on going...
    E_INTERRUPT_TIMER_SYNC_STATUS_INITIALIZED,     ///< Workspace is initialized, timer instance can be used
    E_INTERRUPT_TIMER_SYNC_STATUS_LOCKED, ///< Workspace is in use, some timer instances are called by their API
    E_INTERRUPT_TIMER_SYNC_STATUS_SIZE = INT32_MAX
} eInterruptTimerSyncStatus;

/// @brief Sync status of an interrupt timer instance
typedef enum E_INTERRUPT_TIMER_INST_SYNC_STATUS
{
    E_INTERRUPT_TIMER_INSTANCE_SYNC_STATUS_NOT_USED,  ///< Timer instance is not in use
    E_INTERRUPT_TIMER_INSTANCE_SYNC_STATUS_RESERVING, ///< Timer instance is initializing / in use (Data handle
                                                      ///< initialize concurrent protection)
    E_INTERRUPT_TIMER_INSTANCE_SYNC_STATUS_RESERVED,  ///< Timer instance is reserved. (Data Handle Initialize and
                                                      ///< Finalize concurrent protection)
    E_INTERRUPT_TIMER_INSTANCE_SYNC_STATUS_RELEASING, ///< Timer instance is releasing / in use (Data handle finalize
                                                      ///< concurrent protection)
    E_INTERRUPT_TIMER_INSTANCE_SYNC_STATUS_SIZE = INT32_MAX
} eInterruptTimerInstanceSyncStatus;

/// @brief Get member size of a structure
#define member_size(type, member) sizeof(((type*) 0)->member)

/// @brief Header data of a timer instance information.
typedef struct T_TIMER_INSTANCE_INFO
{
    SOPC_IrqTimer_InstanceStatus wStatus;       ///< Timer status, 0 or 1
    uint32_t wPeriod;                           ///< Period in ticks (1 tick = 100µs)
    uint32_t wOffset;                           ///< Absolute offset in ticks
    void* pUserContext;                         ///< User context
    sopc_irq_timer_cb_start cbStart;            ///< User start callback
    sopc_irq_timer_cb_period_elapsed cbElapsed; ///< User period elapsed callback
    sopc_irq_timer_cb_stop cbStop;              ///< User stop callback. SHALL BE LAST FIELD
    size_t dataSize;                            //< Size of the residual data (TODO: remove me)
} __attribute__((packed)) tTimerInstanceInfo;

/// @brief Type which indicates if the API is in use.
typedef struct T_INTERRUPT_TIMER_API_STATUS
{
    eInterruptTimerInstanceSyncStatus instanceStatus; ///< Timer instance sync status
} tInterruptTimerAPIStatus;

/// @brief Data of an interrupt timer workspace. It can contain several timer instances.
typedef struct T_INTERRUPT_TIMER_DATA
{
    uint32_t nbInstances;      ///< Maximum of timer instances
    uint32_t maxTimerDataSize; ///< Max of data hold by a timer instance
    uint64_t irqTicks;         ///< Current tick value.

    tInterruptTimerAPIStatus bTickIsUpdating; ///< Indicates if tick and timer evaluation function (update) is in use

    tInterruptTimerAPIStatus*
        pTimerInstanceInUse; ///< Indicates if set data, stop, start... function for a instance is in use
    SOPC_IrqTimer_InstanceStatus*
        pTimerInstancePreviousStatus;               ///< Table of previous status, used to generate start or stop event
    SOPC_DoubleBuffer** pTimerInstanceDoubleBuffer; ///< Table of DBO, one by timer instance.
} tInterruptTimerData;

/// @brief Interrupt timer workspace definition
struct SOPC_InterruptTimer
{
    eInterruptTimerSyncStatus status; ///< Status of the workspace
    tInterruptTimerData* pData;       ///< Timer interrupt workspace
};

/// @brief Interrupt timer instance data handle
/// @brief Used to expose interrupt timer instance data handle
struct SOPC_InterruptTimer_DataHandle
{
    struct SOPC_InterruptTimer* pTimer; ///< Linked workspace, set by SOPC_InterruptTimer_DataHandle_Create
    uint32_t maxAllowedSize;            ///< Max allowed size retrieved by SOPC_InterruptTimer_DataHandle_Initialize
    uint8_t* pPublishedData;            ///< Pointer on timer data buffer

    uint32_t idInstanceTimer; ///< Timer instance identifier, set by SOPC_InterruptTimer_DataHandle_Create
    size_t idContainer;       ///< Double buffer reserved buffer used by timer instance identifier, returned by
                              ///< SOPC_DoubleBuffer_GetWriteBuffer
    uint8_t* pDboData;        //< Double buffer data buffer field: timer info followed by timer data
};

// Declaration of private function

/// @brief Internal interrupt timer workspace creation
/// @warning Not thread safe!
/// @param [in] nbInstances Maximum of timer instances (timer identifiers between 0 and nbInstances - 1)
/// @param [in] maxDataSize Maximum of data in bytes hold by each timer
/// @return Internal timer workspace
static inline tInterruptTimerData* SOPC_InterruptTimer_Workspace_Create(
    uint32_t nbInstances,  // Maximum of timer instances (timer identifiers between 0 and nbInstances - 1)
    uint32_t maxDataSize); // Maximum of data in bytes hold by each timer

/// @brief Internal interrupt timer workspace destruction
/// @param [inout] ppWks Interrupt timer private workspace
static inline void SOPC_InterruptTimer_Workspace_Destroy(tInterruptTimerData** ppWks);

/// @brief Increment number of API clients
/// @param [in] pTimer Interrupt timer workspace
/// @return Current status.
static inline eInterruptTimerSyncStatus SOPC_InterruptTimer_IncrementInUseStatus(SOPC_InterruptTimer* pTimer);

/// @brief Decrement number of API clients
/// @param [in] pTimer Interrupt timer workspace
/// @return Current status.
static inline eInterruptTimerSyncStatus SOPC_InterruptTimer_DecrementInUseStatus(SOPC_InterruptTimer* pTimer);

/// @brief Enable or disable timer instance
/// @param [in] pTimer Interrupt timer object
/// @param [in] idInstanceTimer Timer instance identifier
/// @param [in] status Status to set
/// @return SOPC_STATUS_INVALID_STATE if workspace is not initialized or API for this instance is currently in use.
/// @return SOPC_STATUS_NOK for others error. Else SOPC_STATUS_OK.
static inline SOPC_ReturnStatus SOPC_InterruptTimer_SetStatus(SOPC_InterruptTimer* pTimer, // Interrupt timer object
                                                              uint32_t idInstanceTimer,    // Id of timer
                                                              SOPC_IrqTimer_InstanceStatus status); // Status to set

SOPC_InterruptTimer* SOPC_InterruptTimer_Create(void)
{
    SOPC_InterruptTimer* pTimer = NULL;

    pTimer = SOPC_Calloc(1, sizeof(struct SOPC_InterruptTimer));

    if (NULL == pTimer)
    {
        return NULL;
    }

    return pTimer;
}

void SOPC_InterruptTimer_Destroy(SOPC_InterruptTimer** ppTimer)
{
    if (NULL != ppTimer)
    {
        if (NULL != (*ppTimer))
        {
            SOPC_Free(*ppTimer);
            *ppTimer = NULL;
        }
    }
}

SOPC_ReturnStatus SOPC_InterruptTimer_Initialize(SOPC_InterruptTimer* pTimer,
                                                 uint32_t nbInstances,
                                                 uint32_t maxInstanceDataSize)
{
    if (NULL == pTimer || nbInstances < 1)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    // Try to go to INITILIZING from NOT INITIALIZED state
    eInterruptTimerSyncStatus expectedStatus = E_INTERRUPT_TIMER_SYNC_STATUS_NOT_INITIALIZED;
    eInterruptTimerSyncStatus desiredStatus = E_INTERRUPT_TIMER_SYNC_STATUS_INITIALIZING;
    bool bTransition = __atomic_compare_exchange(&pTimer->status, &expectedStatus, &desiredStatus, false,
                                                 __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);

    // If status from NOT INITIALIZED, so create interrupt timer workspace.
    if (bTransition)
    {
        tInterruptTimerData* pWks = SOPC_InterruptTimer_Workspace_Create(nbInstances, maxInstanceDataSize);

        if (NULL == pWks)
        {
            result = SOPC_STATUS_NOK;
        }

        if (SOPC_STATUS_OK == result)
        {
            pTimer->pData = pWks;
            // Update status from INITIALIZING to INITIALIZED
            desiredStatus = E_INTERRUPT_TIMER_SYNC_STATUS_INITIALIZED;
            __atomic_store(&pTimer->status, &desiredStatus, __ATOMIC_SEQ_CST);
        }
        else
        {
            pTimer->pData = NULL;
            // Update status from INITIALIZING to NOT INTIALIZED
            desiredStatus = E_INTERRUPT_TIMER_SYNC_STATUS_NOT_INITIALIZED;
            __atomic_store(&pTimer->status, &desiredStatus, __ATOMIC_SEQ_CST);
        }
    }
    // Invalid state : initializing, resetting or in use status.
    else if (E_INTERRUPT_TIMER_SYNC_STATUS_LOCKED <= expectedStatus ||
             E_INTERRUPT_TIMER_SYNC_STATUS_DEINITIALIZING == expectedStatus ||
             E_INTERRUPT_TIMER_SYNC_STATUS_INITIALIZING == expectedStatus)
    {
        result = SOPC_STATUS_INVALID_STATE;
    }
    // Not successful for other error or already initialized.
    else
    {
        result = SOPC_STATUS_NOK;
    }

    return result;
}

SOPC_ReturnStatus SOPC_InterruptTimer_DeInitialize(SOPC_InterruptTimer* pTimer)
{
    if (NULL == pTimer)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    // Try to go to DEINITIALIZED from INITIALIZED state
    eInterruptTimerSyncStatus expectedStatus = E_INTERRUPT_TIMER_SYNC_STATUS_INITIALIZED;
    eInterruptTimerSyncStatus desiredStatus = E_INTERRUPT_TIMER_SYNC_STATUS_DEINITIALIZING;
    bool bTransition = __atomic_compare_exchange(&pTimer->status, &expectedStatus, &desiredStatus, false,
                                                 __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);

    // If status from INITIALIZED, so destroy interrupt timer workspace.
    if (bTransition)
    {
        SOPC_InterruptTimer_Workspace_Destroy(&pTimer->pData);

        // Update status from  DEINITIALIZING to NOT INITIALIZED
        desiredStatus = E_INTERRUPT_TIMER_SYNC_STATUS_NOT_INITIALIZED;
        __atomic_store(&pTimer->status, &desiredStatus, __ATOMIC_SEQ_CST);
    }
    else if (E_INTERRUPT_TIMER_SYNC_STATUS_LOCKED <= expectedStatus ||
             E_INTERRUPT_TIMER_SYNC_STATUS_DEINITIALIZING == expectedStatus ||
             E_INTERRUPT_TIMER_SYNC_STATUS_INITIALIZING == expectedStatus)
    {
        result = SOPC_STATUS_INVALID_STATE;
    }
    else
    {
        result = SOPC_STATUS_NOK;
    }

    return result;
}

SOPC_ReturnStatus SOPC_InterruptTimer_Instance_Init(SOPC_InterruptTimer* pTimer,
                                                    uint32_t idInstanceTimer,
                                                    uint32_t period,
                                                    uint32_t offset,
                                                    void* pUserContext,
                                                    sopc_irq_timer_cb_start cbStart,
                                                    sopc_irq_timer_cb_period_elapsed cbElapsed,
                                                    sopc_irq_timer_cb_stop cbStop,
                                                    SOPC_IrqTimer_InstanceStatus initStatus)
{
    // Check for incorrect parameters
    if ((NULL == pTimer) || (NULL == pTimer->pData) || (idInstanceTimer >= pTimer->pData->nbInstances) ||
        (initStatus > SOPC_INTERRUPT_TIMER_STATUS_ENABLED || initStatus < SOPC_INTERRUPT_TIMER_STATUS_DISABLED))
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    // Try to update status from a status (if this is the case) INITIALIZED or BEYOND this status to this status + 1
    eInterruptTimerSyncStatus currentStatus = SOPC_InterruptTimer_IncrementInUseStatus(pTimer);

    // If current status is a "in use" status, try to mark api as in use
    if (currentStatus > E_INTERRUPT_TIMER_SYNC_STATUS_INITIALIZED)
    {
        tInterruptTimerData* pWks = pTimer->pData;

        // Try to go from not in use to in use status for the specified instance
        eInterruptTimerInstanceSyncStatus expectedValue = E_INTERRUPT_TIMER_INSTANCE_SYNC_STATUS_NOT_USED;
        eInterruptTimerInstanceSyncStatus desiredValue = E_INTERRUPT_TIMER_INSTANCE_SYNC_STATUS_RESERVING;
        bool bTransition =
            __atomic_compare_exchange(&pWks->pTimerInstanceInUse[idInstanceTimer].instanceStatus, &expectedValue,
                                      &desiredValue, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);

        // If transition OK, get timer instance information in the associated double buffer
        // and set new configuration taken into account by next update call.
        if (bTransition)
        {
            // Try to get a not reading buffer
            size_t idBuffer = 0;

            result =
                SOPC_DoubleBuffer_GetWriteBuffer(pWks->pTimerInstanceDoubleBuffer[idInstanceTimer], &idBuffer, NULL);
            assert(SOPC_STATUS_OK == result);

            // Erase buffer
            result = SOPC_DoubleBuffer_WriteBufferErase(pWks->pTimerInstanceDoubleBuffer[idInstanceTimer], idBuffer);
            assert(SOPC_STATUS_OK == result);

            // Set timer instance configuration
            tTimerInstanceInfo timerInfo;
            timerInfo.wStatus = initStatus;
            timerInfo.wPeriod = period;
            timerInfo.wOffset = offset;
            timerInfo.pUserContext = pUserContext;
            timerInfo.cbElapsed = cbElapsed;
            timerInfo.cbStart = cbStart;
            timerInfo.cbStop = cbStop;
            timerInfo.dataSize = 0;

            // Try to write timer configuration
            result = SOPC_DoubleBuffer_WriteBuffer(pWks->pTimerInstanceDoubleBuffer[idInstanceTimer], idBuffer, 0,
                                                   (uint8_t*) &timerInfo, sizeof(tTimerInstanceInfo), NULL, true, true);
            assert(SOPC_STATUS_OK == result);

            // Release buffer
            SOPC_DoubleBuffer_ReleaseWriteBuffer(pWks->pTimerInstanceDoubleBuffer[idInstanceTimer], idBuffer);

            // Go from in use to not in use status for this timer instance
            desiredValue = E_INTERRUPT_TIMER_INSTANCE_SYNC_STATUS_NOT_USED;
            __atomic_store(&pWks->pTimerInstanceInUse[idInstanceTimer].instanceStatus, &desiredValue, __ATOMIC_SEQ_CST);
        }
        else
        {
            // API is in use...
            result = SOPC_STATUS_INVALID_STATE;
        }

        // Decrement workspace status, from a status beyond initialized to this status - 1
        SOPC_InterruptTimer_DecrementInUseStatus(pTimer);
    }
    else
    {
        result = SOPC_STATUS_NOK;
    }

    return result;
}

SOPC_ReturnStatus SOPC_InterruptTimer_Instance_DeInit(SOPC_InterruptTimer* pTimer, uint32_t idInstanceTimer)
{
    // Check for incorrect parameters
    if ((NULL == pTimer) || (NULL == pTimer->pData) || (idInstanceTimer >= pTimer->pData->nbInstances))
    {
        return SOPC_STATUS_NOK;
    }

    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    // Try to go current initialized status to this + 1
    eInterruptTimerSyncStatus currentStatus = SOPC_InterruptTimer_IncrementInUseStatus(pTimer);

    // If valid status, try to erase timer configuration for specified instance
    if (currentStatus > E_INTERRUPT_TIMER_SYNC_STATUS_INITIALIZED)
    {
        tInterruptTimerData* pWks = pTimer->pData;

        eInterruptTimerInstanceSyncStatus expectedValue = E_INTERRUPT_TIMER_INSTANCE_SYNC_STATUS_NOT_USED;
        eInterruptTimerInstanceSyncStatus desiredValue = E_INTERRUPT_TIMER_INSTANCE_SYNC_STATUS_RESERVING;
        bool bTransition =
            __atomic_compare_exchange(&pWks->pTimerInstanceInUse[idInstanceTimer].instanceStatus, &expectedValue,
                                      &desiredValue, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);

        // If API is not in use for this instance, erase timer instance configuration to take into account by next
        // update call.
        if (bTransition)
        {
            // Reserve DBO for write
            size_t idBuffer = 0;
            result =
                SOPC_DoubleBuffer_GetWriteBuffer(pWks->pTimerInstanceDoubleBuffer[idInstanceTimer], &idBuffer, NULL);
            assert(SOPC_STATUS_OK == result);

            // Write DBO
            result = SOPC_DoubleBuffer_WriteBufferErase(pWks->pTimerInstanceDoubleBuffer[idInstanceTimer], idBuffer);
            assert(SOPC_STATUS_OK == result);

            // Release DBO
            result = SOPC_DoubleBuffer_ReleaseWriteBuffer(pWks->pTimerInstanceDoubleBuffer[idInstanceTimer], idBuffer);
            assert(SOPC_STATUS_OK == result);

            // Mark API as not in use.
            desiredValue = E_INTERRUPT_TIMER_INSTANCE_SYNC_STATUS_NOT_USED;
            __atomic_store(&pWks->pTimerInstanceInUse[idInstanceTimer].instanceStatus, &desiredValue, __ATOMIC_SEQ_CST);
        }
        else
        {
            // Error, API in use for this instance
            result = SOPC_STATUS_INVALID_STATE;
        }

        // Decrement workspace status, from a status beyond initialized to this status - 1
        SOPC_InterruptTimer_DecrementInUseStatus(pTimer);
    }
    else
    {
        result = SOPC_STATUS_NOK;
    }

    return result;
}

static inline SOPC_ReturnStatus SOPC_InterruptTimer_SetStatus(SOPC_InterruptTimer* pTimer,
                                                              uint32_t idInstanceTimer,
                                                              SOPC_IrqTimer_InstanceStatus status)
{
    if ((NULL == pTimer) || (NULL == pTimer->pData) || (idInstanceTimer >= pTimer->pData->nbInstances))
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    // Try to go current initialized status to this + 1
    eInterruptTimerSyncStatus currentStatus = SOPC_InterruptTimer_IncrementInUseStatus(pTimer);

    // If valid status, try to set timer status for specified instance
    if (currentStatus > E_INTERRUPT_TIMER_SYNC_STATUS_INITIALIZED)
    {
        tInterruptTimerData* pWks = pTimer->pData;

        eInterruptTimerInstanceSyncStatus expectedValue = E_INTERRUPT_TIMER_INSTANCE_SYNC_STATUS_NOT_USED;
        eInterruptTimerInstanceSyncStatus desiredValue = E_INTERRUPT_TIMER_INSTANCE_SYNC_STATUS_RESERVED;

        bool bTransition =
            __atomic_compare_exchange(&pWks->pTimerInstanceInUse[idInstanceTimer].instanceStatus, &expectedValue,
                                      &desiredValue, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);

        // If API is not in use for this instance, set timer status
        if (bTransition)
        {
            // New status to take into account by next update call
            tTimerInstanceInfo timerInfo;
            timerInfo.wStatus = status;

            // Reserve DBO for write
            size_t idBuffer = 0;
            result =
                SOPC_DoubleBuffer_GetWriteBuffer(pWks->pTimerInstanceDoubleBuffer[idInstanceTimer], &idBuffer, NULL);
            assert(SOPC_STATUS_OK == result);

            // Try to write DBO
            size_t size = 0;
            result = SOPC_DoubleBuffer_WriteBuffer(pWks->pTimerInstanceDoubleBuffer[idInstanceTimer], idBuffer,
                                                   offsetof(tTimerInstanceInfo, wStatus), (uint8_t*) &timerInfo.wStatus,
                                                   member_size(tTimerInstanceInfo, wStatus), &size, false, false);
            assert(SOPC_STATUS_OK == result);

            // Check written size. Until wStatus field included minimum.
            if (size < offsetof(tTimerInstanceInfo, wStatus) + member_size(tTimerInstanceInfo, wStatus))
            {
                /* TODO: Call ReleaseWriteBuffer anyway and cancel the write */
                result = SOPC_STATUS_NOK;
            }
            else
            {
                // Release DBO
                result =
                    SOPC_DoubleBuffer_ReleaseWriteBuffer(pWks->pTimerInstanceDoubleBuffer[idInstanceTimer], idBuffer);
                assert(SOPC_STATUS_OK == result);
            }

            // Mark API as not in use.
            desiredValue = E_INTERRUPT_TIMER_INSTANCE_SYNC_STATUS_NOT_USED;
            __atomic_store(&pWks->pTimerInstanceInUse[idInstanceTimer].instanceStatus, &desiredValue, __ATOMIC_SEQ_CST);
        }
        else
        {
            result = SOPC_STATUS_INVALID_STATE;
        }

        // Decrement workspace status, from a status beyond initialized to this status - 1
        SOPC_InterruptTimer_DecrementInUseStatus(pTimer);
    }
    else
    {
        result = SOPC_STATUS_NOK;
    }

    return result;
}

SOPC_ReturnStatus SOPC_InterruptTimer_Instance_LastStatus(SOPC_InterruptTimer* pTimer,
                                                          uint32_t idInstanceTimer,
                                                          SOPC_IrqTimer_InstanceStatus* status)
{
    if ((NULL == pTimer) || (NULL == status) || (NULL == pTimer->pData) ||
        (idInstanceTimer >= pTimer->pData->nbInstances))
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    // Try to go current initialized status to this + 1
    eInterruptTimerSyncStatus currentStatus = SOPC_InterruptTimer_IncrementInUseStatus(pTimer);

    // If valid status, try to set timer status for specified instance
    if (currentStatus > E_INTERRUPT_TIMER_SYNC_STATUS_INITIALIZED)
    {
        tInterruptTimerData* pWks = pTimer->pData;

        eInterruptTimerInstanceSyncStatus expectedStatus = E_INTERRUPT_TIMER_INSTANCE_SYNC_STATUS_NOT_USED;
        eInterruptTimerInstanceSyncStatus desiredStatus = E_INTERRUPT_TIMER_INSTANCE_SYNC_STATUS_RESERVED;

        bool bTransition =
            __atomic_compare_exchange(&pWks->pTimerInstanceInUse[idInstanceTimer].instanceStatus, &expectedStatus,
                                      &desiredStatus, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);

        // If API is not in use for this instance, set timer status
        if (bTransition)
        {
            // New status to take into account by next update call
            tTimerInstanceInfo* pTimerInfo;

            // Reserve DBO for write
            size_t idBuffer = 0;
            result =
                SOPC_DoubleBuffer_GetWriteBuffer(pWks->pTimerInstanceDoubleBuffer[idInstanceTimer], &idBuffer, NULL);
            assert(SOPC_STATUS_OK == result);

            // Try to write DBO
            /* TODO: Is that a read call disguised as a write ? */
            result = SOPC_DoubleBuffer_WriteBufferGetPtr(pWks->pTimerInstanceDoubleBuffer[idInstanceTimer], idBuffer,
                                                         (uint8_t**) &pTimerInfo, false);

            if (SOPC_STATUS_OK == result)
            {
                *status = pTimerInfo->wStatus;
            }

            // Mark API as not in use.
            desiredStatus = E_INTERRUPT_TIMER_INSTANCE_SYNC_STATUS_NOT_USED;
            __atomic_store(&pWks->pTimerInstanceInUse[idInstanceTimer].instanceStatus, &desiredStatus,
                           __ATOMIC_SEQ_CST);

            /* TODO: call SOPC_DoubleBuffer_ReleaseWriteBuffer */
        }
        else
        {
            result = SOPC_STATUS_INVALID_STATE;
        }

        // Decrement workspace status, from a status beyond initialized to this status - 1
        SOPC_InterruptTimer_DecrementInUseStatus(pTimer);
    }
    else
    {
        result = SOPC_STATUS_NOK;
    }

    return result;
}

SOPC_ReturnStatus SOPC_InterruptTimer_Instance_Start(SOPC_InterruptTimer* pTimer, uint32_t idInstanceTimer)
{
    return SOPC_InterruptTimer_SetStatus(pTimer, idInstanceTimer, SOPC_INTERRUPT_TIMER_STATUS_ENABLED);
}

SOPC_ReturnStatus SOPC_InterruptTimer_Instance_Stop(SOPC_InterruptTimer* pTimer, uint32_t idInstanceTimer)
{
    return SOPC_InterruptTimer_SetStatus(pTimer, idInstanceTimer, SOPC_INTERRUPT_TIMER_STATUS_DISABLED);
}

SOPC_ReturnStatus SOPC_InterruptTimer_Instance_SetPeriod(SOPC_InterruptTimer* pTimer,
                                                         uint32_t idInstanceTimer,
                                                         uint32_t period)
{
    if ((NULL == pTimer) || (NULL == pTimer->pData) || (idInstanceTimer >= pTimer->pData->nbInstances) || (period < 1))
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    // Try to go current initialized status to this + 1
    eInterruptTimerSyncStatus currentStatus = SOPC_InterruptTimer_IncrementInUseStatus(pTimer);

    // If valid status, try to set timer period for specified instance
    if (currentStatus > E_INTERRUPT_TIMER_SYNC_STATUS_INITIALIZED)
    {
        tInterruptTimerData* pWks = pTimer->pData;

        eInterruptTimerInstanceSyncStatus expectedStatus = E_INTERRUPT_TIMER_INSTANCE_SYNC_STATUS_NOT_USED;
        eInterruptTimerInstanceSyncStatus desiredStatus = E_INTERRUPT_TIMER_INSTANCE_SYNC_STATUS_RESERVED;
        bool bTransition =
            __atomic_compare_exchange(&pWks->pTimerInstanceInUse[idInstanceTimer].instanceStatus, &expectedStatus,
                                      &desiredStatus, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);

        // If API is not in use for this instance, set timer status
        if (bTransition)
        {
            // New period to take into account by next update call
            tTimerInstanceInfo timerInfo;
            timerInfo.wPeriod = period;

            // Reserve buffer
            size_t idBuffer = 0;
            result =
                SOPC_DoubleBuffer_GetWriteBuffer(pWks->pTimerInstanceDoubleBuffer[idInstanceTimer], &idBuffer, NULL);
            assert(SOPC_STATUS_OK == result);

            // Write buffer
            size_t size = 0;
            result = SOPC_DoubleBuffer_WriteBuffer(pWks->pTimerInstanceDoubleBuffer[idInstanceTimer], idBuffer,
                                                   offsetof(tTimerInstanceInfo, wPeriod), (uint8_t*) &timerInfo.wPeriod,
                                                   member_size(tTimerInstanceInfo, wPeriod), &size, false, false);
            assert(SOPC_STATUS_OK == result);

            // Check written size. Until wPeriod field included minimum.
            if (size < offsetof(tTimerInstanceInfo, wPeriod) + member_size(tTimerInstanceInfo, wPeriod))
            {
                /* TODO: Call ReleaseWriteBuffer anyway and cancel the write */
                result = SOPC_STATUS_NOK;
            }
            else
            {
                // Release buffer
                result =
                    SOPC_DoubleBuffer_ReleaseWriteBuffer(pWks->pTimerInstanceDoubleBuffer[idInstanceTimer], idBuffer);
            }

            // Mark API as not in use.
            desiredStatus = E_INTERRUPT_TIMER_INSTANCE_SYNC_STATUS_NOT_USED;
            __atomic_store(&pWks->pTimerInstanceInUse[idInstanceTimer].instanceStatus, &desiredStatus,
                           __ATOMIC_SEQ_CST);
        }
        else
        {
            result = SOPC_STATUS_INVALID_STATE;
        }

        // Decrement workspace status, from a status beyond initialized to this status - 1
        SOPC_InterruptTimer_DecrementInUseStatus(pTimer);
    }
    else
    {
        result = SOPC_STATUS_NOK;
    }

    return result;
}

SOPC_ReturnStatus SOPC_InterruptTimer_Instance_SetOffset(SOPC_InterruptTimer* pTimer,
                                                         uint32_t idInstanceTimer,
                                                         uint32_t offset)
{
    if ((NULL == pTimer) || (NULL == pTimer->pData) || (idInstanceTimer >= pTimer->pData->nbInstances))
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    // Try to go current initialized status to this + 1
    eInterruptTimerSyncStatus currentStatus = SOPC_InterruptTimer_IncrementInUseStatus(pTimer);

    // If valid status, try to set timer period for specified instance
    if (currentStatus > E_INTERRUPT_TIMER_SYNC_STATUS_INITIALIZED)
    {
        tInterruptTimerData* pWks = pTimer->pData;

        eInterruptTimerInstanceSyncStatus expectedStatus = E_INTERRUPT_TIMER_INSTANCE_SYNC_STATUS_NOT_USED;
        eInterruptTimerInstanceSyncStatus desiredStatus = E_INTERRUPT_TIMER_INSTANCE_SYNC_STATUS_RESERVED;
        bool bTransition =
            __atomic_compare_exchange(&pWks->pTimerInstanceInUse[idInstanceTimer].instanceStatus, &expectedStatus,
                                      &desiredStatus, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);

        // If API is not in use for this instance, set timer status
        if (bTransition)
        {
            // New period to take into account by next update call
            tTimerInstanceInfo timerInfo;
            timerInfo.wOffset = offset;

            // Reserve buffer
            size_t idBuffer = 0;
            result =
                SOPC_DoubleBuffer_GetWriteBuffer(pWks->pTimerInstanceDoubleBuffer[idInstanceTimer], &idBuffer, NULL);
            assert(SOPC_STATUS_OK == result);

            // Write buffer
            size_t size = 0;
            result = SOPC_DoubleBuffer_WriteBuffer(pWks->pTimerInstanceDoubleBuffer[idInstanceTimer], idBuffer,
                                                   offsetof(tTimerInstanceInfo, wOffset), (uint8_t*) &timerInfo.wOffset,
                                                   member_size(tTimerInstanceInfo, wOffset), &size, false, false);
            assert(SOPC_STATUS_OK == result);

            // Check written size. Until wPeriod field included minimum.
            if (size < offsetof(tTimerInstanceInfo, wOffset) + member_size(tTimerInstanceInfo, wOffset))
            {
                /* TODO: Call ReleaseWriteBuffer anyway and cancel the write */
                result = SOPC_STATUS_NOK;
            }
            else
            {
                // Release buffer
                result =
                    SOPC_DoubleBuffer_ReleaseWriteBuffer(pWks->pTimerInstanceDoubleBuffer[idInstanceTimer], idBuffer);
            }

            // Mark API as not in use.
            desiredStatus = E_INTERRUPT_TIMER_INSTANCE_SYNC_STATUS_NOT_USED;
            __atomic_store(&pWks->pTimerInstanceInUse[idInstanceTimer].instanceStatus, &desiredStatus,
                           __ATOMIC_SEQ_CST);
        }
        else
        {
            result = SOPC_STATUS_INVALID_STATE;
        }

        // Decrement workspace status, from a status beyond initialized to this status - 1
        SOPC_InterruptTimer_DecrementInUseStatus(pTimer);
    }
    else
    {
        result = SOPC_STATUS_NOK;
    }

    return result;
}

SOPC_ReturnStatus SOPC_InterruptTimer_Instance_SetCallback(SOPC_InterruptTimer* pTimer,
                                                           uint32_t idInstanceTimer,
                                                           void* pUserContext,
                                                           sopc_irq_timer_cb_start cbStart,
                                                           sopc_irq_timer_cb_period_elapsed cbElapsed,
                                                           sopc_irq_timer_cb_stop cbStop)
{
    if ((NULL == pTimer) || (NULL == pTimer->pData) || (idInstanceTimer >= pTimer->pData->nbInstances))
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    // Try to go current initialized status to this + 1
    eInterruptTimerSyncStatus currentStatus = SOPC_InterruptTimer_IncrementInUseStatus(pTimer);

    // If valid status, try to set timer period for specified instance
    if (currentStatus > E_INTERRUPT_TIMER_SYNC_STATUS_INITIALIZED)
    {
        tInterruptTimerData* pWks = pTimer->pData;

        eInterruptTimerInstanceSyncStatus expectedStatus = E_INTERRUPT_TIMER_INSTANCE_SYNC_STATUS_NOT_USED;
        eInterruptTimerInstanceSyncStatus desiredStatus = E_INTERRUPT_TIMER_INSTANCE_SYNC_STATUS_RESERVED;
        bool bTransition =
            __atomic_compare_exchange(&pWks->pTimerInstanceInUse[idInstanceTimer].instanceStatus, &expectedStatus,
                                      &desiredStatus, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);

        // If API is not in use for this instance, set timer status
        if (bTransition)
        {
            // Callback functions to take into account by next update call.
            tTimerInstanceInfo timerInfo;
            timerInfo.pUserContext = pUserContext;
            timerInfo.cbStart = cbStart;
            timerInfo.cbElapsed = cbElapsed;
            timerInfo.cbStop = cbStop;

            // Reserve DBO to write
            size_t idBuffer = 0;
            result =
                SOPC_DoubleBuffer_GetWriteBuffer(pWks->pTimerInstanceDoubleBuffer[idInstanceTimer], &idBuffer, NULL);
            assert(SOPC_STATUS_OK == result);

            // Write DBO
            size_t size = 0;
            result = SOPC_DoubleBuffer_WriteBuffer(
                pWks->pTimerInstanceDoubleBuffer[idInstanceTimer], idBuffer, offsetof(tTimerInstanceInfo, pUserContext),
                (uint8_t*) &timerInfo.pUserContext,
                member_size(tTimerInstanceInfo, pUserContext) + member_size(tTimerInstanceInfo, cbElapsed) +
                    member_size(tTimerInstanceInfo, cbStart) + member_size(tTimerInstanceInfo, cbStop),
                &size, false, false);
            assert(SOPC_STATUS_OK == result);

            // Check written size. Until cbStop field included minimum.
            if (size < offsetof(tTimerInstanceInfo, cbStop) + member_size(tTimerInstanceInfo, cbStop))
            {
                /* TODO: Call ReleaseWriteBuffer anyway and cancel the write */
                result = SOPC_STATUS_NOK;
            }
            else
            {
                // Release DBO
                result =
                    SOPC_DoubleBuffer_ReleaseWriteBuffer(pWks->pTimerInstanceDoubleBuffer[idInstanceTimer], idBuffer);
            }

            // Mark API as not in use.
            desiredStatus = E_INTERRUPT_TIMER_INSTANCE_SYNC_STATUS_NOT_USED;
            __atomic_store(&pWks->pTimerInstanceInUse[idInstanceTimer].instanceStatus, &desiredStatus,
                           __ATOMIC_SEQ_CST);
        }
        else
        {
            result = SOPC_STATUS_INVALID_STATE;
        }

        // Decrement workspace status, from a status beyond initialized to this status - 1
        SOPC_InterruptTimer_DecrementInUseStatus(pTimer);
    }
    else
    {
        result = SOPC_STATUS_NOK;
    }

    return result;
}

SOPC_ReturnStatus SOPC_InterruptTimer_Instance_SetData(SOPC_InterruptTimer* pTimer,
                                                       uint32_t idInstanceTimer,
                                                       uint8_t* pData,
                                                       uint32_t sizeToWrite)
{
    if ((NULL == pTimer) || (NULL == pTimer->pData) || (idInstanceTimer >= pTimer->pData->nbInstances))
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    // Try to go current initialized status to this + 1
    eInterruptTimerSyncStatus currentStatus = SOPC_InterruptTimer_IncrementInUseStatus(pTimer);

    // If valid status, try to set timer period for specified instance
    if (currentStatus > E_INTERRUPT_TIMER_SYNC_STATUS_INITIALIZED)
    {
        tInterruptTimerData* pWks = pTimer->pData;

        eInterruptTimerInstanceSyncStatus expectedStatus = E_INTERRUPT_TIMER_INSTANCE_SYNC_STATUS_NOT_USED;
        eInterruptTimerInstanceSyncStatus desiredStatus = E_INTERRUPT_TIMER_INSTANCE_SYNC_STATUS_RESERVED;
        bool bTransition =
            __atomic_compare_exchange(&pWks->pTimerInstanceInUse[idInstanceTimer].instanceStatus, &expectedStatus,
                                      &desiredStatus, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);

        // If API is not in use for this instance, set timer status
        if (bTransition)
        {
            // Reserve DBO to write
            size_t idBuffer = 0;
            result =
                SOPC_DoubleBuffer_GetWriteBuffer(pWks->pTimerInstanceDoubleBuffer[idInstanceTimer], &idBuffer, NULL);
            assert(SOPC_STATUS_OK == result);

            /* Update the timerInfo structure for the size of the payload and keep the current value for the rest */
            tTimerInstanceInfo timerInfo;
            timerInfo.dataSize = sizeToWrite;
            result = SOPC_DoubleBuffer_WriteBuffer(
                pWks->pTimerInstanceDoubleBuffer[idInstanceTimer], idBuffer, offsetof(tTimerInstanceInfo, dataSize),
                (uint8_t*) &timerInfo.dataSize, sizeof(timerInfo.dataSize), NULL, false, true);
            assert(SOPC_STATUS_OK == result);
            result = SOPC_DoubleBuffer_WriteBuffer(pWks->pTimerInstanceDoubleBuffer[idInstanceTimer], idBuffer,
                                                   sizeof(tTimerInstanceInfo), pData, sizeToWrite, NULL, true, true);

            // Data shall not be less than header + size published, duplicated from last update.
            /* TODO: verify the size before attempting to write */
            if (SOPC_STATUS_OK == result)
            {
                /* TODO: Call ReleaseWriteBuffer anyway and cancel the write */
                // Release DBO
                result =
                    SOPC_DoubleBuffer_ReleaseWriteBuffer(pWks->pTimerInstanceDoubleBuffer[idInstanceTimer], idBuffer);
            }
            // Mark API as not in use.
            desiredStatus = E_INTERRUPT_TIMER_INSTANCE_SYNC_STATUS_NOT_USED;
            __atomic_store(&pWks->pTimerInstanceInUse[idInstanceTimer].instanceStatus, &desiredStatus,
                           __ATOMIC_SEQ_CST);
        }
        else
        {
            result = SOPC_STATUS_INVALID_STATE;
        }

        // Decrement workspace status, from a status beyond initialized to this status - 1
        SOPC_InterruptTimer_DecrementInUseStatus(pTimer);
    }
    else
    {
        result = SOPC_STATUS_NOK;
    }

    return result;
}

SOPC_InterruptTimer_DataHandle* SOPC_InterruptTimer_Instance_DataHandle_Create(SOPC_InterruptTimer* pTimer,
                                                                               uint32_t idInstanceTimer)
{
    SOPC_InterruptTimer_DataHandle* pContainer = NULL;

    if ((NULL == (pTimer) || (NULL == pTimer->pData) || idInstanceTimer >= pTimer->pData->nbInstances))
    {
        return NULL;
    }

    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    // Try to go current initialized status to this + 1
    eInterruptTimerSyncStatus currentStatus = SOPC_InterruptTimer_IncrementInUseStatus(pTimer);

    // If valid status, try to set timer period for specified instance
    if (currentStatus > E_INTERRUPT_TIMER_SYNC_STATUS_INITIALIZED)
    {
        pContainer = SOPC_Calloc(1, sizeof(struct SOPC_InterruptTimer_DataHandle));
        if (NULL == pContainer)
        {
            result = SOPC_STATUS_OUT_OF_MEMORY;
        }

        if (SOPC_STATUS_OK == result)
        {
            pContainer->pTimer = pTimer;
            pContainer->idInstanceTimer = idInstanceTimer;
            pContainer->maxAllowedSize = pTimer->pData->maxTimerDataSize;
            pContainer->idContainer = 0;
        }

        SOPC_InterruptTimer_DecrementInUseStatus(pTimer);
    }

    return pContainer;
}

void SOPC_InterruptTimer_DestroyDataContainer(SOPC_InterruptTimer_DataHandle** ppDataContainer)
{
    if (NULL != ppDataContainer)
    {
        if ((*ppDataContainer) != NULL)
        {
            SOPC_Free((*ppDataContainer));
            *ppDataContainer = NULL;
        }
    }
}

SOPC_ReturnStatus SOPC_InterruptTimer_Instance_DataHandle_Initialize(SOPC_InterruptTimer_DataHandle* pDataContainer)
{
    if ((NULL == pDataContainer) || (NULL == (pDataContainer)->pTimer) || (NULL == (pDataContainer)->pTimer->pData) ||
        ((pDataContainer)->idInstanceTimer >= (pDataContainer)->pTimer->pData->nbInstances))
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    uint32_t idInstanceTimer = pDataContainer->idInstanceTimer;
    SOPC_InterruptTimer* pTimer = pDataContainer->pTimer;
    tInterruptTimerData* pTimerData = pDataContainer->pTimer->pData;
    SOPC_DoubleBuffer* pDbo = pTimer->pData->pTimerInstanceDoubleBuffer[idInstanceTimer];

    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    // Try to go current initialized status to this + 1
    eInterruptTimerSyncStatus currentStatus = SOPC_InterruptTimer_IncrementInUseStatus(pTimer);

    // If valid status, try to set timer period for specified instance
    if (currentStatus > E_INTERRUPT_TIMER_SYNC_STATUS_INITIALIZED)
    {
        eInterruptTimerInstanceSyncStatus expectedStatus = E_INTERRUPT_TIMER_INSTANCE_SYNC_STATUS_NOT_USED;
        eInterruptTimerInstanceSyncStatus desiredStatus = E_INTERRUPT_TIMER_INSTANCE_SYNC_STATUS_RESERVING;
        bool bTransition =
            __atomic_compare_exchange(&pTimerData->pTimerInstanceInUse[idInstanceTimer].instanceStatus, &expectedStatus,
                                      &desiredStatus, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);

        // If API is not in use for this instance, set timer status
        if (bTransition)
        {
            size_t size = 0;
            result = SOPC_DoubleBuffer_GetWriteBuffer(pDbo, &pDataContainer->idContainer, &size);

            if (SOPC_STATUS_OK == result)
            {
                result = SOPC_DoubleBuffer_WriteBufferGetPtr(pDbo, pDataContainer->idContainer,
                                                             &pDataContainer->pDboData, false);
            }

            if (SOPC_STATUS_OK == result)
            {
                pDataContainer->pPublishedData = (pDataContainer->pDboData + sizeof(tTimerInstanceInfo));
            }

            if (SOPC_STATUS_OK != result)
            {
                pDataContainer->idContainer = 0;
                pDataContainer->pDboData = NULL;
                pDataContainer->pPublishedData = NULL;

                result = SOPC_STATUS_NOK;

                desiredStatus = E_INTERRUPT_TIMER_INSTANCE_SYNC_STATUS_NOT_USED;
                __atomic_store(&pTimerData->pTimerInstanceInUse[idInstanceTimer].instanceStatus, &desiredStatus,
                               __ATOMIC_SEQ_CST);
            }
            else
            {
                SOPC_InterruptTimer_IncrementInUseStatus(pTimer);
                desiredStatus = E_INTERRUPT_TIMER_INSTANCE_SYNC_STATUS_RESERVED;
                __atomic_store(&pTimerData->pTimerInstanceInUse[idInstanceTimer].instanceStatus, &desiredStatus,
                               __ATOMIC_SEQ_CST);
            }
        }
        else
        {
            result = SOPC_STATUS_INVALID_STATE;
        }

        SOPC_InterruptTimer_DecrementInUseStatus(pDataContainer->pTimer);
    }
    else
    {
        result = SOPC_STATUS_NOK;
    }

    return result;
}

SOPC_ReturnStatus SOPC_InterruptTimer_Instance_DataHandle_Finalize(SOPC_InterruptTimer_DataHandle* pDataContainer,
                                                                   bool bCancel)
{
    if ((NULL == pDataContainer) || (NULL == pDataContainer->pTimer) || (NULL == pDataContainer->pTimer->pData) ||
        (pDataContainer->idInstanceTimer >= pDataContainer->pTimer->pData->nbInstances))
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    uint32_t idInstanceTimer = pDataContainer->idInstanceTimer;
    SOPC_InterruptTimer* pTimer = pDataContainer->pTimer;
    tInterruptTimerData* pTimerData = pDataContainer->pTimer->pData;
    SOPC_DoubleBuffer* pDbo = pTimerData->pTimerInstanceDoubleBuffer[idInstanceTimer];

    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    // Try to go current initialized status to this + 1
    eInterruptTimerSyncStatus currentStatus = SOPC_InterruptTimer_IncrementInUseStatus(pTimer);

    // If valid status, try to set timer period for specified instance
    if (currentStatus > E_INTERRUPT_TIMER_SYNC_STATUS_INITIALIZED)
    {
        eInterruptTimerInstanceSyncStatus expectedStatus = E_INTERRUPT_TIMER_INSTANCE_SYNC_STATUS_RESERVED;
        eInterruptTimerInstanceSyncStatus desiredStatus = E_INTERRUPT_TIMER_INSTANCE_SYNC_STATUS_RELEASING;
        bool bTransition =
            __atomic_compare_exchange(&pTimerData->pTimerInstanceInUse[idInstanceTimer].instanceStatus, &expectedStatus,
                                      &desiredStatus, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);

        // If API is not in use for this instance, set timer status
        if (bTransition)
        {
            tTimerInstanceInfo* ptrInfo = (void*) (pDataContainer->pDboData);
            if (ptrInfo->dataSize > pTimerData->maxTimerDataSize && !bCancel)
            {
                result = SOPC_STATUS_OUT_OF_MEMORY;
            }

            if (SOPC_STATUS_OK == result && !bCancel)
            {
                result = SOPC_DoubleBuffer_ReleaseWriteBuffer(pDbo, pDataContainer->idContainer);
            }

            pDataContainer->idContainer = 0;

            desiredStatus = E_INTERRUPT_TIMER_INSTANCE_SYNC_STATUS_NOT_USED;

            __atomic_store(&pTimerData->pTimerInstanceInUse[idInstanceTimer].instanceStatus, &desiredStatus,
                           __ATOMIC_SEQ_CST);

            SOPC_InterruptTimer_DecrementInUseStatus(pTimer);
        }
        else
        {
            result = SOPC_STATUS_INVALID_STATE;
        }

        SOPC_InterruptTimer_DecrementInUseStatus(pTimer);
    }
    else
    {
        result = SOPC_STATUS_NOK;
    }

    return result;
}

SOPC_ReturnStatus SOPC_InterruptTimer_Instance_DataHandle_GetBufferInfo(SOPC_InterruptTimer_DataHandle* pContainer,
                                                                        uint32_t* pMaxAllowedSize,
                                                                        size_t* pCurrentSize,
                                                                        uint8_t** ppData)
{
    if (NULL == pContainer || NULL == pMaxAllowedSize || NULL == pCurrentSize || NULL == ppData)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    *pMaxAllowedSize = pContainer->maxAllowedSize;
    tTimerInstanceInfo* ptrInfo = (void*) (pContainer->pDboData);
    *pCurrentSize = ptrInfo->dataSize;
    *ppData = pContainer->pPublishedData;

    return result;
}

SOPC_ReturnStatus SOPC_InterruptTimer_Instance_DataHandle_SetNewSize(SOPC_InterruptTimer_DataHandle* pContainer,
                                                                     size_t newSize)

{
    if (NULL == pContainer || newSize > pContainer->maxAllowedSize)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    tTimerInstanceInfo* ptrInfo = (void*) (pContainer->pDboData);
    ptrInfo->dataSize = newSize;
    return result;
}

SOPC_ReturnStatus SOPC_InterruptTimer_Update(SOPC_InterruptTimer* pTimer, uint32_t externalTickValue)
{
    if ((NULL == pTimer) || (NULL == pTimer->pData))
    {
        return SOPC_STATUS_NOK;
    }

    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    eInterruptTimerSyncStatus currentStatus = SOPC_InterruptTimer_IncrementInUseStatus(pTimer);

    // Check current workspace status
    if (currentStatus > E_INTERRUPT_TIMER_SYNC_STATUS_INITIALIZED)
    {
        tInterruptTimerData* pWks = pTimer->pData;

        // Check if UDPATE API is not in use.
        eInterruptTimerInstanceSyncStatus expectedStatus = E_INTERRUPT_TIMER_INSTANCE_SYNC_STATUS_NOT_USED;
        eInterruptTimerInstanceSyncStatus desiredStatus = E_INTERRUPT_TIMER_INSTANCE_SYNC_STATUS_RESERVED;
        bool bTransition = __atomic_compare_exchange(&pWks->bTickIsUpdating.instanceStatus, &expectedStatus,
                                                     &desiredStatus, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);

        // If API not in use, parse all timers instances and check callback invoke condition
        if (bTransition)
        {
            // Tick update
            {
                uint64_t ulTickValue = (uint64_t) externalTickValue;

                // Check overflow and update new tick value
                if (ulTickValue < (pWks->irqTicks & UINT32_MAX))
                {
                    pWks->irqTicks = (ulTickValue + ((uint64_t) UINT32_MAX) + 1) |
                                     (pWks->irqTicks & (uint64_t)(UINT64_MAX - UINT32_MAX));
                }
                else
                {
                    pWks->irqTicks = ulTickValue | (pWks->irqTicks & (uint64_t)(UINT64_MAX - UINT32_MAX));
                }
            }

            // Parse all instances
            for (uint32_t i = 0; i < pWks->nbInstances; i++)
            {
                {
                    // Indicate timer instance will be read
                    size_t idBuffer = UINT32_MAX;
                    result = SOPC_DoubleBuffer_GetReadBuffer(pWks->pTimerInstanceDoubleBuffer[i], &idBuffer);
                    assert(SOPC_STATUS_OK == result);

                    // Get pointer on data
                    tTimerInstanceInfo* ptrInfo = NULL;
                    result = SOPC_DoubleBuffer_ReadBufferPtr(pWks->pTimerInstanceDoubleBuffer[i], idBuffer,
                                                             (uint8_t**) &ptrInfo);
                    assert(SOPC_STATUS_OK == result);

                    // Verify minimum size
                    /* TODO: test removed because always true */
                    {
                        // Check status change (start)
                        if (pWks->pTimerInstancePreviousStatus[i] != ptrInfo->wStatus)
                        {
                            // If started, invoke start callback
                            if (ptrInfo->cbStart != NULL && SOPC_INTERRUPT_TIMER_STATUS_ENABLED == ptrInfo->wStatus)
                            {
                                ptrInfo->cbStart(i, ptrInfo->pUserContext);
                            }
                        }

                        // Check status and period
                        if (SOPC_INTERRUPT_TIMER_STATUS_ENABLED == ptrInfo->wStatus && ptrInfo->wPeriod > 0)
                        {
                            // Verify timeout
                            if (((pWks->irqTicks + ptrInfo->wOffset) % ptrInfo->wPeriod) == 0)
                            {
                                // Invoke elapsed callback
                                if (NULL != ptrInfo->cbElapsed)
                                {
                                    ptrInfo->cbElapsed(i, ptrInfo->pUserContext,
                                                       ((uint8_t*) ptrInfo) + sizeof(tTimerInstanceInfo),
                                                       (uint32_t) ptrInfo->dataSize); /* TODO: remove cast */
                                }
                            }
                        }

                        // Check status change and stop
                        if (pWks->pTimerInstancePreviousStatus[i] != ptrInfo->wStatus)
                        {
                            // If stopped, invoke stop callback
                            if (ptrInfo->cbStop != NULL && SOPC_INTERRUPT_TIMER_STATUS_DISABLED == ptrInfo->wStatus)
                            {
                                ptrInfo->cbStop(i, ptrInfo->pUserContext);
                            }
                        }

                        // Save new status reference
                        pWks->pTimerInstancePreviousStatus[i] = ptrInfo->wStatus;
                    }

                    // Release double buffer for this timer instance
                    SOPC_DoubleBuffer_ReleaseReadBuffer(pWks->pTimerInstanceDoubleBuffer[i], idBuffer);
                }

            } // End of parsing all instances

            // Indicate that UPDATE API is ready.
            desiredStatus = E_INTERRUPT_TIMER_INSTANCE_SYNC_STATUS_NOT_USED;
            __atomic_store(&pWks->bTickIsUpdating.instanceStatus, &desiredStatus, __ATOMIC_SEQ_CST);
        }
        else
        {
            result = SOPC_STATUS_INVALID_STATE;
        }
        SOPC_InterruptTimer_DecrementInUseStatus(pTimer);
    }
    else
    {
        result = SOPC_STATUS_NOK;
    }

    return result;
}

static inline tInterruptTimerData* SOPC_InterruptTimer_Workspace_Create(uint32_t nbInstances, uint32_t maxDataSize)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    tInterruptTimerData* pWks = SOPC_Calloc(1, sizeof(tInterruptTimerData));

    if (NULL == pWks || nbInstances < 1)
    {
        result = SOPC_STATUS_NOK;
    }

    if (SOPC_STATUS_OK == result)
    {
        pWks->nbInstances = nbInstances;
        pWks->maxTimerDataSize = maxDataSize;

        pWks->pTimerInstancePreviousStatus = SOPC_Calloc(1, nbInstances * sizeof(SOPC_IrqTimer_InstanceStatus));
        if (NULL == pWks->pTimerInstancePreviousStatus)
        {
            result = SOPC_STATUS_NOK;
        }
    }

    if (SOPC_STATUS_OK == result)
    {
        pWks->pTimerInstanceInUse = SOPC_Calloc(1, nbInstances * sizeof(struct T_INTERRUPT_TIMER_API_STATUS));
        if (NULL == pWks->pTimerInstanceInUse)
        {
            result = SOPC_STATUS_NOK;
        }
    }

    if (SOPC_STATUS_OK == result)
    {
        pWks->pTimerInstanceDoubleBuffer = SOPC_Calloc(1, nbInstances * sizeof(SOPC_DoubleBuffer*));
        if (NULL == pWks->pTimerInstanceDoubleBuffer)
        {
            result = SOPC_STATUS_NOK;
        }
    }

    for (uint32_t i = 0; i < nbInstances && SOPC_STATUS_OK == result; i++)
    {
        pWks->pTimerInstanceDoubleBuffer[i] =
            SOPC_DoubleBuffer_Create(2, (uint32_t)(sizeof(tTimerInstanceInfo) + maxDataSize));
        if (NULL == pWks->pTimerInstanceDoubleBuffer[i])
        {
            result = SOPC_STATUS_NOK;
        }
    }

    if (SOPC_STATUS_OK != result)
    {
        SOPC_InterruptTimer_Workspace_Destroy(&pWks);
    }

    return pWks;
}

static inline void SOPC_InterruptTimer_Workspace_Destroy(tInterruptTimerData** ppWks)
{
    if (NULL == ppWks)
    {
        return;
    }

    tInterruptTimerData* pWks = *ppWks;

    if (NULL != pWks)
    {
        if (NULL != pWks->pTimerInstancePreviousStatus)
        {
            SOPC_Free(pWks->pTimerInstancePreviousStatus);
            pWks->pTimerInstancePreviousStatus = NULL;
        }

        if (NULL != pWks->pTimerInstanceInUse)
        {
            SOPC_Free(pWks->pTimerInstanceInUse);
            pWks->pTimerInstanceInUse = NULL;
        }

        if (NULL != pWks->pTimerInstanceDoubleBuffer)
        {
            for (uint32_t i = 0; i < pWks->nbInstances; i++)
            {
                SOPC_DoubleBuffer_Destroy(&pWks->pTimerInstanceDoubleBuffer[i]);
            }
            SOPC_Free(pWks->pTimerInstanceDoubleBuffer);
            pWks->pTimerInstanceDoubleBuffer = NULL;
        }

        SOPC_Free(pWks);
        pWks = NULL;
    }

    *ppWks = NULL;
}

static inline eInterruptTimerSyncStatus SOPC_InterruptTimer_IncrementInUseStatus(SOPC_InterruptTimer* pTimer)
{
    {
        bool result = false;
        eInterruptTimerSyncStatus currentValue = 0;
        eInterruptTimerSyncStatus newValue = 0;

        do
        {
            // Load current counter and atomic increment it if possible
            __atomic_load(&pTimer->status, &currentValue, __ATOMIC_SEQ_CST);
            if (currentValue >= E_INTERRUPT_TIMER_SYNC_STATUS_INITIALIZED)
            {
                newValue = currentValue + 1;
            }
            else
            {
                newValue = currentValue;
            }
            result = __atomic_compare_exchange(&pTimer->status, &currentValue, &newValue, false, __ATOMIC_SEQ_CST,
                                               __ATOMIC_SEQ_CST);
        } while (!result);

        return newValue;
    }
}

static inline eInterruptTimerSyncStatus SOPC_InterruptTimer_DecrementInUseStatus(SOPC_InterruptTimer* pTimer)
{
    {
        bool result = false;
        eInterruptTimerSyncStatus currentValue = 0;
        eInterruptTimerSyncStatus newValue = 0;

        do
        {
            // Load current counter and atomic increment it if possible
            __atomic_load(&pTimer->status, &currentValue, __ATOMIC_SEQ_CST);
            if (currentValue > E_INTERRUPT_TIMER_SYNC_STATUS_INITIALIZED)
            {
                newValue = currentValue - 1;
            }
            else
            {
                newValue = currentValue;
            }
            result = __atomic_compare_exchange(&pTimer->status, &currentValue, &newValue, false, __ATOMIC_SEQ_CST,
                                               __ATOMIC_SEQ_CST);
        } while (!result);

        return newValue;
    }
}
