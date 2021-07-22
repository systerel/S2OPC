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

/*============================================================================
 * DESCRIPTION
 *===========================================================================*/

/** \file
 * Creates an asynchronous queue (pzQueue) which are dequeued in dedicated thread (gThread).
 * When a Request or Response message is received, the event is enqueued. When processed by the task,
 * the message is encoded in a raw buffer which is then sent to SAFE using interface (UAM_NS2S_xxx)
 * UAM_NS_CheckSpduReception shall be called periodically to check the reception of a SPDU from SAFE.
 */

/*============================================================================
 * INCLUDES
 *===========================================================================*/

#include "uam.h"
#include "uam_ns2s_itf.h"
#include "uam_spduEncoders.h"
#include "uam_cache.h"
#include "uas.h"

#include "sopc_builtintypes.h"
#include "sopc_common.h"
#include "sopc_dict.h"
#include "sopc_log_manager.h"
#include "sopc_mem_alloc.h"
#include "sopc_threads.h"
#include "sopc_pub_scheduler.h"
#include "sopc_sub_scheduler.h"
#include "sopc_pub_source_variable.h"
#include "sopc_pubsub_conf.h"
#include "sopc_sub_target_variable.h"
#include "sopc_xml_loader.h"

#include "sopc_async_queue.h"

#include <assert.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <signal.h>
#include "uam_ns_impl.h"

/*============================================================================
 * LOCAL TYPES
 *===========================================================================*/
typedef enum
{
    qaWakeEvent,
    qaSpduRequestToSafe,
    qaSpduResponseToSafe,
    qaSpduPollSafe,
} QueueAction_type;

typedef struct
{
    /** True for a message from NonSafe to Safe, false for a message from Safe to NonSafe */
    QueueAction_type eEvent;
    UAM_SessionHandle dwHandle;
} QueueElement_type;

typedef struct
{
    SOPC_PubSubConfiguration* pConfig;
    SOPC_PubSourceVariableConfig* sourceConfig;
    SOPC_SubTargetVariableConfig* targetConfig;
} UAM_NS_interactive_Context;

/*============================================================================
 * LOCAL VARIABLES
 *===========================================================================*/

static UAM_NS_interactive_Context g_interactive_Context;

/**
 * A dictionary object { UAM_SessionHandle : UAM_NS_Configuration_type* }
 */
static SOPC_Dict* gSessions = NULL;

/**
 * A dictionary object { UAS_UInt32 : UAM_SessionHandle }
 */
static SOPC_Dict* gNodeIdsDict = NULL;

/**
 * A Queue containing some QueueElement_type*
 */
static SOPC_AsyncQueue* pzQueue;

/** The processing message thread */
static Thread gThread;

static volatile sig_atomic_t stopSignal = 0;

static const UAS_UInt32 NoHandle = 0xFFFFFFFEu;

#define MAX_SOCKET_READ_SIZE (2048u)

/*============================================================================
 * DECLARATION OF INTERNAL SERVICES
 *===========================================================================*/
static UAM_NS_Configuration_type* Session_Get(const UAM_SessionHandle key);

static void EnqueueEvent(UAM_SessionHandle dwHandle, const QueueAction_type event);
static void EnqueueNoEvent(void);

static void cache_Notify_CB(const SOPC_NodeId* const pNid, const SOPC_DataValue* const pDv);


/*============================================================================
 * IMPLEMENTATION OF INTERNAL SERVICES
 *===========================================================================*/
/*===========================================================================*/
static void serialize_UInt32(const UAS_UInt32 uVal, UAS_UInt8* pData, const size_t sLen, size_t* pPos)
{
    UAS_UInt32* puTmp = (UAS_UInt32*) (&pData[*pPos]);
    *pPos += sizeof(UAS_UInt32);

    if (*pPos <= sLen)
    {
        (*puTmp) = ntohl(uVal);
    }
}

/*===========================================================================*/
static void serialize_UInt8(const UAS_UInt8 uVal, UAS_UInt8* pData, const size_t sLen, size_t* pPos)
{
    UAS_UInt8* puTmp = (UAS_UInt8*) (&pData[*pPos]);
    *pPos += sizeof(UAS_UInt8);

    if (*pPos <= sLen)
    {
        (*puTmp) = uVal;
    }
}

