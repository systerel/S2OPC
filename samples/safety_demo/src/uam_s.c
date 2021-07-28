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

#include "uam_s.h"
#include "uam_s2ns_itf.h"
#include "uas.h"

#define KILOBYTE (1024lu)
#define HEAP_SIZE (128u * KILOBYTE)
#include "uam_s_libs.h"

/**************************************************************************
 **************************************************************************
 **************************************************************************
 **************************************************************************/


/*============================================================================
 * LOCAL TYPES
 *===========================================================================*/

typedef struct UAM_DynamicSafetyData_struct
{
    bool bInitialized;
    bool bLocked;
    UAM_S_ProviderHandle bNextProviderFreeHandle;
    UAM_S_ConsumerHandle bNextConsumerFreeHandle;
    UAM_SafetyConfiguration_type azProviderConfiguration[UASDEF_MAX_SAFETYPROVIDERS];
    UAM_SafetyConfiguration_type azConsumerConfiguration[UASDEF_MAX_SAFETYCONSUMERS];
    UAM_S_pfProviderApplicationCycle apfProviderCycle[UASDEF_MAX_SAFETYPROVIDERS];
    UAM_S_pfConsumerApplicationCycle apfConsumerCycle[UASDEF_MAX_SAFETYCONSUMERS];
} UAM_DynamicSafetyData_type;

/*============================================================================
 * LOCAL VARIABLES
 *===========================================================================*/
static UAM_S_LOG_LEVEL gLogLevel = UAM_S_LOG_ERROR;
/**
 * Content of all Providers and Consumers configurations
 */
static UAM_DynamicSafetyData_type uamDynamicSafetyData = {.bInitialized = false,
                                                          .bLocked = false,
                                                          .bNextConsumerFreeHandle = 0,
                                                          .azProviderConfiguration = {{0}},
                                                          .azConsumerConfiguration = {{0}},
                                                          .apfProviderCycle = {0},
                                                          .apfConsumerCycle = {0}};

static UAM_LIBS_Heap_type zHeap;

// TODO : make that variable configurable?
#define UAS_ENCODING_BUFFER_SIZE ( (UAM_S_Size) 1500u)

static UAS_Char aEncodingBuffer [UAS_ENCODING_BUFFER_SIZE];

/*============================================================================
 * DECLARATION OF INTERNAL SERVICES
 *===========================================================================*/
static UAS_UInt8 ExecuteSafetyProviders(void);
static UAS_UInt8 ExecuteSafetyConsumers(void);

/**
 * \brief This method is called for each received message from NonSafe.
 */
static void ReadSpduFromSlave(const UAM_SessionId dwSessionId, const void* pData, const UAM_S_Size sReadLen);

/**
 * Convert a SPDU response to a raw buffer.
 */
static void EncodeSpduReponse(void* pData, const UAM_S_Size sMaxLen, UAM_S_Size* psLen,
        const UAM_SafetyConfiguration_type* pzConfig, const UAS_ResponseSpdu_type* pzResponse);

/**
 * Convert a SPDU Request to a raw buffer.
 */
static void EncodeSpduRequest(void* pData, const UAM_S_Size sMaxLen, UAM_S_Size* psLen, const UAS_RequestSpdu_type* pzRequest);

/**
 * \brief Implementation of SPDU Request decoding from raw buffer.
 * \param pData A non-null pointer to the data
 * \param sLen Length of pData
 * \param[OUT] pzRequest non null pointer to the Request to decode. The data shall not be modified
 *          in case of decoding error (typically, mismatching size)
 */
static void DecodeSpduRequest(const void* pData, UAM_S_Size sLen, UAS_RequestSpdu_type* pzRequest);

/**
 * \brief Implementation of SPDU Response decoding from raw buffer.
 * \param pData A non-null pointer to the data
 * \param sLen Length of pData
 * \param pzConfig A non-null pointer to the Safety configuration of matching message.
 * \param[OUT] pzResponse non null pointer to the Request to decode. The data shall not be modified
 *          in case of decoding error (typically, mismatching size)
 */
static void DecodeSpduResponse(const void* pData, UAM_S_Size sLen, const UAM_SafetyConfiguration_type* pzConfig, UAS_ResponseSpdu_type* pzResponse);

/*============================================================================
 * IMPLEMENTATION OF INTERNAL SERVICES
 *===========================================================================*/
/*===========================================================================*/
static void DecodeSpduRequest(const void* pData, UAM_S_Size sLen, UAS_RequestSpdu_type* pzRequest)
{
    UAM_S_DoLog(UAM_S_LOG_SEQUENCE, "DecodeSpduRequest-In");
    // This function proceeds to the reverse decoding of EncodeSpduRequest() of NON-SAFE
    UAM_S_LIBS_ASSERT (pData != NULL);
    UAM_S_LIBS_ASSERT (pzRequest != NULL);

    UAM_S_Size pos = 0;
    const UAS_UInt8* pBytes = pData;
    static const UAM_S_Size sExpectedLength = 9;

    if (sExpectedLength == sLen)
    {
        pzRequest->dwSafetyConsumerId = UAM_S_LIBS_deserialize_UInt32(pBytes, sLen, &pos);
        pzRequest->dwMonitoringNumber = UAM_S_LIBS_deserialize_UInt32(pBytes, sLen, &pos);
        pzRequest->byFlags = UAM_S_LIBS_deserialize_UInt8(pBytes, sLen, &pos);
        UAM_S_DoLog_UInt (UAM_S_LOG_DEBUG, "DecodeSpduRequest => decoding OK. Received len = ", sLen);
        UAM_S_LIBS_ASSERT (pos == sLen);
    }
    else
    {
        UAM_S_DoLog_UInt (UAM_S_LOG_WARN, "Mismatching size in DecodeSpduRequest. Received len = ", sLen);
    }
    UAM_S_DoLog(UAM_S_LOG_SEQUENCE, "DecodeSpduRequest-Out");
}


