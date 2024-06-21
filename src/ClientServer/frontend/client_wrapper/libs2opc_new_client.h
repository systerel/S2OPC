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
 * the client should establish connection using ::SOPC_ClientHelperNew_Connect or call a discovery service
 * using ::SOPC_ClientHelperNew_DiscoveryServiceAsync (or ::SOPC_ClientHelperNew_DiscoveryServiceSync).
 * Until connection is stopped by a call to ::SOPC_ClientHelperNew_Disconnect or due to an error
 * (socket closed, etc.), the client application might use the connections.
 * This is done using OPC UA services using ::SOPC_ClientHelperNew_ServiceAsync (or
 * ::SOPC_ClientHelperNew_ServiceSync).
 * The request messages can be built using the helper functions of libs2opc_request_builder.h
 * (e.g.: ::SOPC_ReadRequest_Create, ::SOPC_ReadRequest_SetReadValue, etc.).
 *
 * Dedicated functions are provided to handle subscriptions:
 * ::SOPC_ClientHelperNew_CreateSubscription (::SOPC_ClientHelperNew_DeleteSubscription) to create (delete) a unique
 * subscription instance for the connection.
 * ::SOPC_ClientHelperNew_Subscription_CreateMonitoredItems (::SOPC_ClientHelperNew_Subscription_DeleteMonitoredItems)
 * to create (delete) monitored items in a subscription instance.
 * ::SOPC_ClientHelperNew_Subscription_AsyncService and ::SOPC_ClientHelperNew_Subscription_SyncService
 * to call other subscription related services on the subscription instance.
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
    SOPC_ClientConnectionEvent_Disconnected, /**< Connection terminated unexpectedly, it will not be established again
                                                unless a new connection attempt is done. To do a new attempt the
                                                following functions shall be called:
                                                - ::SOPC_ClientHelperNew_Disconnect on current connection
                                                - ::SOPC_ClientHelperNew_Connect to create a new connection
                                              */
    SOPC_ClientConnectionEvent_Connected,    /**< (NOT IMPLEMENTED YET) Connection established (SC & session), only
                                                triggered when asynchronous SOPC_ClientHelperNew_StartConnection is used
                                                or in case of reconnection.
                                              */
    SOPC_ClientConnectionEvent_Reconnecting, /**< (NOT IMPLEMENTED YET) Connection temporarily interrupted,
                                                  attempt to re-establish connection on-going.
                                                  Do not use connection until Connected event received */
} SOPC_ClientConnectionEvent;

/**
 * \brief Type of callback called on client connection event
 *
 * \warning No blocking operation shall be done in callback
 *
 * \param config  Indicates the connection concerned by the event
 * \param event   The event that occurred on the connection
 * \param status  Indicates the return status of client connection
 */
typedef void SOPC_ClientConnectionEvent_Fct(SOPC_ClientConnection* config,
                                            SOPC_ClientConnectionEvent event,
                                            SOPC_StatusCode status);

/**
 * \brief Sends a discovery request without user session creation and activation and retrieve response asynchronously.
 *        Service response callback configured through ::SOPC_ClientConfigHelper_SetServiceAsyncResponse will be
 *        called on service response or in case of service request sending failure.
 *
 * \param secConnConfig  The secure connection configuration.
 * \param request   An instance of one of the following OPC UA request:
 *                  - ::OpcUa_FindServersRequest
 *                  - ::OpcUa_FindServersOnNetworkRequest
 *                  - ::OpcUa_GetEndpointsRequest
 *                  - ::OpcUa_RegisterServerRequest
 *                  - ::OpcUa_RegisterServer2Request
 *
 *                  The request messages can be built using the helper functions of libs2opc_request_builder.h
 *                  (e.g.: ::SOPC_GetEndpointsRequest_Create, ::SOPC_GetEndpointsRequest_SetPreferredLocales, etc.).
 *
 * \param userContext  User defined context that will be provided with the corresponding response in
 *                     ::SOPC_LocalServiceAsyncResp_Fct
 *
 * \return SOPC_STATUS_OK in case of success, SOPC_STATUS_INVALID_PARAMETERS in case of invalid parameters,
 *         otherwise SOPC_STATUS_INVALID_STATE if the client is not running.
 *
 * \warning If the server endpoint is not a discovery endpoint, or an activated session is expected,
 *          usual connection and generic services functions shall be used.
 *
 * \warning Caller of this API should wait at least ::SOPC_REQUEST_TIMEOUT_MS milliseconds after calling this function
 *          and prior to call ::SOPC_ClientConfigHelper_Clear.
 *          It is necessary to ensure asynchronous context is freed and no memory leak occurs.
 */
