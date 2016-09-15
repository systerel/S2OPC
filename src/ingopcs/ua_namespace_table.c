/*
 * ua_namspace_table.c
 *
 *  Created on: Sep 15, 2016
 *      Author: vincent
 */

#include <string.h>
#include <stdlib.h>
#include <ua_namespace_table.h>

const char* OPCUA_NAMESPACE_NAME = "http://opcfoundation.org/UA/";

const int32_t OPCUA_NAMESPACE_NAME_MAXLENGTH = INT32_MAX;

UA_NamespaceTable* Namespace_CreateTable(uint32_t length){
    UA_NamespaceTable* result = UA_NULL;
    if(length - 1 <= UINT16_MAX){
        result = malloc(sizeof(UA_NamespaceTable));
        if(result != UA_NULL){
            result->lastIdx = length - 1;
            result->namespaceArray = malloc(sizeof(UA_Namespace)*length);
            if(result->namespaceArray == UA_NULL){
                free(result);
                result = UA_NULL;
            }
        }
    }
    return result;
}

StatusCode Namespace_GetIndex(UA_NamespaceTable* namespaceTable,
                                 const char*        namespaceName,
                                 uint16_t*          index)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_Namespace namespaceEntry;
    if(namespaceTable != UA_NULL){
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

const char* Namespace_GetName(UA_NamespaceTable* namespaceTable,
                                 uint16_t index){
    UA_Namespace namespaceEntry;
    char* result = UA_NULL;
    if(namespaceTable != UA_NULL){
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

void Namespace_Delete(UA_NamespaceTable* namespaceTable)
{
    if(namespaceTable != UA_NULL){
        if(namespaceTable->namespaceArray != UA_NULL){
            free(namespaceTable->namespaceArray);
        }
        free(namespaceTable);
    }
}