/*===========================================================================*/
static void serialize_String(const UAS_UInt8* pzVal,
                             const size_t sValLen,
                             UAS_UInt8* pData,
                             const size_t sLen,
                             size_t* pPos)
{
    if (sValLen + (*pPos) <= sLen)
    {
        memcpy(&pData[*pPos], pzVal, sValLen);
        *pPos += sValLen;
    }
}

/*===========================================================================*/
static UAS_UInt32 deserialize_UInt32(const UAS_UInt8* pData, const size_t sLen, size_t* pPos)
{
    const UAS_UInt32* puTmp = (const UAS_UInt32*) (&pData[*pPos]);
    *pPos += sizeof(UAS_UInt32);

    if (*pPos > sLen)
    {
        return 0;
    }

    return ntohl(*puTmp);
}

/*===========================================================================*/
static UAS_UInt8 deserialize_UInt8(const UAS_UInt8* pData, const size_t sLen, size_t* pPos)
{
    const UAS_UInt8* puTmp = (const UAS_UInt8*) (&pData[*pPos]);
    *pPos += sizeof(UAS_UInt8);

    if (*pPos > sLen)
    {
        return 0;
    }

    return *puTmp;
}

/*===========================================================================*/
static void deserialize_String(const UAS_UInt8* pData,
                               const size_t sLen,
                               size_t* pPos,
                               SOPC_ByteString* pzDest,
                               const size_t sValLen)
{
    if (sValLen + *pPos <= sLen)
    {
        SOPC_ByteString_CopyFromBytes(pzDest, pData + *pPos, (int32_t) sValLen);
    }
    else
    {
        //        printf("deserialize_String fail : sValLen= %u, pos=%u, sLen=%u\n",
        //                (unsigned) sValLen, (unsigned) *pPos, (unsigned) sLen); // TODO clean
    }
    *pPos += sValLen;
}

/*===========================================================================*/
/**
 * \param ppBytes Must be freed by caller after use
 */
static bool EncodeSpduRequest(const UAS_RequestSpdu_type* pzSpdu, SOPC_Byte** ppBytes, size_t* sLen)
{
    static const size_t expLen = 9;
    bool bResult = false;

    *sLen = 0;
    if (pzSpdu != NULL && ppBytes != NULL && sLen != NULL)
    {
        *ppBytes = (SOPC_Byte*) SOPC_Malloc(expLen);
        assert(NULL != *ppBytes);
        serialize_UInt32(pzSpdu->dwSafetyConsumerId, *ppBytes, expLen, sLen);
        serialize_UInt32(pzSpdu->dwMonitoringNumber, *ppBytes, expLen, sLen);
        serialize_UInt8(pzSpdu->byFlags, *ppBytes, expLen, sLen);
        if ((*sLen) == expLen)
        {
            bResult = true;
            LOG_Trace(LOG_DEBUG, "UAM_NS:EncodeSpduRequest Result is %u bytes long", (unsigned) (*sLen));
        }
        else
        {
            LOG_Trace(LOG_WARN, "UAM_NS:Failed to EncodeSpduRequest for sending to PubSub layer (len = %u, exp = %u)",
                      (unsigned) (*sLen), (unsigned) expLen);
        }
    }

    return bResult;
}

/*===========================================================================*/
/**
 * \param ppBytes Must be freed by caller after use
 */
static bool EncodeSpduResponse(const UAS_ResponseSpdu_type* pzSpdu,
                               SOPC_Byte** ppBytes,
                               size_t* sLen,
                               size_t uSafeSize,
                               size_t uNonSafeSize)
{
    bool bResult = false;

    sLen = 0;
    if (!(pzSpdu == NULL || ppBytes == NULL || sLen == NULL))
    {
        const size_t expLen = 25 + uSafeSize + uNonSafeSize;
        *ppBytes = (SOPC_Byte*) SOPC_Malloc(expLen);
        assert(NULL != *ppBytes);
        // SAFE data
        serialize_String(pzSpdu->pbySerializedSafetyData, uSafeSize, *ppBytes, expLen, sLen);
        serialize_UInt8(pzSpdu->byFlags, *ppBytes, expLen, sLen);
        serialize_UInt32(pzSpdu->zSpduId.dwPart1, *ppBytes, expLen, sLen);
        serialize_UInt32(pzSpdu->zSpduId.dwPart2, *ppBytes, expLen, sLen);
        serialize_UInt32(pzSpdu->zSpduId.dwPart3, *ppBytes, expLen, sLen);
        serialize_UInt32(pzSpdu->dwSafetyConsumerId, *ppBytes, expLen, sLen);
        serialize_UInt32(pzSpdu->dwMonitoringNumber, *ppBytes, expLen, sLen);
        serialize_UInt32(pzSpdu->dwCrc, *ppBytes, expLen, sLen);
        serialize_String(pzSpdu->pbySerializedNonSafetyData, uNonSafeSize, *ppBytes, expLen, sLen);
        if ((*sLen) == expLen)
        {
            bResult = true;
            LOG_Trace(LOG_DEBUG, "UAM_NS:EncodeSpduResponse Result is %u bytes long", (unsigned) (*sLen));
        }
        else
        {
            LOG_Trace(LOG_WARN, "UAM_NS:Failed to EncodeSpduResponse for sending to PubSub layer (len = %u, exp = %u)",
                      (unsigned) (*sLen), (unsigned) expLen);
        }
    }

    return bResult;
}

