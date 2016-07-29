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

typedef struct UA_Msg_Buffer {
    Buffer*             buffer;
    TCP_UA_Message_Type type;
    uint32_t            length;
    uint32_t            nbChunks;
    uint32_t            chunkSize;
} UA_Msg_Buffer;

UA_Msg_Buffer* Create_Msg_Buffer(Buffer* buffer);
void Delete_Msg_Buffer(UA_Msg_Buffer* mBuffer);

#endif /* INGOPCS_MSG_BUFFER_H_ */