/*===========================================================================*/
static void DecodeSpduResponse(const void* pData, UAM_S_Size sLen, const UAM_SafetyConfiguration_type* pzConfig,
        UAS_ResponseSpdu_type* pzResponse)
{
    // This function proceeds to the reverse decoding of EncodeSpduResponse()
    UAM_S_LIBS_ASSERT (pData != NULL);
    UAM_S_LIBS_ASSERT (pzConfig != NULL);
    UAM_S_LIBS_ASSERT (pzResponse != NULL);
    UAM_S_DoLog_UHex32(UAM_S_LOG_SEQUENCE, "DecodeSpduResponse-In, HDL = ", pzConfig->dwSessionId);

    UAM_S_Size pos = 0;
    const UAS_UInt8* pBytes = pData;
    static const UAM_S_Size sStaticLength = 25u;
    const UAM_S_Size sExpectedLength = sStaticLength + pzConfig->wNonSafetyDataLength + pzConfig->wSafetyDataLength;

    if (sExpectedLength == sLen)
    {
        // de-serialize Safe data
        UAM_S_LIBS_deserialize_String(pBytes, sLen, &pos, pzResponse->pbySerializedSafetyData, pzConfig->wSafetyDataLength);
        // de-serialize byFlags
        pzResponse->byFlags = UAM_S_LIBS_deserialize_UInt8(pBytes, sLen, &pos);
        // de-serialize zSpduId
        pzResponse->zSpduId.dwPart1 = UAM_S_LIBS_deserialize_UInt32(pBytes, sLen, &pos);
        pzResponse->zSpduId.dwPart2 = UAM_S_LIBS_deserialize_UInt32(pBytes, sLen, &pos);
        pzResponse->zSpduId.dwPart3 = UAM_S_LIBS_deserialize_UInt32(pBytes, sLen, &pos);
        // de-serialize dwSafetyConsumerId
        pzResponse->dwSafetyConsumerId = UAM_S_LIBS_deserialize_UInt32(pBytes, sLen, &pos);
        // de-serialize dwMonitoringNumber
        pzResponse->dwMonitoringNumber = UAM_S_LIBS_deserialize_UInt32(pBytes, sLen, &pos);
        // de-serialize dwCrc
        pzResponse->dwCrc = UAM_S_LIBS_deserialize_UInt32(pBytes, sLen, &pos);
        // de-serialize NonSafe data
        UAM_S_LIBS_deserialize_String(pBytes, sLen, &pos, pzResponse->pbySerializedNonSafetyData, pzConfig->wNonSafetyDataLength);
        UAM_S_LIBS_ASSERT (pos == sLen);

        UAM_S_DoLog_UInt (UAM_S_LOG_DEBUG, "DecodeSpduRequest => decoding OK. Received len = ", sLen);
    }
    else
    {
        UAM_S_DoLog_UInt (UAM_S_LOG_WARN, "Mismatching size in DecodeSpduRequest. Received len = ", sLen);
    }
    UAM_S_DoLog(UAM_S_LOG_SEQUENCE, "DecodeSpduResponse-Out");
}

/*===========================================================================*/
static void EncodeSpduRequest(void* pData, const UAM_S_Size sMaxLen, UAM_S_Size* psLen, const UAS_RequestSpdu_type* pzRequest)
{
    if (psLen == NULL)
    {
        return;
    }
    *psLen = 0;
    if (pzRequest == NULL || pData == NULL)
    {
        return;
    }

    static const size_t expLen = 9u;
    UAM_S_LIBS_ASSERT (sMaxLen >- expLen && "Insufficient buffer size for EncodeSpduRequest");

    UAM_S_LIBS_serialize_UInt32(pzRequest->dwSafetyConsumerId, pData, expLen, psLen);
    UAM_S_LIBS_serialize_UInt32(pzRequest->dwMonitoringNumber, pData, expLen, psLen);
    UAM_S_LIBS_serialize_UInt8(pzRequest->byFlags, pData, expLen, psLen);
    UAM_S_DoLog_UInt(UAM_S_LOG_DEBUG, "EncodeSpduRequest len=", (unsigned) (*psLen));
    UAM_S_LIBS_ASSERT ((*psLen) == expLen);

    return;
}

