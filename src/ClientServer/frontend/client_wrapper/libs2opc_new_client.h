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
 * \deprecated This file is here for retro-compatibility.
 *             It is deprecated since version 1.6.0 and will be removed in version 1.7.0.
 *             Please use libs2opc_client.h instead.
 */

#ifndef LIBS2OPC_NEW_CLIENT_H_
#define LIBS2OPC_NEW_CLIENT_H_

#include "libs2opc_client.h"

/**
 *  \deprecated This function has been renamed ::SOPC_ClientHelper_DiscoveryServiceAsync.
 *              It is deprecated since version 1.6.0 and will be removed in version 1.7.0.
 */
SOPC_ReturnStatus SOPC_ClientHelperNew_DiscoveryServiceAsync(SOPC_SecureConnection_Config* secConnConfig,
                                                             void* request,
                                                             uintptr_t userContext);

/**
 *  \deprecated This function has been renamed ::SOPC_ClientHelperNew_DiscoveryServiceSync.
 *              It is deprecated since version 1.6.0 and will be removed in version 1.7.0.
 */
SOPC_ReturnStatus SOPC_ClientHelperNew_DiscoveryServiceSync(SOPC_SecureConnection_Config* secConnConfig,
                                                            void* request,
                                                            void** response);
/**
 *  \deprecated This function has been renamed ::SOPC_ClientHelper_Connect.
 *              It is deprecated since version 1.6.0 and will be removed in version 1.7.0.
 */
SOPC_ReturnStatus SOPC_ClientHelperNew_Connect(SOPC_SecureConnection_Config* secConnConfig,
                                               SOPC_ClientConnectionEvent_Fct* connectEventCb,
                                               SOPC_ClientConnection** secureConnection);

/**
 *  \deprecated This function has been renamed ::SOPC_ClientHelper_Disconnect.
 *              It is deprecated since version 1.6.0 and will be removed in version 1.7.0.
 */
SOPC_ReturnStatus SOPC_ClientHelperNew_Disconnect(SOPC_ClientConnection** secureConnection);

/**
 *  \deprecated This function has been renamed ::SOPC_ClientHelper_Disconnect.
 *              It is deprecated since version 1.6.0 and will be removed in version 1.7.0.
 */
SOPC_ReturnStatus SOPC_ClientHelperNew_ServiceAsync(SOPC_ClientConnection* secureConnection,
                                                    void* request,
                                                    uintptr_t userContext);

/**
 *  \deprecated This function has been renamed ::SOPC_ClientHelper_ServiceSync.
 *              It is deprecated since version 1.6.0 and will be removed in version 1.7.0.
 */
SOPC_ReturnStatus SOPC_ClientHelperNew_ServiceSync(SOPC_ClientConnection* secureConnection,
                                                   void* request,
                                                   void** response);

/**
 *  \deprecated This function has been renamed ::SOPC_ClientHelper_CreateSubscription.
 *              It is deprecated since version 1.6.0 and will be removed in version 1.7.0.
 */
SOPC_ClientHelper_Subscription* SOPC_ClientHelperNew_CreateSubscription(
    SOPC_ClientConnection* secureConnection,
    OpcUa_CreateSubscriptionRequest* subParams,
    SOPC_ClientSubscriptionNotification_Fct* subNotifCb,
    uintptr_t userParam);

/**
 *  \deprecated This function has been renamed ::SOPC_ClientHelper_DeleteSubscription.
 *              It is deprecated since version 1.6.0 and will be removed in version 1.7.0.
 */
SOPC_ReturnStatus SOPC_ClientHelperNew_DeleteSubscription(SOPC_ClientHelper_Subscription** subscription);

/**
 *  \deprecated This function has been renamed ::SOPC_ClientHelper_Subscription_SetAvailableTokens.
 *              It is deprecated since version 1.6.0 and will be removed in version 1.7.0.
 */