/*===========================================================================*/
static bool DecodeSpduResponse(const UAM_SpduResponseHandle dwNumericId, const SOPC_Byte* pBytes, size_t sLen)
{
    if (pBytes == NULL)
    {
        return false;
    }

    size_t pos = 0;
    bool bResult = false;
    UAS_ResponseSpdu_type zSpdu;
    size_t uSafeSize = 0;
    size_t uNonSafeSize = 0;

    SOPC_ByteString bsSafeData;
    SOPC_ByteString bsNonSafeData;

    UAM_SpduEncoder_GetResponseSizes(dwNumericId, &uSafeSize, &uNonSafeSize);
    SOPC_ByteString_Initialize(&bsSafeData);
    SOPC_ByteString_Initialize(&bsNonSafeData);
    const size_t expLen = 25 + uSafeSize + uNonSafeSize;

    // de-serialize Safe data
    deserialize_String(pBytes, sLen, &pos, &bsSafeData, uSafeSize);
    zSpdu.pbySerializedSafetyData = bsSafeData.Data;
    // de-serialize byFlags
    zSpdu.byFlags = deserialize_UInt8(pBytes, sLen, &pos);
    // de-serialize zSpduId
    zSpdu.zSpduId.dwPart1 = deserialize_UInt32(pBytes, sLen, &pos);
    zSpdu.zSpduId.dwPart2 = deserialize_UInt32(pBytes, sLen, &pos);
    zSpdu.zSpduId.dwPart3 = deserialize_UInt32(pBytes, sLen, &pos);
    // de-serialize dwSafetyConsumerId
    zSpdu.dwSafetyConsumerId = deserialize_UInt32(pBytes, sLen, &pos);
    // de-serialize dwMonitoringNumber
    zSpdu.dwMonitoringNumber = deserialize_UInt32(pBytes, sLen, &pos);
    // de-serialize dwCrc
    zSpdu.dwCrc = deserialize_UInt32(pBytes, sLen, &pos);
    // de-serialize NonSafe data
    deserialize_String(pBytes, sLen, &pos, &bsNonSafeData, uNonSafeSize);
    zSpdu.pbySerializedNonSafetyData = bsNonSafeData.Data;

    if (pos == expLen && pos == sLen)
    {
        LOG_Trace(LOG_DEBUG, "Encoded message to Safe(%08X). LEN= %u, Safe len = %d, NonSafe len = %d",
                  (unsigned) dwNumericId, (unsigned) sLen, (int) bsSafeData.Length, (int) bsNonSafeData.Length);
        // Simply put the SPDU in encoder cache.
        UAM_SpduEncoder_SetResponse(dwNumericId, &zSpdu);
        bResult = true;
    }
    else
    {
        LOG_Trace(LOG_WARN,
                  "Failed to decode SPDU RESPONSE %08X for sending to PubSub layer (len = %u, pos= %u, exp = %u)",
                  dwNumericId, (unsigned) pos, (unsigned) sLen, (unsigned) expLen);
    }

    SOPC_ByteString_Clear(&bsSafeData);
    SOPC_ByteString_Clear(&bsNonSafeData);

    return bResult;
}

