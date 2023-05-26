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
 * \brief High level interface to run an OPC UA client.
 *
 * Once the client is configured using functions of libs2opc_client_config.h,
 * the client should establish connection using ::SOPC_ClientHelperNew_Connect.
 * Until connection is stopped by a call to ::SOPC_ClientHelperNew_CloseConnection or due to an error (listening address
 * busy, etc.), the client application might use the connections.
 * This is done using same OPC UA services client are using but in a local way called
 * "local services" in this client API trough ::SOPC_ClientHelperNew_LocalServiceAsync (or
 * ::SOPC_ClientHelperNew_LocalServiceSync)
 *
 */

#ifndef LIBS2OPC_NEW_CLIENT_H_
#define LIBS2OPC_NEW_CLIENT_H_

#include <stdbool.h>
#include <stdint.h>

#include "sopc_types.h"

#include "libs2opc_client_config.h"

/**
 * \brief Structure representing a secure connection to a server
 */
typedef struct SOPC_ClientConnection SOPC_ClientConnection;

typedef enum
{
    SOPC_ClientConnectionEvent_Disconnected, /* Connection terminated, it will not be established again unless a new
                                                connection attempt is done.
                                                To do a new attempt the following functions shall be called:
                                                - ::SOPC_ClientHelperNew_Disconnect on current connection
                                                - ::SOPC_ClientHelperNew_Connect to create a new connection
                                              */
    SOPC_ClientConnectionEvent_Connected,    /* Connection established (SC & session), only triggered when
                                                ::SOPC_ClientHelperNew_StartConnection is used. */
    SOPC_ClientConnectionEvent_Reconnecting, /* Connection temporarily interrupted, attempt to re-establish connection
                                                on-going.
                                                TODO: not implemented in state machine */
} SOPC_ClientConnectionEvent;

/**
 * \brief Type of callback called on client connection event
 *
 * \warning No blocking operation shall be done in callback
 *
 * \param config  Indicates the connection concerned by the event
 * \param event   The event that occurred on the connection
 * \param status  Indicates the return status of client, SOPC_GoodGenericStatus if stopped on purpose.
 */
typedef void SOPC_ClientConnectionEvent_Fct(SOPC_ClientConnection* config,
                                            SOPC_ClientConnectionEvent event,
                                            SOPC_StatusCode status);

/**
 * \brief Send a discovery request without user session creation and activation and retrieve response synchronously.
 *        Service response callback configured through ::SOPC_HelperConfigClient_SetServiceAsyncResponse will be
 *        called on service response or in case of service request sending failure.
 *
 * \param secConnConfig  The secure connection configuration.
 * \param request   An instance of one of the following OPC UA request:
 *                  - ::OpcUa_FindServersRequest
 *                  - ::OpcUa_FindServersOnNetworkRequest
 *                  - ::OpcUa_GetEndpointsRequest
 *                  - ::OpcUa_RegisterServerRequest
 *                  - ::OpcUa_RegisterServer2Request
 * \param userContext  User defined context that will be provided with the corresponding response in
 *                     ::SOPC_LocalServiceAsyncResp_Fct
 *
 * \return SOPC_STATUS_OK in case of success, SOPC_STATUS_INVALID_PARAMETERS in case of invalid parameters,
 *         otherwise SOPC_STATUS_INVALID_STATE if the client is not running.
 *
 *  \warning If the server endpoint is not a discovery endpoint or an activated session is expected
 *           usual connection and generic services functions shall be used.
 *
 * \warning Caller of this API should wait at least ::SOPC_REQUEST_TIMEOUT_MS milliseconds after calling this function
 *          and prior to call ::SOPC_HelperConfigClient_Clear.
 *          It is necessary to ensure asynchronous context is freed and no memory leak occurs.
 */
SOPC_ReturnStatus SOPC_ClientHelperNew_DiscoveryServiceAsync(SOPC_SecureConnection_Config* secConnConfig,
                                                             void* request,
                                                             uintptr_t userContext);

