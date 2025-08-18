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

#ifndef S2OPC_SAMPLES_EMBEDDED_CLI_PUBSUB_SERVER_SRC_BOARDS_MIMXRT1180_EVK_H_
#define S2OPC_SAMPLES_EMBEDDED_CLI_PUBSUB_SERVER_SRC_BOARDS_MIMXRT1180_EVK_H_

#include <stdint.h>

// SOPC configuration this is actually not depending of the board
// This must be done before other includes
#define WITH_USER_ASSERT 1

#define SOPC_PTR_SIZE 4
#define WITH_USER_ASSERT 1

#define S2OPC_CRYPTO_MBEDTLS 1

#define BOARD_TYPE "MIMXRT1180-EVK"
#define SDK_PROVIDER_NXP

#define DEBUG_CONSOLE_TRANSFER_NON_BLOCKING 1
#define DEBUG_CONSOLE_RX_ENABLE 1

// Board-Specific configuration
#include <errno.h>
#include "FreeRTOSConfig_Gen.h"
#include "freertos_platform_dep.h"
#include "freertos_shell.h"

#if S2OPC_CRYPTO_MBEDTLS
#include "sopc_mbedtls_config.h"

// MbedTLS
#define MBEDTLS_HAVE_TIME

#define MBEDTLS_PLATFORM_TIME_ALT

#define MBEDTLS_NO_PLATFORM_ENTROPY

#ifdef MBEDTLS_TIMING_C
#undef MBEDTLS_TIMING_C
#endif

#ifdef MBEDTLS_NET_C
#undef MBEDTLS_NET_C
#endif

#ifdef MBEDTLS_FS_IO
#undef MBEDTLS_FS_IO
#endif

#ifdef MBEDTLS_PSA_ITS_FILE_C
#undef MBEDTLS_PSA_ITS_FILE_C
#endif

#define MBEDTLS_PLATFORM_MS_TIME_ALT

#ifdef MBEDTLS_PSA_CRYPTO_STORAGE_C
#undef MBEDTLS_PSA_CRYPTO_STORAGE_C
#endif

#ifdef SDIO_ENABLED
#undef SDIO_ENABLED
#endif

#ifdef SD_ENABLED
#undef SD_ENABLED
#endif
#endif // SOPC_MBEDTLS

// FreeRTOS
#define SOPC_FREERTOS_UDP_RAM_BASE ((void*) 0x20508000) // SRAM OC2 + 8000

#ifdef configUSE_TICK_HOOK
#undef configUSE_TICK_HOOK
#endif
#define configUSE_TICK_HOOK 1

#ifdef configCHECK_FOR_STACK_OVERFLOW
#undef configCHECK_FOR_STACK_OVERFLOW
#endif
#define configCHECK_FOR_STACK_OVERFLOW 1

#ifdef configUSE_NEWLIB_REENTRANT
#undef configUSE_NEWLIB_REENTRANT
#endif
#define configUSE_NEWLIB_REENTRANT 1

#define __HeapBase _pvHeapStart
#define __HeapLimit _pvHeapLimit
#define HEAP_SIZE _HeapSize

// LWIP
#define LWIP_DNS 1
#define LWIP_NETIF_LOOPBACK 1
#define LWIP_IGMP 1
#define SO_REUSE 1

#define MEMP_MEM_MALLOC 1
#define MEMP_MEM_INIT 1
#define MEM_LIBC_MALLOC 0
#define LWIP_ALLOW_MEM_FREE_FROM_OTHER_CONTEXT 1

#define LWIP_ERRNO_STDINCLUDE 1

#define NETC_PROMISCUOUS 1

#define LWIP_SUPPORT_CUSTOM_PBUF 1

// Console Related
#define DEBUG_CONSOLE_TRANSFER_NON_BLOCKING 1
#define DEBUG_CONSOLE_RX_ENABLE 1
#include "fsl_debug_console.h"
#undef PRINT
#define PRINT SOPC_Shell_Printf
#undef PRINTF
#define PRINTF SOPC_Shell_Printf

#define OPT_LAZY false

// Here, the default values can be overridden for this specific card.
#define CONFIG_SOPC_PUBLISHER_ADDRESS "opc.udp://232.1.2.100:4840"

#define CONFIG_SOPC_SUBSCRIBER_ADDRESS "opc.udp://232.1.2.101:4840"

#define CONFIG_SOPC_PUBSUB_SECURITY_MODE SOPC_SecurityMode_None

#endif /* S2OPC_SAMPLES_EMBEDDED_CLI_PUBSUB_SERVER_SRC_BOARDS_MIMXRT1180_EVK_H_ */