/*===========================================================================*/
static void EncodeSpduReponse(void* pData, const UAM_S_Size sMaxLen, UAM_S_Size* psLen,
        const UAM_SafetyConfiguration_type* pzConfig, const UAS_ResponseSpdu_type* pzResponse)
{
    static const UAM_S_Size sStaticLength = 25u;
    if (psLen == NULL)
    {
        return;
    }
    *psLen = 0;
    if (pzResponse == NULL || pData == NULL)
    {
        return;
    }

    const size_t expLen = sStaticLength + pzConfig->wNonSafetyDataLength + pzConfig->wSafetyDataLength;
    UAM_S_LIBS_ASSERT (sMaxLen >= expLen && "Insufficient buffer size for EncodeSpduRequest");

    // SAFE data
    UAM_S_LIBS_serialize_String(pzResponse->pbySerializedSafetyData, pzConfig->wSafetyDataLength, pData, expLen, psLen);
    UAM_S_LIBS_serialize_UInt8(pzResponse->byFlags, pData, expLen, psLen);
    UAM_S_LIBS_serialize_UInt32(pzResponse->zSpduId.dwPart1, pData, expLen, psLen);
    UAM_S_LIBS_serialize_UInt32(pzResponse->zSpduId.dwPart2, pData, expLen, psLen);
    UAM_S_LIBS_serialize_UInt32(pzResponse->zSpduId.dwPart3, pData, expLen, psLen);
    UAM_S_LIBS_serialize_UInt32(pzResponse->dwSafetyConsumerId, pData, expLen, psLen);
    UAM_S_LIBS_serialize_UInt32(pzResponse->dwMonitoringNumber, pData, expLen, psLen);
    UAM_S_LIBS_serialize_UInt32(pzResponse->dwCrc, pData, expLen, psLen);
    UAM_S_LIBS_serialize_String(pzResponse->pbySerializedNonSafetyData, pzConfig->wNonSafetyDataLength, pData, expLen, psLen);
    UAM_S_DoLog_UInt(UAM_S_LOG_DEBUG, "EncodeSpduReponse len=", (unsigned) (*psLen));
    UAM_S_LIBS_ASSERT ((*psLen) == expLen);

    return;
}

/*===========================================================================*/
static void ReadSpduFromSlave(const UAM_SessionId dwSessionId, const void* pData, const UAM_S_Size sReadLen)
{
    UAM_S_DoLog(UAM_S_LOG_SEQUENCE, "ReadSpduFromSlave-In");


    UAS_UInt16 wInstanceCount = 0u;
    // TODO: make lookup more efficient (not needed if there are no more than a few PROV/CONS

    for (wInstanceCount = 0u; wInstanceCount < uamDynamicSafetyData.bNextProviderFreeHandle; wInstanceCount++)
    {
        UAS_SafetyProvider_type* pzProvider = &azUAS_SafetyProviders[wInstanceCount];
        const UAM_SafetyConfiguration_type* pzSafetyCfg =
                &uamDynamicSafetyData.azProviderConfiguration[wInstanceCount];

        UAM_S_LIBS_ASSERT (wInstanceCount == pzProvider->dwHandle);// TODO remove this assert if OK

        if (pzSafetyCfg->dwSessionId == dwSessionId)
        {
            // For a provider the received SPDU is the REQUEST. Update buffer for UAS
            DecodeSpduRequest (pData, sReadLen, &pzProvider->zRequestSPDU);
        }
    }

    for (wInstanceCount = 0u; wInstanceCount < uamDynamicSafetyData.bNextConsumerFreeHandle; wInstanceCount++)
    {
        UAS_SafetyConsumer_type* pzConsumer = &azUAS_SafetyConsumers[wInstanceCount];
        const UAM_SafetyConfiguration_type* pzSafetyCfg =
                &uamDynamicSafetyData.azProviderConfiguration[wInstanceCount];

        UAM_S_LIBS_ASSERT (wInstanceCount == pzConsumer->dwHandle);// TODO remove this assert if OK

        if (pzSafetyCfg->dwSessionId == dwSessionId)
        {
            // For a consumer the received SPDU is the RESPONSE. Update buffer for UAS
            DecodeSpduResponse (pData, sReadLen, pzSafetyCfg, &pzConsumer->zResponseSPDU);
        }
    }


    UAM_S_DoLog(UAM_S_LOG_SEQUENCE, "ReadSpduFromSlave-Out");
}

/*============================================================================
 * IMPLEMENTATION OF EXTERNAL SERVICES
 *===========================================================================*/

/*===========================================================================*/
void UAM_S_Initialize(const UAM_S_LOG_LEVEL initLogLevel)
{
    UAM_S_DoLog_UInt(UAM_S_LOG_DEFAULT, "Logs initialized with level :", initLogLevel);
    UAM_S_DoLog(UAM_S_LOG_SEQUENCE, "UAM_S_Initialize-In");
    UAM_S_LIBS_ASSERT(!uamDynamicSafetyData.bInitialized);
    UAS_UInt16 index = 0;

    gLogLevel = initLogLevel;
    UAM_S_LIBS_HEAP_Init (&zHeap);
    UAM_S_LIBS_MemZero(azUAS_SafetyProviders, sizeof(azUAS_SafetyProviders));
    uamDynamicSafetyData.bNextProviderFreeHandle = 0;
    uamDynamicSafetyData.bNextConsumerFreeHandle = 0;

    for (index = 0; index < UASDEF_MAX_SAFETYPROVIDERS; ++index)
    {
        UAM_SafetyConfiguration_type* pzConf = &uamDynamicSafetyData.azProviderConfiguration[index];
        pzConf->wNonSafetyDataLength = 0;
        pzConf->wSafetyDataLength = 0;
        uamDynamicSafetyData.apfProviderCycle[index] = NULL;
    }
    for (index = 0; index < UASDEF_MAX_SAFETYCONSUMERS; ++index)
    {
        UAM_SafetyConfiguration_type* pzConf = &uamDynamicSafetyData.azConsumerConfiguration[index];
        pzConf->wNonSafetyDataLength = 0;
        pzConf->wSafetyDataLength = 0;
        uamDynamicSafetyData.apfConsumerCycle[index] = NULL;
    }
    uamDynamicSafetyData.bInitialized = true;
    uamDynamicSafetyData.bLocked = false;

    UAM_S2NS_Initialize();

    UAM_S_DoLog(UAM_S_LOG_SEQUENCE, "UAM_S_Initialize-Out");
}

