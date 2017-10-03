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
#include "sopc_toolkit_config_internal.h"
#include "sopc_user_app_itf.h"
#include "sopc_services_events.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "sopc_sockets_api.h"
#include "sopc_secure_channels_api.h"

#include "sopc_stack_config.h"
#include "sopc_mutexes.h"
#include "sopc_time.h"
#include "singly_linked_list.h"
#include "sopc_encodeable.h"

#include "io_dispatch_mgr.h"
#include "toolkit_header_init.h"
#include "address_space_impl.h"
#include "util_b2c.h"

static struct {
    uint8_t     initDone;
    uint8_t     locked;
    Mutex       mut;
    SLinkedList* scConfigs;
    SLinkedList* epConfigs;
} tConfig = {
    .initDone = false,
    .locked = false,
    .scConfigs = NULL,
    .epConfigs = NULL,
};

// Structure used to close all connections in a synchronous way
// (necessary on toolkit clear)
static struct {
    Mutex           mutex;
    Condition       cond;
    bool            allDisconnectedFlag;
    bool            requestedFlag;
} closeAllConnectionsSync = {
  .requestedFlag = false,
  .allDisconnectedFlag = false
};


static uint32_t scConfigIdxMax = 0;
static uint32_t epConfigIdxMax = 0;

SOPC_EventDispatcherManager* servicesEventDispatcherMgr = NULL;

SOPC_EventDispatcherManager* applicationEventDispatcherMgr = NULL;
static SOPC_ComEvent_Fct* appFct = NULL;
static SOPC_AddressSpaceNotif_Fct* pAddSpaceFct = NULL;

void SOPC_ApplicationEventDispatcher(int32_t  eventAndType, 
                                     uint32_t id, 
                                     void*    params, 
                                     uint32_t auxParam){
  switch(SOPC_AppEvent_AppEventType_Get(eventAndType)){
  case APP_COM_EVENT:
    if(NULL != appFct){
      if(SOPC_AppEvent_ComEvent_Get(eventAndType) == SE_ACTIVATED_SESSION){
        appFct(SOPC_AppEvent_ComEvent_Get(eventAndType),
               (void*) &id, // session id
               (SOPC_StatusCode) auxParam);
      }else{
        appFct(SOPC_AppEvent_ComEvent_Get(eventAndType),
               params, // TBD
               (SOPC_StatusCode) auxParam); // TBD
      }
      if(SOPC_AppEvent_ComEvent_Get(eventAndType) == SE_RCV_SESSION_RESPONSE){
        // Message to deallocate ? => if not application shall deallocate !
        SOPC_Encodeable_Delete(*(SOPC_EncodeableType**)params, &params);
        // TBD: free only in this case ?
        free(params);
      }
    }
    break;
  case APP_ADDRESS_SPACE_NOTIF:
    if(NULL != pAddSpaceFct){
      pAddSpaceFct(SOPC_AppEvent_AddSpaceEvent_Get(eventAndType),
                   params, // TBD
                   (SOPC_StatusCode) auxParam); // TBD
    }
    break;
  default:
    assert(false);
  }
}

SOPC_StatusCode SOPC_Toolkit_Initialize(SOPC_ComEvent_Fct* pAppFct){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(NULL != pAppFct){
        appFct = pAppFct;
        status = OpcUa_BadInvalidState;
        if(tConfig.initDone == false){
            status = STATUS_NOK;
            Mutex_Initialization(&tConfig.mut);
            Mutex_Lock(&tConfig.mut);
            tConfig.initDone = true;

            status = SOPC_StackConfiguration_Initialize();

            if(STATUS_OK == status){
                status = STATUS_NOK; // Set to OK at the end
                tConfig.scConfigs = SLinkedList_Create(0);
            }

            if(NULL != tConfig.scConfigs){
              tConfig.epConfigs = SLinkedList_Create(0);
            }

            if(NULL != tConfig.epConfigs){
                SOPC_Sockets_Initialize();
                SOPC_SecureChannels_Initialize();
                servicesEventDispatcherMgr =
                    SOPC_EventDispatcherManager_CreateAndStart(SOPC_ServicesEventDispatcher,
                                                               "Services event dispatcher manager");
            }
            if(NULL != servicesEventDispatcherMgr){
              applicationEventDispatcherMgr =
                  SOPC_EventDispatcherManager_CreateAndStart(SOPC_ApplicationEventDispatcher,
                                                             "(Services) Application event dispatcher manager");
            }

            if(NULL != applicationEventDispatcherMgr){
                status = STATUS_OK;
            }

            Mutex_Unlock(&tConfig.mut);
            // Init async close management flag
            Mutex_Initialization(&closeAllConnectionsSync.mutex);
            Condition_Init(&closeAllConnectionsSync.cond);

            if(STATUS_OK != status){
              SOPC_Toolkit_Clear();
            }
        }
    }
    return status;
}


