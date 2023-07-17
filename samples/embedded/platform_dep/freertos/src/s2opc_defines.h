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

/* Defines proper to S2OPC configuration other MIMXRT1064
 *  should be adapted in regard of other boards and software */
//
//#define _POSIX_SOURCE
//#define XIP_BOOT_HEADER_DCD_ENABLE 1
//#define SKIP_SYSCLK_INIT
//#define DATA_SECTION_IS_CACHEABLE 1

#define FSL_FEATURE_PHYKSZ8081_USE_RMII50M_MODE
// #define MBEDTLS_SSL_CIPHERSUITES MBEDTLS_TLS_ECDHE_RSA
#define _WITH_AES_128_GCM_SHA256
#define FSL_SDK_ENABLE_DRIVER_CACHE_CONTROL 1
#define USE_RTOS 1

#ifdef SDK_OS_BAREMETAL
#undef SDK_OS_BAREMETAL
#endif

#define PRINTF_ADVANCED_ENABLE 1
#define HTTPSRV_CFG_WEBSOCKET_ENABLED 1
#define HTTPSRV_CFG_MBEDTLS_ENABLE 1
#define LWIP_DISABLE_PBUF_POOL_SIZE_SANITY_CHECKS 1
#define MBEDTLS_THREADING_C
#define MBEDTLS_THREADING_ALT
#define SERIAL_PORT_TYPE_UART 1
#ifndef SDK_OS_FREE_RTOS
#define SDK_OS_FREE_RTOS
#endif
#define MCUXPRESSO_SDK
#ifndef SDK_DEBUGCONSOLE_UART
#define SDK_DEBUGCONSOLE_UART
#endif
#define CR_INTEGER_PRINTF

// MBEDTLS configuration
#ifdef MBEDTLS_CONFIG_FILE
#undef MBEDTLS_CONFIG_FILE
#endif
#define MBEDTLS_CONFIG_FILE "sopc_mbedtls_config.h"
#include MBEDTLS_CONFIG_FILE

// new test from existing example
#define SOPC_WITH_EXPAT 0
#define SOPC_PTR_SIZE 4
#define WITH_USER_ASSERT 1

// LWIP parameters
#define MEMP_NUM_REASSDATA 3
#define ENET_RXBD_NUM 10
#define IP_REASS_MAX_PBUFS 9
// #define DNS_MAX_NAME_LENGTH 64
// #define pvPortCalloc SOPC_Calloc

// SOPC parameters
#define SOPC_MAX_SOCKETS 40
#define SOPC_MAX_SESSIONS 4
#define SOPC_MAX_REVERSE_CLIENT_CONNECTIONS 1
#define SOPC_MAX_ENDPOINT_DESCRIPTION_CONFIGURATIONS 5
#define SOPC_DEFAULT_TCP_UA_MAX_BUFFER_SIZE (SOPC_TCP_UA_MIN_BUFFER_SIZE * 4)

#define SOPC_CONSOLE_PRINTF PRINTF

// FREERTOS configuration
#define PRINTF_ADVANCED_ENABLE 1
#define SDK_OS_FREE_RTOS 1
#define USE_RTOS 1

// Add support for printf for 64 bits
#ifndef PRIu64
#define PRIu64 "llu"
#endif
#ifndef PRIi64
#define PRIi64 "lli"
#endif
#ifndef PRId64
#define PRId64 "lld"
#endif
#ifndef PRIx64
#define PRIx64 "llx"
#endif

#define DEMO_BOARD "mimxrt1064"
#include <stdbool.h>
#include <stdint.h>

#include "ethernetif.h"

#define demo_xstr(s) demo_str(s)
#define demo_str(s) #s

/* Ethernet configuration. */
/* IP address configuration. */
#define configIP_ADDR0 192
#define configIP_ADDR1 168
#define configIP_ADDR2 42
#define configIP_ADDR3 102
#define configIP_ADDR_STR \
    demo_xstr(configIP_ADDR0) "." demo_xstr(configIP_ADDR1) "." demo_xstr(configIP_ADDR2) "." demo_xstr(configIP_ADDR3)

/* Netmask configuration. */
#define configNET_MASK0 255
#define configNET_MASK1 255
#define configNET_MASK2 255
#define configNET_MASK3 0

/* Gateway address configuration. */
#define configGW_ADDR0 192
#define configGW_ADDR1 168
#define configGW_ADDR2 42
#define configGW_ADDR3 100

/* MAC address configuration. */
#define PHY_MAC_ADDRESS                    \
    {                                      \
        0x02, 0x12, 0x13, 0x10, 0x15, 0x11 \
    }
#define PHY_MAC_ADDRESS_STR "02:12:13:10:15:11"

extern ethernetif_config_t board_enet_config();
#define BOARD_NETIF_INIT_FN ethernetif0_init

extern void board_gpio_setup(void);

// Set endpoint according to actual IP

#define CONFIG_SOPC_ENDPOINT_ADDRESS "opc.tcp://" configIP_ADDR_STR ":4841"

// General checks

#if !XIP_EXTERNAL_FLASH == 1
//#define XIP_EXTERNAL_FLASH 1
#error "XIP_EXTERNAL_FLASH is expected to be 1"
#endif

#ifdef LWIP_PROVIDE_ERRNO
#error 'LWIP_PROVIDE_ERRNO not expected'
#endif
