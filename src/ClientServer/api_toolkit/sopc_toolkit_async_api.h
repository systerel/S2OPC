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

/**
 * \file
 *
 * \brief This module provides an asynchronous API to request toolkit services
 *        It is required to configure the toolkit before calling any service.
 *
 *   Service responses are always provided asynchronously through the callback
 *   defined during toolkit configuration.
 */

#ifndef SOPC_TOOLKIT_ASYNC_API_H_
#define SOPC_TOOLKIT_ASYNC_API_H_

#include <stdbool.h>
#include <stdint.h>

#include "sopc_builtintypes.h"
#include "sopc_enums.h"
#include "sopc_toolkit_config.h"
#include "sopc_user_app_itf.h"

/**
 * \brief Request to open a connection listener for the given endpoint description configuration as a server.
 *
 *   In case of failure the SE_CLOSED_ENDPOINT event will be triggered to SOPC_ComEvent_Fct(),
 *   otherwise the listener could be considered as opened.
 *
 * \param endpointConfigIdx  Endpoint description configuration index provided by
 *                           ::SOPC_ToolkitServer_AddEndpointConfig()
 *
 */
void SOPC_ToolkitServer_AsyncOpenEndpoint(SOPC_EndpointConfigIdx endpointConfigIdx);

/**
 * \brief Request to close a connection listener for the given endpoint description configuration.
 *
 *   In any case the SE_CLOSED_ENDPOINT event will be triggered to ::SOPC_ComEvent_Fct(),
 *   once triggered if the listener was opened it could be now considered closed.
 *
 * \param endpointConfigIdx  Endpoint description configuration index provided to
 *                           ::SOPC_ToolkitServer_AsyncOpenEndpoint()
 *
 */
void SOPC_ToolkitServer_AsyncCloseEndpoint(SOPC_EndpointConfigIdx endpointConfigIdx);

/**
 * \brief Request to execute locally the given service request on server and receive response.
 *
 *   The SE_LOCAL_SERVICE_RESPONSE event will be triggered to SOPC_ComEvent_Fct(),
 *   once service request evaluated.
 *
 * \param endpointConfigIdx  Endpoint description configuration index provided to
 * \param requestStruct      OPC UA message payload structure pointer (OpcUa_<MessageStruct>*).
 *                           Deallocated by the toolkit.
 * \param requestContext     A context value, it will be provided with corresponding response
 *
 * Note: the provided request message structure and its content is automatically deallocated by the toolkit
 */
void SOPC_ToolkitServer_AsyncLocalServiceRequest(SOPC_EndpointConfigIdx endpointConfigIdx,
                                                 void* requestStruct,
                                                 uintptr_t requestContext);

/**
 * \brief Enumerated type for a connection to a server Endpoint
 */
typedef enum SOPC_EndpointConnectionType
{
    SOPC_EndpointConnectionType_Classic, /*<< Classic connection to server endpoint initiated by client */
    SOPC_EndpointConnectionType_Reverse, /*<< Reverse connection to server endpoint initiated by server */
} SOPC_EndpointConnectionType;

/**
 * \brief Configuration parameters for a classic connection to a server endpoint
 */
typedef struct SOPC_EndpointConnection_Classic
{
    SOPC_SecureChannelConfigIdx
        secureChannelConfigIdx; /*<< Index of the Secure Channel configuration for endpoint connection
                                     returned by ::SOPC_ToolkitClient_AddSecureChannelConfig() */
} SOPC_EndpointConnection_Classic;

/**
 * \brief Configuration parameters for a reverse connection to a server endpoint
 */
typedef struct SOPC_EndpointConnection_Reverse
{
    SOPC_ReverseEndpointConfigIdx
        reverseEndpointConfigIdx; /*<< Index of the Reverse Endpoint configuration to listen for server connection
                                       returned by ::SOPC_ToolkitClient_AddReverseEndpointConfig() */
    SOPC_SecureChannelConfigIdx
        secureChannelConfigIdx; /*<< Index of the Secure Channel configuration for endpoint connection
                                    returned by ::SOPC_ToolkitClient_AddSecureChannelConfig() */
} SOPC_EndpointConnection_Reverse;

/**
 * \brief Configuration parameters for a connection to a server endpoint.
 *        The connection is either initiated by the client (classic) or by the server (reverse).
 */
typedef struct SOPC_EndpointConnectionCfg
{
    SOPC_EndpointConnectionType connectionType;
    union {
        SOPC_EndpointConnection_Classic classic;
        SOPC_EndpointConnection_Reverse reverse;
    } data;
} SOPC_EndpointConnectionCfg;

