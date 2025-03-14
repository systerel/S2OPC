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
 * \brief Requests to open a connection listener for the given endpoint description configuration as a server.
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
 * \brief Requests to close a connection listener for the given endpoint description configuration.
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
 * \brief Requests to execute locally the given service request on server and receive response.
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
 * \brief Triggers the given event from the given node as notifier
 *
 * \param notifierNodeId      NodeId of the node notifier for the triggered event
 * \param event               The event to be triggered
 * \param optSessionId        (optional) The sessionId for which the event is triggered or 0.
 * \param optSubscriptionId   (optional) The subscriptionId for which the event is triggered or 0.
 *                            When both \p optSessionId and \p optSubscriptionId are set,
 *                            the event is triggered only if the subscription is part of the given session
 *                            otherwise the event is not triggered.
 * \param optMonitoredItemId  (optional) The monitored item Id for which the event is triggered or 0.
 *
 * Note: the provided event and its content is automatically deallocated by the toolkit
 */
void SOPC_ToolkitServer_TriggerEvent(const SOPC_NodeId* notifierNodeId,
                                     SOPC_Event* event,
                                     SOPC_SessionId optSessionId,
                                     uint32_t optSubscriptionId,
                                     uint32_t optMonitoredItemId);

/**
 * \brief Requests to re-evaluate the current server secure channels due to server certificate / key update (force SC
 * re-establishment) or server PKI trust list update (client certificate re-validation necessary) When \p ownCert is set
 * it concerns a certificate / key application update, otherwise it concerns a PKI trust list update.
 *
 *        This shall be triggered in case of server certificate / key update ( \p ownCert = true)
 *        or when an server PKI trust list update occurred ( \p ownCert = false).
 *
 *        If server certificate changed, the secure channels using it are closed.
 *        If server PKI trustlist changed and client certificate is not valid or trusted anymore, the secure channel is
 * closed.
 *
 * \param ownCert  It shall be true when server certificate / key update occurred and false when server PKI trust list
 * occurred
 */
void SOPC_ToolkitServer_AsyncReEvalSecureChannels(bool ownCert);

/**
 * \brief Request to re-evaluate X509IdentityToken certificates for all active sessions due to
 *        user PKI trustlist update.
 *        If user PKI trustlist changed and user certificate is not valid or trusted anymore,
 *        the associated session is closed.
 */
void SOPC_ToolkitServer_AsyncReEvalUserCertSessions(void);

typedef struct SOPC_EndpointConnectionCfg
{
    SOPC_ReverseEndpointConfigIdx
        reverseEndpointConfigIdx; /*<< Index of the Reverse Endpoint configuration to listen for server connection
                                       returned by ::SOPC_ToolkitClient_AddReverseEndpointConfig().
                                       It shall be 0 for a classic connection and > 0 for a reverse connection. */
    SOPC_SecureChannelConfigIdx
        secureChannelConfigIdx; /*<< Index of the Secure Channel configuration for endpoint connection
                                     returned by ::SOPC_ToolkitClient_AddSecureChannelConfig().
                                     It shall not be 0. */
} SOPC_EndpointConnectionCfg;

/**
 * \brief Creates an endpoint connection configuration for a classic connection (initiated by client)
 *
 * \param secureChannelConfigIdx  Index of the Secure Channel configuration for endpoint connection
 *                                     returned by ::SOPC_ToolkitClient_AddSecureChannelConfig()
 */
SOPC_EndpointConnectionCfg SOPC_EndpointConnectionCfg_CreateClassic(SOPC_SecureChannelConfigIdx secureChannelConfigIdx);

/**
 * \brief Creates an endpoint connection configuration for a reverse connection (initiated by server)
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
 * \brief Requests to activate a new session for the given endpoint connection configuration as client.
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
 * \param userTokenCtx           Context for X509IdentityToken, an allocated SOPC_SerializedAsymmetricKey* is expected
 *                               which will be deallocated by toolkit.
 *                               NULL if \p userToken is not a OpcUa_X509IdentityToken extension object.
 *
 * \return                       SOPC_STATUS_OK in case of success, SOPC_STATUS_INVALID_PARAMETERS or
 *                               SOPC_STATUS_OUT_OF_MEMORY otherwise
 *
 */
SOPC_ReturnStatus SOPC_ToolkitClient_AsyncActivateSession(SOPC_EndpointConnectionCfg endpointConnectionCfg,
                                                          const char* sessionName,
                                                          uintptr_t sessionContext,
                                                          SOPC_ExtensionObject* userToken,
                                                          void* userTokenCtx);

