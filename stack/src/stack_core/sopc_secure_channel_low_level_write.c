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

#include "sopc_secure_channel_low_level.h"
#include "sopc_tcp_ua_low_level.h"

#include <assert.h>

SOPC_StatusCode SC_WriteSecureMsgBuffer(SOPC_MsgBuffer*  msgBuffer,
                                        const SOPC_Byte* data_src,
                                        uint32_t         count){
    SOPC_StatusCode status = STATUS_NOK;
    SOPC_String reason;
    SC_Connection* scConnection = NULL;
    if(msgBuffer == NULL){
        return STATUS_INVALID_PARAMETERS;
    }
    if(msgBuffer->type == TCP_UA_Message_SecureMessage){
        // Use chunks mechanism if it is a secure message
        if(data_src == NULL || msgBuffer == NULL || msgBuffer->flushData == NULL)
        {
            status = STATUS_INVALID_PARAMETERS;
        }else{
            status = STATUS_OK;
            scConnection = (SC_Connection*) msgBuffer->flushData;
            assert(msgBuffer->sequenceNumberPosition +
                    UA_SECURE_MESSAGE_SEQUENCE_LENGTH +
                    scConnection->sendingMaxBodySize >= msgBuffer->buffers->position);

            while(STATUS_OK == status && // if not enough space in current buffer, create an intermediary chunk
                  (msgBuffer->buffers->position + count >
                   msgBuffer->sequenceNumberPosition +
                   UA_SECURE_MESSAGE_SEQUENCE_LENGTH +
                   scConnection->sendingMaxBodySize))
            {
                // Precedent position cannot be greater than message size:
                //  otherwise it means size has not been checked precedent time (it could occurs only when writing headers)
                assert(msgBuffer->sequenceNumberPosition +
                        UA_SECURE_MESSAGE_SEQUENCE_LENGTH +
                        scConnection->sendingMaxBodySize >= msgBuffer->buffers->position);
                if(msgBuffer->maxChunks != 0 && msgBuffer->nbChunks + 1 > msgBuffer->maxChunks){
                    if(scConnection->transportConnection->serverSideConnection == FALSE){
                        status = OpcUa_BadRequestTooLarge;
                    }else{
                        status = OpcUa_BadResponseTooLarge;
                    }
                    SOPC_String_Initialize(&reason);
                    SC_AbortMsg(msgBuffer, status, &reason);
                }else{
                    // Fill buffer with maximum amount of bytes
                    uint32_t tmpCount = // Maximum Count - Precedent Count => Count to write
                     (msgBuffer->sequenceNumberPosition + UA_SECURE_MESSAGE_SEQUENCE_LENGTH +
                      scConnection->sendingMaxBodySize) - msgBuffer->buffers->position;
                    status = Buffer_Write(msgBuffer->buffers, data_src, tmpCount);

                    // Flush it !
                    if(status == STATUS_OK){
                        // Update count and pointer to data to write
                        count = count - tmpCount;
                        data_src = data_src + tmpCount;

                        status = SC_FlushSecureMsgBuffer(msgBuffer,
                                                         SOPC_Msg_Chunk_Intermediate);
                    }

                    if(status != STATUS_OK){
                        // TODO: Should send flush error in case of too many chunks ?
                        SOPC_String reason;
                        SOPC_String_Initialize(&reason);
                        SOPC_String_AttachFromCstring(&reason, "Error encoding intermediate chunk");
                        SC_AbortMsg(msgBuffer, OpcUa_BadEncodingError, &reason);
                        SOPC_String_Clear(&reason);
                    }
                }
            } // While not fitting in current buffer, fill and flush current buffer

            if(status == STATUS_OK){
                status = Buffer_Write(msgBuffer->buffers, data_src, count);
            }
        }
    }else{
        // Use the simple TCP UA write, no chunk management needed
        status = TCP_UA_WriteMsgBuffer(msgBuffer, data_src, count);
    }
    return status;
}