/*===========================================================================*/
UAS_UInt8 UAM_S_InitSafetyProvider(const UAM_SafetyConfiguration_type* const pzInstanceConfiguration,
                                   const UAS_SafetyProviderSPI_type* const pzSPI,
                                   UAM_S_pfProviderApplicationCycle pfProviderCycle,
                                   UAM_S_ProviderHandle* phHandle)
{
    UAM_S_DoLog(UAM_S_LOG_SEQUENCE, "UAM_S_InitSafetyProvider-In");
    UAM_S_LIBS_ASSERT(uamDynamicSafetyData.bInitialized);
    UAM_S_LIBS_ASSERT(!uamDynamicSafetyData.bLocked);
    UAM_S_LIBS_ASSERT(NULL != pzSPI);
    UAM_S_LIBS_ASSERT(NULL != pfProviderCycle);
    UAM_S_LIBS_ASSERT(NULL != pzInstanceConfiguration);
    UAM_S_LIBS_ASSERT(NULL != phHandle);
    UAM_S_LIBS_ASSERT(uamDynamicSafetyData.bNextProviderFreeHandle < UASDEF_MAX_SAFETYPROVIDERS);

    const UAS_UInt8 handle = (UAS_UInt8) uamDynamicSafetyData.bNextProviderFreeHandle;
    UAS_UInt8 byRetVal = UAS_OK;
    UAS_ParameterError_type nResult = UAS_PARAMETER_OK;
    UAS_SafetyProvider_type* pzSafetyProvider = &(azUAS_SafetyProviders[handle]);

    // Create unique entry
    *phHandle = (UAS_UInt8) handle;
    uamDynamicSafetyData.bNextProviderFreeHandle++;

    // Initialize all in/out
    UAM_S_LIBS_MemZero(&pzSafetyProvider->zInputSAPI, sizeof(pzSafetyProvider->zInputSAPI));
    UAM_S_LIBS_MemZero(&pzSafetyProvider->zOutputSAPI, sizeof(pzSafetyProvider->zOutputSAPI));
    pzSafetyProvider->bAppDone = 0;
    pzSafetyProvider->bCommDone = 0;
    // Allocate buffers for IN/OUTs
    pzSafetyProvider->zInputSAPI.pbySerializedNonSafetyData =
        (UAS_UInt8*) UAM_S_LIBS_HEAP_Malloc(&zHeap, pzInstanceConfiguration->wNonSafetyDataLength);
    pzSafetyProvider->zInputSAPI.pbySerializedSafetyData =
        (UAS_UInt8*) UAM_S_LIBS_HEAP_Malloc(&zHeap, pzInstanceConfiguration->wSafetyDataLength);
    pzSafetyProvider->zResponseSPDU.pbySerializedNonSafetyData =
        (UAS_UInt8*) UAM_S_LIBS_HEAP_Malloc(&zHeap, pzInstanceConfiguration->wNonSafetyDataLength);
    pzSafetyProvider->zResponseSPDU.pbySerializedSafetyData =
        (UAS_UInt8*) UAM_S_LIBS_HEAP_Malloc(&zHeap, pzInstanceConfiguration->wSafetyDataLength);
    UAM_S_LIBS_ASSERT(NULL != pzSafetyProvider->zInputSAPI.pbySerializedNonSafetyData);
    UAM_S_LIBS_ASSERT(NULL != pzSafetyProvider->zInputSAPI.pbySerializedSafetyData);
    UAM_S_LIBS_ASSERT(NULL != pzSafetyProvider->zResponseSPDU.pbySerializedNonSafetyData);
    UAM_S_LIBS_ASSERT(NULL != pzSafetyProvider->zResponseSPDU.pbySerializedSafetyData);

    uamDynamicSafetyData.azProviderConfiguration[handle] = *pzInstanceConfiguration;
    uamDynamicSafetyData.apfProviderCycle[handle] = pfProviderCycle;

    pzSafetyProvider->dwHandle = handle;
    pzSafetyProvider->wSafetyDataLength = pzInstanceConfiguration->wSafetyDataLength;
    pzSafetyProvider->wNonSafetyDataLength = pzInstanceConfiguration->wNonSafetyDataLength;
    pzSafetyProvider->zSPI = *pzSPI;

    UAM_S2NS_InitializeSpdu(pzInstanceConfiguration->dwSessionId);

    byRetVal = byUAS_InitSafetyProvider(pzSafetyProvider, handle, &nResult);
    UAM_S_DoLog(UAM_S_LOG_SEQUENCE, "UAM_S_InitSafetyProvider-Out");
    return byRetVal;
}