SOPC_ReturnStatus SOPC_ClientHelperNew_DiscoveryServiceAsync(SOPC_SecureConnection_Config* secConnConfig,
                                                             void* request,
                                                             uintptr_t userContext);

/**
 * \brief Sends a discovery request without user session creation and activation and retrieve response synchronously.
 *
 * \param secConnConfig  The secure connection configuration.
 * \param request   An instance of one of the following OPC UA request:
 *                  - ::OpcUa_FindServersRequest
 *                  - ::OpcUa_FindServersOnNetworkRequest
 *                  - ::OpcUa_GetEndpointsRequest
 *                  - ::OpcUa_RegisterServerRequest
 *                  - ::OpcUa_RegisterServer2Request
 *
 *                  The request messages can be built using the helper functions of libs2opc_request_builder.h
 *                  (e.g.: ::SOPC_GetEndpointsRequest_Create, ::SOPC_GetEndpointsRequest_SetPreferredLocales, etc.).
 *
 * \param[out] response  Pointer into which instance of response complying with the OPC UA request is provided:
 *                     \li ::OpcUa_FindServersResponse
 *                     \li ::OpcUa_FindServersOnNetworkResponse
 *                     \li ::OpcUa_GetEndpointsResponse
 *                     \li ::OpcUa_GetEndpointsResponse
 *                     \li ::OpcUa_RegisterServerResponse
 *                     \li ::OpcUa_RegisterServer2Response
 *
 *                     In case of service failure the response type is always ::OpcUa_ServiceFault,
 *                     in this case the \c response.encodeableType points to ::OpcUa_ServiceFault_EncodeableType
 *                     and ::SOPC_IsGoodStatus(\c response.ResponseHeader.ServiceResult) is \c false.
 *
 * \return SOPC_STATUS_OK in case of success, SOPC_STATUS_TIMEOUT in case of timeout to receive response,
 *         SOPC_STATUS_INVALID_PARAMETERS in case of invalid parameters,
 *         SOPC_STATUS_INVALID_STATE if the client is not running. And dedicated status if request sending failed.
 *
 * \note Request memory is managed by the client after a successful return or in case of timeout
 * \note Caller is responsible of output response memory after successful call
 *
 * \warning If the server endpoint is not a discovery endpoint, or an activated session is expected,
 *          usual connection and generic services functions shall be used.
 *
 * \warning Service synchronous call shall only be called from the application thread and shall not be called from
 * client callbacks used for notification, asynchronous response, client event, etc. (::SOPC_ServiceAsyncResp_Fct,
 * ::SOPC_ClientConnectionEvent_Fct, etc.). Otherwise this will lead to a deadlock situation.
 */
SOPC_ReturnStatus SOPC_ClientHelperNew_DiscoveryServiceSync(SOPC_SecureConnection_Config* secConnConfig,
                                                            void* request,
                                                            void** response);
/**
 * \brief Establishes the connection in a blocking way (synchronously).
 *
 * \param secConnConfig          The secure connection configuration to establish
 * \param connectEventCb         Callback called on connection event, it is only used in case of further
 *                               unexpected disconnection
 *
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
 * \warning This function shall not be called if pointed \p secureConnection is currently in use
 *          for any other operation and shall be prevented to be used for other operation.
 *          Internal references to \p secureConnection should have been cleared prior to this call.
 *
 * \param secureConnection  Pointer to the secure connection reference to stop, set to NULL during successful call.
 *
 * \return SOPC_STATUS_OK in case of success, SOPC_STATUS_INVALID_PARAMETERS in case of invalid parameters,
 *         otherwise SOPC_STATUS_INVALID_STATE if the connection is already disconnected
 */
SOPC_ReturnStatus SOPC_ClientHelperNew_Disconnect(SOPC_ClientConnection** secureConnection);

