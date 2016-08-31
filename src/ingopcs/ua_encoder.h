/*
 * ua_encoder.h
 *
 *  Created on: Aug 3, 2016
 *      Author: vincent
 */

#ifndef INGOPCS_UA_ENCODER_H_
#define INGOPCS_UA_ENCODER_H_

#include <platform_deps.h>
#include "ua_builtintypes.h"
#include "ua_msg_buffer.h"

#define SWAP_2_BYTES(x) (x & 0x00FF << 8) | (x & 0xFF00 >> 8)
#define SWAP_3_BYTES(x) (x & 0x0000FF << 16) | (x & 0x00FF00) \
| (x & 0xFF0000 >> 16)
#define SWAP_4_BYTES(x) (x & 0x000000FF << 24) | (x & 0x0000FF00) << 8 \
| (x & 0xFF000000 >> 24) | (x & 0x00FF0000 >> 8)
#define SWAP_8_BYTES(x) \
(x & 0x00000000000000FF) << 56 \
| (x & 0x000000000000FF00) << 40 \
| (x & 0x0000000000FF0000) << 24 \
| (x & 0x00000000FF000000) << 8 \
| (x & 0xFF00000000000000) >> 56 \
| (x & 0x00FF000000000000) >> 40 \
| (x & 0x0000FF0000000000) >> 24 \
| (x & 0x000000FF00000000) >> 8

uint16_t EncodeDecode_UInt16(uint16_t from);
uint32_t EncodeDecode_UInt32(uint32_t from);
uint32_t EncodeDecode_Int32(int32_t from);
uint64_t EncodeDecode_Int64(int64_t from);


StatusCode Byte_Write(UA_MsgBuffer* msgBuffer, const UA_Byte* value);
StatusCode Byte_Read(UA_MsgBuffer* msgBuffer, UA_Byte* value);
StatusCode Boolean_Write(UA_MsgBuffer* msgBuffer, const UA_Boolean* value);
StatusCode Boolean_Read(UA_MsgBuffer* msgBuffer, UA_Boolean* value);
StatusCode UInt16_Write(UA_MsgBuffer* msgBuffer, const uint16_t* value);
StatusCode UInt16_Read(UA_MsgBuffer* msgBuffer, uint16_t* value);
StatusCode UInt32_Write(UA_MsgBuffer* msgBuffer, const uint32_t* value);
StatusCode UInt32_Read(UA_MsgBuffer* msgBuffer, uint32_t* value);
StatusCode Int32_Write(UA_MsgBuffer* msgBuffer, const int32_t* value);
StatusCode Int32_Read(UA_MsgBuffer* msgBuffer, int32_t* value);
StatusCode Int64_Write(UA_MsgBuffer* msgBuffer, const int64_t* value);
StatusCode Int64_Read(UA_MsgBuffer* msgBuffer, int64_t* value);
StatusCode ByteString_Write(UA_MsgBuffer* msgBuffer, const UA_ByteString* str);
StatusCode ByteString_Read(UA_MsgBuffer* msgBuffer, UA_ByteString* str);
StatusCode String_Write(UA_MsgBuffer* msgBuffer, const UA_String* str);
StatusCode String_Read(UA_MsgBuffer* msgBuffer, UA_String* str);
StatusCode XmlElement_Write(UA_MsgBuffer* msgBuffer, const UA_XmlElement* xml);
StatusCode XmlElement_Read(UA_MsgBuffer* msgBuffer, UA_XmlElement* xml);
StatusCode DateTime_Write(UA_MsgBuffer* msgBuffer, const UA_DateTime* date);
StatusCode DateTime_Read(UA_MsgBuffer* msgBuffer, UA_DateTime* date);
StatusCode Guid_Write(UA_MsgBuffer* msgBuffer, const UA_Guid* guid);
StatusCode Guid_Read(UA_MsgBuffer* msgBuffer, UA_Guid* guid);
StatusCode NodeId_Write(UA_MsgBuffer* msgBuffer, const UA_NodeId* nodeId);
StatusCode NodeId_Read(UA_MsgBuffer* msgBuffer, UA_NodeId* nodeId);
StatusCode ExpandedNodeId_Write(UA_MsgBuffer* msgBuffer, const UA_ExpandedNodeId* expNodeId);
StatusCode ExpandedNodeId_Read(UA_MsgBuffer* msgBuffer, UA_ExpandedNodeId* expNodeId);
StatusCode Guid_Write(UA_MsgBuffer* msgBuffer, const UA_Guid* guid);
StatusCode Guid_Read(UA_MsgBuffer* msgBuffer, UA_Guid* guid);
StatusCode StatusCode_Write(UA_MsgBuffer* msgBuffer, const StatusCode* status);
StatusCode StatusCode_Read(UA_MsgBuffer* msgBuffer, StatusCode* status);
StatusCode DiagnosticInfo_Write(UA_MsgBuffer* msgBuffer, const UA_DiagnosticInfo* diagInfo);
StatusCode DiagnosticInfo_Read(UA_MsgBuffer* msgBuffer, UA_DiagnosticInfo* diagInfo);


#endif /* INGOPCS_UA_ENCODER_H_ */
