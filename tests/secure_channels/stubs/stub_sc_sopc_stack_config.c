/*
 *  Copyright (C) 2017 Systerel and others.
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

#include "sopc_stack_config.h"

#include <stdlib.h>
#include <string.h>

#include "sopc_types.h"
#include "platform_deps.h"

static SOPC_NamespaceTable* nsTable = NULL;

SOPC_StatusCode SOPC_StackConfiguration_Initialize(){
    Namespace_Initialize(nsTable);
    InitPlatformDependencies();
    return STATUS_OK;
}

void SOPC_StackConfiguration_Clear(){
    Namespace_Clear(nsTable);
    nsTable = NULL;
}

SOPC_EncodeableType** SOPC_StackConfiguration_GetEncodeableTypes()
{
    // No additional types: return static known types
    return SOPC_KnownEncodeableTypes;
}

SOPC_NamespaceTable* SOPC_StackConfiguration_GetNamespaces()
{
    return nsTable;
}
