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
#include <singly_linked_list.h>
#include <ua_builtintypes.h>
#include <ua_encodeable.h>
#include <ua_namespace_table.h>
#include <ua_secure_channel_low_level.h>


typedef struct SC_ClientConnection
{
    UA_NamespaceTable      namespaces;
    UA_EncodeableType**    encodeableTypes;
    PKIProvider*           pkiProvider;
    UA_ByteString          serverCertificate;
    UA_ByteString          clientCertificate;
    PrivateKey             clientKey;
    SLinkedList*           pendingRequests;
    UA_MessageSecurityMode securityMode;
    UA_String              securityPolicy;
    uint32_t               requestedLifetime;
    SC_Connection*         instance;
    P_Timer                watchdogTimer;
    SC_ConnectionEvent_CB* callback;
    void*                  callbackData;

} SC_ClientConnection;

typedef StatusCode (SC_ResponseEvent_CB) (SC_ClientConnection* connection,
                                          void*                response,
                                          UA_EncodeableType*   responseType,
                                          void*                callbackData,
                                          StatusCode           status);

typedef struct PendingRequest
{
    uint32_t             requestId; // 0 is invalid request
    UA_EncodeableType*   responseType;
    uint32_t             timeoutHint;
    uint32_t             startTime;
    SC_ResponseEvent_CB* callback;
    void*                callbackData;
} PendingRequest;

PendingRequest* SC_PendingRequestCreate(uint32_t             requestId,
                                        UA_EncodeableType*   responseType,
                                        uint32_t             timeoutHint,
                                        uint32_t             startTime,
                                        SC_ResponseEvent_CB* callback,
                                        void*                callbackData);

void SC_PendingRequestDelete(PendingRequest*);


SC_ClientConnection* SC_Client_Create(UA_NamespaceTable*  namespaceTable,
                                      UA_EncodeableType** encodeableTypes);
void SC_Client_Delete(SC_ClientConnection* scConnection);

StatusCode SC_Client_Connect(SC_ClientConnection*   connection,
                             char*                  uri,
                             void*                  pkiConfig,
                             UA_ByteString*         clientCertificate,
                             UA_ByteString*         clientKey,
                             UA_ByteString*         serverCertificate,
                             UA_MessageSecurityMode securityMode,
                             char*                  securityPolicy,
                             uint32_t               requestedLifetime,
                             SC_ConnectionEvent_CB* callback,
                             void*                  callbackData);

StatusCode SC_Send_Request(SC_ClientConnection* connection,
                           UA_EncodeableType*   requestType,
                           void*                request,
                           UA_EncodeableType*   responseType,
                           uint32_t             timeout,
                           SC_ResponseEvent_CB* callback,
                           void*                callbackData);


#endif /* INGOPCS_SECURE_CHANNEL_CLIENT_CONNECTION_H_ */
