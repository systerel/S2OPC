/*
 * secure_channel_server_endpoint.h
 *
 *  Created on: Jul 26, 2016
 *      Author: vincent
 */

#ifndef INGOPCS_SECURE_CHANNEL_SERVER_ENDPOINT_H_
#define INGOPCS_SECURE_CHANNEL_SERVER_ENDPOINT_H_

typedef enum SC_EndpointState
{
    SC_Endpoint_Opened,
    SC_Endpoint_Closed,
    SC_Endpoint_Error
} SC_EndpointState;

typedef struct SecurityPolicy
{
    OpcUa_MessageSecurityMode securityMode;
    SOPC_String                 SecurityPolicy;
} SecurityPolicy;

typedef void* SC_EndpointEvent_CB;

typedef struct SC_ServerEndpoint
{
    SOPC_NamespaceTable*   namespaces;
    SOPC_EncodeableType*   encodeableTypes;
    PKIProvider          pkiProvider;
    SOPC_Byte*             serverCertificate;
    SecretBuffer*        serverKey;
    SC_EndpointState     state;
    SecurityPolicy*      securityPolicies;
    uint32_t             lastSecureChannelId;
    SC_Connection*       secureChannelConnections;
    TCP_SOPC_Listener*     transportConnection;
    P_Timer              watchdogTimer;
    SC_EndpointEvent_CB* callback;
    void*                callbackData;

} SC_ServerEndpoint;


#endif /* INGOPCS_SECURE_CHANNEL_SERVER_ENDPOINT_H_ */
