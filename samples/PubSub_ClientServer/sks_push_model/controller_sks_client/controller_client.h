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
 * Client's API, split client functionalities in smaller chunks.
 */

#ifndef CLIENT_H_
#define CLIENT_H_

#include "libs2opc_client_config.h"
#include "sopc_key_manager.h"
#include "sopc_sk_builder.h"
#include "sopc_sk_manager.h"

/* TODO multi key : add an internal structure to keep configuration and session number */

typedef struct Client_SKS_GetKeys_Response
{
    SOPC_String* SecurityPolicyUri;
    uint32_t FirstTokenId;
    SOPC_ByteString* Keys;
    uint32_t NbKeys;
    uint32_t TimeToNextKey;
    uint32_t KeyLifetime;
} Client_SKS_GetKeys_Response;

typedef enum
{
    SESSION_CONN_FAILED = -1,
    SESSION_CONN_CLOSED,
    SESSION_CONN_NEW,
    SESSION_CONN_CONNECTED,
    SESSION_CONN_MSG_RECEIVED,
} SessionConnectedState;

/*
 * \brief Start Client service. To be called before Client_SetSecurityKeys()
 */
void Client_Start(void);

/*
 * \brief Stop Client service.
 */
void Client_Stop(void);

/*
 * \brief Initialize and configure the client application
 */
SOPC_ReturnStatus Client_Initialize(void);

/*
 * \brief Clear Client application.
 */
void Client_Clear(void);

/*
 * \brief Add a new Secure Connection Configuration to connect to a SKS server.
 *        If \p endpoint_url already used for another SC configuration, existing configuration is returned.
 *        Security Policy is Basic256Sha256.
 * \param endpoint_url     Endpoint Url of the SKS Server
 * \param server_cert      A server certificate
 * \return                 A Secure connection configuration or NULL if failed.
 */
SOPC_SecureConnection_Config* Client_AddSecureConnectionConfig(const char* endpoint_url,
                                                               SOPC_SerializedCertificate* server_cert);

SOPC_ReturnStatus Client_SetSecurityKeys(SOPC_SecureConnection_Config* config,
                                         const char* securityGroupId,
                                         SOPC_String* SecurityPolicyUri,
                                         uint32_t CurrentTokenId,
                                         SOPC_ByteString* CurrentKey,
                                         uint32_t NbKeys,
                                         SOPC_ByteString* FutureKeys,
                                         uint32_t TimeToNextKey,
                                         uint32_t KeyLifetime);

/*
 * \brief  Create an instance of SOPC_SKProvider to get Keys using SKS Get Security Keys request
 *
 * \param SecureChannel_Id    Endpoint connection configuration index provided by Client_AddSecureConnectionConfig()
 * \return a SOPC_SKProvider object or NULL if not enough memory
 */
SOPC_SKProvider* Client_Provider_BySKS_Create(SOPC_SecureConnection_Config* config);

// session identifier
extern uint32_t g_session;
// used to identify the active session response
extern uintptr_t g_Client_SessionContext;
// Session state (SessionConnectedState)
extern int32_t g_scState;
// indicates that request send failed
extern int32_t g_sendFailures;

#endif // CLIENT_H_
