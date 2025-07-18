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
//#define SOPC_PTR_SIZE 4
#define WITH_USER_ASSERT 1

#define SOPC_PTR_SIZE 4
#define WITH_USER_ASSERT 1

#define BOARD_TYPE "MIMXRT1180-EVK"
#define SDK_PROVIDER_NXP
#define PRINT SOPC_Shell_Printf

// Board-Specific configuration
#include "FreeRTOSConfig_Gen.h"
#include "freertos_platform_dep.h"
#include "freertos_shell.h"
#include "sopc_mbedtls_config.h"
#include <errno.h>

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

// FreeRTOS
#define SOPC_FREERTOS_UDP_RAM_BASE ((void*) 0x30007000) // DTCM + 28K (=32 - 4 K)

#define ConfigFRTOS_MEMORY_SCHEME \
    5 // This macro is deprecated from the SDK but used here to tell platform dep we are using heap 5

#ifdef configUSE_TICK_HOOK
#undef configUSE_TICK_HOOK
#endif
#define configUSE_TICK_HOOK 1

#ifdef INCLUDE_uxTaskGetStackHighWaterMark
#undef INCLUDE_uxTaskGetStackHighWaterMark
#endif
#define INCLUDE_uxTaskGetStackHighWaterMark 1

#ifdef configUSE_NEWLIB_REENTRANT
#undef configUSE_NEWLIB_REENTRANT
#endif
#define configUSE_NEWLIB_REENTRANT 1

#define __HeapBase _pvHeapStart
#define __HeapLimit  _pvHeapLimit
#define HEAP_SIZE _HeapSize

// LWIP
#define LWIP_DNS 1

#define LWIP_IGMP 1

#define LWIP_IPV6 0

#define LWIP_NETIF_LOOPBACK 1

#define LWIP_SINGLE_NETIF 0

#define SO_REUSE 1

#define LWIP_MULTICAST_TX_OPTIONS 1

#define MEMP_MEM_MALLOC 0
#define MEMP_MEM_INIT 0
#define MEM_LIBC_MALLOC 0

// Disable hardware-assisted checksum
#define CHECKSUM_GEN_IP 1
#define CHECKSUM_GEN_UDP 1
#define CHECKSUM_GEN_TCP 1
#define CHECKSUM_GEN_ICMP 1
#define CHECKSUM_GEN_ICMP6 1
#define CHECKSUM_CHECK_ICMP 1
#define CHECKSUM_CHECK_ICMP6 1
#define IP_REASSEMBLY 0
#define IP_FRAG 0

#include "lwipopts.h"

//needs to be seet after "lwipopts.h" because the overwrite guard are broken
#define LWIP_ERRNO_STDINCLUDE 1

#ifdef LWIP_PROVIDE_ERRNO
#undef LWIP_PROVIDE_ERRNO
#endif

#define NETC_PROMISCUOUS 1

#define LWIP_SUPPORT_CUSTOM_PBUF 1

#define OPT_LAZY false

// Here, the default values can be overridden for this specific card.
#define CONFIG_SOPC_PUBLISHER_ADDRESS "opc.udp://232.1.2.100:4840"

#define CONFIG_SOPC_SUBSCRIBER_ADDRESS "opc.udp://232.1.2.101:4840"

// #define CONFIG_SOPC_PUBSUB_SECURITY_MODE SOPC_SecurityMode_SignAndEncrypt
#define CONFIG_SOPC_PUBSUB_SECURITY_MODE SOPC_SecurityMode_None

#endif /* S2OPC_SAMPLES_EMBEDDED_CLI_PUBSUB_SERVER_SRC_BOARDS_MIMXRT1180_EVK_H_ */
