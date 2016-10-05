/*
 * ua_stack_config.c
 *
 *  Created on: Oct 4, 2016
 *      Author: vincent
 */

#include "ua_stack_config.h"

UA_StackConfiguration g_stackConfiguration;

static uint8_t initDone = UA_FALSE;

void StackConfiguration_Initialize(){
    if(initDone == UA_FALSE){
        StackConfiguration_Clear();
        initDone = 1;
    }
}

void StackConfiguration_Clear(){
    g_stackConfiguration.nsTable = NULL;
    g_stackConfiguration.traceLevel = 0;
    initDone = UA_FALSE;
}

StatusCode StackConfiguration_SetNamespaceUris(UA_NamespaceTable* nsTable){
    StatusCode status = STATUS_INVALID_STATE;
    if(initDone != UA_FALSE){
        if(nsTable == NULL){
            status = STATUS_INVALID_PARAMETERS;
        }else{
            g_stackConfiguration.nsTable = nsTable;
        }
    }
    return status;
}

StatusCode StackConfiguration_AddTypes(UA_EncodeableType** encTypesTable){
    // TODO: realloc table ?
    return STATUS_NOK;
}
