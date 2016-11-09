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

#include "sopc_msg_buffer.h"

#include <assert.h>
#include <stdlib.h>

const SOPC_Byte HEL[3] = {'H','E','L'};
const SOPC_Byte ACK[3] = {'A','C','K'};
const SOPC_Byte ERR[3] = {'E','R','R'};
const SOPC_Byte MSG[3] = {'M','S','G'};
const SOPC_Byte OPN[3] = {'O','P','N'};
const SOPC_Byte CLO[3] = {'C','L','O'};

SOPC_MsgBuffer* MsgBuffer_Create(Buffer*             buffer,
                               uint32_t            maxChunks,
                               void*               flushData,
                               SOPC_NamespaceTable*  nsTable,
                               SOPC_EncodeableType** encTypesTable)
{
    SOPC_MsgBuffer* mBuffer = NULL;
    if(buffer != NULL){
        mBuffer = (SOPC_MsgBuffer*) malloc(sizeof(SOPC_MsgBuffer));
        mBuffer->nbBuffers = 1;
        mBuffer->buffers = buffer;
        mBuffer->type = TCP_SOPC_Message_Unknown;
        mBuffer->secureType = SOPC_SecureMessage;
        mBuffer->currentChunkSize = 0;
        mBuffer->nbChunks = 1;
        mBuffer->maxChunks = maxChunks;
        mBuffer->sequenceNumberPosition = 0;
        mBuffer->isFinal = SOPC_Msg_Chunk_Unknown;
        mBuffer->receivedReqId = 0;
        mBuffer->flushData = flushData;
        Namespace_Initialize(&mBuffer->nsTable);
        Namespace_AttachTable(&mBuffer->nsTable, nsTable);
        mBuffer->encTypesTable = encTypesTable;
    }
    return mBuffer;
}

void MsgBuffer_Delete(SOPC_MsgBuffer** ptBuffer){
    if(ptBuffer != NULL && *ptBuffer != NULL){
        assert((*ptBuffer)->nbBuffers == 1);
        if((*ptBuffer)->buffers != NULL){
            Buffer_Delete((*ptBuffer)->buffers);
        }
        free(*ptBuffer);
        *ptBuffer = NULL;
    }
}

void MsgBuffer_Reset(SOPC_MsgBuffer* mBuffer){
    if(mBuffer != NULL){
        assert(mBuffer->nbBuffers == 1);
        Buffer_Reset(mBuffer->buffers);
        mBuffer->type = TCP_SOPC_Message_Unknown;
        mBuffer->secureType = SOPC_SecureMessage;
        mBuffer->currentChunkSize = 0;
        mBuffer->nbChunks = 1;
        mBuffer->isFinal = SOPC_Msg_Chunk_Unknown;
        mBuffer->receivedReqId = 0;
        mBuffer->sequenceNumberPosition = 0;
    }
}

SOPC_StatusCode MsgBuffer_ResetNextChunk(SOPC_MsgBuffer* mBuffer,
                                    uint32_t      bodyPosition){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(mBuffer != NULL){
        assert(mBuffer->nbBuffers == 1);
        mBuffer->currentChunkSize = bodyPosition;
        status = Buffer_ResetAfterPosition(mBuffer->buffers, bodyPosition);
        if(status == STATUS_OK &&
           (mBuffer->maxChunks == 0
            || mBuffer->nbChunks < mBuffer->maxChunks))
        {
            mBuffer->nbChunks = mBuffer->nbChunks + 1;
        }else{
            status = STATUS_INVALID_STATE;
        }
    }
    return status;
}


SOPC_StatusCode MsgBuffer_SetSecureMsgType(SOPC_MsgBuffer*        mBuffer,
                                      SOPC_SecureMessageType sType){
    SOPC_StatusCode status = STATUS_INVALID_STATE;
    if(mBuffer != NULL &&
        (mBuffer->type == TCP_SOPC_Message_Unknown || mBuffer->type == TCP_SOPC_Message_SecureMessage)){
        assert(mBuffer->nbBuffers == 1);
        mBuffer->secureType = sType;
        mBuffer->type = TCP_SOPC_Message_SecureMessage;
        status = STATUS_OK;
    }
    return status;
}