/**
 * \brief Requests to activate an anonymous session. See SOPC_ToolkitClient_AsyncActivateSession()
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
 * \brief Requests to activate a session with a UserNameIdentityToken. See SOPC_ToolkitClient_AsyncActivateSession().
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
 * \brief Requests to activate a session with a x509IdentityToken. See ::SOPC_ToolkitClient_AsyncActivateSession().
 *
 *
 * \param endpointConnectionCfg  Endpoint connection configuration.
 * \param sessionName            (Optional) Human readable string that identifies the session (NULL terminated C string)
 *                               If defined it should be unique for the client.
 * \param sessionContext         A context value, it will be provided in case of session activation or failure
 *                               notification
 * \param policyId               The policy id to use for the identity token, must not be NULL
 * \param pCertX509         Certificate of the X509IdentityToken.
 * \param pKey              A valid pointer to the private key of the X509IdentityToken.
 *                         This object should never be freed by the caller of this function, let the toolkit do it.
 *
 * \return SOPC_STATUS_OK when ::SOPC_ToolkitClient_AsyncActivateSession() is called successfully.
 */
SOPC_ReturnStatus SOPC_ToolkitClient_AsyncActivateSession_Certificate(SOPC_EndpointConnectionCfg endpointConnectionCfg,
                                                                      const char* sessionName,
                                                                      uintptr_t sessionContext,
                                                                      const char* policyId,
                                                                      const SOPC_SerializedCertificate* pCertX509,
                                                                      SOPC_SerializedAsymmetricKey* pKey);

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
 * \brief Requests to close the given session.
 *
 *   When the session is closed, the SE_CLOSED_SESSION event will be triggered to SOPC_ComEvent_Fct().
 *
 * \param sessionId      Session Id (provided by event SE_ACTIVATED_SESSION) on which the service request shall be sent
 */
void SOPC_ToolkitClient_AsyncCloseSession(SOPC_SessionId sessionId);

/**
 * \brief Requests to send a discovery service request without using session.
 *
 *   In case of service response received, the SE_RCV_DISCOVERY_RESPONSE event will be triggered to SOPC_ComEvent_Fct().
 *
 * \param endpointConnectionCfg  Endpoint connection configuration.
 * \param discoveryReqStruct     OPC UA Discovery message request payload structure pointer (OpcUa_<MessageStruct>*).
 *                               Deallocated by toolkit.
 * \param requestContext         A context value, it will be provided with corresponding response or in case of sending
 *                               error notification
 *
 * \return SOPC_STATUS_OK in case of success, SOPC_STATUS_INVALID_PARAMETERS or SOPC_STATUS_OUT_OF_MEMORY otherwise
 *
 * Note: the provided request message structure and its content is automatically deallocated by the toolkit
 */
SOPC_ReturnStatus SOPC_ToolkitClient_AsyncSendDiscoveryRequest(SOPC_EndpointConnectionCfg endpointConnectionCfg,
                                                               void* discoveryReqStruct,
                                                               uintptr_t requestContext);
/**
 * \brief Requests to open a connection listener for the given reverse endpoint description configuration as a client.
 *
 *   In case of failure the SE_CLOSED_ENDPOINT event will be triggered to ::SOPC_ComEvent_Fct,
 *   otherwise the listener must be considered as opened.
 *
 * \param reverseEndpointConfigIdx  Endpoint description configuration index provided by
 *                                  SOPC_ToolkitClient_AddReverseEndpointConfig()
 *
 */
void SOPC_ToolkitClient_AsyncOpenReverseEndpoint(SOPC_ReverseEndpointConfigIdx reverseEndpointConfigIdx);

/**
 * \brief Requests to close a connection listener for the given endpoint description configuration.
 *
 *   In any case the SE_CLOSED_ENDPOINT event will be triggered to ::SOPC_ComEvent_Fct,
 *   once triggered if the listener was opened it must be considered closed.
 *
 * \param reverseEndpointConfigIdx  Endpoint description configuration index provided to
 *                                  SOPC_ToolkitClient_AsyncOpenReverseEndpoint()
 *
 */
void SOPC_ToolkitClient_AsyncCloseReverseEndpoint(SOPC_ReverseEndpointConfigIdx reverseEndpointConfigIdx);

/**
 * \brief Requests to re-evaluate the client secure channels due to client certificate / key update (force SC
 * re-establishment) or client PKI trust list update (server certificate re-validation necessary) When \p ownCert is set
 * it concerns a certificate / key application update, otherwise it concerns a PKI trust list update.
 *
 *        This shall be triggered in case of client certificate / key update ( \p ownCert = true)
 *        or when an client PKI trust list update occurred ( \p ownCert = false).
 *
 *        If client certificate changed, the secure channels using it are closed.
 *        If client PKI trustlist changed and server certificate is not valid or trusted anymore, the secure channel is
 * closed.
 *
 * \param ownCert  It shall be true when client certificate / key update occurred and false when client PKI trust list
 * occurred
 */
void SOPC_ToolkitClient_AsyncReEvalSecureChannels(bool ownCert);

#endif /* SOPC_TOOLKIT_ASYNC_API_H_ */
