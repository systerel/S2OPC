/*
 * secure_channel_client_connection.h
 *
 *  Created on: Jul 22, 2016
 *      Author: vincent
 */

#ifndef INGOPCS_SECURE_CHANNEL_CLIENT_CONNECTION_H_
#define INGOPCS_SECURE_CHANNEL_CLIENT_CONNECTION_H_

#include <secret_buffer.h>
#include <pki.h>
#include <singly_linked_list.h>
#include <ua_builtintypes.h>
#include <ua_encodeable.h>
#include <ua_namespace_table.h>
#include <ua_secure_channel_low_level.h>

struct SC_ClientConnection;

typedef SOPC_StatusCode (SC_ConnectionEvent_CB)(struct SC_ClientConnection* cConnection,
                                           void*                       cbData,
                                           SC_ConnectionEvent          event,
                                           SOPC_StatusCode                  status);

typedef struct SC_ClientConnection
{
    SOPC_NamespaceTable         namespaces;
    SOPC_EncodeableType**       encodeableTypes;
    const PKIProvider*        pkiProvider;
    const Certificate*        serverCertificate;
    const Certificate*        clientCertificate;
    AsymmetricKey*            clientKey;
    SLinkedList*              pendingRequests;
    OpcUa_MessageSecurityMode securityMode;
    SOPC_String                 securityPolicy;
    uint32_t                  requestedLifetime;
    SC_Connection*            instance;
    P_Timer                   watchdogTimer;
    SC_ConnectionEvent_CB*    callback;
    void*                     callbackData;

} SC_ClientConnection;

typedef SOPC_StatusCode (SC_ResponseEvent_CB) (SC_ClientConnection* connection,
                                          void*                response,
                                          SOPC_EncodeableType*   responseType,
                                          void*                callbackData,
                                          SOPC_StatusCode           status);

typedef struct PendingRequest
{
    uint32_t             requestId; // 0 is invalid request
    SOPC_EncodeableType*   responseType;
    uint32_t             timeoutHint;
    uint32_t             startTime;
    SC_ResponseEvent_CB* callback;
    void*                callbackData;
} PendingRequest;

PendingRequest* SC_PendingRequestCreate(uint32_t             requestId,
                                        SOPC_EncodeableType*   responseType,
                                        uint32_t             timeoutHint,
                                        uint32_t             startTime,
                                        SC_ResponseEvent_CB* callback,
                                        void*                callbackData);

void SC_PendingRequestDelete(PendingRequest*);


SC_ClientConnection* SC_Client_Create();
SOPC_StatusCode SC_Client_Configure(SC_ClientConnection* cConnection,
                               SOPC_NamespaceTable*   namespaceTable,
                               SOPC_EncodeableType**  encodeableTypes);

SC_ClientConnection* SC_Client_CreateAndConfigure(SOPC_NamespaceTable*  namespaceTable,
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

SOPC_StatusCode SC_Send_Request(SC_ClientConnection* connection,
                           SOPC_EncodeableType*   requestType,
                           void*                request,
                           SOPC_EncodeableType*   responseType,
                           uint32_t             timeout,
                           SC_ResponseEvent_CB* callback,
                           void*                callbackData);


#endif /* INGOPCS_SECURE_CHANNEL_CLIENT_CONNECTION_H_ */
