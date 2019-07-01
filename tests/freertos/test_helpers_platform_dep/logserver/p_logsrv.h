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

#ifndef FREERTOS_P_LOGSRV_H_
#define FREERTOS_P_LOGSRV_H_

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

#define P_LOG_SRV_ONLINE_PERIOD (10)
#define P_LOG_SRV_BINDING_WAIT (10)
#define P_LOG_CLT_MONITOR_PERIOD (10)
#define P_LOG_CLT_TX_PERIOD (10)
#define P_LOG_CLT_RX_PERIOD (10)

#define P_LOG_SRV_CALLBACK_STACK (512)
#define P_LOG_CLT_MON_CALLBACK_STACK (512)
#define P_LOG_CLT_TX_CALLBACK_STACK (512)
#define P_LOG_CLT_RX_CALLBACK_STACK (512)

#define P_LOG_FIFO_DATA_SIZE (4096)
#define P_LOG_FIFO_ELT_MAX_SIZE (1024)
#define P_LOG_FIFO_MAX_NB_ELT (512)

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

typedef struct T_LOG_SERVER_WORKSPACE tLogSrvWks;
typedef struct T_LOG_CLIENT_WORKSPACE tLogClientWks;

typedef eResultDecoder (*ptrFct_AnalyzerCallback)(void* pAnalyzerContext,     //
                                                  tLogClientWks* pClt,        //
                                                  uint8_t* pBufferInOut,      //
                                                  uint16_t* dataSize,         //
                                                  uint16_t maxSizeBufferOut); //

typedef eResultDecoder (*ptrFct_AnalyzerPeriodic)(void* pAnalyzerContext,     //
                                                  tLogClientWks* pClt,        //
                                                  uint8_t* pBufferOut,        //
                                                  uint16_t* dataSize,         //
                                                  uint16_t maxSizeBufferOut); //

typedef void (*ptrFct_AnalyzerContextCreation)(void** pAnalyzerContext, //
                                               tLogClientWks* pClt);    //

typedef void (*ptrFct_AnalyzerContextDestruction)(void** pAnalyzerContext, //
                                                  tLogClientWks* pClt);    //

typedef void (*ptrFct_EncoderContextCreation)(void** ppEncoderContext);

typedef void (*ptrFct_EncoderContextDestruction)(void** ppEncoderContext);

typedef eResultEncoder (*ptrFct_EncoderCallback)(void* pEncoderContext,      //
                                                 uint8_t* pBufferInOut,      //
                                                 uint16_t* pNbBytesToEncode, //
                                                 uint16_t maxSizeBufferOut); //

typedef eResultEncoder (*ptrFct_EncoderPeriodicCallback)(void* pEncoderContext);

typedef uint16_t (*ptrFct_EncoderTransmitHelloCallback)(uint8_t* pBufferInOut,      //
                                                        uint16_t nbBytesToEncode,   //
                                                        uint16_t maxSizeBufferOut); //

tLogSrvWks* P_LOG_SRV_CreateAndStart(uint16_t port,                                                          //
                                     uint16_t portHello,                                                     //
                                     int16_t maxClient,                                                      //
                                     uint32_t timeoutS,                                                      //
                                     uint32_t periodeHelloS,                                                 //
                                     ptrFct_AnalyzerContextCreation cbAnalyzerContextCreationCallback,       //
                                     ptrFct_AnalyzerContextDestruction cbAnalyzerContextDestructionCallback, //
                                     ptrFct_AnalyzerCallback cbAnalyzerCallback,                             //
                                     ptrFct_AnalyzerPeriodic cbAnalyzerTimeOutCallback,                      //
                                     ptrFct_EncoderContextCreation cbSenderContextCreation,                  //
                                     ptrFct_EncoderContextDestruction cbSenderContextDestruction,            //
                                     ptrFct_EncoderCallback cbSenderCallback,                                //
                                     ptrFct_EncoderPeriodicCallback cbSenderTimeoutCallback,                 //
                                     ptrFct_EncoderTransmitHelloCallback cbSenderHelloCallback);             //

void P_LOG_SRV_StopAndDestroy(tLogSrvWks** p);

eLogSrvResult P_LOG_SRV_SendToAllClient(tLogSrvWks* p,          //
                                        const uint8_t* pBuffer, //
                                        uint16_t length,        //
                                        uint16_t* sentLength);  //

eLogSrvResult P_LOG_CLIENT_SendResponse(tLogClientWks* pClt,     //
                                        const uint8_t* pBuffer,  //
                                        uint16_t length,         //
                                        uint16_t* pNbBytesSent); //

#endif /* FREERTOS_P_LOGSRV_H_ */
