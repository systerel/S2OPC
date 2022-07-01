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

#include "sopc_macros.h"
#include "sopc_secure_channels_internal_ctx.h"
#include "sopc_sockets_api.h"

SOPC_SecureListener secureListenersArray[SOPC_MAX_ENDPOINT_DESCRIPTION_CONFIGURATIONS + 1];
SOPC_SecureConnection secureConnectionsArray[SOPC_MAX_SECURE_CONNECTIONS_PLUS_BUFFERED + 1];
uint32_t lastSecureConnectionArrayIdx = 0;

SOPC_Looper* secureChannelsLooper = NULL;
SOPC_EventHandler* secureChannelsInputEventHandler = NULL;
SOPC_EventHandler* secureChannelsInternalEventHandler = NULL;
SOPC_EventHandler* secureChannelsSocketsEventHandler = NULL;
SOPC_EventHandler* secureChannelsTimerEventHandler = NULL;
SOPC_EventHandler* secureChannelsEventHandler = NULL;

void SOPC_SecureChannelsInternalContext_Initialize(SOPC_SetListenerFunc* setSocketsListener)
{
    memset(secureListenersArray, 0, sizeof(SOPC_SecureListener) * (SOPC_MAX_ENDPOINT_DESCRIPTION_CONFIGURATIONS + 1));
    memset(secureConnectionsArray, 0, sizeof(SOPC_SecureConnection) * (SOPC_MAX_SECURE_CONNECTIONS_PLUS_BUFFERED + 1));
    lastSecureConnectionArrayIdx = 0;

    secureChannelsLooper = SOPC_Looper_Create("Secure_Channels");
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
    if (connectionIdx > 0 && connectionIdx <= SOPC_MAX_SECURE_CONNECTIONS_PLUS_BUFFERED)
    {
        scConnection = &(secureConnectionsArray[connectionIdx]);
    }
    return scConnection;
}

void SOPC_SecureChannelsInternalContext_Clear(void)
{
    SOPC_Looper_Delete(secureChannelsLooper);
}

const SOPC_CertificateList* SC_OwnCertificate(SOPC_SecureConnection* conn)
{
    return conn->isServerConnection ? conn->serverCertificate : conn->clientCertificate;
}

const SOPC_CertificateList* SC_PeerCertificate(SOPC_SecureConnection* conn)
{
    return conn->isServerConnection ? conn->clientCertificate : conn->serverCertificate;
}

uint32_t SOPC_ScInternalContext_GetNbIntermediateInputChunks(SOPC_SecureConnection_ChunkMgrCtx* chunkCtx)
{
    assert(NULL != chunkCtx);
    if (NULL == chunkCtx->intermediateChunksInputBuffers)
    {
        return 0;
    }
    else
    {
        return SOPC_SLinkedList_GetLength(chunkCtx->intermediateChunksInputBuffers);
    }
}

bool SOPC_ScInternalContext_AddIntermediateInputChunk(SOPC_SecureConnection_TcpProperties* tcpProperties,
                                                      SOPC_SecureConnection_ChunkMgrCtx* chunkCtx,
                                                      SOPC_Buffer* intermediateChunk)
{
    assert(NULL != chunkCtx);
    if (NULL == chunkCtx->intermediateChunksInputBuffers)
    {
        chunkCtx->intermediateChunksInputBuffers = SOPC_SLinkedList_Create(tcpProperties->receiveMaxChunkCount);
        if (NULL == chunkCtx->intermediateChunksInputBuffers)
        {
            return false;
        }
    }

    void* result = SOPC_SLinkedList_Append(chunkCtx->intermediateChunksInputBuffers,
                                           SOPC_SLinkedList_GetLength(chunkCtx->intermediateChunksInputBuffers),
                                           intermediateChunk);

    return NULL != result;
}

static void SOPC_ScInternalContext_DeleteIntermediateInputBuffer(uint32_t id, void* val)
{
    SOPC_UNUSED_ARG(id);
    SOPC_Buffer_Delete((SOPC_Buffer*) val);
}

void SOPC_ScInternalContext_ClearIntermediateInputChunks(SOPC_SecureConnection_ChunkMgrCtx* chunkCtx)
{
    assert(NULL != chunkCtx);
    if (NULL != chunkCtx->intermediateChunksInputBuffers)
    {
        SOPC_SLinkedList_Apply(chunkCtx->intermediateChunksInputBuffers,
                               SOPC_ScInternalContext_DeleteIntermediateInputBuffer);
        SOPC_SLinkedList_Delete(chunkCtx->intermediateChunksInputBuffers);
        chunkCtx->intermediateChunksInputBuffers = NULL;
    }
}

/** @brief Clear the current chunk context but not the intermediate chunks context */
void SOPC_ScInternalContext_ClearCurrentInputChunkContext(SOPC_SecureConnection_ChunkMgrCtx* chunkCtx)
{
    assert(NULL != chunkCtx);
    SOPC_Buffer_Delete(chunkCtx->currentChunkInputBuffer);
    chunkCtx->currentChunkInputBuffer = NULL;
    chunkCtx->currentMsgSize = 0;
    chunkCtx->currentMsgType = SOPC_MSG_TYPE_INVALID;
    chunkCtx->currentMsgIsFinal = SOPC_MSG_ISFINAL_INVALID;
}

/** @brief Clear the current chunk and intermediate chunks context */
void SOPC_ScInternalContext_ClearInputChunksContext(SOPC_SecureConnection_ChunkMgrCtx* chunkCtx)
{
    assert(NULL != chunkCtx);
    SOPC_ScInternalContext_ClearIntermediateInputChunks(chunkCtx);
    SOPC_ScInternalContext_ClearCurrentInputChunkContext(chunkCtx);
    // Delete complete message context:
    SOPC_Buffer_Delete(chunkCtx->currentMessageInputBuffer);
    chunkCtx->currentMessageInputBuffer = NULL;
    chunkCtx->hasCurrentMsgRequestId = false;
    chunkCtx->currentMsgRequestId = 0;
}