/*===========================================================================*/
static bool DecodeSpduRequest(UAM_SpduRequestHandle dwNumericId, const SOPC_Byte* pBytes, size_t sLen)
{
    size_t pos = 0;
    bool bResult = false;
    if (pBytes == NULL)
    {
        return bResult;
    }

    UAS_RequestSpdu_type zSpdu;
    zSpdu.dwSafetyConsumerId = deserialize_UInt32(pBytes, sLen, &pos);
    zSpdu.dwMonitoringNumber = deserialize_UInt32(pBytes, sLen, &pos);
    zSpdu.byFlags = deserialize_UInt8(pBytes, sLen, &pos);

    if (pos == sLen)
    {
        UAM_SpduEncoder_SetRequest(dwNumericId, &zSpdu);
        bResult = true;
    }
    else
    {
        //        printf ("Failed to DecodeSpduRequest to PubSub layer (len = %u, exp = %u)\n",
        //                (unsigned)pos, (unsigned)sLen); // TODO clean
    }
    return bResult;
}

/*===========================================================================*/
static void DoPollSafeMessages(const UAM_NS_Configuration_type* pzSession)
{
    assert(pzSession != NULL);
    void* pzBuffer;
    size_t sReadLen = 0;

    pzBuffer = SOPC_Malloc(MAX_SOCKET_READ_SIZE);
    assert(pzBuffer != NULL);

    UAM_NS2S_ReceiveSpduImpl(pzSession->dwHandle, pzBuffer, MAX_SOCKET_READ_SIZE, &sReadLen);

    if (sReadLen > 0)
    {
        LOG_Trace(LOG_DEBUG, "DoPollSafeMessages received %u bytes", (unsigned) sReadLen);
        if (pzSession->bIsProvider)
        {
            // For provider the SPDU from SAFE is a response
            DecodeSpduResponse(pzSession->uUserResponseId, pzBuffer, sReadLen);
        }
        else
        {
            // For consumer the SPDU from SAFE is a request
            DecodeSpduRequest(pzSession->uUserRequestId, pzBuffer, sReadLen);
        }
    }

    SOPC_Free(pzBuffer);
}

/*===========================================================================*/
static void EnqueueEvent(UAM_SessionHandle dwHandle, const QueueAction_type event)
{
    QueueElement_type* pEvent = SOPC_Malloc(sizeof(*pEvent));
    assert(pEvent != NULL);
    pEvent->dwHandle = dwHandle;
    pEvent->eEvent = event;
//
//    LOG_Trace(LOG_DEBUG, "EnqueueEvent HDL=%u Evt=%d", (unsigned) dwHandle, event);

    SOPC_AsyncQueue_BlockingEnqueue(pzQueue, pEvent);
}
/*===========================================================================*/
static void EnqueueNoEvent(void)
{
    EnqueueEvent(NoHandle, qaWakeEvent);
}

/*===========================================================================*/
static void* Thread_Impl(void* data)
{
    assert(data == NULL);
    size_t sLen;
    bool bResult = false;
    size_t uSafeSize;
    size_t uNonSafeSize;

    while (stopSignal == 0)
    {
        SOPC_Byte* pBytesToSafe = NULL;
        QueueElement_type* pEvent = NULL;
        SOPC_AsyncQueue_BlockingDequeue(pzQueue, (void**) &pEvent);

        const UAM_NS_Configuration_type* pzSession = Session_Get(pEvent->dwHandle);

//        LOG_Trace(LOG_DEBUG, "DequeueEvent HDL=%u Evt=%d, pzSession= %p", (unsigned) pEvent->dwHandle, pEvent->eEvent,
//                  pzSession);
        if (pzSession != NULL && pEvent != NULL)
        {
            switch (pEvent->eEvent)
            {
            case qaSpduRequestToSafe:
            {
                // A message was received on PUBSUB side and has to be forwarded on SAFE
                // Convert the message for SAFE
                UAS_RequestSpdu_type zSpdu;
                UAM_SpduEncoder_GetRequest(pzSession->uUserRequestId, &zSpdu);

                bResult = EncodeSpduRequest(&zSpdu, &pBytesToSafe, &sLen);

                break;
            }
            case qaSpduResponseToSafe:
            {
                // A message was received on PUBSUB side and has to be forwarded on SAFE
                // Convert the message for SAFE
                UAS_ResponseSpdu_type zSpdu;
                uSafeSize = 0;
                uNonSafeSize = 0;
                UAM_SpduEncoder_GetResponse(pzSession->uUserResponseId, &zSpdu, &uSafeSize, &uNonSafeSize);

                bResult = EncodeSpduResponse(&zSpdu, &pBytesToSafe, &sLen, uSafeSize, uNonSafeSize);

                break;
            }
            case qaSpduPollSafe:
            {
                // Check if A message was received from SAFE and has to be forwarded on PUB SUB.
                DoPollSafeMessages(pzSession);
                break;
            }
            case qaWakeEvent:
            default:
                break;
            }

            if (bResult && sLen > 0 && pBytesToSafe != NULL)
            {
                // Send it to SAFE
                UAM_NS2S_SendSpduImpl(pEvent->dwHandle, pBytesToSafe, sLen);
            }
            if (pBytesToSafe != NULL)
            {
                SOPC_Free (pBytesToSafe);
            }
        }

        SOPC_Free(pEvent);
    }
    LOG_Trace(LOG_DEBUG, "UALM_NS:  Thread_Impl stopped signal=%d", stopSignal);
    return NULL;
}