/**
 * \brief Create an endpoint connection configuration for a classic connection (initiated by client)
 *
 * \param secureChannelConfigIdx  Index of the Secure Channel configuration for endpoint connection
 *                                     returned by ::SOPC_ToolkitClient_AddSecureChannelConfig()
 */
SOPC_EndpointConnectionCfg SOPC_EndpointConnectionCfg_CreateClassic(SOPC_SecureChannelConfigIdx secureChannelConfigIdx);

/**
 * \brief Create an endpoint connection configuration for a reverse connection (initiated by server)
 *
 * \param reverseEndpointConfigIdx     Index of the Reverse Endpoint configuration to listen for server connection
 *                                     returned by ::SOPC_ToolkitClient_AddReverseEndpointConfig()
 * \param secureChannelConfigIdx  Index of the Secure Channel configuration for endpoint connection returned by
 * ::SOPC_ToolkitClient_AddSecureChannelConfig()
 */
SOPC_EndpointConnectionCfg SOPC_EndpointConnectionCfg_CreateReverse(
    SOPC_ReverseEndpointConfigIdx reverseEndpointConfigIdx,
    SOPC_SecureChannelConfigIdx secureChannelConfigIdx);

/**
 * \brief Request to activate a new session for the given endpoint connection configuration as client.
 *
 *   When requesting activation of a session the following steps are automatically done:
 *   - Establish a new secure channel for the endpoint connection configuration provided if not existing
 *   - When secure channel established, request creation of a session
 *   - When session is created, request activation of the session
 *   - When session is activated, notify session is active
 *
 *   In case of failure SE_SESSION_ACTIVATION_FAILURE event will be triggered to ::SOPC_ComEvent_Fct(),
 *   otherwise SE_ACTIVATED_SESSION event will be triggered when session is activated provided the session Id
 *   for other operations on session.
 *
 *   See helper functions ::SOPC_ToolkitClient_AsyncActivateSession_Anonymous(),
 *   ::SOPC_ToolkitClient_AsyncActivateSession_UsernamePassword().
 *
 * \param endpointConnectionCfg  Endpoint connection configuration.
 * \param sessionName            (Optional) Human readable string that identifies the session (NULL terminated C string)
 *                               If defined it should be unique for the client.
 * \param sessionContext         A context value, it will be provided in case of session activation or failure
 *                               notification
 * \param userToken              An extension object, containing either an OpcUa_AnonymousIdentityToken, a
 *                               OpcUa_UserNameIdentityToken, or a OpcUa_X509IdentityToken. This object is borrowed by
 *                               the Toolkit and shall not be freed or modified by the caller.
 *
 * \return                       SOPC_STATUS_OK in case of success, SOPC_STATUS_INVALID_PARAMETERS or
 *                               SOPC_STATUS_OUT_OF_MEMORY otherwise
 *
 */
SOPC_ReturnStatus SOPC_ToolkitClient_AsyncActivateSession(SOPC_EndpointConnectionCfg endpointConnectionCfg,
                                                          const char* sessionName,
                                                          uintptr_t sessionContext,
                                                          SOPC_ExtensionObject* userToken);

/**
 * \brief Request to activate an anonymous session. See SOPC_ToolkitClient_AsyncActivateSession()
 *
 * \param endpointConnectionCfg  Endpoint connection configuration.
 * \param sessionName            (Optional) Human readable string that identifies the session (NULL terminated C string)
 *                               If defined it should be unique for the client.
 * \param sessionContext         A context value, it will be provided in case of session activation or failure
 *                               notification
 * \param policyId               The policy id to use for the identity token.
 *
 * \return SOPC_STATUS_OK when SOPC_ToolkitClient_AsyncActivateSession() is called successfully.
 */
SOPC_ReturnStatus SOPC_ToolkitClient_AsyncActivateSession_Anonymous(SOPC_EndpointConnectionCfg endpointConnectionCfg,
                                                                    const char* sessionName,
                                                                    uintptr_t sessionContext,
                                                                    const char* policyId);