/**
 * \brief Executes an OPC UA service on server (read, write, browse, discovery service, etc.) asynchronously.
 *        Service response callback configured through ::SOPC_ClientConfigHelper_SetServiceAsyncResponse will be
 *        called on service response or in case of service request sending failure.
 *
 * \note ::SOPC_ClientHelperNew_Connect shall have been called and the connection shall be still active
 *
 * \param secureConnection The client connection instance to use to execute the service
 * \param request   An instance of one of the following OPC UA request:
 *                  - ::OpcUa_ReadRequest
 *                  - ::OpcUa_WriteRequest
 *                  - ::OpcUa_BrowseRequest
 *                  - ::OpcUa_TranslateBrowsePathsToNodeIdsRequest
 *                  - ::OpcUa_GetEndpointsRequest
 *                  - ::OpcUa_FindServersRequest
 *                  - ::OpcUa_FindServersOnNetworkRequest
 *                  - ::OpcUa_RegisterServer2Request
 *                  - etc, ...
 *
 *                  The request messages can be built using the helper functions of libs2opc_request_builder.h
 *                  (e.g.: ::SOPC_ReadRequest_Create, ::SOPC_ReadRequest_SetReadValue, etc.).
 *
 * \param userContext  User defined context that will be provided with the corresponding response in
 *                     ::SOPC_LocalServiceAsyncResp_Fct
 *
 * \return SOPC_STATUS_OK in case of success, SOPC_STATUS_INVALID_PARAMETERS in case of invalid parameters,
 *         otherwise SOPC_STATUS_INVALID_STATE if the client is not running.
 *
 * \note Request memory is managed by the client after a successful return
 *
 * \warning Caller of this API should wait at least ::SOPC_REQUEST_TIMEOUT_MS milliseconds after calling this function
 *          and prior to call ::SOPC_ClientConfigHelper_Clear.
 *          It is necessary to ensure asynchronous context is freed and no memory leak occurs.
 */
SOPC_ReturnStatus SOPC_ClientHelperNew_ServiceAsync(SOPC_ClientConnection* secureConnection,
                                                    void* request,
                                                    uintptr_t userContext);

/**
 * \brief Executes an OPC UA service on server (read, write, browse, discovery service, etc.) synchronously.
 *
 * \note ::SOPC_ClientHelperNew_Connect shall have been called
 *       and the connection shall be still active
 *
 * \param secureConnection The client connection instance to use to execute the service
 * \param request   An instance of OPC UA request:
 *                  - ::OpcUa_ReadRequest
 *                  - ::OpcUa_WriteRequest
 *                  - ::OpcUa_BrowseRequest
 *                  - ::OpcUa_TranslateBrowsePathsToNodeIdsRequest
 *                  - ::OpcUa_GetEndpointsRequest
 *                  - ::OpcUa_FindServersRequest
 *                  - ::OpcUa_FindServersOnNetworkRequest
 *                  - ::OpcUa_RegisterServer2Request
 *                  - etc, ...
 *
 *                  The request messages can be built using the helper functions of libs2opc_request_builder.h
 *                  (e.g.: ::SOPC_ReadRequest_Create, ::SOPC_ReadRequest_SetReadValue, etc.).
 *
 *                  Note: it shall be allocated on heap since it will be freed by S2OPC library during treatment.
 *
 * \param[out] response  Pointer into which instance of response complying with the OPC UA request is provided:
 *                     \li ::OpcUa_ReadResponse
 *                     \li ::OpcUa_WriteResponse
 *                     \li ::OpcUa_BrowseResponse
 *                     \li ::OpcUa_TranslateBrowsePathsToNodeIdsResponse
 *                     \li ::OpcUa_GetEndpointsResponse
 *                     \li ::OpcUa_FindServersResponse
 *                     \li ::OpcUa_FindServersOnNetworkResponse
 *                     \li ::OpcUa_RegisterServer2Response
 *                     \li etc, ...
 *
 *                     In case of service failure the response type is always ::OpcUa_ServiceFault,
 *                     in this case the \c response.encodeableType points to ::OpcUa_ServiceFault_EncodeableType
 *                     and ::SOPC_IsGoodStatus(\c response.ResponseHeader.ServiceResult) is \c false.
 *
 * \return SOPC_STATUS_OK in case of success,  SOPC_STATUS_TIMEOUT in case of timeout to receive response,
 *         SOPC_STATUS_INVALID_PARAMETERS in case of invalid parameters,
 *         SOPC_STATUS_INVALID_STATE if the client is not running. And dedicated status if request sending failed.
 *
 * \note Request memory is managed by the client after a successful return or in case of timeout.
 * \note Caller is responsible of output response memory after successful call. E.g. use ::SOPC_Encodeable_Delete.
 *
 * \warning service synchronous call shall only be called from the application thread and shall not be called from
 * client callbacks used for asynchronous response, connections event, etc. (::SOPC_ServiceAsyncResp_Fct,
 * ::SOPC_ClientConnectionEvent_Fct, etc.). Otherwise this will lead to a deadlock situation.
 */
