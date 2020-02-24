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

#include "sopc_interrupttimer.h"

// Status of an interrupt timer workspace
// ORDER of those status is important. INITIALIZED SHALL BE THE LAST STATUS !!!
typedef enum
{
    SOPC_INTERRUPT_TIMER_SYNC_STATUS_NOT_INITIALIZED, // Workspace is created and not initialized
    SOPC_INTERRUPT_TIMER_SYNC_STATUS_INITIALIZING,    // Initializing on going...
    SOPC_INTERRUPT_TIMER_SYNC_STATUS_DEINITIALIZING,  // Resetting on going...
    SOPC_INTERRUPT_TIMER_SYNC_STATUS_INITIALIZED,     // Workspace is initialized, timer instance can be used
    SOPC_INTERRUPT_TIMER_SYNC_STATUS_LOCKED, // Workspace is in use, some timer instances are called by their API
    SOPC_INTERRUPT_TIMER_SYNC_STATUS_SIZE = INT32_MAX
} SOPC_InterruptTimerStatus;

typedef enum
{
    SOPC_INTERRUPT_TIMER_STATUS_NOT_USED,
    SOPC_INTERRUPT_TIMER_STATUS_RESERVING,
    SOPC_INTERRUPT_TIMER_STATUS_RESERVED,
    SOPC_INTERRUPT_TIMER_STATUS_RELEASING,
    SOPC_INTERRUPT_TIMER_STATUS_SIZE = INT32_MAX
} E_API_IN_USE_STATUS;

#define member_size(type, member) sizeof(((type*) 0)->member)

// Header data of a timer instance information.
typedef struct T_TIMER_INSTANCE_INFO
{
    SOPC_IrqTimer_InstanceStatus wStatus;       // Timer status, 0 or 1
    uint32_t wPeriod;                           // Period in ticks (1 tick = 100µs)
    uint32_t wOffset;                           // Absolute offset in ticks
    void* pUserContext;                         // User context
    sopc_irq_timer_cb_start cbStart;            // User start callback
    sopc_irq_timer_cb_period_elapsed cbElapsed; // User period elapsed callback
    sopc_irq_timer_cb_stop cbStop;              // User stop callback. SHALL BE LAST FIELD
} __attribute__((packed)) tTimerInstanceInfo;

// Type which indicates if the API is in use.
typedef struct T_API_IN_USE
{
    E_API_IN_USE_STATUS instanceStatus;
} SOPC_InterruptTimerAPIStatus;

// Data of an interrupt timer workspace. It can contain several timer instances
typedef struct SOPC_INTERRUPT_TIMER_DATA
{
    uint32_t nbInstances;      // Maximum of timer instances
    uint32_t maxTimerDataSize; // Max of data hold by a timer instance
    uint64_t irqTicks;         // Current tick value.

    SOPC_InterruptTimerAPIStatus bTickIsUpdating; // Indicates if tick and timer evaluation function (update) is in use

    SOPC_InterruptTimerAPIStatus*
        pTimerInstanceInUse; // Indicates if set data, stop, start... function for a instance is in use
    SOPC_IrqTimer_InstanceStatus*
        pTimerInstancePreviousStatus;               // Table of previous status, used to generate start or stop event
    SOPC_DoubleBuffer** pTimerInstanceDoubleBuffer; // Table of DBO, one by timer instance.
} SOPC_InterruptTimerData;

// Interrupt timer definition
struct SOPC_InterruptTimer
{
    SOPC_InterruptTimerStatus status; // Status of the workspace
    SOPC_InterruptTimerData* pData;   // Timer interrupt workspace
};

struct SOPC_InterruptTimer_DataHandle
{
    struct SOPC_InterruptTimer* pTimer;
    uint32_t maxAllowedSize;
    uint32_t currentSize;
    uint8_t* pPublishedData;

    uint32_t idInstanceTimer;
    uint32_t idContainer;
    uint8_t* pDboData;
    uint32_t* pDboSize;
};

// Declaration of private function

// Workspace creation
static inline SOPC_InterruptTimerData* SOPC_InterruptTimer_Workspace_Create(
    uint32_t nbInstances,  // Maximum of timer instances (timer identifiers between 0 and nbInstances - 1)
    uint32_t maxDataSize); // Maximum of data in bytes hold by each timer

// Workspace destruction
static inline void SOPC_InterruptTimer_Workspace_Destroy(SOPC_InterruptTimerData** pWks);

// Increment number of API clients
static inline SOPC_InterruptTimerStatus SOPC_InterruptTimer_IncrementInUseStatus(SOPC_InterruptTimer* pTimer);

// Decrement number of API clients
static inline SOPC_InterruptTimerStatus SOPC_InterruptTimer_DecrementInUseStatus(SOPC_InterruptTimer* pTimer);

// Enable or disable timer instance
static inline SOPC_ReturnStatus SOPC_InterruptTimer_SetStatus(SOPC_InterruptTimer* pTimer, // Interrupt timer object
                                                              uint32_t idInstanceTimer,    // Id of timer
                                                              SOPC_IrqTimer_InstanceStatus status); // Status to set

// Create interrupt timer. This function shall be followed by initialize function to use the timer.
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

// Destroy interrupt timer. This function shall be called only if de initialized ended with OK result.
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

