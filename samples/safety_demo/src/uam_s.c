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
#include "uas.h"

#include "assert.h"

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

/*============================================================================
 * DECLARATION OF INTERNAL SERVICES
 *===========================================================================*/
/**
 * Read the SPDURequest received on a provider and copy it to pzProvider->zRequestSPDU
 * \param[inout] pzProvider A non-NULL pointer to a provider configuration.
 *          Reads dwRequestHandle to get the SPDU content and updates zRequestSPDU
 */
static bool Get_SPDU_Request(UAS_SafetyProvider_type* pzProvider);

static void ExecuteSafetyProviders(void);
static void ExecuteSafetyConsumers(void);

/*============================================================================
 * IMPLEMENTATION OF INTERNAL SERVICES
 *===========================================================================*/
static bool Get_SPDU_Request(UAS_SafetyProvider_type* pzProvider)
 {
     assert(NULL != pzProvider);
     assert(pzProvider->dwHandle < UASDEF_MAX_SAFETYPROVIDERS);
     bool bResult = false;
     // TODO @BEM : replace UAM_SpduEncoder by request received from NON SAFE
     /* const UAM_SafetyConfiguration_type* pzSafetyCfg =
         &uamDynamicSafetyData.azProviderConfiguration[pzProvider->dwHandle];
     UAS_RequestSpdu_type zSpdu;

     bResult = UAM_SpduEncoder_GetRequest(pzSafetyCfg->dwRequestHandle, &zSpdu);
     if (bResult == SOPC_STATUS_OK)
     {
         pzProvider->zRequestSPDU = zSpdu;

         bResult = SOPC_STATUS_OK;
     }*/
     return bResult;
 }

/*===========================================================================*/
static bool Get_SPDU_Response(UAS_SafetyConsumer_type* pzConsumer,
        UAS_UInt32* puSafeSize,
        UAS_UInt32* puNonSafeSize)
{
    assert(NULL != pzConsumer);
    assert(pzConsumer->dwHandle < UASDEF_MAX_SAFETYCONSUMERS);
    bool bResult = false;
    (void)puSafeSize;
    (void)puNonSafeSize;
    // TODO @BEM : replace UAM_SpduEncoder_GetResponse by response received from NON SAFE
    /*
    const UAM_SafetyConfiguration_type* pzSafetyCfg =
        &uamDynamicSafetyData.azConsumerConfiguration[pzConsumer->dwHandle];

    bResult = UAM_SpduEncoder_GetResponse(pzSafetyCfg->dwResponseHandle, &pzConsumer->zResponseSPDU, puSafeSize,
                                          puNonSafeSize);
*/
    return bResult;
}


/*============================================================================
 * IMPLEMENTATION OF EXTERNAL SERVICES
 *===========================================================================*/

/*===========================================================================*/
void UAM_S_Initialize(void)
{
    assert(!uamDynamicSafetyData.bInitialized);
    UAS_UInt16 index = 0;

    UAM_S_LIBS_HEAP_Init (&zHeap);
    UAM_S_LIBS_MemZero(azUAS_SafetyProviders, sizeof(azUAS_SafetyProviders));
    uamDynamicSafetyData.bNextProviderFreeHandle = 0;
    uamDynamicSafetyData.bNextConsumerFreeHandle = 0;

    for (index = 0; index < UASDEF_MAX_SAFETYPROVIDERS; ++index)
    {
        UAM_SafetyConfiguration_type* pzConf = &uamDynamicSafetyData.azProviderConfiguration[index];
        pzConf->dwRequestHandle = UAM_NoHandle;
        pzConf->dwResponseHandle = UAM_NoHandle;
        pzConf->wNonSafetyDataLength = 0;
        pzConf->wSafetyDataLength = 0;
        uamDynamicSafetyData.apfProviderCycle[index] = NULL;
    }
    for (index = 0; index < UASDEF_MAX_SAFETYCONSUMERS; ++index)
    {
        UAM_SafetyConfiguration_type* pzConf = &uamDynamicSafetyData.azConsumerConfiguration[index];
        pzConf->dwRequestHandle = UAM_NoHandle;
        pzConf->dwResponseHandle = UAM_NoHandle;
        pzConf->wNonSafetyDataLength = 0;
        pzConf->wSafetyDataLength = 0;
        uamDynamicSafetyData.apfConsumerCycle[index] = NULL;
    }
    uamDynamicSafetyData.bInitialized = true;
    uamDynamicSafetyData.bLocked = false;
}

