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

#include "sopc_tcp_ua_low_level.h"

#include <assert.h>
#include <string.h>

#include "opcua_statuscodes.h"
#include "sopc_encoder.h"

const uint32_t tcpProtocolVersion = 0;

SOPC_StatusCode TCP_UA_EncodeHeader(SOPC_MsgBuffer* msgBuffer,
                                    TCP_UA_MsgType  type){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SOPC_Byte fByte = 'F';

    if(msgBuffer != NULL){
        status = STATUS_OK;
    }

    if(STATUS_OK == status){
        assert(msgBuffer->buffers->max_size > TCP_UA_HEADER_LENGTH);
        switch(type){
            case TCP_UA_Message_Hello:
                status = Buffer_Write(msgBuffer->buffers, SOPC_HEL, 3);
                break;
            case TCP_UA_Message_Acknowledge:
                status = Buffer_Write(msgBuffer->buffers, SOPC_ACK, 3);
                break;
            case TCP_UA_Message_Error:
                status = Buffer_Write(msgBuffer->buffers, SOPC_ERR, 3);
                break;
            case TCP_UA_Message_SecureMessage:
                // Managed by secure channel layer
                break;
            default:
                // Error case (Invalid or Unknown)
                status = STATUS_NOK;
        }
    }
    if(status == STATUS_OK){
        // reserved byte
        status = TCP_UA_WriteMsgBuffer(msgBuffer, &fByte, 1);
    }
    if(status == STATUS_OK){
        const uint32_t headerLength = TCP_UA_HEADER_LENGTH;
        status = SOPC_UInt32_Write(&headerLength, msgBuffer);
        if(status == STATUS_OK){
            msgBuffer->type = type;
            msgBuffer->currentChunkSize = TCP_UA_HEADER_LENGTH;
            msgBuffer->nbChunks = 1;
        }
    }
    return status;
}

SOPC_StatusCode TCP_UA_FinalizeHeader(SOPC_MsgBuffer* msgBuffer){
    assert(msgBuffer->type == TCP_UA_Message_Hello
           || msgBuffer->type == TCP_UA_Message_Acknowledge
           || msgBuffer->type == TCP_UA_Message_Error);
    SOPC_StatusCode status = STATUS_NOK;
    const uint32_t currentPosition = msgBuffer->buffers->position;

    status = Buffer_SetPosition(msgBuffer->buffers, UA_HEADER_LENGTH_POSITION);

    if(status == STATUS_OK){
        status = SOPC_UInt32_Write(&currentPosition, msgBuffer);
    }
    if(status == STATUS_OK){
        status = Buffer_SetPosition(msgBuffer->buffers, currentPosition);
        msgBuffer->currentChunkSize = currentPosition;
    }
    return status;
}