/*===========================================================================*/
UAS_UInt8 UAM_S_InitSafetyConsumer(const UAM_SafetyConfiguration_type* const pzInstanceConfiguration,
                                   const UAS_SafetyConsumerSPI_type* const pzSPI,
                                   UAM_S_pfConsumerApplicationCycle pfConsumerCycle,
                                   UAM_S_ProviderHandle* phHandle)
{
    UAM_S_DoLog(UAM_S_LOG_SEQUENCE, "UAM_S_InitSafetyConsumer-In");
    UAM_S_LIBS_ASSERT(uamDynamicSafetyData.bInitialized);
    UAM_S_LIBS_ASSERT(!uamDynamicSafetyData.bLocked);
    UAM_S_LIBS_ASSERT(NULL != pzSPI);
    UAM_S_LIBS_ASSERT(NULL != pfConsumerCycle);
    UAM_S_LIBS_ASSERT(NULL != pzInstanceConfiguration);
    UAM_S_LIBS_ASSERT(NULL != phHandle);
    UAM_S_LIBS_ASSERT(uamDynamicSafetyData.bNextConsumerFreeHandle < UASDEF_MAX_SAFETYCONSUMERS);

    const UAS_UInt8 handle = (UAS_UInt8) uamDynamicSafetyData.bNextConsumerFreeHandle;
    UAS_UInt8 byRetVal = UAS_OK;
    UAS_ParameterError_type nResult = UAS_PARAMETER_OK;
    UAS_SafetyConsumer_type* pzSafetyProvider = &(azUAS_SafetyConsumers[handle]);

    // Create unique entry
    *phHandle = (UAS_UInt8) handle;
    uamDynamicSafetyData.bNextConsumerFreeHandle++;

    // Allocate buffers for IN/OUTs
    pzSafetyProvider->zResponseSPDU.pbySerializedNonSafetyData =
        (UAS_UInt8*) UAM_S_LIBS_HEAP_Malloc(&zHeap, pzInstanceConfiguration->wNonSafetyDataLength);
    pzSafetyProvider->zResponseSPDU.pbySerializedSafetyData =
        (UAS_UInt8*) UAM_S_LIBS_HEAP_Malloc(&zHeap, pzInstanceConfiguration->wSafetyDataLength);
    pzSafetyProvider->zOutputSAPI.pbySerializedNonSafetyData =
        (UAS_UInt8*) UAM_S_LIBS_HEAP_Malloc(&zHeap, pzInstanceConfiguration->wNonSafetyDataLength);
    pzSafetyProvider->zOutputSAPI.pbySerializedSafetyData =
        (UAS_UInt8*) UAM_S_LIBS_HEAP_Malloc(&zHeap, pzInstanceConfiguration->wSafetyDataLength);
    UAM_S_LIBS_ASSERT(NULL != pzSafetyProvider->zResponseSPDU.pbySerializedNonSafetyData);
    UAM_S_LIBS_ASSERT(NULL != pzSafetyProvider->zResponseSPDU.pbySerializedSafetyData);
    UAM_S_LIBS_ASSERT(NULL != pzSafetyProvider->zOutputSAPI.pbySerializedSafetyData);
    UAM_S_LIBS_ASSERT(NULL != pzSafetyProvider->zOutputSAPI.pbySerializedNonSafetyData);

    uamDynamicSafetyData.azConsumerConfiguration[handle] = *pzInstanceConfiguration;
    uamDynamicSafetyData.apfConsumerCycle[handle] = pfConsumerCycle;

    pzSafetyProvider->dwHandle = handle;
    pzSafetyProvider->wSafetyDataLength = pzInstanceConfiguration->wSafetyDataLength;
    pzSafetyProvider->wNonSafetyDataLength = pzInstanceConfiguration->wNonSafetyDataLength;
    pzSafetyProvider->zSPI = *pzSPI;

    byRetVal = byUAS_InitSafetyConsumer(pzSafetyProvider, handle, &nResult);
    UAM_S_DoLog(UAM_S_LOG_SEQUENCE, "UAM_S_InitSafetyConsumer-Out");
    return byRetVal;
}
/*===========================================================================*/
UAS_UInt8 UAM_S_StartSafety(void)
{
    UAM_S_DoLog(UAM_S_LOG_SEQUENCE, "UAM_S_StartSafety-In");
    UAM_S_LIBS_ASSERT(uamDynamicSafetyData.bInitialized);
    UAM_S_LIBS_ASSERT(!uamDynamicSafetyData.bLocked);
    UAS_UInt8 byRetVal = UAS_OK;
    UAS_UInt16 wInstanceCount = 0u;

    for (wInstanceCount = 0u; wInstanceCount < uamDynamicSafetyData.bNextProviderFreeHandle; wInstanceCount++)
    {
        UAS_SafetyProvider_type* pzInstance = &azUAS_SafetyProviders[wInstanceCount];

        if (byRetVal == UAS_OK)
        {
            byRetVal = byUAS_StartSafetyProvider(wInstanceCount, pzInstance->dwHandle);
        }
    }

    for (wInstanceCount = 0u; wInstanceCount < uamDynamicSafetyData.bNextConsumerFreeHandle; wInstanceCount++)
    {
        UAS_SafetyConsumer_type* pzInstance = &azUAS_SafetyConsumers[wInstanceCount];

        if (byRetVal == UAS_OK)
        {
            byRetVal = byUAS_StartSafetyConsumer(wInstanceCount, pzInstance->dwHandle);
        }
    }

    if (byRetVal == UAS_OK)
    {
        uamDynamicSafetyData.bLocked = true;
    }
    UAM_S_DoLog(UAM_S_LOG_SEQUENCE, "UAM_S_StartSafety-Out");
    return byRetVal;
}
/*===========================================================================*/
UAS_UInt8 UAM_S_Cycle(void)
{
    UAM_S_DoLog(UAM_S_LOG_SEQUENCE, "UAM_S_Cycle-In");
    UAS_UInt8 byRetVal = UAS_SOFT_ERR;
    UAM_S_LIBS_ASSERT(uamDynamicSafetyData.bInitialized);
    UAM_S_LIBS_ASSERT(uamDynamicSafetyData.bLocked);

    UAM_S2NS_ReceiveAllSpdusFromNonSafe(&ReadSpduFromSlave);

    byRetVal = ExecuteSafetyProviders();
    if (byRetVal == UAS_OK)
    {
        byRetVal = ExecuteSafetyConsumers();
    }

    UAM_S_DoLog(UAM_S_LOG_SEQUENCE, "UAM_S_Cycle-Out");
    return byRetVal;
}

