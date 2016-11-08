/*
 * ua_namspace_table.c
 *
 *  Created on: Sep 15, 2016
 *      Author: vincent
 */

#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "ua_namespace_table.h"

#include <ua_builtintypes.h>

const char* OPCUA_NAMESPACE_NAME = "http://opcfoundation.org/UA/";

const int32_t OPCUA_NAMESPACE_NAME_MAXLENGTH = INT32_MAX;

void Namespace_Initialize(SOPC_NamespaceTable* nsTable){
    if(nsTable != NULL){
        nsTable->lastIdx = 0;
        nsTable->namespaceArray = NULL;
        nsTable->clearTable = 1; // True
    }
}

SOPC_StatusCode Namespace_AllocateTable(SOPC_NamespaceTable* nsTable, uint32_t length){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(nsTable != NULL){
        status = STATUS_OK;
        nsTable->clearTable = 1; // True
        nsTable->lastIdx = length - 1;
        nsTable->namespaceArray = malloc(sizeof(SOPC_Namespace)*length);
        if(nsTable->namespaceArray == NULL){
            status = STATUS_NOK;
        }
    }
    return status;
}

SOPC_NamespaceTable* Namespace_CreateTable(uint32_t length){
    SOPC_NamespaceTable* result = NULL;
    if(length - 1 <= UINT16_MAX){
        result = malloc(sizeof(SOPC_NamespaceTable));
        if(Namespace_AllocateTable(result, length) != STATUS_OK && result != NULL){
            free(result);
            result = NULL;
        }
    }
    return result;
}

SOPC_StatusCode Namespace_AttachTable(SOPC_NamespaceTable* dst, SOPC_NamespaceTable* src){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(dst != NULL && dst->namespaceArray == NULL &&
       src != NULL && src->namespaceArray != NULL){
        status = STATUS_OK;
        dst->clearTable = FALSE;
        dst->lastIdx = src->lastIdx;
        dst->namespaceArray = src->namespaceArray;
    }
    return status;
}

SOPC_StatusCode Namespace_GetIndex(SOPC_NamespaceTable* namespaceTable,
                              const char*        namespaceName,
                              uint16_t*          index)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SOPC_Namespace namespaceEntry;
    if(namespaceTable != NULL){
        status = STATUS_NOK;
        uint32_t idx = 0;
        for (idx = 0; idx <= namespaceTable->lastIdx; idx++){
            namespaceEntry = namespaceTable->namespaceArray[idx];
            if(strncmp(namespaceEntry.namespaceName, namespaceName, OPCUA_NAMESPACE_NAME_MAXLENGTH) == 0){
                status = STATUS_OK;
                *index = namespaceEntry.namespaceIndex;
            }
        }
    }
    return status;
}

const char* Namespace_GetName(SOPC_NamespaceTable* namespaceTable,
                              uint16_t index){
    SOPC_Namespace namespaceEntry;
    char* result = NULL;
    if(namespaceTable != NULL){
        uint32_t idx = 0;
        for (idx = 0; idx <= namespaceTable->lastIdx; idx++){
            namespaceEntry = namespaceTable->namespaceArray[idx];
            if(namespaceEntry.namespaceIndex == index){
                result = namespaceEntry.namespaceName;
            }
        }
    }
    return result;
}

void Namespace_Clear(SOPC_NamespaceTable* namespaceTable)
{
    if(namespaceTable != NULL){
        if(namespaceTable->clearTable != FALSE &&
           namespaceTable->namespaceArray != NULL)
        {
            free(namespaceTable->namespaceArray);
        }
    }
}

void Namespace_Delete(SOPC_NamespaceTable* namespaceTable)
{
    if(namespaceTable != NULL){
        Namespace_Clear(namespaceTable);
        free(namespaceTable);
    }
}

