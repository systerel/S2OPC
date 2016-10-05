/*
 * ua_stack_config.c
 *
 *  Created on: Oct 4, 2016
 *      Author: vincent
 */

#include "ua_stack_config.h"

#include <stdlib.h>

#include <ua_types.h>

UA_StackConfiguration g_stackConfiguration;
uint8_t g_lockedConfig = UA_FALSE;

static uint8_t initDone = UA_FALSE;

void StackConfiguration_Initialize(){
    if(initDone == UA_FALSE){
        StackConfiguration_Clear();
        initDone = 1;
    }
}

void StackConfiguration_Locked(){
    g_lockedConfig = 1;
}

void StackConfiguration_Unlocked(){
    g_lockedConfig = UA_FALSE;
}

void StackConfiguration_Clear(){
    if(g_stackConfiguration.encTypesTable != NULL){
        free(g_stackConfiguration.encTypesTable);
    }
    g_stackConfiguration.nsTable = NULL;
    g_stackConfiguration.encTypesTable = NULL;
    g_stackConfiguration.nbEncTypesTable = 0;
    g_stackConfiguration.traceLevel = 0;
    StackConfiguration_Unlocked();
    initDone = UA_FALSE;
}

StatusCode StackConfiguration_SetNamespaceUris(UA_NamespaceTable* nsTable){
    StatusCode status = STATUS_INVALID_STATE;
    if(initDone != UA_FALSE && g_lockedConfig == UA_FALSE){
        if(nsTable == NULL){
            status = STATUS_INVALID_PARAMETERS;
        }else{
            g_stackConfiguration.nsTable = nsTable;
        }
    }
    return status;
}

static uint32_t GetKnownEncodeableTypesLength(){
    uint32_t result = 0;
    for(result = 0; UA_KnownEncodeableTypes[result] != NULL; result++);
    return result + 1;
}

StatusCode StackConfiguration_AddTypes(UA_EncodeableType** encTypesTable,
                                       uint32_t            nbTypes){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    uint32_t idx = 0;
    uint32_t nbKnownTypes = 0;
    UA_EncodeableType** additionalTypes = NULL;

    if(initDone == UA_FALSE  || g_lockedConfig != UA_FALSE){
        return STATUS_INVALID_STATE;
    }

    if(encTypesTable != NULL && nbTypes > 0 ){
        status = STATUS_OK;
        if(g_stackConfiguration.encTypesTable == NULL){
            // known types to be added
            nbKnownTypes = GetKnownEncodeableTypesLength();
            // +1 for null value termination
            g_stackConfiguration.encTypesTable = malloc(sizeof(UA_EncodeableType*) * nbKnownTypes + nbTypes + 1);
            if(g_stackConfiguration.encTypesTable == NULL ||
               g_stackConfiguration.encTypesTable != memcpy(g_stackConfiguration.encTypesTable,
                                                            UA_KnownEncodeableTypes,
                                                            nbKnownTypes * sizeof(UA_EncodeableType*)))
            {
                g_stackConfiguration.encTypesTable = NULL;
            }else{
                additionalTypes = g_stackConfiguration.encTypesTable;
                g_stackConfiguration.nbEncTypesTable = nbKnownTypes;
            }
        }else{
            // +1 for null value termination
            additionalTypes = realloc(g_stackConfiguration.encTypesTable,
                                      sizeof(UA_EncodeableType*) * g_stackConfiguration.nbEncTypesTable + nbTypes + 1);
        }

        if(additionalTypes != NULL){
            g_stackConfiguration.encTypesTable = additionalTypes;

            for(idx = 0; idx < nbTypes; idx++){
                g_stackConfiguration.encTypesTable[g_stackConfiguration.nbEncTypesTable + idx] = encTypesTable[idx];
            }
            g_stackConfiguration.nbEncTypesTable += nbTypes;
            // NULL terminated table

        }else{
            status = STATUS_NOK;
        }
    }
    return status;
}

UA_EncodeableType** StackConfiguration_GetEncodeableTypes()
{
    if (g_stackConfiguration.encTypesTable != NULL && g_stackConfiguration.nbEncTypesTable > 0){
        // Additional types are present: contains known types + additional
        return g_stackConfiguration.encTypesTable;
    }else{
        // No additional types: return static known types
        return UA_KnownEncodeableTypes;
    }
}
