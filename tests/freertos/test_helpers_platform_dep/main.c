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

#include <stdio.h>

#include "lwip/opt.h"
#include "lwip/tcpip.h"

#include "board.h"
#include "clock_config.h"
#include "ethernetif.h"
#include "netif/ethernet.h"
#include "peripherals.h"
#include "pin_mux.h"

#include "FreeRTOS.h"
#include "MIMXRT1064.h"
#include "queue.h"
#include "task.h"
#include "timers.h"

#include "fsl_debug_console.h"
#include "fsl_device_registers.h"
#include "fsl_gpio.h"
#include "fsl_iomuxc.h"
#include "fsl_qtmr.h"

#include "sopc_mutexes.h"
#include "sopc_threads.h"

#include "FreeRTOSTest.h"

#include "p_ethernet_if.h"
#include "p_logsrv.h"

#include "ksdk_mbedtls_config.h"

#include "p_time.h"

// Global variables

Condition gHandleConditionVariable;
Condition gHandleSignalConnexionSrvLog;

uint16_t cbHelloCallback(uint8_t* pBufferInOut, uint16_t nbBytesToEncode, uint16_t maxSizeBufferOut)
{
    uint16_t size;

    snprintf((void*) pBufferInOut + nbBytesToEncode, maxSizeBufferOut - (2 * nbBytesToEncode + 1), "%s",
             "Hello, log server is listening on the following site : ");
    size = strlen((void*) pBufferInOut + nbBytesToEncode);
    memmove((void*) pBufferInOut + nbBytesToEncode + size, (void*) pBufferInOut, nbBytesToEncode);
    memmove((void*) pBufferInOut, (void*) pBufferInOut + nbBytesToEncode, nbBytesToEncode + size);
    return nbBytesToEncode + size;
}

void cbOneConnexion(void** pAnalyzerContext, tLogClientWks* pClt)
{
    Condition_SignalAll(&gHandleSignalConnexionSrvLog);
}

eResultDecoder cbEchoCallback(void* pAnalyzerContext,
                              tLogClientWks* pClt,
                              uint8_t* pBufferInOut,
                              uint16_t* dataSize,
                              uint16_t maxSizeBufferOut)
{
    // Don't modify dataSize output, echo simulation
    return 0;
}

int main(void)
{
    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    BOARD_InitBootPeripherals();

    P_ETHERNET_IF_Initialize();

    P_TIME_SetInitialDateToBuildTime();

    mbedtls_threading_initialize();

    gLogServer = P_LOG_SRV_CreateAndStart(60,               //
                                          4023,             //
                                          2,                //
                                          0,                //
                                          5,                //
                                          cbOneConnexion,   //
                                          NULL,             //
                                          cbEchoCallback,   //
                                          NULL,             //
                                          NULL,             //
                                          NULL,             //
                                          NULL,             //
                                          NULL,             //
                                          cbHelloCallback); //

    Condition_Init(&gHandleConditionVariable);
    Condition_Init(&gHandleSignalConnexionSrvLog);

    // FREE_RTOS_TEST_API_S2OPC_THREAD(&gHandleConditionVariable);

    FREE_RTOS_TEST_S2OPC_SERVER(&gHandleSignalConnexionSrvLog);

    // FREE_RTOS_TEST_S2OPC_TIME(&gHandleSignalConnexionSrvLog);

    vTaskStartScheduler();

    for (;;)
        ;
}