void SOPC_Internal_AllClientSecureChannelsDisconnected(){
  Mutex_Lock(&closeAllConnectionsSync.mutex);
  if(closeAllConnectionsSync.requestedFlag != false){
    closeAllConnectionsSync.allDisconnectedFlag = true;
    Condition_SignalAll(&closeAllConnectionsSync.cond);
  }
  Mutex_Unlock(&closeAllConnectionsSync.mutex);
}

SOPC_StatusCode SOPC_Toolkit_Configured(){
    SOPC_StatusCode status = OpcUa_BadInvalidState;
    if(tConfig.initDone != false){
        Mutex_Lock(&tConfig.mut);
        if(tConfig.locked == false){
            tConfig.locked = true;
            /* Init B model */
            INITIALISATION();
            status = STATUS_OK;
        }
        Mutex_Unlock(&tConfig.mut);
    }
    return status;
}

void SOPC_Toolkit_ClearScConfigElt(uint32_t id, void *val)
{
    (void)(id);
    SOPC_SecureChannel_Config* scConfig = val;
    if(scConfig != NULL && scConfig->isClientSc == false){
        // In case of server it is an internally created config
        // => only client certificate was specifically allocated
        KeyManager_Certificate_Free((Certificate*) scConfig->crt_cli);
        free(scConfig);
    }
}

// Deallocate fields allocated on server side only and free all the SC configs
static void SOPC_Toolkit_ClearScConfigs(){
    SLinkedList_Apply(tConfig.scConfigs, SOPC_Toolkit_ClearScConfigElt);
    SLinkedList_Delete(tConfig.scConfigs);
    tConfig.scConfigs = NULL;
}

void SOPC_Toolkit_Clear(){
    SOPC_StatusCode status = STATUS_OK;
    if(tConfig.initDone != false){
      Mutex_Lock(&closeAllConnectionsSync.mutex);
      closeAllConnectionsSync.requestedFlag = true;
      // Do a synchronous connections closed (effective on client only)
      SOPC_EventDispatcherManager_AddEvent(servicesEventDispatcherMgr,
                                           APP_TO_SE_CLOSE_ALL_CONNECTIONS,
                                           0,
                                           NULL,
                                           0,
                                           "Services: Close all channel !");
      while(closeAllConnectionsSync.allDisconnectedFlag == false){
        Mutex_UnlockAndWaitCond(&closeAllConnectionsSync.cond, &closeAllConnectionsSync.mutex);
      }
      Mutex_Unlock(&closeAllConnectionsSync.mutex);
      Mutex_Clear(&closeAllConnectionsSync.mutex);
      Condition_Clear(&closeAllConnectionsSync.cond);
      Mutex_Lock(&tConfig.mut);
      SOPC_Sockets_Clear();
      SOPC_SecureChannels_Clear();
      status = SOPC_EventDispatcherManager_StopAndDelete(&servicesEventDispatcherMgr);
      (void) status; // log
      status = SOPC_EventDispatcherManager_StopAndDelete(&applicationEventDispatcherMgr);
      (void) status; // log
      SOPC_StackConfiguration_Clear();
      SOPC_Toolkit_ClearScConfigs();
      SLinkedList_Delete(tConfig.epConfigs);
      tConfig.epConfigs = NULL;
      address_space_bs__UNINITIALISATION();
      appFct = NULL;
      pAddSpaceFct = NULL;
      tConfig.locked = false;
      tConfig.initDone = false;
      Mutex_Unlock(&tConfig.mut);
      Mutex_Clear(&tConfig.mut);
    }
}