/*===========================================================================*/
UAS_UInt8 UAM_S_InitSafetyProvider(const UAM_SafetyConfiguration_type* const pzInstanceConfiguration,
                                         const UAS_SafetyProviderSPI_type* const pzSPI,
                                         UAM_S_pfProviderApplicationCycle pfProviderCycle,
                                         UAM_S_ProviderHandle* phHandle)
{
    assert(uamDynamicSafetyData.bInitialized);
    assert(!uamDynamicSafetyData.bLocked);
    assert(NULL != pzSPI);
    assert(NULL != pfProviderCycle);
    assert(NULL != pzInstanceConfiguration);
    assert(NULL != phHandle);
    assert(uamDynamicSafetyData.bNextProviderFreeHandle < UASDEF_MAX_SAFETYPROVIDERS);

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
    assert(NULL != pzSafetyProvider->zInputSAPI.pbySerializedNonSafetyData);
    assert(NULL != pzSafetyProvider->zInputSAPI.pbySerializedSafetyData);
    assert(NULL != pzSafetyProvider->zResponseSPDU.pbySerializedNonSafetyData);
    assert(NULL != pzSafetyProvider->zResponseSPDU.pbySerializedSafetyData);

    uamDynamicSafetyData.azProviderConfiguration[handle] = *pzInstanceConfiguration;
    uamDynamicSafetyData.apfProviderCycle[handle] = pfProviderCycle;

    pzSafetyProvider->dwHandle = handle;
    pzSafetyProvider->wSafetyDataLength = pzInstanceConfiguration->wSafetyDataLength;
    pzSafetyProvider->wNonSafetyDataLength = pzInstanceConfiguration->wNonSafetyDataLength;
    pzSafetyProvider->zSPI = *pzSPI;

    byRetVal = byUAS_InitSafetyProvider(pzSafetyProvider, handle, &nResult);
    return byRetVal;
}

/*===========================================================================*/
UAS_UInt8 UAM_S_InitSafetyConsumer(const UAM_SafetyConfiguration_type* const pzInstanceConfiguration,
                                         const UAS_SafetyConsumerSPI_type* const pzSPI,
                                         UAM_S_pfConsumerApplicationCycle pfConsumerCycle,
                                         UAM_S_ProviderHandle* phHandle)
{
    assert(uamDynamicSafetyData.bInitialized);
    assert(!uamDynamicSafetyData.bLocked);
    assert(NULL != pzSPI);
    assert(NULL != pfConsumerCycle);
    assert(NULL != pzInstanceConfiguration);
    assert(NULL != phHandle);
    assert(uamDynamicSafetyData.bNextConsumerFreeHandle < UASDEF_MAX_SAFETYCONSUMERS);

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
    assert(NULL != pzSafetyProvider->zResponseSPDU.pbySerializedNonSafetyData);
    assert(NULL != pzSafetyProvider->zResponseSPDU.pbySerializedSafetyData);
    assert(NULL != pzSafetyProvider->zOutputSAPI.pbySerializedSafetyData);
    assert(NULL != pzSafetyProvider->zOutputSAPI.pbySerializedNonSafetyData);

    uamDynamicSafetyData.azConsumerConfiguration[handle] = *pzInstanceConfiguration;
    uamDynamicSafetyData.apfConsumerCycle[handle] = pfConsumerCycle;

    pzSafetyProvider->dwHandle = handle;
    pzSafetyProvider->wSafetyDataLength = pzInstanceConfiguration->wSafetyDataLength;
    pzSafetyProvider->wNonSafetyDataLength = pzInstanceConfiguration->wNonSafetyDataLength;
    pzSafetyProvider->zSPI = *pzSPI;

    byRetVal = byUAS_InitSafetyConsumer(pzSafetyProvider, handle, &nResult);
    return byRetVal;
}
/*===========================================================================*/
UAS_UInt8 UAM_S_StartSafety(void)
{
    assert(uamDynamicSafetyData.bInitialized);
    assert(!uamDynamicSafetyData.bLocked);
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
    return byRetVal;
}
/*===========================================================================*/
UAS_UInt8 UAM_S_Cycle(void)
{
    UAS_UInt8 byRetVal = UAS_OK;
    assert(uamDynamicSafetyData.bInitialized);
    assert(uamDynamicSafetyData.bLocked);

    ExecuteSafetyProviders();

    ExecuteSafetyConsumers();

    return byRetVal;
}

