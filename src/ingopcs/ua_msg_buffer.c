/*
 * msg_buffer.c
 *
 *  Created on: 29 juil. 2016
 *      Author: Vincent
 */

#include "ua_msg_buffer.h"

#include <assert.h>
#include <stdlib.h>

const UA_Byte HEL[3] = {'H','E','L'};
const UA_Byte ACK[3] = {'A','C','K'};
const UA_Byte ERR[3] = {'E','R','R'};
const UA_Byte MSG[3] = {'M','S','G'};
const UA_Byte OPN[3] = {'O','P','N'};
const UA_Byte CLO[3] = {'C','L','O'};

UA_MsgBuffer* MsgBuffer_Create(Buffer*             buffer,
                               uint32_t            maxChunks,
                               void*               flushData,
                               UA_NamespaceTable*  nsTable,
                               UA_EncodeableType** encTypesTable)
{
    UA_MsgBuffer* mBuffer = UA_NULL;
    if(buffer != UA_NULL){
        mBuffer = (UA_MsgBuffer*) malloc(sizeof(UA_MsgBuffer));
        mBuffer->nbBuffers = 1;
        mBuffer->buffers = buffer;
        mBuffer->type = TCP_UA_Message_Unknown;
        mBuffer->secureType = UA_SecureMessage;
        mBuffer->msgSize = 0;
        mBuffer->nbChunks = 1;
        mBuffer->maxChunks = maxChunks;
        mBuffer->sequenceNumberPosition = 0;
        mBuffer->isFinal = UA_Msg_Chunk_Unknown;
        mBuffer->receivedReqId = 0;
        mBuffer->flushData = flushData;
        mBuffer->nsTable = nsTable;
        mBuffer->encTypesTable = encTypesTable;
    }
    return mBuffer;
}

void MsgBuffer_Delete(UA_MsgBuffer** ptBuffer){
    if(ptBuffer != UA_NULL && *ptBuffer != UA_NULL){
        assert((*ptBuffer)->nbBuffers == 1);
        if((*ptBuffer)->buffers != UA_NULL){
            Buffer_Delete((*ptBuffer)->buffers);
        }
        free(*ptBuffer);
        *ptBuffer = UA_NULL;
    }
}

void MsgBuffer_Reset(UA_MsgBuffer* mBuffer){
    if(mBuffer != UA_NULL){
        assert(mBuffer->nbBuffers == 1);
        Buffer_Reset(mBuffer->buffers);
        mBuffer->type = TCP_UA_Message_Unknown;
        mBuffer->secureType = UA_SecureMessage;
        mBuffer->msgSize = 0;
        mBuffer->nbChunks = 1;
        mBuffer->isFinal = UA_Msg_Chunk_Unknown;
        mBuffer->receivedReqId = 0;
        mBuffer->sequenceNumberPosition = 0;
    }
}

StatusCode MsgBuffer_ResetNextChunk(UA_MsgBuffer* mBuffer,
                                    uint32_t      bodyPosition){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(mBuffer != UA_NULL){
        assert(mBuffer->nbBuffers == 1);
        mBuffer->msgSize = bodyPosition;
        Buffer_ResetAfterPosition(mBuffer->buffers, bodyPosition);
        if(mBuffer->maxChunks == 0
           || mBuffer->nbChunks < mBuffer->maxChunks)
        {
            mBuffer->nbChunks = mBuffer->nbChunks + 1;
        }else{
            status = STATUS_INVALID_STATE;
        }
    }
    return status;
}


StatusCode MsgBuffer_SetSecureMsgType(UA_MsgBuffer*        mBuffer,
                                      UA_SecureMessageType sType){
    StatusCode status = STATUS_INVALID_STATE;
    if(mBuffer != UA_NULL && mBuffer->type == TCP_UA_Message_Unknown){
        assert(mBuffer->nbBuffers == 1);
        mBuffer->secureType = sType;
        status = STATUS_OK;
    }
    return status;
}

void MsgBuffer_InternalCopyProperties(UA_MsgBuffer* destMsgBuffer,
                                      UA_MsgBuffer* srcMsgBuffer){
    assert(destMsgBuffer != UA_NULL && srcMsgBuffer != UA_NULL);
    // Copy all internal properties except the buffers and number of buffers
    destMsgBuffer->type = srcMsgBuffer->type;
    destMsgBuffer->secureType = srcMsgBuffer->secureType;
    destMsgBuffer->msgSize = srcMsgBuffer->msgSize;
    destMsgBuffer->nbChunks = srcMsgBuffer->nbChunks;
    destMsgBuffer->sequenceNumberPosition = srcMsgBuffer->sequenceNumberPosition;
    destMsgBuffer->isFinal = srcMsgBuffer->isFinal;
    destMsgBuffer->receivedReqId = srcMsgBuffer->receivedReqId;
}

