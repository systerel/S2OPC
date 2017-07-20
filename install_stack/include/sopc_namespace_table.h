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

#ifndef SOPC_NAMESPACE_TABLE_H_
#define SOPC_NAMESPACE_TABLE_H_

#include "sopc_builtintypes.h"

#define OPCUA_NAMESPACE_INDEX 0
extern const char* OPCUA_NAMESPACE_NAME;

extern const int32_t OPCUA_NAMESPACE_NAME_MAXLENGTH;

typedef struct {
    uint16_t namespaceIndex;
    char*    namespaceName;
} SOPC_Namespace;

typedef struct {
    uint16_t        lastIdx;
    SOPC_Namespace* namespaceArray;
    uint8_t         clearTable;
} SOPC_NamespaceTable;

void Namespace_Initialize(SOPC_NamespaceTable* nsTable);

SOPC_StatusCode Namespace_AllocateTable(SOPC_NamespaceTable* nsTable, uint32_t length);

SOPC_NamespaceTable* Namespace_CreateTable(uint32_t length); // length + 1 <= UINT16_MAX

SOPC_StatusCode Namespace_AttachTable(SOPC_NamespaceTable* dst, SOPC_NamespaceTable* src);

SOPC_StatusCode Namespace_GetIndex(SOPC_NamespaceTable* namespaceTable,
                                   const char*          namespaceName,
                                   uint16_t*            index);
const char* Namespace_GetName(SOPC_NamespaceTable* namespaceTable,
                              uint16_t             index);

void Namespace_Clear(SOPC_NamespaceTable* namespaceTable);

void Namespace_Delete(SOPC_NamespaceTable* namespaceTable);

#endif /* SOPC_NAMESPACE_TABLE_H_ */
