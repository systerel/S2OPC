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
SOPC_ReturnStatus Server_LoadAddressSpace(void);
/** Calls Toolkit_Configured(), starts the server */
SOPC_ReturnStatus Server_StartServer(void);
bool Server_IsRunning(void);

/** Intercat with Pub module */
struct publisherDsmIdentifier
{
    SOPC_Conf_PublisherId pubId;
    uint16_t writerGroupId;
    uint16_t dataSetWriterId;
    bool enableEmission;
};

struct networkMessageIdentifier
{
    SOPC_Conf_PublisherId pubId;
    uint16_t writerGroupId;
};

SOPC_ReturnStatus Server_WritePubSubNodes(void);
bool Server_Trigger_Publisher(struct networkMessageIdentifier networkMessageId);
void Server_StopAndClear(void);

void Server_PubSubStop_RequestRestart(void);
bool Server_PubSubStop_Requested(void);
bool Server_PubSubStart_Requested(void);
struct networkMessageIdentifier Server_PubAcyclicSend_Requested(void);

struct publisherDsmIdentifier Server_PubFilteringDataSetMessage_Requested(void);

bool Server_Trigger_FilteringDsmEmission(struct publisherDsmIdentifier pubDsmId);
/* Interacting with the Sub module */

SOPC_Array* Server_GetConfigurationPaths(void); /* Returns an array of char* */
void Server_SetSubStatusAsync(SOPC_PubSubState state);
void Server_SetSubStatusSync(SOPC_PubSubState state);

bool Server_SetTargetVariables(const OpcUa_WriteValue* nodesToWrite, const int32_t nbValues);
SOPC_DataValue* Server_GetSourceVariables(const OpcUa_ReadValueId* lrv, const int32_t nbValues);

#endif /* SERVER_H_ */