static SOPC_StatusCode SOPC_IntToolkitConfig_AddConfig(SLinkedList* configList,
                                                       uint32_t     idx,
                                                       void*        config){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    void* res = SLinkedList_FindFromId(configList, idx);
    if(NULL == res && NULL != config){ // Idx is unique
        if(config == SLinkedList_Prepend(configList, idx, config)){
            status = STATUS_OK;
        }else{
            status = STATUS_NOK;
        }
    }
    return status;
}

uint32_t SOPC_ToolkitClient_AddSecureChannelConfig(SOPC_SecureChannel_Config* scConfig){
  uint32_t result = 0;
  SOPC_StatusCode status;
  if(NULL != scConfig){
        if(tConfig.initDone != false){
            Mutex_Lock(&tConfig.mut);
            // TODO: check maximum value < max configs
            scConfigIdxMax++;
            status = SOPC_IntToolkitConfig_AddConfig(tConfig.scConfigs, scConfigIdxMax, (void*) scConfig);
            if(STATUS_OK == status){
              result = scConfigIdxMax;
            }else{
              scConfigIdxMax--;
            }
            Mutex_Unlock(&tConfig.mut);
        }
    }
    return result;
}

SOPC_SecureChannel_Config* SOPC_ToolkitClient_GetSecureChannelConfig(uint32_t scConfigIdx){
    SOPC_SecureChannel_Config* res = NULL;
    if(tConfig.initDone != false){
        Mutex_Lock(&tConfig.mut);
        if(tConfig.locked != false){
            res = (SOPC_SecureChannel_Config*) SLinkedList_FindFromId(tConfig.scConfigs, scConfigIdx);
        }
        Mutex_Unlock(&tConfig.mut);
    }
    return res;
}

uint32_t SOPC_ToolkitServer_AddEndpointConfig(SOPC_Endpoint_Config* epConfig){
    uint32_t result = 0;
    SOPC_StatusCode status;
    if(NULL != epConfig){
        if(tConfig.initDone != false){
            Mutex_Lock(&tConfig.mut);
            if(false == tConfig.locked){
              // TODO: check maximum value < max configs
              epConfigIdxMax++;
              status = SOPC_IntToolkitConfig_AddConfig(tConfig.epConfigs, epConfigIdxMax, (void*) epConfig);
              if(STATUS_OK == status){
                result = epConfigIdxMax;
              }else{
                epConfigIdxMax--;
              }
            }
            Mutex_Unlock(&tConfig.mut);
        }
    }
    return result;
}

SOPC_Endpoint_Config* SOPC_ToolkitServer_GetEndpointConfig(uint32_t epConfigIdx){
    SOPC_Endpoint_Config* res = NULL;
    if(tConfig.initDone != false){
        Mutex_Lock(&tConfig.mut);
        if(tConfig.locked != false){
            res = (SOPC_Endpoint_Config*) SLinkedList_FindFromId(tConfig.epConfigs, epConfigIdx);
        }
        Mutex_Unlock(&tConfig.mut);
    }
    return res;
}

SOPC_StatusCode SOPC_ToolkitConfig_SetNamespaceUris(SOPC_NamespaceTable* nsTable){
    SOPC_StatusCode status = OpcUa_BadInvalidState;
    if(tConfig.initDone != false){
        Mutex_Lock(&tConfig.mut);
        if(false == tConfig.locked){
          status = SOPC_StackConfiguration_SetNamespaceUris(nsTable);
        }
        Mutex_Unlock(&tConfig.mut);
    }
    return status;
}

SOPC_StatusCode SOPC_ToolkitConfig_AddTypes(SOPC_EncodeableType** encTypesTable,
                                                   uint32_t              nbTypes){
    SOPC_StatusCode status = OpcUa_BadInvalidState;
    if(tConfig.initDone != false){
        Mutex_Lock(&tConfig.mut);
        if(false == tConfig.locked){
          status = SOPC_StackConfiguration_AddTypes(encTypesTable, nbTypes);
        }
        Mutex_Unlock(&tConfig.mut);
    }
    return status;
}

