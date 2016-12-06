/*
 *  Copyright (C) 2016 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SOPC_TCP_UA_LOW_LEVEL_H_
#define SOPC_TCP_UA_LOW_LEVEL_H_

#include "sopc_msg_buffer.h"
#include "sopc_sockets.h"

extern const uint32_t tcpProtocolVersion;

SOPC_StatusCode TCP_UA_WriteMsgBuffer(SOPC_MsgBuffer*  msgBuffer,
                                      const SOPC_Byte* data_src,
                                      uint32_t         count);

SOPC_StatusCode TCP_UA_ReadMsgBuffer(SOPC_Byte*      data_dest,
                                     uint32_t        size,
                                     SOPC_MsgBuffer* msgBuffer,
                                     uint32_t        count);

SOPC_StatusCode TCP_UA_FlushMsgBuffer(SOPC_MsgBuffer* msgBuffer);

SOPC_StatusCode TCP_UA_EncodeHeader(SOPC_MsgBuffer* msgBuffer,
                                    TCP_UA_MsgType  type);

SOPC_StatusCode TCP_UA_FinalizeHeader(SOPC_MsgBuffer* msgBuffer);

SOPC_StatusCode TCP_UA_ReadData(SOPC_Socket*    socket,
                                SOPC_MsgBuffer* msgBuffer);

SOPC_StatusCode TCP_UA_ReadHeader(SOPC_MsgBuffer* msgBuffer);

SOPC_StatusCode TCP_UA_CheckURI(const char* uri);

#endif /* SOPC_TCP_UA_LOW_LEVEL_H_ */
