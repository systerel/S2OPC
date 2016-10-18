/*
 * ua_encoder.h
 *
 *  Created on: Aug 3, 2016
 *      Author: vincent
 */

#ifndef INGOPCS_UA_ENCODER_H_
#define INGOPCS_UA_ENCODER_H_

#include <platform_deps.h>
#include <ua_builtintypes.h>
#include <ua_msg_buffer.h>

#define SWAP_2_BYTES(x) (x & 0x00FF) << 8 | (x & 0xFF00) >> 8
#define SWAP_3_BYTES(x) (x & 0x0000FF) << 16 | (x & 0x00FF00) \
| (x & 0xFF0000) >> 16
#define SWAP_4_BYTES(x) (x & 0x000000FF) << 24 | (x & 0x0000FF00) << 8 \
| (x & 0xFF000000) >> 24 | (x & 0x00FF0000) >> 8
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

/**
 *  \brief Encode an signed 16 bits integer from machine endianess representation to binary UA encoding endianess representation.
 *  And respectively integer from UA binary to machine endianess representation.
 *  Note: UA binary representation is little endian thus nothing is done if machine representaiton is little endian
 *  (platform dependency module providing endianess information is used to determine the current case)
 *
 *  \param intv     Pointer to the integer value to encode or decode with correct endianess in place
 */
void EncodeDecode_Int16(int16_t* intv);

/**
 *  \brief Encode a unsigned 16 bits integer from machine endianess representation to binary UA encoding endianess representation.
 *  And respectively integer from UA binary to machine endianess representation.
 *  Note: UA binary representation is little endian thus nothing is done if machine representaiton is little endian
 *  (platform dependency module providing endianess information is used to determine the current case)
 *
 *  \param intv     Pointer to the integer value to encode or decode with correct endianess in place
 */
void EncodeDecode_UInt16(uint16_t* uintv);

/**
 *  \brief Encode a signed 32 bits integer from machine endianess representation to binary UA encoding endianess representation.
 *  And respectively decode integer from UA binary to machine endianess representation.
 *  Note: UA binary representation is little endian thus nothing is done if machine representaiton is little endian
 *  (platform dependency module providing endianess information is used to determine the current case)
 *
 *  \param intv     Pointer to the integer value to encode or decode with correct endianess in place
 */
void EncodeDecode_Int32(int32_t* intv);

/**
 *  \brief Encode an unsigned 32 bits integer from machine endianess representation to binary UA encoding endianess representation.
 *  And respectively integer from UA binary to machine endianess representation.
 *  Note: UA binary representation is little endian thus nothing is done if machine representaiton is little endian
 *  (platform dependency module providing endianess information is used to determine the current case)
 *
 *  \param intv     Pointer to the integer value to encode or decode with correct endianess in place
 */
void EncodeDecode_UInt32(uint32_t* uintv);

/**
 *  \brief Encode a signed 64 bits integer from machine endianess representation to binary UA encoding endianess representation.
 *  And respectively integer from UA binary to machine endianess representation.
 *  Note: UA binary representation is little endian thus nothing is done if machine representaiton is little endian
 *  (platform dependency module providing endianess information is used to determine the current case)
 *
 *  \param intv     Pointer to the integer value to encode or decode with correct endianess in place
 */
void EncodeDecode_Int64(int64_t* intv);

/**
 *  \brief Encode an unsigned 64 bits integer from machine endianess representation to binary UA encoding endianess representation.
 *  And respectively integer from UA binary to machine endianess representation.
 *  Note: UA binary representation is little endian thus nothing is done if machine representaiton is little endian
 *  (platform dependency module providing endianess information is used to determine the current case)
 *
 *  \param intv     Pointer to the integer value to encode or decode with correct endianess in place
 */
void EncodeDecode_UInt64(uint64_t* uintv);

/**
 *  \brief Encode a float from machine endianess representation to binary UA encoding endianess representation.
 *  And respectively float from UA binary to machine endianess representation.
 *  Note: UA binary representation is little endian thus nothing is done if machine representaiton is little endian
 *  (platform dependency module providing endianess information is used to determine the current case)
 *
 *  \param floatv     Pointer to the integer value to encode or decode with correct endianess in place
 */
void EncodeDecode_Float(float* floatv);

/**
 *  \brief Encode a double from machine endianess representation to binary UA encoding endianess representation.
 *  And respectively decode a double from UA binary to machine endianess representation.
 *  Note: UA binary representation is little endian thus nothing is done if machine representaiton is little endian
 *  (platform dependency module providing endianess information is used to determine the current case)
 *
 *  \param doublev     Pointer to the integer value to encode or decode with correct endianess in place
 */
void EncodeDecode_Double(double* doublev);

