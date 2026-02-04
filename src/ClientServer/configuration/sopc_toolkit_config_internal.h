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
 * \brief Toolkit internal use only: access to the shared configuration of the Toolkit and tools for interaction with
 * user application.
 *
 */

#ifndef SOPC_TOOLKIT_CONFIG_INTERNAL_H_
#define SOPC_TOOLKIT_CONFIG_INTERNAL_H_

#include "sopc_address_space.h"
#include "sopc_event_handler.h"
#include "sopc_toolkit_config.h"
#include "sopc_toolkit_config_constants.h"
#include "sopc_user_app_itf.h"

// Macro used to check EP configuration index validity depending on classic / reverse type
#define SOPC_IS_VALID_EP_CONFIGURATION(x) ((x) > 0 && (x) <= 2 * SOPC_MAX_ENDPOINT_DESCRIPTION_CONFIGURATIONS)
#define SOPC_IS_VALID_CLASSIC_EP_CONFIGURATION(x) ((x) > 0 && (x) <= SOPC_MAX_ENDPOINT_DESCRIPTION_CONFIGURATIONS)
#define SOPC_IS_VALID_REVERSE_EP_CONFIGURATION(x) \
    ((x) > SOPC_MAX_ENDPOINT_DESCRIPTION_CONFIGURATIONS && (x) <= 2 * SOPC_MAX_ENDPOINT_DESCRIPTION_CONFIGURATIONS)

/**
 *  \brief Return the endpoint configuration for the given index or null if not defined.
 * (::SOPC_ToolkitServer_Configured required)
 *
 *  \param epConfigIdx  The endpoint configuration index requested
 *
 *  \return Endpoint configuration at given index or NULL if
 *  index invalid or toolkit is not configured yet
 */
SOPC_Endpoint_Config* SOPC_ToolkitServer_GetEndpointConfig(SOPC_EndpointConfigIdx epConfigIdx);

/**
 *  \brief (SERVER SIDE ONLY) Record the given secure channel configuration in returned index
 *  (::SOPC_Toolkit_Initialize required and prior to ::SOPC_ToolkitServer_Configured call)
 * Note: the set of indexes of secure channel configuration for client and server are disjoint
 *
 *  \return secure channel configuration index if configuration succeeded,
 *  0 if toolkit is not initialized, already
 *  configured or otherwise
 */
SOPC_SecureChannelConfigIdx SOPC_ToolkitServer_AddSecureChannelConfig(SOPC_SecureChannel_Config* scConfig);

/**
 *  \brief (SERVER SIDE ONLY) Remove the secure channel configuration for the given server index
 *
 * Note: the set of indexes of secure channel configuration for client and server are disjoint
 *
 *  \param serverScConfigIdx  The secure channel configuration index requested
 *
 *  \return true if succeeded, false otherwise
 */
bool SOPC_ToolkitServer_RemoveSecureChannelConfig(uint32_t serverScConfigIdx);

/**
 *  \brief (SERVER SIDE ONLY) Return the secure channel configuration for the given index or null if not defined.
 * (::SOPC_ToolkitServer_Configured required)
 * Note: the set of indexes of secure channel configuration for client and server are disjoint
 *
 *  \param serverScConfigIdx  The secure channel configuration index requested
 *
 *  \return Secure channel configuration at given index or NULL if
 *  index invalid or toolkit is not configured yet
 */
SOPC_SecureChannel_Config* SOPC_ToolkitServer_GetSecureChannelConfig(uint32_t serverScConfigIdx);

/**
 *  \brief (CLIENT SIDE ONLY)  Return the secure channel configuration for the given index or null if not defined.
 * Note: the set of indexes of secure channel configuration for client and server are disjoint
 *
 *  \param scConfigIdx  The secure channel configuration index requested
 *
 *  \return Secure channel configuration at given index or NULL if
 *  index invalid or toolkit is not initialized
 */
