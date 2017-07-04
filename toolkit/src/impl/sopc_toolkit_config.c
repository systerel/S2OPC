/*
 *  Copyright (C) 2017 Systerel and others.
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

#include "sopc_toolkit_config.h"
#include "sopc_services_events.h"

#include <stdlib.h>
#include <string.h>

#include "sopc_stack_config.h"
#include "sopc_mutexes.h"
#include "singly_linked_list.h"

#include "io_dispatch_mgr.h"

static struct {
    uint8_t     initDone;
    uint8_t     locked;
    Mutex       mut;
    SLinkedList* scConfigs;
    SLinkedList* epConfigs;
} tConfig = {
    .initDone = FALSE,
    .locked = FALSE,
    .scConfigs = NULL,
    .epConfigs = NULL,
};

SOPC_EventDispatcherManager* servicesEventDispatcherMgr;

SOPC_StatusCode SOPC_Toolkit_Initialize(){
    SOPC_StatusCode status = OpcUa_BadInvalidState;
    if(tConfig.initDone == FALSE){
        Mutex_Initialization(&tConfig.mut);
        Mutex_Lock(&tConfig.mut);
        tConfig.initDone = !FALSE;
        status = SOPC_StackConfiguration_Initialize();
        if(STATUS_OK == status){
            tConfig.scConfigs = SLinkedList_Create(0);
        }
        if(STATUS_OK == status && NULL != tConfig.scConfigs){
            tConfig.epConfigs = SLinkedList_Create(0);
        }
        if(STATUS_OK == status && NULL != tConfig.epConfigs){
            servicesEventDispatcherMgr = SOPC_EventDispatcherManager_CreateAndStart(SOPC_ServicesEventDispatcher,
                                                                                    "Services event dispatcher manager");
        }
        Mutex_Unlock(&tConfig.mut);
    }
    return status;
}

SOPC_StatusCode SOPC_Toolkit_Configured(){
    SOPC_StatusCode status = OpcUa_BadInvalidState;
    if(tConfig.initDone != FALSE){
        Mutex_Lock(&tConfig.mut);
        if(tConfig.locked == FALSE){
            tConfig.locked = !FALSE;
            status = STATUS_OK;
        }
        Mutex_Unlock(&tConfig.mut);
    }
    return status;
}

void SOPC_Toolkit_Clear(){
    if(tConfig.initDone != FALSE){
        Mutex_Lock(&tConfig.mut);
        SOPC_StackConfiguration_Clear();
        io_dispatch_mgr__close_all_active_connections();
        SLinkedList_Delete(tConfig.scConfigs);
        SLinkedList_Delete(tConfig.epConfigs);
        tConfig.locked = FALSE;
        tConfig.initDone = FALSE;
        Mutex_Unlock(&tConfig.mut);
        Mutex_Clear(&tConfig.mut);
    }
}

static SOPC_StatusCode SOPC_IntToolkitConfig_AddConfig(SLinkedList* configList,
                                                       uint32_t     idx,
                                                       void*        config){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    void* res = SLinkedList_FindFromId(configList, idx);
    if(res == NULL){ // Idx is unique
        if(config == SLinkedList_Prepend(configList, idx, config)){
            status = STATUS_OK;
        }else{
            status = STATUS_NOK;
        }
    }
    return status;
}

SOPC_StatusCode SOPC_ToolkitConfig_AddSecureChannelConfig(uint32_t                   scConfigIdx,
                                                          SOPC_SecureChannel_Config* scConfig){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(NULL != scConfig){
        status = OpcUa_BadInvalidState;
        if(tConfig.initDone != FALSE){
            Mutex_Lock(&tConfig.mut);
            status = SOPC_IntToolkitConfig_AddConfig(tConfig.scConfigs, scConfigIdx, (void*) scConfig);
            Mutex_Unlock(&tConfig.mut);
        }
    }
    return status;
}

SOPC_SecureChannel_Config* SOPC_ToolkitConfig_GetSecureChannelConfig(uint32_t scConfigIdx){
    SOPC_SecureChannel_Config* res = NULL;
    if(tConfig.initDone != FALSE){
        Mutex_Lock(&tConfig.mut);
        if(tConfig.locked != FALSE){
            res = (SOPC_SecureChannel_Config*) SLinkedList_FindFromId(tConfig.scConfigs, scConfigIdx);
        }
        Mutex_Unlock(&tConfig.mut);
    }
    return res;
}

SOPC_StatusCode SOPC_ToolkitConfig_AddEndpointConfig(uint32_t              epConfigIdx,
                                                     SOPC_Endpoint_Config* epConfig){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(NULL != epConfig){
        status = OpcUa_BadInvalidState;
        if(tConfig.initDone != FALSE){
            Mutex_Lock(&tConfig.mut);
            status = SOPC_IntToolkitConfig_AddConfig(tConfig.epConfigs, epConfigIdx, (void*) epConfig);
            Mutex_Unlock(&tConfig.mut);
        }
    }
    return status;
}

SOPC_Endpoint_Config* SOPC_ToolkitConfig_GetEndpointConfig(uint32_t epConfigIdx){
    SOPC_Endpoint_Config* res = NULL;
    if(tConfig.initDone != FALSE){
        Mutex_Lock(&tConfig.mut);
        if(tConfig.locked != FALSE){
            res = (SOPC_Endpoint_Config*) SLinkedList_FindFromId(tConfig.epConfigs, epConfigIdx);
        }
        Mutex_Unlock(&tConfig.mut);
    }
    return res;
}

SOPC_StatusCode SOPC_ToolkitConfig_SetNamespaceUris(SOPC_NamespaceTable* nsTable){
    SOPC_StatusCode status = OpcUa_BadInvalidState;
    if(tConfig.initDone != FALSE){
        Mutex_Lock(&tConfig.mut);
        status = SOPC_StackConfiguration_SetNamespaceUris(nsTable);
        Mutex_Unlock(&tConfig.mut);
    }
    return status;
}

SOPC_StatusCode SOPC_ToolkitConfig_AddTypes(SOPC_EncodeableType** encTypesTable,
                                                   uint32_t              nbTypes){
    SOPC_StatusCode status = OpcUa_BadInvalidState;
    if(tConfig.initDone != FALSE){
        Mutex_Lock(&tConfig.mut);
        status = SOPC_StackConfiguration_AddTypes(encTypesTable, nbTypes);
        Mutex_Unlock(&tConfig.mut);
    }
    return status;
}

SOPC_EncodeableType** SOPC_ToolkitConfig_GetEncodeableTypes()
{
    SOPC_EncodeableType** res = NULL;
    if(tConfig.initDone != FALSE){
        Mutex_Lock(&tConfig.mut);
        if(tConfig.locked != FALSE){
            res = SOPC_StackConfiguration_GetEncodeableTypes();
        }
        Mutex_Unlock(&tConfig.mut);
    }
    return res;
}

SOPC_NamespaceTable* SOPC_ToolkitConfig_GetNamespaces()
{
    SOPC_NamespaceTable* res = NULL;
    if(tConfig.initDone != FALSE){
        Mutex_Lock(&tConfig.mut);
        if(tConfig.locked != FALSE){
            res = SOPC_StackConfiguration_GetNamespaces();
        }
        Mutex_Unlock(&tConfig.mut);
    }
    return res;
}
