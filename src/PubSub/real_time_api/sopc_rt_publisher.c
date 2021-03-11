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

/// @file sopc_rt_publisher.c

#include "sopc_rt_publisher.h"

/// @brief Private status of RT Publisher
typedef enum E_PUBLISHER_SYNC_STATUS
{
    E_PUBLISHER_SYNC_STATUS_NOT_INITIALIZED, ///< RT Publisher not initialized
    E_PUBLISHER_SYNC_STATUS_INITIALIZING,    ///< RT Publisher initializing
    E_PUBLISHER_SYNC_STATUS_DEINITIALIZING,  ///< RT Publisher deinitializing
    E_PUBLISHER_SYNC_STATUS_INITIALIZED,     ///< RT Publisher well initialized
    E_PUBLUSHER_SYNC_STATUS_LOCKED, ///< RT Publisher with this status and beyond can't be de initialized (in use)
    E_PUBLISHER_SYNC_STATUS_SIZE = INT32_MAX
} ePublisherSyncStatus;

/// @brief Private message publication configuration
typedef struct SOPC_RT_Publisher_Message_Config
{
    uint32_t period;                               ///< Period in beat heart
    uint32_t offset;                               ///< Offset in beat heart
    void* pUserContext;                            ///< User context.
    ptrCallbackStart cbMessagePublishingStarted;   ///< Message publishing status change from STOP to START
    ptrCallbackSend cbMessageSend;                 ///< Message data should be sent
    ptrCallbackStop cbMessagePublishingStopped;    ///< Message publishing status change from START to STOP
    SOPC_RT_Publisher_MessageStatus initialStatus; ///< Message initial status after SOPC_RT_Publisher_Create
    struct SOPC_RT_Publisher_Message_Config* next; ///< Next message configuration
} SOPC_RT_Publisher_Message_Config;

/// @brief RT Publisher Initializer
struct SOPC_RT_Publisher_Initializer
{
    uint32_t nbMessages;                              ///< Number of messages handled
    uint32_t maxSizeOfMessage;                        ///< Max size of a message
    SOPC_RT_Publisher_Message_Config* pMessageConfig; ///< Message configuration list
};

/// @brief RT Publisher object
struct SOPC_RT_Publisher
{
    ePublisherSyncStatus status;          ///< RT publisher status
    uint32_t nbMessages;                  ///< Number of messages
    uint32_t maxSizeOfMessage;            ///< Max significant bytes, common to all messages
    SOPC_InterruptTimer* pInterruptTimer; ///< Interrupt Timer workspace
    SOPC_InterruptTimer_DataHandle**
        ppTmrDataHandle; ///< Interrupt timer data handle used to publish data without intermediate buffer
};

// Private functions declarations

/// @brief Version of SOPC_RT_Publisher_DeInitialize without concurrent protection
/// @param [in] pPub RT Publisher object
static void _SOPC_RT_Publisher_DeInitialize(SOPC_RT_Publisher* pPub);

/// @brief Version of SOPC_RT_Publisher_Initialize without concurrent protection
/// @param [in] pPub RT Publisher object
/// @param [in] pConfig RT Initializer object, well initialized
/// @return SOPC_STATUS_OK if well initialized. Else SOPC_STATUS_INVALID_STATE in case of bad status (well initialized
/// or initializing)
static SOPC_ReturnStatus _SOPC_RT_Publisher_Initialize(SOPC_RT_Publisher* pPub, SOPC_RT_Publisher_Initializer* pConfig);

/// @brief Increment status of RT Publisher object if well initialized. Status indicates the current number of API using
/// level, tested to authorize deinitialization.
/// @param [in] pRtPub RT Publisher object
/// @return RT Publisher sync status, INITIALIZED and beyond.
static inline ePublisherSyncStatus SOPC_RT_Publisher_IncrementInUseStatus(SOPC_RT_Publisher* pRtPub);

/// @brief Decrement status of RT Publisher object if beyond initialized status. Status indicates the current number of
/// API using level, tested to authorize deinitialization.
/// @param [in] pRtPub RT Publisher object
/// @return RT Publisher sync status, INITIALIZED and beyond.
static inline ePublisherSyncStatus SOPC_RT_Publisher_DecrementInUseStatus(SOPC_RT_Publisher* pRtPub);

