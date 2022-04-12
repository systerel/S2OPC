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
 * \brief High level interface to configure an OPC UA server
 *
 */

#ifndef LIBS2OPC_SERVER_CONFIG_H_
#define LIBS2OPC_SERVER_CONFIG_H_

#include <stdbool.h>

#include "sopc_common.h"
#include "sopc_encodeabletype.h"
#include "sopc_types.h"
#include "sopc_user_app_itf.h"

/**
 * \brief Initialize the S2OPC Server frontend configuration
 *        Call to ::SOPC_HelperConfigServer_Initialize is required before any other operation
 *        and shall be done after a call to ::SOPC_CommonHelper_Initialize
 *
 * \result SOPC_STATUS_OK in case of success, otherwise SOPC_STATUS_INVALID_STATE in case of double initialization.
 */
SOPC_ReturnStatus SOPC_HelperConfigServer_Initialize(void);

/**
 * \brief Clear the S2OPC Server frontend configuration
 *        It shall be done before a call to ::SOPC_CommonHelper_Clear
 */
void SOPC_HelperConfigServer_Clear(void);

/**
 * \brief
 *   Structure reserved for future use in order to custom the configuration through XML.
 *   e.g.: PKI provider alternative, etc.
 */
typedef struct SOPC_ConfigServerXML_Custom SOPC_ConfigServerXML_Custom;

/**
 * \brief Configure server from XML configuration files for: server endpoints, address space
 *        and users credential and rights.
 *
 * If not used or used partially, see libs2opc_config_custom.h to manually configure through API.
 *
 * \param serverConfigPath        path to server configuration XML file (s2opc_clientserver_config.xsd schema)
 *                                or NULL for manual configuration.
 * \param addressSpaceConfigPath  path to address space configuration XML file (UANodeSet.xsd schema)
 *                                or NULL for manual configuration.
 * \param userConfigPath          path to users credential and rights configuration XML file
 *                                (s2opc_clientserver_users_config.xsd schema)
 *                                or NULL for manual configuration.
 * \param customConfig            shall be NULL. Reserved for future customization of configuration from XML
 *                                (PKI provider, etc.).
 *
 * \return     SOPC_STATUS_OK in case of success,
 *             otherwise SOPC_STATUS_INVALID_PARAMETERS if a path is invalid or all paths are NULL or
 *             SOPC_STATUS_INVALID_STATE if the configuration is not possible
 *             (toolkit not initialized, server already started).
 */
SOPC_ReturnStatus SOPC_HelperConfigServer_ConfigureFromXML(const char* serverConfigPath,
                                                           const char* addressSpaceConfigPath,
                                                           const char* userConfigPath,
                                                           SOPC_ConfigServerXML_Custom* customConfig);

/**
 * \brief Method Call service configuration.
 *
 * It can be instantiated with ::SOPC_MethodCallManager_Create() or specific code by applicative code.
 *
 * \param mcm  A manager to implement method behavior for method nodes that can be used by CallMethod.
 *             It shall be compliant with struct ::SOPC_MethodCallManager of sopc_call_method_manager.h,
 *             ::SOPC_MethodCallManager_Create() provides a default implementation.
 *
 * \return SOPC_STATUS_OK in case of success, otherwise SOPC_STATUS_INVALID_PARAMETERS if \p mcm is invalid
 *             or SOPC_STATUS_INVALID_STATE if the configuration is not possible
 *             (toolkit not initialized, server already started).
 * */
SOPC_ReturnStatus SOPC_HelperConfigServer_SetMethodCallManager(SOPC_MethodCallManager* mcm);

/**
 * \brief Type of callback to provide to receive write notification on address space.
 *
 * \param callCtxPtr   Context provided by server, see getters available (::SOPC_CallContext_GetUser, etc.)
 * \param writeValue   The value resulting from the write operation in case of write success,
 *                     or the value requested to be written in case of failure
 *
 * \param writeStatus  The service resulting status code for the write value operation requested
 *
 *
 * \note Value resulting from write operation might be different from the write value requested
 *       since it is possible to request partial write in arrays and strings.
 *       Thus it indicates the complete final value here in case of success.
 *
 * \warning The callback function shall not do anything blocking or long treatment since it will block any other
 *          callback call (other instance of write notification, local service sync/async response, etc.).
 */
typedef void SOPC_WriteNotif_Fct(const SOPC_CallContext* callCtxPtr,
                                 OpcUa_WriteValue* writeValue,
                                 SOPC_StatusCode writeStatus);

/**
 * \brief Define the write notification callback to be used.
 *
 * This is optional but if used it shall be defined before starting server.
 *
 * \param writeNotifCb  The write notification callback to be used
 *
 * \return SOPC_STATUS_OK in case of success, otherwise SOPC_STATUS_INVALID_PARAMETERS if \p writeNotifCb is invalid
 *             or SOPC_STATUS_INVALID_STATE if the configuration is not possible
 *             (toolkit not initialized, server already started).
 *
 * \warning The callback function shall not do anything blocking or long treatment since it will block any other
 *          callback call (other instance of write notification, local service sync/async response, etc.).
 */
SOPC_ReturnStatus SOPC_HelperConfigServer_SetWriteNotifCallback(SOPC_WriteNotif_Fct* writeNotifCb);

