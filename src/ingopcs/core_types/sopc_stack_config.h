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

typedef struct {
    uint8_t             traceLevel; // enum TBD
    // TBD: lengths configurable
    SOPC_NamespaceTable*  nsTable;
    SOPC_EncodeableType** encTypesTable;
    uint32_t            nbEncTypesTable;
} SOPC_StackConfiguration;

extern SOPC_StackConfiguration g_stackConfiguration;

SOPC_StatusCode StackConfiguration_Initialize(); // Init stack configuration and platform dependent code
void StackConfiguration_Locked();
void StackConfiguration_Unlocked();
void StackConfiguration_Clear();

SOPC_StatusCode StackConfiguration_SetNamespaceUris(SOPC_NamespaceTable* nsTable);
SOPC_StatusCode StackConfiguration_AddTypes(SOPC_EncodeableType** encTypesTable,
                                       uint32_t            nbTypes);
SOPC_EncodeableType** StackConfiguration_GetEncodeableTypes();
SOPC_NamespaceTable* StackConfiguration_GetNamespaces();

#endif /* INGOPCS_SOPC_STACK_CONFIG_H_ */