SOPC_RT_Publisher_Initializer* SOPC_RT_Publisher_Initializer_Create(uint32_t maxSizeOfMessage) //
{
    SOPC_RT_Publisher_Initializer* pInitializer = SOPC_Calloc(1, sizeof(SOPC_RT_Publisher_Initializer));
    if (NULL == pInitializer)
    {
        return NULL;
    }

    if (maxSizeOfMessage < 1)
    {
        SOPC_Free(pInitializer);
        return NULL;
    }

    pInitializer->nbMessages = 0;
    pInitializer->maxSizeOfMessage = maxSizeOfMessage;
    pInitializer->pMessageConfig = NULL;

    return pInitializer;
}

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

SOPC_ReturnStatus SOPC_RT_Publisher_Initializer_AddMessage(SOPC_RT_Publisher_Initializer* pConfig,        //
                                                           uint32_t period,                               //
                                                           uint32_t offset,                               //
                                                           void* pContext,                                //
                                                           ptrCallbackStart cbStart,                      //
                                                           ptrCallbackSend cbSend,                        //
                                                           ptrCallbackStop cbStop,                        //
                                                           SOPC_RT_Publisher_MessageStatus initialStatus, //
                                                           uint32_t* pOutMsgId)                           //
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    if (pConfig == NULL || period < 1 || cbSend == NULL || NULL == pOutMsgId)
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

SOPC_ReturnStatus SOPC_RT_Publisher_Initialize(SOPC_RT_Publisher* pPub,                     //
                                               SOPC_RT_Publisher_Initializer* pInitializer) //
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

SOPC_RT_Publisher* SOPC_RT_Publisher_Create(void)
{
    return (SOPC_RT_Publisher*) SOPC_Calloc(1, sizeof(struct SOPC_RT_Publisher));
}

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

SOPC_ReturnStatus SOPC_RT_Publisher_GetMessagePubStatus(SOPC_RT_Publisher* pPub,                  //
                                                        uint32_t msgIdentifier,                   //
                                                        SOPC_RT_Publisher_MessageStatus* pStatus) //
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

SOPC_ReturnStatus SOPC_RT_Publisher_GetBuffer(SOPC_RT_Publisher* pPub, uint32_t msgIdentifier, SOPC_Buffer* pBuffer)
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
            size_t size = 0;
            result = SOPC_InterruptTimer_Instance_DataHandle_GetBufferInfo(
                pPub->ppTmrDataHandle[msgIdentifier], &pBuffer->maximum_size, &size, &pBuffer->data);
            if (size < UINT32_MAX)
            {
                pBuffer->length = (uint32_t) size;
            }
            else
            {
                result = SOPC_STATUS_OUT_OF_MEMORY;
            }

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

SOPC_ReturnStatus SOPC_RT_Publisher_ReleaseBuffer(SOPC_RT_Publisher* pPub, //
                                                  uint32_t msgIdentifier,  //
                                                  SOPC_Buffer* pBuffer,    //
                                                  bool bCancel)            //
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

        bool bLocalCancel = bCancel;
        if (SOPC_STATUS_OK != result)
        {
            bLocalCancel = true;
        }

        SOPC_InterruptTimer_Instance_DataHandle_Finalize(pPub->ppTmrDataHandle[msgIdentifier], bLocalCancel);

        SOPC_RT_Publisher_DecrementInUseStatus(pPub);
    }
    else
    {
        result = SOPC_STATUS_INVALID_STATE;
    }

    pBuffer->data = NULL;
    pBuffer->current_size = 0;
    pBuffer->maximum_size = 0;
    pBuffer->initial_size = 0;
    pBuffer->position = 0;

    return result;
}

SOPC_ReturnStatus SOPC_RT_Publisher_SetMessageValue(SOPC_RT_Publisher* pPub, //
                                                    uint32_t msgIdentifier,  //
                                                    uint8_t* value,          //
                                                    uint32_t size)           //
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

SOPC_ReturnStatus SOPC_RT_Publisher_HeartBeat(SOPC_RT_Publisher* pPub, //
                                              uint32_t tickValue)      //
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

static SOPC_ReturnStatus _SOPC_RT_Publisher_Initialize(SOPC_RT_Publisher* pPub,                //
                                                       SOPC_RT_Publisher_Initializer* pConfig) //
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
