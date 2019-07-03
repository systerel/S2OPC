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

#ifndef P_CHANNEL_H
#define P_CHANNEL_H

#include <inttypes.h> /* stdlib includes */
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h" /* freeRtos includes */
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include "timers.h"

#include "p_utils.h"

// Channel result
typedef enum E_CHANNEL_RESULT
{
    E_CHANNEL_RESULT_OK,          // No error
    E_CHANNEL_RESULT_NOK,         // General error
    E_CHANNEL_RESULT_ERROR_FULL,  // Not enough space
    E_CHANNEL_RESULT_ERROR_EMPTY, // Empty
    E_CHANNEL_RESULT_ERROR_TMO,   // Timeout
    E_CHANNEL_RESULT_MORE_DATA
} eChannelResult;

// Channel receive mode
typedef enum E_CHANNEL_READ_MODE
{
    E_CHANNEL_RD_MODE_NORMAL,      // Normal, oldest elt is read then removed
    E_CHANNEL_RD_MODE_JUST_KEEPING // Just keeping, read without remove oldest elt.
} eChannelReadMode;

// Channel send mode
typedef enum E_CHANNEL_WRITE_MODE
{
    E_CHANNEL_WR_MODE_NORMAL,   // Normal, elt is write to channel if enough space
    E_CHANNEL_WR_MODE_OVERWRITE // Overwrited, oldest elts are removed until enough space to write new elt.
} eChannelWriteMode;

// Channel workspace
typedef struct T_CHANNEL
{
    uint16_t iWr;               // Write buffer record index
    uint16_t iRd;               // Read buffer record index
    uint16_t iWrData;           // Write buffer data index
    uint16_t iRdData;           // Read buffer data index
    uint16_t maxSizeTotalData;  // Max of cumulative data
    uint16_t maxSizeTotalElts;  // Max of cumulative record
    uint16_t maxSizeDataPerElt; // Max size for each record
    uint16_t currentNbElts;     // Current nb records
    uint16_t currentNbDatas;    // Current cumulative data for current nb records
    uint16_t overflowCpt;       // Overflow counter
    QueueHandle_t lock;         // Critical section
    QueueHandle_t isNotEmpty;   // Signal not empty used by receive timeout
    uint8_t* channelData;       // Buffer of data
    uint16_t* channelRecord;    // Buffer of record (of size for each elt).
} tChannel;

void P_CHANNEL_DeInit(tChannel* p);

eChannelResult P_CHANNEL_Init(tChannel* p,             // Channel workspace
                              size_t maxTotalDataSize, // Total datasize
                              size_t maxEltSize,       // Max atomic element size
                              size_t nbElts);          // Max elements

eChannelResult P_CHANNEL_Send(tChannel* p,               // Channel workspace
                              const uint8_t* pBuffer,    // Data to send
                              uint16_t size,             // Size of data to send
                              uint16_t* pbNbWritedBytes, // Nb bytes writed
                              eChannelWriteMode mode);   // Mode, overwrite or normal.

eChannelResult P_CHANNEL_Flush(tChannel* p);

eChannelResult P_CHANNEL_Receive(
    tChannel* p,             // Channel workspace
    uint16_t* pOutEltSize,   // Output element size
    uint8_t* pBuffer,        // Buffer.  If null, only element size without pop is read
    uint16_t* pNbReadBytes,  // Nb bytes read
    uint16_t maxBytesToRead, // Max bytes to read
    TickType_t xTimeToWait,  // Time to wait in ticks
    eChannelReadMode mode);  // Mode RD or KEEP_ONLY. KEEP ONLY read without pop older elemt.

#endif
