/*
 * ua_encoder.h
 *
 *  Created on: Aug 3, 2016
 *      Author: vincent
 */

#ifndef INGOPCS_UA_ENCODER_H_
#define INGOPCS_UA_ENCODER_H_

#include <opcua_ingopcs_types.h>
#include <platform_deps.h>
#include <msg_buffer.h>

#define SWAP_2_BYTES(x) (x & 0x00FF << 8) | (x & 0xFF00 >> 8)
#define SWAP_3_BYTES(x) (x & 0x0000FF << 16) | (x & 0x00FF00) \
| (x & 0xFF0000 >> 16)
#define SWAP_4_BYTES(x) (x & 0x000000FF << 24) | (x & 0x0000FF00) << 8 \
| (x & 0xFF000000 >> 24) | (x & 0x00FF0000 >> 8)
#define SWAP_8_BYTES(x) \
(x & 0x00000000000000FF) << 56) \
| (x & 0x000000000000FF00) << 40 \
| (x & 0x0000000000FF0000) << 24 \
| (x & 0x00000000FF000000) << 8 \
| (x & 0xFF00000000000000) >> 56) \
| (x & 0x00FF000000000000) >> 40 \
| (x & 0x0000FF0000000000) >> 24 \
| (x & 0x000000FF00000000) >> 8

StatusCode Write_UInt32(UA_Msg_Buffer* msgBuffer, uint32_t value);
StatusCode Read_UInt32(UA_Msg_Buffer* msgBuffer, uint32_t* value);
StatusCode Write_Int32(UA_Msg_Buffer* msgBuffer, int32_t value);
StatusCode Read_Int32(UA_Msg_Buffer* msgBuffer, int32_t* value);
StatusCode Write_UA_String(UA_Msg_Buffer* msgBuffer, UA_String* str);
StatusCode Read_UA_String(UA_Msg_Buffer* msgBuffer, UA_String* str);

#endif /* INGOPCS_UA_ENCODER_H_ */
