/*
 * wrapper_proxystub.c
 *
 *  Created on: Nov 17, 2016
 *      Author: vincent
 */

#include "wrapper_proxystub.h"

#include "sopc_stack_config.h"

void* OpcUa_ProxyStub_g_PlatformLayerCalltable = NULL;
void* OpcUa_ProxyStub_g_Configuration = NULL;

static SOPC_NamespaceTable* gNsTable = NULL;

SOPC_StatusCode OpcUa_ProxyStub_Initialize(void* pCalltable,
                                           void* pConfig)
{
    if(pCalltable != NULL){
        OpcUa_ProxyStub_g_PlatformLayerCalltable = pCalltable;
    }
    OpcUa_ProxyStub_g_Configuration = pConfig;
    return StackConfiguration_Initialize();
}

void OpcUa_ProxyStub_Clear(void)
{
    if(gNsTable != NULL){
        Namespace_Delete(gNsTable);
        gNsTable = NULL;
    }
    StackConfiguration_Clear();
}

SOPC_StatusCode OpcUa_ProxyStub_ReInitialize(void* pConfig){
    OpcUa_ProxyStub_Clear();
    return OpcUa_ProxyStub_Initialize(NULL, pConfig);
}

SOPC_StatusCode OpcUa_ProxyStub_AddTypes(SOPC_EncodeableType** types)
{
    uint32_t idx = 0;
    SOPC_EncodeableType* encType = types[idx];
    for(idx = 1; idx < UINT32_MAX && encType != NULL; idx++){
        encType = types[idx];
    }
    return StackConfiguration_AddTypes(types, idx);
}

SOPC_StatusCode OpcUa_ProxyStub_SetNamespaceUris(char** namespaceUris)
{
    if(gNsTable != NULL){
        return STATUS_NOK;
    }
    // count namespaces
    char* ns = NULL;
    uint32_t idx = 0;
    for(idx = 0; idx < UINT32_MAX && ns != NULL; idx++){
        ns = namespaceUris[idx];
    }
    gNsTable = Namespace_CreateTable(idx);
    for(idx = 0; idx < UINT32_MAX && ns != NULL; idx++){
        gNsTable->namespaceArray[idx].namespaceIndex = idx;
        gNsTable->namespaceArray[idx].namespaceName = namespaceUris[idx];
    }
    return StackConfiguration_SetNamespaceUris(gNsTable);
}