SOPC_SecureChannel_Config* SOPC_ToolkitClient_GetSecureChannelConfig(uint32_t scConfigIdx);

/**
 *  \brief (CLIENT SIDE ONLY)  Return the client endpoint URL for the given reverse endpoint configuration index
 *  or null if not defined
 * Note: the set of indexes of (reverse) endpoint configuration indexes for client and server are disjoint
 *
 *  \param reverseEpCfgIdx  The secure channel configuration index requested
 *
 *  \return The client endpointURL of the reverse endpoint or NULL if index invalid or toolkit is not initialized
 */
const char* SOPC_ToolkitClient_GetReverseEndpointURL(SOPC_ReverseEndpointConfigIdx reverseEpCfgIdx);

/**
 *  \brief Sets the given Address Space for the current toolkit server
 *  (::SOPC_Toolkit_Initialize required and prior to ::SOPC_ToolkitServer_Configured call).
 *  Note: only one address space can be set, further call will be refused.
 *
 *  \param addressSpace  The address space definition
 *
 *  \return SOPC_STATUS_OK if configuration succeeded,
 *  SOPC_STATUS_INVALID_STATE if toolkit is not initialized, already
 *  configured or address space is already set, SOPC_STATUS_NOK otherwise
 */
SOPC_ReturnStatus SOPC_ToolkitServer_SetAddressSpaceConfig(SOPC_AddressSpace* addressSpace);

/**
 * \brief Defines whether the locked server configuration is currently used by services/secure channel layers.
 *        When the services/secure channel layer receives the associated event
 *        (APP_TO_SE_SERVER_CONFIGURED and derived one for SC),
 *        it calls this function to inform the configuration layer.
 *
 * \param activate  true to inform that locked configuration is used
 *                  (APP_TO_SE_SERVER_CONFIGURED received with flag set),
 *                  false to inform that locked configuration is not used anymore
 *                  (APP_TO_SE_SERVER_CONFIGURED received unset)
 *
 *  \return SOPC_STATUS_OK if operation succeeded,
 *          SOPC_STATUS_INVALID_STATE if config cannot be used/unused in current state.
 */
SOPC_ReturnStatus SOPC_ToolkitServer_UsingLockedConfig(bool activate);

/**
 *  \brief  Defines toolkit configuration as not configured anymore and unlock its state
 *          until SOPC_ToolkitServer_Configured is called again,
 *          any further call to server services will fail until a new configuration is done.
 *
 *  \warning Server SHALL be stopped prior to call this function
 *           (i.e. all endpoints closed and no client connection possible).
 *
 *  \return SOPC_STATUS_OK if initialization succeeded,
 *  SOPC_STATUS_INVALID_STATE if toolkit is not initialized or already
 *  configured,
 *  SOPC_STATUS_INVALID_PARAMETERS if server configuration is defined but no address space is set,
 *  any other error if it is not possible to synchronize the Services layer to unconfigure the server.
 */
SOPC_ReturnStatus SOPC_ToolkitServer_UnConfigure(void);

/**
 *  \brief Removes all the endpoint configurations previously defined on server side
 *  (::SOPC_Toolkit_Initialize required and prior to ::SOPC_ToolkitServer_Configured call
 *   or after ::SOPC_ToolkitServer_UnConfigure).
 *
 *  \return SOPC_STATUS_OK if endpoint configurations removal succeeded,
 *  SOPC_STATUS_INVALID_STATE if toolkit is not initialized or already configured.
 */
SOPC_ReturnStatus SOPC_ToolkitServer_RemoveAllEndpointsConfig(void);

/**
 * \brief Checks if the server security configuration is consistent (for tests only)
 *
 *  \return SOPC_STATUS_OK if security configuration is consistent.
 */
SOPC_ReturnStatus SOPC_ToolkitServer_SecurityCheck(void);

#endif /* SOPC_TOOLKIT_CONFIG_INTERNAL_H_ */