SOPC_EncodeableType** SOPC_ToolkitConfig_GetEncodeableTypes()
{
    SOPC_EncodeableType** res = NULL;
    if(tConfig.initDone != false){
        Mutex_Lock(&tConfig.mut);
        if(tConfig.locked != false){
            res = SOPC_StackConfiguration_GetEncodeableTypes();
        }
        Mutex_Unlock(&tConfig.mut);
    }
    return res;
}

SOPC_NamespaceTable* SOPC_ToolkitConfig_GetNamespaces()
{
    SOPC_NamespaceTable* res = NULL;
    if(tConfig.initDone != false){
        Mutex_Lock(&tConfig.mut);
        if(tConfig.locked != false){
            res = SOPC_StackConfiguration_GetNamespaces();
        }
        Mutex_Unlock(&tConfig.mut);
    }
    return res;
}

int32_t SOPC_AppEvent_ComEvent_Create(SOPC_App_Com_Event event){
  return APP_COM_EVENT + (event << 8);
}

int32_t SOPC_AppEvent_AddSpaceEvent_Create(SOPC_App_AddSpace_Event event){
  return APP_ADDRESS_SPACE_NOTIF + (event << 8);
}

SOPC_App_EventType SOPC_AppEvent_AppEventType_Get(int32_t iEvent){
  return (iEvent & 0xFF);
}

SOPC_App_Com_Event SOPC_AppEvent_ComEvent_Get(int32_t iEvent){
  return (iEvent >> 8);
}

SOPC_App_AddSpace_Event SOPC_AppEvent_AddSpaceEvent_Get(int32_t iEvent){
    return (iEvent >> 8);
}

void SOPC_Internal_ToolkitServer_SetAddressSpaceConfig(SOPC_AddressSpace* addressSpace){
    /* Glue the address_space_bs machine content to the generated address space content */
   
    /* Number of nodes by nodeclass */
    assert(addressSpace->nbNodesTotal <= INT32_MAX);
    address_space_bs__nNodeIds = addressSpace->nbNodesTotal;
    address_space_bs__nVariables = addressSpace->nbVariables;
    address_space_bs__nVariableTypes = addressSpace->nbVariableTypes;
    address_space_bs__nObjectTypes = addressSpace->nbObjectTypes;
    address_space_bs__nReferenceTypes = addressSpace->nbReferenceTypes;
    address_space_bs__nDataTypes = addressSpace->nbDataTypes;
    address_space_bs__nMethods = addressSpace->nbMethods;
    address_space_bs__nObjects = addressSpace->nbObjects;
    address_space_bs__nViews = addressSpace->nbViews;
    
    /* Attributes */
    address_space_bs__a_NodeId = addressSpace->nodeIdArray;
    /* Converts NodeClasses */
    address_space_bs__a_NodeClass = addressSpace->nodeClassArray;
    address_space_bs__a_BrowseName = addressSpace->browseNameArray;
    address_space_bs__a_DisplayName = addressSpace->displayNameArray;
    address_space_bs__a_DisplayName_begin = addressSpace->displayNameIdxArray_begin;
    address_space_bs__a_DisplayName_end = addressSpace->displayNameIdxArray_end;
    address_space_bs__a_Value = addressSpace->valueArray;
    /* Converts status codes */
    address_space_bs__a_Value_StatusCode = addressSpace->valueStatusArray;
    address_space_bs__HasTypeDefinition = NULL;
    
    /* References */
    address_space_bs__refs_ReferenceType = addressSpace->referenceTypeArray;
    address_space_bs__refs_TargetNode = addressSpace->referenceTargetArray;
    address_space_bs__refs_IsForward = addressSpace->referenceIsForwardArray;
    address_space_bs__RefIndexBegin = addressSpace->referenceIdxArray_begin;
    address_space_bs__RefIndexEnd = addressSpace->referenceIdxArray_end;   
}

SOPC_StatusCode SOPC_ToolkitServer_SetAddressSpaceConfig(SOPC_AddressSpace* addressSpace){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(addressSpace != NULL){
        status = STATUS_INVALID_STATE;
        if(tConfig.initDone != false){
            Mutex_Lock(&tConfig.mut);
            if(false == tConfig.locked){
                status = STATUS_OK;
                SOPC_Internal_ToolkitServer_SetAddressSpaceConfig(addressSpace);
            }
            Mutex_Unlock(&tConfig.mut);
        }
    }
    return status;
}