StatusCode Byte_Write(const UA_Byte* value, UA_MsgBuffer* msgBuffer);
StatusCode Byte_Read(UA_Byte* value, UA_MsgBuffer* msgBuffer);
StatusCode Boolean_Write(const UA_Boolean* value, UA_MsgBuffer* msgBuffer);
StatusCode Boolean_Read(UA_Boolean* value, UA_MsgBuffer* msgBuffer);
StatusCode SByte_Write(const UA_SByte* value, UA_MsgBuffer* msgBuffer);
StatusCode SByte_Read(UA_SByte* value, UA_MsgBuffer* msgBuffer);
StatusCode Int16_Write(const int16_t* value, UA_MsgBuffer* msgBuffer);
StatusCode Int16_Read(int16_t* value, UA_MsgBuffer* msgBuffer);
StatusCode UInt16_Write(const uint16_t* value, UA_MsgBuffer* msgBuffer);
StatusCode UInt16_Read(uint16_t* value, UA_MsgBuffer* msgBuffer);
StatusCode UInt32_Write(const uint32_t* value, UA_MsgBuffer* msgBuffer);
StatusCode UInt32_Read(uint32_t* value, UA_MsgBuffer* msgBuffer);
StatusCode Int32_Write(const int32_t* value, UA_MsgBuffer* msgBuffer);
StatusCode Int32_Read(int32_t* value, UA_MsgBuffer* msgBuffer);
StatusCode Int64_Write(const int64_t* value, UA_MsgBuffer* msgBuffer);
StatusCode Int64_Read(int64_t* value, UA_MsgBuffer* msgBuffer);
StatusCode Float_Write(const float* value, UA_MsgBuffer* msgBuffer);
StatusCode Float_Read(float* value, UA_MsgBuffer* msgBuffer);
StatusCode Double_Write(const double* value, UA_MsgBuffer* msgBuffer);
StatusCode Double_Read(double* value, UA_MsgBuffer* msgBuffer);
StatusCode ByteString_Write(const UA_ByteString* str, UA_MsgBuffer* msgBuffer);
StatusCode ByteString_Read(UA_ByteString* str, UA_MsgBuffer* msgBuffer);
StatusCode String_Write(const UA_String* str, UA_MsgBuffer* msgBuffer);
StatusCode String_Read(UA_String* str, UA_MsgBuffer* msgBuffer);
StatusCode XmlElement_Write(const UA_XmlElement* xml, UA_MsgBuffer* msgBuffer);
StatusCode XmlElement_Read(UA_XmlElement* xml, UA_MsgBuffer* msgBuffer);
StatusCode DateTime_Write(const UA_DateTime* date, UA_MsgBuffer* msgBuffer);
StatusCode DateTime_Read(UA_DateTime* date, UA_MsgBuffer* msgBuffer);
StatusCode Guid_Write(const UA_Guid* guid, UA_MsgBuffer* msgBuffer);
StatusCode Guid_Read(UA_Guid* guid, UA_MsgBuffer* msgBuffer);
StatusCode NodeId_Write(const UA_NodeId* nodeId, UA_MsgBuffer* msgBuffer);
StatusCode NodeId_Read(UA_NodeId* nodeId, UA_MsgBuffer* msgBuffer);
StatusCode ExpandedNodeId_Write(const UA_ExpandedNodeId* expNodeId, UA_MsgBuffer* msgBuffer);
StatusCode ExpandedNodeId_Read(UA_ExpandedNodeId* expNodeId, UA_MsgBuffer* msgBuffer);
StatusCode StatusCode_Write(const StatusCode* status, UA_MsgBuffer* msgBuffer);
StatusCode StatusCode_Read(StatusCode* status, UA_MsgBuffer* msgBuffer);
StatusCode DiagnosticInfo_Write(const UA_DiagnosticInfo* diagInfo, UA_MsgBuffer* msgBuffer);
StatusCode DiagnosticInfo_Read(UA_DiagnosticInfo* diagInfo, UA_MsgBuffer* msgBuffer);
StatusCode QualifiedName_Write(const UA_QualifiedName* qname, UA_MsgBuffer* msgBuffer);
StatusCode QualifiedName_Read(UA_QualifiedName* qname, UA_MsgBuffer* msgBuffer);
StatusCode LocalizedText_Write(const UA_LocalizedText* localizedText, UA_MsgBuffer* msgBuffer);
StatusCode LocalizedText_Read(UA_LocalizedText* localizedText, UA_MsgBuffer* msgBuffer);
StatusCode ExtensionObject_Write(const UA_ExtensionObject* extObj, UA_MsgBuffer* msgBuffer);
StatusCode ExtensionObject_Read(UA_ExtensionObject* extObj, UA_MsgBuffer* msgBuffer);
StatusCode Variant_Write(const UA_Variant* variant, UA_MsgBuffer* msgBuffer);
StatusCode Variant_Read(UA_Variant* variant, UA_MsgBuffer* msgBuffer);
StatusCode DataValue_Write(const UA_DataValue* dataValue, UA_MsgBuffer* msgBuffer);
StatusCode DataValue_Read(UA_DataValue* dataValue, UA_MsgBuffer* msgBuffer);


#endif /* INGOPCS_UA_ENCODER_H_ */
