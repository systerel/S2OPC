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

/** \file samples_platform_dep.c
 *
 * \brief Provides the implementation for FreeRTOS OS-dependant features required for samples
 */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#if SOPC_FREERTOS_RAW_INIT
#include "board.h"
#include "clock_config.h"
#include "pin_mux.h"

#include "fsl_debug_console.h"
#ifndef configMAC_ADDR
#include "fsl_silicon_id.h"
#endif

#endif // SOPC_FREERTOS_RAW_INIT
#include "FreeRTOS.h"

#include "ethernetif.h"
#include "task.h"

/* freeRtos includes */
// order enforced
#include "lwip.h"
#include "lwip/netif.h"
#include "netif/etharp.h"

#include "samples_platform_dep.h"
#include "sopc_mbedtls_config.h"

#include "mbedtls.h"

#include "freertos_shell.h"
#include "sopc_assert.h"
#include "sopc_common_constants.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_mutexes.h"
#include "sopc_platform_time.h"

#define MAIN_STACK_SIZE ((unsigned short) 2048)

#define PRINTF SOPC_Shell_Printf

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Local Functions
 ******************************************************************************/

/*************************************************/
void SOPC_Platform_Setup(void)
{
    PRINTF("Initializing LwIP\n");
    MX_LWIP_Init();
    extern uint8_t IP_ADDRESS[4];
    extern uint8_t NETMASK_ADDRESS[4];
    PRINTF("\r\n************************************************\n");
    PRINTF("SOPC / FreeRTOS / MbedTLS / LwIP example\n");
    PRINTF("************************************************\n");
    PRINTF(" IPv4 Address     : %u.%u.%u.%u\n", IP_ADDRESS[0], IP_ADDRESS[1], IP_ADDRESS[2], IP_ADDRESS[3]);
    PRINTF(" IPv4 Subnet mask : %u.%u.%u.%u\n", NETMASK_ADDRESS[0], NETMASK_ADDRESS[1], NETMASK_ADDRESS[2],
           NETMASK_ADDRESS[3]);
    PRINTF("************************************************\n");

    PRINTF("Initializing MbedTLS\n");
    //    MX_MBEDTLS_Init(); / Already done by generated main

    // Init tick to UTC time (build date)
    P_TIME_SetInitialDateToBuildTime();

    PRINTF(" Call command \"help\" to get help about usage of this sample \n");
}

/*************************************************/
void SOPC_Platform_Shutdown(const bool reboot)
{
    vTaskEndScheduler();
    if (reboot)
    {
        PRINTF("\n# Rebooting in 5 seconds...\r\n");
        SOPC_Sleep(5000);
        SOPC_Platform_Main();
    }
    PRINTF("\n# Shutdown\r\n");
    configASSERT(0);
}

/*************************************************/
void cb_main_wrapper(void* param)
{
    SOPC_Shell_Printf("cb_main_wrapper(%p)\r\n", param);
    SOPC_UNUSED_ARG(param);

    SOPC_Platform_Target_Debug();
    SOPC_Platform_Main();
}

#if SOPC_FREERTOS_RAW_INIT
/*************************************************/
static void BOARD_InitModuleClock(void)
{
    const clock_enet_pll_config_t config = {
        .enableClkOutput = true,
        .enableClkOutput25M = false,
        .loopDivider = 1,
    };
    CLOCK_InitEnetPll(&config);
}

/*************************************************/
int main(void)
{
    static const int priority = 0;

    BOARD_ConfigMPU();
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
    BOARD_InitModuleClock();

    board_gpio_setup();
    sopc_main();
    vTaskStartScheduler();
    return 0;
}
#endif

/*************************************************/
void sopc_main(void)
{
    static const int priority = 0;
    TaskHandle_t tHandle;

    xTaskCreate(cb_main_wrapper, "Main", MAIN_STACK_SIZE, NULL, priority, &tHandle);
}

/*************************************************/
const char* SOPC_Platform_Get_Default_Net_Itf(void)
{
    return (netif_default == NULL ? "NULL" : netif_default->name);
}

/*************************************************/
void SOPC_Platform_Target_Debug(void)
{
    PRINTF(
        "\r\n"
        "************************************\r\n");
    PRINTF("** BOOTING FreeRTOS S2OPC SAMPLE  **\r\n");
    PRINTF("** BUILD ON " __DATE__ " " __TIME__ "  **\r\n");

    const unsigned remHeap = xPortGetFreeHeapSize();
    const unsigned minHeap = xPortGetMinimumEverFreeHeapSize();
    PRINTF("** HEAP : Size : %u, Free: %u, MinFree : %u \r\n", configTOTAL_HEAP_SIZE, remHeap, minHeap);
}
