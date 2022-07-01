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

#ifndef P_LOGSRV_H
#define P_LOGSRV_H

#include <inttypes.h> /* stdlib includes */
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "sopc_enums.h"
#include "sopc_mutexes.h"
#include "sopc_threads.h"

#include "FreeRTOS.h" /* freeRtos includes */
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include "timers.h"

#include "p_ethernet_if.h"
#include "p_synchronisation.h" /* synchronisation include */
#include "p_threads.h"
#include "p_utils.h" /* private list include */

#include "ethernetif.h"
#include "lwip/dhcp.h"
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/netif.h"
#include "lwip/opt.h"
#include "lwip/timeouts.h"
#include "lwipopts.h"
#include "netif/etharp.h"

#include "lwip/init.h"
#include "lwip/netif.h"
#include "lwip/sockets.h"
#include "lwip/tcpip.h"

// ************Private API**************

#define configPRIORITY_LOGSRV 0

// Periodic config
#define P_LOG_SRV_ONLINE_PERIOD (1000)
#define P_LOG_SRV_BINDING_WAIT (1000)
#define P_LOG_SRV_TX_PERIOD (1000)
#define P_LOG_CLT_MONITOR_PERIOD (1000)
#define P_LOG_CLT_TX_PERIOD (1000)
#define P_LOG_CLT_RX_PERIOD (1000)

// Stack config
#define P_LOG_SRV_CALLBACK_STACK (384)
#define P_LOG_SRV_CALLBACK_TX_STACK (128)
#define P_LOG_CLT_MON_CALLBACK_STACK (384)
#define P_LOG_CLT_TX_CALLBACK_STACK (384)
#define P_LOG_CLT_RX_CALLBACK_STACK (384)

// Atomic fifo config
#define P_LOG_FIFO_DATA_SIZE_SRV_TX (8196)    // Max cumulative size
#define P_LOG_FIFO_ELT_MAX_SIZE_SRV_TX (2048) // Atomic max size
#define P_LOG_FIFO_MAX_NB_ELT_SRV_TX (1024)   // Max elt

#define P_LOG_FIFO_DATA_SIZE_CLT_TX (8196)
#define P_LOG_FIFO_ELT_MAX_SIZE_CLT_TX (256)
#define P_LOG_FIFO_MAX_NB_ELT_CLT_TX (1024)

#define P_LOG_FIFO_DATA_SIZE_CLT_RX (1024)
#define P_LOG_FIFO_ELT_MAX_SIZE_CLT_RX (128)
#define P_LOG_FIFO_MAX_NB_ELT_CLT_RX (128)

#define UDP_BUFFER_SIZE (128)
#define LWIP_RX_BUFFER_SIZE (P_LOG_FIFO_ELT_MAX_SIZE_CLT_RX)
#define ANALYZER_RX_BUFFER_SIZE (P_LOG_FIFO_ELT_MAX_SIZE_CLT_RX)
#define ENCODER_RX_BUFFER_SIZE (P_LOG_FIFO_ELT_MAX_SIZE_CLT_TX * 2)
#define LOG_RECORD_RX_BUFFER_SIZE (P_LOG_FIFO_ELT_MAX_SIZE_SRV_TX)

#define PADDING ((ENCODER_RX_BUFFER_SIZE + LWIP_RX_BUFFER_SIZE + ANALYZER_RX_BUFFER_SIZE) % 4)

typedef enum E_LOG_SRV_RESULT
{
    E_LOG_SRV_RESULT_OK,
    E_LOG_SRV_RESULT_NOK
} eLogSrvResult;

typedef enum E_RESULT_ENCODER
{
    E_ENCODER_RESULT_OK,
    E_ENCODER_RESULT_ERROR_NOK // Disconnect client
} eResultEncoder;

typedef enum E_RESULT_DECODER
{
    E_DECODER_RESULT_OK,
    E_DECODER_RESULT_ERROR_NOK // Disconnect client
} eResultDecoder;

// Log server status
typedef enum E_LOG_SERVER_STATUS
{
    E_LOG_SERVER_CLOSING,
    E_LOG_SERVER_BINDING,
    E_LOG_SERVER_ONLINE,
    E_LOG_SERVER_SIZEOF = UINT32_MAX
} eLogServerStatus;

typedef struct T_LOG_SERVER_WORKSPACE tLogSrvWks;
typedef struct T_LOG_CLIENT_WORKSPACE tLogClientWks;

