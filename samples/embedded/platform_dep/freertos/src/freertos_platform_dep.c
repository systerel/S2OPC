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

#ifdef SDK_PROVIDER_STM
#define xTaskHandle TaskHandle_t
#endif // SDK_PROVIDER

#include "ethernetif.h"
#include "task.h"

/* freeRtos includes */
// order enforced
#if defined SDK_PROVIDER_NXP
#include "board.h"
#include "lwip/init.h"
#include "lwip/tcpip.h"
#elif defined SDK_PROVIDER_STM
#include "lwip.h"
#else
#error "Unsuported or Undefined SDK provider"
#endif // SDK_PROVIDER
#include "lwip/netif.h"
#include "netif/etharp.h"

#include "freertos_platform_dep.h"
#include "samples_platform_dep.h"

#if S2OPC_CRYPTO_MBEDTLS
#include "entropy_poll.h"
#include "sopc_mbedtls_config.h"

#if defined SDK_PROVIDER_NXP
#include "mbedtls/entropy.h"

#elif defined SDK_PROVIDER_STM
#include "mbedtls.h"
#include "mbedtls/entropy.h"
#include "mbedtls/entropy_poll.h"
#else
#error "Unsuported or Undefined SDK provider"
#endif // SDK_PROVIDER

#endif // S2OPC_CRYPTO_MBEDTLS

#include "freertos_shell.h"
#include "p_sopc_common_time.h"
#include "sopc_assert.h"
#include "sopc_common_constants.h"
#include "sopc_date_time.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_mutexes.h"
#include "sopc_threads.h"

#define MAIN_STACK_SIZE ((unsigned short) 2048)

#ifdef PRINTF
#undef PRINTF
#define PRINTF SOPC_Shell_Printf
#endif
#ifndef BOARD_TYPE
#define BOARD_TYPE "Undefined"
#endif

/*******************************************************************************
 * Variables
 ******************************************************************************/
#if defined SDK_PROVIDER_STM
#define HEAP_REGION2_SIZE (128 * 1024)
#define HEAP_REGION2_BASE 0x20000000

static uint8_t ucHeap[configTOTAL_HEAP_SIZE];

HeapRegion_t board_heap5_regions[] = {{(void*) HEAP_REGION2_BASE, HEAP_REGION2_SIZE}, // DTCM RAM
                                      {ucHeap, configTOTAL_HEAP_SIZE},
                                      {NULL, 0}};

#elif defined SDK_PROVIDER_NXP
typedef struct TcpIpInitSync
{
    SOPC_Mutex mutex;
    SOPC_Condition cond;
    bool initialized;
} TcpIpInitSync_t;

// todo
#else
#error "Sorry, Memory mapping is not defined for this board. Unsuported or Undefined SDK provider"
#endif // SDK_PROVIDER
/*******************************************************************************
 * Local Functions
 ******************************************************************************/

/*************************************************/
static void rng_self_test(void)
{
#if S2OPC_CRYPTO_MBEDTLS
    unsigned char buff[32];
    size_t oLen = 0;
    int res = mbedtls_hardware_poll(NULL, buff, sizeof(buff), &oLen);
    SOPC_ASSERT(res == 0);

    SOPC_Shell_Printf("res=[");
    for (size_t i = 0; i < oLen; i++)
    {
        SOPC_Shell_Printf("%02X ", buff[i]);
    }
    SOPC_Shell_Printf("]\n");
#else
    SOPC_Shell_Printf("Mbedtls not used\n");
#endif
}

/*************************************************/
const char* get_IP_str(void)
{
    static char* result = NULL;
    if (NULL == result)
    {
        result = SOPC_Calloc(17, 1);
        SOPC_ASSERT(NULL != result);
#if defined SDK_PROVIDER_NXP
        extern struct netif* netif_list;
        ip4_addr_t* addr = ip_2_ip4(&netif_list->ip_addr);
        snprintf(result, 16, "%u.%u.%u.%u", ip4_addr1(addr), ip4_addr2(addr), ip4_addr3(addr), ip4_addr4(addr));
#elif defined SDK_PROVIDER_STM
        extern uint8_t IP_ADDRESS[4];
        snprintf(result, 16, "%u.%u.%u.%u", IP_ADDRESS[0], IP_ADDRESS[1], IP_ADDRESS[2], IP_ADDRESS[3]);
#else
#error "Unsuported or Undefined SDK provider"
#endif // SDK_PROVIDER
    }
    return result;
}

/*************************************************/
const char* get_EP_str(void)
{
    static char* result = NULL;
    if (NULL == result)
    {
        result = SOPC_Calloc(17 + 15, 1);
        SOPC_ASSERT(NULL != result);
        snprintf(result, 17 + 15, "opc.tcp://%s:4841", get_IP_str());
    }
    return result;
}

/*************************************************/
// True if lwip stack was already initialized, False otherwise
#if defined SDK_PROVIDER_STM
static bool IslwipInitialized(void)
{
    extern struct netif gnetif;
    return NULL != gnetif.next;
}
#endif // SDK_PROVIDER

#if defined SDK_PROVIDER_NXP
static bool IslwipInitialized(void)
{
    extern struct netif* netif_list;
    return (netif_list != NULL);
}
#endif // SDK_PROVIDER

