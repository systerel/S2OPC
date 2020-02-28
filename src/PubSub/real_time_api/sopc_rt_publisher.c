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

#include "sopc_rt_publisher.h"

// Private status of RT Publisher
typedef enum E_PUBLISHER_SYNC_STATUS
{
    E_PUBLISHER_SYNC_STATUS_NOT_INITIALIZED,
    E_PUBLISHER_SYNC_STATUS_INITIALIZING,
    E_PUBLISHER_SYNC_STATUS_DEINITIALIZING,
    E_PUBLISHER_SYNC_STATUS_INITIALIZED,
    E_PUBLUSHER_SYNC_STATUS_LOCKED,
    E_PUBLISHER_SYNC_STATUS_SIZE = INT32_MAX
} ePublisherSyncStatus;

// Private message publication configuration
typedef struct SOPC_RT_Publisher_Message_Config
{
    uint32_t period;                             // Period in beat heart
    uint32_t offset;                             // Offset in beat heart
    void* pUserContext;                          // User context.
    ptrCallbackStart cbMessagePublishingStarted; // Message publishing status change from STOP to START
    ptrCallbackSend cbMessageSend;               // Message data should be sent
    ptrCallbackStop cbMessagePublishingStopped;  // Message publishing status change from START to STOP
    SOPC_RT_Publisher_MessageStatus initialStatus;
    struct SOPC_RT_Publisher_Message_Config* next;
} SOPC_RT_Publisher_Message_Config;

// RT Publisher Initializer
struct SOPC_RT_Publisher_Initializer
{
    uint32_t nbMessages;                              // Number of messages handled
    uint32_t maxSizeOfMessage;                        // Max size of a message
    SOPC_RT_Publisher_Message_Config* pMessageConfig; // Message configuration list
};

struct SOPC_RT_Publisher
{
    ePublisherSyncStatus status; // RT publisher status
    uint32_t nbMessages;
    uint32_t maxSizeOfMessage;
    SOPC_InterruptTimer* pInterruptTimer; // Interrupt Timer workspace
    SOPC_InterruptTimer_DataHandle** ppTmrDataHandle;
};

// Private functions declarations

static void _SOPC_RT_Publisher_DeInitialize(SOPC_RT_Publisher* pPub);
static SOPC_ReturnStatus _SOPC_RT_Publisher_Initialize(SOPC_RT_Publisher* pPub, SOPC_RT_Publisher_Initializer* pConfig);
static inline ePublisherSyncStatus SOPC_RT_Publisher_IncrementInUseStatus(SOPC_RT_Publisher* pRtPub);
static inline ePublisherSyncStatus SOPC_RT_Publisher_DecrementInUseStatus(SOPC_RT_Publisher* pRtPub);

// Public functions definitions

// Creation of initializer. It will be used by a RT publisher initialization. After used by RT publisher function, this
// one can be destroyed. Configuration will be initialized with Initializer_AddMessage function.
// Returns : NULL if invalid  parameters or out of memory.
SOPC_RT_Publisher_Initializer* SOPC_RT_Publisher_Initializer_Create(
    uint32_t maxSizeOfMessage) // Max size of a message, common to all messages
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    SOPC_RT_Publisher_Initializer* pInitializer = SOPC_Calloc(1, sizeof(SOPC_RT_Publisher_Initializer));
    if (NULL == pInitializer || maxSizeOfMessage < 1)
    {
        return NULL;
    }

    pInitializer->nbMessages = 0;
    pInitializer->maxSizeOfMessage = maxSizeOfMessage;
    pInitializer->pMessageConfig = NULL;

    if (SOPC_STATUS_OK != result)
    {
        SOPC_Free(pInitializer);
        pInitializer = NULL;
    }

    return pInitializer;
}

// Destroy initializer.
void SOPC_RT_Publisher_Initializer_Destroy(SOPC_RT_Publisher_Initializer** ppInitializer)
{
    if (NULL != ppInitializer)
    {
        if ((*ppInitializer) != NULL)
        {
            if (NULL != (*ppInitializer)->pMessageConfig)
            {
                SOPC_RT_Publisher_Message_Config* pToFree = NULL;
                while ((*ppInitializer)->pMessageConfig != NULL)
                {
                    pToFree = (*ppInitializer)->pMessageConfig;
                    (*ppInitializer)->pMessageConfig = (*ppInitializer)->pMessageConfig->next;
                    SOPC_Free(pToFree);
                }
            }

            SOPC_Free((*ppInitializer));
            *ppInitializer = NULL;
        }
    }
}

