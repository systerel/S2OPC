/*
 * Licensed to Systerel under one or more contributor license
 * agreements. See the NOTICE file distributed with this work
 * for additional information regarding copyright ownership.
 * Systerel licenses this file to you under the Apache
 * License, Version 2.0 (the "License"); you may not use this
 * file except in compliance with the License. You may obtain
 * a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "sopc_secure_channels_internal_ctx.h"
#include "sopc_sockets_api.h"

SOPC_SecureListener secureListenersArray[SOPC_MAX_ENDPOINT_DESCRIPTION_CONFIGURATIONS + 1];
SOPC_SecureConnection secureConnectionsArray[SOPC_MAX_SECURE_CONNECTIONS + 1];
uint32_t lastSecureConnectionArrayIdx = 0;

SOPC_Looper* secureChannelsLooper = NULL;
SOPC_EventHandler* secureChannelsInputEventHandler = NULL;
SOPC_EventHandler* secureChannelsInternalEventHandler = NULL;
SOPC_EventHandler* secureChannelsSocketsEventHandler = NULL;
SOPC_EventHandler* secureChannelsTimerEventHandler = NULL;
SOPC_EventHandler* secureChannelsEventHandler = NULL;

void SOPC_SecureChannelsInternalContext_Initialize(SOPC_SetListenerFunc setSocketsListener)
{
    memset(secureListenersArray, 0, sizeof(SOPC_SecureListener) * (SOPC_MAX_ENDPOINT_DESCRIPTION_CONFIGURATIONS + 1));
    memset(secureConnectionsArray, 0, sizeof(SOPC_SecureConnection) * (SOPC_MAX_SECURE_CONNECTIONS + 1));
    lastSecureConnectionArrayIdx = 0;

    secureChannelsLooper = SOPC_Looper_Create("SecureChannel");
    assert(secureChannelsLooper != NULL);

    secureChannelsInputEventHandler = SOPC_EventHandler_Create(secureChannelsLooper, SOPC_SecureChannels_OnInputEvent);
    assert(secureChannelsInputEventHandler != NULL);

    secureChannelsInternalEventHandler =
        SOPC_EventHandler_Create(secureChannelsLooper, SOPC_SecureChannels_OnInternalEvent);
    assert(secureChannelsInternalEventHandler != NULL);

    secureChannelsSocketsEventHandler =
        SOPC_EventHandler_Create(secureChannelsLooper, SOPC_SecureChannels_OnSocketsEvent);
    assert(secureChannelsSocketsEventHandler != NULL);

    secureChannelsTimerEventHandler = SOPC_EventHandler_Create(secureChannelsLooper, SOPC_SecureChannels_OnTimerEvent);
    assert(secureChannelsTimerEventHandler != NULL);

    setSocketsListener(secureChannelsSocketsEventHandler);
}

SOPC_SecureConnection* SC_GetConnection(uint32_t connectionIdx)
{
    SOPC_SecureConnection* scConnection = NULL;
    if (connectionIdx > 0 && connectionIdx <= SOPC_MAX_SECURE_CONNECTIONS)
    {
        scConnection = &(secureConnectionsArray[connectionIdx]);
    }
    return scConnection;
}

void SOPC_SecureChannelsInternalContext_Clear()
{
    SOPC_Looper_Delete(secureChannelsLooper);
}

const SOPC_Certificate* SC_OwnCertificate(SOPC_SecureConnection* conn)
{
    return conn->isServerConnection ? conn->serverCertificate : conn->clientCertificate;
}

const SOPC_Certificate* SC_PeerCertificate(SOPC_SecureConnection* conn)
{
    return conn->isServerConnection ? conn->clientCertificate : conn->serverCertificate;
}
