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

/** \file
 *
 * \brief High level interface to run an OPC UA server.
 *
 * Once the server is configured using functions of libs2opc_server_config.h,
 * the server should be started using ::SOPC_ServerHelper_StartServer (or ::SOPC_ServerHelper_Serve).
 * Until it is stopped by call to ::SOPC_ServerHelper_StopServer or due to an error (listening address busy, etc.),
 * the server is then accessible for connections by OPC UA client applications
 * and local access/modification of address space content by server application itself.
 * This is done using same OPC UA services client are using but in a local way called "local services" in this server
 * API trough ::SOPC_ServerHelper_LocalServiceAsync (or ::SOPC_ServerHelper_LocalServiceSync)
 *
 * \note Local services:
 *  Local services are restricted to a minimal set of services to access or modify address space content without any
 * access restriction. This is the way to access or modify data of the server in the server application itself.
 *
 * \note When server is started, the configuration is freezed and the Server node content is initialized
 *
 */

#ifndef LIBS2OPC_SERVER_H_
#define LIBS2OPC_SERVER_H_

#include <stdbool.h>
#include <stdint.h>

#include "libs2opc_server_config.h"

/**
 * \brief Type of callback called when server stopped
 *
 * \warning No blocking operation shall be done in callback
 *
 * \param status  Indicates the return status of server, SOPC_STATUS_OK if stopped on purpose.
 */
typedef void SOPC_ServerStopped_Fct(SOPC_ReturnStatus status);

/**
 * \brief Start the server asynchronously.
 *        Server information node is updated and endpoints are asynchronously requested to be opened.
 *
 * \param stoppedCb  callback called when server will stop (on purpose or due to endpoint opening isssue), it is
 *                   mandatory to define it and to wait it is called after call to ::SOPC_ServerHelper_StopServer.
 *
 * \return SOPC_STATUS_OK in case of success, otherwise SOPC_STATUS_INVALID_STATE
 *         if the configuration is not possible (toolkit not initialized, server already started).
 */
SOPC_ReturnStatus SOPC_ServerHelper_StartServer(SOPC_ServerStopped_Fct* stoppedCb);

/**
 * \brief Call to stop the server started with SOPC_ServerHelper_StartServer.
 *        If server started with ::SOPC_ServerHelper_StartServer, this call is blocking during shutdown phase and
 *        ::SOPC_ServerStopped_Fct is called on actual shutdown. Caller is responsible to wait for
 *        ::SOPC_ServerStopped_Fct call prior to use any ::SOPC_HelperConfigServer_Clear function.
 *        If server started with ::SOPC_ServerHelper_Serve, this call is asynchronous and server is actually stopped on
 *        ::SOPC_ServerHelper_Serve return (after shutdown phase).
 *
 * \return SOPC_STATUS_OK in case of success, otherwise SOPC_STATUS_INVALID_STATE if the server is not running.
 *
 * \note Server stops after configured seconds for shutdown phase (see ::SOPC_HelperConfigServer_SetShutdownCountdown )
 *       to indicate shutdown in ServerState node
 */
SOPC_ReturnStatus SOPC_ServerHelper_StopServer(void);

/**
 * \brief Run the server synchronously (alternative to ::SOPC_ServerHelper_StartServer)
 *        Server information node is updated and endpoints are opened.
 *        Run until requested to stop (call to ::SOPC_ServerHelper_StopServer or signal stop if active) or on endpoint
 *        opening error.
 *
 * \param catchSigStop  If set to true, the stop signal (Ctrl+C) is caught and server shutdown initiated.
 *                      When flag is set the server will stop on first event occurring between
 *                      stop signal and call to ::SOPC_ServerHelper_StopServer.
 *
 * \return SOPC_STATUS_OK in case server stopped on purpose, SOPC_STATUS_INVALID_STATE
 *         if the configuration is not possible (toolkit not initialized, server already started),
 *         otherwise error status.
 *
 * \note Server stops after configured seconds for shutdown phase (see ::SOPC_HelperConfigServer_SetShutdownCountdown )
 *       to indicate shutdown in ServerState node
 *
 */
SOPC_ReturnStatus SOPC_ServerHelper_Serve(bool catchSigStop);