SOPC_StatusCode TCP_UA_ReadData(SOPC_Socket*    socket,
                                SOPC_MsgBuffer* msgBuffer){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    int32_t readBytes;

    if(socket != NULL && socket->sock != SOPC_INVALID_SOCKET && msgBuffer != NULL){
        status = STATUS_NOK;
        if(msgBuffer->buffers->length >= TCP_UA_HEADER_LENGTH){
            assert(msgBuffer->currentChunkSize > 0);

            if(msgBuffer->buffers->length < msgBuffer->currentChunkSize){
                //incomplete message, continue to read it
                status = STATUS_OK;
            }else if(msgBuffer->buffers->length == msgBuffer->currentChunkSize){
                if(msgBuffer->isFinal == SOPC_Msg_Chunk_Intermediate){
                    status = MsgBuffer_ResetNextChunk(msgBuffer, 0);
                }else{
                    MsgBuffer_Reset(msgBuffer);
                    // status will be set reading the header
                }
            }else{
                // constraint error: header length <= buffer length <= message size
                status = STATUS_INVALID_PARAMETERS;
            }
        }

        if(msgBuffer->buffers->length < TCP_UA_HEADER_LENGTH){

            // Attempt to read header
            if(msgBuffer->buffers->max_size > TCP_UA_HEADER_LENGTH){
                readBytes = TCP_UA_HEADER_LENGTH - msgBuffer->buffers->length;
                status = SOPC_Socket_Read(socket, msgBuffer->buffers->data, readBytes, &readBytes);
                if(status == STATUS_OK && readBytes > 0){
                    Buffer_SetDataLength(msgBuffer->buffers, msgBuffer->buffers->length + readBytes);

                    if(msgBuffer->buffers->length == TCP_UA_HEADER_LENGTH){
                        status = TCP_UA_ReadHeader(msgBuffer);
                    }else if(msgBuffer->buffers->length > TCP_UA_HEADER_LENGTH){
                        status = STATUS_INVALID_STATE;
                    }else{
                        // Incomplete header: Wait for new read event !
                        status = STATUS_OK_INCOMPLETE;
                    }
                }else{
                    // TODO: manage other statuses disconnect, etc.
                }
            }
        }
    }

    if(status == STATUS_OK){
        if(msgBuffer->buffers->max_size >= msgBuffer->currentChunkSize){

            readBytes = msgBuffer->currentChunkSize - msgBuffer->buffers->length;
            status = SOPC_Socket_Read(socket,
                                    &(msgBuffer->buffers->data[msgBuffer->buffers->length]),
                                    readBytes,
                                    &readBytes);
            if(status == STATUS_OK && readBytes > 0){
                Buffer_SetDataLength(msgBuffer->buffers, msgBuffer->buffers->length + readBytes);
            }
            if(msgBuffer->buffers->length == msgBuffer->currentChunkSize){
                // Message complete just return
                assert(status == STATUS_OK);
            }else if(msgBuffer->buffers->length == msgBuffer->currentChunkSize){
                status = STATUS_INVALID_STATE;
            }else{
                status = STATUS_OK_INCOMPLETE;
            }

        }else{
            status = OpcUa_BadTcpMessageTooLarge;
        }
    }

    return status;
}

SOPC_StatusCode TCP_UA_ReadHeader(SOPC_MsgBuffer* msgBuffer){
    SOPC_StatusCode status = STATUS_OK;
    SOPC_Byte msgType[3];
    SOPC_Byte isFinal;

    if(msgBuffer != NULL
       && msgBuffer->buffers->length == TCP_UA_HEADER_LENGTH
       && msgBuffer->type == TCP_UA_Message_Unknown)
    {
        // READ message type
        status = Buffer_Read(msgType, msgBuffer->buffers, 3);
        if(status == STATUS_OK){
            if(memcmp(msgType, SOPC_HEL, 3) == 0){
                msgBuffer->type = TCP_UA_Message_Hello;
            }else if(memcmp(msgType, SOPC_ACK, 3) == 0){
                msgBuffer->type = TCP_UA_Message_Acknowledge;
            }else if(memcmp(msgType, SOPC_ERR, 3) == 0){
                msgBuffer->type = TCP_UA_Message_Error;
            }

            if(msgBuffer->type == TCP_UA_Message_Unknown){
                // should be a secure message
                if(memcmp(msgType, SOPC_MSG, 3) == 0){
                    msgBuffer->type = TCP_UA_Message_SecureMessage;
                    msgBuffer->secureType = SOPC_SecureMessage;
                }else if(memcmp(msgType, SOPC_OPN, 3) == 0){
                    msgBuffer->type = TCP_UA_Message_SecureMessage;
                    msgBuffer->secureType = SOPC_OpenSecureChannel;
                }else if(memcmp(msgType, SOPC_CLO, 3) == 0){
                    msgBuffer->type = TCP_UA_Message_SecureMessage;
                    msgBuffer->secureType = SOPC_CloseSecureChannel;
                }else{
                    msgBuffer->type = TCP_UA_Message_Invalid;
                    status = OpcUa_BadTcpMessageTypeInvalid;
                }
            }

            assert(msgBuffer->type != TCP_UA_Message_Unknown);
            assert(msgBuffer->type != TCP_UA_Message_Invalid || status != STATUS_OK);
        }

        // READ IsFinal message chunk
        if(status == STATUS_OK)
        {
            status = Buffer_Read(&isFinal, msgBuffer->buffers, 1);
            if(status == STATUS_OK){
                switch(isFinal){
                    case 'C':
                        msgBuffer->isFinal = SOPC_Msg_Chunk_Intermediate;
                        break;
                    case 'F':
                        msgBuffer->isFinal = SOPC_Msg_Chunk_Final;
                        break;
                    case 'A':
                        msgBuffer->isFinal = SOPC_Msg_Chunk_Abort;
                        break;
                    default:
                        msgBuffer->isFinal = SOPC_Msg_Chunk_Invalid;
                        status = OpcUa_BadTcpMessageTypeInvalid;
                        break;
                }

                //In TCP UA non secure messages reserved byte shall be set to 'F'
                if(msgBuffer->type != TCP_UA_Message_SecureMessage
                   || msgBuffer->secureType == SOPC_OpenSecureChannel // As indicated by mantis #3378
                   || msgBuffer->secureType == SOPC_CloseSecureChannel ) // As indicated by mantis #3378
                {
                    if(msgBuffer->isFinal != SOPC_Msg_Chunk_Final){
                        status = OpcUa_BadTcpMessageTypeInvalid;
                    }
                }
            } // read isFinal chunk
        }

        // READ message size
        if(status == STATUS_OK){
            status = SOPC_UInt32_Read(&msgBuffer->currentChunkSize, msgBuffer);
        }

    }else{
        status = STATUS_INVALID_PARAMETERS;
    }
    return status;
}

