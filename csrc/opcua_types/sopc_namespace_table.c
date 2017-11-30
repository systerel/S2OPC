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

#include "sopc_namespace_table.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "sopc_builtintypes.h"
#include "sopc_helper_string.h"

const char* OPCUA_NAMESPACE_NAME = "http://opcfoundation.org/UA/";

const int32_t OPCUA_NAMESPACE_NAME_MAXLENGTH = INT32_MAX;

void SOPC_Namespace_Initialize(SOPC_NamespaceTable* nsTable)
{
    if (nsTable != NULL)
    {
        nsTable->lastIdx = 0;
        nsTable->namespaceArray = NULL;
        nsTable->clearTable = 1; // True
    }
}

SOPC_ReturnStatus SOPC_Namespace_AllocateTable(SOPC_NamespaceTable* nsTable, uint32_t length)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (nsTable != NULL)
    {
        status = SOPC_STATUS_OK;
        nsTable->clearTable = 1; // True
        nsTable->lastIdx = length - 1;
        nsTable->namespaceArray = malloc(sizeof(SOPC_Namespace) * (size_t) length);
        if (NULL == nsTable->namespaceArray)
        {
            status = SOPC_STATUS_NOK;
        }
    }
    return status;
}

SOPC_NamespaceTable* SOPC_Namespace_CreateTable(uint32_t length)
{
    SOPC_NamespaceTable* result = NULL;
    if (length - 1 <= UINT16_MAX)
    {
        result = malloc(sizeof(SOPC_NamespaceTable));
        if (SOPC_Namespace_AllocateTable(result, length) != SOPC_STATUS_OK && result != NULL)
        {
            free(result);
            result = NULL;
        }
    }
    return result;
}

SOPC_ReturnStatus SOPC_Namespace_AttachTable(SOPC_NamespaceTable* dst, SOPC_NamespaceTable* src)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (dst != NULL && NULL == dst->namespaceArray && src != NULL && src->namespaceArray != NULL)
    {
        status = SOPC_STATUS_OK;
        dst->clearTable = false;
        dst->lastIdx = src->lastIdx;
        dst->namespaceArray = src->namespaceArray;
    }
    return status;
}

SOPC_ReturnStatus SOPC_Namespace_GetIndex(SOPC_NamespaceTable* namespaceTable,
                                          const char* namespaceName,
                                          uint16_t* index)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    SOPC_Namespace namespaceEntry;
    if (namespaceTable != NULL)
    {
        if (NULL == namespaceName)
        {
            status = SOPC_STATUS_OK;
            index = OPCUA_NAMESPACE_INDEX;
        }
        else
        {
            status = SOPC_STATUS_NOK;
            uint32_t idx = 0;
            for (idx = 0; idx <= namespaceTable->lastIdx; idx++)
            {
                namespaceEntry = namespaceTable->namespaceArray[idx];
                if (SOPC_strncmp_ignore_case(namespaceEntry.namespaceName, namespaceName, strlen(namespaceName) + 1) ==
                    0)
                {
                    status = SOPC_STATUS_OK;
                    *index = namespaceEntry.namespaceIndex;
                }
            }
        }
    }
    return status;
}

const char* SOPC_Namespace_GetName(SOPC_NamespaceTable* namespaceTable, uint16_t index)
{
    SOPC_Namespace namespaceEntry;
    char* result = NULL;
    if (namespaceTable != NULL && namespaceTable->namespaceArray != NULL)
    {
        uint32_t idx = 0;
        for (idx = 0; idx <= namespaceTable->lastIdx; idx++)
        {
            namespaceEntry = namespaceTable->namespaceArray[idx];
            if (namespaceEntry.namespaceIndex == index)
            {
                result = namespaceEntry.namespaceName;
            }
        }
    }
    return result;
}

void SOPC_Namespace_Clear(SOPC_NamespaceTable* namespaceTable)
{
    if (namespaceTable != NULL)
    {
        if (namespaceTable->clearTable != false && namespaceTable->namespaceArray != NULL)
        {
            free(namespaceTable->namespaceArray);
        }
    }
}

void SOPC_Namespace_Delete(SOPC_NamespaceTable* namespaceTable)
{
    if (namespaceTable != NULL)
    {
        SOPC_Namespace_Clear(namespaceTable);
        free(namespaceTable);
    }
}
