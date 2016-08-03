/*
 * tcp_ua_low_level.h
 *
 *  Created on: Aug 2, 2016
 *      Author: vincent
 */

#ifndef INGOPCS_TCP_UA_LOW_LEVEL_H_
#define INGOPCS_TCP_UA_LOW_LEVEL_H_

#include <msg_buffer.h>

StatusCode Encode_TCP_UA_Header(UA_Msg_Buffer* msgBuffer,
                                TCP_UA_Message_Type type);

StatusCode Finalize_TCP_UA_Header(UA_Msg_Buffer* msgBuffer);

StatusCode Write_Msg_Buffer(UA_Msg_Buffer* msgBuffer,
                            UA_Byte* data_src,
                            uint32_t count);

StatusCode Flush_Msg_Buffer(Socket socket,
                            UA_Msg_Buffer* msgBuffer);

#endif /* INGOPCS_TCP_UA_LOW_LEVEL_H_ */