SOPC_ReturnStatus SOPC_ClientHelperNew_Subscription_SetAvailableTokens(SOPC_ClientHelper_Subscription* subscription,
                                                                       uint32_t nbPublishTokens);

/**
 *  \deprecated This function has been renamed ::SOPC_ClientHelper_Subscription_GetRevisedParameters.
 *              It is deprecated since version 1.6.0 and will be removed in version 1.7.0.
 */
SOPC_ReturnStatus SOPC_ClientHelperNew_Subscription_GetRevisedParameters(SOPC_ClientHelper_Subscription* subscription,
                                                                         double* revisedPublishingInterval,
                                                                         uint32_t* revisedLifetimeCount,
                                                                         uint32_t* revisedMaxKeepAliveCount);

/**
 *  \deprecated This function has been renamed ::SOPC_ClientHelper_Subscription_GetUserParam.
 *              It is deprecated since version 1.6.0 and will be removed in version 1.7.0.
 */
uintptr_t SOPC_ClientHelperNew_Subscription_GetUserParam(const SOPC_ClientHelper_Subscription* subscription);

/**
 *  \deprecated This function has been renamed ::SOPC_ClientHelper_GetSecureConnection.
 *              It is deprecated since version 1.6.0 and will be removed in version 1.7.0.
 */
SOPC_ClientConnection* SOPC_ClientHelperNew_GetSecureConnection(const SOPC_ClientHelper_Subscription* subscription);

/**
 *  \deprecated This function has been renamed ::SOPC_ClientHelper_GetSubscriptionId.
 *              It is deprecated since version 1.6.0 and will be removed in version 1.7.0.
 */
SOPC_ReturnStatus SOPC_ClientHelperNew_GetSubscriptionId(const SOPC_ClientHelper_Subscription* subscription,
                                                         uint32_t* pSubscriptionId);

/**
 *  \deprecated This function has been renamed ::SOPC_ClientHelper_Subscription_CreateMonitoredItems.
 *              It is deprecated since version 1.6.0 and will be removed in version 1.7.0.
 */
SOPC_ReturnStatus SOPC_ClientHelperNew_Subscription_CreateMonitoredItems(
    const SOPC_ClientHelper_Subscription* subscription,
    OpcUa_CreateMonitoredItemsRequest* monitoredItemsReq,
    const uintptr_t* monitoredItemCtxArray,
    OpcUa_CreateMonitoredItemsResponse* monitoredItemsResp);

/**
 *  \deprecated This function has been renamed ::SOPC_ClientHelper_Subscription_DeleteMonitoredItems.
 *              It is deprecated since version 1.6.0 and will be removed in version 1.7.0.
 */
SOPC_ReturnStatus SOPC_ClientHelperNew_Subscription_DeleteMonitoredItems(
    const SOPC_ClientHelper_Subscription* subscription,
    OpcUa_DeleteMonitoredItemsRequest* delMonitoredItemsReq,
    OpcUa_DeleteMonitoredItemsResponse* delMonitoredItemsResp);

/**
 *  \deprecated This function has been renamed ::SOPC_ClientHelper_Subscription_SyncService.
 *              It is deprecated since version 1.6.0 and will be removed in version 1.7.0.
 */
SOPC_ReturnStatus SOPC_ClientHelperNew_Subscription_SyncService(const SOPC_ClientHelper_Subscription* subscription,
                                                                void* subOrMIrequest,
                                                                void** subOrMIresponse);

/**
 *  \deprecated This function has been renamed ::SOPC_ClientHelper_Subscription_AsyncService.
 *              It is deprecated since version 1.6.0 and will be removed in version 1.7.0.
 */
SOPC_ReturnStatus SOPC_ClientHelperNew_Subscription_AsyncService(const SOPC_ClientHelper_Subscription* subscription,
                                                                 void* subOrMIrequest,
                                                                 uintptr_t userContext);

#endif
