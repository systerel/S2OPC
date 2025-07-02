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

extern int32_t serverOnline;

/* Running the Server */

/* Event management */
void Server_Treat_Local_Service_Response(SOPC_EncodeableType* type, void* response, uintptr_t userContext);

/* SOPC_ReturnStatus Server_SetRuntimeVariables(void); */ /* Future */
SOPC_ReturnStatus Server_CreateServerConfig(void);
/** Calls Toolkit_Configured(), starts the server */
SOPC_ReturnStatus Server_StartServer(void);
bool Server_IsRunning(void);

/** Intercat with Pub module */

void Server_StopAndClear(void);

/* Interacting with the Sub module */
bool Server_SetTargetVariables(const OpcUa_WriteValue* nodesToWrite, const int32_t nbValues);
SOPC_DataValue* Server_GetSourceVariables(const OpcUa_ReadValueId* lrv, const int32_t nbValues);

#endif /* SERVER_H_ */
