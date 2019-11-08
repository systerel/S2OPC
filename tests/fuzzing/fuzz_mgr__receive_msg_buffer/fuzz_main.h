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

#ifndef FUZZ_MGR__RECEIVE_MSG_BUFFER
#define FUZZ_MGR__RECEIVE_MSG_BUFFER

#define ENDPOINT_URL "opc.tcp://localhost:4841"
#define APPLICATION_URI "urn:S2OPC:localhost"
#define PRODUCT_URI "urn:S2OPC:localhost"

#include <stdbool.h>

#include "sopc_crypto_profiles.h"
#include "sopc_types.h"

#define ENDPOINT_URL "opc.tcp://localhost:4841"

void Fuzz_Event_Fct(SOPC_App_Com_Event event, uint32_t idOrStatus, void* param, uintptr_t appContext);
SOPC_ReturnStatus SOPC_EpConfig_serv();

typedef struct s_Cerkey
{
    SOPC_SerializedCertificate* Certificate;
    SOPC_SerializedAsymmetricKey* Key;
    SOPC_SerializedCertificate* authCertificate;
    SOPC_PKIProvider* pkiProvider;
} t_CerKey;

typedef enum
{
    SESSION_CONN_FAILED = -1,
    SESSION_CONN_CLOSED,
    SESSION_CONN_NEW,
    SESSION_CONN_CONNECTED,
    SESSION_CONN_MSG_RECEIVED,
} SessionConnectedState;

extern bool debug;
extern int32_t sendFailures;
extern bool secuActive;
extern SOPC_SecureChannel_Config scConfig;

extern SOPC_Endpoint_Config epConfig;
extern SOPC_S2OPC_Config output_s2opcConfig;

#endif // FUZZ_MGR__RECEIVE_MSG_BUFFER
