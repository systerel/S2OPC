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

#ifndef SOPC_ENCODER_H_
#define SOPC_ENCODER_H_

#include "platform_deps.h"
#include "sopc_builtintypes.h"
#include "sopc_msg_buffer.h"

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
    SOPC_LocalizedText_Locale = 0x01,
    SOPC_LocalizedText_Text   = 0x02
} SOPC_LocalizedText_EncodingFlag;

typedef enum {
    SOPC_DataValue_NotNullValue = 0x01,
    SOPC_DataValue_NotGoodStatusCode = 0x02,
    SOPC_DataValue_NotMinSourceDate = 0x04,
    SOPC_DataValue_NotMinServerDate = 0x08,
    SOPC_DataValue_NotZeroSourcePico = 0x10,
    SOPC_DataValue_NotZeroServerPico = 0x20
} SOPC_DataValue_EncodingFlag;

/**
 *  \brief Encode an signed 16 bits integer from machine endianess representation to binary UA encoding endianess representation.
 *  And respectively integer from UA binary to machine endianess representation.
 *  Note: UA binary representation is little endian thus nothing is done if machine representaiton is little endian
 *  (platform dependency module providing endianess information is used to determine the current case)
 *
 *  \param intv     Pointer to the integer value to encode or decode with correct endianess in place
 */
void SOPC_EncodeDecode_Int16(int16_t* intv);

/**
 *  \brief Encode a unsigned 16 bits integer from machine endianess representation to binary UA encoding endianess representation.
 *  And respectively integer from UA binary to machine endianess representation.
 *  Note: UA binary representation is little endian thus nothing is done if machine representaiton is little endian
 *  (platform dependency module providing endianess information is used to determine the current case)
 *
 *  \param intv     Pointer to the integer value to encode or decode with correct endianess in place
 */
void SOPC_EncodeDecode_UInt16(uint16_t* uintv);

/**
 *  \brief Encode a signed 32 bits integer from machine endianess representation to binary UA encoding endianess representation.
 *  And respectively decode integer from UA binary to machine endianess representation.
 *  Note: UA binary representation is little endian thus nothing is done if machine representaiton is little endian
 *  (platform dependency module providing endianess information is used to determine the current case)
 *
 *  \param intv     Pointer to the integer value to encode or decode with correct endianess in place
 */
void SOPC_EncodeDecode_Int32(int32_t* intv);

/**
 *  \brief Encode an unsigned 32 bits integer from machine endianess representation to binary UA encoding endianess representation.
 *  And respectively integer from UA binary to machine endianess representation.
 *  Note: UA binary representation is little endian thus nothing is done if machine representaiton is little endian
 *  (platform dependency module providing endianess information is used to determine the current case)
 *
 *  \param intv     Pointer to the integer value to encode or decode with correct endianess in place
 */
void SOPC_EncodeDecode_UInt32(uint32_t* uintv);

/**
 *  \brief Encode a signed 64 bits integer from machine endianess representation to binary UA encoding endianess representation.
 *  And respectively integer from UA binary to machine endianess representation.
 *  Note: UA binary representation is little endian thus nothing is done if machine representaiton is little endian
 *  (platform dependency module providing endianess information is used to determine the current case)
 *
 *  \param intv     Pointer to the integer value to encode or decode with correct endianess in place
 */
void SOPC_EncodeDecode_Int64(int64_t* intv);

/**
 *  \brief Encode an unsigned 64 bits integer from machine endianess representation to binary UA encoding endianess representation.
 *  And respectively integer from UA binary to machine endianess representation.
 *  Note: UA binary representation is little endian thus nothing is done if machine representaiton is little endian
 *  (platform dependency module providing endianess information is used to determine the current case)
 *
 *  \param intv     Pointer to the integer value to encode or decode with correct endianess in place
 */
void SOPC_EncodeDecode_UInt64(uint64_t* uintv);

/**
 *  \brief Encode a float from machine endianess representation to binary UA encoding endianess representation.
 *  And respectively float from UA binary to machine endianess representation.
 *  Note: UA binary representation is little endian thus nothing is done if machine representaiton is little endian
 *  (platform dependency module providing endianess information is used to determine the current case)
 *
 *  \param floatv     Pointer to the integer value to encode or decode with correct endianess in place
 */
void SOPC_EncodeDecode_Float(float* floatv);

