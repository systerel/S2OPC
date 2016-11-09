/* Copyright (c) 1996-2016, OPC Foundation. All rights reserved.

   The source code in this file is covered under a dual-license scenario:
     - RCL: for OPC Foundation members in good-standing
     - GPL V2: everybody else

   RCL license terms accompanied with this source code. See http://opcfoundation.org/License/RCL/1.00/

   GNU General Public License as published by the Free Software Foundation;
   version 2 of the License are accompanied with this source code. See http://opcfoundation.org/License/GPLv2

   This source code is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#include "opcua_proxystub.h"

#include "sopc_namespace_table.h"
#include "sopc_stack_config.h"
#include "sopc_types_wrapper.h"

#ifndef OPCUA_PROXYSTUB_VERSIONSTRING
# define OPCUA_PROXYSTUB_VERSIONSTRING  "0.1"
#endif /* OPCUA_PROXYSTUB_VERSIONSTRING */

#ifndef OPCUA_PROXYSTUB_STATICCONFIGSTRING
# define OPCUA_PROXYSTUB_STATICCONFIGSTRING "default"
#endif /* OPCUA_PROXYSTUB_STATICCONFIGSTRING */

OpcUa_ProxyStubConfiguration        OpcUa_ProxyStub_g_Configuration;

static SOPC_NamespaceTable* gNsTable = NULL;

/*============================================================================
 * OpcUa_ProxyStub_Initialize
 *===========================================================================*/
OpcUa_StatusCode OpcUa_ProxyStub_Initialize(OpcUa_Handle                  a_pPlatformLayerCalltable,
                                            OpcUa_ProxyStubConfiguration* a_pProxyStubConfiguration)
{
    (void) a_pPlatformLayerCalltable;
    (void) a_pProxyStubConfiguration;
    return StackConfiguration_Initialize();
}

/*============================================================================
 * OpcUa_ProxyStub_Clear
 *===========================================================================*/
OpcUa_Void OpcUa_ProxyStub_Clear(OpcUa_Void)
{
    if(gNsTable != NULL){
        Namespace_Delete(gNsTable);
        gNsTable = NULL;
    }
    StackConfiguration_Clear();
}

/*============================================================================
 * OpcUa_ProxyStub_AddTypes
 *===========================================================================*/
OpcUa_StatusCode OpcUa_ProxyStub_AddTypes(OpcUa_EncodeableType** a_ppTypes)
{
    uint32_t idx = 0;
    OpcUa_EncodeableType* encType = a_ppTypes[idx];
    for(idx = 1; idx < UINT32_MAX && encType != NULL; idx++){
        encType = a_ppTypes[idx];
    }
    return StackConfiguration_AddTypes((SOPC_EncodeableType**) a_ppTypes, idx);
}

/*============================================================================
 * OpcUa_ProxyStub_SetNamespaceUris
 *===========================================================================*/
OpcUa_StatusCode OpcUa_ProxyStub_SetNamespaceUris(OpcUa_StringA* a_psNamespaceUris)
{
    if(gNsTable != NULL){
        return STATUS_NOK;
    }
    // count namespaces
    char* ns = NULL;
    uint32_t idx = 0;
    for(idx = 0; idx < UINT32_MAX && ns != NULL; idx++){
        ns = a_psNamespaceUris[idx];
    }
    gNsTable = Namespace_CreateTable(idx);
    for(idx = 0; idx < UINT32_MAX && ns != NULL; idx++){
        gNsTable->namespaceArray[idx].namespaceIndex = idx;
        gNsTable->namespaceArray[idx].namespaceName = a_psNamespaceUris[idx];
    }
    return StackConfiguration_SetNamespaceUris(gNsTable);
}

OpcUa_StatusCode OpcUa_P_Initialize(OpcUa_Handle* a_pPlatformLayerHandle){
    (void) a_pPlatformLayerHandle;
    return STATUS_OK;
}

OpcUa_StatusCode OpcUa_P_Clean(OpcUa_Handle* a_pPlatformLayerHandle){
    (void) a_pPlatformLayerHandle;
    return STATUS_OK;
}