// Adding message publishing configuration to publisher initializer
// Returns : SOPC_STATUS_OK if valid message identifier is returned.
SOPC_ReturnStatus SOPC_RT_Publisher_Initializer_AddMessage(
    SOPC_RT_Publisher_Initializer* pConfig,        // RT publisher configuration
    uint32_t period,                               // Period in heart beats
    uint32_t offset,                               // Offset in heart beats
    void* pContext,                                // User context
    ptrCallbackStart cbStart,                      // Start callback
    ptrCallbackSend cbSend,                        // Send callback
    ptrCallbackStop cbStop,                        // Stop callback
    SOPC_RT_Publisher_MessageStatus initialStatus, // initial status
    uint32_t* pOutMsgId)                           // ===> Message Identifier
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    if (pConfig == NULL || period < 1 || cbSend == NULL || cbStop == NULL || cbStart == NULL || NULL == pOutMsgId)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_RT_Publisher_Message_Config* pMsg = NULL;

    if (SOPC_STATUS_OK == result)
    {
        pMsg = SOPC_Calloc(1, sizeof(SOPC_RT_Publisher_Message_Config));

        if (pMsg == NULL)
        {
            result = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }

    if (SOPC_STATUS_OK == result)
    {
        pMsg->cbMessageSend = cbSend;
        pMsg->cbMessagePublishingStopped = cbStop;
        pMsg->cbMessagePublishingStarted = cbStart;
        pMsg->pUserContext = pContext;
        pMsg->period = period;
        pMsg->offset = offset;
        pMsg->initialStatus = initialStatus;
        pMsg->next = NULL;

        SOPC_RT_Publisher_Message_Config* pMsgIter = pConfig->pMessageConfig;
        SOPC_RT_Publisher_Message_Config* pMsgPrevious = NULL;

        while (pMsgIter != NULL)
        {
            pMsgPrevious = pMsgIter;
            pMsgIter = pMsgIter->next;
        }

        if (NULL == pMsgPrevious)
        {
            pConfig->pMessageConfig = pMsg;
        }
        else
        {
            pMsgPrevious->next = pMsg;
        }

        *pOutMsgId = pConfig->nbMessages;

        pConfig->nbMessages++;
    }

    return result;
}

// Start RT publisher with initializer in parameter
// Returns : SOPC_STATUS_OK if switches to initialized from not initialized status.
// NOK if already initialized.
// INVALID STATE in the other status
SOPC_ReturnStatus SOPC_RT_Publisher_Initialize(SOPC_RT_Publisher* pPub,                     // RT Publisher object
                                               SOPC_RT_Publisher_Initializer* pInitializer) // RT Publisher initializer
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    if (NULL == pPub || NULL == pInitializer)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    ePublisherSyncStatus expectedValue = E_PUBLISHER_SYNC_STATUS_NOT_INITIALIZED;
    ePublisherSyncStatus desiredValue = E_PUBLISHER_SYNC_STATUS_INITIALIZING;
    bool bTransition = __atomic_compare_exchange(&pPub->status,     //
                                                 &expectedValue,    //
                                                 &desiredValue,     //
                                                 false,             //
                                                 __ATOMIC_SEQ_CST,  //
                                                 __ATOMIC_SEQ_CST); //

    if (bTransition)
    {
        result = _SOPC_RT_Publisher_Initialize(pPub, pInitializer);
        if (SOPC_STATUS_OK == result)
        {
            desiredValue = E_PUBLISHER_SYNC_STATUS_INITIALIZED;
            __atomic_store(&pPub->status, &desiredValue, __ATOMIC_SEQ_CST);
        }
        else
        {
            _SOPC_RT_Publisher_DeInitialize(pPub);
            desiredValue = E_PUBLISHER_SYNC_STATUS_NOT_INITIALIZED;
            __atomic_store(&pPub->status, &desiredValue, __ATOMIC_SEQ_CST);
        }
    }
    else if (E_PUBLISHER_SYNC_STATUS_INITIALIZING == expectedValue ||   //
             E_PUBLISHER_SYNC_STATUS_DEINITIALIZING == expectedValue || //
             E_PUBLUSHER_SYNC_STATUS_LOCKED <= expectedValue)           //
    {
        result = SOPC_STATUS_INVALID_STATE;
    }
    else
    {
        result = SOPC_STATUS_NOK;
    }
    return result;
}

