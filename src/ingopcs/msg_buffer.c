/*
 * msg_buffer.c
 *
 *  Created on: 29 juil. 2016
 *      Author: Vincent
 */

#include <stdlib.h>
#include <msg_buffer.h>

UA_Msg_Buffer* Create_Msg_Buffer(Buffer* buffer){
    UA_Msg_Buffer* mBuffer = UA_NULL;
    if(buffer != UA_NULL){
        mBuffer = (UA_Msg_Buffer*) malloc(sizeof(UA_Msg_Buffer));
        mBuffer->buffer = buffer;
        mBuffer->chunkSize = 0;
        mBuffer->length = 0;
        mBuffer->nbChunks = 0;
        mBuffer->type = OpcUa_TcpStream_MessageType_Unknown;
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
