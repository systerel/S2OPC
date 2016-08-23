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

typedef enum UA_Secure_Message_Type{
    UA_SecureMessage,
    UA_OpenSecureChannel,
    UA_CloseSecureChannel
} UA_Secure_Message_Type;

typedef enum TCP_UA_Message_Type{
    TCP_UA_Message_Unknown,
    TCP_UA_Message_Invalid,
    TCP_UA_Message_Hello,
    TCP_UA_Message_Acknowledge,
    TCP_UA_Message_Error,
    TCP_UA_Message_SecureMessage
} TCP_UA_Message_Type;


typedef enum UA_Msg_Final_Chunk{
    UA_Msg_Chunk_Unknown,
    UA_Msg_Chunk_Invalid,
    UA_Msg_Chunk_Intermediate,
    UA_Msg_Chunk_Final,
    UA_Msg_Chunk_Abort
} UA_Msg_Final_Chunk;

typedef struct UA_Msg_Buffer {
    uint32_t               nbBuffers;
    Buffer*                buffers;
    TCP_UA_Message_Type    type;
    UA_Secure_Message_Type secureType; //only valid if type = SecureMessage
    uint32_t               msgSize;
    uint32_t               nbChunks;
    uint32_t               maxChunks;
    uint32_t               sequenceNumberPosition; // Position of sequence number (data to encrypt)
    UA_Msg_Final_Chunk     isFinal;
    uint32_t               requestId;
    void*                  flushData;
} UA_Msg_Buffer;

// Only 1 buffer by msg mode:
UA_Msg_Buffer* Create_Msg_Buffer(Buffer*  buffer,
                                 uint32_t maxChunks,
                                 void*    flushData);
void Delete_Msg_Buffer(UA_Msg_Buffer** mBuffer);
void Reset_Msg_Buffer(UA_Msg_Buffer* mBuffer);
StatusCode Reset_Msg_Buffer_Next_Chunk(UA_Msg_Buffer* mBuffer,
                                       uint32_t       bodyPosition);
StatusCode Set_Secure_Message_Type(UA_Msg_Buffer* mBuffer,
                                   UA_Secure_Message_Type sType);

StatusCode Copy_Buffer_To_Msg_Buffer(UA_Msg_Buffer* destMsgBuffer,
                                     UA_Msg_Buffer* srcMsgBuffer);

typedef struct UA_Msg_Buffer UA_Msg_Buffers;
// Several buffers by msg mode (secure message input buffer only)
UA_Msg_Buffers* Create_Msg_Buffers(uint32_t maxChunks,
                                   uint32_t bufferSize);
void Reset_Msg_Buffers(UA_Msg_Buffers* mBuffer);
void Delete_Msg_Buffers(UA_Msg_Buffers** mBuffer);

Buffer* Get_Current_Chunk_From_Msg_Buffers(UA_Msg_Buffers* mBuffer);
Buffer* Next_Chunk_From_Msg_Buffers(UA_Msg_Buffers* mBuffer,
                                    uint32_t*       bufferIdx);

StatusCode Copy_Buffer_To_Msg_Buffers(UA_Msg_Buffers* destMsgBuffer,
                                      uint32_t        bufferIdx,
                                      UA_Msg_Buffer*  srcMsgBuffer);


#endif /* INGOPCS_MSG_BUFFER_H_ */
