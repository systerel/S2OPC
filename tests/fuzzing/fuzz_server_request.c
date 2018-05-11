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

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "sopc_buffer.h"
#include "sopc_chunks_mgr_internal.h"
#include "sopc_helper_endianness_cfg.h"
#include "sopc_secure_channels_internal_ctx.h"
#include "sopc_secure_connection_state_mgr.h"
#include "sopc_secure_connection_state_mgr_internal.h"

int LLVMFuzzerTestOneInput(const uint8_t* buf, size_t len)
{
    assert(len <= UINT32_MAX);

    if (len == 0)
    {
        return 0;
    }

    SOPC_Helper_EndiannessCfg_Initialize();

    SOPC_Buffer* sopc_buffer = SOPC_Buffer_Create((uint32_t) len);
    assert(sopc_buffer != NULL);
    assert(SOPC_Buffer_Write(sopc_buffer, buf, (uint32_t) len) == SOPC_STATUS_OK);
    SOPC_Buffer_SetPosition(sopc_buffer, 0);

    uint32_t conn_idx;
    assert(SC_InitNewConnection(&conn_idx));
    SOPC_SecureConnection* sc = SC_GetConnection(conn_idx);
    sc->isServerConnection = true;
    SOPC_SecureConnection_ChunkMgrCtx* chunkCtx = &sc->chunksCtx;
    SOPC_StatusCode errorStatus = SOPC_GoodGenericStatus;
    uint32_t request_id;

    while (SOPC_Buffer_Remaining(sopc_buffer) > 0)
    {
        if (NULL == chunkCtx->chunkInputBuffer)
        {
            // No incomplete message data: create a new buffer
            chunkCtx->chunkInputBuffer = SOPC_Buffer_Create(sc->tcpMsgProperties.receiveBufferSize);
            assert(chunkCtx->chunkInputBuffer != NULL);
        }

        if (!SC_Chunks_DecodeReceivedBuffer(&sc->chunksCtx, sopc_buffer, &errorStatus))
        {
            break;
        }

        // Decode OPC UA Secure Conversation MessageChunk specific headers if necessary (not HEL/ACK/ERR)
        if (SC_Chunks_TreatTcpPayload(sc, &request_id, &errorStatus))
        {
            SOPC_Buffer_Delete(chunkCtx->chunkInputBuffer);
            chunkCtx->chunkInputBuffer = NULL;
        }
        else
        {
            break;
        }
    }

    SC_CloseConnection(conn_idx);
    SOPC_Buffer_Delete(chunkCtx->chunkInputBuffer);
    SOPC_Buffer_Delete(sopc_buffer);

    return 0;
}
