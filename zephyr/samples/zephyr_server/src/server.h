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

/** \file
 *
 * Server's API, split server functionalities in smaller chunks.
 */

#ifndef SERVER_H_
#define SERVER_H_

#include <stdbool.h>

#include "sopc_array.h"
#include "sopc_builtintypes.h"
#include "sopc_event_handler.h"
#include "sopc_log_manager.h"
#include "sopc_pub_source_variable.h"
#include "sopc_sub_scheduler.h"
#include "sopc_user_app_itf.h"

#define APPLICATION_URI "urn:S2OPC:localhost"
#define PRODUCT_URI "urn:S2OPC:localhost"
#define SERVER_DESCRIPTION "S2OPC Zephyr demo Server"

/* Running the Server */

SOPC_ReturnStatus Server_Initialize(const SOPC_Log_Configuration* pLogCfg);
/* SOPC_ReturnStatus Server_SetRuntimeVariables(void); */ /* Future */
SOPC_ReturnStatus Server_CreateServerConfig(SOPC_S2OPC_Config* output_s2opcConfig);
SOPC_ReturnStatus Server_LoadAddressSpace(void);
/** Calls Toolkit_Configured(), starts the server */
SOPC_ReturnStatus Server_ConfigureStartServer(SOPC_Endpoint_Config* pEpConfig);
void Server_Interrupt(void);
bool Server_IsRunning(void);
void Server_StopAndClear(SOPC_S2OPC_Config* pConfig);

/* Writes into Address space */
bool Server_LocalWriteSingleNode(SOPC_NodeId* pNid, SOPC_DataValue* pDv);

#endif /* SERVER_H_ */