/**
 * \brief Executes a local OPC UA service on server (read, write, browse or discovery service) asynchronously.
 *        On local service response callback configured through ::SOPC_HelperConfigServer_SetLocalServiceAsyncResponse
 *        will be called.
 *
 * \note ::SOPC_ServerHelper_StartServer or ::SOPC_ServerHelper_Serve shall have been called
 *       and the server shall still running
 *
 * \note Local services are not restricted by AccessLevel attribute value but only Value attribute is modifiable.
 *
 * \param request   An instance of on of the following OPC UA request:
 *                  - ::OpcUa_ReadRequest
 *                  - ::OpcUa_WriteRequest
 *                  - ::OpcUa_BrowseRequest
 *                  - ::OpcUa_TranslateBrowsePathsToNodeIdsRequest
 *                  - ::OpcUa_GetEndpointsRequest
 *                  - ::OpcUa_FindServersRequest
 *                  - ::OpcUa_FindServersOnNetworkRequest
 *                  - ::OpcUa_RegisterServer2Request
 *
 * \param userContext  User defined context that will be provided with the corresponding response in
 *                     ::SOPC_LocalServiceAsyncResp_Fct
 *
 * \return SOPC_STATUS_OK in case of success, otherwise SOPC_STATUS_INVALID_STATE if the server is not running.
 *
 * \note request memory is managed by the server after a successful return
 */
SOPC_ReturnStatus SOPC_ServerHelper_LocalServiceAsync(void* request, uintptr_t userContext);

/**
 * \brief Executes a local OPC UA service on server (read, write, browse or discovery service) synchronously.
 *
 * \note ::SOPC_ServerHelper_StartServer or ::SOPC_ServerHelper_Serve shall have been called
 *       and the server shall still running
 *
 * \note Local services are not restricted by AccessLevel attribute value but only Value attribute is modifiable.
 *
 * \param request   An instance of on of the following OPC UA request:
 *                  - ::OpcUa_ReadRequest
 *                  - ::OpcUa_WriteRequest
 *                  - ::OpcUa_BrowseRequest
 *                  - ::OpcUa_TranslateBrowsePathsToNodeIdsRequest
 *                  - ::OpcUa_GetEndpointsRequest
 *                  - ::OpcUa_FindServersRequest
 *                  - ::OpcUa_FindServersOnNetworkRequest
 *                  - ::OpcUa_RegisterServer2Request
 *                  Note: it shall be allocated on heap since it will be freed by S2OPC library during treatment
 * \param[out] response  Pointer into which instance of response complying with the OPC UA request is provided:
 *                       - ::OpcUa_ReadResponse
 *                       - ::OpcUa_WriteResponse
 *                       - ::OpcUa_BrowseResponse
 *                       - ::OpcUa_TranslateBrowsePathsToNodeIdsResponse
 *                       - ::OpcUa_GetEndpointsResponse
 *                       - ::OpcUa_FindServersResponse
 *                       - ::OpcUa_FindServersOnNetworkResponse
 *                       - ::OpcUa_RegisterServer2Response
 *                       e.g.: OpcUa_ReadResponse* resp = NULL; SOPC_ServerHelper_LocalServiceSync(request, &resp);
 *
 * \return SOPC_STATUS_OK in case of success, SOPC_STATUS_INVALID_STATE if the server is not running otherwise
 *         SOPC_STATUS_TIMEOUT if ::SOPC_HELPER_LOCAL_RESPONSE_TIMEOUT_MS is reached before response provided.
 *
 * \note request memory is managed by the server after a successful return or in case of timeout
 * \note caller is responsible of output response memory after successful call
 *
 * \warning local service synchronous call shall only be called from the application thread and shall not be called from
 * server callbacks used for notification, asynchronous response, client event, etc. (::SOPC_LocalServiceAsyncResp_Fct,
 * ::SOPC_WriteNotif_Fct,  ::SOPC_ComEvent_Fct, etc.). Otherwise this will lead to a temporary deadlock situation which
 * will lead to fail with SOPC_STATUS_TIMEOUT.
 */
SOPC_ReturnStatus SOPC_ServerHelper_LocalServiceSync(void* request, void** response);

#endif