// Stop RT publisher
// Returns : SOPC_STATUS_OK if switches to no initialized from initialized status.
// NOK if already stopped.
// SOPC_INVALID_STATE could be returned if API is in use or if already stopped(BeatHeart, MessageSetValue... )
// In that case, retry to stop.
SOPC_ReturnStatus SOPC_RT_Publisher_DeInitialize(SOPC_RT_Publisher* pPub)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    if (NULL == pPub)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    ePublisherSyncStatus expectedValue = E_PUBLISHER_SYNC_STATUS_INITIALIZED;
    ePublisherSyncStatus desiredValue = E_PUBLISHER_SYNC_STATUS_DEINITIALIZING;
    bool bTransition = __atomic_compare_exchange(&pPub->status,     //
                                                 &expectedValue,    //
                                                 &desiredValue,     //
                                                 false,             //
                                                 __ATOMIC_SEQ_CST,  //
                                                 __ATOMIC_SEQ_CST); //

    if (bTransition)

    {
        _SOPC_RT_Publisher_DeInitialize(pPub);
        desiredValue = E_PUBLISHER_SYNC_STATUS_NOT_INITIALIZED;
        __atomic_store(&pPub->status, &desiredValue, __ATOMIC_SEQ_CST);
    }
    else if (E_PUBLISHER_SYNC_STATUS_INITIALIZING == expectedValue ||   //
             E_PUBLISHER_SYNC_STATUS_DEINITIALIZING == expectedValue || //
             E_PUBLUSHER_SYNC_STATUS_LOCKED <= expectedValue)           //
    {
        result = SOPC_STATUS_INVALID_STATE;
    }
    else
    {
        result = SOPC_STATUS_NOK;
    }

    return result;
}

// RT Publisher object creation.
SOPC_RT_Publisher* SOPC_RT_Publisher_Create(void)
{
    return (SOPC_RT_Publisher*) SOPC_Calloc(1, sizeof(struct SOPC_RT_Publisher));
}

// RT Publisher object destruction.
void SOPC_RT_Publisher_Destroy(SOPC_RT_Publisher** ppPubRt)
{
    if (NULL != ppPubRt)
    {
        if (NULL != (*ppPubRt))
        {
            SOPC_Free((*ppPubRt));
            *ppPubRt = NULL;
        }
    }
    return;
}

// Get the next message publishing status, which will be taken into account by the next heart beat.
SOPC_ReturnStatus SOPC_RT_Publisher_GetMessagePubStatus(
    SOPC_RT_Publisher* pPub,                  // Publisher object
    uint32_t msgIdentifier,                   // Message identifier
    SOPC_RT_Publisher_MessageStatus* pStatus) // Next status will be taken into account by next heart beat
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    if (NULL == pPub || NULL == pStatus)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    ePublisherSyncStatus currentStatus = SOPC_RT_Publisher_IncrementInUseStatus(pPub);

    if (currentStatus > E_PUBLISHER_SYNC_STATUS_INITIALIZED)
    {
        SOPC_IrqTimer_InstanceStatus status = SOPC_INTERRUPT_TIMER_STATUS_INVALID;
        result = SOPC_InterruptTimer_Instance_LastStatus(pPub->pInterruptTimer, //
                                                         msgIdentifier,         //
                                                         &status);              //
        if (SOPC_STATUS_OK == result)
        {
            switch (status)
            {
            case SOPC_INTERRUPT_TIMER_STATUS_DISABLED:
                *pStatus = SOPC_RT_PUBLISHER_MSG_PUB_STATUS_DISABLED;
                break;
            case SOPC_INTERRUPT_TIMER_STATUS_ENABLED:
                *pStatus = SOPC_RT_PUBLISHER_MSG_PUB_STATUS_ENABLED;
                break;
            default:
                *pStatus = SOPC_RT_PUBLISHER_MSG_PUB_STATUS_SIZE;
                break;
            }
        }

        SOPC_RT_Publisher_DecrementInUseStatus(pPub);
    }
    else
    {
        result = SOPC_STATUS_INVALID_STATE;
    }

    return result;
}