SOPC_ReturnStatus SOPC_ClientHelperNew_ServiceSync(SOPC_ClientConnection* secureConnection,
                                                   void* request,
                                                   void** response);

typedef struct SOPC_ClientHelper_Subscription SOPC_ClientHelper_Subscription;

/**
 * \brief Type of callback called on Subscription Notification
 *
 * \warning No blocking operation shall be done in callback
 *          (e.g. call to ::SOPC_ClientHelperNew_Subscription_CreateMonitoredItems and
 *                ::SOPC_ClientHelperNew_Subscription_SyncService are forbidden)
 *
 * \param subscription          Indicates the subscription concerned by the notification
 * \param status                OPC UA status code for the subscription,
 *                              \p notification is only valid when ::SOPC_IsGoodStatus
 * \param notificationType      Type of notification received (::OpcUa_DataChangeNotification_EncodeableType or
 *                              ::OpcUa_EventNotificationList_EncodeableType) or NULL (if \p status is not good)
 * \param nbNotifElts           Number of elements in \p notification received and in \p monitoredItemCtxArray
 * \param notification          Notification of the type indicated by \p notificationType,
 *                              either pointer to a ::OpcUa_DataChangeNotification or
 *                              ::OpcUa_EventNotificationList. Or NULL if \p status is not good.
 *                              Content is freed after callback return, thus any content to record shall be copied.
 * \param monitoredItemCtxArray Array of context for monitored items for which notification were received in
 *                              \p notification.
 *                              Notification element and monitored item context have the same index in the array.
 *
 */
typedef void SOPC_ClientSubscriptionNotification_Fct(const SOPC_ClientHelper_Subscription* subscription,
                                                     SOPC_StatusCode status,
                                                     SOPC_EncodeableType* notificationType,
                                                     uint32_t nbNotifElts,
                                                     const void* notification,
                                                     uintptr_t* monitoredItemCtxArray);

/**
 * \brief Creates a subscription on the server
 *
 * \warning The current implementation is limited to 1 subscription per connection
 *
 * \param secureConnection The client connection instance to use to execute the service
 * \param subParams        The subscription creation request containing subscription parameters
 *                         (created using ::SOPC_CreateSubscriptionRequest_CreateDefault or
 *                          :: SOPC_CreateSubscriptionRequest_Create)
 * \param subNotifCb       The callback to be called on subscription notification
 * \param userParam        The user parameter associated to the subscription that can be accessed using
 *                         ::SOPC_ClientHelperNew_Subscription_GetUserParam
 *
 * \return The subscription instance or NULL in case of error (invalid parameters, subscription already created, etc.)
 */
SOPC_ClientHelper_Subscription* SOPC_ClientHelperNew_CreateSubscription(
    SOPC_ClientConnection* secureConnection,
    OpcUa_CreateSubscriptionRequest* subParams,
    SOPC_ClientSubscriptionNotification_Fct* subNotifCb,
    uintptr_t userParam);

/**
 * \brief Deletes a subscription on the server
 *
 * \param subscription     Pointer to the subscription pointer returned by ::SOPC_ClientHelperNew_CreateSubscription
 *                         and to be deleted.
 *
 * \return SOPC_STATUS_OK in case of success, SOPC_STATUS_INVALID_PARAMETERS in case of invalid parameters,
 *         SOPC_STATUS_INVALID_STATE if the client or subscription is not running.
 */
SOPC_ReturnStatus SOPC_ClientHelperNew_DeleteSubscription(SOPC_ClientHelper_Subscription** subscription);

