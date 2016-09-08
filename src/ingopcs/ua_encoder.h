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

typedef enum {
    NodeIdEncoding_TwoByte = 0x00,
    NodeIdEncoding_FourByte = 0x01,
    NodeIdEncoding_Numeric = 0x02,
    NodeIdEncoding_String = 0x03,
    NodeIdEncoding_Guid = 0x04,
    NodeIdEncoding_ByteString = 0x05,
    NodeIdEncoding_NamespaceUriFlag = 0x80,
    NodeIdEncoding_ServerIndexFlag = 0x40,
    NodeIdEncoding_Invalid = 0xFF
} UA_NodeId_DataEncoding;

typedef enum {
    DiagInfoEncoding_SymbolicId = 0x01,
    DiagInfoEncoding_Namespace = 0x02,
    DiagInfoEncoding_LocalizedTest = 0x04,
    DiagInfoEncoding_Locale = 0x08,
    DiagInfoEncoding_AdditionalInfo = 0x10,
    DiagInfoEncoding_InnerStatusCode = 0x20,
    DiagInfoEncoding_InnerDianosticInfo = 0x40,
} UA_DiagInfo_EncodingFlag;

typedef enum {
    LocalizedText_Locale = 0x01,
    LocalizedText_Text   = 0x02
} UA_LocalizedText_EncodingFlag;

typedef enum {
    DataValue_NotNullValue = 0x01,
    DataValue_NotGoodStatusCode = 0x02,
    DataValue_NotMinSourceDate = 0x04,
    DataValue_NotMinServerDate = 0x08,
    DataValue_NotZeroSourcePico = 0x10,
    DataValue_NotZeroServerPico = 0x20
} UA_DataValue_EncodingFlag;

void EncodeDecode_Int16(int16_t* intv);
void EncodeDecode_UInt16(uint16_t* uintv);
void EncodeDecode_Int32(int32_t* intv);
void EncodeDecode_UInt32(uint32_t* uintv);
void EncodeDecode_Int64(int64_t* intv);
void EncodeDecode_UInt64(uint64_t* uintv);
void EncodeDecode_Float(float* floatv);
void EncodeDecode_Double(double* doublev);

StatusCode Byte_Write(UA_MsgBuffer* msgBuffer, const UA_Byte* value);
StatusCode Byte_Read(UA_MsgBuffer* msgBuffer, UA_Byte* value);
StatusCode Boolean_Write(UA_MsgBuffer* msgBuffer, const UA_Boolean* value);
StatusCode Boolean_Read(UA_MsgBuffer* msgBuffer, UA_Boolean* value);
StatusCode SByte_Write(UA_MsgBuffer* msgBuffer, const UA_SByte* value);
StatusCode SByte_Read(UA_MsgBuffer* msgBuffer, UA_SByte* value);
StatusCode Int16_Write(UA_MsgBuffer* msgBuffer, const int16_t* value);
StatusCode Int16_Read(UA_MsgBuffer* msgBuffer, int16_t* value);
StatusCode UInt16_Write(UA_MsgBuffer* msgBuffer, const uint16_t* value);
StatusCode UInt16_Read(UA_MsgBuffer* msgBuffer, uint16_t* value);
StatusCode UInt32_Write(UA_MsgBuffer* msgBuffer, const uint32_t* value);
StatusCode UInt32_Read(UA_MsgBuffer* msgBuffer, uint32_t* value);
StatusCode Int32_Write(UA_MsgBuffer* msgBuffer, const int32_t* value);
StatusCode Int32_Read(UA_MsgBuffer* msgBuffer, int32_t* value);
StatusCode Int64_Write(UA_MsgBuffer* msgBuffer, const int64_t* value);
StatusCode Int64_Read(UA_MsgBuffer* msgBuffer, int64_t* value);
StatusCode Float_Write(UA_MsgBuffer* msgBuffer, const float* value);
StatusCode Float_Read(UA_MsgBuffer* msgBuffer, float* value);
StatusCode Double_Write(UA_MsgBuffer* msgBuffer, const double* value);
StatusCode Double_Read(UA_MsgBuffer* msgBuffer, double* value);
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
StatusCode StatusCode_Write(UA_MsgBuffer* msgBuffer, const StatusCode* status);
StatusCode StatusCode_Read(UA_MsgBuffer* msgBuffer, StatusCode* status);
StatusCode DiagnosticInfo_Write(UA_MsgBuffer* msgBuffer, const UA_DiagnosticInfo* diagInfo);
StatusCode DiagnosticInfo_Read(UA_MsgBuffer* msgBuffer, UA_DiagnosticInfo* diagInfo);
StatusCode QualifiedName_Write(UA_MsgBuffer* msgBuffer, const UA_QualifiedName* qname);
StatusCode QualifiedName_Read(UA_MsgBuffer* msgBuffer, UA_QualifiedName* qname);
StatusCode LocalizedText_Write(UA_MsgBuffer* msgBuffer, const UA_LocalizedText* localizedText);
StatusCode LocalizedText_Read(UA_MsgBuffer* msgBuffer, UA_LocalizedText* localizedText);
StatusCode ExtensionObject_Write(UA_MsgBuffer* msgBuffer, const UA_ExtensionObject* extObj);
StatusCode ExtensionObject_Read(UA_MsgBuffer* msgBuffer, UA_ExtensionObject* extObj);
StatusCode Variant_Write(UA_MsgBuffer* msgBuffer, const UA_Variant* variant);
StatusCode Variant_Read(UA_MsgBuffer* msgBuffer, UA_Variant* variant);
StatusCode DataValue_Write(UA_MsgBuffer* msgBuffer, const UA_DataValue* dataValue);
StatusCode DataValue_Read(UA_MsgBuffer* msgBuffer, UA_DataValue* dataValue);


#endif /* INGOPCS_UA_ENCODER_H_ */