void MsgBuffer_InternalCopyProperties(SOPC_MsgBuffer* destMsgBuffer,
                                      SOPC_MsgBuffer* srcMsgBuffer){
    assert(destMsgBuffer != NULL && srcMsgBuffer != NULL);
    // Copy all internal properties except the buffers and number of buffers
    destMsgBuffer->type = srcMsgBuffer->type;
    destMsgBuffer->secureType = srcMsgBuffer->secureType;
    destMsgBuffer->currentChunkSize = srcMsgBuffer->currentChunkSize;
    destMsgBuffer->nbChunks = srcMsgBuffer->nbChunks;
    destMsgBuffer->sequenceNumberPosition = srcMsgBuffer->sequenceNumberPosition;
    destMsgBuffer->isFinal = srcMsgBuffer->isFinal;
    destMsgBuffer->receivedReqId = srcMsgBuffer->receivedReqId;
}

SOPC_StatusCode MsgBuffer_CopyBuffer(SOPC_MsgBuffer* destMsgBuffer,
                                SOPC_MsgBuffer* srcMsgBuffer){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(destMsgBuffer != NULL && srcMsgBuffer != NULL){
        assert(destMsgBuffer->nbBuffers == 1);
        assert(srcMsgBuffer->nbBuffers == 1);
        status = STATUS_OK;
        // Copy everything except the flush data which can be used for a different flush level
        //  and nbBuffers which is set on initialization
        MsgBuffer_InternalCopyProperties(destMsgBuffer, srcMsgBuffer);

        status = Buffer_Copy(destMsgBuffer->buffers, srcMsgBuffer->buffers);
    }
    return status;
}

SOPC_MsgBuffers* MsgBuffers_Create(uint32_t            maxChunks,
                                 uint32_t            bufferSize,
                                 SOPC_NamespaceTable*  nsTable,
                                 SOPC_EncodeableType** encTypesTable)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SOPC_MsgBuffers* mBuffers = NULL;
    uint32_t idx = 0;
    if(maxChunks > 0 && bufferSize > 0 &&
       nsTable != NULL && encTypesTable != NULL)
    {
        status = STATUS_NOK;
        mBuffers = (SOPC_MsgBuffer*) malloc(sizeof(SOPC_MsgBuffer));
        if(mBuffers != NULL){
            status = STATUS_OK;
            mBuffers->nbBuffers = maxChunks;
            mBuffers->buffers = (Buffer*) malloc(sizeof(Buffer) * maxChunks);

            if(mBuffers->buffers != NULL){
                while(status == STATUS_OK && idx < maxChunks){
                    status = Buffer_Init(&(mBuffers->buffers[idx]), bufferSize);
                    idx++;
                }

                if(status != STATUS_OK){
                    // To could deallocate correctly
                    mBuffers->nbBuffers = idx + 1;
                }

            }else{
               status = STATUS_NOK;
               mBuffers->nbBuffers = 0;
            }

            if(status == STATUS_OK){
                mBuffers->type = TCP_SOPC_Message_Unknown;
                mBuffers->secureType = SOPC_SecureMessage;
                mBuffers->currentChunkSize = 0;
                mBuffers->nbChunks = 0;
                mBuffers->maxChunks = maxChunks;
                mBuffers->sequenceNumberPosition = 0;
                mBuffers->isFinal = SOPC_Msg_Chunk_Unknown;
                mBuffers->receivedReqId = 0;
                mBuffers->flushData = NULL;
                Namespace_Initialize(&mBuffers->nsTable);
                Namespace_AttachTable(&mBuffers->nsTable, nsTable);
                mBuffers->encTypesTable = encTypesTable;
            }else{
                MsgBuffers_Delete(&mBuffers);
            }
        }
    }
    return mBuffers;
}

