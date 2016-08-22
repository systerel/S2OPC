/*
 * secure_channel_client_connection.h
 *
 *  Created on: Jul 22, 2016
 *      Author: vincent
 */

#ifndef INGOPCS_SECURE_CHANNEL_CLIENT_CONNECTION_H_
#define INGOPCS_SECURE_CHANNEL_CLIENT_CONNECTION_H_

#include <wrappers.h>

#include <opcua_ingopcs_types.h>
#include <private_key.h>
#include <secure_channel_low_level.h>

typedef struct PendingRequest
{
    uint32_t           requestId; // 0 is invalid request
    uint32_t           timeoutHint;
    uint32_t           startTime;
    Response_Event_CB* callback;
    void*              callbackData;
} PendingRequest;

typedef struct SC_Channel_Client_Connection
{
    Namespace*                namespaces;
    EncodeableType*           encodeableTypes;
    PKIProvider*              pkiProvider;
    UA_Byte_String*           serverCertificate;
    UA_Byte_String*           clientCertificate;
    Private_Key*              clientKey;
    uint32_t                  nbPendingRequests; // array size
    PendingRequest*           pendingRequests; //replace by a linked list impl
    Msg_Security_Mode         securityMode;
    UA_String*                securityPolicy;
    uint32_t                  requestedLifetime;
    SecureChannel_Connection* instance;
    P_Timer                   watchdogTimer;
    SC_Connection_Event_CB*   callback;
    void*                     callbackData;

} SC_Channel_Client_Connection;


SC_Channel_Client_Connection* Create_Client_Channel(Namespace*      namespac,
                                                    EncodeableType* encodeableTypes);
void Delete_Client_Channel(SC_Channel_Client_Connection* scConnection);

StatusCode Connect_Client_Channel(SC_Channel_Client_Connection* connection,
                                  char*                         uri,
                                  void*                         pkiConfig,
                                  UA_Byte_String*               clientCertificate,
                                  UA_Byte_String*               clientKey,
                                  UA_Byte_String*               serverCertificate,
                                  Msg_Security_Mode             securityMode,
                                  char*                         securityPolicy,
                                  uint32_t                      requestedLifetime,
                                  SC_Connection_Event_CB*       callback,
                                  void*                         callbackData
                                  );

StatusCode Send_Open_Secure_Channel_Request(SC_Channel_Client_Connection* cConnection);
StatusCode Receive_Open_Secure_Channel_Response(SC_Channel_Client_Connection* cConnection,
                                                UA_Msg_Buffer* transportMsgBuffer);


#endif /* INGOPCS_SECURE_CHANNEL_CLIENT_CONNECTION_H_ */
