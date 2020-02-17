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

#ifndef S2OPC_CSRC_HELPERS_PLATFORM_DEP_ZEPHYR_P_LOG_SERVER_H_
#define S2OPC_CSRC_HELPERS_PLATFORM_DEP_ZEPHYR_P_LOG_SERVER_H_

#include <stdbool.h>
#include <stdlib.h>

#include <inttypes.h>

#include "sopc_enums.h"

#define SOPC_LOGSRV_INVALID_HANDLE (UINT32_MAX)

// Log server handle
typedef uint32_t SOPC_LogServer_Handle;

// Log server print.
SOPC_ReturnStatus SOPC_LogServer_Print(SOPC_LogServer_Handle handle, // Server instance handle
                                       const uint8_t* value,         // Data to log
                                       uint32_t size,
                                       bool bIncludeDate); // Data size

// Destruction of log server. Handle is set to invalid handle value
SOPC_ReturnStatus SOPC_LogServer_Destroy(SOPC_LogServer_Handle* pHandle);

// Creation of new log server
SOPC_ReturnStatus SOPC_LogServer_Create(SOPC_LogServer_Handle* pHandle, // Returned log server instance handle
                                        uint32_t port);                 // TCP port between 60 and 120

#endif /* S2OPC_CSRC_HELPERS_PLATFORM_DEP_ZEPHYR_P_LOG_SERVER_H_ */