/**
 * \brief  Sets the number of publish tokens to be used for the subscription.
 *         This number shall be greater than 0 and indicates the number of publish request
 *         sent to the server that might be used to send back notifications.
 *
 * \note By default 3 publish tokens are configured.
 * \note A new publish request token is sent back to the server each time a publish response is received
 *       (i.e. token consumed)
 *
 * \param subscription     The subscription instance
 * \param nbPublishTokens  The number of publish tokens to be used by the client
 *
 * \return SOPC_STATUS_OK in case of success, SOPC_STATUS_INVALID_PARAMETERS in case of invalid parameters,
 *         SOPC_STATUS_INVALID_STATE if the client or subscription is not running.
 */
SOPC_ReturnStatus SOPC_ClientHelperNew_Subscription_SetAvailableTokens(SOPC_ClientHelper_Subscription* subscription,
                                                                       uint32_t nbPublishTokens);

/**
 * \brief Gets the created subscription parameters values revised by the server.
 *
 * \param subscription                    The subscription instance
 * \param[out] revisedPublishingInterval  Pointer for the revised publishing interval output value (optional)
 * \param[out] revisedLifetimeCount       Pointer for the revised lifetime count output value (optional)
 * \param[out] revisedMaxKeepAliveCount   Pointer for the revised max keep alive count output value (optional)
 *
 * \return SOPC_STATUS_OK in case of success, SOPC_STATUS_INVALID_PARAMETERS in case of invalid subscription pointer,
 *         SOPC_STATUS_INVALID_STATE if the client or connection is not running.
 */
SOPC_ReturnStatus SOPC_ClientHelperNew_Subscription_GetRevisedParameters(SOPC_ClientHelper_Subscription* subscription,
                                                                         double* revisedPublishingInterval,
                                                                         uint32_t* revisedLifetimeCount,
                                                                         uint32_t* revisedMaxKeepAliveCount);

/**
 * \brief Returns the user parameter defined in ::SOPC_ClientHelperNew_CreateSubscription
 *
 * \param subscription  The subscription instance
 *
 * \return              User defined parameter provided in ::SOPC_ClientHelperNew_CreateSubscription
 *                      for creation of \p subscription
 */
uintptr_t SOPC_ClientHelperNew_Subscription_GetUserParam(const SOPC_ClientHelper_Subscription* subscription);

/**
 * \brief Gets the secure connection on which the subscription rely on
 *
 * \param subscription  The subscription instance
 *
 * \return              The secure connection instance
 */
SOPC_ClientConnection* SOPC_ClientHelperNew_GetSecureConnection(const SOPC_ClientHelper_Subscription* subscription);

/**
 * \brief Gets the subscription Id associated to the subscription.
 *        It should only be used in the context of server method calls (ConditionRefresh, etc.)
 *        and is not referenced by the rest of this client API.
 *
 * \param subscription          The subscription instance
 * \param[out] pSubscriptionId  The subscription Id associated to the subscription by server
 *
 * \return SOPC_STATUS_OK in case of success, SOPC_STATUS_INVALID_PARAMETERS in case of invalid subscription pointer,
 *         SOPC_STATUS_INVALID_STATE if the client or connection is not running.
 */
SOPC_ReturnStatus SOPC_ClientHelperNew_GetSubscriptionId(const SOPC_ClientHelper_Subscription* subscription,
                                                         uint32_t* pSubscriptionId);

/**
 * \brief Creates new monitored items on the given subscription.
 *        A context array might be provided and context will be provided
 *        on notification for corresponding monitored item in notification callback.
 *        A pointer to empty message response might be provided to retrieve
 *        the creation result and monitored item id affected by the server
 *        (needed to modify or delete monitored items prior to subscription deletion)
 *
 * \param subscription          The subscription instance on which monitored items shall be created
 * \param monitoredItemsReq     The create monitored items requests to use for creation parameters.
 *                              SubscriptionId and ClientHandle parameters are ignored and set automatically.
 *                              Simplified way to create it is to use ::SOPC_CreateMonitoredItemsRequest_CreateDefault
 *                              or ::SOPC_CreateMonitoredItemsRequest_CreateDefaultFromStrings.
 * \param monitoredItemCtxArray (optional) The array of context for monitored items to be created
 *                              (might be freed by caller after call only content is recorded in subscription)
 * \param[out] monitoredItemsResp    (optional) Pointer to the empty response that will be filled
 *                                   with the response received from the server and containing the status result
 *                                   and server monitored items ids.
 */
