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
 * \brief Common internal interface for different client libraries
 *
 * The functions of this interface are threadsafe, except stated otherwise.
 */

#ifndef LIBS2OPC_CLIENT_COMMON_H_
#define LIBS2OPC_CLIENT_COMMON_H_

#include <stdint.h>
#include "libs2opc_client.h"
#include "libs2opc_client_cmds.h"

/*
 * ================
 * TYPE DEFINITIONS
 * ================
 */

/**
 * @brief
 *   Callback type for discovery events
 * @param requestStatus
 *   status code of the request
 * @param response
 *   discovery request response
 * @param responseContext
 *   context of the response
 * @warning
 *   ServiceFault can be retrieve instead of the expected response
 */
typedef void SOPC_ClientCommon_DiscoveryCbk(const SOPC_StatusCode requestStatus,
                                            const void* response,
                                            const uintptr_t responseContext);

/*
 ===================
 SERVICES DEFINITION
 =================== */

/**
 @brief
    Configure the library. This function shall be called once by the host application
    before any other service can be used.
    It shall be done after a call to ::SOPC_CommonHelper_Initialize
 @warning
    This function is not threadsafe.
 @param pCfg
    Non null pointer to the static configuration. The content of the configuration is copied
    and the object pointed by /p pCfg can be freed by the caller.
 @param cbkGetEndpoints
    GetEndpoints request callback, NULL if not used
 @return
    The operation status */
SOPC_ReturnStatus SOPC_ClientCommon_Initialize(const SOPC_LibSub_StaticCfg* pCfg,
                                               SOPC_ClientCommon_DiscoveryCbk* const cbkGetEndpoints);

/**
 @brief
    Clears the connections and configurations.
    It shall be done before a call to ::SOPC_CommonHelper_Clear
 @warning
    As this function should be called only once, it is not threadsafe. */
void SOPC_ClientCommon_Clear(void);

/**
 @brief
    Configure a future connection. The given /p pCfgId is later used to create connections.
 @param pCfg
    Non null pointer to the connection configuration. The content of the configuration is copied
    and the object pointed by /p pCfg can be freed by the caller.
 @param[out] pCfgId
    The configuration connection id. Set when the value returned is "SOPC_STATUS_OK".
 @return
    The operation status */
SOPC_ReturnStatus SOPC_ClientCommon_ConfigureConnection(const SOPC_LibSub_ConnectionCfg* pCfg,
                                                        SOPC_LibSub_ConfigurationId* pCfgId);

/**
 @brief
    Creates a new connection to a remote OPC server from configuration id cfg_id.
    The connection represent the whole client and is later identified by the returned cli_id.
    The function waits until the client is effectively connected, or the Toolkit times out.
 @param cfgId
    The parameters of the connection to create, return by SOPC_ClientCommon_ConfigureConnection().
 @param[out] pCliId
    The connection id of the newly created client, set when returned status is SOPC_STATUS_OK
 @return
    The operation status and SOPC_STATUS_TIMEOUT when connection hanged for more than
    connection_cfg->timeout_ms milliseconds
 @warning
    The disconnect callback might be called before the function returns if connection succeeds and then fails
    immediately (in this case *pCliId is already set)
 */
SOPC_ReturnStatus SOPC_ClientCommon_Connect(const SOPC_LibSub_ConfigurationId cfgId, SOPC_LibSub_ConnectionId* pCliId);

/**
 * @brief
 *    Create a subscription.
 * @param cliId
 *    The connection id.
 * @param cbkWrapper
 *    The data change callback with wrapper style
 * @return
 *    The operation status.
 */
SOPC_ReturnStatus SOPC_ClientCommon_CreateSubscription(const SOPC_LibSub_ConnectionId cliId,
                                                       SOPC_ClientHelper_DataChangeCbk cbkWrapper);
/**
 * @brief
 *    deletes a subscription
 * @param cliId
 *    The connection id.
 * @return
 *    The operation status.
 */
SOPC_ReturnStatus SOPC_ClientCommon_DeleteSubscription(const SOPC_LibSub_ConnectionId cliId);

/**
 @brief
    Add variables to the subscription of the connection.
    This call is synchroneous: it waits for the server response, or the Toolkit times out.
    The connection timeout is also used for this function.
 @param cliId
    The connection id.
 @param lszNodeId
    An array of zero-terminated strings describing the NodeIds to add.
    It should be at least \p nElements long.
 @param lattrId
    An array of attributes id. The subscription is created for the attribute lAttrId[i]
    for the node id lszNodeId[i].
    It should be at least \p nElements long.
 @param nElements
    The number of elements in previous arrays.
 @param[out] lDataId
    A pre-allocated array to the output unique variable data identifiers.
    It should be at least \p nElements long.
    The values will be used in call to SOPC_LibSub_DataChangeCbk data_change_callback.
 @param[out] results
    A pre-allocated response structure that contains the detailed result for each element added.
    It should be used to check if some elements failed to be added to the subscription in the server
    or to be aware of revised values provided by the server.
    It might be NULL if the caller does not need this information
 @return
    The operation status. lDataId is only valid when the return status is SOPC_STATUS_OK.
    SOPC_STATUS_TIMEOUT is returned when the timeout expires before receiving a response. */
