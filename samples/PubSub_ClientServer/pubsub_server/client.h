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
 * \brief Start Client service. To be called before Client_GetSecurityKeys()
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

/*
 * \brief Get Security Keys using a Security Keys Service.
 *  This function creates a Session and sends a Call Method request.
 *  It returns when the response is received or on timeout.
 *  Service should be started with Client_Start() before calling this function.
 *  If Client_GetSecurityKeys() is running when Client_Stop() is called, it returns SOPC_STATUS_INVALID_STATE.
 *
 * \param SecureChannel_Id    Endpoint connection configuration index provided by Client_AddSecureConnectionConfig()
 * \param securityGroupId     SecurityGroupId to use as parameter of the method
 * \param StartingTokenId     Requested Starting Token ID
 * \param RequestedKeys       Number of requested Keys
 * \param response            A valid pointer of a Structure where received data will be copied
 *
 * \return                    SOPC_STATUS_OK in case of success, error status otherwise
 */
SOPC_ReturnStatus Client_GetSecurityKeys(SOPC_SecureConnection_Config* config,
                                         const char* securityGroupId,
                                         uint32_t StartingTokenId,
                                         uint32_t requestedKeys,
                                         Client_SKS_GetKeys_Response* response);

/*
 * To be called only by Toolkit callback when received response of Client_GetSecurityKeys() request
 */
void Client_Treat_Session_Response(void* param, uintptr_t appContext);

/*
 * \brief  Create an instance of SOPC_SKProvider to get Keys using SKS Get Security Keys request
 *
 * \param SecureChannel_Id    Endpoint connection configuration index provided by Client_AddSecureConnectionConfig()
 * \return a SOPC_SKProvider object or NULL if not enough memory
 */
SOPC_SKProvider* Client_Provider_BySKS_Create(SOPC_SecureConnection_Config* config);

/*
 * \brief Fallback provider that retrieve default data keys available (static or local file)
 *
 */
SOPC_SKProvider* Fallback_Provider_Create(void);

// session identifier
extern uint32_t g_session;
// used to identify the active session response
extern uintptr_t g_Client_SessionContext;
// Session state (SessionConnectedState)
extern int32_t g_scState;
// indicates that request send failed
extern int32_t g_sendFailures;

#endif // CLIENT_H_