// Configure a message in the case of already started publisher.
SOPC_ReturnStatus SOPC_RT_Publisher_ConfigureMessage(SOPC_RT_Publisher* pPub,    // RT Publisher object
                                                     uint32_t messageIdentifier, // Message identifier
                                                     uint32_t period,            // Period in heart beats
                                                     uint32_t offset,            // offset in beat hearts
                                                     void* pContext,             // User context
                                                     ptrCallbackStart cbStart,   // Start callback
                                                     ptrCallbackSend cbSend,     // Send callback
                                                     ptrCallbackStart cbStop,    // Stop callback)
                                                     SOPC_RT_Publisher_MessageStatus initialStatus) // initial status
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    if (NULL == pPub)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    ePublisherSyncStatus currentStatus = SOPC_RT_Publisher_IncrementInUseStatus(pPub);

    if (currentStatus > E_PUBLISHER_SYNC_STATUS_INITIALIZED)
    {
        result = SOPC_InterruptTimer_Instance_Init(pPub->pInterruptTimer,                         //
                                                   messageIdentifier,                             //
                                                   period,                                        //
                                                   offset,                                        //
                                                   pContext,                                      //
                                                   cbStart,                                       //
                                                   cbSend,                                        //
                                                   cbStop,                                        //
                                                   (SOPC_IrqTimer_InstanceStatus) initialStatus); //

        SOPC_RT_Publisher_DecrementInUseStatus(pPub);
    }
    else
    {
        result = SOPC_STATUS_INVALID_STATE;
    }

    return result;
}

// Start message publishing for a given message identifier. RT Publisher should be started.
SOPC_ReturnStatus SOPC_RT_Publisher_StartMessagePublishing(SOPC_RT_Publisher* pPub, // RT Publisher object
                                                           uint32_t msgIdentifier)  // Message identifier
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    if (NULL == pPub)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    ePublisherSyncStatus currentStatus = SOPC_RT_Publisher_IncrementInUseStatus(pPub);

    if (currentStatus > E_PUBLISHER_SYNC_STATUS_INITIALIZED)
    {
        result = SOPC_InterruptTimer_Instance_Start(pPub->pInterruptTimer, msgIdentifier);

        SOPC_RT_Publisher_DecrementInUseStatus(pPub);
    }
    else
    {
        result = SOPC_STATUS_INVALID_STATE;
    }

    return result;
}

// Stop message publishing for a given message identifier. RT Publisher should be started
SOPC_ReturnStatus SOPC_RT_Publisher_StopMessagePublishing(SOPC_RT_Publisher* pPub, // RT Publisher object
                                                          uint32_t msgIdentifier)  // Message identifier
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    if (NULL == pPub)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    ePublisherSyncStatus currentStatus = SOPC_RT_Publisher_IncrementInUseStatus(pPub);

    if (currentStatus > E_PUBLISHER_SYNC_STATUS_INITIALIZED)
    {
        result = SOPC_InterruptTimer_Instance_Stop(pPub->pInterruptTimer, //
                                                   msgIdentifier);        //

        SOPC_RT_Publisher_DecrementInUseStatus(pPub);
    }
    else
    {
        result = SOPC_STATUS_INVALID_STATE;
    }

    return result;
}

// Get buffer handle to write directly to DBO.
// Returns : SOPC_STATUS_OK : data pointer and maximum size are updated
// with pointer where write and max size allowed
SOPC_ReturnStatus SOPC_RT_Publisher_GetBuffer(SOPC_RT_Publisher* pPub, // RT Publisher object
                                              uint32_t msgIdentifier,  // message identifier
                                              SOPC_Buffer* pBuffer)    // SOPC Buffer with unallocated data pointer
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    if (NULL == pPub || NULL == pBuffer || pBuffer->data != NULL || msgIdentifier >= pPub->nbMessages)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    ePublisherSyncStatus currentStatus = SOPC_RT_Publisher_IncrementInUseStatus(pPub);

    if (currentStatus > E_PUBLISHER_SYNC_STATUS_INITIALIZED)
    {
        result = SOPC_InterruptTimer_Instance_DataHandle_Initialize(pPub->ppTmrDataHandle[msgIdentifier]);

        if (SOPC_STATUS_OK == result)
        {
            result = SOPC_InterruptTimer_Instance_DataHandle_GetBufferInfo(pPub->ppTmrDataHandle[msgIdentifier], //
                                                                           &pBuffer->maximum_size,               //
                                                                           &pBuffer->length,                     //
                                                                           &pBuffer->data);                      //

            if (SOPC_STATUS_OK == result)
            {
                pBuffer->initial_size = pBuffer->maximum_size;
                pBuffer->current_size = pBuffer->maximum_size;
                pBuffer->position = 0;
            }
        }

        SOPC_RT_Publisher_DecrementInUseStatus(pPub);
    }
    else
    {
        result = SOPC_STATUS_INVALID_STATE;
    }

    if (SOPC_STATUS_OK != result)
    {
        pBuffer->data = NULL;
        pBuffer->current_size = 0;
        pBuffer->maximum_size = 0;
        pBuffer->initial_size = 0;
        pBuffer->position = 0;
    }

    return result;
}

