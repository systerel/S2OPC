/*
 * msg_buffer.h
 *
 *  Created on: Jul 22, 2016
 *      Author: vincent
 */

#ifndef INGOPCS_MSG_BUFFER_H_
#define INGOPCS_MSG_BUFFER_H_

#include <wrappers.h>

#include <buffer.h>

#define TCP_UA_ACK_MSG_LENGTH 28
#define TCP_UA_ERR_MIN_MSG_LENGTH 16

#define UA_HEADER_LENGTH 8
#define UA_HEADER_SN_LENGTH 4
#define UA_HEADER_LENGTH_POSITION 4
#define UA_HEADER_ISFINAL_POSITION 3

#define UA_SECURE_MESSAGE_HEADER_LENGTH 12
#define UA_SECURE_MESSAGE_SEQUENCE_LENGTH 8

extern const UA_Byte HEL[3];
extern const UA_Byte ACK[3];
extern const UA_Byte ERR[3];
extern const UA_Byte MSG[3];
extern const UA_Byte OPN[3];
extern const UA_Byte CLO[3];

typedef enum {
    UA_SecureMessage,
    UA_OpenSecureChannel,
    UA_CloseSecureChannel
} UA_SecureMessageType;

typedef enum {
    TCP_UA_Message_Unknown,
    TCP_UA_Message_Invalid,
    TCP_UA_Message_Hello,
    TCP_UA_Message_Acknowledge,
    TCP_UA_Message_Error,
    TCP_UA_Message_SecureMessage
} TCP_UA_MsgType;


typedef enum {
    UA_Msg_Chunk_Unknown,
    UA_Msg_Chunk_Invalid,
    UA_Msg_Chunk_Intermediate,
    UA_Msg_Chunk_Final,
    UA_Msg_Chunk_Abort
} UA_MsgFinalChunk;

typedef struct UA_MsgBuffer {
    uint32_t             nbBuffers;
    Buffer*              buffers;
    TCP_UA_MsgType       type;
    UA_SecureMessageType secureType; //only valid if type = SecureMessage
    uint32_t             msgSize;
    uint32_t             nbChunks;
    uint32_t             maxChunks;
    uint32_t             sequenceNumberPosition; // Position of sequence number (data to encrypt)
    UA_MsgFinalChunk     isFinal;
    uint32_t             requestId;
    void*                flushData;
} UA_MsgBuffer;

// Only 1 buffer by msg mode:
UA_MsgBuffer* MsgBuffer_Create(Buffer*  buffer,
                               uint32_t maxChunks,
                               void*    flushData);
void MsgBuffer_Delete(UA_MsgBuffer** mBuffer);
void MsgBuffer_Reset(UA_MsgBuffer* mBuffer);
StatusCode MsgBuffer_ResetNextChunk(UA_MsgBuffer* mBuffer,
                                    uint32_t      bodyPosition);
StatusCode MsgBuffer_SetSecureMsgType(UA_MsgBuffer* mBuffer,
                                      UA_SecureMessageType sType);

StatusCode MsgBuffer_CopyBuffer(UA_MsgBuffer* destMsgBuffer,
                                UA_MsgBuffer* srcMsgBuffer);

typedef struct UA_MsgBuffer UA_MsgBuffers;
// Several buffers by msg mode (secure message input buffer only)
UA_MsgBuffers* MsgBuffers_Create(uint32_t maxChunks,
                                 uint32_t bufferSize);
void MsgBuffers_Reset(UA_MsgBuffers* mBuffer);
void MsgBuffers_Delete(UA_MsgBuffers** mBuffer);

Buffer* MsgBuffers_GetCurrentChunk(UA_MsgBuffers* mBuffer);
Buffer* MsgBuffers_NextChunk(UA_MsgBuffers* mBuffer,
                             uint32_t*      bufferIdx);

StatusCode MsgBuffers_CopyBuffer(UA_MsgBuffers* destMsgBuffer,
                                 uint32_t       bufferIdx,
                                 UA_MsgBuffer*  srcMsgBuffer,
                                 uint32_t       limitedLength);


#endif /* INGOPCS_MSG_BUFFER_H_ */
