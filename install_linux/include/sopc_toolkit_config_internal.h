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
#include "sopc_user_app_itf.h"

/**
 *  \brief Return the endpoint configuration for the given index or null if not defined. (SOPC_Toolkit_Configured
 * required)
 *
 *  \param epConfigIdx  The endpoint configuration index requested
 *
 *  \return Endpoint configuration at given index or NULL if
 *  index invalid or toolkit is not configured yet
 */
SOPC_Endpoint_Config* SOPC_ToolkitServer_GetEndpointConfig(uint32_t epConfigIdx);

/**
 *  \brief (SERVER SIDE ONLY) Record the given secure channel configuration in returned index
 *  (SOPC_ToolkitClient_Initialize required, !SOPC_Toolkit_Configured required)
 * Note: the set of indexes of secure channel configuration for client and server are disjoint
 *
 *  \return secure channel configuration index if configuration succeeded,
 *  0 if toolkit is not initialized, already
 *  configured or otherwise
 */
uint32_t SOPC_ToolkitServer_AddSecureChannelConfig(SOPC_SecureChannel_Config* scConfig);

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
 * (SOPC_Toolkit_Configured required)
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
 * (SOPC_Toolkit_Configured required)
 * Note: the set of indexes of secure channel configuration for client and server are disjoint
 *
 *  \param scConfigIdx  The secure channel configuration index requested
 *
 *  \return Secure channel configuration at given index or NULL if
 *  index invalid or toolkit is not configured yet
 */
SOPC_SecureChannel_Config* SOPC_ToolkitClient_GetSecureChannelConfig(uint32_t scConfigIdx);

#endif /* SOPC_TOOLKIT_CONFIG_INTERNAL_H_ */
