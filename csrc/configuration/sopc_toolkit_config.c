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
#include "sopc_services_api.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "sopc_helper_endianess_cfg.h"
#include "sopc_sockets_api.h"
#include "sopc_secure_channels_api.h"

#include "sopc_mutexes.h"
#include "sopc_time.h"
#include "sopc_singly_linked_list.h"
#include "sopc_encodeable.h"

#include "address_space_impl.h"
#include "util_b2c.h"

static struct {
    uint8_t               initDone;
    uint8_t               locked;
    Mutex                 mut;
    SOPC_SLinkedList* scConfigs;
    SOPC_SLinkedList* epConfigs;
    /* OPC UA namespace and encodeable types */
    SOPC_NamespaceTable*  nsTable;
    SOPC_EncodeableType** encTypesTable;
    uint32_t              nbEncTypesTable;
} tConfig = {
    .initDone = false,
    .locked = false,
    .scConfigs = NULL,
    .epConfigs = NULL,
    .nsTable = NULL,
    .encTypesTable = NULL,
    .nbEncTypesTable = 0,
};

static uint32_t scConfigIdxMax = 0;
static uint32_t epConfigIdxMax = 0;

static SOPC_ComEvent_Fct* appFct = NULL;
static SOPC_AddressSpaceNotif_Fct* pAddSpaceFct = NULL;