SOPC_ReturnStatus SOPC_ClientCommon_AddToSubscription(const SOPC_LibSub_ConnectionId cliId,
                                                      const SOPC_LibSub_CstString* lszNodeId,
                                                      const SOPC_LibSub_AttributeId* lattrId,
                                                      int32_t nElements,
                                                      SOPC_LibSub_DataId* lDataId,
                                                      OpcUa_CreateMonitoredItemsResponse* results);

/**
 * @brief
 *    Delete subscription
 * @param cliId
 *    The connection id.
 * @return
 *    The operation status.
 */
SOPC_ReturnStatus SOPC_ClientCommon_DeleteSubscription(const SOPC_LibSub_ConnectionId cliId);

/**
 @brief
    Sends a generic request on the connection. The request must be accepted by the SOPC encoders
 (OpcUa_<MessageStruct>*) which are defined in "sopc_types.h". Upon response, the SOPC_ClientCommon_EventCbk callback
 configured with this connection is called with the OpcUa response.
 @param cliId
    The connection id.
 @param requestStruct
    OPC UA message payload structure pointer (OpcUa_<MessageStruct>*). Deallocated by toolkit.
 @param requestContext
    A context value, it will be provided in the callback alongside the corresponding response.
 */
SOPC_ReturnStatus SOPC_ClientCommon_AsyncSendRequestOnSession(SOPC_LibSub_ConnectionId cliId,
                                                              void* requestStruct,
                                                              uintptr_t requestContext);

/**
 * @brief
 *    sends a GetEndpoints request
 * @param endpointUrl
 *    url of the endpoint
 * @param requestContext
 *    context of the request. It will be passed alongside the response.
 * @return
 *    the operation status
 */
SOPC_ReturnStatus SOPC_ClientCommon_AsyncSendGetEndpointsRequest(const char* endpointUrl, uintptr_t requestContext);

/**
 @brief
    Disconnect from a remote OPC server.
    The function waits until the client is effectively disconnected, or the Toolkit times out.
 @param cliId
    The connection id to disconnect
 @return
    The operation status. Erroneous case are:
    - unitialized or unconfigured toolkit (SOPC_STATUS_INVALID_STATE),
    - inexisting connection (SOPC_STATUS_INVALID_PARAMETERS),
    - already closed connection (SOPC_STATUS_NOK). */
SOPC_ReturnStatus SOPC_ClientCommon_Disconnect(const SOPC_LibSub_ConnectionId cliId);

/**
 * \brief Define client preferred locales ids from an array of locale strings.
 *
 * \param nbLocales  The number of locales defined in the array. It might be 0 if no locale defined (only default exist)
 * \param localeIds  The array of locales in priority order for localized strings to be returned by server.
 *                   Array and its content is copied by function.
 *
 * \return SOPC_STATUS_OK in case of success, otherwise SOPC_STATUS_INVALID_PARAMETERS
 *         if \p localeIds is invalid when \p nbLocales \> 0
 *         or SOPC_STATUS_INVALID_STATE if the configuration is not possible.
 *
 * \warning It shall not be called once a connection is established
 */
SOPC_ReturnStatus SOPC_ClientCommon_SetLocaleIds(size_t nbLocales, char** localeIds);

/**
 * \brief Define client application description
 *
 * \param applicationUri        The globally unique identifier for the application instance.
 * \param productUri            The globally unique identifier for the product.
 * \param defaultAppName        The name of the application using the default locale language.
 * \param defaultAppNameLocale  The default locale if any. If defined it shall exists in preferred locales.
 * \param applicationType       The type of application, it shall be one of the OpcUa_ApplicationType_Client* types
 *
 * \return SOPC_STATUS_OK in case of success, otherwise SOPC_STATUS_INVALID_PARAMETERS
 *         if \p applicationUri, \p productUri or \p defaultAppName are invalid
 *         or SOPC_STATUS_INVALID_STATE if the configuration is not possible.
 *
 * \warning It shall not be called once a connection is established
 *
 */
SOPC_ReturnStatus SOPC_ClientCommon_SetApplicationDescription(const char* applicationUri,
                                                              const char* productUri,
                                                              const char* defaultAppName,
                                                              const char* defaultAppNameLocale,
                                                              OpcUa_ApplicationType applicationType);

#endif /* LIBS2OPC_CLIENT_COMMON_H_ */