/**
 * \brief Send a discovery request without user session creation and activation and retrieve response synchronously.
 *
 * \param secConnConfig  The secure connection configuration.
 * \param request   An instance of one of the following OPC UA request:
 *                  - ::OpcUa_FindServersRequest
 *                  - ::OpcUa_FindServersOnNetworkRequest
 *                  - ::OpcUa_GetEndpointsRequest
 *                  - ::OpcUa_RegisterServerRequest
 *                  - ::OpcUa_RegisterServer2Request
 * \param[out] response  Pointer into which instance of response complying with the OPC UA request is provided:
 *                     \li ::OpcUa_FindServersRequest
 *                     \li ::OpcUa_FindServersOnNetworkRequest
 *                     \li ::OpcUa_GetEndpointsRequest
 *                     \li ::OpcUa_GetEndpointsResponse
 *                     \li ::OpcUa_RegisterServerRequest
 *                     \li ::OpcUa_RegisterServer2Request
 *
 *                     In case of service failure the response type is always ::OpcUa_ServiceFault,
 *                     in this case the \c response.encodeableType points to ::OpcUa_ServiceFault_EncodeableType
 *                     and ::SOPC_IsGoodStatus(\c response.ResponseHeader.ServiceResult) is \c false.
 *
 * \return SOPC_STATUS_OK in case of success, SOPC_STATUS_INVALID_PARAMETERS in case of invalid parameters,
 *         SOPC_STATUS_INVALID_STATE if the client is not running. And dedicated status if request sending failed.
 *
 * \note request memory is managed by the client after a successful return or in case of timeout
 * \note caller is responsible of output response memory after successful call
 *
 * \warning If the server endpoint is not a discovery endpoint or an activated session is expected
 *          usual connection and generic services functions shall be used.
 *
 * \warning local service synchronous call shall only be called from the application thread and shall not be called from
 * client callbacks used for notification, asynchronous response, client event, etc. (::SOPC_ServiceAsyncResp_Fct,
 * ::SOPC_DataChangeNotif_Fct,  ::SOPC_ClientConnectionEvent_Fct, etc.). Otherwise this will lead to a deadlock
 * situation.
 */
SOPC_ReturnStatus SOPC_ClientHelperNew_DiscoveryServiceSync(SOPC_SecureConnection_Config* secConnConfig,
                                                            void* request,
                                                            void** response);

/**
 * NOT IMPLEMENTED
 *
 * \brief Starts the connection establishment in a non blocking way (asynchronously).
 *
 * \param config                 the connection
 * \param connectEventCb         callback called on connection event, ::SOPC_ClientConnectionEvent_Connected
 *                               shall be awaited prior to use OPC UA services
 * \param[out] secureConnection  Pointer to the secure connection established when returned value is SOPC_STATUS_OK
 *
 * \return SOPC_STATUS_OK in case of success, otherwise SOPC_STATUS_INVALID_STATE
 *         if the configuration is not possible (toolkit not initialized, connection state unexpected).
 */
/*SOPC_ReturnStatus SOPC_ClientHelperNew_StartConnection(SOPC_SecureChannel_Config* config,
                                                      SOPC_ClientConnectionEvent_Fct* connectEventCb,
                                                      SOPC_ClientSecureConnection** secureConnection);
*/

/**
 * \brief Establishes the connection in a blocking way (synchronously).
 *
 * \param secConnConfig          The secure connection configuration to establish
 * \param connectEventCb         Callback called on connection event, it is only used in case of further disconnection
 * \param[out] secureConnection  Pointer to the secure connection established when returned value is SOPC_STATUS_OK
 *
 * \return SOPC_STATUS_OK in case of success, SOPC_STATUS_CLOSED in case of failure,
 *         SOPC_STATUS_INVALID_PARAMETERS in case of invalid parameters,
 *         otherwise SOPC_STATUS_INVALID_STATE
 *         if the configuration is not possible (toolkit not initialized, connection state unexpected).
 */
SOPC_ReturnStatus SOPC_ClientHelperNew_Connect(SOPC_SecureConnection_Config* secConnConfig,
                                               SOPC_ClientConnectionEvent_Fct* connectEventCb,
                                               SOPC_ClientConnection** secureConnection);

/**
 * \brief Disconnects the connection established with SOPC_ClientHelperNew_Connect in a blocking way (synchronously).
 *
 * \param secureConnection  The secure connection to stop
 * *
 * \return SOPC_STATUS_OK in case of success, SOPC_STATUS_INVALID_PARAMETERS in case of invalid parameters,
 *         otherwise SOPC_STATUS_INVALID_STATE if the connection is already disconnected
 */
