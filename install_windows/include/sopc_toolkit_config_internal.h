/*
 *  Copyright (C) 2018 Systerel and others.
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

/** \file
 *
 * \brief Toolkit internal use only: access to the shared configuration of the Toolkit and tools for interaction with
 * user application.
 *
 */

#ifndef SOPC_TOOLKIT_CONFIG_INTERNAL_H_
#define SOPC_TOOLKIT_CONFIG_INTERNAL_H_

#include "sopc_user_app_itf.h"

#include "sopc_namespace_table.h"

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

/**
 *  \brief Return the encodeable types table configuration used by the stack (SOPC_Toolkit_Configured required)
 *
 *  \return Encodeable types table (terminated by NULL value) or NULL if
 *  toolkit is not configured yet
 */
SOPC_EncodeableType** SOPC_ToolkitConfig_GetEncodeableTypes(void);

/**
 *  \brief Return the namespace table configuration  (SOPC_Toolkit_Configured required)
 *
 *  \return Namespace table or NULL if toolkit is not configured yet
 */
SOPC_NamespaceTable* SOPC_ToolkitConfig_GetNamespaces(void);

void SOPC_Internal_ToolkitServer_SetAddressSpaceConfig(SOPC_AddressSpace* addressSpace);

typedef enum SOPC_App_EventType { SOPC_APP_COM_EVENT = 0x0, SOPC_APP_ADDRESS_SPACE_NOTIF = 0x01 } SOPC_App_EventType;

void SOPC_Internal_ApplicationEventDispatcher(int32_t eventAndType, uint32_t id, void* params, uintptr_t auxParam);

int32_t SOPC_AppEvent_ComEvent_Create(SOPC_App_Com_Event event);
int32_t SOPC_AppEvent_AddSpaceEvent_Create(SOPC_App_AddSpace_Event event);

SOPC_App_EventType SOPC_AppEvent_AppEventType_Get(int32_t iEvent);
SOPC_App_Com_Event SOPC_AppEvent_ComEvent_Get(int32_t iEvent);
SOPC_App_AddSpace_Event SOPC_AppEvent_AddSpaceEvent_Get(int32_t iEvent);

#endif /* SOPC_TOOLKIT_CONFIG_INTERNAL_H_ */
