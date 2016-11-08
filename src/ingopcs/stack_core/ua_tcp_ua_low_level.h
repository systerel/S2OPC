/*
 * tcp_ua_low_level.h
 *
 *  Created on: Aug 2, 2016
 *      Author: vincent
 */

#ifndef INGOPCS_TCP_UA_LOW_LEVEL_H_
#define INGOPCS_TCP_UA_LOW_LEVEL_H_

#include <ua_msg_buffer.h>
#include <ua_sockets.h>

extern const uint32_t tcpProtocolVersion;

SOPC_StatusCode TCP_UA_WriteMsgBuffer(UA_MsgBuffer*  msgBuffer,
                                 const UA_Byte* data_src,
                                 uint32_t       count);

SOPC_StatusCode TCP_UA_ReadMsgBuffer(UA_Byte* data_dest,
                                uint32_t size,
                                UA_MsgBuffer* msgBuffer,
                                uint32_t count);

SOPC_StatusCode TCP_UA_FlushMsgBuffer(UA_MsgBuffer* msgBuffer);

SOPC_StatusCode TCP_UA_EncodeHeader(UA_MsgBuffer*  msgBuffer,
                               TCP_UA_MsgType type);

SOPC_StatusCode TCP_UA_FinalizeHeader(UA_MsgBuffer* msgBuffer);

SOPC_StatusCode TCP_UA_ReadData(UA_Socket*    socket,
                           UA_MsgBuffer* msgBuffer);

SOPC_StatusCode TCP_UA_ReadHeader(UA_MsgBuffer* msgBuffer);

#endif /* INGOPCS_TCP_UA_LOW_LEVEL_H_ */
