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

/*--------------------------------------------------------------------------*/
/******************************* I M P O R T ********************************/
/*--------------------------------------------------------------------------*/

#include "safetyDemo.h"
#include "config.h"

#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "sopc_mem_alloc.h"
#include "sopc_time.h"
#include "uas_def.h"

/*-------------*/
/*  T Y P E S  */
/*-------------*/

/*---------------------*/
/*  V A R I A B L E S  */
/*---------------------*/
static bool bHasAcknowledgement = false;
static bool bIsEnabledFlag = true;
static bool bSafetyDataB1;
static bool bSafetyDataB2;
static uint8_t bSafetyDataV1;
static uint8_t bSafetyDataV2;
static char sSafetyDataText10[10] = {0};

static const UAS_SafetyProviderSPI_type SPI1P_Sample = {.dwSafetyProviderId = SAMPLE_PROVID1_ID,
                                                        .zSafetyBaseId = SAMPLE_PROVID1_GUID,
                                                        .dwSafetyStructureSignature = SAMPLE_PROVID1_SIGN};

static const UAS_SafetyConsumerSPI_type SPI1C_Sample = {.dwSafetyProviderId = SAMPLE_PROVID1_ID,
                                                        .zSafetyBaseId = SAMPLE_PROVID1_GUID,
                                                        .dwSafetyConsumerId = SAMPLE_CONSUM1_HANDLE,
                                                        .bySafetyProviderLevel = UASDEF_SAFETY_LEVEL,
                                                        .dwSafetyStructureSignature = SAMPLE_PROVID1_SIGN,
                                                        .dwSafetyConsumerTimeout = SAMPLE_PROVID1_TIMEOUT_MS,
                                                        .bSafetyOperatorAckNecessary = false,
                                                        .wSafetyErrorIntervalLimit = 60};
static UAM_ProviderHandle hProviderHandle = UAM_NoHandle;
static UAM_ConsumerHandle hConsumerHandle = UAM_NoHandle;

/*---------------------*/
/*  F U N C T I O N S  */
/*---------------------*/
static bool fProvider1SampleCycle(const UAM_SafetyConfiguration_type* const pzConfiguration,
                                  const UAM_ProviderSAPI_Input* pzAppInputs,
                                  UAM_ProviderSAPI_Output* pzAppOutputs);
static bool fConsumer1SampleCycle(const UAM_SafetyConfiguration_type* const pzConfiguration,
                                  const UAM_ConsumerSAPI_Input* pzAppInputs,
                                  UAM_ConsumerSAPI_Output* pzAppOutputs);

/*------------------------------*/
/*  I M P L E ME N T A T I O N  */
/*------------------------------*/

/** Example of PROVIDER behavior */
static bool fProvider1SampleCycle(const UAM_SafetyConfiguration_type* const pzConfiguration,
                                  const UAM_ProviderSAPI_Input* pzAppInputs,
                                  UAM_ProviderSAPI_Output* pzAppOutputs)
{
    assert(pzConfiguration != NULL);
    assert(pzAppInputs != NULL);
    assert(pzAppOutputs != NULL);

    // Do some simulation stuff...
    static UAS_UInt32 uDemoCounter = 0;
    static SafetyDemo_Sample_Safe1_type safeData = {0, 0, 0, 0, 0, 0, {0}};
    uDemoCounter++;
    if (safeData.u8Val1 < 100)
    {
        safeData.u8Val1++;
    }
    else
    {
        safeData.u8Val1 = 10;
    }
    safeData.bData1 = bSafetyDataB1;
    safeData.bData2 = bSafetyDataB2;
    safeData.u8Val1 = bSafetyDataV1;
    safeData.u8Val2 = bSafetyDataV2;
    memcpy(safeData.sText10, sSafetyDataText10, 10);

    // Set output flags
    pzAppOutputs->bOperatorAckProvider = SafetyDemo_hasAcknowledgement();

    // Set Output Non Safe data
    assert(pzAppOutputs->pbySerializedNonSafetyData != NULL);
    memset(pzAppOutputs->pbySerializedNonSafetyData, 0, pzConfiguration->wNonSafetyDataLength);
    snprintf((char*) pzAppOutputs->pbySerializedNonSafetyData, pzConfiguration->wSafetyDataLength, "NonSafety Data %u",
             pzAppInputs->dwMonitoringNumber);

    // Set Output Safe data
    assert(pzAppOutputs->pbySerializedSafetyData != NULL);
    assert(sizeof(safeData) == pzConfiguration->wSafetyDataLength);
    memcpy(pzAppOutputs->pbySerializedSafetyData, (void*) &safeData, sizeof(safeData));

    return true;
}