SOPC_ReturnStatus SOPC_ClientHelperNew_Subscription_CreateMonitoredItems(
    const SOPC_ClientHelper_Subscription* subscription,
    OpcUa_CreateMonitoredItemsRequest* monitoredItemsReq,
    const uintptr_t* monitoredItemCtxArray,
    OpcUa_CreateMonitoredItemsResponse* monitoredItemsResp);

/**
 * \brief Deletes monitored items on the given subscription using the server monitored item ids.
 *        A pointer to empty message response might be provided to retrieve
 *        the deletion result
 *
 * \param subscription             The subscription instance on which monitored items shall be created
 * \param delMonitoredItemsReq     The delete monitored items requests to use for creation parameters.
 *                                 SubscriptionId parameter is ignored and set automatically.
 * \param[out] delMonitoredItemsResp  (optional) Pointer to the empty response that will be filled
 *                                    with the response received from the server and containing the status result.
 */
SOPC_ReturnStatus SOPC_ClientHelperNew_Subscription_DeleteMonitoredItems(
    const SOPC_ClientHelper_Subscription* subscription,
    OpcUa_DeleteMonitoredItemsRequest* delMonitoredItemsReq,
    OpcUa_DeleteMonitoredItemsResponse* delMonitoredItemsResp);

/**
 * \brief Executes an OPC UA service on server related to the given subscription synchronously.
 *
 * \note The subscription identifier part in the request provided is automatically set by this function.
 * \note The requests to create / delete subscription or monitored items are not supported since dedicated
 *       API function shall be used.
 * \note The transfer subscription service is not supported by client.
 *
 * \param subscription The subscription instance for which the service shall be executed
 * \param subOrMIrequest   An instance of one of the following OPC UA request:
 *                         - ::OpcUa_ModifySubscriptionRequest
 *                         - ::OpcUa_SetPublishingModeRequest (expecting 1 subscription id to fill in allocated array)
 *                         - ::OpcUa_ModifyMonitoredItemsRequest
 *                         - ::OpcUa_SetMonitoringModeRequest
 *                         - ::OpcUa_SetTriggeringRequest
 *
 * \param[out] subOrMIresponse  Pointer into which instance of response complying with the OPC UA request is provided:
 *                     \li ::OpcUa_ModifySubscriptionResponse
 *                     \li ::OpcUa_SetPublishingModeResponse
 *                     \li ::OpcUa_ModifyMonitoredItemsResponse
 *                     \li ::OpcUa_SetMonitoringModeResponse
 *                     \li ::OpcUa_SetTriggeringResponse

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
 * \warning service synchronous call shall only be called from the application thread and shall not be called from
 * client callbacks used for asynchronous response, connections event, etc. (::SOPC_ServiceAsyncResp_Fct,
 * ::SOPC_ClientConnectionEvent_Fct, etc.). Otherwise this will lead to a deadlock situation.
 */
SOPC_ReturnStatus SOPC_ClientHelperNew_Subscription_SyncService(const SOPC_ClientHelper_Subscription* subscription,
                                                                void* subOrMIrequest,
                                                                void** subOrMIresponse);

/**
 * \brief Executes an OPC UA service on server related to the given subscription asynchronously.
 *        Service response callback configured through ::SOPC_ClientConfigHelper_SetServiceAsyncResponse will be
 *        called on service response or in case of service request sending failure.
 *
 *
 * \note The subscription identifier part in the request provided is automatically set by this function.
 * \note The requests to create / delete subscription or monitored items are not supported since dedicated
 *       API function shall be used.
 * \note The transfer subscription service is not supported by client.
 *
 * \param subscription The subscription instance for which the service shall be executed
 * \param subOrMIrequest   An instance of one of the following OPC UA request:
 *                         - ::OpcUa_ModifySubscriptionRequest
 *                         - ::OpcUa_SetPublishingModeRequest (expecting 1 subscription id to fill in allocated array)
 *                         - ::OpcUa_ModifyMonitoredItemsRequest
 *                         - ::OpcUa_SetMonitoringModeRequest
 *                         - ::OpcUa_SetTriggeringRequest
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
 *          and prior to call ::SOPC_ClientConfigHelper_Clear.
 *          It is necessary to ensure asynchronous context is freed and no memory leak occurs.
 */
SOPC_ReturnStatus SOPC_ClientHelperNew_Subscription_AsyncService(const SOPC_ClientHelper_Subscription* subscription,
                                                                 void* subOrMIrequest,
                                                                 uintptr_t userContext);

#endif
