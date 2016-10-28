/*
 * ua_encodeable.c
 *
 *  Created on: Sep 15, 2016
 *      Author: vincent
 */

#include <string.h>

#include "ua_encodeable.h"

#include <ua_builtintypes.h>

UA_EncodeableType* EncodeableType_GetEncodeableType(UA_EncodeableType** encTypesTable,
                                                    const char*         namespace,
                                                    uint32_t            typeId){
    UA_EncodeableType* current = NULL;
    UA_EncodeableType* result = NULL;
    uint32_t idx = 0;
    if(encTypesTable != NULL){
        current = encTypesTable[idx];
        while(current != NULL && result == NULL){
            if(typeId == result->TypeId || typeId == result->BinaryEncodingTypeId){
                // || typeId = result->xmlTypeId => should not be the case since we use UA binary !
                //TODO: max namespace or string size ?
                if(strncmp(namespace, current->NamespaceUri, INT32_MAX) == 0){ // Use maximum UA_String size representation size
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