StatusCode MsgBuffer_CopyBuffer(UA_MsgBuffer* destMsgBuffer,
                                UA_MsgBuffer* srcMsgBuffer){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(destMsgBuffer != UA_NULL && srcMsgBuffer != UA_NULL){
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

UA_MsgBuffers* MsgBuffers_Create(uint32_t            maxChunks,
                                 uint32_t            bufferSize,
                                 UA_NamespaceTable*  nsTable,
                                 UA_EncodeableType** encTypesTable)
{
    assert(maxChunks > 0);
    StatusCode status = STATUS_OK;
    UA_MsgBuffers* mBuffers = UA_NULL;
    uint32_t idx = 0;
    mBuffers = (UA_MsgBuffer*) malloc(sizeof(UA_MsgBuffer));
    if(mBuffers != UA_NULL){
        mBuffers->nbBuffers = maxChunks;
        mBuffers->buffers = (Buffer*) malloc(sizeof(Buffer) * maxChunks);

        if(mBuffers->buffers != UA_NULL){
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
            mBuffers->type = TCP_UA_Message_Unknown;
            mBuffers->secureType = UA_SecureMessage;
            mBuffers->msgSize = 0;
            mBuffers->nbChunks = 0;
            mBuffers->maxChunks = maxChunks;
            mBuffers->sequenceNumberPosition = 0;
            mBuffers->isFinal = UA_Msg_Chunk_Unknown;
            mBuffers->receivedReqId = 0;
            mBuffers->flushData = UA_NULL;
            mBuffers->nsTable = nsTable;
            mBuffers->encTypesTable = encTypesTable;
        }else{
            MsgBuffers_Delete(&mBuffers);
        }
    }
    return mBuffers;
}

void MsgBuffers_Reset(UA_MsgBuffers* mBuffer){
    uint32_t idx = 0;
    if(mBuffer != UA_NULL){
        for(idx = 0; idx < mBuffer->nbChunks; idx++){
            Buffer_Reset(&(mBuffer->buffers[idx]));
        }
        mBuffer->type = TCP_UA_Message_Unknown;
        mBuffer->secureType = UA_SecureMessage;
        mBuffer->msgSize = 0;
        mBuffer->nbChunks = 0;
        mBuffer->isFinal = UA_Msg_Chunk_Unknown;
        mBuffer->receivedReqId = 0;
        mBuffer->sequenceNumberPosition = 0;
    }
}

void MsgBuffers_Delete(UA_MsgBuffers** ptBuffers){
    uint32_t idx = 0;
    if(ptBuffers != UA_NULL && *ptBuffers != UA_NULL){
        if((*ptBuffers)->buffers != UA_NULL){
            for(idx = 0; idx < (*ptBuffers)->nbBuffers; idx++){
                Buffer_Delete (&((*ptBuffers)->buffers[idx]));
            }
        }
        free(*ptBuffers);
        *ptBuffers = UA_NULL;
    }
}

Buffer* MsgBuffers_GetCurrentChunk(UA_MsgBuffers* mBuffer){
    Buffer* buf = UA_NULL;
    if(mBuffer != UA_NULL){
        if(mBuffer->nbChunks > 0){
            buf = &(mBuffer->buffers[mBuffer->nbChunks - 1]);
        }
    }
    return buf;
}

Buffer* MsgBuffers_NextChunk(UA_MsgBuffers* mBuffer,
                             uint32_t*      bufferIdx){
    Buffer* buf = UA_NULL;
    if(mBuffer != UA_NULL && bufferIdx != UA_NULL){
        if(mBuffer->nbChunks < mBuffer->maxChunks){
            *bufferIdx = mBuffer->nbChunks;
            buf = &(mBuffer->buffers[mBuffer->nbChunks]);
            mBuffer->nbChunks = mBuffer->nbChunks + 1;
        }
    }
    return buf;
}

void MsgBuffers_InternalCopyProperties(UA_MsgBuffers* destMsgBuffer,
                                       UA_MsgBuffer*  srcMsgBuffer){
    MsgBuffer_InternalCopyProperties((UA_MsgBuffer*) destMsgBuffer, srcMsgBuffer);
}

StatusCode MsgBuffers_CopyBuffer(UA_MsgBuffers* destMsgBuffer,
                                 uint32_t       bufferIdx,
                                 UA_MsgBuffer*  srcMsgBuffer,
                                 uint32_t       limitedLength){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(destMsgBuffer != UA_NULL && srcMsgBuffer != UA_NULL){
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

