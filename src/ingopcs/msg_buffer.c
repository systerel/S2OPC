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
        mBuffer->nbBuffers = 1;
        mBuffer->buffers = buffer;
        mBuffer->type = TCP_UA_Message_Unknown;
        mBuffer->secureType = UA_SecureMessage;
        mBuffer->msgSize = 0;
        mBuffer->nbChunks = 1;
        mBuffer->maxChunks = maxChunks;
        mBuffer->sequenceNumberPosition = 0;
        mBuffer->isFinal = UA_Msg_Chunk_Unknown;
        mBuffer->requestId = 0;
        mBuffer->flushData = flushData;
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

    }
}

void Reset_Msg_Buffer(UA_Msg_Buffer* mBuffer){
    if(mBuffer != UA_NULL){
        assert(mBuffer->nbBuffers == 1);
        Reset_Buffer(mBuffer->buffers);
        mBuffer->type = TCP_UA_Message_Unknown;
        mBuffer->secureType = UA_SecureMessage;
        mBuffer->msgSize = 0;
        mBuffer->nbChunks = 1;
        mBuffer->isFinal = UA_Msg_Chunk_Unknown;
        mBuffer->requestId = 0;
        mBuffer->sequenceNumberPosition = 0;
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

void Internal_Copy_Msg_Buffer_Properties(UA_Msg_Buffer* destMsgBuffer,
                                         UA_Msg_Buffer* srcMsgBuffer){
    assert(destMsgBuffer != UA_NULL && srcMsgBuffer != UA_NULL);
    // Copy all internal properties except the buffers and number of buffers
    destMsgBuffer->type = srcMsgBuffer->type;
    destMsgBuffer->secureType = srcMsgBuffer->secureType;
    destMsgBuffer->msgSize = srcMsgBuffer->msgSize;
    destMsgBuffer->nbChunks = srcMsgBuffer->nbChunks;
    destMsgBuffer->sequenceNumberPosition = srcMsgBuffer->sequenceNumberPosition;
    destMsgBuffer->isFinal = srcMsgBuffer->isFinal;
    destMsgBuffer->requestId = srcMsgBuffer->requestId;
}

StatusCode Copy_Buffer_To_Msg_Buffer(UA_Msg_Buffer* destMsgBuffer,
                                     UA_Msg_Buffer* srcMsgBuffer){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(destMsgBuffer != UA_NULL && srcMsgBuffer != UA_NULL){
        assert(destMsgBuffer->nbBuffers == 1);
        assert(srcMsgBuffer->nbBuffers == 1);
        status = STATUS_OK;
        // Copy everything except the flush data which can be used for a different flush level
        //  and nbBuffers which is set on initialization
        Internal_Copy_Msg_Buffer_Properties(destMsgBuffer, srcMsgBuffer);

        status = Copy_Buffer(destMsgBuffer->buffers, srcMsgBuffer->buffers);
    }
    return status;
}

UA_Msg_Buffers* Create_Msg_Buffers(uint32_t maxChunks,
                                   uint32_t bufferSize){
    assert(maxChunks > 0);
    StatusCode status = STATUS_OK;
    UA_Msg_Buffers* mBuffers = UA_NULL;
    uint32_t idx = 0;
    mBuffers = (UA_Msg_Buffer*) malloc(sizeof(UA_Msg_Buffer));
    if(mBuffers != UA_NULL){
        mBuffers->nbBuffers = maxChunks;
        mBuffers->buffers = (Buffer*) malloc(sizeof(Buffer) * maxChunks);

        if(mBuffers->buffers != UA_NULL){
            while(status == STATUS_OK && idx < maxChunks){
                status = Init_Buffer(&(mBuffers->buffers[idx]), bufferSize);
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
            mBuffers->requestId = 0;
            mBuffers->flushData = UA_NULL;
        }else{
            Delete_Msg_Buffers(&mBuffers);
        }
    }
    return mBuffers;
}

void Reset_Msg_Buffers(UA_Msg_Buffers* mBuffer){
    uint32_t idx = 0;
    if(mBuffer != UA_NULL){
        for(idx = 0; idx < mBuffer->maxChunks; idx++){
            Reset_Buffer(&(mBuffer->buffers[idx]));
        }
        mBuffer->type = TCP_UA_Message_Unknown;
        mBuffer->secureType = UA_SecureMessage;
        mBuffer->msgSize = 0;
        mBuffer->nbChunks = 0;
        mBuffer->isFinal = UA_Msg_Chunk_Unknown;
        mBuffer->requestId = 0;
        mBuffer->sequenceNumberPosition = 0;
    }
}

void Delete_Msg_Buffers(UA_Msg_Buffers** ptBuffers){
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

Buffer* Get_Current_Chunk_From_Msg_Buffers(UA_Msg_Buffers* mBuffer){
    Buffer* buf = UA_NULL;
    if(mBuffer != UA_NULL){
        if(mBuffer->nbChunks > 0){
            buf = &(mBuffer->buffers[mBuffer->nbChunks - 1]);
        }
    }
    return buf;
}

Buffer* Next_Chunk_From_Msg_Buffers(UA_Msg_Buffers* mBuffer,
                                    uint32_t*       bufferIdx){
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

void Internal_Copy_Msg_Buffers_Properties(UA_Msg_Buffers* destMsgBuffer,
                                          UA_Msg_Buffer*  srcMsgBuffer){
    Internal_Copy_Msg_Buffer_Properties((UA_Msg_Buffer*) destMsgBuffer, srcMsgBuffer);
}

StatusCode Copy_Buffer_To_Msg_Buffers(UA_Msg_Buffers* destMsgBuffer,
                                      uint32_t        bufferIdx,
                                      UA_Msg_Buffer*  srcMsgBuffer,
                                      uint32_t        limitedLength){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(destMsgBuffer != UA_NULL && srcMsgBuffer != UA_NULL){
        assert(srcMsgBuffer->nbBuffers == 1);
        status = STATUS_OK;
        // Copy everything except the flush data which can be used for a different flush level
        //  and nbBuffers which is set on initialization
        Internal_Copy_Msg_Buffer_Properties(destMsgBuffer, srcMsgBuffer);

        status = Copy_Buffer_Limited_Length(&(destMsgBuffer->buffers[bufferIdx]),
                                            srcMsgBuffer->buffers,
                                            limitedLength);
    }
    return status;
}