/*===========================================================================*/
void UAM_S_Clear(void)
{
    UAM_S_DoLog(UAM_S_LOG_SEQUENCE, "UAM_S_Clear-In");
    UAS_UInt16 index = 0;

    UAM_S2NS_Clear();
    for (index = 0; index < uamDynamicSafetyData.bNextProviderFreeHandle; ++index)
    {
        UAS_SafetyProvider_type* pzSafetyProvider = &(azUAS_SafetyProviders[index]);
        UAM_S_LIBS_ASSERT(pzSafetyProvider->zInputSAPI.pbySerializedSafetyData != NULL);
        UAM_S_LIBS_ASSERT(pzSafetyProvider->zInputSAPI.pbySerializedNonSafetyData != NULL);
        UAM_S_LIBS_ASSERT(pzSafetyProvider->zResponseSPDU.pbySerializedSafetyData != NULL);
        UAM_S_LIBS_ASSERT(pzSafetyProvider->zResponseSPDU.pbySerializedNonSafetyData != NULL);
        // Frees managed by HEAP_Clear
//        free(pzSafetyProvider->zInputSAPI.pbySerializedSafetyData);
//        free(pzSafetyProvider->zInputSAPI.pbySerializedNonSafetyData);
//        free(pzSafetyProvider->zResponseSPDU.pbySerializedSafetyData);
//        free(pzSafetyProvider->zResponseSPDU.pbySerializedNonSafetyData);
        pzSafetyProvider->zInputSAPI.pbySerializedSafetyData = NULL;
        pzSafetyProvider->zInputSAPI.pbySerializedNonSafetyData = NULL;
        pzSafetyProvider->zResponseSPDU.pbySerializedSafetyData = NULL;
        pzSafetyProvider->zResponseSPDU.pbySerializedNonSafetyData = NULL;
        uamDynamicSafetyData.apfProviderCycle[index] = NULL;
    }
    for (index = 0; index < uamDynamicSafetyData.bNextConsumerFreeHandle; ++index)
    {
        UAS_SafetyConsumer_type* pzSafetyConsumer = &(azUAS_SafetyConsumers[index]);
        UAM_S_LIBS_ASSERT(pzSafetyConsumer->zOutputSAPI.pbySerializedSafetyData != NULL);
        UAM_S_LIBS_ASSERT(pzSafetyConsumer->zOutputSAPI.pbySerializedNonSafetyData != NULL);
        UAM_S_LIBS_ASSERT(pzSafetyConsumer->zResponseSPDU.pbySerializedSafetyData != NULL);
        UAM_S_LIBS_ASSERT(pzSafetyConsumer->zResponseSPDU.pbySerializedNonSafetyData != NULL);
        // Frees managed by HEAP_Clear
//        free(pzSafetyConsumer->zOutputSAPI.pbySerializedSafetyData);
//        free(pzSafetyConsumer->zOutputSAPI.pbySerializedNonSafetyData);
//        free(pzSafetyConsumer->zResponseSPDU.pbySerializedSafetyData);
//        free(pzSafetyConsumer->zResponseSPDU.pbySerializedNonSafetyData);
        pzSafetyConsumer->zOutputSAPI.pbySerializedSafetyData = NULL;
        pzSafetyConsumer->zOutputSAPI.pbySerializedNonSafetyData = NULL;
        pzSafetyConsumer->zResponseSPDU.pbySerializedSafetyData = NULL;
        pzSafetyConsumer->zResponseSPDU.pbySerializedNonSafetyData = NULL;
        uamDynamicSafetyData.apfConsumerCycle[index] = NULL;
    }
    uamDynamicSafetyData.bNextProviderFreeHandle = 0;
    uamDynamicSafetyData.bNextConsumerFreeHandle = 0;
    uamDynamicSafetyData.bInitialized = false;
    UAM_S_LIBS_HEAP_Clear (&zHeap);
    UAM_S_DoLog(UAM_S_LOG_SEQUENCE, "UAM_S_Clear-Out");
}

