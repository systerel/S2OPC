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

Condition* handleCondition;
tUtilsList list;
// tLogSrvWks* pLogSrv;

int main(void)
{
    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    BOARD_InitBootPeripherals();

    P_ETHERNET_IF_Initialize();

    // Init tick to UTC time (build date)
    P_TIME_SetInitialDateToBuildTime();

    // Attach S2OPC Mutexes mechanism to mbedtls.
    mbedtls_threading_initialize();

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

    // pLogSrv = P_LOG_SRV_CreateAndStart(60,2);

    // handleCondition = Condition_Create();

    FREE_RTOS_TEST_S2OPC_PUBSUB(NULL);

    vTaskStartScheduler();

    for (;;)
        ;
}