// Initialize interrupt timer
// Returns : SOPC_STATUS_INVALID_STATE if interrupt timer is in use, initializing or resetting state.
// SOPC_STATUS_NOK for others error. Else SOPC_STATUS_OK.
SOPC_ReturnStatus SOPC_InterruptTimer_Initialize(
    SOPC_InterruptTimer* pTimer,  // Interrupt timer object
    uint32_t nbInstances,         // Maximum of timer instances (timer identifiers between 0 and nbInstances - 1)
    uint32_t maxInstanceDataSize) // Maximum of data in bytes hold by each timer
{
    if (NULL == pTimer || nbInstances < 1)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    // Try to go to INITILIZING from NOT INITIALIZED state
    SOPC_InterruptTimerStatus expectedStatus = SOPC_INTERRUPT_TIMER_SYNC_STATUS_NOT_INITIALIZED;
    SOPC_InterruptTimerStatus desiredStatus = SOPC_INTERRUPT_TIMER_SYNC_STATUS_INITIALIZING;
    bool bTransition = __atomic_compare_exchange(&pTimer->status,
                                                 &expectedStatus,   //
                                                 &desiredStatus,    //
                                                 false,             //
                                                 __ATOMIC_SEQ_CST,  //
                                                 __ATOMIC_SEQ_CST); //

    // If status from NOT INITIALIZED, so create interrupt timer workspace.
    if (bTransition)
    {
        SOPC_InterruptTimerData* pWks = SOPC_InterruptTimer_Workspace_Create(nbInstances, maxInstanceDataSize);

        if (NULL == pWks)
        {
            result = SOPC_STATUS_NOK;
        }

        if (SOPC_STATUS_OK == result)
        {
            pTimer->pData = pWks;
            // Update status from INITIALIZING to INITIALIZED
            desiredStatus = SOPC_INTERRUPT_TIMER_SYNC_STATUS_INITIALIZED;
            __atomic_store(&pTimer->status, &desiredStatus, __ATOMIC_SEQ_CST);
        }
        else
        {
            pTimer->pData = NULL;
            // Update status from INITIALIZING to NOT INTIALIZED
            desiredStatus = SOPC_INTERRUPT_TIMER_SYNC_STATUS_NOT_INITIALIZED;
            __atomic_store(&pTimer->status, &desiredStatus, __ATOMIC_SEQ_CST);
        }
    }
    // Invalid state : initializing, resetting or in use status.
    else if (SOPC_INTERRUPT_TIMER_SYNC_STATUS_LOCKED <= expectedStatus ||
             SOPC_INTERRUPT_TIMER_SYNC_STATUS_DEINITIALIZING == expectedStatus ||
             SOPC_INTERRUPT_TIMER_SYNC_STATUS_INITIALIZING == expectedStatus)
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

// DeInitialize interrupt timer
// Returns : SOPC_STATUS_INVALID_STATE if interrupt timer is in other state that initialized (in use, resetting,
// initializing). SOPC_STATUS_NOK for others error or already not initialized. Else SOPC_STATUS_OK.
SOPC_ReturnStatus SOPC_InterruptTimer_DeInitialize(SOPC_InterruptTimer* pTimer)
{
    if (NULL == pTimer)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    // Try to go to DEINITIALIZED from INITIALIZED state
    SOPC_InterruptTimerStatus expectedStatus = SOPC_INTERRUPT_TIMER_SYNC_STATUS_INITIALIZED;
    SOPC_InterruptTimerStatus desiredStatus = SOPC_INTERRUPT_TIMER_SYNC_STATUS_DEINITIALIZING;
    bool bTransition = __atomic_compare_exchange(&pTimer->status,   //
                                                 &expectedStatus,   //
                                                 &desiredStatus,    //
                                                 false,             //
                                                 __ATOMIC_SEQ_CST,  //
                                                 __ATOMIC_SEQ_CST); //

    // If status from INITIALIZED, so destroy interrupt timer workspace.
    if (bTransition)
    {
        SOPC_InterruptTimer_Workspace_Destroy(&pTimer->pData);

        // Update status from  DEINITIALIZING to NOT INITIALIZED
        desiredStatus = SOPC_INTERRUPT_TIMER_SYNC_STATUS_NOT_INITIALIZED;
        __atomic_store(&pTimer->status, &desiredStatus, __ATOMIC_SEQ_CST);
    }
    else if (SOPC_INTERRUPT_TIMER_SYNC_STATUS_LOCKED <= expectedStatus ||
             SOPC_INTERRUPT_TIMER_SYNC_STATUS_DEINITIALIZING == expectedStatus ||
             SOPC_INTERRUPT_TIMER_SYNC_STATUS_INITIALIZING == expectedStatus)
    {
        result = SOPC_STATUS_INVALID_STATE;
    }
    else
    {
        result = SOPC_STATUS_NOK;
    }

    return result;
}

// Reset timer and initialize it on next update
// Returns : SOPC_STATUS_INVALID_STATE if workspace is not initialized or API for this instance is currently in use.
// SOPC_STATUS_NOK for others error. Else SOPC_STATUS_OK.
SOPC_ReturnStatus SOPC_InterruptTimer_Instance_Init(
    SOPC_InterruptTimer* pTimer,                // Interrupt timer object
    uint32_t idInstanceTimer,                   // Id of timer instance
    uint32_t period,                            // Timer Period
    uint32_t wOffset,                           // Timer offset
    void* pUserContext,                         // Free user context
    sopc_irq_timer_cb_start cbStart,            // Start event callback
    sopc_irq_timer_cb_period_elapsed cbElapsed, // Elapsed event callback
    sopc_irq_timer_cb_stop cbStop,
    SOPC_IrqTimer_InstanceStatus initStatus) // Stop event callback
{
    // Check for incorrect parameters
    if ((NULL == pTimer) || (NULL == pTimer->pData) || (idInstanceTimer >= pTimer->pData->nbInstances) ||
        (initStatus > SOPC_INTERRUPT_TIMER_STATUS_ENABLED || initStatus < SOPC_INTERRUPT_TIMER_STATUS_DISABLED))
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    // Try to update status from a status (if this is the case) INITIALIZED or BEYOND this status to this status + 1
    SOPC_InterruptTimerStatus currentStatus = SOPC_InterruptTimer_IncrementInUseStatus(pTimer);

    // If current status is a "in use" status, try to mark api as in use
    if (currentStatus > SOPC_INTERRUPT_TIMER_SYNC_STATUS_INITIALIZED)
    {
        SOPC_InterruptTimerData* pWks = pTimer->pData;

        // Try to go from not in use to in use status for the specified instance
        E_API_IN_USE_STATUS expectedValue = SOPC_INTERRUPT_TIMER_STATUS_NOT_USED;
        E_API_IN_USE_STATUS desiredValue = SOPC_INTERRUPT_TIMER_STATUS_RESERVING;
        bool bTransition = __atomic_compare_exchange(&pWks->pTimerInstanceInUse[idInstanceTimer].instanceStatus, //
                                                     &expectedValue,                                             //
                                                     &desiredValue,                                              //
                                                     false,                                                      //
                                                     __ATOMIC_SEQ_CST,                                           //
                                                     __ATOMIC_SEQ_CST);                                          //

        // If transition OK, get timer instance information in the associated double buffer
        // and set new configuration taken into account by next update call.
        if (bTransition)
        {
            // Try to get a not reading buffer
            uint32_t idBuffer = UINT32_MAX;

            SOPC_DoubleBuffer_GetWriteBuffer(pWks->pTimerInstanceDoubleBuffer[idInstanceTimer], &idBuffer, NULL);

            // Erase buffer
            SOPC_DoubleBuffer_WriteBufferErase(pWks->pTimerInstanceDoubleBuffer[idInstanceTimer], idBuffer);

            // Set timer instance configuration
            tTimerInstanceInfo timerInfo;
            timerInfo.wStatus = initStatus;
            timerInfo.wPeriod = period;
            timerInfo.wOffset = wOffset;
            timerInfo.pUserContext = pUserContext;
            timerInfo.cbElapsed = cbElapsed;
            timerInfo.cbStart = cbStart;
            timerInfo.cbStop = cbStop;

            // Try to write timer configuration
            uint32_t size = 0;

            SOPC_DoubleBuffer_WriteBuffer(pWks->pTimerInstanceDoubleBuffer[idInstanceTimer], // Double buffer object
                                          idBuffer,                                          // Id of buffer
                                          0,                                                 // No offset
                                          (uint8_t*) &timerInfo,                             // Data
                                          sizeof(tTimerInstanceInfo),                        // Data size
                                          &size,                                             // Written size
                                          true,  // Ignore the least old data before offset
                                          true); // Ignore the least old data after written data

            // Release buffer
            SOPC_DoubleBuffer_ReleaseWriteBuffer(pWks->pTimerInstanceDoubleBuffer[idInstanceTimer], &idBuffer);

            // Go from in use to not in use status for this timer instance
            desiredValue = SOPC_INTERRUPT_TIMER_STATUS_NOT_USED;
            __atomic_store(&pWks->pTimerInstanceInUse[idInstanceTimer].instanceStatus, &desiredValue, __ATOMIC_SEQ_CST);

            // Check if result is OK
            if (size < sizeof(tTimerInstanceInfo))
            {
                result = SOPC_STATUS_NOK;
            }
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

// Force next update of timer to stop without calling any intermediate callback (stop/start/elapsed)
// Returns : SOPC_STATUS_INVALID_STATE if workspace is not initialized or API for this instance is currently in use.
// SOPC_STATUS_NOK for others error. Else SOPC_STATUS_OK.
SOPC_ReturnStatus SOPC_InterruptTimer_Instance_DeInit(SOPC_InterruptTimer* pTimer, // Interrupt timer object
                                                      uint32_t idInstanceTimer)    // Timer instance identifier
{
    // Check for incorrect parameters
    if ((NULL == pTimer) || (NULL == pTimer->pData) || (idInstanceTimer >= pTimer->pData->nbInstances))
    {
        return SOPC_STATUS_NOK;
    }

    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    // Try to go current initialized status to this + 1
    SOPC_InterruptTimerStatus currentStatus = SOPC_InterruptTimer_IncrementInUseStatus(pTimer);

    // If valid status, try to erase timer configuration for specified instance
    if (currentStatus > SOPC_INTERRUPT_TIMER_SYNC_STATUS_INITIALIZED)
    {
        SOPC_InterruptTimerData* pWks = pTimer->pData;

        E_API_IN_USE_STATUS expectedValue = SOPC_INTERRUPT_TIMER_STATUS_NOT_USED;
        E_API_IN_USE_STATUS desiredValue = SOPC_INTERRUPT_TIMER_STATUS_RESERVING;
        bool bTransition = __atomic_compare_exchange(&pWks->pTimerInstanceInUse[idInstanceTimer].instanceStatus, //
                                                     &expectedValue,                                             //
                                                     &desiredValue,                                              //
                                                     false,                                                      //
                                                     __ATOMIC_SEQ_CST,                                           //
                                                     __ATOMIC_SEQ_CST);                                          //

        // If API is not in use for this instance, erase timer instance configuration to take into account by next
        // update call.
        if (bTransition)
        {
            // Reserve DBO for write
            uint32_t idBuffer = UINT32_MAX;
            SOPC_DoubleBuffer_GetWriteBuffer(pWks->pTimerInstanceDoubleBuffer[idInstanceTimer], &idBuffer, NULL);

            // Write DBO
            SOPC_DoubleBuffer_WriteBufferErase(pWks->pTimerInstanceDoubleBuffer[idInstanceTimer], idBuffer);

            // Release DBO
            SOPC_DoubleBuffer_ReleaseWriteBuffer(pWks->pTimerInstanceDoubleBuffer[idInstanceTimer], &idBuffer);

            // Mark API as not in use.
            desiredValue = SOPC_INTERRUPT_TIMER_STATUS_NOT_USED;
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

// Set a timer instance status : started or stopped. Used by Stop and Start public API.
// Returns : SOPC_STATUS_INVALID_STATE if workspace is not initialized or API for this instance is currently in use.
// SOPC_STATUS_NOK for others error. Else SOPC_STATUS_OK.
static inline SOPC_ReturnStatus SOPC_InterruptTimer_SetStatus(
    SOPC_InterruptTimer* pTimer,         // Interrupt timer object
    uint32_t idInstanceTimer,            // Timer instance identifier
    SOPC_IrqTimer_InstanceStatus status) // Status ENABLED or DISABLED
{
    if ((NULL == pTimer) || (NULL == pTimer->pData) || (idInstanceTimer >= pTimer->pData->nbInstances))
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    // Try to go current initialized status to this + 1
    SOPC_InterruptTimerStatus currentStatus = SOPC_InterruptTimer_IncrementInUseStatus(pTimer);

    // If valid status, try to set timer status for specified instance
    if (currentStatus > SOPC_INTERRUPT_TIMER_SYNC_STATUS_INITIALIZED)
    {
        SOPC_InterruptTimerData* pWks = pTimer->pData;

        E_API_IN_USE_STATUS expectedValue = SOPC_INTERRUPT_TIMER_STATUS_NOT_USED;
        E_API_IN_USE_STATUS desiredValue = SOPC_INTERRUPT_TIMER_STATUS_RESERVED;

        bool bTransition = __atomic_compare_exchange(&pWks->pTimerInstanceInUse[idInstanceTimer].instanceStatus, //
                                                     &expectedValue,                                             //
                                                     &desiredValue,                                              //
                                                     false,                                                      //
                                                     __ATOMIC_SEQ_CST,                                           //
                                                     __ATOMIC_SEQ_CST);                                          //

        // If API is not in use for this instance, set timer status
        if (bTransition)
        {
            // New status to take into account by next update call
            tTimerInstanceInfo timerInfo;
            timerInfo.wStatus = status;

            // Reserve DBO for write
            uint32_t idBuffer = UINT32_MAX;

            SOPC_DoubleBuffer_GetWriteBuffer(pWks->pTimerInstanceDoubleBuffer[idInstanceTimer], &idBuffer, NULL);

            // Try to write DBO
            uint32_t size = 0;

            SOPC_DoubleBuffer_WriteBuffer(pWks->pTimerInstanceDoubleBuffer[idInstanceTimer], // DBO object
                                          idBuffer,                                          // Reserved buffer
                                          offsetof(tTimerInstanceInfo, wStatus),             // Offset of status
                                          (uint8_t*) &timerInfo.wStatus,                     // value of status
                                          member_size(tTimerInstanceInfo, wStatus),          // size of status
                                          &size,                                             // written size
                                          false,  // Don't ignore the least old data previous offset
                                          false); // Don't ignore the least old data after written data

            // Check written size. Until wStatus field included minimum.
            if (size < offsetof(tTimerInstanceInfo, wStatus) + member_size(tTimerInstanceInfo, wStatus))
            {
                result = SOPC_STATUS_NOK;
            }
            else
            {
                // Release DBO
                SOPC_DoubleBuffer_ReleaseWriteBuffer(pWks->pTimerInstanceDoubleBuffer[idInstanceTimer], //
                                                     &idBuffer);                                        //
            }

            // Mark API as not in use.
            desiredValue = SOPC_INTERRUPT_TIMER_STATUS_NOT_USED;
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

// Get a timer instance status : started or stopped.
// Returns : SOPC_STATUS_INVALID_STATE if workspace is not initialized or API for this instance is currently in use.
// SOPC_STATUS_NOK for others error. Else SOPC_STATUS_OK.
SOPC_ReturnStatus SOPC_InterruptTimer_Instance_LastStatus(
    SOPC_InterruptTimer* pTimer,          // Interrupt timer object
    uint32_t idInstanceTimer,             // Timer instance identifier
    SOPC_IrqTimer_InstanceStatus* status) // Status ENABLED or DISABLED
{
    if ((NULL == pTimer) || (NULL == status) || (NULL == pTimer->pData) ||
        (idInstanceTimer >= pTimer->pData->nbInstances))
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    // Try to go current initialized status to this + 1
    SOPC_InterruptTimerStatus currentStatus = SOPC_InterruptTimer_IncrementInUseStatus(pTimer);

    // If valid status, try to set timer status for specified instance
    if (currentStatus > SOPC_INTERRUPT_TIMER_SYNC_STATUS_INITIALIZED)
    {
        SOPC_InterruptTimerData* pWks = pTimer->pData;

        E_API_IN_USE_STATUS expectedStatus = SOPC_INTERRUPT_TIMER_STATUS_NOT_USED;
        E_API_IN_USE_STATUS desiredStatus = SOPC_INTERRUPT_TIMER_STATUS_RESERVED;

        bool bTransition = __atomic_compare_exchange(&pWks->pTimerInstanceInUse[idInstanceTimer].instanceStatus, //
                                                     &expectedStatus,                                            //
                                                     &desiredStatus,                                             //
                                                     false,                                                      //
                                                     __ATOMIC_SEQ_CST,                                           //
                                                     __ATOMIC_SEQ_CST);                                          //

        // If API is not in use for this instance, set timer status
        if (bTransition)
        {
            // New status to take into account by next update call
            tTimerInstanceInfo* pTimerInfo;
            uint32_t* pSize = 0;

            // Reserve DBO for write
            uint32_t idBuffer = UINT32_MAX;

            SOPC_DoubleBuffer_GetWriteBuffer(pWks->pTimerInstanceDoubleBuffer[idInstanceTimer], &idBuffer, NULL);

            // Try to write DBO

            SOPC_DoubleBuffer_WriteBufferGetPtr(pWks->pTimerInstanceDoubleBuffer[idInstanceTimer], // DBO object
                                                idBuffer,                                          // Reserved buffer
                                                (uint8_t**) &pTimerInfo, // Pointer on data field
                                                &pSize,                  // Pointer on size field
                                                false); // Don't ignore the least old data after written data

            // Check written size. Until wStatus field included minimum.
            if (*pSize < offsetof(tTimerInstanceInfo, wStatus) + member_size(tTimerInstanceInfo, wStatus))
            {
                result = SOPC_STATUS_NOK;
            }
            else
            {
                *status = pTimerInfo->wStatus;
            }

            // Mark API as not in use.
            desiredStatus = SOPC_INTERRUPT_TIMER_STATUS_NOT_USED;
            __atomic_store(&pWks->pTimerInstanceInUse[idInstanceTimer].instanceStatus, //
                           &desiredStatus,                                             //
                           __ATOMIC_SEQ_CST);                                          //
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

// Next update of timer will start it
// Returns : SOPC_STATUS_INVALID_STATE if workspace is not initialized or API for this instance is currently in use.
// SOPC_STATUS_NOK for others error. Else SOPC_STATUS_OK.
SOPC_ReturnStatus SOPC_InterruptTimer_Instance_Start(SOPC_InterruptTimer* pTimer, // Interrupt timer object
                                                     uint32_t idInstanceTimer)    // Timer instance identifier
{
    return SOPC_InterruptTimer_SetStatus(pTimer, idInstanceTimer, SOPC_INTERRUPT_TIMER_STATUS_ENABLED);
}

// Next update of timer will stop it
// Returns : SOPC_STATUS_INVALID_STATE if workspace is not initialized or API for this instance is currently in use.
// SOPC_STATUS_NOK for others error. Else SOPC_STATUS_OK.
SOPC_ReturnStatus SOPC_InterruptTimer_Instance_Stop(SOPC_InterruptTimer* pTimer, // Interrupt timer object
                                                    uint32_t idInstanceTimer)    // Timer instance identifier
{
    return SOPC_InterruptTimer_SetStatus(pTimer, idInstanceTimer, SOPC_INTERRUPT_TIMER_STATUS_DISABLED);
}

// Next update of timer will set new period
// Returns : SOPC_STATUS_INVALID_STATE if workspace is not initialized or API for this instance is currently in use.
// SOPC_STATUS_NOK for others error. Else SOPC_STATUS_OK.
SOPC_ReturnStatus SOPC_InterruptTimer_Instance_SetPeriod(SOPC_InterruptTimer* pTimer, // Interrupt timer object
                                                         uint32_t idInstanceTimer,    // Timer instance identifier
                                                         uint32_t period)             // Period in ticks
{
    if ((NULL == pTimer) || (NULL == pTimer->pData) || (idInstanceTimer >= pTimer->pData->nbInstances) || (period < 1))
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    // Try to go current initialized status to this + 1
    SOPC_InterruptTimerStatus currentStatus = SOPC_InterruptTimer_IncrementInUseStatus(pTimer);

    // If valid status, try to set timer period for specified instance
    if (currentStatus > SOPC_INTERRUPT_TIMER_SYNC_STATUS_INITIALIZED)
    {
        SOPC_InterruptTimerData* pWks = pTimer->pData;

        E_API_IN_USE_STATUS expectedStatus = SOPC_INTERRUPT_TIMER_STATUS_NOT_USED;
        E_API_IN_USE_STATUS desiredStatus = SOPC_INTERRUPT_TIMER_STATUS_RESERVED;
        bool bTransition = __atomic_compare_exchange(&pWks->pTimerInstanceInUse[idInstanceTimer].instanceStatus, //
                                                     &expectedStatus,                                            //
                                                     &desiredStatus,                                             //
                                                     false,                                                      //
                                                     __ATOMIC_SEQ_CST,                                           //
                                                     __ATOMIC_SEQ_CST);                                          //

        // If API is not in use for this instance, set timer status
        if (bTransition)
        {
            // New period to take into account by next update call
            tTimerInstanceInfo timerInfo;
            timerInfo.wPeriod = period;

            // Reserve buffer
            uint32_t idBuffer = UINT32_MAX;

            SOPC_DoubleBuffer_GetWriteBuffer(pWks->pTimerInstanceDoubleBuffer[idInstanceTimer], &idBuffer, NULL);

            // Write buffer
            uint32_t size = 0;

            SOPC_DoubleBuffer_WriteBuffer(pWks->pTimerInstanceDoubleBuffer[idInstanceTimer], // DBO object
                                          idBuffer,                                          // Reserved buffer
                                          offsetof(tTimerInstanceInfo, wPeriod),    // Period offset in the buffer
                                          (uint8_t*) &timerInfo.wPeriod,            // Period value
                                          member_size(tTimerInstanceInfo, wPeriod), // Size of period value
                                          &size,                                    // Written size
                                          false,  // Don't ignore the least old data previous offset
                                          false); // Don't ignore the least old data after written data

            // Check written size. Until wPeriod field included minimum.
            if (size < offsetof(tTimerInstanceInfo, wPeriod) + member_size(tTimerInstanceInfo, wPeriod))
            {
                result = SOPC_STATUS_NOK;
            }
            else
            {
                // Release buffer
                SOPC_DoubleBuffer_ReleaseWriteBuffer(pWks->pTimerInstanceDoubleBuffer[idInstanceTimer], //
                                                     &idBuffer);                                        //
            }

            // Mark API as not in use.
            desiredStatus = SOPC_INTERRUPT_TIMER_STATUS_NOT_USED;
            __atomic_store(&pWks->pTimerInstanceInUse[idInstanceTimer].instanceStatus, //
                           &desiredStatus,                                             //
                           __ATOMIC_SEQ_CST);                                          //
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

// Next update of timer will set new period
// Returns : SOPC_STATUS_INVALID_STATE if workspace is not initialized or API for this instance is currently in use.
// SOPC_STATUS_NOK for others error. Else SOPC_STATUS_OK.
SOPC_ReturnStatus SOPC_InterruptTimer_Instance_SetOffset(SOPC_InterruptTimer* pTimer, // Interrupt timer object
                                                         uint32_t idInstanceTimer,    // Timer instance identifier
                                                         uint32_t wOffset)            // offset in ticks
{
    if ((NULL == pTimer) || (NULL == pTimer->pData) || (idInstanceTimer >= pTimer->pData->nbInstances))
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    // Try to go current initialized status to this + 1
    SOPC_InterruptTimerStatus currentStatus = SOPC_InterruptTimer_IncrementInUseStatus(pTimer);

    // If valid status, try to set timer period for specified instance
    if (currentStatus > SOPC_INTERRUPT_TIMER_SYNC_STATUS_INITIALIZED)
    {
        SOPC_InterruptTimerData* pWks = pTimer->pData;

        E_API_IN_USE_STATUS expectedStatus = SOPC_INTERRUPT_TIMER_STATUS_NOT_USED;
        E_API_IN_USE_STATUS desiredStatus = SOPC_INTERRUPT_TIMER_STATUS_RESERVED;
        bool bTransition = __atomic_compare_exchange(&pWks->pTimerInstanceInUse[idInstanceTimer].instanceStatus, //
                                                     &expectedStatus,                                            //
                                                     &desiredStatus,                                             //
                                                     false,                                                      //
                                                     __ATOMIC_SEQ_CST,                                           //
                                                     __ATOMIC_SEQ_CST);                                          //

        // If API is not in use for this instance, set timer status
        if (bTransition)
        {
            // New period to take into account by next update call
            tTimerInstanceInfo timerInfo;
            timerInfo.wOffset = wOffset;

            // Reserve buffer
            uint32_t idBuffer = UINT32_MAX;

            SOPC_DoubleBuffer_GetWriteBuffer(pWks->pTimerInstanceDoubleBuffer[idInstanceTimer], &idBuffer, NULL);

            // Write buffer
            uint32_t size = 0;

            SOPC_DoubleBuffer_WriteBuffer(pWks->pTimerInstanceDoubleBuffer[idInstanceTimer], // DBO object
                                          idBuffer,                                          // Reserved buffer
                                          offsetof(tTimerInstanceInfo, wOffset),    // Period offset in the buffer
                                          (uint8_t*) &timerInfo.wOffset,            // Period value
                                          member_size(tTimerInstanceInfo, wOffset), // Size of period value
                                          &size,                                    // Written size
                                          false,  // Don't ignore the least old data previous offset
                                          false); // Don't ignore the least old data after written data

            // Check written size. Until wPeriod field included minimum.
            if (size < offsetof(tTimerInstanceInfo, wOffset) + member_size(tTimerInstanceInfo, wOffset))
            {
                result = SOPC_STATUS_NOK;
            }
            else
            {
                // Release buffer
                SOPC_DoubleBuffer_ReleaseWriteBuffer(pWks->pTimerInstanceDoubleBuffer[idInstanceTimer], //
                                                     &idBuffer);                                        //
            }

            // Mark API as not in use.
            desiredStatus = SOPC_INTERRUPT_TIMER_STATUS_NOT_USED;
            __atomic_store(&pWks->pTimerInstanceInUse[idInstanceTimer].instanceStatus, //
                           &desiredStatus,                                             //
                           __ATOMIC_SEQ_CST);                                          //
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

// Next update of timer will set callback linked to start, stop, elapsed event
// Returns : SOPC_STATUS_INVALID_STATE if workspace is not initialized or API for this instance is currently in use.
// SOPC_STATUS_NOK for others error. Else SOPC_STATUS_OK.
SOPC_ReturnStatus SOPC_InterruptTimer_Instance_SetCallback(
    SOPC_InterruptTimer* pTimer,                // Interrupt timer object
    uint32_t idInstanceTimer,                   // Timer instance identifier
    void* pUserContext,                         // User context
    sopc_irq_timer_cb_start cbStart,            // Callback called when timer is started
    sopc_irq_timer_cb_period_elapsed cbElapsed, // Callback called when timer is elapsed
    sopc_irq_timer_cb_stop cbStop)              // Callback called when timer is stopped
{
    if ((NULL == pTimer) || (NULL == pTimer->pData) || (idInstanceTimer >= pTimer->pData->nbInstances))
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    // Try to go current initialized status to this + 1
    SOPC_InterruptTimerStatus currentStatus = SOPC_InterruptTimer_IncrementInUseStatus(pTimer);

    // If valid status, try to set timer period for specified instance
    if (currentStatus > SOPC_INTERRUPT_TIMER_SYNC_STATUS_INITIALIZED)
    {
        SOPC_InterruptTimerData* pWks = pTimer->pData;

        E_API_IN_USE_STATUS expectedStatus = SOPC_INTERRUPT_TIMER_STATUS_NOT_USED;
        E_API_IN_USE_STATUS desiredStatus = SOPC_INTERRUPT_TIMER_STATUS_RESERVED;
        bool bTransition = __atomic_compare_exchange(&pWks->pTimerInstanceInUse[idInstanceTimer].instanceStatus, //
                                                     &expectedStatus,                                            //
                                                     &desiredStatus,                                             //
                                                     false,                                                      //
                                                     __ATOMIC_SEQ_CST,                                           //
                                                     __ATOMIC_SEQ_CST);                                          //

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
            uint32_t idBuffer = UINT32_MAX;

            SOPC_DoubleBuffer_GetWriteBuffer(pWks->pTimerInstanceDoubleBuffer[idInstanceTimer], &idBuffer, NULL);

            // Write DBO
            uint32_t size = 0;

            SOPC_DoubleBuffer_WriteBuffer(pWks->pTimerInstanceDoubleBuffer[idInstanceTimer], // DBO buffer
                                          idBuffer,                                          // Buffer reserved to write
                                          offsetof(tTimerInstanceInfo, pUserContext),        // Offset of write
                                          (uint8_t*) &timerInfo.pUserContext,                // Data to write
                                          member_size(tTimerInstanceInfo, pUserContext) +    //
                                              member_size(tTimerInstanceInfo, cbElapsed) +   //
                                              member_size(tTimerInstanceInfo, cbStart) +     //
                                              member_size(tTimerInstanceInfo, cbStop),       // Size to write
                                          &size,                                             // written size
                                          false,  // Don't ignore previous last updated data before offset
                                          false); // Don't ignore after last field

            // Check written size. Until cbStop field included minimum.
            if (size < offsetof(tTimerInstanceInfo, cbStop) + member_size(tTimerInstanceInfo, cbStop))
            {
                result = SOPC_STATUS_NOK;
            }
            else
            {
                // Release DBO
                SOPC_DoubleBuffer_ReleaseWriteBuffer(pWks->pTimerInstanceDoubleBuffer[idInstanceTimer], //
                                                     &idBuffer);                                        //
            }

            // Mark API as not in use.
            desiredStatus = SOPC_INTERRUPT_TIMER_STATUS_NOT_USED;
            __atomic_store(&pWks->pTimerInstanceInUse[idInstanceTimer].instanceStatus, //
                           &desiredStatus,                                             //
                           __ATOMIC_SEQ_CST);                                          //
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

// Publish new data to the timer instance, take into account by next update call.
// Returns : SOPC_STATUS_INVALID_STATE if workspace is not initialized or API for this instance is currently in use.
// SOPC_STATUS_NOK for others error. Else SOPC_STATUS_OK.
SOPC_ReturnStatus SOPC_InterruptTimer_Instance_SetData(SOPC_InterruptTimer* pTimer, // Interrupt timer object
                                                       uint32_t idInstanceTimer,    // Timer instance identifier
                                                       uint8_t* pData,              // Data to publish
                                                       uint32_t sizeToWrite)        // Data size
{
    if ((NULL == pTimer) || (NULL == pTimer->pData) || (idInstanceTimer >= pTimer->pData->nbInstances))
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    // Try to go current initialized status to this + 1
    SOPC_InterruptTimerStatus currentStatus = SOPC_InterruptTimer_IncrementInUseStatus(pTimer);

    // If valid status, try to set timer period for specified instance
    if (currentStatus > SOPC_INTERRUPT_TIMER_SYNC_STATUS_INITIALIZED)
    {
        SOPC_InterruptTimerData* pWks = pTimer->pData;

        E_API_IN_USE_STATUS expectedStatus = SOPC_INTERRUPT_TIMER_STATUS_NOT_USED;
        E_API_IN_USE_STATUS desiredStatus = SOPC_INTERRUPT_TIMER_STATUS_RESERVED;
        bool bTransition = __atomic_compare_exchange(&pWks->pTimerInstanceInUse[idInstanceTimer].instanceStatus, //
                                                     &expectedStatus,                                            //
                                                     &desiredStatus,                                             //
                                                     false,                                                      //
                                                     __ATOMIC_SEQ_CST,                                           //
                                                     __ATOMIC_SEQ_CST);                                          //

        // If API is not in use for this instance, set timer status
        if (bTransition)
        {
            // Reserve DBO to write
            uint32_t idBuffer = UINT32_MAX;
            SOPC_DoubleBuffer_GetWriteBuffer(pWks->pTimerInstanceDoubleBuffer[idInstanceTimer], &idBuffer, NULL);

            // Write DBO
            uint32_t size = 0;

            SOPC_DoubleBuffer_WriteBuffer(pWks->pTimerInstanceDoubleBuffer[idInstanceTimer], // DBO buffer
                                          idBuffer,                                          // Buffer reserved to write
                                          sizeof(tTimerInstanceInfo), // Offset of write, after timer info
                                          pData,                      // Data to write
                                          sizeToWrite,                // Data size
                                          &size,                      // written size
                                          false,                      // Don't ignore before offset
                                          true);                      // Ignore after data written

            // Data shall not be less than header + size published, duplicated from last update.
            if (size < (sizeToWrite + sizeof(tTimerInstanceInfo)))
            {
                result = SOPC_STATUS_NOK;
            }
            else
            {
                // Release DBO
                SOPC_DoubleBuffer_ReleaseWriteBuffer(pWks->pTimerInstanceDoubleBuffer[idInstanceTimer], &idBuffer);
            }
            // Mark API as not in use.
            desiredStatus = SOPC_INTERRUPT_TIMER_STATUS_NOT_USED;
            __atomic_store(&pWks->pTimerInstanceInUse[idInstanceTimer].instanceStatus, //
                           &desiredStatus,                                             //
                           __ATOMIC_SEQ_CST);                                          //
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

//*** Specific API get data container ***

// Create data container for an interrupt timer object and one of its instances
SOPC_InterruptTimer_DataHandle* SOPC_InterruptTimer_Instance_DataHandle_Create(SOPC_InterruptTimer* pTimer, //
                                                                               uint32_t idInstanceTimer)    //
{
    SOPC_InterruptTimer_DataHandle* pContainer = NULL;

    if ((NULL == (pTimer) || (NULL == pTimer->pData) || idInstanceTimer >= pTimer->pData->nbInstances))
    {
        return NULL;
    }

    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    // Try to go current initialized status to this + 1
    SOPC_InterruptTimerStatus currentStatus = SOPC_InterruptTimer_IncrementInUseStatus(pTimer);

    // If valid status, try to set timer period for specified instance
    if (currentStatus > SOPC_INTERRUPT_TIMER_SYNC_STATUS_INITIALIZED)
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
            pContainer->idContainer = UINT32_MAX;
        }

        SOPC_InterruptTimer_DecrementInUseStatus(pTimer);
    }

    return pContainer;
}

// Destroy data container
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

// Initialize data handle used to publish data.
// This function shall be followed by Commit in order to take into account new data size
SOPC_ReturnStatus SOPC_InterruptTimer_Instance_DataHandle_Initialize(
    SOPC_InterruptTimer_DataHandle* pDataHandle // Data handle
)
{
    if ((NULL == pDataHandle) || (NULL == (pDataHandle)->pTimer) || (NULL == (pDataHandle)->pTimer->pData) ||
        ((pDataHandle)->idInstanceTimer >= (pDataHandle)->pTimer->pData->nbInstances))
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    uint32_t idInstanceTimer = pDataHandle->idInstanceTimer;
    SOPC_InterruptTimer* pTimer = pDataHandle->pTimer;
    SOPC_InterruptTimerData* pTimerData = pDataHandle->pTimer->pData;
    SOPC_DoubleBuffer* pDbo = pTimer->pData->pTimerInstanceDoubleBuffer[idInstanceTimer];

    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    // Try to go current initialized status to this + 1
    SOPC_InterruptTimerStatus currentStatus = SOPC_InterruptTimer_IncrementInUseStatus(pTimer);

    // If valid status, try to set timer period for specified instance
    if (currentStatus > SOPC_INTERRUPT_TIMER_SYNC_STATUS_INITIALIZED)
    {
        E_API_IN_USE_STATUS expectedStatus = SOPC_INTERRUPT_TIMER_STATUS_NOT_USED;
        E_API_IN_USE_STATUS desiredStatus = SOPC_INTERRUPT_TIMER_STATUS_RESERVING;
        bool bTransition =
            __atomic_compare_exchange(&pTimerData->pTimerInstanceInUse[idInstanceTimer].instanceStatus, //
                                      &expectedStatus,                                                  //
                                      &desiredStatus,                                                   //
                                      false,                                                            //
                                      __ATOMIC_SEQ_CST,                                                 //
                                      __ATOMIC_SEQ_CST);                                                //

        // If API is not in use for this instance, set timer status
        if (bTransition)
        {
            pDataHandle->idContainer = UINT32_MAX;

            result = SOPC_DoubleBuffer_GetWriteBuffer(pDbo,                      //
                                                      &pDataHandle->idContainer, //
                                                      NULL);                     //

            if (SOPC_STATUS_OK == result)
            {
                result = SOPC_DoubleBuffer_WriteBufferGetPtr(pDbo,                     //
                                                             pDataHandle->idContainer, //
                                                             &pDataHandle->pDboData,   //
                                                             &pDataHandle->pDboSize,   //
                                                             false);                   //
            }

            if (SOPC_STATUS_OK == result)
            {
                pDataHandle->currentSize = (uint32_t)((*pDataHandle->pDboSize) - sizeof(tTimerInstanceInfo));
                pDataHandle->pPublishedData = (pDataHandle->pDboData + sizeof(tTimerInstanceInfo));
            }

            if (SOPC_STATUS_OK != result)
            {
                pDataHandle->idContainer = UINT32_MAX;
                pDataHandle->pDboData = NULL;
                pDataHandle->pDboSize = 0;
                pDataHandle->pPublishedData = NULL;
                pDataHandle->currentSize = 0;

                result = SOPC_STATUS_NOK;

                desiredStatus = SOPC_INTERRUPT_TIMER_STATUS_NOT_USED;
                __atomic_store(&pTimerData->pTimerInstanceInUse[idInstanceTimer].instanceStatus, //
                               &desiredStatus,                                                   //
                               __ATOMIC_SEQ_CST);                                                //
            }
            else
            {
                SOPC_InterruptTimer_IncrementInUseStatus(pTimer);
                desiredStatus = SOPC_INTERRUPT_TIMER_STATUS_RESERVED;
                __atomic_store(&pTimerData->pTimerInstanceInUse[idInstanceTimer].instanceStatus, //
                               &desiredStatus,                                                   //
                               __ATOMIC_SEQ_CST);                                                //
            }
        }
        else
        {
            result = SOPC_STATUS_INVALID_STATE;
        }

        SOPC_InterruptTimer_DecrementInUseStatus(pDataHandle->pTimer);
    }
    else
    {
        result = SOPC_STATUS_NOK;
    }

    return result;
}

// Commit data container with new data
SOPC_ReturnStatus SOPC_InterruptTimer_Instance_DataHandle_Finalize(SOPC_InterruptTimer_DataHandle* pDataHandle, //
                                                                   bool bCancel)                                //
{
    if ((NULL == pDataHandle) || (NULL == pDataHandle->pTimer) || (NULL == pDataHandle->pTimer->pData) ||
        (pDataHandle->idInstanceTimer >= pDataHandle->pTimer->pData->nbInstances))
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    uint32_t idInstanceTimer = pDataHandle->idInstanceTimer;
    SOPC_InterruptTimer* pTimer = pDataHandle->pTimer;
    SOPC_InterruptTimerData* pTimerData = pDataHandle->pTimer->pData;
    SOPC_DoubleBuffer* pDbo = pTimer->pData->pTimerInstanceDoubleBuffer[idInstanceTimer];

    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    // Try to go current initialized status to this + 1
    SOPC_InterruptTimerStatus currentStatus = SOPC_InterruptTimer_IncrementInUseStatus(pTimer);

    // If valid status, try to set timer period for specified instance
    if (currentStatus > SOPC_INTERRUPT_TIMER_SYNC_STATUS_INITIALIZED)
    {
        E_API_IN_USE_STATUS expectedStatus = SOPC_INTERRUPT_TIMER_STATUS_RESERVED;
        E_API_IN_USE_STATUS desiredStatus = SOPC_INTERRUPT_TIMER_STATUS_RELEASING;
        bool bTransition =
            __atomic_compare_exchange(&pTimerData->pTimerInstanceInUse[idInstanceTimer].instanceStatus, //
                                      &expectedStatus,                                                  //
                                      &desiredStatus,                                                   //
                                      false,                                                            //
                                      __ATOMIC_SEQ_CST,                                                 //
                                      __ATOMIC_SEQ_CST);                                                //

        // If API is not in use for this instance, set timer status
        if (bTransition)
        {
            if (pDataHandle->currentSize > pTimerData->maxTimerDataSize && !bCancel)
            {
                result = SOPC_STATUS_OUT_OF_MEMORY;
            }

            if (SOPC_STATUS_OK == result && !bCancel)
            {
                *(pDataHandle->pDboSize) = (uint32_t)(pDataHandle->currentSize + sizeof(tTimerInstanceInfo));

                result = SOPC_DoubleBuffer_ReleaseWriteBuffer(pDbo,                         //
                                                              &(pDataHandle->idContainer)); //
            }

            pDataHandle->idContainer = UINT32_MAX;

            desiredStatus = SOPC_INTERRUPT_TIMER_STATUS_NOT_USED;

            __atomic_store(&pTimerData->pTimerInstanceInUse[idInstanceTimer].instanceStatus, //
                           &desiredStatus,                                                   //
                           __ATOMIC_SEQ_CST);                                                //

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

// Expose data container.
// This function shall be used between InitDataContainer and CommitDataContainer.
SOPC_ReturnStatus SOPC_InterruptTimer_Instance_DataHandle_GetBufferInfo(SOPC_InterruptTimer_DataHandle* pContainer,
                                                                        uint32_t* pMaxAllowedSize,
                                                                        uint32_t* pCurrentSize,
                                                                        uint8_t** ppData)
{
    if (NULL == pContainer || NULL == pMaxAllowedSize || NULL == pCurrentSize || NULL == ppData)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    *pMaxAllowedSize = pContainer->maxAllowedSize;
    *pCurrentSize = pContainer->currentSize;
    *ppData = pContainer->pPublishedData;

    return result;
}

// Update data handle size via SOPC_Buffer structure currentSize field.
// This function shall be used between Initialize  and Finalize
SOPC_ReturnStatus SOPC_InterruptTimer_Instance_DataHandle_SetNewSize(SOPC_InterruptTimer_DataHandle* pContainer, //
                                                                     uint32_t newSize)                           //

{
    if (NULL == pContainer || newSize > pContainer->maxAllowedSize)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    pContainer->currentSize = newSize;
    return result;
}

//*** -- ***

// Update timer tick with external tick value. Invoke callback if necessary.
// Returns : SOPC_STATUS_INVALID_STATE if workspace is not initialized or API for this instance is currently in use.
// SOPC_STATUS_NOK for others error. Else SOPC_STATUS_OK.
SOPC_ReturnStatus SOPC_InterruptTimer_Update(SOPC_InterruptTimer* pTimer, // Interrupt timer object
                                             uint32_t externalTickValue)  // Tick value. Shall be always incremented.
{
    if ((NULL == pTimer) || (NULL == pTimer->pData))
    {
        return SOPC_STATUS_NOK;
    }

    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    SOPC_InterruptTimerStatus currentStatus = SOPC_InterruptTimer_IncrementInUseStatus(pTimer);

    // Check current workspace status
    if (currentStatus > SOPC_INTERRUPT_TIMER_SYNC_STATUS_INITIALIZED)
    {
        SOPC_InterruptTimerData* pWks = pTimer->pData;

        // Check if UDPATE API is not in use.
        E_API_IN_USE_STATUS expectedStatus = SOPC_INTERRUPT_TIMER_STATUS_NOT_USED;
        E_API_IN_USE_STATUS desiredStatus = SOPC_INTERRUPT_TIMER_STATUS_RESERVED;
        bool bTransition = __atomic_compare_exchange(&pWks->bTickIsUpdating.instanceStatus, //
                                                     &expectedStatus,                       //
                                                     &desiredStatus,                        //
                                                     false,                                 //
                                                     __ATOMIC_SEQ_CST,
                                                     __ATOMIC_SEQ_CST); //

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
                    uint32_t idBuffer = UINT32_MAX;

                    SOPC_DoubleBuffer_GetReadBuffer(pWks->pTimerInstanceDoubleBuffer[i], &idBuffer);

                    // Get pointer on data
                    tTimerInstanceInfo* ptrInfo = NULL;
                    uint32_t size = 0;

                    SOPC_DoubleBuffer_ReadBufferPtr(pWks->pTimerInstanceDoubleBuffer[i], //
                                                    idBuffer,                            //
                                                    (uint8_t**) &ptrInfo,
                                                    &size); //

                    // Verify minimum size
                    if (size >= sizeof(tTimerInstanceInfo))
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
                                    ptrInfo->cbElapsed(
                                        i,                                                 // Timer id
                                        ptrInfo->pUserContext,                             // User context
                                        ((uint8_t*) ptrInfo) + sizeof(tTimerInstanceInfo), // Data published
                                        (uint32_t)(size - sizeof(tTimerInstanceInfo)));    // Data size
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
                    else
                    {
                        // Invalid timer info reference tick and status reset.
                        pWks->pTimerInstancePreviousStatus[i] = SOPC_INTERRUPT_TIMER_STATUS_INVALID;
                    }

                    // Release double buffer for this timer instance
                    SOPC_DoubleBuffer_ReleaseReadBuffer(pWks->pTimerInstanceDoubleBuffer[i], //
                                                        &idBuffer);                          //
                }

            } // End of parsing all instances

            // Indicate that UPDATE API is ready.
            desiredStatus = SOPC_INTERRUPT_TIMER_STATUS_NOT_USED;
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

// Creation of interrupt timer workspace, called by SOPC_InterruptTimer_Initialize.
static inline SOPC_InterruptTimerData* SOPC_InterruptTimer_Workspace_Create(
    uint32_t nbInstances, // Max instanciable timer
    uint32_t maxDataSize) // Max size of data hold by each timer
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    SOPC_InterruptTimerData* pWks = SOPC_Calloc(1, sizeof(SOPC_InterruptTimerData));

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
        pWks->pTimerInstanceInUse = SOPC_Calloc(1, nbInstances * sizeof(struct T_API_IN_USE));
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
            SOPC_DoubleBuffer_Create(1, (uint32_t)(sizeof(tTimerInstanceInfo) + maxDataSize));
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

// Destroy interrupt timer workspace. Called by SOPC_InterruptTimer_DeInitialize.
static inline void SOPC_InterruptTimer_Workspace_Destroy(SOPC_InterruptTimerData** ppWks)
{
    if (NULL == ppWks)
    {
        return;
    }

    SOPC_InterruptTimerData* pWks = *ppWks;

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

// Increment utilization of module counter. Try to pass INITIALIZED STATUS or beyond to this status + 1.
static inline SOPC_InterruptTimerStatus SOPC_InterruptTimer_IncrementInUseStatus(SOPC_InterruptTimer* pTimer)
{
    {
        bool result = false;
        SOPC_InterruptTimerStatus currentValue = 0;
        SOPC_InterruptTimerStatus newValue = 0;

        do
        {
            // Load current counter and atomic increment it if possible
            __atomic_load(&pTimer->status, &currentValue, __ATOMIC_SEQ_CST);
            if (currentValue >= SOPC_INTERRUPT_TIMER_SYNC_STATUS_INITIALIZED)
            {
                newValue = currentValue + 1;
            }
            else
            {
                newValue = currentValue;
            }
            result = __atomic_compare_exchange(&pTimer->status,   //
                                               &currentValue,     //
                                               &newValue,         //
                                               false,             //
                                               __ATOMIC_SEQ_CST,  //
                                               __ATOMIC_SEQ_CST); //
        } while (!result);

        return newValue;
    }
}

// Decrement utilization of module counter.  Try to pass INITIALIZED STATUS + 1 or beyond to this status - 1.
static inline SOPC_InterruptTimerStatus SOPC_InterruptTimer_DecrementInUseStatus(SOPC_InterruptTimer* pTimer)
{
    {
        bool result = false;
        SOPC_InterruptTimerStatus currentValue = 0;
        SOPC_InterruptTimerStatus newValue = 0;

        do
        {
            // Load current counter and atomic increment it if possible
            __atomic_load(&pTimer->status, &currentValue, __ATOMIC_SEQ_CST);
            if (currentValue > SOPC_INTERRUPT_TIMER_SYNC_STATUS_INITIALIZED)
            {
                newValue = currentValue - 1;
            }
            else
            {
                newValue = currentValue;
            }
            result = __atomic_compare_exchange(&pTimer->status,   //
                                               &currentValue,     //
                                               &newValue,         //
                                               false,             //
                                               __ATOMIC_SEQ_CST,  //
                                               __ATOMIC_SEQ_CST); //
        } while (!result);

        return newValue;
    }
}
