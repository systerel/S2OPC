/*
 * msg_buffer.h
 *
 *  Created on: Jul 22, 2016
 *      Author: vincent
 */

#ifndef INGOPCS_MSG_BUFFER_H_
#define INGOPCS_MSG_BUFFER_H_

#include <opcua_tcpstream.h>

#include <buffer.h>

typedef OpcUa_TcpStream_MessageType TCP_UA_Message_Type;

typedef struct UA_Msg_Buffer {
    Buffer*             buffer;
    TCP_UA_Message_Type type;
    uint32_t            length;
    uint32_t            nbChunks;
    uint32_t            chunkSize;
} UA_Msg_Buffer;

#endif /* INGOPCS_MSG_BUFFER_H_ */
