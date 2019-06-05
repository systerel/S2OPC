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
#include "FreeRTOS.h"
#include "MIMXRT1064.h"
#include "board.h"
#include "clock_config.h"
#include "fsl_debug_console.h"
#include "peripherals.h"
#include "pin_mux.h"

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "timers.h"

/* Freescale includes. */
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_device_registers.h"

#include "clock_config.h"
#include "pin_mux.h"

#include "p_synchronisation.h"
#include "sopc_mutexes.h"

#include "FreeRTOSTest.h"

Condition* handleCondition;
tUtilsList list;

int main(void)
{
    /* Init board hardware. */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitBootPeripherals();
    /* Init FSL debug console. */
    BOARD_InitDebugConsole();

    /*memset(&list,0,sizeof(tUtilsList));

    P_UTILS_LIST_Init(&list,4);

    P_UTILS_LIST_AddElt(&list,1,0,0,0);
    P_UTILS_LIST_AddElt(&list,2,0,0,0);
    P_UTILS_LIST_AddElt(&list,3,0,0,0);

    P_UTILS_LIST_RemoveElt(&list,1,0,0);
    P_UTILS_LIST_RemoveElt(&list,2,0,0);
    P_UTILS_LIST_RemoveElt(&list,3,0,0);

    P_UTILS_LIST_AddElt(&list,4,0,0,0);

    P_UTILS_LIST_RemoveElt(&list,4,0,0);

    P_UTILS_LIST_AddElt(&list,4,0,0,0);

    P_UTILS_LIST_RemoveElt(&list,4,0,0);

    P_UTILS_LIST_DeInit(&list);*/

    handleCondition = Condition_Create();

    FREE_RTOS_TEST_API_S2OPC_THREAD(handleCondition);

    vTaskStartScheduler();

    for (;;)
        ;
}