/**
 * \brief Request to activate a session with a UserNameIdentityToken. See SOPC_ToolkitClient_AsyncActivateSession().
 *
 * \note The password will be encrypted, or not, depending on the user token security policy associated to the policyId
 *       or if it is empty depending on the SecureChannel security policy.
 *
 * \warning The UserNamePassword mode should never be used in the following cases
 *          since the password will be sent as plain text to the server:
 *          - SecureChannel security policy is None and user token security policy is None or empty
 *          - SecureChannel security mode is Sign only and user token security policy is None
 *
 * \param endpointConnectionCfg  Endpoint connection configuration.
 * \param sessionName            (Optional) Human readable string that identifies the session (NULL terminated C string)
 *                               If defined it should be unique for the client.
 * \param sessionContext         A context value, it will be provided in case of session activation or failure
 *                               notification
 * \param policyId               The policy id to use for the identity token, must not be NULL
 * \param username               The zero-terminated string username, may be NULL
 * \param password               The bytestring containing the password, may be NULL
 * \param length_password        The password length, ignored when password is NULL
 *
 * \return SOPC_STATUS_OK when SOPC_ToolkitClient_AsyncActivateSession() is called successfully.
 */
SOPC_ReturnStatus SOPC_ToolkitClient_AsyncActivateSession_UsernamePassword(
    SOPC_EndpointConnectionCfg endpointConnectionCfg,
    const char* sessionName,
    uintptr_t sessionContext,
    const char* policyId,
    const char* username,
    const uint8_t* password,
    int32_t length_password);

/**
 * \brief Request to send a service request on given active session.
 *
 *   In case of service response received, the SE_RCV_SESSION_RESPONSE event will be triggered to ::SOPC_ComEvent_Fct().
 *
 * \param sessionId      Session Id (provided by event SE_ACTIVATED_SESSION) on which the service request shall be sent
 * \param requestStruct  OPC UA message payload structure pointer (OpcUa_<MessageStruct>*). Deallocated by toolkit.
 * \param requestContext A context value, it will be provided with corresponding response or in case of sending error
 * notification
 *
 * Note: the provided request message structure and its content is automatically deallocated by the toolkit
 */
void SOPC_ToolkitClient_AsyncSendRequestOnSession(SOPC_SessionId sessionId,
                                                  void* requestStruct,
                                                  uintptr_t requestContext);

/**
 * \brief Request to close the given session.
 *
 *   When the session is closed, the SE_CLOSED_SESSION event will be triggered to SOPC_ComEvent_Fct().
 *
 * \param sessionId      Session Id (provided by event SE_ACTIVATED_SESSION) on which the service request shall be sent
 */
void SOPC_ToolkitClient_AsyncCloseSession(SOPC_SessionId sessionId);

/**
 * \brief Request to send a discovery service request without using session.
 *
 *   In case of service response received, the SE_RCV_DISCOVERY_RESPONSE event will be triggered to SOPC_ComEvent_Fct().
 *
 * \param endpointConnectionCfg  Endpoint connection configuration.
 * \param discoveryReqStruct     OPC UA Discovery message request payload structure pointer (OpcUa_<MessageStruct>*).
 *                               Deallocated by toolkit.
 * \param requestContext         A context value, it will be provided with corresponding response or in case of sending
 *                               error notification
 *
 * \return true in case of success, false otherwise
 *
 * Note: the provided request message structure and its content is automatically deallocated by the toolkit
 */
bool SOPC_ToolkitClient_AsyncSendDiscoveryRequest(SOPC_EndpointConnectionCfg endpointConnectionCfg,
                                                  void* discoveryReqStruct,
                                                  uintptr_t requestContext);
/**
 * \brief Request to open a connection listener for the given reverse endpoint description configuration as a client.
 *
 *   In case of failure the SE_CLOSED_ENDPOINT event will be triggered to SOPC_ComEvent_Fct(),
 *   otherwise the listener could be considered as opened.
 *
 * \param reverseEndpointConfigIdx  Endpoint description configuration index provided by
 *                                  SOPC_ToolkitClient_AddReverseEndpointConfig()
 *
 */
void SOPC_ToolkitClient_AsyncOpenReverseEndpoint(SOPC_ReverseEndpointConfigIdx reverseEndpointConfigIdx);

/**
 * \brief Request to close a connection listener for the given endpoint description configuration.
 *
 *   In any case the SE_CLOSED_ENDPOINT event will be triggered to SOPC_ComEvent_Fct(),
 *   once triggered if the listener was opened it could be now considered closed.
 *
 * \param reverseEndpointConfigIdx  Endpoint description configuration index provided to
 *                                  SOPC_ToolkitClient_AsyncOpenReverseEndpoint()
 *
 */
void SOPC_ToolkitClient_AsyncCloseReverseEndpoint(SOPC_ReverseEndpointConfigIdx reverseEndpointConfigIdx);

#endif /* SOPC_TOOLKIT_ASYNC_API_H_ */
