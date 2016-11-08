/*
 *sopc_encodeable.c
 *
 *  Created on: Sep 15, 2016
 *      Author: vincent
 */

#include <sopc_encodeable.h>

#include <string.h>

#include <sopc_builtintypes.h>

SOPC_EncodeableType* EncodeableType_GetEncodeableType(SOPC_EncodeableType** encTypesTable,
                                                    const char*         namespace,
                                                    uint32_t            typeId){
    SOPC_EncodeableType* current = NULL;
    SOPC_EncodeableType* result = NULL;
    uint32_t idx = 0;
    if(encTypesTable != NULL){
        current = encTypesTable[idx];
        while(current != NULL && result == NULL){
            if(typeId == result->TypeId || typeId == result->BinaryEncodingTypeId){
                // || typeId = result->xmlTypeId => should not be the case since we use UA binary !
                //TODO: max namespace or string size ?
                if(strncmp(namespace, current->NamespaceUri, INT32_MAX) == 0){ // Use maximum SOPC_String size representation size
                    result = current;
                }
            }else if(idx < UINT32_MAX){
                idx++;
                current = encTypesTable[idx];
            }else{
                current = NULL;
            }
        }
    }
    return result;
}
