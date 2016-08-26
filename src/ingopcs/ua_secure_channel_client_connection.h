/*
 * secure_channel_client_connection.h
 *
 *  Created on: Jul 22, 2016
 *      Author: vincent
 */

#ifndef INGOPCS_SECURE_CHANNEL_CLIENT_CONNECTION_H_
#define INGOPCS_SECURE_CHANNEL_CLIENT_CONNECTION_H_

#include <wrappers.h>

#include <private_key.h>
#include "ua_secure_channel_low_level.h"
#include "ua_types.h"

typedef struct PendingRequest
{
    uint32_t          requestId; // 0 is invalid request
    uint32_t          timeoutHint;
    uint32_t          startTime;
    ResponseEvent_CB* callback;
    void*             callbackData;
} PendingRequest;

typedef struct SC_ClientConnection
{
    Namespace*             namespaces;
    EncodeableType*        encodeableTypes;
    PKIProvider*           pkiProvider;
    UA_ByteString*         serverCertificate;
    UA_ByteString*         clientCertificate;
    PrivateKey*            clientKey;
    uint32_t               nbPendingRequests; // array size
    PendingRequest*        pendingRequests; //replace by a linked list impl
    MsgSecurityMode        securityMode;
    UA_String*             securityPolicy;
    uint32_t               requestedLifetime;
    SC_Connection*         instance;
    P_Timer                watchdogTimer;
    SC_ConnectionEvent_CB* callback;
    void*                  callbackData;

} SC_ClientConnection;


SC_ClientConnection* SC_Client_Create(Namespace*      namespac,
                                      EncodeableType* encodeableTypes);
void SC_Client_Delete(SC_ClientConnection* scConnection);

StatusCode SC_Client_Connect(SC_ClientConnection*   connection,
                             char*                  uri,
                             void*                  pkiConfig,
                             UA_ByteString*         clientCertificate,
                             UA_ByteString*         clientKey,
                             UA_ByteString*         serverCertificate,
                             MsgSecurityMode        securityMode,
                             char*                  securityPolicy,
                             uint32_t               requestedLifetime,
                             SC_ConnectionEvent_CB* callback,
                             void*                  callbackData);

#endif /* INGOPCS_SECURE_CHANNEL_CLIENT_CONNECTION_H_ */
