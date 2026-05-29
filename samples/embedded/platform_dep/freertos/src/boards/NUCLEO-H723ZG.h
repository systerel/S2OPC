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

#ifndef S2OPC_SAMPLES_EMBEDDED_CLI_PUBSUB_SERVER_SRC_BOARDS_NUCLEO_H723ZG_H_
#define S2OPC_SAMPLES_EMBEDDED_CLI_PUBSUB_SERVER_SRC_BOARDS_NUCLEO_H723ZG_H_

// LWIP Debug options
// define LWIP_DEBUG to enable LWIP debug.
// #define LWIP_DEBUG
#define LWIP_DBG_MIN_LEVEL LWIP_DBG_LEVEL_ALL

// Set LWIP_DBG_ON to enable debug of specific module otherwise set to LWIP_DBG_OFF
#define ETHARP_DEBUG LWIP_DBG_OFF
#define NETIF_DEBUG LWIP_DBG_OFF
#define PBUF_DEBUG LWIP_DBG_OFF
#define API_LIB_DEBUG LWIP_DBG_OFF
#define API_MSG_DEBUG LWIP_DBG_OFF
#define SOCKETS_DEBUG LWIP_DBG_OFF
#define ICMP_DEBUG LWIP_DBG_OFF
#define IGMP_DEBUG LWIP_DBG_OFF
#define INET_DEBUG LWIP_DBG_OFF
#define IP_DEBUG LWIP_DBG_OFF
#define IP_REASS_DEBUG LWIP_DBG_OFF
#define RAW_DEBUG LWIP_DBG_OFF
#define MEM_DEBUG LWIP_DBG_OFF
#define MEMP_DEBUG LWIP_DBG_OFF
#define SYS_DEBUG LWIP_DBG_OFF
#define TIMERS_DEBUG LWIP_DBG_OFF
#define TCP_DEBUG LWIP_DBG_OFF
#define TCP_INPUT_DEBUG LWIP_DBG_OFF
#define TCP_FR_DEBUG LWIP_DBG_OFF
#define TCP_RTO_DEBUG LWIP_DBG_OFF
#define TCP_CWND_DEBUG LWIP_DBG_OFF
#define TCP_WND_DEBUG LWIP_DBG_OFF
#define TCP_OUTPUT_DEBUG LWIP_DBG_OFF
#define TCP_RST_DEBUG LWIP_DBG_OFF
#define TCP_QLEN_DEBUG LWIP_DBG_OFF
#define UDP_DEBUG LWIP_DBG_OFF
#define TCPIP_DEBUG LWIP_DBG_OFF
#define SLIP_DEBUG LWIP_DBG_OFF
#define DHCP_DEBUG LWIP_DBG_OFF
#define AUTOIP_DEBUG LWIP_DBG_OFF
#define DNS_DEBUG LWIP_DBG_OFF
#define IP6_DEBUG LWIP_DBG_OFF

// By default LWIP_PLATFORM_DIAG use printf
#define LWIP_PLATFORM_DIAG(x) \
    do                        \
    {                         \
        PRINT x;              \
    } while (0)

// SOPC configuration this is actually not depending of the board
// This must be done before other includes
#define SOPC_PTR_SIZE 4
#define WITH_USER_ASSERT 1

// Override MBEDTLS_CONFIG_FILE
#ifdef MBEDTLS_CONFIG_FILE
#undef MBEDTLS_CONFIG_FILE
#endif
#define MBEDTLS_CONFIG_FILE "sopc_mbedtls_config.h"

// MbedTLS specific configuration
#define MBEDTLS_PLATFORM_TIME_ALT
#define MBEDTLS_PLATFORM_MS_TIME_ALT
#define MBEDTLS_HAVE_TIME

#define MBEDTLS_ENTROPY_C
#define MBEDTLS_ENTROPY_HARDWARE_ALT
#define MBEDTLS_NO_PLATFORM_ENTROPY

// MbedTLS board specific configuration
// Place MbedTLS heap in a DTMCRAM location
#define MBEDTLS_HEAP_SECTION __attribute__((section(".data_dtcmram")))

#ifdef MBEDTLS_HEAP_SIZE
#undef MBEDTLS_HEAP_SIZE
#endif
#define MBEDTLS_HEAP_SIZE (42 * 1024)

// Board-Specific configuration

#define BOARD_TYPE "NUCLEO_H723ZG"
#define SDK_PROVIDER_STM

#include "stm32h7xx_hal.h"

#include "freertos_platform_dep.h"

#include "freertos_shell.h"

#undef PRINT
#define PRINT SOPC_Shell_Printf
#undef PRINTF
#define PRINTF SOPC_Shell_Printf

#define SOPC_FREERTOS_UDP_RAM_BASE ((void*) 0x30007000) // DTCM + 28K (=32 - 4 K)

// Here, the default values can be overridden for this specific card.
#define CONFIG_SOPC_PUBLISHER_ADDRESS "opc.udp://232.1.2.100:4840"
#define CONFIG_SOPC_SUBSCRIBER_ADDRESS "opc.udp://232.1.2.101:4840"
// #define CONFIG_SOPC_PUBSUB_SECURITY_MODE SOPC_SecurityMode_SignAndEncrypt
#define CONFIG_SOPC_PUBSUB_SECURITY_MODE SOPC_SecurityMode_None

// Tailor S2OPC lib to minimize impact in memory
#define SOPC_DEFAULT_TCP_UA_MAX_BUFFER_SIZE (SOPC_TCP_UA_MIN_BUFFER_SIZE + 2048)
#define SOPC_DEFAULT_SEND_MAX_NB_CHUNKS 1
#define SOPC_DEFAULT_RECEIVE_MAX_MESSAGE_LENGTH SOPC_DEFAULT_TCP_UA_MAX_BUFFER_SIZE
#define SOPC_DEFAULT_RECEIVE_MAX_NB_CHUNKS 1
#define SOPC_DEFAULT_MAX_ARRAY_LENGTH 128
#define SOPC_MAX_SECURE_CONNECTIONS 6
#define SOPC_MAX_SOCKETS 10
#define SOPC_MAX_SESSIONS 5
#define SOPC_MAX_PENDING_REQUESTS 64

#define SOPC_PUBSUB_BUFFER_SIZE 256
#define SOPC_PUBSUB_MAX_PUBLISHER_PER_SCHEDULER 1
#define SOPC_PUBSUB_MAX_MESSAGE_PER_PUBLISHER 4

#define SOPC_MAX_NB_ELEMENTS_ASYNC_QUEUE 16
#define SOPC_MAX_SESSION_SUBSCRIPTIONS 3

#endif /* S2OPC_SAMPLES_EMBEDDED_CLI_PUBSUB_SERVER_SRC_BOARDS_NUCLEO_H723ZG_H_ */