/*===========================================================================*/
static UAS_UInt8 ExecuteSafetyConsumers(void)
{
    UAM_S_DoLog(UAM_S_LOG_SEQUENCE, "ExecuteSafetyConsumers-In");
    UAS_UInt8 byUasRetval = UAS_OK;
    UAS_UInt16 wInstanceCount = 0u;

    for (wInstanceCount = 0u; wInstanceCount < uamDynamicSafetyData.bNextConsumerFreeHandle; wInstanceCount++)
    {
        UAM_S_DoLog_UInt(UAM_S_LOG_SEQUENCE, "ExecuteSafetyConsumers, Id=", wInstanceCount);
        UAM_S_LIBS_ASSERT(NULL != uamDynamicSafetyData.apfConsumerCycle);
        UAS_SafetyConsumer_type* pzInstance = &azUAS_SafetyConsumers[wInstanceCount];
        UAM_SafetyConfiguration_type* pzConfig = &uamDynamicSafetyData.azConsumerConfiguration[wInstanceCount];

        /* Get ResponseSPDU */
        // Note: reception of SPDU Response (in pzProvider->zResponsePDU) has already been performed in ReadSpduFromSlave()
        pzInstance->bCommDone = 1u;

        pzInstance->bAppDone =
            (*uamDynamicSafetyData.apfConsumerCycle)(pzConfig, &pzInstance->zOutputSAPI, &pzInstance->zInputSAPI);

        /* Execute SafetyConsumer */
        if (byUasRetval == UAS_OK)
        {
            byUasRetval = byUAS_ExecuteSafetyConsumer(wInstanceCount, pzInstance->dwHandle);
            if (UAS_OK != byUasRetval)
            {
                UAM_S_DoLog_UInt(UAM_S_LOG_ERROR, "byUAS_ExecuteSafetyConsumer() failed for HDL = ", pzInstance->dwHandle);
                UAM_S_DoLog_UHex32(UAM_S_LOG_ERROR, "byUAS_ExecuteSafetyConsumer() code was = ", byUasRetval);
            }
        }

        /* Set RequestSPDU */
        if (byUasRetval == UAS_OK)
        {
            UAM_S_Size sLen = 0;
            EncodeSpduRequest(aEncodingBuffer, UAS_ENCODING_BUFFER_SIZE, &sLen, &pzInstance->zRequestSPDU);
            if (sLen > 0)
            {
                UAM_S2NS_SendSpduImpl (pzConfig->dwSessionId, aEncodingBuffer, sLen);
                UAM_S_DoLog_UInt(UAM_S_LOG_DEBUG, "UAM_SetRequestSPDU HDL =", pzInstance->dwHandle);
                UAM_S_DoLog_UHex32(UAM_S_LOG_DEBUG, "   SafetyConsumerId = ", pzInstance->zRequestSPDU.dwSafetyConsumerId);
                UAM_S_DoLog_UHex32(UAM_S_LOG_DEBUG, "   MonitoringNumber = ", pzInstance->zRequestSPDU.dwMonitoringNumber);
                UAM_S_DoLog_UHex32(UAM_S_LOG_DEBUG, "   Flags            = ", pzInstance->zRequestSPDU.byFlags);
            } /* if */
            else
            {
                UAM_S_DoLog_UInt(UAM_S_LOG_ERROR, "UAM_SetRequestSPDU() failed for HDL = ", pzInstance->dwHandle);
            }
        }
    } /* for wNumberOfSafetyConsumers */
    UAM_S_DoLog(UAM_S_LOG_SEQUENCE, "ExecuteSafetyConsumers-Out");
    return byUasRetval;
}

/*===========================================================================*/
static UAS_UInt8 ExecuteSafetyProviders(void)
{
    UAM_S_DoLog(UAM_S_LOG_SEQUENCE, "ExecuteSafetyProviders-In");
    UAS_UInt8 byUasRetval = UAS_OK;
    UAS_UInt16 wInstanceCount = 0u;

    for (wInstanceCount = 0u; wInstanceCount < uamDynamicSafetyData.bNextProviderFreeHandle; wInstanceCount++)
    {
        UAM_S_DoLog_UInt(UAM_S_LOG_SEQUENCE, "ExecuteSafetyProviders, Id=", wInstanceCount);
        UAM_S_LIBS_ASSERT(NULL != uamDynamicSafetyData.apfProviderCycle);
        UAS_SafetyProvider_type* pzInstance = &azUAS_SafetyProviders[wInstanceCount];
        UAM_SafetyConfiguration_type* pzConfig = &uamDynamicSafetyData.azProviderConfiguration[wInstanceCount];

        pzInstance->bCommDone = 1;
        /* Execute the application */
        pzInstance->bAppDone =
            (*uamDynamicSafetyData.apfProviderCycle)(pzConfig, &pzInstance->zOutputSAPI, &pzInstance->zInputSAPI);
        if (pzInstance->bAppDone != 1u)
        {
            UAM_S_DoLog(UAM_S_LOG_ERROR, "byUAS_ExecuteSafetyProvider( ) Application cycle (apfProviderCycle) returned an error");
            byUasRetval = UAS_SOFT_ERR;
        }
        // Note: reception of SPDU request (in pzProvider->zRequestSPDU) has already been performed in ReadSpduFromSlave()
        else
        {
            /* Execute SafetyProvider */
            byUasRetval = byUAS_ExecuteSafetyProvider(wInstanceCount, pzInstance->dwHandle);
        }

        if (UAS_OK == byUasRetval)
        {
            UAM_S_DoLog_UInt(UAM_S_LOG_DEBUG, "byUAS_ExecuteSafetyProvider() succeeded, HDL = ", pzInstance->dwHandle);
        } /* if */
        else
        {
            UAM_S_DoLog_UHex32(UAM_S_LOG_ERROR, "byUAS_ExecuteSafetyProvider( ) RET = ", byUasRetval);
        }

        /* Set ResponseSPDU */

        if (UAS_OK == byUasRetval)
        {
            UAM_S_Size sLen = 0;
            EncodeSpduReponse (aEncodingBuffer, UAS_ENCODING_BUFFER_SIZE, &sLen, pzConfig, &pzInstance->zResponseSPDU);
            if (sLen > 0)
            {
                UAM_S2NS_SendSpduImpl (pzConfig->dwSessionId, aEncodingBuffer, sLen);
                UAM_S_DoLog_UInt(UAM_S_LOG_DEBUG, "UAM_SetResponseSPDU succeeded:", pzInstance->dwHandle);
//                LOG_Data(LOG_DEBUG, "   SafetyData      ", pzInstance->wSafetyDataLength,
//                         pzInstance->zResponseSPDU.pbySerializedSafetyData);
                UAM_S_DoLog_UHex32(UAM_S_LOG_DEBUG, "   Flags            = 0x%02X", pzInstance->zResponseSPDU.byFlags);
                UAM_S_DoLog_UHex32(UAM_S_LOG_DEBUG, "   SpduId1           = ", pzInstance->zResponseSPDU.zSpduId.dwPart1);
                UAM_S_DoLog_UHex32(UAM_S_LOG_DEBUG, "   SpduId2           = ", pzInstance->zResponseSPDU.zSpduId.dwPart2);
                UAM_S_DoLog_UHex32(UAM_S_LOG_DEBUG, "   SpduId3           = ", pzInstance->zResponseSPDU.zSpduId.dwPart3);
                UAM_S_DoLog_UHex32(UAM_S_LOG_DEBUG, "   SafetyConsumerId = ", pzInstance->zResponseSPDU.dwSafetyConsumerId);
                UAM_S_DoLog_UHex32(UAM_S_LOG_DEBUG, "   MonitoringNumber = ", pzInstance->zResponseSPDU.dwMonitoringNumber);
                UAM_S_DoLog_UHex32(UAM_S_LOG_DEBUG, "   Crc              = ", pzInstance->zResponseSPDU.dwCrc);
//                LOG_Data(LOG_DEBUG, "   NonSafetyData   ", pzInstance->wNonSafetyDataLength,
//                         pzInstance->zResponseSPDU.pbySerializedNonSafetyData);
            }
            else
            {
                UAM_S_DoLog_UInt(UAM_S_LOG_ERROR, "UAM_SetResponseSPDU() failed for HDL = ", pzInstance->dwHandle);
            }
        }
    } /* for wNumberOfSafetyProviders */
    UAM_S_DoLog(UAM_S_LOG_SEQUENCE, "ExecuteSafetyProviders-Out");
    return byUasRetval;
}

