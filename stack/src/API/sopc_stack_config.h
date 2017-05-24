/**
 *  \file sopc_stack_config.h
 *
 *  \brief Stack initialization and configuration
 */
/*
 *  Copyright (C) 2016 Systerel and others.
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

#ifndef SOPC_STACK_CONFIG_H_
#define SOPC_STACK_CONFIG_H_

#include "sopc_builtintypes.h"
#include "sopc_namespace_table.h"

/**
 *  \brief  Clear the stack configuration (namespaces and encodeable types)
 *          and initialize the library and platform dependent code
 *
 *  \return STATUS_OK if initialization succeeded, STATUS_NOK otherwise
 */
SOPC_StatusCode StackConfiguration_Initialize();

/**
 *  \brief  Clear the stack configuration (namespaces and encodeable types).
 */
void StackConfiguration_Clear();

/**
 *  \brief Set the given namespace table to the configuration of the stack
 *
 *  \param nsTable  The new namespace table to be used by the stack
 *
 *  \return STATUS_OK if configuration succeeded, STATUS_NOK otherwise (NULL pointer or invalid state)
 */
SOPC_StatusCode StackConfiguration_SetNamespaceUris(SOPC_NamespaceTable* nsTable);

/**
 *  \brief Return the encodeable types table configuration used by the stack
 *
 *  \return Encodeable types table (terminated by NULL value)
 */
SOPC_EncodeableType** StackConfiguration_GetEncodeableTypes();


/**
 *  \brief Add the given encodeable types to the configuration of the stack
 *
 *  \param encTypesTable  The encodeable types to add to the encodeable types configuration of the stack (NULL terminated)
 *
 *  \return STATUS_OK if configuration succeeded, STATUS_NOK otherwise
 */
SOPC_StatusCode StackConfiguration_AddTypes(SOPC_EncodeableType** encTypesTable,
                                            uint32_t              nbTypes);

/**
 *  \brief Return the namespace table configuration used by the stack
 *
 *  \return Namespace table
 */
SOPC_NamespaceTable* StackConfiguration_GetNamespaces();


/**
 *  \brief  (Managed by SOPC_Channel on connection) Lock modifications of the stack configuration
 *          (namespaces and encodeable types)
 */
void StackConfiguration_Locked();

/**
 *  \brief  (Managed by SOPC_Channel on disconnection) Unlock modifications of the stack configuration
 *          (namespaces and encodeable types)
 */
void StackConfiguration_Unlocked();


#endif /* SOPC_STACK_CONFIG_H_ */