/*************************************************/
void SOPC_Platform_Setup(void)
{
    if (!IslwipInitialized())
    {
        PRINTF("Initializing LwIP\n");
#if defined SDK_PROVIDER_NXP
        lwip_init();
#elif defined SDK_PROVIDER_STM
        MX_LWIP_Init();
#else
#error "Unsuported or Undefined SDK provider"
#endif // SDK_PROVIDER
    }
#if defined SDK_PROVIDER_NXP
    extern struct netif* netif_list;
    ip4_addr_t* addr = ip_2_ip4(&netif_list->netmask);
#elif defined SDK_PROVIDER_STM
    extern uint8_t NETMASK_ADDRESS[4];
#else
#error "Unsuported or Undefined SDK provider"
#endif // SDK_PROVIDER
    PRINTF("\n************************************************\n");
    PRINTF("SOPC / FreeRTOS / MbedTLS / LwIP example\n");
    PRINTF("************************************************\n");
    PRINTF(" Board type       : %s\n", BOARD_TYPE);
    PRINTF(" IPv4 Address     : %s\n", get_IP_str());
#if defined SDK_PROVIDER_NXP
    PRINTF(" IPv4 Subnet mask : %u.%u.%u.%u\n", ip4_addr1(addr), ip4_addr2(addr), ip4_addr3(addr), ip4_addr4(addr));
#elif defined SDK_PROVIDER_STM

    PRINTF(" IPv4 Subnet mask : %u.%u.%u.%u\n", NETMASK_ADDRESS[0], NETMASK_ADDRESS[1], NETMASK_ADDRESS[2],
           NETMASK_ADDRESS[3]);
#else
#error "Unsuported or Undefined SDK provider"
#endif // SDK_PROVIDER
    PRINTF("************************************************\n");

    PRINTF(" Initializing MbedTLS\n");
    //    MX_MBEDTLS_Init(); / Already done by generated main

    PRINTF(" Call command \"help\" to get help about usage of this sample \n");
}

/*************************************************/
void SOPC_Platform_Shutdown(const bool reboot)
{
    vTaskEndScheduler();
    if (reboot)
    {
        PRINTF("\n# Rebooting in 5 seconds...\n");
        SOPC_Sleep(5000);
        SOPC_Platform_Main();
    }
    PRINTF("\n# Shutdown\n");
    configASSERT(0);
}

/*************************************************/
void sopc_main_entry(void* param)
{
    SOPC_UNUSED_ARG(param);

    SOPC_Platform_Target_Debug(NULL);
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
#endif // SOPC_FREERTOS_RAW_INIT

/*************************************************/
void sopc_main(void)
{
    static const int priority = 0;
    TaskHandle_t tHandle;

    BaseType_t returned;
    returned = xTaskCreate(sopc_main_entry, "Main", MAIN_STACK_SIZE, NULL, priority, &tHandle);
    SOPC_ASSERT(pdPASS == returned);
}

/*************************************************/
const char* SOPC_Platform_Get_Default_Net_Itf(void)
{
    static char netif[6] = {0};
    if (netif[0] == 0)
    {
        // Note: FreeRTOS does not interpret correctly PRIu8.
        sprintf(netif, "%c%c%d", netif_default->name[0], netif_default->name[1], (int) netif_default->num);
    }
    return netif;
}

/*************************************************/
void SOPC_Platform_Target_Debug(const char* param)
{
    if (NULL == param || *param == 0)
    {
        PRINTF(
            "\n"
            "************************************\n");
        PRINTF("** BOOTING FreeRTOS S2OPC SAMPLE  **\n");
        PRINTF("** BUILD ON " __DATE__ " " __TIME__ "  **\n");

#ifndef SDK_PROVIDER_NXP
        const unsigned remHeap = xPortGetFreeHeapSize();
        const unsigned minHeap = xPortGetMinimumEverFreeHeapSize();
        PRINTF("** HEAP : Size : %u, Free: %u, MinFree : %u \n", configTOTAL_HEAP_SIZE + HEAP_REGION2_SIZE, remHeap,
               minHeap);
#else
        // GetMinimumEverFree is not available in newlib's malloc implementation.
        const unsigned remHeap = xPortGetFreeHeapSize();
        PRINTF("** HEAP : Free: %u, Free : %u \n", configTOTAL_HEAP_SIZE, remHeap);
#endif //! SDK_PROVIDER_NXP
    }
    else if (0 == strcmp(param, "help"))
    {
        PRINTF("\nDebug sub commands: arp, rng\n");
    }
    else if (0 == strcmp(param, "arp"))
    {
        PRINTF("\nSending a gratuitous ERP packet...\n");
        etharp_gratuitous(netif_default);
    }
    else if (0 == strcmp(param, "rng"))
    {
        PRINTF("\nRNG self-test...\n");
        rng_self_test();
    }
    else if (0 == strncmp(param, "date", 4))
    {
        P_SOPC_COMMON_TIME_SetDateOffset(atoi(param + 4));
    }
}

#if S2OPC_CRYPTO_MBEDTLS
// MBEDTLS Platform specific declarations
/*************************************************/
// #warning "Following function should return time since 1970 à 00:00:00 of 1st january"
time_t sopc_time_alt(time_t* timer)
{
    time_t t;
    SOPC_ReturnStatus status = SOPC_Time_ToUnixTime(SOPC_Time_GetCurrentTimeUTC(), &t);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    if (NULL != timer)
    {
        *timer = t;
    }
    return t;
}
#endif // S2OPC_CRYPTO_MBEDTLS
