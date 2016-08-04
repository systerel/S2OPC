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

#define TCP_UA_HEADER_LENGTH 8
#define TCP_UA_HEADER_LENGTH_POSITION 4
#define TCP_UA_ACK_MSG_LENGTH 48

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
    Buffer*                buffer;
    TCP_UA_Message_Type    type;
    UA_Secure_Message_Type secureType; //only valid if type = SecureMessage
    uint32_t               msgSize;
    uint32_t               nbChunks;
    uint32_t               maxChunks;
    UA_Msg_Final_Chunk     isFinal;
    //uint32_t            chunkSize;
} UA_Msg_Buffer;

UA_Msg_Buffer* Create_Msg_Buffer(Buffer*  buffer,
                                 uint32_t maxChunks);
void Delete_Msg_Buffer(UA_Msg_Buffer* mBuffer);
void Reset_Msg_Buffer(UA_Msg_Buffer* mBuffer);
StatusCode Reset_Msg_Buffer_Next_Chunk(UA_Msg_Buffer* mBuffer);

#endif /* INGOPCS_MSG_BUFFER_H_ */