/**
 *  \brief Encode a double from machine endianess representation to binary UA encoding endianess representation.
 *  And respectively decode a double from UA binary to machine endianess representation.
 *  Note: UA binary representation is little endian thus nothing is done if machine representaiton is little endian
 *  (platform dependency module providing endianess information is used to determine the current case)
 *
 *  \param doublev     Pointer to the integer value to encode or decode with correct endianess in place
 */
void SOPC_EncodeDecode_Double(double* doublev);

SOPC_StatusCode SOPC_Byte_Write(const SOPC_Byte* value, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode SOPC_Byte_Read(SOPC_Byte* value, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode SOPC_Boolean_Write(const SOPC_Boolean* value, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode SOPC_Boolean_Read(SOPC_Boolean* value, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode SOPC_SByte_Write(const SOPC_SByte* value, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode SOPC_SByte_Read(SOPC_SByte* value, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode SOPC_Int16_Write(const int16_t* value, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode SOPC_Int16_Read(int16_t* value, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode SOPC_UInt16_Write(const uint16_t* value, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode SOPC_UInt16_Read(uint16_t* value, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode SOPC_Int32_Write(const int32_t* value, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode SOPC_Int32_Read(int32_t* value, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode SOPC_UInt32_Write(const uint32_t* value, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode SOPC_UInt32_Read(uint32_t* value, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode SOPC_Int64_Write(const int64_t* value, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode SOPC_Int64_Read(int64_t* value, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode SOPC_UInt64_Write(const uint64_t* value, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode SOPC_UInt64_Read(uint64_t* value, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode SOPC_Float_Write(const float* value, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode SOPC_Float_Read(float* value, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode SOPC_Double_Write(const double* value, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode SOPC_Double_Read(double* value, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode SOPC_DateTime_Write(const SOPC_DateTime* date, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode SOPC_DateTime_Read(SOPC_DateTime* date, SOPC_MsgBuffer* msgBuffer);

SOPC_StatusCode SOPC_ByteString_Write(const SOPC_ByteString* str, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode SOPC_ByteString_Read(SOPC_ByteString* str, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode SOPC_String_Write(const SOPC_String* str, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode SOPC_String_Read(SOPC_String* str, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode SOPC_XmlElement_Write(const SOPC_XmlElement* xml, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode SOPC_XmlElement_Read(SOPC_XmlElement* xml, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode SOPC_Guid_Write(const SOPC_Guid* guid, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode SOPC_Guid_Read(SOPC_Guid* guid, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode SOPC_NodeId_Write(const SOPC_NodeId* nodeId, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode SOPC_NodeId_Read(SOPC_NodeId* nodeId, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode SOPC_ExpandedNodeId_Write(const SOPC_ExpandedNodeId* expNodeId, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode SOPC_ExpandedNodeId_Read(SOPC_ExpandedNodeId* expNodeId, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode SOPC_StatusCode_Write(const SOPC_StatusCode* status, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode SOPC_StatusCode_Read(SOPC_StatusCode* status, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode SOPC_DiagnosticInfo_Write(const SOPC_DiagnosticInfo* diagInfo, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode SOPC_DiagnosticInfo_Read(SOPC_DiagnosticInfo* diagInfo, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode SOPC_QualifiedName_Write(const SOPC_QualifiedName* qname, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode SOPC_QualifiedName_Read(SOPC_QualifiedName* qname, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode SOPC_LocalizedText_Write(const SOPC_LocalizedText* localizedText, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode SOPC_LocalizedText_Read(SOPC_LocalizedText* localizedText, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode SOPC_ExtensionObject_Write(const SOPC_ExtensionObject* extObj, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode SOPC_ExtensionObject_Read(SOPC_ExtensionObject* extObj, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode SOPC_Variant_Write(const SOPC_Variant* variant, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode SOPC_Variant_Read(SOPC_Variant* variant, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode SOPC_DataValue_Write(const SOPC_DataValue* dataValue, SOPC_MsgBuffer* msgBuffer);
SOPC_StatusCode SOPC_DataValue_Read(SOPC_DataValue* dataValue, SOPC_MsgBuffer* msgBuffer);


#endif /* SOPC_ENCODER_H_ */
