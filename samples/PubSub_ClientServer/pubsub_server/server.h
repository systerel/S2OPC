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
#include "sopc_pub_source_variable.h"
#include "sopc_sub_scheduler.h"
#include "sopc_user_app_itf.h"

/* Running the Server */

SOPC_ReturnStatus Server_Initialize(void);
/* SOPC_ReturnStatus Server_SetRuntimeVariables(void); */ /* Future */
SOPC_ReturnStatus Server_CreateServerConfig(SOPC_S2OPC_Config* output_s2opcConfig);
SOPC_ReturnStatus Server_LoadAddressSpace(void);
/** Calls Toolkit_Configured(), starts the server */
SOPC_ReturnStatus Server_ConfigureStartServer(SOPC_Endpoint_Config* pEpConfig);
bool Server_IsRunning(void);
SOPC_ReturnStatus Server_WritePubSubNodes(void);
void Server_StopAndClear(SOPC_S2OPC_Config* pConfig);

bool Server_PubSubStop_Requested(void);
bool Server_PubSubStart_Requested(void);

/* Interacting with the Sub module */

SOPC_Array* Server_GetConfigurationPaths(void); /* Returns an array of char* */
void Server_SetSubStatus(SOPC_PubSubState state);
bool Server_SetTargetVariables(OpcUa_WriteValue* nodesToWrite, int32_t nbValues);
SOPC_DataValue* Server_GetSourceVariables(OpcUa_ReadValueId* lrv, int32_t nbValues);

#endif /* SERVER_H_ */
