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

//#include "p_ethernet_if.h"
//#include "p_logsrv.h"

Condition* handleCondition = NULL;
Condition* handleSigConnexion = NULL;
tUtilsList list;

extern tLogSrvWks* gLogServer;

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
    Condition_SignalAll(handleSigConnexion);
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

    // P_ETHERNET_IF_Initialize();

    // p = P_LOG_SRV_CreateAndStart(80,2);
    // gpLogServ = P_LOG_SRV_CreateAndStart(80,4);

    // memset(&list,0,sizeof(tUtilsList));

    // P_UTILS_LIST_Init(&list,4);

    //  P_UTILS_LIST_AddElt(&list,1,0,0,0);
    // P_UTILS_LIST_AddElt(&list,2,0,0,0);
    //  P_UTILS_LIST_AddElt(&list,3,0,0,0);
    //
    // P_UTILS_LIST_RemoveElt(&list,1,0,0);
    //  P_UTILS_LIST_RemoveElt(&list,2,0,0);
    //  P_UTILS_LIST_RemoveElt(&list,3,0,0);

    //  P_UTILS_LIST_AddElt(&list,4,0,0,0);

    //  P_UTILS_LIST_RemoveElt(&list,4,0,0);

    //  P_UTILS_LIST_AddElt(&list,4,0,0,0);

    //  P_UTILS_LIST_RemoveElt(&list,4,0,0);

    // P_UTILS_LIST_DeInit(&list);

    // int fd = fopen("path","w");

    gLogServer = P_LOG_SRV_CreateAndStart(60, 4023, 8, 0, 5, cbOneConnexion, NULL, cbEchoCallback, NULL, NULL, NULL,
                                          NULL, NULL, cbHelloCallback);

    handleCondition = Condition_Create();
    handleSigConnexion = Condition_Create();

    // FREE_RTOS_TEST_API_S2OPC_THREAD(handleCondition);

    FREE_RTOS_TEST_S2OPC_SERVER(handleSigConnexion);

    vTaskStartScheduler();

    for (;;)
        ;
}
