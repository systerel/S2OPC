/*
 * msg_buffer.c
 *
 *  Created on: 29 juil. 2016
 *      Author: Vincent
 */

#include <assert.h>
#include <stdlib.h>
#include <msg_buffer.h>

const UA_Byte HEL[3] = {'H','E','L'};
const UA_Byte ACK[3] = {'A','C','K'};
const UA_Byte ERR[3] = {'E','R','R'};
const UA_Byte MSG[3] = {'M','S','G'};
const UA_Byte OPN[3] = {'O','P','N'};
const UA_Byte CLO[3] = {'C','L','O'};

UA_Msg_Buffer* Create_Msg_Buffer(Buffer*  buffer,
                                 uint32_t maxChunks,
                                 void*    flushData)
{
    UA_Msg_Buffer* mBuffer = UA_NULL;
    if(buffer != UA_NULL){
        mBuffer = (UA_Msg_Buffer*) malloc(sizeof(UA_Msg_Buffer));
        mBuffer->flushData = flushData;
        mBuffer->nbBuffers = 1;
        mBuffer->buffers = buffer;
        mBuffer->msgSize = 0;
        mBuffer->nbChunks = 1;
        mBuffer->type = TCP_UA_Message_Unknown;
        mBuffer->isFinal = UA_Msg_Chunk_Unknown;
        mBuffer->requestId = 0;
        mBuffer->secureType = UA_SecureMessage;
        mBuffer->maxChunks = maxChunks;
    }
    return mBuffer;
}

void Delete_Msg_Buffer(UA_Msg_Buffer** ptBuffer){
    if(ptBuffer != UA_NULL && *ptBuffer != UA_NULL){
        assert((*ptBuffer)->nbBuffers == 1);
        if((*ptBuffer)->buffers != UA_NULL){
            Delete_Buffer((*ptBuffer)->buffers);
        }
        free(*ptBuffer);
        *ptBuffer = UA_NULL;
    }
}

void Reset_Buffer_Properties(UA_Msg_Buffer* mBuffer){
    if(mBuffer != UA_NULL){
        assert(mBuffer->nbBuffers == 1);
        mBuffer->msgSize = 0;
        mBuffer->type = TCP_UA_Message_Unknown;
        mBuffer->isFinal = UA_Msg_Chunk_Unknown;
        mBuffer->requestId = 0;
        mBuffer->secureType = UA_SecureMessage;
        Reset_Buffer(mBuffer->buffers);
    }
}

void Reset_Msg_Buffer(UA_Msg_Buffer* mBuffer){
    if(mBuffer != UA_NULL){
        assert(mBuffer->nbBuffers == 1);
        Reset_Buffer_Properties(mBuffer);
        mBuffer->nbChunks = 1;
    }
}

StatusCode Reset_Msg_Buffer_Next_Chunk(UA_Msg_Buffer* mBuffer,
                                       uint32_t       bodyPosition){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(mBuffer != UA_NULL){
        assert(mBuffer->nbBuffers == 1);
        mBuffer->msgSize = bodyPosition;
        Reset_Buffer_After_Position(mBuffer->buffers, bodyPosition);
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


StatusCode Set_Secure_Message_Type(UA_Msg_Buffer* mBuffer,
                                   UA_Secure_Message_Type sType){
    StatusCode status = STATUS_INVALID_STATE;
    if(mBuffer != UA_NULL && mBuffer->type == TCP_UA_Message_Unknown){
        assert(mBuffer->nbBuffers == 1);
        mBuffer->secureType = sType;
        status = STATUS_OK;
    }
    return status;
}

StatusCode Attach_Buffer_To_Msg_Buffer(UA_Msg_Buffer* destMsgBuffer,
                                       UA_Msg_Buffer* srcMsgBuffer){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(destMsgBuffer != UA_NULL && srcMsgBuffer != UA_NULL){
        assert(destMsgBuffer->nbBuffers == 1);
        assert(srcMsgBuffer->nbBuffers == 1);
        status = STATUS_OK;
        Delete_Buffer(destMsgBuffer->buffers);
        destMsgBuffer->nbChunks = srcMsgBuffer->nbChunks;
        destMsgBuffer->buffers = srcMsgBuffer->buffers;
        destMsgBuffer->type = srcMsgBuffer->type;
        destMsgBuffer->secureType = srcMsgBuffer->secureType;
        destMsgBuffer->msgSize = srcMsgBuffer->msgSize;
    }
    return status;
}

UA_Msg_Buffers* Create_Msg_Buffers(uint32_t maxChunks){
    assert(maxChunks > 0);
    UA_Msg_Buffers* mBuffers = UA_NULL;
    mBuffers = (UA_Msg_Buffer*) malloc(sizeof(UA_Msg_Buffer));
    mBuffers->flushData = UA_NULL;
    mBuffers->nbBuffers = maxChunks;
    mBuffers->buffers = (Buffer*) malloc(sizeof(Buffer) * maxChunks);
    mBuffers->msgSize = 0;
    mBuffers->nbChunks = 1;
    mBuffers->type = TCP_UA_Message_Unknown;
    mBuffers->isFinal = UA_Msg_Chunk_Unknown;
    mBuffers->secureType = UA_SecureMessage;
    mBuffers->maxChunks = maxChunks;
    return mBuffers;
}

void Delete_Msg_Buffers(UA_Msg_Buffer** ptBuffers){
    uint32_t idx = 0;
    if(ptBuffers != UA_NULL && *ptBuffers != UA_NULL){
        if((*ptBuffers)->buffers != UA_NULL){
            for(idx = 0; idx < (*ptBuffers)->nbBuffers; idx++){
                Delete_Buffer (&((*ptBuffers)->buffers[idx]));
            }
        }
        free(*ptBuffers);
        *ptBuffers = UA_NULL;
    }
}