// Commit data to publish and release buffer
// Returns : SOPC_STATUS_OK if data well commit.
// SOPC_Buffer contains current size to take into account.
SOPC_ReturnStatus SOPC_RT_Publisher_ReleaseBuffer(SOPC_RT_Publisher* pPub, // RT Publisher object
                                                  uint32_t msgIdentifier,  // Message identifier
                                                  SOPC_Buffer* pBuffer)    // Buffer with current size to commit
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    if (NULL == pPub || NULL == pBuffer || msgIdentifier >= pPub->nbMessages)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    ePublisherSyncStatus currentStatus = SOPC_RT_Publisher_IncrementInUseStatus(pPub);

    if (currentStatus > E_PUBLISHER_SYNC_STATUS_INITIALIZED)
    {
        result = SOPC_InterruptTimer_Instance_DataHandle_SetNewSize(pPub->ppTmrDataHandle[msgIdentifier], //
                                                                    pBuffer->length);                     //

        bool bCancel = false;
        if (SOPC_STATUS_OK != result)
        {
            bCancel = true;
        }

        SOPC_InterruptTimer_Instance_DataHandle_Finalize(pPub->ppTmrDataHandle[msgIdentifier], bCancel);

        SOPC_RT_Publisher_DecrementInUseStatus(pPub);
    }
    else
    {
        result = SOPC_STATUS_INVALID_STATE;
    }

    return result;
}

// Set message publishing value for a given message identifier.
// After start, if message value is not set, if valid period and timer started, elapsed callback will
// called with size = 0. Event if size = 0, value point always on valid space, but bytes are not significant !!!
SOPC_ReturnStatus SOPC_RT_Publisher_SetMessageValue(SOPC_RT_Publisher* pPub, // RT Publisher object
                                                    uint32_t msgIdentifier,  // Message identifier
                                                    uint8_t* value,          // Value to publish
                                                    uint32_t size)           // Size of value
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    if (NULL == pPub)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    ePublisherSyncStatus currentStatus = SOPC_RT_Publisher_IncrementInUseStatus(pPub);

    if (currentStatus > E_PUBLISHER_SYNC_STATUS_INITIALIZED)
    {
        result = SOPC_InterruptTimer_Instance_SetData(pPub->pInterruptTimer, msgIdentifier, value, size);

        SOPC_RT_Publisher_DecrementInUseStatus(pPub);
    }
    else
    {
        result = SOPC_STATUS_INVALID_STATE;
    }

    return result;
}

// Beat heart, this function invoke update function of interrupt timer. It should be called from IRQ or high priority
// task. This function invoke user callback. So, this function should not take more time that period set and don't be
// blocking.
SOPC_ReturnStatus SOPC_RT_Publisher_HeartBeat(SOPC_RT_Publisher* pPub, // RT Publisher object
                                              uint32_t tickValue)      // Cumulative tick value
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    if (NULL == pPub)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    ePublisherSyncStatus currentStatus = SOPC_RT_Publisher_IncrementInUseStatus(pPub);

    if (currentStatus > E_PUBLISHER_SYNC_STATUS_INITIALIZED)
    {
        result = SOPC_InterruptTimer_Update(pPub->pInterruptTimer, tickValue);

        SOPC_RT_Publisher_DecrementInUseStatus(pPub);
    }
    else
    {
        result = SOPC_STATUS_INVALID_STATE;
    }

    return result;
}

// Private functions definitions

// Resetting RT Publisher. Protected by RT Publisher Sync Status
static void _SOPC_RT_Publisher_DeInitialize(SOPC_RT_Publisher* pPub)
{
    SOPC_InterruptTimer_DeInitialize(pPub->pInterruptTimer);
    SOPC_InterruptTimer_Destroy(&pPub->pInterruptTimer);
    if (NULL != pPub->ppTmrDataHandle)
    {
        for (uint32_t i = 0; i < pPub->nbMessages; i++)
        {
            SOPC_InterruptTimer_DestroyDataContainer(&pPub->ppTmrDataHandle[i]);
        }
        SOPC_Free(pPub->ppTmrDataHandle);
        pPub->ppTmrDataHandle = NULL;
    }
}