void MsgBuffers_Reset(SOPC_MsgBuffers* mBuffer){
    uint32_t idx = 0;
    if(mBuffer != NULL){
        for(idx = 0; idx < mBuffer->nbChunks; idx++){
            Buffer_Reset(&(mBuffer->buffers[idx]));
        }
        mBuffer->type = TCP_SOPC_Message_Unknown;
        mBuffer->secureType = SOPC_SecureMessage;
        mBuffer->currentChunkSize = 0;
        mBuffer->nbChunks = 0;
        mBuffer->isFinal = SOPC_Msg_Chunk_Unknown;
        mBuffer->receivedReqId = 0;
        mBuffer->sequenceNumberPosition = 0;
    }
}

void MsgBuffers_Delete(SOPC_MsgBuffers** ptBuffers){
    uint32_t idx = 0;
    SOPC_MsgBuffers* msgBuffers = *ptBuffers;
    if(ptBuffers != NULL && msgBuffers != NULL){
        if(msgBuffers->buffers != NULL){
            for(idx = 0; idx < msgBuffers->nbBuffers; idx++){
                Buffer_Clear (&(msgBuffers->buffers[idx]));
            }
            free(msgBuffers->buffers);
        }
        free(*ptBuffers);
        *ptBuffers = NULL;
    }
}

Buffer* MsgBuffers_GetCurrentChunk(SOPC_MsgBuffers* mBuffer){
    Buffer* buf = NULL;
    if(mBuffer != NULL){
        if(mBuffer->nbChunks > 0){
            buf = &(mBuffer->buffers[mBuffer->nbChunks - 1]);
        }
    }
    return buf;
}

Buffer* MsgBuffers_NextChunk(SOPC_MsgBuffers* mBuffer,
                             uint32_t*      bufferIdx){
    Buffer* buf = NULL;
    if(mBuffer != NULL && bufferIdx != NULL){
        if(mBuffer->nbChunks < mBuffer->maxChunks){
            *bufferIdx = mBuffer->nbChunks;
            buf = &(mBuffer->buffers[mBuffer->nbChunks]);
            mBuffer->nbChunks = mBuffer->nbChunks + 1;
        }
    }
    return buf;
}

SOPC_StatusCode MsgBuffers_SetCurrentChunkFirst(SOPC_MsgBuffers* mBuffer){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    uint32_t idx = 0;
    Buffer* buffer = NULL;
    if(mBuffer != NULL && mBuffer->nbChunks > 1){
        // Keep current buffer reference
        buffer = MsgBuffers_GetCurrentChunk(mBuffer);
        if(buffer != NULL){
            status = Buffer_Copy(&mBuffer->buffers[0], buffer);
            for(idx = 1; idx < mBuffer->nbChunks; idx++){
                Buffer_Reset(&mBuffer->buffers[idx]);
            }
            mBuffer->nbChunks = 1;
        }
    }
    return status;
}

void MsgBuffers_InternalCopyProperties(SOPC_MsgBuffers* destMsgBuffer,
                                       SOPC_MsgBuffer*  srcMsgBuffer){
    MsgBuffer_InternalCopyProperties((SOPC_MsgBuffer*) destMsgBuffer, srcMsgBuffer);
}

SOPC_StatusCode MsgBuffers_CopyBuffer(SOPC_MsgBuffers* destMsgBuffer,
                                 uint32_t       bufferIdx,
                                 SOPC_MsgBuffer*  srcMsgBuffer,
                                 uint32_t       limitedLength){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(destMsgBuffer != NULL && srcMsgBuffer != NULL &&
       bufferIdx < destMsgBuffer->maxChunks &&
       srcMsgBuffer->buffers->length >= limitedLength &&
       destMsgBuffer->buffers[bufferIdx].max_size >= limitedLength){
        assert(srcMsgBuffer->nbBuffers == 1);
        status = STATUS_OK;
        // Copy everything except the flush data which can be used for a different flush level
        //  and nbBuffers which is set on initialization
        MsgBuffer_InternalCopyProperties(destMsgBuffer, srcMsgBuffer);

        status = Buffer_CopyWithLength(&(destMsgBuffer->buffers[bufferIdx]),
                                       srcMsgBuffer->buffers,
                                       limitedLength);
    }
    return status;
}

