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