/*===========================================================================*/
static uint64_t Intptr_KeyHash_Fct(const void* pKey)
{
    return (const uint64_t)(const UAS_INVERSE_PTR) pKey;
}

/*===========================================================================*/
static bool Intptr_KeyEqual_Fct(const void* a, const void* b)
{
    return a == b;
}

/*===========================================================================*/
static UAM_NS_Configuration_type* Session_Get(const UAM_SessionHandle key)
{
    if (gSessions == NULL)
    {
        return NULL;
    }
    return (UAM_NS_Configuration_type*) SOPC_Dict_Get(gSessions, (void*) (UAS_INVERSE_PTR) key, NULL);
}

/*===========================================================================*/
static UAM_NS_Configuration_type* NumricIdToSession(const UAS_UInt32 key)
{
    if (gNodeIdsDict == NULL)
    {
        return NULL;
    }
    return (UAM_NS_Configuration_type*) SOPC_Dict_Get(gNodeIdsDict, (void*) (UAS_INVERSE_PTR) key, NULL);
}

/*===========================================================================*/
static bool Session_Add(const UAM_NS_Configuration_type* const pzConfig)
{
    bool bResult = false;
    assert(NULL != pzConfig);

    UAM_NS_Configuration_type* pzPrevConfig = Session_Get(pzConfig->dwHandle);
    if (pzPrevConfig == NULL)
    {
        UAM_NS_Configuration_type* pzNewConfig = SOPC_Malloc(sizeof(*pzNewConfig));
        bResult = (NULL != pzNewConfig);
        if (bResult)
        {
            *pzNewConfig = *pzConfig;
            // Note : Values and Keys are freed
            bResult = SOPC_Dict_Insert(gSessions, (void*) (UAS_INVERSE_PTR) pzConfig->dwHandle, pzNewConfig);

            SOPC_Dict_Insert (gNodeIdsDict, (void*) (UAS_INVERSE_PTR) pzConfig->uUserRequestId, pzNewConfig);
            SOPC_Dict_Insert (gNodeIdsDict, (void*) (UAS_INVERSE_PTR) pzConfig->uUserResponseId, pzNewConfig);
        }
    }
    return bResult;
}

/*===========================================================================*/
static void PubSub_initialize_publisher(SOPC_ReturnStatus* pResult)
{
    assert (NULL != pResult);
    SOPC_PubSubConnection* firstConnection = NULL;
    uint32_t nbPub = 0;
    if (SOPC_STATUS_OK == *pResult)
    {
        assert(NULL != g_interactive_Context.pConfig);

        g_interactive_Context.sourceConfig = SOPC_PubSourceVariableConfig_Create(&UAM_Cache_GetSourceVariables);

        nbPub = SOPC_PubSubConfiguration_Nb_PubConnection(g_interactive_Context.pConfig);
        if (0 == nbPub)
        {
            LOG_Trace(LOG_DEBUG, "# Info: No Publisher configured");
        }
        else
        {
            bool res = SOPC_PubScheduler_Start(g_interactive_Context.pConfig, g_interactive_Context.sourceConfig, 0);
            if (res)
            {
                firstConnection = SOPC_PubSubConfiguration_Get_PubConnection_At(g_interactive_Context.pConfig, 0);
                LOG_Trace(LOG_DEBUG, "# Info: Publisher started on %s", SOPC_PubSubConnection_Get_Address(firstConnection));
            }
            else
            {
                LOG_Trace(LOG_DEBUG, "# Error while starting the Publisher, do you have administrator privileges?");
                *pResult = SOPC_STATUS_NOK;
            }
        }
    }
}