SOPC_StatusCode TCP_UA_ReadMsgBuffer(SOPC_Byte*      data_dest,
                                     uint32_t        size,
                                     SOPC_MsgBuffer* msgBuffer,
                                     uint32_t        count){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    Buffer* buffer = NULL;
    if(data_dest != NULL && size > 0 && msgBuffer != NULL && count > 0){
        // Treat single buffer messages and multiple buffers message (SOPC_MsgBuffers)
        if(msgBuffer->nbBuffers == 1){
            buffer = msgBuffer->buffers;
        }else if(msgBuffer->nbBuffers > 1 && msgBuffer->nbChunks > 0){
            // TCP UA layer never treat chunks
            assert(msgBuffer->nbChunks == 1);
            buffer = MsgBuffers_GetCurrentChunk(msgBuffer);
            if(buffer == NULL){
                status = STATUS_NOK;
            }
        }

        // Check for size and count values
        if(data_dest == NULL || msgBuffer == NULL
           || size < count
           || buffer->length - buffer->position < count)
        {
            status = STATUS_INVALID_PARAMETERS;
        }else{
            status = Buffer_Read(data_dest, buffer, count);
        }
    }

    return status;
}

SOPC_StatusCode TCP_UA_FlushMsgBuffer(SOPC_MsgBuffer*               msgBuffer,
                                      SOPC_Socket_Transaction_Event transactionEvent,
                                      uint32_t                      transactionId,
                                      SOPC_Socket_EndOperation_CB*  callback,
                                      void*                         callbackData){
    return SOPC_CreateAction_SocketWrite((SOPC_Socket*) msgBuffer->flushData,
                                         msgBuffer->buffers->data,
                                         msgBuffer->buffers->length,
                                         transactionEvent,
                                         transactionId,
                                         callback,
                                         callbackData);
}


SOPC_StatusCode TCP_UA_WriteMsgBuffer(SOPC_MsgBuffer*  msgBuffer,
                                      const SOPC_Byte* data_src,
                                      uint32_t         count)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(data_src != NULL && msgBuffer != NULL && count > 0)
    {
        if(msgBuffer->buffers->position + count > msgBuffer->buffers->max_size){
            // Error message should be managed at secure channel level:
            //  no possible message chunks except for MSG type !
            status = STATUS_INVALID_STATE;
        }else{
            status = Buffer_Write(msgBuffer->buffers, data_src, count);
        }
    }
    return status;
}

