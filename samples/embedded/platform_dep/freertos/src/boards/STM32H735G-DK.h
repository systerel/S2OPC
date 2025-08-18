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

#ifndef S2OPC_SAMPLES_EMBEDDED_CLI_PUBSUB_SERVER_SRC_BOARDS_STM32H735G_DK_H_
#define S2OPC_SAMPLES_EMBEDDED_CLI_PUBSUB_SERVER_SRC_BOARDS_STM32H735G_DK_H_

// SOPC configuration this is actually not depending of the board
// This must be done before other includes
#define SOPC_PTR_SIZE 4
#define WITH_USER_ASSERT 1

// Override MBEDTLS_CONFIG_FILE
#ifdef MBEDTLS_CONFIG_FILE
#undef MBEDTLS_CONFIG_FILE
#endif
#define MBEDTLS_CONFIG_FILE "sopc_mbedtls_config.h"

// Board-Specific configuration

#define BOARD_TYPE_STM32H735G -DK
#define SDK_PROVIDER_STM

#include "stm32h7xx_hal.h"

#include "freertos_platform_dep.h"

#include "freertos_shell.h"

#undef PRINT
#define PRINT SOPC_Shell_Printf
#undef PRINTF
#define PRINTF SOPC_Shell_Printf

// Here, the default values can be overridden for this specific card.
#define CONFIG_SOPC_PUBLISHER_ADDRESS "opc.udp://232.1.2.100:4840"
#define CONFIG_SOPC_SUBSCRIBER_ADDRESS "opc.udp://232.1.2.101:4840"
// #define CONFIG_SOPC_PUBSUB_SECURITY_MODE SOPC_SecurityMode_SignAndEncrypt
#define CONFIG_SOPC_PUBSUB_SECURITY_MODE SOPC_SecurityMode_None

#endif /* S2OPC_SAMPLES_EMBEDDED_CLI_PUBSUB_SERVER_SRC_BOARDS_STM32H735G_DK_H_ */