/** Example of CONSUMER behavior */
static bool fConsumer1SampleCycle(const UAM_SafetyConfiguration_type* const pzConfiguration,
                                  const UAM_ConsumerSAPI_Input* pzAppInputs,
                                  UAM_ConsumerSAPI_Output* pzAppOutputs)
{
    assert(pzConfiguration != NULL);
    assert(pzAppInputs != NULL);
    assert(pzAppInputs->pbySerializedSafetyData != NULL);
    assert(pzAppOutputs != NULL);
    SafetyDemo_Sample_Safe1_union safeUnion = {.pzVoid = pzAppInputs->pbySerializedSafetyData};

    safeUnion.pzVoid = pzAppInputs->pbySerializedSafetyData;

    if (!pzAppInputs->bOperatorAckRequested && pzAppOutputs->bOperatorAckConsumer)
    {
        SafetyDemo_ClearAck();
    }
    pzAppOutputs->bOperatorAckConsumer = SafetyDemo_hasAcknowledgement();
    pzAppOutputs->bEnable = SafetyDemo_GetEnableFlag();

    printf("\033[17;1H");
    printf("Safe Data = [b1=%d,b2=%d,v1=%02X, v2=%02X,... , txt=<%s>]\033[0K\n", safeUnion.pzType->bData1,
           safeUnion.pzType->bData2, safeUnion.pzType->u8Val1, safeUnion.pzType->u8Val2, safeUnion.pzType->sText10);

    return true;
}

void SafetyDemo_DoAck(void)
{
    bHasAcknowledgement = true;
}

void SafetyDemo_ClearAck(void)
{
    bHasAcknowledgement = false;
}
bool SafetyDemo_hasAcknowledgement(void)
{
    return bHasAcknowledgement;
}

void SafetyDemo_SwitchEnableFlag(void)
{
    bIsEnabledFlag = !bIsEnabledFlag;
}

void SafetyDemo_SetSafetyDataB1(const bool b1Val)
{
    bSafetyDataB1 = b1Val;
}

void SafetyDemo_SetSafetyDataB2(const bool b2Val)
{
    bSafetyDataB2 = b2Val;
}
void SafetyDemo_SetSafetyDataV1(const uint8_t v1Val)
{
    bSafetyDataV1 = v1Val;
}

void SafetyDemo_SetSafetyDataV2(const uint8_t v2Val)
{
    bSafetyDataV2 = v2Val;
}
void SafetyDemo_SetSafetyDataTxt(const char* text)
{
    memcpy(sSafetyDataText10, text, 10);
}

bool SafetyDemo_GetEnableFlag(void)
{
    return bIsEnabledFlag;
}

static const UAM_SafetyConfiguration_type yInstanceConfigSample1 = {.dwRequestHandle = NODEID_SPDU_REQUEST_NUM,
                                                                    .dwResponseHandle = NODEID_SPDU_RESPONSE_NUM,
                                                                    .wSafetyDataLength = SAMPLE1_SAFETY_DATA_LEN,
                                                                    .wNonSafetyDataLength = SAMPLE1_UNSAFE_DATA_LEN};

SOPC_ReturnStatus SafetyDemo_Create_ProviderSample(void)
{
    SOPC_ReturnStatus byUasRetval;
    UAM_SafetyConfiguration_type zInstanceConfiguration = yInstanceConfigSample1;
    const UAS_SafetyProviderSPI_type zSPI = SPI1P_Sample;

    UAM_Initialize();

    byUasRetval = UAM_InitSafetyProvider(&zInstanceConfiguration, &zSPI, &fProvider1SampleCycle, &hProviderHandle);
    if (SOPC_STATUS_OK == byUasRetval)
    {
        LOG_Trace(LOG_INFO, "byUAS_InitSafetyProvider( handle = 0x%08X ) succeeded", hProviderHandle);
        SafetyDemo_SetSafetyDataTxt ("Hello!");
    } /* if */
    else
    {
        LOG_Trace(LOG_ERROR, "byUAS_InitSafetyProvider( handle ) failed, error code = 0x%02X", hProviderHandle,
                  byUasRetval);
    } /* else */
    return byUasRetval;
}

SOPC_ReturnStatus SafetyDemo_Create_ConsumerSample(void)
{
    SOPC_ReturnStatus byUasRetval;
    UAM_SafetyConfiguration_type zInstanceConfiguration = yInstanceConfigSample1;
    const UAS_SafetyConsumerSPI_type zSPI = SPI1C_Sample;

    UAM_Initialize();

    byUasRetval = UAM_InitSafetyConsumer(&zInstanceConfiguration, &zSPI, &fConsumer1SampleCycle, &hConsumerHandle);
    if (SOPC_STATUS_OK == byUasRetval)
    {
        LOG_Trace(LOG_INFO, "UAM_InitSafetyConsumer( handle = 0x%08X ) succeeded", hConsumerHandle);
        UAS_SafetyConsumer_type* pzInstance = UAM_GetConsumer(hConsumerHandle);
        assert(pzInstance != NULL);
        memset(&pzInstance->zInputSAPI, 0, sizeof(pzInstance->zInputSAPI));
        pzInstance->zInputSAPI.bEnable = 1;

    } /* if */
    else
    {
        LOG_Trace(LOG_ERROR, "UAM_InitSafetyConsumer( handle ) failed, error code = 0x%02X", hConsumerHandle,
                  byUasRetval);
    } /* else */
    return byUasRetval;
}
