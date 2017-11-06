/*
 *  Copyright (C) 2016 Systerel and others.
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

#include "sopc_secure_listener_state_mgr.h"

#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>

#include "sopc_toolkit_constants.h"
#include "sopc_toolkit_config.h"
#include "sopc_toolkit_config_internal.h"
#include "sopc_secure_channels_api.h"
#include "sopc_secure_channels_api_internal.h"
#include "sopc_secure_channels_internal_ctx.h"
#include "sopc_sockets_api.h"

#include "sopc_services_api.h"

static bool SOPC_SecureListenerStateMgr_OpeningListener(uint32_t endpointConfigIdx){
    bool result = false;
    SOPC_SecureListener* scListener = NULL;
    if(endpointConfigIdx < SOPC_MAX_ENDPOINT_DESCRIPTION_CONFIGURATIONS){
        scListener = &(secureListenersArray[endpointConfigIdx]);
        if(scListener->state == SECURE_LISTENER_STATE_CLOSED){
            scListener->state = SECURE_LISTENER_STATE_OPENING;
            scListener->serverEndpointConfigIdx = endpointConfigIdx;
            result = true;
        }
    }
    return result;
}

static bool SOPC_SecureListenerStateMgr_CloseListener(uint32_t endpointConfigIdx){
    SOPC_SecureListener* scListener = NULL;
    bool result = false;
    uint32_t idx = 0;
    if(endpointConfigIdx < SOPC_MAX_ENDPOINT_DESCRIPTION_CONFIGURATIONS){
        result = true;
        scListener = &(secureListenersArray[endpointConfigIdx]);
        if(scListener->state == SECURE_LISTENER_STATE_OPENED){
            // Close all active secure connections established on the listener
            for(idx = 0; idx < SOPC_MAX_SOCKETS_CONNECTIONS; idx++){
                if(scListener->isUsedConnectionIdxArray[idx] != false){
                    SOPC_SecureChannels_EnqueueInternalEventAsNext(INT_EP_SC_CLOSE,
                                                                   scListener->connectionIdxArray[idx],
                                                                   NULL,
                                                                   endpointConfigIdx);
                    scListener->isUsedConnectionIdxArray[idx] = false;
                    scListener->connectionIdxArray[idx] = 0;

                }
            }
            // Close the socket listener
            SOPC_Sockets_EnqueueEvent(SOCKET_CLOSE,
                    scListener->socketIndex,
                    NULL,
                    0);
            memset(scListener, 0, sizeof(SOPC_SecureListener));
        }else if(scListener->state == SECURE_LISTENER_STATE_OPENING){
            memset(scListener, 0, sizeof(SOPC_SecureListener));
        }
    }
    return result;
}

static SOPC_SecureListener* SOPC_SecureListenerStateMgr_GetListener(uint32_t endpointConfigIdx){
    SOPC_SecureListener* scListener = NULL;
    if(endpointConfigIdx < SOPC_MAX_ENDPOINT_DESCRIPTION_CONFIGURATIONS){
        scListener = &(secureListenersArray[endpointConfigIdx]);
    }
    return scListener;
}

static bool SOPC_SecureListenerStateMgr_AddConnection(SOPC_SecureListener* scListener,
                                                      uint32_t             newConnectionIndex){
    assert(scListener != NULL);
    uint32_t idx = (scListener->lastConnectionIdxArrayIdx + 1) % SOPC_MAX_SOCKETS_CONNECTIONS;
    uint32_t lastIdx = 0;
    bool result = false;
    do{
        lastIdx = idx;
        if(scListener->isUsedConnectionIdxArray[idx] == false){
            scListener->connectionIdxArray[idx] = newConnectionIndex;
            scListener->isUsedConnectionIdxArray[idx] = true;
            result = true;
        }
        idx = (idx + 1) % SOPC_MAX_SOCKETS_CONNECTIONS;
    }while(idx != scListener->lastConnectionIdxArrayIdx && result == false);

    if(result != false){
        scListener->lastConnectionIdxArrayIdx = lastIdx;
    }

    return result;
}

static void SOPC_SecureListenerStateMgr_RemoveConnection(SOPC_SecureListener* scListener,
                                                         uint32_t             connectionIndex){
    assert(scListener != NULL);
    uint32_t idx = 0;
    bool result = false;
    do{
        if(scListener->isUsedConnectionIdxArray[idx] != false &&
           scListener->connectionIdxArray[idx] == connectionIndex){
            scListener->isUsedConnectionIdxArray[idx] = false;
            scListener->connectionIdxArray[idx] = 0;
            result = true;
        }
        idx++;
    }while(idx < SOPC_MAX_SOCKETS_CONNECTIONS && result == false);
}

void SOPC_SecureListenerStateMgr_Dispatcher(SOPC_SecureChannels_InputEvent event,
                                            uint32_t                       eltId,
                                            void*                          params,
                                            int32_t                        auxParam){
    (void) params;
    bool result = false;
    SOPC_Endpoint_Config* epConfig = NULL;
    SOPC_SecureListener* scListener = NULL;
    switch(event){
    /* Sockets events: */
    /* Sockets manager -> SC listener state manager */
    case SOCKET_LISTENER_OPENED:
        if(SOPC_DEBUG_PRINTING != false){
            printf("ScListenerMgr: SOCKET_LISTENER_OPENED\n");
        }
        /* id = endpoint description config index,
           auxParam = socket index */
        scListener = SOPC_SecureListenerStateMgr_GetListener(eltId);
        if(scListener == NULL ||
           scListener->state != SECURE_LISTENER_STATE_OPENING){
            // Error case: require socket closure
            SOPC_Sockets_EnqueueEvent(SOCKET_CLOSE,
                                      auxParam,
                                      NULL,
                                      0);
        }else{
            scListener->state = SECURE_LISTENER_STATE_OPENED;
            scListener->socketIndex = auxParam;
        }
        break;
    case SOCKET_LISTENER_CONNECTION:
        if(SOPC_DEBUG_PRINTING != false){
            printf("ScListenerMgr: SOCKET_LISTENER_CONNECTION\n");
        }
        /* id = endpoint description config index,
           auxParam = new connection socket index */
        scListener = SOPC_SecureListenerStateMgr_GetListener(eltId);
        if(scListener == NULL || scListener->state != SECURE_LISTENER_STATE_OPENED){
            // Error case: require socket closure
            SOPC_Sockets_EnqueueEvent(SOCKET_CLOSE,
                                      auxParam,
                                      NULL,
                                      0);
        }else{
            // Request creation of a new secure connection with given socket
            SOPC_SecureChannels_EnqueueInternalEvent(INT_EP_SC_CREATE,
                                                     eltId,
                                                     NULL,
                                                     auxParam);
        }
        break;
    case SOCKET_LISTENER_FAILURE:
        if(SOPC_DEBUG_PRINTING != false){
            printf("ScListenerMgr: SOCKET_LISTENER_FAILURE\n");
        }
        /* id = endpoint description configuration index */
        epConfig = SOPC_ToolkitServer_GetEndpointConfig(eltId);
        if(epConfig != NULL){
            result = SOPC_SecureListenerStateMgr_CloseListener(eltId);
        }
        // Notify Services layer that EP_OPEN failed
        SOPC_Services_EnqueueEvent(SC_TO_SE_EP_CLOSED,
                                   eltId,
                                   NULL,
                                   0);
        break;
    /* Services events: */
    /* Services manager -> SC listener state manager */
    case EP_OPEN:
        if(SOPC_DEBUG_PRINTING != false){
            printf("ScListenerMgr: EP_OPEN\n");
        }
        /* id = endpoint description configuration index */
        // Retrieve EP configuration
        epConfig = SOPC_ToolkitServer_GetEndpointConfig(eltId);
        if(epConfig != NULL){
            result = SOPC_SecureListenerStateMgr_OpeningListener(eltId);
        }
        if(result == false){
            // Nothing to do: it means EP is already open or in opening step
        }else{
// URL is not modified but API cannot allow to keep const qualifier: cast to const on treatment
#pragma GCC diagnostic ignored "-Wcast-qual"
            // Notify Sockets layer to create the listener
            SOPC_Sockets_EnqueueEvent(SOCKET_CREATE_SERVER,
                                      eltId,
                                      (void*) epConfig->endpointURL,
                                      SOPC_LISTENER_LISTEN_ALL_INTERFACES);
#pragma GCC diagnostic pop
        }
        break;
    case EP_CLOSE:
        if(SOPC_DEBUG_PRINTING != false){
            printf("ScListenerMgr: EP_CLOSE\n");
        }
        /* id = endpoint description configuration index */
        epConfig = SOPC_ToolkitServer_GetEndpointConfig(eltId);
        if(epConfig != NULL){
            result = SOPC_SecureListenerStateMgr_CloseListener(eltId);
        }
        // Notify Services layer that EP_OPEN failed
        SOPC_Services_EnqueueEvent(SC_TO_SE_EP_CLOSED,
                                   eltId,
                                   NULL,
                                   result);
        break;
    /* Internal events: */
    /* SC connection manager -> SC listener state manager */
    case INT_EP_SC_CREATED:
        if(SOPC_DEBUG_PRINTING != false){
            printf("ScListenerMgr: INT_EP_SC_CREATED\n");
        }
        /* id = endpoint description configuration index,
           auxParam = socket index for connection */
        scListener = SOPC_SecureListenerStateMgr_GetListener(eltId);
        if(scListener == NULL || scListener->state != SECURE_LISTENER_STATE_OPENED){
            // Error case: require secure channel closure
            SOPC_Sockets_EnqueueEvent(INT_EP_SC_CLOSE,
                                      auxParam,
                                      NULL,
                                      eltId);
        }else{
            // Associates the secure channel connection to the secure listener
            result = SOPC_SecureListenerStateMgr_AddConnection(scListener,
                                                               auxParam);
            if(result == false){
                // Error case: require secure channel closure
                SOPC_Sockets_EnqueueEvent(INT_EP_SC_CLOSE,
                                          auxParam,
                                          NULL,
                                          eltId);
            }
        }
        break;
    case INT_EP_SC_DISCONNECTED:
        if(SOPC_DEBUG_PRINTING != false){
            printf("ScListenerMgr: INT_EP_SC_DISCONNECTED\n");
        }
        /* id = endpoint description configuration index,
           auxParam = secure channel connection index */
        scListener = SOPC_SecureListenerStateMgr_GetListener(eltId);
        if(scListener != NULL && scListener->state == SECURE_LISTENER_STATE_OPENED){
            SOPC_SecureListenerStateMgr_RemoveConnection(scListener,
                                                         auxParam);
        }
        break;
    default:
        // Already filtered by secure channels API module
        assert(false);
    }
}

