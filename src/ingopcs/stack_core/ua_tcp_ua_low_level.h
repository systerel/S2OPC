/*
 * tcp_ua_low_level.h
 *
 *  Created on: Aug 2, 2016
 *      Author: vincent
 */

#ifndef INGOPCS_TCP_SOPC_LOW_LEVEL_H_
#define INGOPCS_TCP_SOPC_LOW_LEVEL_H_

#include <ua_msg_buffer.h>
#include <ua_sockets.h>

extern const uint32_t tcpProtocolVersion;

SOPC_StatusCode TCP_SOPC_WriteMsgBuffer(SOPC_MsgBuffer*  msgBuffer,
                                 const SOPC_Byte* data_src,
                                 uint32_t       count);

SOPC_StatusCode TCP_SOPC_ReadMsgBuffer(SOPC_Byte* data_dest,
                                uint32_t size,
                                SOPC_MsgBuffer* msgBuffer,
                                uint32_t count);

SOPC_StatusCode TCP_SOPC_FlushMsgBuffer(SOPC_MsgBuffer* msgBuffer);

SOPC_StatusCode TCP_SOPC_EncodeHeader(SOPC_MsgBuffer*  msgBuffer,
                               TCP_SOPC_MsgType type);

SOPC_StatusCode TCP_SOPC_FinalizeHeader(SOPC_MsgBuffer* msgBuffer);

SOPC_StatusCode TCP_SOPC_ReadData(SOPC_Socket*    socket,
                           SOPC_MsgBuffer* msgBuffer);

SOPC_StatusCode TCP_SOPC_ReadHeader(SOPC_MsgBuffer* msgBuffer);

#endif /* INGOPCS_TCP_SOPC_LOW_LEVEL_H_ */