/*===========================================================================*/
static void PubSub_initialize_subscriber(SOPC_ReturnStatus* pResult)
{
    assert (NULL != pResult);
    SOPC_PubSubConnection* firstConnection = NULL;
    uint32_t nbSub = 0;
    if (SOPC_STATUS_OK == *pResult)
    {
        assert(NULL != g_interactive_Context.pConfig);

        g_interactive_Context.targetConfig = SOPC_SubTargetVariableConfig_Create(&UAM_Cache_SetTargetVariables);

        nbSub = SOPC_PubSubConfiguration_Nb_SubConnection(g_interactive_Context.pConfig);
        if (0 < nbSub)
        {
            bool res = SOPC_SubScheduler_Start(g_interactive_Context.pConfig, g_interactive_Context.targetConfig, NULL);
            if (res)
            {
                firstConnection = SOPC_PubSubConfiguration_Get_SubConnection_At(g_interactive_Context.pConfig, 0);
                LOG_Trace(LOG_DEBUG, "# Info: Subscriber started %s", SOPC_PubSubConnection_Get_Address(firstConnection));
            }
            else
            {
                LOG_Trace(LOG_DEBUG, "# Error while starting the Subscriber");
                *pResult = SOPC_STATUS_NOK;
            }
        }
    }
}

/*===========================================================================*/
static void cache_Notify_CB(const SOPC_NodeId* const pNid, const SOPC_DataValue* const pDv)
{
    // Reminder: this function is called when the Cache is updated (in this case: on Pub-Sub new reception)
    if (pNid == NULL || pDv == NULL)
    {
        return;
    }
    if (pNid->Namespace == UAM_NAMESPACE && pNid->IdentifierType == SOPC_IdentifierType_Numeric &&
        pDv->Value.ArrayType == SOPC_VariantArrayType_SingleValue &&
        pDv->Value.BuiltInTypeId == SOPC_ExtensionObject_Id && pDv->Value.Value.ExtObject->Length > 0)
    {
        // We received an extension object (not of array type) on the correct namespace.
        const UAM_NS_Configuration_type* pzSession = NumricIdToSession (pNid->Data.Numeric);

        if (pzSession != NULL)
        {
            // Retrieve and copy the object
            // Check whether this is a SPDU received data

            if (pNid->Data.Numeric == pzSession->uUserRequestId)
            {
                UAM_NS_RequestMessageReceived(pzSession->dwHandle);
            }
            if (pNid->Data.Numeric == pzSession->uUserResponseId)
            {
                UAM_NS_ResponseMessageReceived(pzSession->dwHandle);
            }
        }
    }
}

/*============================================================================
 * IMPLEMENTATION OF EXTERNAL SERVICES
 *===========================================================================*/

/*===========================================================================*/
SOPC_ReturnStatus UAM_NS_Initialize(void)
{
    SOPC_ReturnStatus sopcResult = SOPC_STATUS_INVALID_PARAMETERS;
    assert(gSessions == NULL);
    gSessions = SOPC_Dict_Create(NULL, Intptr_KeyHash_Fct, Intptr_KeyEqual_Fct, NULL, SOPC_Free);
    assert(gSessions != NULL);
    assert (gNodeIdsDict == NULL);
    gNodeIdsDict = SOPC_Dict_Create(NULL, Intptr_KeyHash_Fct, Intptr_KeyEqual_Fct, NULL, NULL);
    assert (gNodeIdsDict != NULL);

    sopcResult = SOPC_AsyncQueue_Init(&pzQueue, "UAM_NS_Events");
    if (SOPC_STATUS_OK == sopcResult)
    {
        sopcResult = SOPC_Thread_Create(&gThread, &Thread_Impl, NULL, "UAM_NS_Events task");
    }

    return sopcResult;
}