/*===========================================================================*/
UAS_SafetyProvider_type* UAM_S_GetProvider(const UAM_S_ProviderHandle hHandle)
{
    UAM_S_LIBS_ASSERT(uamDynamicSafetyData.bInitialized);
    UAM_S_LIBS_ASSERT(uamDynamicSafetyData.bLocked);
    UAS_SafetyProvider_type* pzResult = NULL;
    if (hHandle < uamDynamicSafetyData.bNextProviderFreeHandle)
    {
        pzResult = &azUAS_SafetyProviders[hHandle];
    }
    return pzResult;
}
/*===========================================================================*/
UAS_SafetyConsumer_type* UAM_S_GetConsumer(const UAM_S_ConsumerHandle hHandle)
{
    UAM_S_LIBS_ASSERT(uamDynamicSafetyData.bInitialized);
    UAS_SafetyConsumer_type* pzResult = NULL;
    if (hHandle < uamDynamicSafetyData.bNextConsumerFreeHandle)
    {
        pzResult = &azUAS_SafetyConsumers[hHandle];
    }
    return pzResult;
}

// TODO : replace printf by a deported LOG feature
#include <stdio.h>

/*===========================================================================*/
static const char* log_prefix(const UAM_S_LOG_LEVEL level)
{
    static const char * const prefixes  [] = {"[XX]", "[EE]", "[WW]", "[II]", "[DD]", "[AA]"};
    static const UAS_UInt32 prefixesLen = sizeof(prefixes)/ sizeof(*prefixes);
    return (level < prefixesLen ? prefixes[level] : "[??]");
}

/*===========================================================================*/
void UAM_S_DoLog(const UAM_S_LOG_LEVEL level, const char* txt)
{
    // TODO: use a channel to NON SAFE! Create a function UAM_S2NS_SendLogData (const char* ptext);
    // TODO : ensure the log limit size on each text line!
    // TOOD: should not  use either snprintf or such function!
    if (txt != NULL && level <= gLogLevel)
    {
        printf("%s%s\n",log_prefix (level), txt);
    }
}

/*===========================================================================*/
void UAM_S_DoLog_UHex32 (const UAM_S_LOG_LEVEL level, const char* txt, const UAS_UInt32 u32)
{
    if (txt != NULL && level <= gLogLevel)
    {
        printf("%s%s:0x%08X\n",log_prefix (level), txt, (unsigned) u32);
    }
}

/*===========================================================================*/
void UAM_S_DoLog_UInt (const UAM_S_LOG_LEVEL level, const char* txt, const UAS_UInt64 u64)
{
    if (txt != NULL && level <= gLogLevel)
    {
        printf("%s%s:%lu\n",log_prefix (level), txt, (unsigned long) u64);
    }
}

/*===========================================================================*/
void UAM_S_DoLog_Int (const UAM_S_LOG_LEVEL level, const char* txt, const UAS_Int64 s64)
{
    if (txt != NULL && level <= gLogLevel)
    {
    printf("%s%s:%ld\n",log_prefix (level), txt, (long int) s64);
    }
}

/*===========================================================================*/
void UAM_S_DoLog_Text (const UAM_S_LOG_LEVEL level, const char* txt, const char* ptxt)
{
    if (txt != NULL && level <= gLogLevel)
    {
        printf("%s%s:%s\n",log_prefix (level), txt, (ptxt == NULL ? "NULL": ptxt) );
    }
}
/*===========================================================================*/
void UAM_S_DoLog_Pointer (const UAM_S_LOG_LEVEL level, const char* txt, const void* pAddr)
{
    if (txt != NULL && level <= gLogLevel)
    {
        printf("%s%s:%p\n",log_prefix (level), txt, pAddr );
    }

}
