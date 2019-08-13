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

#ifndef P_ANALYZER_H
#define P_ANALYZER_H

#include <inttypes.h> /* stdlib includes */
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "p_channel.h"

#define PLT_SOF (0x01)
#define PLT_EOF (0x03)
#define PLT_DLM (0x10)
#define PLT_MASK_DELIM_ON (0x80)
#define PLT_MASK_DELIM_OFF (0x7F)
#define PLT_POLY (0x8005)
#define PLT_MAX_LENGTH (256)
#define CRC_BITS_PER_TRANSPORT_BYTE (8)
#define CRC_REAL_BITS_LENGTH (16)
#define CRC_NB_TRANSPORT_BYTES (2)
#define LNG_BITS_PER_TRANSPORT_BYTE (8)
#define LNG_REAL_BITS_LENGTH (16)
#define LNG_NB_TRANSPORT_BYTES (2)

typedef enum E_ANALYZER_STATUS
{
    ANALYZER_STATUS_WAIT_FOR_SOF,
    ANALYZER_STATUS_WAIT_FOR_TAG,
    ANALYZER_STATUS_WAIT_FOR_DELIM_TAG,
    ANALYZER_STATUS_WAIT_FOR_LENGTH,
    ANALYZER_STATUS_WAIT_FOR_DELIM_LENGTH,
    ANALYZER_STATUS_WAIT_FOR_DATA,
    ANALYZER_STATUS_WAIT_FOR_DELIM_DATA,
    ANALYZER_STATUS_WAIT_FOR_CRC,
    ANALYZER_STATUS_WAIT_FOR_DELIM_CRC,
    ANALYZER_STATUS_WAIT_FOR_EOF,
    ANALYZER_STATUS_READY,
    ANALYZER_STATUS_MAX,
    ANALYZER_STATUS_SIZEOF = UINT32_MAX
} eAnalyzerStatus;

typedef enum E_ANALYZER_RESULT
{
    ANALYZER_RESULT_OK,
    ANALYZER_RESULT_NOK
} eAnalyzerResult;

typedef struct T_ANALYZER_MSG
{
    uint8_t tag;
    uint16_t length;
    uint8_t data[PLT_MAX_LENGTH];
} __attribute__((packed)) tAnalyzerMsg;

typedef struct T_ANALYZER_WKS
{
    eAnalyzerStatus status;
    uint16_t receivedCrc;
    uint16_t computedCrc;
    uint16_t currentFieldLength;
    uint16_t cptTimeout;
    uint16_t trigTmoCpt;
    tAnalyzerMsg currentMsg;
    tChannel inputMessages;
} tAnalyzerWks;

tAnalyzerWks* CreateAnalyzer(uint16_t nbPendingMessages, uint16_t trigTmoCpt);
void DestroyAnalyzer(tAnalyzerWks** ppCtx);
eAnalyzerResult ExecuteAnalyzer(tAnalyzerWks* pCtx, const uint8_t* buffer, uint16_t length);
eAnalyzerResult UpdateAnalyzerTmo(tAnalyzerWks* pCtx);

#endif
