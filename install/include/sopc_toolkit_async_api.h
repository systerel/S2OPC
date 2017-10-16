/*
 *  Copyright (C) 2017 Systerel and others.
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

/**
 * \file
 *
 * \brief This module provides an asynchronous API to request toolkit services
 *        It is required to configure the toolkit before calling any service.
 *        Service responses are always provided asynchronously through the callback
 *        defined during toolkit configuration.
 */

#ifndef _SOPC_TOOLKIT_ASYNC_API_H
#define _SOPC_TOOLKIT_ASYNC_API_H

#include <stdint.h>

#include "sopc_user_app_itf.h"

/**
 * \brief Request to open a connection listener for the given endpoint description configuration as a server.
 *        In case of failure the SE_CLOSED_ENDPOINT event will be triggered to SOPC_ComEvent_Fct(),
 *        otherwise the listener could be considered as opened.
 *
 * \param endpointDescriptionIdx  Endpoint description configuration index provided by SOPC_ToolkitServer_AddEndpointConfig()
 *
 */
void SOPC_ToolkitServer_AsyncOpenEndpoint(uint32_t endpointDescriptionIdx);


/**
 * \brief Request to close a connection listener for the given endpoint description configuration.
 *        In any case the SE_CLOSED_ENDPOINT event will be triggered to SOPC_ComEvent_Fct(),
 *        once triggered if the listener was opened it could be now considered closed.
 *
 * \param endpointDescriptionIdx  Endpoint description configuration index provided to SOPC_ToolkitServer_AsyncOpenEndpoint()
 *
 */
void SOPC_ToolkitServer_AsyncCloseEndpoint(uint32_t endpointDescriptionIdx);

/**
 * \brief Request to activate a new session for the given endpoint connection configuration as client.
 *        When requesting activation of a session the following steps are automatically done:
 *        - Establish a new secure channel for the endpoint connection configuration provided if not existing
 *        - When secure channel established, request creation of a session
 *        - When session is created, request activation of the session
 *        - When session is activated, notify session is active
 *
 *        In case of failure SE_SESSION_ACTIVATION_FAILURE event will be triggered to SOPC_ComEvent_Fct(),
 *        otherwise SE_ACTIVATED_SESSION event will be triggered when session is activated provided the session Id
 *        for other operations on session.
 *
 *  Note: since current activation is limited to anonymous user, no user parameter can be provided
 *
 * \param endpointConnectionIdx  Endpoint connection configuration index provided by SOPC_ToolkitClient_AddSecureChannelConfig()
 *
 */
void SOPC_ToolkitClient_AsyncActivateSession(uint32_t endpointConnectionIdx);

/**
 * \brief Request to send a service request on given active session.
 *        In case of service response received, the SE_RCV_SESSION_RESPONSE event will be triggered to SOPC_ComEvent_Fct().
 *
 * \param sessionId      Session Id (provided by event SE_ACTIVATED_SESSION) on which the service request shall be sent
 * \param requestStruct  OPC UA message payload structure pointer (OpcUa_<MessageStruct>*)
 *
 */
void SOPC_ToolkitClient_AsyncSendRequestOnSession(uint32_t sessionId,
                                                  void*    requestStruct);

/**
 * \brief Request to close the given session.
 *        When the session is closed, the SE_CLOSED_SESSION event will be triggered to SOPC_ComEvent_Fct().
 *
 * \param sessionId      Session Id (provided by event SE_ACTIVATED_SESSION) on which the service request shall be sent
 */
void SOPC_ToolkitClient_AsyncCloseSession(uint32_t sessionId);


/*void SOPC_ToolkitClient_AsyncSendDiscoveryRequest(uint32_t endpointConnectionIdx,
                                                  void*    requestStruct);*/

#endif /* _SOPC_TOOLKIT_ASYNC_API_H */
