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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

#include "ksdk_mbedtls_config.h"
#include "p_ethernet_if.h"
#include "p_time.h"
#include "publisher.h"
#include "subscriber.h"

Condition* handleCondition;
tUtilsList list;

int FREE_RTOS_S2OPC_PUBLISHER(void* ptr);
int FREE_RTOS_S2OPC_SUBSCRIBER(void* ptr);

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

    // Create S2OPC publisher Thread
    // FREE_RTOS_S2OPC_PUBLISHER(NULL);

    // Create S2OPC Subscriber thread
    FREE_RTOS_S2OPC_SUBSCRIBER(NULL);

    vTaskStartScheduler();

    for (;;)
        ;
}

int FREE_RTOS_S2OPC_PUBLISHER(void* ptr)
{
    SOPC_ReturnStatus status;
    Condition* pv = (Condition*) ptr;
    Thread pX = NULL;
    status = SOPC_Thread_Create(&pX, (void*) cbToolkit_publisher, pv, NULL);
    return status;
}

int FREE_RTOS_S2OPC_SUBSCRIBER(void* ptr)
{
    SOPC_ReturnStatus status;
    Condition* pv = (Condition*) ptr;
    Thread pX = NULL;
    status = SOPC_Thread_Create(&pX, (void*) cbToolkit_subscriber, pv, NULL);
    return status;
}
