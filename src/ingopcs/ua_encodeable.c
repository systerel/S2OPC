/*
 * ua_encodeable.c
 *
 *  Created on: Sep 15, 2016
 *      Author: vincent
 */
#include <ua_builtintypes.h>
#include <string.h>

UA_EncodeableType* EncodeableType_GetEncodeableType(UA_EncodeableType** encTypesTable,
                                                    const char*         namespace,
                                                    uint32_t            typeId){
    UA_EncodeableType* current = UA_NULL;
    UA_EncodeableType* result = UA_NULL;
    uint32_t idx = 0;
    if(encTypesTable != UA_NULL){
        current = encTypesTable[idx];
        while(current != UA_NULL && result == UA_NULL){
            if(typeId == result->typeId || typeId == result->binaryTypeId){
                // || typeId = result->xmlTypeId => should not be the case since we use UA binary !
                //TODO: max namespace or string size ?
                if(strncmp(namespace, current->namespace, INT32_MAX) == 0){ // Use maximum UA_String size representation size
                    result = current;
                }
            }else if(idx < UINT32_MAX){
                idx++;
                current = encTypesTable[idx];
            }else{
                current = UA_NULL;
            }
        }
    }
    return result;
}
