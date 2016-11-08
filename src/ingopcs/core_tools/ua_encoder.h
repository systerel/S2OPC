/*
 * ua_encoder.h
 *
 *  Created on: Aug 3, 2016
 *      Author: vincent
 */

#ifndef INGOPCS_SOPC_ENCODER_H_
#define INGOPCS_SOPC_ENCODER_H_

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
} SOPC_NodeId_DataEncoding;

typedef enum {
    DiagInfoEncoding_SymbolicId = 0x01,
    DiagInfoEncoding_Namespace = 0x02,
    DiagInfoEncoding_LocalizedTest = 0x04,
    DiagInfoEncoding_Locale = 0x08,
    DiagInfoEncoding_AdditionalInfo = 0x10,
    DiagInfoEncoding_InnerStatusCode = 0x20,
    DiagInfoEncoding_InnerDianosticInfo = 0x40,
} SOPC_DiagInfo_EncodingFlag;

typedef enum {
    LocalizedText_Locale = 0x01,
    LocalizedText_Text   = 0x02
} SOPC_LocalizedText_EncodingFlag;

typedef enum {
    DataValue_NotNullValue = 0x01,
    DataValue_NotGoodStatusCode = 0x02,
    DataValue_NotMinSourceDate = 0x04,
    DataValue_NotMinServerDate = 0x08,
    DataValue_NotZeroSourcePico = 0x10,
    DataValue_NotZeroServerPico = 0x20
} SOPC_DataValue_EncodingFlag;

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

SOPC_StatusCode Byte_Write(const SOPC_Byte* value, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode Byte_Read(SOPC_Byte* value, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode Boolean_Write(const SOPC_Boolean* value, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode Boolean_Read(SOPC_Boolean* value, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode SByte_Write(const SOPC_SByte* value, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode SByte_Read(SOPC_SByte* value, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode Int16_Write(const int16_t* value, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode Int16_Read(int16_t* value, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode UInt16_Write(const uint16_t* value, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode UInt16_Read(uint16_t* value, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode Int32_Write(const int32_t* value, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode Int32_Read(int32_t* value, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode UInt32_Write(const uint32_t* value, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode UInt32_Read(uint32_t* value, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode Int64_Write(const int64_t* value, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode Int64_Read(int64_t* value, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode UInt64_Write(const uint64_t* value, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode UInt64_Read(uint64_t* value, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode Float_Write(const float* value, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode Float_Read(float* value, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode Double_Write(const double* value, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode Double_Read(double* value, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode DateTime_Write(const SOPC_DateTime* date, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode DateTime_Read(SOPC_DateTime* date, SOPC_MsgBuffer* msgBuffer);

SOPC_StatusCode ByteString_Write(const SOPC_ByteString* str, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode ByteString_Read(SOPC_ByteString* str, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode String_Write(const SOPC_String* str, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode String_Read(SOPC_String* str, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode XmlElement_Write(const SOPC_XmlElement* xml, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode XmlElement_Read(SOPC_XmlElement* xml, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode Guid_Write(const SOPC_Guid* guid, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode Guid_Read(SOPC_Guid* guid, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode NodeId_Write(const SOPC_NodeId* nodeId, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode NodeId_Read(SOPC_NodeId* nodeId, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode ExpandedNodeId_Write(const SOPC_ExpandedNodeId* expNodeId, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode ExpandedNodeId_Read(SOPC_ExpandedNodeId* expNodeId, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode StatusCode_Write(const SOPC_StatusCode* status, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode StatusCode_Read(SOPC_StatusCode* status, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode DiagnosticInfo_Write(const SOPC_DiagnosticInfo* diagInfo, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode DiagnosticInfo_Read(SOPC_DiagnosticInfo* diagInfo, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode QualifiedName_Write(const SOPC_QualifiedName* qname, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode QualifiedName_Read(SOPC_QualifiedName* qname, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode LocalizedText_Write(const SOPC_LocalizedText* localizedText, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode LocalizedText_Read(SOPC_LocalizedText* localizedText, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode ExtensionObject_Write(const SOPC_ExtensionObject* extObj, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode ExtensionObject_Read(SOPC_ExtensionObject* extObj, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode Variant_Write(const SOPC_Variant* variant, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode Variant_Read(SOPC_Variant* variant, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode DataValue_Write(const SOPC_DataValue* dataValue, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode DataValue_Read(SOPC_DataValue* dataValue, SOPC_MsgBuffer* msgBuffer);


#endif /* INGOPCS_SOPC_ENCODER_H_ */
