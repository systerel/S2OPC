/*
 *  Copyright (C) 2016 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SOPC_SECURE_CHANNEL_CLIENT_CONNECTION_H_
#define SOPC_SECURE_CHANNEL_CLIENT_CONNECTION_H_

#include "secret_buffer.h"
#include "pki.h"
#include "singly_linked_list.h"
#include "sopc_builtintypes.h"
#include "sopc_encodeabletype.h"
#include "sopc_namespace_table.h"
#include "sopc_secure_channel_low_level.h"
#include "sopc_mutexes.h"

struct SC_ClientConnection;

typedef SOPC_StatusCode (SC_ConnectionEvent_CB)(struct SC_ClientConnection* cConnection,
                                                void*                       cbData,
                                                SC_ConnectionEvent          event,
                                                SOPC_StatusCode             status);

typedef struct SC_LatestRequestResponseInfo {
    uint32_t             requestId;
    SOPC_EncodeableType* requestType;
} SC_LatestPendingRequestInfo;

typedef struct SC_ClientConnection
{
    SOPC_NamespaceTable         namespaces;
    SOPC_EncodeableType**       encodeableTypes;
    const PKIProvider*          pkiProvider;
    const Certificate*          serverCertificate;
    const Certificate*          clientCertificate;
    AsymmetricKey*              clientKey;
    SLinkedList*                pendingRequests;
    SC_LatestPendingRequestInfo lastPendingRequestInfo;
    OpcUa_MessageSecurityMode   securityMode;
    SOPC_String                 securityPolicy;
    uint32_t                    requestedLifetime;
    SC_Connection*              instance;
    P_Timer                     watchdogTimer;
    SC_ConnectionEvent_CB*      callback;
    void*                       callbackData;
    Mutex                       mutex;

} SC_ClientConnection;

typedef SOPC_StatusCode (SC_ResponseEvent_CB) (SC_ClientConnection* connection,
                                               void*                response,
                                               SOPC_EncodeableType* responseType,
                                               void*                callbackData,
                                               SOPC_StatusCode      status);

SC_ClientConnection* SC_Client_Create();
SOPC_StatusCode SC_Client_Configure(SC_ClientConnection*  cConnection,
                                    SOPC_NamespaceTable*  namespaceTable,
                                    SOPC_EncodeableType** encodeableTypes);

void SC_Client_Delete(SC_ClientConnection* scConnection);

SOPC_StatusCode SC_Client_Connect(SC_ClientConnection*      connection,
                                  const char*               uri,
                                  const PKIProvider*        pki,
                                  const Certificate*        crt_cli,
                                  const AsymmetricKey*      key_priv_cli,
                                  const Certificate*        crt_srv,
                                  OpcUa_MessageSecurityMode securityMode,
                                  const char*               securityPolicy,
                                  uint32_t                  requestedLifetime,
                                  SC_ConnectionEvent_CB*    callback,
                                  void*                     callbackData);

SOPC_StatusCode SC_Client_Disconnect(SC_ClientConnection* cConnection);

SOPC_StatusCode SC_CreateAction_Send_Request(SC_ClientConnection*         connection,
                                             SOPC_EncodeableType*         requestType,
                                             void*                        request,
                                             SOPC_EncodeableType*         responseType,
                                             uint32_t                     timeout,
                                             SC_ResponseEvent_CB*         callback,
                                             void*                        callbackData,
                                             SOPC_Socket_EndOperation_CB* endSendCallback,
                                             void*                        endSendCallbackData);

#endif /* SOPC_SECURE_CHANNEL_CLIENT_CONNECTION_H_ */