void SOPC_Internal_ApplicationEventDispatcher(int32_t  eventAndType,
                                              uint32_t id,
                                              void*    params,
                                              uint32_t auxParam){
  switch(SOPC_AppEvent_AppEventType_Get(eventAndType)){
  case SOPC_APP_COM_EVENT:
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
  case SOPC_APP_ADDRESS_SPACE_NOTIF:
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
    if(NULL != pAppFct){
        appFct = pAppFct;
        if(false == tConfig.initDone){
            Mutex_Initialization(&tConfig.mut);
            Mutex_Lock(&tConfig.mut);
            tConfig.initDone = true;

            SOPC_Helper_EndianessCfg_Initialize();
            SOPC_Namespace_Initialize(tConfig.nsTable);
            tConfig.scConfigs = SOPC_SLinkedList_Create(0);

            if(NULL != tConfig.scConfigs){
              tConfig.epConfigs = SOPC_SLinkedList_Create(0);
            }

            if(NULL != tConfig.epConfigs){
                SOPC_Sockets_Initialize();
                SOPC_SecureChannels_Initialize();
                SOPC_Services_Initialize();
            }

            Mutex_Unlock(&tConfig.mut);
        }
    }
    return STATUS_OK;
}

SOPC_StatusCode SOPC_Toolkit_Configured(){
    SOPC_StatusCode status = OpcUa_BadInvalidState;
    if(tConfig.initDone != false){
        Mutex_Lock(&tConfig.mut);
        if(false == tConfig.locked){
            tConfig.locked = true;
            SOPC_Services_ToolkitConfigured();
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
    if(scConfig != NULL && false == scConfig->isClientSc){
        // In case of server it is an internally created config
        // => only client certificate was specifically allocated
// Exceptional case: configuration added internally and shall be freed on clear call
SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
        SOPC_KeyManager_Certificate_Free((SOPC_Certificate*) scConfig->crt_cli);
SOPC_GCC_DIAGNOSTIC_RESTORE
        free(scConfig);
    }
}

// Deallocate fields allocated on server side only and free all the SC configs
static void SOPC_Toolkit_ClearScConfigs(void) {
    SOPC_SLinkedList_Apply(tConfig.scConfigs, SOPC_Toolkit_ClearScConfigElt);
    SOPC_SLinkedList_Delete(tConfig.scConfigs);
    tConfig.scConfigs = NULL;
}

void SOPC_Toolkit_Clear(){
    if(tConfig.initDone != false){
      // Services are in charge to gracefully close all connections.
      // It must be done before stopping the services
      SOPC_Services_PreClear();

      SOPC_Sockets_Clear();
      SOPC_SecureChannels_Clear();
      SOPC_Services_Clear();

      Mutex_Lock(&tConfig.mut);
      if(tConfig.encTypesTable != NULL){
          free(tConfig.encTypesTable);
      }
      tConfig.nsTable = NULL;
      tConfig.encTypesTable = NULL;
      tConfig.nbEncTypesTable = 0;

      SOPC_Toolkit_ClearScConfigs();
      SOPC_SLinkedList_Delete(tConfig.epConfigs);
      tConfig.epConfigs = NULL;
      appFct = NULL;
      pAddSpaceFct = NULL;
      tConfig.locked = false;
      tConfig.initDone = false;
      Mutex_Unlock(&tConfig.mut);
      Mutex_Clear(&tConfig.mut);
    }
}

static SOPC_StatusCode SOPC_IntToolkitConfig_AddConfig(SOPC_SLinkedList* configList,
                                                       uint32_t          idx,
                                                       void*             config){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    void* res = SOPC_SLinkedList_FindFromId(configList, idx);
    if(NULL == res && NULL != config){ // Idx is unique
        if(config == SOPC_SLinkedList_Prepend(configList, idx, config)){
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
      // TODO: check all parameters of scConfig (requested lifetime >= MIN, etc)
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

SOPC_SecureChannel_Config* SOPC_Toolkit_GetSecureChannelConfig(uint32_t scConfigIdx){
    SOPC_SecureChannel_Config* res = NULL;
    if(tConfig.initDone != false){
        Mutex_Lock(&tConfig.mut);
        if(tConfig.locked != false){
            res = (SOPC_SecureChannel_Config*) SOPC_SLinkedList_FindFromId(tConfig.scConfigs, scConfigIdx);
        }
        Mutex_Unlock(&tConfig.mut);
    }
    return res;
}

uint32_t SOPC_ToolkitServer_AddEndpointConfig(SOPC_Endpoint_Config* epConfig){
    uint32_t result = 0;
    SOPC_StatusCode status;
    if(NULL != epConfig){
        // TODO: check all parameters of epConfig: certificate presence w.r.t. secu policy, app desc (Uris are valid w.r.t. part 6), etc.
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
            res = (SOPC_Endpoint_Config*) SOPC_SLinkedList_FindFromId(tConfig.epConfigs, epConfigIdx);
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
            status = STATUS_OK;
            tConfig.nsTable = nsTable;
        }
        Mutex_Unlock(&tConfig.mut);
    }
    return status;
}

static uint32_t GetKnownEncodeableTypesLength(void) {
    uint32_t result = 0;
    for(result = 0; SOPC_KnownEncodeableTypes[result] != NULL; result++);
    return result + 1;
}

SOPC_StatusCode SOPC_ToolkitConfig_AddTypes(SOPC_EncodeableType** encTypesTable,
                                                   uint32_t              nbTypes){
    SOPC_StatusCode status = OpcUa_BadInvalidState;
    if(tConfig.initDone != false){
        Mutex_Lock(&tConfig.mut);
        if(false == tConfig.locked){
            uint32_t idx = 0;
            uint32_t nbKnownTypes = 0;
            SOPC_EncodeableType** additionalTypes = NULL;

            if(encTypesTable != NULL && nbTypes > 0 ){
                status = STATUS_OK;
                if(NULL == tConfig.encTypesTable){
                    // known types to be added
                    nbKnownTypes = GetKnownEncodeableTypesLength();
                    // +1 for null value termination
                    tConfig.encTypesTable = malloc(sizeof(SOPC_EncodeableType*) * (nbKnownTypes + nbTypes + 1));
                    if(NULL == tConfig.encTypesTable ||
                       tConfig.encTypesTable != memcpy(tConfig.encTypesTable,
                                                       SOPC_KnownEncodeableTypes,
                                                       nbKnownTypes * sizeof(SOPC_EncodeableType*)))
                    {
                        tConfig.encTypesTable = NULL;
                    }else{
                        additionalTypes = tConfig.encTypesTable;
                        tConfig.nbEncTypesTable = nbKnownTypes;
                    }
                }else{
                    // +1 for null value termination
                    additionalTypes = realloc(tConfig.encTypesTable,
                                              sizeof(SOPC_EncodeableType*) * tConfig.nbEncTypesTable + nbTypes + 1);
                }

                if(additionalTypes != NULL){
                    tConfig.encTypesTable = additionalTypes;

                    for(idx = 0; idx < nbTypes; idx++){
                        tConfig.encTypesTable[tConfig.nbEncTypesTable + idx] = encTypesTable[idx];
                    }
                    tConfig.nbEncTypesTable += nbTypes;
                    // NULL terminated table

                }else{
                    status = STATUS_NOK;
                }
            }
            return status;
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
            if (tConfig.encTypesTable != NULL && tConfig.nbEncTypesTable > 0){
                // Additional types are present: contains known types + additional
                res = tConfig.encTypesTable;
            }else{
                // No additional types: return static known types
                res = SOPC_KnownEncodeableTypes;
            }
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
            res = tConfig.nsTable;
        }
        Mutex_Unlock(&tConfig.mut);
    }
    return res;
}

int32_t SOPC_AppEvent_ComEvent_Create(SOPC_App_Com_Event event){
  return SOPC_APP_COM_EVENT + (event << 8);
}

int32_t SOPC_AppEvent_AddSpaceEvent_Create(SOPC_App_AddSpace_Event event){
  return SOPC_APP_ADDRESS_SPACE_NOTIF + (event << 8);
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