SOPC_ReturnStatus SOPC_ClientHelperNew_Disconnect(SOPC_ClientConnection** secureConnection);

/**
 * \brief Executes an OPC UA service on server (read, write, browse or discovery service) asynchronously.
 *        Service response callback configured through ::SOPC_HelperConfigClient_SetServiceAsyncResponse will be
 *        called on service response or in case of service request sending failure.
 *
 * \note ::SOPC_ClientHelperNew_StartConnect or ::SOPC_ClientHelperNew_Connect shall have been called
 *       and the connection shall be still active
 *
 * \param config    The connection configuration
 * \param request   An instance of one of the following OPC UA request:
 *                  - ::OpcUa_ReadRequest
 *                  - ::OpcUa_WriteRequest
 *                  - ::OpcUa_BrowseRequest
 *                  - ::OpcUa_TranslateBrowsePathsToNodeIdsRequest
 *                  - ::OpcUa_GetEndpointsRequest
 *                  - ::OpcUa_FindClientsRequest
 *                  - ::OpcUa_FindClientsOnNetworkRequest
 *                  - ::OpcUa_RegisterClient2Request
 *                  - :: TO BE COMPLETED (MI, etc.)
 *
 * \param userContext  User defined context that will be provided with the corresponding response in
 *                     ::SOPC_LocalServiceAsyncResp_Fct
 *
 * \return SOPC_STATUS_OK in case of success, SOPC_STATUS_INVALID_PARAMETERS in case of invalid parameters,
 *         otherwise SOPC_STATUS_INVALID_STATE if the client is not running.
 *
 * \note request memory is managed by the client after a successful return
 *
 * \warning Caller of this API should wait at least ::SOPC_REQUEST_TIMEOUT_MS milliseconds after calling this function
 *          and prior to call ::SOPC_HelperConfigClient_Clear.
 *          It is necessary to ensure asynchronous context is freed and no memory leak occurs.
 */
SOPC_ReturnStatus SOPC_ClientHelperNew_ServiceAsync(SOPC_ClientConnection* secureConnection,
                                                    void* request,
                                                    uintptr_t userContext);

/**
 * \brief Executes a local OPC UA service on client (read, write, browse or discovery service) synchronously.
 *
 * \note ::SOPC_ClientHelperNew_Connect shall have been called
 *       and the connection shall be still active
 * \param config    The connection configuration
 * \param request   An instance of OPC UA request:
 *                  - ::OpcUa_ReadRequest
 *                  - ::OpcUa_WriteRequest
 *                  - ::OpcUa_BrowseRequest
 *                  - ::OpcUa_TranslateBrowsePathsToNodeIdsRequest
 *                  - ::OpcUa_GetEndpointsRequest
 *                  - ::OpcUa_FindClientsRequest
 *                  - ::OpcUa_FindClientsOnNetworkRequest
 *                  - ::OpcUa_RegisterClient2Request
 *                  - :: TO BE COMPLETED (MI, etc.)
 *                  Note: it shall be allocated on heap since it will be freed by S2OPC library during treatment
 * \param[out] response  Pointer into which instance of response complying with the OPC UA request is provided:
 *                     \li ::OpcUa_ReadResponse
 *                     \li ::OpcUa_WriteResponse
 *                     \li ::OpcUa_BrowseResponse
 *                     \li ::OpcUa_GetEndpointsResponse
 *                     \li ::OpcUa_FindServersResponse
 *                     \li ::OpcUa_FindServersOnNetworkResponse
 *                     \li ::OpcUa_RegisterServer2Response
 *
 *                     In case of service failure the response type is always ::OpcUa_ServiceFault,
 *                     in this case the \c response.encodeableType points to ::OpcUa_ServiceFault_EncodeableType
 *                     and ::SOPC_IsGoodStatus(\c response.ResponseHeader.ServiceResult) is \c false.
 *
 * \return SOPC_STATUS_OK in case of success, SOPC_STATUS_INVALID_PARAMETERS in case of invalid parameters,
 *         SOPC_STATUS_INVALID_STATE if the client is not running. And dedicated status if request sending failed.
 *
 * \note request memory is managed by the client after a successful return or in case of timeout
 * \note caller is responsible of output response memory after successful call. E.g. use ::SOPC_Encodeable_Delete.
 *
 * \warning local service synchronous call shall only be called from the application thread and shall not be called from
 * client callbacks used for notification, asynchronous response, client event, etc. (::SOPC_ServiceAsyncResp_Fct,
 * ::SOPC_DataChangeNotif_Fct,  ::SOPC_ClientConnectionEvent_Fct, etc.). Otherwise this will lead to a deadlock
 * situation.
 */