//***Analyzer callbacks***
// Analyzer callback.
typedef eResultDecoder ptrFct_AnalyzerCallback(
    void* pAnalyzerContext,     // Context analyzer.
    tLogClientWks* pClt,        // Client workspace, can be used to send response by callback
    uint8_t* pBufferInOut,      // Last buffer received
    uint16_t* dataSize,         // Data size of significant data in buffer then out buffer
    uint16_t maxSizeBufferOut); // Max size  of buffer.

// Analyzer periodic callback. Called each P_LOG_CLT_RX_PERIOD defined in this .H
typedef eResultDecoder ptrFct_AnalyzerPeriodic(
    void* pAnalyzerContext,     // Context analyzer
    tLogClientWks* pClt,        // Client workspace, can be used to send response by callback
    uint8_t* pBufferOut,        // Buffer to send
    uint16_t* dataSize,         // data size to send
    uint16_t maxSizeBufferOut); // max size of buffer.

// Analyzer context creation callback
typedef void ptrFct_AnalyzerContextCreation(void** pAnalyzerContext, // Context analyzer to free
                                            tLogClientWks* pClt);    // Client workspace, can be used to send response

// Analyzer context destruction callback
typedef void ptrFct_AnalyzerContextDestruction(void** pAnalyzerContext, // Context analyzer to destroy
                                               tLogClientWks* pClt); // Client workspace, can be used to send response

//***Encoder callbacks***
typedef void ptrFct_EncoderContextCreation(void** ppEncoderContext);

typedef void ptrFct_EncoderContextDestruction(void** ppEncoderContext);

typedef eResultEncoder ptrFct_EncoderCallback(void* pEncoderContext,      // Encoder context
                                              uint8_t* pBufferInOut,      // Buffer in out
                                              uint16_t* pNbBytesToEncode, // Signicant bytes in / out
                                              uint16_t maxSizeBufferOut); // Max size of buffer

typedef eResultEncoder ptrFct_EncoderPeriodicCallback(void* pEncoderContext);

// Callback used to modify default hello string sent on UDP broadcast. Return nb bytes of new size of buffer in out
typedef uint16_t ptrFct_EncoderTransmitHelloCallback(uint8_t* pBufferInOut, // Receive "192.168.1.102:4063" by default
                                                     uint16_t nbBytesToEncode,   // Strlen of inOut
                                                     uint16_t maxSizeBufferOut); // Max size of buffer.

/***Public API declaration***/

tLogSrvWks* P_LOG_SRV_CreateAndStart(uint16_t port,          // Listen port
                                     uint16_t portHello,     // UDP Hello port destination
                                     int16_t maxClient,      // Max client
                                     uint32_t timeoutS,      // Timeout. If 0, no timeout.
                                     uint32_t periodeHelloS, // Period of hello message
                                     ptrFct_AnalyzerContextCreation* cbAnalyzerContextCreationCallback,       //
                                     ptrFct_AnalyzerContextDestruction* cbAnalyzerContextDestructionCallback, //
                                     ptrFct_AnalyzerCallback* cbAnalyzerCallback,                             //
                                     ptrFct_AnalyzerPeriodic* cbAnalyzerTimeOutCallback,                      //
                                     ptrFct_EncoderContextCreation* cbSenderContextCreation,                  //
                                     ptrFct_EncoderContextDestruction* cbSenderContextDestruction,            //
                                     ptrFct_EncoderCallback* cbSenderCallback,                                //
                                     ptrFct_EncoderPeriodicCallback* cbSenderTimeoutCallback,                 //
                                     ptrFct_EncoderTransmitHelloCallback* cbSenderHelloCallback);             //

void P_LOG_SRV_StopAndDestroy(tLogSrvWks** p);

eLogSrvResult P_LOG_SRV_SendToAllClient(tLogSrvWks* p,          // Server handle
                                        const uint8_t* pBuffer, // Buffer to send
                                        uint16_t length,        // Length
                                        uint16_t* sentLength);  // Sent lenght

eLogSrvResult P_LOG_SRV_SendToAllClientFullBuffer(tLogSrvWks* p,
                                                  const uint8_t* pBuffer,
                                                  uint16_t length,
                                                  uint16_t* sentLength,
                                                  bool bFlushOnFull);

eLogServerStatus P_LOG_SRV_GetStatus(tLogSrvWks* p);

eLogSrvResult P_LOG_CLIENT_SendResponse(tLogClientWks* pClt,     // Client handle
                                        const uint8_t* pBuffer,  // Buffer to send
                                        uint16_t length,         // Length
                                        uint16_t* pNbBytesSent); // Sent length

#endif /* P_LOGSRV_H */
