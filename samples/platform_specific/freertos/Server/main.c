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

/* specific boards inclusion */
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"

/* S2OPC helpers  */
#include "p_time.h"
#include "sopc_threads.h"

/* boards portage Mbedtls and LwIP helpers */
#include "ksdk_mbedtls_config.h"
#include "p_ethernet_if.h"

#include "server.h"

/* Create a S2OPC server thread */
static int FREE_RTOS_S2OPC_SERVER(void);

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

    // Create S2OPC server Thread
    FREE_RTOS_S2OPC_SERVER();

    vTaskStartScheduler();

    for (;;)
        ;
}

int FREE_RTOS_S2OPC_SERVER(void)
{
    SOPC_ReturnStatus status;
    Thread pX = NULL;
    status = SOPC_Thread_Create(&pX, (void*) cbToolkit_test_server, NULL, NULL);
    return status;
}