SOPC_ReturnStatus SOPC_ClientHelperNew_ServiceSync(SOPC_ClientConnection* secureConnection,
                                                   void* request,
                                                   void** response);

typedef struct SOPC_ClientHelperNew_Subscription SOPC_ClientHelperNew_Subscription;

/**
 * NEED for subscription error (from server + timeout on client side to manage)
 * \brief Type of callback called on Subscription Notification
 *
 * \warning No blocking operation shall be done in callback
 *
 * \param config        Indicates the connection concerned by the notification
 * \param subscription  Indicates the subscription concerned by the notification
 * \param status        OPC UA status code, \p publishResp is only valid when ::SOPC_IsGoodStatus
 * \param publishResp   Publish response received for the subscription (use helpers to extract data)
 * \param userParam     User defined parameter
 */
typedef void SOPC_ClientSubscriptionNotification_Fct(SOPC_ClientHelperNew_Subscription* subscription,
                                                     SOPC_StatusCode status,
                                                     OpcUa_PublishResponse* publishResp,
                                                     uintptr_t userParam);

/**
 * Subscription helper: manage only Publish tokens, the rest is in Request/Response helpers
 * Note: limited to 1 per connection, synchronous, all params managed ?
 */
SOPC_ClientHelperNew_Subscription* SOPC_ClientHelperNew_CreateSubscription(
    SOPC_ClientConnection* secureConnection,
    uint32_t nbPublishTokens,
    OpcUa_CreateSubscriptionRequest* subParams,
    SOPC_ClientSubscriptionNotification_Fct* subNotifCb,
    uintptr_t userParam);

/**
 * \brief Gets the secure connection on which the subscription relies on
 */
SOPC_ClientConnection* SOPC_ClientHelperNew_GetSecureConnection(SOPC_ClientHelperNew_Subscription* subscription);

/**
 * FORBIDS SubscriptionId use in generic services with CreateSubscription / DeleteSubscription / PublishRequest
 */
uint32_t SOPC_ClientHelperNew_Subscription_GetSubscriptionId(SOPC_ClientHelperNew_Subscription* subscription);

/* NOT PROVIDED since can be done through generic services
SOPC_ReturnStatus SOPC_ClientHelper_SetSubscriptionPublishingMode(SOPC_SecureChannel_Config* config,
                                                                  SOPC_ClientHelper_Subscription* subscription,
                                                                  bool enable);

SOPC_ReturnStatus SOPC_ClientHelper_ModifySubscription(SOPC_SecureChannel_Config* config,
                                                       SOPC_ClientHelper_Subscription* subscription,
                                                       OpcUa_CreateSubscriptionRequest* newParams);

SOPC_ReturnStatus SOPC_ClientHelper_RepublishSubscription(SOPC_SecureChannel_Config* config,
                                                          SOPC_ClientHelper_Subscription* subscription,
                                                          OpcUa_RepublishRequest* newParams);
*/
/* AddMI / ModifyMI / DeleteMI in same case */

SOPC_ReturnStatus SOPC_ClientHelperNew_DeleteSubscription(SOPC_ClientHelperNew_Subscription** subscription);

/**
 * \brief An optional monitored items manager which records the created monitored items in a subscription to:
 *        - Keep record of the client handle id, monitored item id and node id
 *        - Associate an client context which might be pointer to a monitored item
 *
 * \warning In order to be up to date for a subscription in which the manager is used,
 *          the following functions shall be called for each call to the services:
 *          - CreateMonitoredItems: ::SOPC_ClientHelperNew_MonitoredItemsManager_CreateHandlers
 *          - ModifyMonitoredItems: ::SOPC_ClientHelperNew_MonitoredItemsManager_UpdateHandlers
 *          - DeleteMonitoredItems: ::SOPC_ClientHelperNew_MonitoredItemsManager_DeleteHandlers
 *
 * \note Provides fresh client handle ids generator ?
 *       => already done elsewhere but not efficient if mass freed ids,
 *          we need a tested helper to manage it efficiently ...
 */