/**
 * \brief Type of the callback called by CreateMonitoredItem service when a NodeId is not already part of server
 *        address space, the callback result indicates if it shall be considered known by server
 *        (and might exist later using AddNode service).
 *
 *        It returns true when CreateMonitoredItem for this \p nodeId shall succeed,
 *        in this case \p outNodeClass shall be set to the expected NodeClass
 *        and \p outBadStatus shall be set with an appropriate Bad StatusCode
 *        returned in the Publish response as first value notification.
 *        It returns false otherwise, in this case server will return Bad_NodeIdUnknown
 *        in the CreateMonitoredItem response.
 *
 * \warning This callback shall not block the thread that calls it, and shall return immediately.
 *
 *
 * \param      nodeId       NodeId that is not part of the server address space yet
 *                          and which is requested in a MonitoredItemCreateRequest.
 *                          It might be added by AddNode service later.
 * \param[out] outNodeClass The NodeClass of the known node when it will be available.
 *                          It shall always be the same for the same NodeId.
 * \param[out] outBadStatus The appropriate Bad StatusCode to return in the Publish response.
 *                          ::OpcUa_BadDataUnavailable or ::OpcUa_BadWouldBlock are recommended.
 *
 * \return                  true when CreateMonitoredItem for this \p nodeId shall succeed, false otherwise.
 */
typedef bool SOPC_CreateMI_NodeAvail_Fct(const SOPC_NodeId* nodeId,
                                         OpcUa_NodeClass* outNodeClass,
                                         SOPC_StatusCode* outUnavailabilityStatus);

/**
 * \brief Define the callback called by CreateMonitoredItem service when a NodeId is not already part of server.
 *        The callback result indicates if it shall be considered known by server (see ::SOPC_CreateMI_NodeAvail_Fct).
 *
 * This is optional but if used it shall be defined before starting server.
 *
 * \param nodeAvailCb  The MonitoredItem node availability callback to be used by server
 *
 * \return SOPC_STATUS_OK in case of success, otherwise SOPC_STATUS_INVALID_PARAMETERS if \p nodeAvailCb is invalid
 *             or SOPC_STATUS_INVALID_STATE if the configuration is not possible
 *             (toolkit not initialized, server already started).
 *
 * \warning This callback shall not block the thread that calls it, and shall return immediately.
 */
SOPC_ReturnStatus SOPC_HelperConfigServer_SetMonitItemNodeAvailCallback(SOPC_CreateMI_NodeAvail_Fct* nodeAvailCb);

/**
 * \brief Type of callback to provide to receive asynchronous local service response
 *
 * \param response     An asynchronous response to a local service request sent using
 *                     ::SOPC_ServerHelper_LocalServiceAsync (see authorized requests).
 *                     Response will be a pointer to one of the following types:
 *                     - ::OpcUa_ReadResponse
 *                     - ::OpcUa_WriteResponse
 *                     - ::OpcUa_BrowseResponse
 *                     - ::OpcUa_GetEndpointsResponse
 *                     - ::OpcUa_FindServersResponse
 *                     - ::OpcUa_FindServersOnNetworkResponse
 *                     - ::OpcUa_RegisterServer2Response
 *
 * \param userContext  The context that was provided with the corresponding request provided on
 *                     ::SOPC_ServerHelper_LocalServiceAsync call
 *
 * \warning The callback function shall not do anything blocking or long treatment since it will block any other
 *          callback call (other instance of response, write notification, etc.).
 */
typedef void SOPC_LocalServiceAsyncResp_Fct(SOPC_EncodeableType* type, void* response, uintptr_t userContext);

/**
 * \brief Define the local service response callback to be used.
 *
 * This is optional if not used or only synchronous version used.
 * This shall be defined before starting the server and using ::SOPC_ServerHelper_LocalServiceAsync.
 *
 * \param asyncRespCb  The local service asynchronous response callback to be used
 *
 * \return SOPC_STATUS_OK in case of success, otherwise SOPC_STATUS_INVALID_PARAMETERS if \p asyncRespCb is invalid
 *         or SOPC_STATUS_INVALID_STATE if the configuration is not possible
 *         (toolkit not initialized, server already started).
 *
 * \warning The callback function shall not do anything blocking or long treatment since it will block any other
 *          callback call (other instance of write notification, local service sync/async response, etc.).
 */
SOPC_ReturnStatus SOPC_HelperConfigServer_SetLocalServiceAsyncResponse(SOPC_LocalServiceAsyncResp_Fct* asyncRespCb);

/**
 * \brief Define duration of the shutdown phase when stopping the server.
 *
 * According to OPC UA standard the server shall indicate state change and
 * seconds remaining until shutdown during shutdown phase and before actually stopping.
 *
 * \param secondsTillShutdown  The number of seconds of the shutdown phase prior to actually stopping server
 *
 * \return SOPC_STATUS_OK in case of success, otherwise SOPC_STATUS_INVALID_STATE
 *         if the configuration is not possible (toolkit not initialized, server already started).
 *
 * \note Default value is DEFAULT_SHUTDOWN_PHASE_IN_SECONDS (5 seconds) if not set.
 *       Value 0 should not be used for OPC UA certification compliance.
 */
SOPC_ReturnStatus SOPC_HelperConfigServer_SetShutdownCountdown(uint16_t secondsTillShutdown);

#endif
