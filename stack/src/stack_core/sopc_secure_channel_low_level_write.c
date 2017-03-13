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

//void SOPC_OperationEnd_WriteSecure_CB(void*           arg,
//                                      SOPC_StatusCode writeStatus){
//    SOPC_MsgBuffer* msgBuffers = (SOPC_MsgBuffer*) arg;
//    uint8_t willReleaseMsgQueueToken = FALSE;
//    if(STATUS_OK != writeStatus){
//        SOPC_String reason;
//        SOPC_String_Initialize(&reason);
//        SOPC_String_AttachFromCstring(&reason, "Error encoding intermediate chunk");
//        SC_AbortMsg(msgBuffers, msgBuffers->msgRequestId, OpcUa_BadEncodingError, &reason, &willReleaseMsgQueueToken);
//        SOPC_String_Clear(&reason);
//    }
//}

SOPC_StatusCode SC_WriteSecureMsgBuffer(SOPC_MsgBuffers*  msgBuffers,
                                        const SOPC_Byte*  data_src,
                                        uint32_t          count){
    SOPC_StatusCode status = STATUS_NOK;
    Buffer* buffer = NULL;
    SC_Connection* scConnection = NULL;

    if(msgBuffers == NULL){
        return STATUS_INVALID_PARAMETERS;
    }
    if(msgBuffers->type == TCP_UA_Message_SecureMessage){
        // Use chunks mechanism if it is a secure message
        if(data_src == NULL || msgBuffers == NULL || msgBuffers->flushData == NULL)
        {
            status = STATUS_INVALID_PARAMETERS;
        }else{
            status = STATUS_OK;
            scConnection = (SC_Connection*) msgBuffers->flushData;
            buffer = MsgBuffers_GetCurrentChunk(msgBuffers);
            assert(msgBuffers->sequenceNumberPosition +
                    UA_SECURE_MESSAGE_SEQUENCE_LENGTH +
                    scConnection->sendingMaxBodySize >= msgBuffers->buffers->position);

            while(STATUS_OK == status && // if not enough space in current buffer, create an intermediary chunk
                  (msgBuffers->buffers->position + count >
                   msgBuffers->sequenceNumberPosition +
                   UA_SECURE_MESSAGE_SEQUENCE_LENGTH +
                   scConnection->sendingMaxBodySize))
            {
                // Precedent position cannot be greater than message size:
                //  otherwise it means size has not been checked precedent time (it could occurs only when writing headers)
                assert(msgBuffers->sequenceNumberPosition +
                        UA_SECURE_MESSAGE_SEQUENCE_LENGTH +
                        scConnection->sendingMaxBodySize >= msgBuffers->buffers->position);
                if(msgBuffers->maxChunks != 0 && msgBuffers->nbChunks + 1 > msgBuffers->maxChunks){
                    if(scConnection->transportConnection->serverSideConnection == FALSE){
                        status = OpcUa_BadRequestTooLarge;
                    }else{
                        status = OpcUa_BadResponseTooLarge;
                    }
                }else{
                    // Fill buffer with maximum amount of bytes
                    uint32_t tmpCount = // Maximum Count - Precedent Count => Count to write
                     (msgBuffers->sequenceNumberPosition + UA_SECURE_MESSAGE_SEQUENCE_LENGTH +
                      scConnection->sendingMaxBodySize) - msgBuffers->buffers->position;
                    status = Buffer_Write(buffer, data_src, tmpCount);

                    // Continue in next buffer
                    if(STATUS_OK == status){
                        // Update count and pointer to data to write
                        count = count - tmpCount;
                        data_src = data_src + tmpCount;
                        // Set next chunk as current buffer
                        buffer = MsgBuffers_NextChunkWithHeadersCopy(msgBuffers,
                                                                     msgBuffers->sequenceNumberPosition + UA_SECURE_MESSAGE_SEQUENCE_LENGTH);
                    }
                }
            } // While not fitting in current buffer, fill and flush current buffer

            if(status == STATUS_OK){
                status = Buffer_Write(buffer, data_src, count);
            }
        }
    }else{
        // Use the simple TCP UA write, no chunk management needed
        status = TCP_UA_WriteMsgBuffer(msgBuffers->buffers, data_src, count);
    }
    return status;
}
