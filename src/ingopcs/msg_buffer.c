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
        //mBuffer->chunkSize = 0;
        mBuffer->msgSize = 0;
        mBuffer->nbChunks = 1;
        mBuffer->type = TCP_UA_Message_Unknown;
        mBuffer->isFinal = UA_Msg_Chunk_Unknown;
        mBuffer->secureType = UA_SecureMessage;
        mBuffer->maxChunks = maxChunks;
    }
    return mBuffer;
}

void Delete_Msg_Buffer(UA_Msg_Buffer* mBuffer){
    assert(mBuffer->nbBuffers == 1);
    if(mBuffer != UA_NULL){
        if(mBuffer->buffers != UA_NULL){
            Delete_Buffer (mBuffer->buffers);
        }
        free(mBuffer);
    }
}

void Reset_Buffer_Properties(UA_Msg_Buffer* mBuffer){
    assert(mBuffer->nbBuffers == 1);
    mBuffer->msgSize = 0;
    mBuffer->type = TCP_UA_Message_Unknown;
    mBuffer->isFinal = UA_Msg_Chunk_Unknown;
    mBuffer->secureType = UA_SecureMessage;
    Reset_Buffer(mBuffer->buffers);
}

void Reset_Msg_Buffer(UA_Msg_Buffer* mBuffer){
    assert(mBuffer->nbBuffers == 1);
    if(mBuffer != UA_NULL){
        Reset_Buffer_Properties(mBuffer);
        mBuffer->nbChunks = 1;
    }
}

StatusCode Reset_Msg_Buffer_Next_Chunk(UA_Msg_Buffer* mBuffer,
                                       uint32_t       bodyPosition){
    assert(mBuffer->nbBuffers == 1);
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(mBuffer != UA_NULL){
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
    if(mBuffer->type == TCP_UA_Message_Unknown){
        mBuffer->secureType = sType;
        status = STATUS_OK;
    }
    return status;
}

UA_Msg_Buffers* Create_Msg_Buffers(uint32_t maxChunks){
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

void Delete_Msg_Buffers(UA_Msg_Buffer* mBuffers){
    uint32_t idx = 0;
    if(mBuffers != UA_NULL){
        if(mBuffers->buffers != UA_NULL){
            for(idx = 0; idx < mBuffers->nbBuffers; idx++){
                Delete_Buffer (&(mBuffers->buffers[idx]));
            }
        }
        free(mBuffers);
    }
}

