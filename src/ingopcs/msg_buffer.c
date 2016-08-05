/*
 * msg_buffer.c
 *
 *  Created on: 29 juil. 2016
 *      Author: Vincent
 */

#include <stdlib.h>
#include <msg_buffer.h>

UA_Msg_Buffer* Create_Msg_Buffer(Socket   socket,
                                 Buffer*  buffer,
                                 uint32_t maxChunks){
    UA_Msg_Buffer* mBuffer = UA_NULL;
    if(buffer != UA_NULL){
        mBuffer = (UA_Msg_Buffer*) malloc(sizeof(UA_Msg_Buffer));
        mBuffer->socket = socket;
        mBuffer->buffer = buffer;
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
    if(mBuffer != UA_NULL){
        if(mBuffer->buffer != UA_NULL){
            Delete_Buffer (mBuffer->buffer);
        }
        free(mBuffer);
    }
}

void Reset_Buffer_Properties(UA_Msg_Buffer* mBuffer){
    mBuffer->msgSize = 0;
    mBuffer->type = TCP_UA_Message_Unknown;
    mBuffer->isFinal = UA_Msg_Chunk_Unknown;
    mBuffer->secureType = UA_SecureMessage;
    Reset_Buffer(mBuffer->buffer);
}

void Reset_Msg_Buffer(UA_Msg_Buffer* mBuffer){
    if(mBuffer != UA_NULL){
        Reset_Buffer_Properties(mBuffer);
        mBuffer->nbChunks = 1;
    }
}

StatusCode Reset_Msg_Buffer_Next_Chunk(UA_Msg_Buffer* mBuffer){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(mBuffer != UA_NULL){
        Reset_Buffer_Properties(mBuffer);
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