typedef struct SOPC_ClientHelperNew_MonitoredItemsManager SOPC_ClientHelperNew_MonitoredItemsManager;

/**
 * \brief Gets the monitored items manager associated to the given subscription.
 *
 * \warning Do not keep reference on returned manager after use since it will not be valid anymore after
 *          ::SOPC_ClientHelperNew_DeleteSubscription on the corresponding subscription
 *
 * \param subscription A subscription created by ::SOPC_ClientHelperNew_CreateSubscription and not deleted yet.
 *
 * \return The monitored items manager associated to the given subscription or NULL if the subscription is invalid.
 */
SOPC_ClientHelperNew_MonitoredItemsManager* SOPC_ClientHelperNew_Subscription_GetMonitoredItemsManager(
    SOPC_ClientHelperNew_Subscription* subscription);

/**
 * \brief Creates handlers for the successfully created monitored items.
 *        This function uses the CreateMonitoredItems request and response
 *        and an optional array of additional client context to record the monitored items created.
 *
 * \note This function ignores the monitored items creation which are not a success in the response \p resp.
 *
 * \warning Since the CreateMonitoredItems request memory is managed by function calling the service,
 *          a copy of the request shall be done prior to call to ::SOPC_ClientHelperNew_ServiceAsync or
 *          ::SOPC_ClientHelperNew_ServiceSync.
 *
 * \param manager         The monitored items manager of the concerned subscription
 * \param req             The create monitored items request that was used to call the service.
 *                        This parameter shall be a copy made prior to service call using
 *                        ::SOPC_EncodeableObject_Copy function.
 * \param nbClientCtx     The number of client context provided in the array.
 *                        It shall be 0 if no context is provided
 *                        or shall match the number of monitored items in create request.
 * \param clientCtxArray  Pointer to the array of client context which size is \p nbClientCtx
 *                        or NULL if \p nbClientCtx is 0 \param resp
 * \param resp            The create monitored items response that was returned by the service call.
 *                        It shall match the request \p req and as consequence have the same number of monitored items.
 *
 * \return SOPC_STATUS_OK in case of successfully created handlers,
 *         SOPC_STATUS_INVALID_PARAMETERS in case of invalid parameters.
 */
SOPC_ReturnStatus SOPC_ClientHelperNew_MonitoredItemsManager_CreateHandlers(
    SOPC_ClientHelperNew_MonitoredItemsManager* manager,
    const OpcUa_CreateMonitoredItemsRequest* req,
    size_t nbClientCtx,
    uintptr_t* clientCtxArray,
    const OpcUa_CreateMonitoredItemsResponse* resp);

/**
 * \brief Updates the handlers for the successfully modified monitored items.
 *        This function uses the ModifyMonitoredItems request and response
 *        and an optional array of additional client context to record the monitored items created.
 *        The client context associated to monitored items during the creation remains unchanged
 *        (see :: SOPC_ClientHelperNew_MonitoredItemsManager_CreateHandlers).
 *
 * \note This function ignores the monitored items modification which are not a success in the response \p resp.
 *
 * \warning Since the ModifyMonitoredItems request memory is managed by function calling the service,
 *          a copy of the request shall be done prior to call to ::SOPC_ClientHelperNew_ServiceAsync or
 *          ::SOPC_ClientHelperNew_ServiceSync.
 *
 * \param manager         The monitored items manager of the concerned subscription
 * \param req             The modify monitored items request that was used to call the service.
 *                        This parameter shall be a copy made prior to service call using
 *                        ::SOPC_EncodeableObject_Copy function.
 * \param resp            The modify monitored items response that was returned by the service call.
 *                        It shall match the request \p req and as consequence have the same number of monitored items.
 *
 * \return SOPC_STATUS_OK in case of successfully created handlers,
 *         SOPC_STATUS_INVALID_PARAMETERS in case of invalid parameters.
 */
