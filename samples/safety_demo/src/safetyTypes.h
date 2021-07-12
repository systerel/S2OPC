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

/** \file Provides a simple example of use of UAM mapper
 *
 * GetSource and SetTarget callback will get and set their values in the cache.
 */

#ifndef SOPC_SAFETY_DEMO_SAFETYTYPES_H_
#define SOPC_SAFETY_DEMO_SAFETYTYPES_H_

#include "sopc_builtintypes.h"
#include "sopc_common.h"

#include "uam.h"
#include "uas.h"

// Note : this example supposes that CONS and PROV use the same endianness!
typedef struct
{
    uint8_t u8Val1;
    uint8_t u8Val2;
    bool bData1;
    bool bData2;
    uint32_t u32Cnt1;
    uint32_t u32Cnt2;
    char sText10[10];
} SafetyDemo_Sample_Safe1_type;

/** Note : using a temporary union to convert address is required, so as to avoid getting warning
 * on conversion from VOID* to STRUCT*
 */
typedef union {
    SafetyDemo_Sample_Safe1_type* pzType;
    void* pzVoid;
} SafetyDemo_Sample_Safe1_union;

typedef struct
{
    char sText10[30];
} SafetyDemo_Sample_NonSafe1_type;

#define SAMPLE1_SAFETY_DATA_LEN (sizeof(SafetyDemo_Sample_Safe1_type))
#define SAMPLE1_UNSAFE_DATA_LEN (sizeof(SafetyDemo_Sample_NonSafe1_type))

#define SAMPLE_CONSUMER_HDL_TAG (1 << 24)
#define SAMPLE_PROVIDER_HDL_TAG (2 << 24)
/////////////////////////////////////////////////////////
//// PROVIDER 1 SAMPLES
/////////////////////////////////////////////////////////
#define SAMPLE_PROVID1_TIMEOUT_MS 1000
#define SAMPLE_PROVID1_ID 0x10001234u
#define SAMPLE_PROVID1_SIGN 0x5194A101
#define SAMPLE_PROVID1_GUID                             \
    {                                                   \
        0x11111111u, 0x22222222, 0x33333333, 0x44444444 \
    }
#define SAMPLE_PROVID1_HANDLE (SAMPLE_PROVIDER_HDL_TAG + 1)

/////////////////////////////////////////////////////////
//// CONSUMER 1 SAMPLES
/////////////////////////////////////////////////////////
#define SAMPLE_CONSUM1_ID 0x20001222u
#define SAMPLE_CONSUM1_SIGN 0xBBBBBBBBu
#define SAMPLE_CONSUM1_GUID                             \
    {                                                   \
        0x55555555u, 0x66666666, 0xAAAAAAAA, 0xBCDEFA12 \
    }
#define SAMPLE_CONSUM1_HANDLE (SAMPLE_CONSUMER_HDL_TAG + 1)

/////////////////////////////////////////////////////////
//// DEFINITIONS OF EXTERNAL SERVICES
/////////////////////////////////////////////////////////

SOPC_ReturnStatus SafetyTypes_Create_ProviderSample(void);
SOPC_ReturnStatus SafetyTypes_Create_ConsumerSample(void);

/** Signals an acknowledgement */
void SafetyTypes_DoAck(void);
void SafetyTypes_ClearAck(void);
/* Checks for an acknwoledgement, \see SafetyTypes_DoAck */
bool SafetyTypes_hasAcknowledgement(void);

/** Switch enable flag (CONSUMER only) */
void SafetyTypes_SwitchEnableFlag(void);
bool SafetyTypes_GetEnableFlag(void);

void SafetyTypes_SetSafetyDataB1(const bool b1Val);
void SafetyTypes_SetSafetyDataB2(const bool b2Val);
void SafetyTypes_SetSafetyDataV1(const uint8_t v1Val);
void SafetyTypes_SetSafetyDataV2(const uint8_t v2Val);
void SafetyTypes_SetSafetyDataTxt(const char* text);
#endif /* SOPC_SAFETY_DEMO_SAFETYTYPES_H_ */