/*===========================================================================*/
SOPC_ReturnStatus UAM_NS_Impl_Initialize(const char * pubSubXmlConfigFile)
{
    SOPC_ReturnStatus sopcResult = SOPC_STATUS_OK;
    FILE* fd;

    g_interactive_Context.pConfig = NULL;

    if (SOPC_STATUS_OK == sopcResult)
    {
        fd = fopen(pubSubXmlConfigFile, "r");
        if (NULL == fd)
        {
            LOG_Trace(LOG_ERROR, "Cannot read configuration file %s", pubSubXmlConfigFile);
            sopcResult = SOPC_STATUS_INVALID_PARAMETERS;
        }
    }

    if (SOPC_STATUS_OK == sopcResult)
    {
        g_interactive_Context.pConfig = SOPC_PubSubConfig_ParseXML(fd);
        int closed = fclose(fd);

        sopcResult = (0 == closed && NULL != g_interactive_Context.pConfig) ? SOPC_STATUS_OK : SOPC_STATUS_INVALID_PARAMETERS;
        if (SOPC_STATUS_OK != sopcResult)
        {
            LOG_Trace(LOG_ERROR, "Error while loading PubSub configuration from %s", pubSubXmlConfigFile);
        }
        else
        {
            LOG_Trace(LOG_DEBUG, "[OK] PUBSUB initialized: (%s)", pubSubXmlConfigFile);
        }
    }

    // CACHE

    if (SOPC_STATUS_OK == sopcResult)
    {
        bool res;
        assert(NULL != g_interactive_Context.pConfig);

        res = UAM_Cache_Initialize(g_interactive_Context.pConfig); // TODO should be moved in UAM NS?

        if (!res)
        {
            LOG_Trace(LOG_ERROR, "Error while initializing the cache, refer to log files");
            sopcResult = SOPC_STATUS_NOK;
        }
        else
        {
            UAM_Cache_SetNotify(&cache_Notify_CB);
        }
    }

    if (SOPC_STATUS_OK == sopcResult)
    {
        PubSub_initialize_publisher(&sopcResult);
        LOG_Trace(LOG_DEBUG, "PubSub_initialize_publisher returned %d", sopcResult);
    }

    if (SOPC_STATUS_OK == sopcResult)
    {
        PubSub_initialize_subscriber(&sopcResult);
        LOG_Trace(LOG_DEBUG, "PubSub_initialize_subscriber returned %d", sopcResult);
    }
    return sopcResult;
}

/*===========================================================================*/
void UAM_NS_Impl_Clear(void)
{
    if (g_interactive_Context.pConfig != NULL)
    {
        SOPC_PubSubConfiguration_Delete (g_interactive_Context.pConfig);
    }
}

/*===========================================================================*/
bool UAM_NS_CreateSpdu(const UAM_NS_Configuration_type* const pzConfig)
{
    bool bResult = false;
    if (gSessions != NULL && pzConfig != NULL)
    {
        LOG_Trace(LOG_DEBUG, "UAM_NS_CreateSpdu, HDL=%u", (unsigned) pzConfig->dwHandle);
        bResult = Session_Add(pzConfig);
        if (bResult)
        {
            bResult = UAM_NS2S_Initialize(pzConfig->dwHandle);
        }
    }
    return bResult;
}

/*===========================================================================*/
void UAM_NS_RequestMessageReceived(UAM_SessionHandle dwHandle)
{
    // Simply notify the sending thread because Cache is already up to date.
    (void) dwHandle;

    if (pzQueue != NULL)
    {
        EnqueueEvent(dwHandle, qaSpduRequestToSafe);
    }
}

/*===========================================================================*/
void UAM_NS_ResponseMessageReceived(UAM_SessionHandle dwHandle)
{
    // Simply notify the sending thread because Cache is already up to date.
    if (pzQueue != NULL)
    {
        EnqueueEvent(dwHandle, qaSpduResponseToSafe);
    }
}

/*===========================================================================*/
void UAM_NS_CheckSpduReception(UAM_SessionHandle dwHandle)
{
    if (pzQueue != NULL)
    {
        EnqueueEvent(dwHandle, qaSpduPollSafe);
    }
}

/*===========================================================================*/
void UAM_NS_Clear(void)
{

    // Request the Thread to terminate, using an empty event
    stopSignal = 1;
    EnqueueNoEvent();

    if (gSessions != NULL)
    {
        SOPC_Dict_Delete(gSessions);
        gSessions = NULL;
    }

    if (gNodeIdsDict != NULL)
    {
        SOPC_Dict_Delete(gNodeIdsDict);
        gNodeIdsDict = NULL;
    }

    LOG_Trace(LOG_INFO, "UAM_NS_Clear : waiting for Thread termination...");
    SOPC_Thread_Join(gThread);
    LOG_Trace(LOG_INFO, "UAM_NS_Clear : Thread terminated");
    SOPC_AsyncQueue_Free(&pzQueue);
    UAM_NS2S_Clear();
}
