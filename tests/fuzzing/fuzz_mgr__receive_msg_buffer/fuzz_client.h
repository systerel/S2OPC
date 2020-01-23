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

#ifndef FUZZ_MGR__RECEIVE_MSG_BUFFER_CLIENT
#define FUZZ_MGR__RECEIVE_MSG_BUFFER_CLIENT

#define NB_SESSIONS 1
#define DESIGNATE_NEW(T, ...) memcpy(SOPC_Malloc(sizeof(T)), &(T const){__VA_ARGS__}, sizeof(T))

#include "fuzz_main.h"

// Prototypage
SOPC_ReturnStatus Wait_response_client();
OpcUa_WriteRequest* newWriteRequest_client(const uint8_t* buff, size_t len);
SOPC_ReturnStatus AddSecureChannelconfig_client();

SOPC_ReturnStatus Setup_client();
SOPC_ReturnStatus Run_client(const uint8_t* buff, size_t len);
SOPC_ReturnStatus Teardown_client();

extern OpcUa_WriteRequest* pWriteReq;

extern t_CerKey ck_cli;
extern uint32_t session;
extern uintptr_t sessionContext[2];
extern SessionConnectedState scState;
extern uint32_t channel_config_idx;

#endif // FUZZ_MGR__RECEIVE_MSG_BUFFER_CLIENT
