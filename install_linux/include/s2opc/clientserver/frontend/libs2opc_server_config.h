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
 * \brief Initialize the S2OPC library (start threads, initialize configuration, etc.) and define a custom log
 * configuration.
 *
 * Call to ::SOPC_Helper_Initialize is required before any other operation.
 *
 * \param optLogConfig the custom log configuration or NULL to keep default configuration
 *
 * \result SOPC_STATUS_OK in case of success, otherwise SOPC_STATUS_INVALID_STATE in case of double initialization.
 */
SOPC_ReturnStatus SOPC_Helper_Initialize(SOPC_Log_Configuration* optLogConfig);

/**
 * \brief Clear the S2OPC library (stop threads, clear configuration, etc.)
 */
void SOPC_Helper_Clear(void);

/**
 * \brief Retrieve the toolkit build info (version, date, etc.).
 * Shortcut to ::SOPC_ToolkitConfig_GetBuildInfo.
 *
 * \return Toolkit build information
 *
 */
SOPC_Toolkit_Build_Info SOPC_Helper_GetBuildInfo(void);

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
 *        If not used or used partially, see libs2opc_config_custom.h to manually configure through API.
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
 *        It can be instantiated with ::SOPC_MethodCallManager_Create() or specific code by applicative code.
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
typedef void SOPC_WriteNotif_Fct(OpcUa_WriteValue* writeValue, SOPC_StatusCode writeStatus);

/**
 * \brief Define the write notification callback to be used.
 *
 *        This is optional but if used it shall be defined before starting server.
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
 * \brief Type of callback to provide to receive asynchronous local service response
 *
 * \param response     An asynchronous response to a local service request sent using
 *                     ::SOPC_ServerHelper_LocalServiceAsync. It shall be a pointer to one of the following types:
 *                     - OpcUa_ReadRequest
 *                     - OpcUa_WriteRequest
 *                     - OpcUa_BrowseRequest
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
 *        This is optional if not used or only synchronous version used.
 *        This shall be defined before starting the server and using ::SOPC_ServerHelper_LocalServiceAsync.
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
 *        According to OPC UA standard the server shall indicate state change and
 *        seconds remaining until shutdown during shutdown phase and before actually stopping.
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

/**
 * \brief Define a function to be called on client side communication events.
 *
 *        It allows to manage a low-level client in addition to the frontend server use.
 *        It is only possible to have a low-level client with a server,
 *        as the client wrapper and server wrappers cannot be used at same time in this current version.
 *
 * \param clientComEvtCb  The function callback to re-route all client related events
 *
 * \return SOPC_STATUS_OK in case of success, otherwise SOPC_STATUS_INVALID_STATE
 *         if the configuration is not possible (toolkit not initialized, server already started).
 *
 * \warning low-level client should only define its secure channel configuration
 *          between call to ::SOPC_Helper_Initialize and call to start the server.
 * \warning The callback function shall not do anything blocking or long treatment since it will block any other
 *          callback call (other instance of write notification, local service sync/async response, etc.).
 */
SOPC_ReturnStatus SOPC_HelperConfigClient_SetRawClientComEvent(SOPC_ComEvent_Fct* clientComEvtCb);

#endif