/*===========================================================================*/
void UAM_S_Clear(void)
{
    UAS_UInt16 index = 0;
    for (index = 0; index < uamDynamicSafetyData.bNextProviderFreeHandle; ++index)
    {
        UAS_SafetyProvider_type* pzSafetyProvider = &(azUAS_SafetyProviders[index]);
        assert(pzSafetyProvider->zInputSAPI.pbySerializedSafetyData != NULL);
        assert(pzSafetyProvider->zInputSAPI.pbySerializedNonSafetyData != NULL);
        assert(pzSafetyProvider->zResponseSPDU.pbySerializedSafetyData != NULL);
        assert(pzSafetyProvider->zResponseSPDU.pbySerializedNonSafetyData != NULL);
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
        assert(pzSafetyConsumer->zOutputSAPI.pbySerializedSafetyData != NULL);
        assert(pzSafetyConsumer->zOutputSAPI.pbySerializedNonSafetyData != NULL);
        assert(pzSafetyConsumer->zResponseSPDU.pbySerializedSafetyData != NULL);
        assert(pzSafetyConsumer->zResponseSPDU.pbySerializedNonSafetyData != NULL);
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
}

/*===========================================================================*/
static void ExecuteSafetyConsumers(void)
{
    UAS_UInt8 byUasRetval = UAS_OK;
    bool bStatus = true;
    UAS_UInt16 wInstanceCount = 0u;

    for (wInstanceCount = 0u; wInstanceCount < uamDynamicSafetyData.bNextConsumerFreeHandle; wInstanceCount++)
    {
        assert(NULL != uamDynamicSafetyData.apfConsumerCycle);
        UAS_SafetyConsumer_type* pzInstance = &azUAS_SafetyConsumers[wInstanceCount];
        UAM_SafetyConfiguration_type* pzConfig = &uamDynamicSafetyData.azConsumerConfiguration[wInstanceCount];

        /* Get ResponseSPDU */
        bStatus = Get_SPDU_Response(pzInstance, NULL, NULL);
        if (bStatus)
        {
            UAM_S_DoLog_UInt32(UAM_S_LOG_WARN, "Get_SPDU_Response failed for Consumer #\n", wInstanceCount);
        }
        else
        {
            UAM_S_DoLog_UInt32(UAM_S_LOG_INFO, "Get_SPDU_Response succeeded HDL=:", pzInstance->dwHandle);
//            LOG_Data(LOG_DEBUG, "   SafetyData      ", pzInstance->wSafetyDataLength,
//                     pzInstance->zResponseSPDU.pbySerializedSafetyData);
            UAM_S_DoLog_UHex32(UAM_S_LOG_DEBUG, "   Flags            = ", pzInstance->zResponseSPDU.byFlags);
            UAM_S_DoLog_UHex32(UAM_S_LOG_DEBUG, "   SpduId1          = ", pzInstance->zResponseSPDU.zSpduId.dwPart1);
            UAM_S_DoLog_UHex32(UAM_S_LOG_DEBUG, "   SpduId2          = ", pzInstance->zResponseSPDU.zSpduId.dwPart2);
            UAM_S_DoLog_UHex32(UAM_S_LOG_DEBUG, "   SpduId3          = ", pzInstance->zResponseSPDU.zSpduId.dwPart3);
            UAM_S_DoLog_UHex32(UAM_S_LOG_DEBUG, "   SafetyConsumerId = ", pzInstance->zResponseSPDU.dwSafetyConsumerId);
            UAM_S_DoLog_UHex32(UAM_S_LOG_DEBUG, "   MonitoringNumber = ", pzInstance->zResponseSPDU.dwMonitoringNumber);
            UAM_S_DoLog_UHex32(UAM_S_LOG_DEBUG, "   Crc              = ", pzInstance->zResponseSPDU.dwCrc);
//            LOG_Trace(LOG_DEBUG, "   NonSafetyData    =");
//            LOG_Data(LOG_DEBUG, "   NonSafetyData   ", pzInstance->wNonSafetyDataLength,
//                     pzInstance->zResponseSPDU.pbySerializedNonSafetyData);
        }
        pzInstance->bCommDone = 1u;

        pzInstance->bAppDone =
            (*uamDynamicSafetyData.apfConsumerCycle)(pzConfig, &pzInstance->zOutputSAPI, &pzInstance->zInputSAPI);

        /* Execute SafetyConsumer */
        if (bStatus)
        {
            byUasRetval = byUAS_ExecuteSafetyConsumer(wInstanceCount, pzInstance->dwHandle);
            if (UAS_OK == byUasRetval)
            {
            } /* if */
            else
            {
                UAM_S_DoLog_UInt32(UAM_S_LOG_ERROR, "byUAS_ExecuteSafetyConsumer() failed for HDL = ", pzInstance->dwHandle);
                UAM_S_DoLog_UHex32(UAM_S_LOG_ERROR, "byUAS_ExecuteSafetyConsumer() code was = ", byUasRetval);
            }
        }

        /* Set RequestSPDU */
        if (bStatus)
        {
            // TODO @ BEM : replace by REQUEST encoding and sending to NONSAFE
            // status = UAM_SpduEncoder_SetRequest(pzConfig->dwRequestHandle, &pzInstance->zRequestSPDU);
            if (bStatus)
            {
                UAM_S_DoLog_UInt32(UAM_S_LOG_DEBUG, "UAM_SetRequestSPDU HDL =", pzInstance->dwHandle);
                UAM_S_DoLog_UHex32(UAM_S_LOG_DEBUG, "   SafetyConsumerId = ", pzInstance->zRequestSPDU.dwSafetyConsumerId);
                UAM_S_DoLog_UHex32(UAM_S_LOG_DEBUG, "   MonitoringNumber = ", pzInstance->zRequestSPDU.dwMonitoringNumber);
                UAM_S_DoLog_UHex32(UAM_S_LOG_DEBUG, "   Flags            = ", pzInstance->zRequestSPDU.byFlags);
            } /* if */
            else
            {
                UAM_S_DoLog_UInt32(UAM_S_LOG_ERROR, "UAM_SetRequestSPDU() failed for HDL = ", pzInstance->dwHandle);
            }
        }
    } /* for wNumberOfSafetyConsumers */
}

/*===========================================================================*/
static void ExecuteSafetyProviders(void)
{
    UAS_UInt8 byUasRetval;
    bool bStatus = true;
    UAS_UInt16 wInstanceCount = 0u;

    for (wInstanceCount = 0u; wInstanceCount < uamDynamicSafetyData.bNextProviderFreeHandle; wInstanceCount++)
    {
        assert(NULL != uamDynamicSafetyData.apfProviderCycle);
        UAS_SafetyProvider_type* pzInstance = &azUAS_SafetyProviders[wInstanceCount];
        UAM_SafetyConfiguration_type* pzConfig = &uamDynamicSafetyData.azProviderConfiguration[wInstanceCount];

        /* Execute the application */
        pzInstance->bAppDone =
            (*uamDynamicSafetyData.apfProviderCycle)(pzConfig, &pzInstance->zOutputSAPI, &pzInstance->zInputSAPI);
        if (pzInstance->bAppDone != 1u)
        {
            bStatus = false;
        }
        bStatus = Get_SPDU_Request(pzInstance);
        if (bStatus)
        {
            UAM_S_DoLog_UInt32(UAM_S_LOG_WARN, "Get_SPDU_Request failed for Provider #\n", wInstanceCount);
        }
        else
        {
            pzInstance->bCommDone = 1;
            UAM_S_DoLog_UInt32(UAM_S_LOG_INFO, "Get_SPDU_Request succeeded for HDL:", pzInstance->dwHandle);
            UAM_S_DoLog_UHex32(UAM_S_LOG_DEBUG, "   SafetyConsumerId = ", pzInstance->zRequestSPDU.dwSafetyConsumerId);
            UAM_S_DoLog_UHex32(UAM_S_LOG_DEBUG, "   MonitoringNumber = ", pzInstance->zRequestSPDU.dwMonitoringNumber);
            UAM_S_DoLog_UHex32(UAM_S_LOG_DEBUG, "   Flags            = ", pzInstance->zRequestSPDU.byFlags);
        }

        /* Execute SafetyProvider */
        byUasRetval = byUAS_ExecuteSafetyProvider(wInstanceCount, pzInstance->dwHandle);

        if (UAS_OK == byUasRetval)
        {
            UAM_S_DoLog_UInt32(UAM_S_LOG_DEBUG, "byUAS_ExecuteSafetyProvider() succeeded, HDL = ", pzInstance->dwHandle);
        } /* if */
        else
        {
            UAM_S_DoLog_UInt32(UAM_S_LOG_ERROR, "byUAS_ExecuteSafetyProvider( ) HDL = ", pzInstance->dwHandle);
        }

        /* Set ResponseSPDU */

        if (bStatus)
        {
            // TODO @ BEM : replace by RESPONSE encoding and sending to NONSAFE
//            bStatus = UAM_SpduEncoder_SetResponse(pzConfig->dwResponseHandle, &pzInstance->zResponseSPDU);
            if (bStatus)
            {
                UAM_S_DoLog_UInt32(UAM_S_LOG_DEBUG, "UAM_SetResponseSPDU succeeded:", pzInstance->dwHandle);
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
                UAM_S_DoLog_UInt32(UAM_S_LOG_ERROR, "UAM_SetResponseSPDU() failed for HDL = ", pzInstance->dwHandle);
            }
        }
    } /* for wNumberOfSafetyProviders */
}

/*===========================================================================*/
UAS_SafetyProvider_type* UAM_S_GetProvider(const UAM_S_ProviderHandle hHandle)
{
    assert(uamDynamicSafetyData.bInitialized);
    assert(uamDynamicSafetyData.bLocked);
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
    assert(uamDynamicSafetyData.bInitialized);
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
    printf("%s%s\n",log_prefix (level), txt);
}

/*===========================================================================*/
void UAM_S_DoLog_UHex32 (const UAM_S_LOG_LEVEL level, const char* txt, const UAS_UInt32 u32)
{
    if (txt != NULL)
    {
        printf("%s%s:0x%08X\n",log_prefix (level), txt, (unsigned) u32);
    }
}

/*===========================================================================*/
void UAM_S_DoLog_UInt32 (const UAM_S_LOG_LEVEL level, const char* txt, const UAS_UInt32 u32)
{
    if (txt != NULL)
    {
        printf("%s%s:%u\n",log_prefix (level), txt, (unsigned) u32);
    }
}

/*===========================================================================*/
void UAM_S_DoLog_Int32 (const UAM_S_LOG_LEVEL level, const char* txt, const UAS_Int32 s32)
{
    if (txt != NULL)
    {
    printf("%s%s:%d\n",log_prefix (level), txt, (int) s32);
    }
}

/*===========================================================================*/
void UAM_S_DoLog_Text (const UAM_S_LOG_LEVEL level, const char* txt, const char* ptxt)
{
    if (txt != NULL)
    {
        printf("%s%s:%s\n",log_prefix (level), txt, (ptxt == NULL ? "NULL": ptxt) );
    }
}
/*===========================================================================*/
void UAM_S_DoLog_Pointer (const UAM_S_LOG_LEVEL level, const char* txt, const void* pAddr)
{
    if (txt != NULL)
    {
        printf("%s%s:%p\n",log_prefix (level), txt, pAddr );
    }

}