// Initialize RT Publisher. Protected by RT Publisher Sync Status
static SOPC_ReturnStatus _SOPC_RT_Publisher_Initialize(
    SOPC_RT_Publisher* pPub,                // RT Publisher object
    SOPC_RT_Publisher_Initializer* pConfig) // RT Publisher configuration
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    // Timer workspace creation
    pPub->pInterruptTimer = SOPC_InterruptTimer_Create();

    if (pPub->pInterruptTimer == NULL)
    {
        result = SOPC_STATUS_NOK;
    }

    // Timer workspace initialization
    if (SOPC_STATUS_OK == result)
    {
        result = SOPC_InterruptTimer_Initialize(pPub->pInterruptTimer,      //
                                                pConfig->nbMessages,        //
                                                pConfig->maxSizeOfMessage); //
    }

    if (SOPC_STATUS_OK == result)
    {
        pPub->ppTmrDataHandle = SOPC_Calloc(1, pConfig->nbMessages * sizeof(SOPC_InterruptTimer_DataHandle*));
        if (pPub->ppTmrDataHandle == NULL)
        {
            result = SOPC_STATUS_OUT_OF_MEMORY;
        }
        else
        {
            pPub->nbMessages = pConfig->nbMessages;
            pPub->maxSizeOfMessage = pConfig->maxSizeOfMessage;
        }
    }

    SOPC_RT_Publisher_Message_Config* pMsg = pConfig->pMessageConfig;
    // Timer instance configuration with each message configuration
    for (uint32_t msgId = 0; msgId < pConfig->nbMessages && SOPC_STATUS_OK == result; msgId++)
    {
        // Reset timer
        result = SOPC_InterruptTimer_Instance_DeInit(pPub->pInterruptTimer, msgId);

        // Set timer configuration from initializer messages configuration list.
        if (SOPC_STATUS_OK == result)
        {
            if (pMsg != NULL && SOPC_STATUS_OK == result)
            {
                result = SOPC_InterruptTimer_Instance_Init(pPub->pInterruptTimer, //
                                                           msgId,                 //
                                                           pMsg->period,          //
                                                           pMsg->offset,
                                                           pMsg->pUserContext,                                  //
                                                           pMsg->cbMessagePublishingStarted,                    //
                                                           pMsg->cbMessageSend,                                 //
                                                           pMsg->cbMessagePublishingStopped,                    //
                                                           (SOPC_IrqTimer_InstanceStatus) pMsg->initialStatus); //

                pPub->ppTmrDataHandle[msgId] =
                    SOPC_InterruptTimer_Instance_DataHandle_Create(pPub->pInterruptTimer, msgId);

                if (NULL == pPub->ppTmrDataHandle[msgId])
                {
                    result = SOPC_STATUS_OUT_OF_MEMORY;
                }

                pMsg = pMsg->next;
            }
        }
    }

    return result;
}

// Increment utilization of module counter. Try to pass INITIALIZED STATUS or beyond to this status + 1.
static inline ePublisherSyncStatus SOPC_RT_Publisher_IncrementInUseStatus(SOPC_RT_Publisher* pRtPub)
{
    {
        bool result = false;

        ePublisherSyncStatus currentValue = 0;

        ePublisherSyncStatus newValue = 0;

        do
        {
            // Load current counter and atomic increment it if possible
            __atomic_load(&pRtPub->status, &currentValue, __ATOMIC_SEQ_CST);

            if (currentValue >= E_PUBLISHER_SYNC_STATUS_INITIALIZED)
            {
                newValue = currentValue + 1;
            }
            else
            {
                newValue = currentValue;
            }
            result = __atomic_compare_exchange(&pRtPub->status,   //
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
static inline ePublisherSyncStatus SOPC_RT_Publisher_DecrementInUseStatus(SOPC_RT_Publisher* pRtPub)
{
    {
        bool result = false;
        ePublisherSyncStatus currentValue = 0;
        ePublisherSyncStatus newValue = 0;

        do
        {
            // Load current counter and atomic increment it if possible
            __atomic_load(&pRtPub->status, &currentValue, __ATOMIC_SEQ_CST);
            if (currentValue > E_PUBLISHER_SYNC_STATUS_INITIALIZED)
            {
                newValue = currentValue - 1;
            }
            else
            {
                newValue = currentValue;
            }
            result = __atomic_compare_exchange(&pRtPub->status,   //
                                               &currentValue,     //
                                               &newValue,         //
                                               false,             //
                                               __ATOMIC_SEQ_CST,  //
                                               __ATOMIC_SEQ_CST); //
        } while (!result);

        return newValue;
    }
}
