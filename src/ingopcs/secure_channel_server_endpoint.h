/*
 * secure_channel_server_endpoint.h
 *
 *  Created on: Jul 26, 2016
 *      Author: vincent
 */

#ifndef INGOPCS_SECURE_CHANNEL_SERVER_ENDPOINT_H_
#define INGOPCS_SECURE_CHANNEL_SERVER_ENDPOINT_H_

typedef OpcUa_Endpoint_PfnEndpointCallback SC_Endpoint_Event_CB;

typedef enum SC_Endpoint_State
{
    SC_Endpoint_Opened,
    SC_Endpoint_Closed,
    SC_Endpoint_Error
} SC_Endpoint_State;

typedef struct SecurityPolicy
{
    Msg_Security_Mode securityMode;
    UA_String         SecurityPolicy;
} SecurityPolicy;

typedef struct SC_Channel_Server_Endpoint
{
    Namespace                 namespaces;
    EncodeableType*           encodeableTypes;
    PKIProvider               pkiProvider;
    UA_Byte*                  serverCertificate;
    Private_Key               serverKey;
    SC_Endpoint_State         state;
    SecurityPolicy*           securityPolicies;
    uint32_t                  lastSecureChannelId;
    SecureChannel_Connection* secureChannelConnections;
    TCP_UA_Listener*          transportConnection;
    Timer                     watchdogTimer;
    SC_Endpoint_Event_CB*     callback;
    void*                     callbackData;

} SC_Channel_Client_Connection;


#endif /* INGOPCS_SECURE_CHANNEL_SERVER_ENDPOINT_H_ */