SOPC_ReturnStatus SOPC_ClientHelperNew_MonitoredItemsManager_UpdateHandlers(
    SOPC_ClientHelperNew_MonitoredItemsManager* manager,
    const OpcUa_ModifyMonitoredItemsRequest* req,
    const OpcUa_ModifyMonitoredItemsResponse* resp);

/**
 * \brief Deletes the handlers for the deleted monitored items.
 *        The client context associated to the monitored items might be freed by caller after this call.
 *
 * \param manager         The monitored items manager of the concerned subscription
 * \param req             The delete monitored items request that will be used to call the service.
 *                        This parameter might be the actual request prior to the service call
 *                        or it shall be a copy made prior to service call using
 *                        ::SOPC_EncodeableObject_Copy function if service is already called.
 *
 * \return SOPC_STATUS_OK in case of successfully created handlers,
 *         SOPC_STATUS_INVALID_PARAMETERS in case of invalid parameters.
 */
SOPC_ReturnStatus SOPC_ClientHelperNew_MonitoredItemsManager_DeleteHandlers(
    SOPC_ClientHelperNew_MonitoredItemsManager* manager,
    const OpcUa_DeleteMonitoredItemsRequest* req);

/**
 * \brief Gets the client monitored item client handle id associated to the given NodeId (if unique).
 *
 * \warning The NodeId shall be unique in the created monitored items in order the function to return a result
 *
 * \param manager              The monitored items manager of the concerned subscription
 * \param nodeId               The NodeId of the monitored item, it shall be unique in order to obtain a result
 * \param[out] clientHandleId  The pointer to the client handle id associated to the monitored item uniquely identified
 *                             by the NodeId if the function returned value is true.
 *
 * \return true if the client handle id is returned in \p clientHandleId,
 *         false otherwise if not found or several associated to the NodeId.
 */
bool SOPC_ClientHelperNew_MonitoredItemsManager_GetClientHandleId(
    const SOPC_ClientHelperNew_MonitoredItemsManager* manager,
    const SOPC_NodeId* nodeId,
    uint32_t* clientHandleId);

/**
 * \brief Gets the monitored item id associated by the server to the client handle id
 *
 * \param manager               The monitored items manager of the concerned subscription
 * \param clientHandlerId       The client handler id of the monitored item
 * \param[out] monitoredItemId  The pointer to the monitored item id associated to the monitored item uniquely
 *                              identified by the NodeId if the function returned value is true.
 *
 * \return true if the monitored item id is returned in \p monitoredItemId,
 *         false otherwise if client hanldled id invalid.
 */
bool SOPC_ClientHelperNew_MonitoredItemsManager_GetMonitoredItemId(
    const SOPC_ClientHelperNew_MonitoredItemsManager* manager,
    uint32_t clientHandleId,
    uint32_t* monitoredItemId);

/**
 * \brief Gets the monitored item client context associated to the client handle id
 *
 * \param manager               The monitored items manager of the concerned subscription
 * \param clientHandlerId       The client handler id of the monitored item
 * \param[out] clientCtx        The pointer to the client context associated to the client handle id
 *                              if the function returned value is true.
 *
 * \return true if the client context is returned in \p clientCtx,
 *         false otherwise if client hanldled id invalid.
 */
bool SOPC_ClientHelperNew_MonitoredItemsManager_GetClientCtx(const SOPC_ClientHelperNew_MonitoredItemsManager* manager,
                                                             uint32_t clientHandleId,
                                                             uintptr_t* clientCtx);

/**
 * \brief Gets the monitored item identification associated to the client handle id
 *
 * \param manager            The monitored items manager of the concerned subscription
 * \param clientHandlerId    The client handler id of the monitored item
 * \param[out] nodeId
 * \param[out] attribute
 * \param[out] indexRange
 *
 * \return
 */
const SOPC_NodeId* SOPC_ClientHelperNew_MonitoredItemsManager_GetIdentification(
    const SOPC_ClientHelperNew_MonitoredItemsManager* manager,
    uint32_t clientHandleId,
    const SOPC_NodeId** nodeId,
    SOPC_AttributeId* attribute,
    const SOPC_String** indexRange);

#endif
