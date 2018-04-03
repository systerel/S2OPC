/*
 *  Copyright (C) 2018 Systerel and others.
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

#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "sopc_logger.h"
#include "sopc_secure_channels_api.h"
#include "sopc_secure_channels_api_internal.h"
#include "sopc_secure_channels_internal_ctx.h"
#include "sopc_sockets_api.h"
#include "sopc_toolkit_config.h"
#include "sopc_toolkit_config_internal.h"
#include "sopc_toolkit_constants.h"

#include "sopc_services_api.h"

static bool SOPC_SecureListenerStateMgr_OpeningListener(uint32_t endpointConfigIdx)
{
    bool result = false;
    SOPC_SecureListener* scListener = NULL;
    if (endpointConfigIdx > 0 && endpointConfigIdx <= SOPC_MAX_ENDPOINT_DESCRIPTION_CONFIGURATIONS)
    {
        scListener = &(secureListenersArray[endpointConfigIdx]);
        if (scListener->state == SECURE_LISTENER_STATE_CLOSED)
        {
            scListener->state = SECURE_LISTENER_STATE_OPENING;
            scListener->serverEndpointConfigIdx = endpointConfigIdx;
            result = true;
        }
    }
    return result;
}

static bool SOPC_SecureListenerStateMgr_CloseListener(uint32_t endpointConfigIdx)
{
    SOPC_SecureListener* scListener = NULL;
    bool result = false;
    uint32_t idx = 0;
    if (endpointConfigIdx > 0 && endpointConfigIdx <= SOPC_MAX_ENDPOINT_DESCRIPTION_CONFIGURATIONS)
    {
        result = true;
        scListener = &(secureListenersArray[endpointConfigIdx]);
        if (scListener->state == SECURE_LISTENER_STATE_OPENED)
        {
            // Close all active secure connections established on the listener
            for (idx = 0; idx < SOPC_MAX_SOCKETS_CONNECTIONS; idx++)
            {
                if (scListener->isUsedConnectionIdxArray[idx] != false)
                {
                    SOPC_SecureChannels_EnqueueInternalEventAsNext(INT_EP_SC_CLOSE, scListener->connectionIdxArray[idx],
                                                                   NULL, endpointConfigIdx);
                    scListener->isUsedConnectionIdxArray[idx] = false;
                    scListener->connectionIdxArray[idx] = 0;
                }
            }
            // Close the socket listener
            SOPC_Sockets_EnqueueEvent(SOCKET_CLOSE, scListener->socketIndex, NULL, 0);
            memset(scListener, 0, sizeof(SOPC_SecureListener));
        }
        else if (scListener->state == SECURE_LISTENER_STATE_OPENING)
        {
            memset(scListener, 0, sizeof(SOPC_SecureListener));
        }
    }
    return result;
}

static SOPC_SecureListener* SOPC_SecureListenerStateMgr_GetListener(uint32_t endpointConfigIdx)
{
    SOPC_SecureListener* scListener = NULL;
    if (endpointConfigIdx > 0 && endpointConfigIdx <= SOPC_MAX_ENDPOINT_DESCRIPTION_CONFIGURATIONS)
    {
        scListener = &(secureListenersArray[endpointConfigIdx]);
    }
    return scListener;
}

static bool SOPC_SecureListenerStateMgr_AddConnection(SOPC_SecureListener* scListener, uint32_t newConnectionIndex)
{
    assert(scListener != NULL);
    uint32_t idx = (scListener->lastConnectionIdxArrayIdx + 1) % SOPC_MAX_SOCKETS_CONNECTIONS;
    uint32_t lastIdx = 0;
    bool result = false;
    do
    {
        lastIdx = idx;
        if (false == scListener->isUsedConnectionIdxArray[idx])
        {
            scListener->connectionIdxArray[idx] = newConnectionIndex;
            scListener->isUsedConnectionIdxArray[idx] = true;
            result = true;
        }
        idx = (idx + 1) % SOPC_MAX_SOCKETS_CONNECTIONS;
    } while (idx != scListener->lastConnectionIdxArrayIdx && false == result);

    if (result != false)
    {
        scListener->lastConnectionIdxArrayIdx = lastIdx;
    }

    return result;
}

static void SOPC_SecureListenerStateMgr_RemoveConnection(SOPC_SecureListener* scListener, uint32_t connectionIndex)
{
    assert(scListener != NULL);
    uint32_t idx = 0;
    bool result = false;
    do
    {
        if (scListener->isUsedConnectionIdxArray[idx] != false &&
            scListener->connectionIdxArray[idx] == connectionIndex)
        {
            scListener->isUsedConnectionIdxArray[idx] = false;
            scListener->connectionIdxArray[idx] = 0;
            result = true;
        }
        idx++;
    } while (idx < SOPC_MAX_SOCKETS_CONNECTIONS && false == result);
}

void SOPC_SecureListenerStateMgr_Dispatcher(SOPC_SecureChannels_InputEvent event,
                                            uint32_t eltId,
                                            void* params,
                                            uintptr_t auxParam)
{
    (void) params;
    bool result = false;
    SOPC_Endpoint_Config* epConfig = NULL;
    SOPC_SecureListener* scListener = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    switch (event)
    {
    /* Sockets events: */
    /* Sockets manager -> SC listener state manager */
    case SOCKET_LISTENER_OPENED:
        SOPC_Logger_TraceDebug("ScListenerMgr: SOCKET_LISTENER_OPENED epCfgIdx=%" PRIu32 " socketIdx=%" PRIuPTR, eltId,
                               auxParam);
        /* id = endpoint description config index,
           auxParam = socket index */
        if (auxParam <= UINT32_MAX)
        {
            scListener = SOPC_SecureListenerStateMgr_GetListener(eltId);
            if (NULL == scListener || scListener->state != SECURE_LISTENER_STATE_OPENING)
            {
                // Error case: require socket closure
                SOPC_Sockets_EnqueueEvent(SOCKET_CLOSE, (uint32_t) auxParam, NULL, 0);
            }
            else
            {
                scListener->state = SECURE_LISTENER_STATE_OPENED;
                scListener->socketIndex = (uint32_t) auxParam;
            }
        }
        break;
    case SOCKET_LISTENER_CONNECTION:
        SOPC_Logger_TraceDebug("ScListenerMgr: SOCKET_LISTENER_CONNECTION epCfgIdx=%" PRIu32 " socketIdx=%" PRIuPTR,
                               eltId, auxParam);
        /* id = endpoint description config index,
           auxParam = new connection socket index */
        scListener = SOPC_SecureListenerStateMgr_GetListener(eltId);
        if (NULL == scListener || scListener->state != SECURE_LISTENER_STATE_OPENED)
        {
            if (auxParam <= UINT32_MAX)
            {
                // Error case: require socket closure
                SOPC_Sockets_EnqueueEvent(SOCKET_CLOSE, (uint32_t) auxParam, NULL, 0);
            }
        }
        else
        {
            // Request creation of a new secure connection with given socket
            SOPC_SecureChannels_EnqueueInternalEvent(INT_EP_SC_CREATE, eltId, NULL, auxParam);
        }
        break;
    case SOCKET_LISTENER_FAILURE:
        SOPC_Logger_TraceDebug("ScListenerMgr: SOCKET_LISTENER_FAILURE epCfgIdx=%" PRIu32, eltId);
        /* id = endpoint description configuration index */
        epConfig = SOPC_ToolkitServer_GetEndpointConfig(eltId);
        if (epConfig != NULL)
        {
            SOPC_SecureListenerStateMgr_CloseListener(eltId);
        }
        // Notify Services layer that EP_OPEN failed
        SOPC_Services_EnqueueEvent(SC_TO_SE_EP_CLOSED, eltId, NULL, SOPC_STATUS_CLOSED);
        break;
    /* Services events: */
    /* Services manager -> SC listener state manager */
    case EP_OPEN:
        SOPC_Logger_TraceDebug("ScListenerMgr: EP_OPEN epCfgIdx=%" PRIu32, eltId);
        /* id = endpoint description configuration index */
        // Retrieve EP configuration
        epConfig = SOPC_ToolkitServer_GetEndpointConfig(eltId);
        if (epConfig != NULL)
        {
            result = SOPC_SecureListenerStateMgr_OpeningListener(eltId);
        }
        if (false == result)
        {
            // Nothing to do: it means EP is already open or in opening step
        }
        else
        {
            // URL is not modified but API cannot allow to keep const qualifier: cast to const on treatment
            SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
            // Notify Sockets layer to create the listener
            SOPC_Sockets_EnqueueEvent(SOCKET_CREATE_SERVER, eltId, (void*) epConfig->endpointURL,
                                      SOPC_LISTENER_LISTEN_ALL_INTERFACES);
            SOPC_GCC_DIAGNOSTIC_RESTORE
        }
        break;
    case EP_CLOSE:
        SOPC_Logger_TraceDebug("ScListenerMgr: EP_CLOSE epCfgIdx=%" PRIu32, eltId);
        /* id = endpoint description configuration index */
        epConfig = SOPC_ToolkitServer_GetEndpointConfig(eltId);
        if (epConfig != NULL)
        {
            result = SOPC_SecureListenerStateMgr_CloseListener(eltId);
            if (result == false)
            {
                status = SOPC_STATUS_INVALID_PARAMETERS;
            }
            else
            {
                status = SOPC_STATUS_OK;
            }
        }
        // Notify Services layer that EP_OPEN failed
        SOPC_Services_EnqueueEvent(SC_TO_SE_EP_CLOSED, eltId, NULL, status);
        break;
    /* Internal events: */
    /* SC connection manager -> SC listener state manager */
    case INT_EP_SC_CREATED:
        SOPC_Logger_TraceDebug("ScListenerMgr: INT_EP_SC_CREATED epCfgIdx=%" PRIu32 " scIdx=%" PRIuPTR, eltId,
                               auxParam);
        /* id = endpoint description configuration index,
           auxParam = socket index for connection */
        if (auxParam <= UINT32_MAX)
        {
            scListener = SOPC_SecureListenerStateMgr_GetListener(eltId);
            if (NULL == scListener || scListener->state != SECURE_LISTENER_STATE_OPENED)
            {
                // Error case: require secure channel closure
                SOPC_SecureChannels_EnqueueInternalEvent(INT_EP_SC_CLOSE, (uint32_t) auxParam, NULL, eltId);
            }
            else
            {
                // Associates the secure channel connection to the secure listener
                result = SOPC_SecureListenerStateMgr_AddConnection(scListener, (uint32_t) auxParam);
                if (false == result)
                {
                    // Error case: require secure channel closure
                    SOPC_SecureChannels_EnqueueInternalEvent(INT_EP_SC_CLOSE, (uint32_t) auxParam, NULL, eltId);
                }
            }
        }
        break;
    case INT_EP_SC_DISCONNECTED:
        SOPC_Logger_TraceDebug("ScListenerMgr: INT_EP_SC_DISCONNECTED epCfgIdx=%" PRIu32 " scIdx=%" PRIuPTR, eltId,
                               auxParam);

        /* id = endpoint description configuration index,
           auxParam = secure channel connection index */
        scListener = SOPC_SecureListenerStateMgr_GetListener(eltId);
        if (scListener != NULL && scListener->state == SECURE_LISTENER_STATE_OPENED && auxParam <= UINT32_MAX)
        {
            SOPC_SecureListenerStateMgr_RemoveConnection(scListener, (uint32_t) auxParam);
        }
        break;
    default:
        // Already filtered by secure channels API module
        assert(false);
    }
}
