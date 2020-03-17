/*
 * Licensed to Systerel under one or more contributor license
 * agreements. See the NOTICE file distributed with this work
 * for additional information regarding copyright ownership.
 * Systerel licenses this file to you under the Apache
 * License, Version 2.0 (the "License"); you may not use this
 * file except in compliance with the License. You may obtain
 * a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#ifndef SOPC_ENCODER_H_
#define SOPC_ENCODER_H_

#include "sopc_buffer.h"
#include "sopc_builtintypes.h"
#include "sopc_encodeabletype.h"
#include "sopc_enums.h"

#define SWAP_2_BYTES(x) (uint16_t)(((x) & (uint16_t) 0x00FF) << 8 | ((x) & (uint16_t) 0xFF00) >> 8)
#define SWAP_3_BYTES(x) (((x) &0x0000FF) << 16 | ((x) &0x00FF00) | ((x) &0xFF0000) >> 16)
#define SWAP_4_BYTES(x) \
    (((x) &0x000000FF) << 24 | ((x) &0x0000FF00) << 8 | ((x) &0xFF000000) >> 24 | ((x) &0x00FF0000) >> 8)
#define SWAP_8_BYTES(x)                                                                                    \
    (((x) &0x00000000000000FF) << 56 | ((x) &0x000000000000FF00) << 40 | ((x) &0x0000000000FF0000) << 24 | \
     ((x) &0x00000000FF000000) << 8 | ((x) &0xFF00000000000000) >> 56 | ((x) &0x00FF000000000000) >> 40 |  \
     ((x) &0x0000FF0000000000) >> 24 | ((x) &0x000000FF00000000) >> 8)
#define SWAP_2_DWORDS(x) (((x) &0x00000000FFFFFFFF) << 32 | ((x) &0xFFFFFFFF00000000) >> 32)

typedef enum
{
    SOPC_NodeIdEncoding_TwoBytes = 0x00,
    SOPC_NodeIdEncoding_FourBytes = 0x01,
    SOPC_NodeIdEncoding_Numeric = 0x02,
    SOPC_NodeIdEncoding_String = 0x03,
    SOPC_NodeIdEncoding_Guid = 0x04,
    SOPC_NodeIdEncoding_ByteString = 0x05,
    SOPC_NodeIdEncoding_NamespaceUriFlag = 0x80,
    SOPC_NodeIdEncoding_ServerIndexFlag = 0x40,
    SOPC_NodeIdEncoding_Invalid = 0xFF
} SOPC_NodeId_DataEncoding;

typedef enum
{
    SOPC_DiagInfoEncoding_SymbolicId = 0x01,
    SOPC_DiagInfoEncoding_Namespace = 0x02,
    SOPC_DiagInfoEncoding_LocalizedTest = 0x04,
    SOPC_DiagInfoEncoding_Locale = 0x08,
    SOPC_DiagInfoEncoding_AdditionalInfo = 0x10,
    SOPC_DiagInfoEncoding_InnerStatusCode = 0x20,
    SOPC_DiagInfoEncoding_InnerDianosticInfo = 0x40,
} SOPC_DiagInfo_EncodingFlag;

typedef enum
{
    SOPC_LocalizedText_Locale = 0x01,
    SOPC_LocalizedText_Text = 0x02
} SOPC_LocalizedText_EncodingFlag;

typedef enum
{
    SOPC_DataValue_NotNullValue = 0x01,
    SOPC_DataValue_NotGoodStatusCode = 0x02,
    SOPC_DataValue_NotMinSourceDate = 0x04,
    SOPC_DataValue_NotMinServerDate = 0x08,
    SOPC_DataValue_NotZeroSourcePico = 0x10,
    SOPC_DataValue_NotZeroServerPico = 0x20
} SOPC_DataValue_EncodingFlag;

/**
 * \brief Encodes an signed 16 bits integer from machine endianness representation to binary UA encoding endianness
 * representation. Also decodes from UA binary to machine endianness representation.
 *
 * \note UA binary representation is little endian thus nothing is done if machine representation is little endian
 * (SOPC_Helper_Endianness_GetInteger() is used to determine the machine endianness)
 *
 *  \param intv     Pointer to the integer value to encode or decode with correct endianness in place
 */
void SOPC_EncodeDecode_Int16(int16_t* intv);

/**
 * \brief Encodes a unsigned 16 bits integer from machine endianness representation to binary UA encoding endianness
 * representation. Also decodes from UA binary to machine endianness representation.
 *
 * \note UA binary representation is little endian thus nothing is done if machine representation is little endian
 * (SOPC_Helper_Endianness_GetInteger() is used to determine the machine endianness)
 *
 *  \param uintv     Pointer to the integer value to encode or decode with correct endianness in place
 */
void SOPC_EncodeDecode_UInt16(uint16_t* uintv);

/**
 * \brief Encodes a signed 32 bits integer from machine endianness representation to binary UA encoding endianness
 * representation. Also decodes from UA binary to machine endianness representation.
 *
 * \note UA binary representation is little endian thus nothing is done if machine representation is little endian
 * (SOPC_Helper_Endianness_GetInteger() is used to determine the machine endianness)
 *
 *  \param intv     Pointer to the integer value to encode or decode with correct endianness in place
 */
void SOPC_EncodeDecode_Int32(int32_t* intv);

/**
 * \brief Encodes an unsigned 32 bits integer from machine endianness representation to binary UA encoding endianness
 * representation. Also decodes from UA binary to machine endianness representation.
 *
 * \note UA binary representation is little endian thus nothing is done if machine representation is little endian
 * (SOPC_Helper_Endianness_GetInteger() is used to determine the machine endianness)
 *
 *  \param uintv     Pointer to the integer value to encode or decode with correct endianness in place
 */
void SOPC_EncodeDecode_UInt32(uint32_t* uintv);

/**
 * \brief Encodes a signed 64 bits integer from machine endianness representation to binary UA encoding endianness
 * representation. Also decodes from UA binary to machine endianness representation.
 *
 * \note UA binary representation is little endian thus nothing is done if machine representation is little endian
 * (SOPC_Helper_Endianness_GetInteger() is used to determine the machine endianness)
 *
 *  \param intv     Pointer to the integer value to encode or decode with correct endianness in place
 */
void SOPC_EncodeDecode_Int64(int64_t* intv);

/**
 * \brief Encodes an unsigned 64 bits integer from machine endianness representation to binary UA encoding endianness
 * representation. Also decodes from UA binary to machine endianness representation.
 *
 * \note UA binary representation is little endian thus nothing is done if machine representation is little endian
 * (SOPC_Helper_Endianness_GetInteger() is used to determine the machine endianness)
 *
 *  \param uintv     Pointer to the integer value to encode or decode with correct endianness in place
 */
void SOPC_EncodeDecode_UInt64(uint64_t* uintv);

/**
 * \brief Encodes a float from machine endianness representation to binary UA encoding endianness representation.
 * Also decodes from UA binary to machine endianness representation.
 *
 * \note UA binary representation is little endian thus nothing is done if machine representation is little endian
 * (SOPC_Helper_Endianness_GetFloat() is used to determine the machine endianness)
 *
 * \param floatv     Pointer to the integer value to encode or decode with correct endianness in place
 */
void SOPC_EncodeDecode_Float(float* floatv);

/**
 * \brief Encodes a double from machine endianness representation to binary UA encoding endianness representation.
 * Also decodes from UA binary to machine endianness representation.
 *
 * \note UA binary representation is little endian thus nothing is done if machine representation is little endian
 * (SOPC_Helper_Endianness_GetFloat() is used to determine the machine endianness)
 *
 * \param doublev     Pointer to the integer value to encode or decode with correct endianness in place
 */
void SOPC_EncodeDecode_Double(double* doublev);

/**
 * Note: SOPC_*_Write operations return SOPC_STATUS_WOULD_BLOCK if not enough bytes available in buffer to write the
 * value
 */

SOPC_ReturnStatus SOPC_Byte_Write(const SOPC_Byte* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_Byte_WriteAux(const void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_Byte_Read(SOPC_Byte* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_Byte_ReadAux(void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_Boolean_Write(const SOPC_Boolean* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_Boolean_WriteAux(const void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_Boolean_Read(SOPC_Boolean* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_Boolean_ReadAux(void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_SByte_Write(const SOPC_SByte* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_SByte_WriteAux(const void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_SByte_Read(SOPC_SByte* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_SByte_ReadAux(void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_Int16_Write(const int16_t* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_Int16_WriteAux(const void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_Int16_Read(int16_t* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_Int16_ReadAux(void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_UInt16_Write(const uint16_t* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_UInt16_WriteAux(const void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_UInt16_Read(uint16_t* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_UInt16_ReadAux(void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_Int32_Write(const int32_t* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_Int32_WriteAux(const void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_Int32_Read(int32_t* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_Int32_ReadAux(void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_UInt32_Write(const uint32_t* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_UInt32_WriteAux(const void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_UInt32_Read(uint32_t* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_UInt32_ReadAux(void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_Int64_Write(const int64_t* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_Int64_WriteAux(const void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_Int64_Read(int64_t* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_Int64_ReadAux(void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_UInt64_Write(const uint64_t* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_UInt64_WriteAux(const void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_UInt64_Read(uint64_t* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_UInt64_ReadAux(void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_Float_Write(const float* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_Float_WriteAux(const void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_Float_Read(float* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_Float_ReadAux(void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_Double_Write(const double* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_Double_WriteAux(const void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_Double_Read(double* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_Double_ReadAux(void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_DateTime_Write(const SOPC_DateTime* date, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_DateTime_WriteAux(const void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_DateTime_Read(SOPC_DateTime* date, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_DateTime_ReadAux(void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);

SOPC_ReturnStatus SOPC_ByteString_Write(const SOPC_ByteString* str, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_ByteString_WriteAux(const void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_ByteString_Read(SOPC_ByteString* str, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_ByteString_ReadAux(void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_String_Write(const SOPC_String* str, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_String_WriteAux(const void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_String_Read(SOPC_String* str, SOPC_Buffer* buf, uint32_t nestedStructLevel);
// Read with a limited length, returns SOPC_WOULD_BLOCK if length is more than restricted
SOPC_ReturnStatus SOPC_String_ReadWithLimitedLength(SOPC_String* str,
                                                    int32_t maxLength,
                                                    SOPC_Buffer* buf,
                                                    uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_String_ReadAux(void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_XmlElement_Write(const SOPC_XmlElement* xml, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_XmlElement_WriteAux(const void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_XmlElement_Read(SOPC_XmlElement* xml, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_XmlElement_ReadAux(void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_Guid_Write(const SOPC_Guid* guid, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_Guid_WriteAux(const void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_Guid_Read(SOPC_Guid* guid, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_Guid_ReadAux(void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_NodeId_Write(const SOPC_NodeId* nodeId, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_NodeId_WriteAux(const void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_NodeId_Read(SOPC_NodeId* nodeId, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_NodeId_ReadAux(void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_ExpandedNodeId_Write(const SOPC_ExpandedNodeId* expNodeId,
                                            SOPC_Buffer* buf,
                                            uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_ExpandedNodeId_WriteAux(const void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_ExpandedNodeId_Read(SOPC_ExpandedNodeId* expNodeId,
                                           SOPC_Buffer* buf,
                                           uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_ExpandedNodeId_ReadAux(void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_StatusCode_Write(const SOPC_StatusCode* status, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_StatusCode_WriteAux(const void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_StatusCode_Read(SOPC_StatusCode* status, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_StatusCode_ReadAux(void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_DiagnosticInfo_Write(const SOPC_DiagnosticInfo* diagInfo,
                                            SOPC_Buffer* buf,
                                            uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_DiagnosticInfo_WriteAux(const void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_DiagnosticInfo_Read(SOPC_DiagnosticInfo* diagInfo, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_DiagnosticInfo_ReadAux(void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_QualifiedName_Write(const SOPC_QualifiedName* qname,
                                           SOPC_Buffer* buf,
                                           uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_QualifiedName_WriteAux(const void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_QualifiedName_Read(SOPC_QualifiedName* qname, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_QualifiedName_ReadAux(void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
// Note: write only the defaultLocale and defaultText fields (should be the ones to send)
SOPC_ReturnStatus SOPC_LocalizedText_Write(const SOPC_LocalizedText* localizedText,
                                           SOPC_Buffer* buf,
                                           uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_LocalizedText_WriteAux(const void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
// Note: set only the defaultLocale and defaultText fields since unique value received
SOPC_ReturnStatus SOPC_LocalizedText_Read(SOPC_LocalizedText* localizedText,
                                          SOPC_Buffer* buf,
                                          uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_LocalizedText_ReadAux(void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_ExtensionObject_Write(const SOPC_ExtensionObject* extObj,
                                             SOPC_Buffer* buf,
                                             uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_ExtensionObject_WriteAux(const void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_ExtensionObject_Read(SOPC_ExtensionObject* extObj, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_ExtensionObject_ReadAux(void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_Variant_Write(const SOPC_Variant* variant, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_Variant_WriteAux(const void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_Variant_WriteAux_Nested(const void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_Variant_Read(SOPC_Variant* variant, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_Variant_ReadAux(void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_Variant_ReadAux_Nested(void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_DataValue_Write(const SOPC_DataValue* dataValue, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_DataValue_WriteAux(const void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_DataValue_WriteAux_Nested(const void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_DataValue_Read(SOPC_DataValue* dataValue, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_DataValue_ReadAux(void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_DataValue_ReadAux_Nested(void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel);

SOPC_ReturnStatus SOPC_Read_Array(SOPC_Buffer* msgBuf,
                                  int32_t* noOfElts,
                                  void** eltsArray,
                                  size_t sizeOfElt,
                                  SOPC_EncodeableObject_PfnDecode* decodeFct,
                                  SOPC_EncodeableObject_PfnInitialize* initializeFct,
                                  SOPC_EncodeableObject_PfnClear* clearFct,
                                  uint32_t nestedStructLevel);
SOPC_ReturnStatus SOPC_Write_Array(SOPC_Buffer* msgBuf,
                                   const int32_t* const noOfElts,
                                   const void* const* eltsArray,
                                   size_t sizeOfElt,
                                   SOPC_EncodeableObject_PfnEncode* encodeFct,
                                   uint32_t nestedStructLevel);

// Encode msg body type as node id + msg header + msg body content from encodeable type and corresponding structure
SOPC_ReturnStatus SOPC_EncodeMsg_Type_Header_Body(SOPC_Buffer* buf,
                                                  SOPC_EncodeableType* encType,
                                                  SOPC_EncodeableType* headerType,
                                                  void* msgHeader,
                                                  void* msgBody);

// Read the msg body type from buffer and return the encodeable type of the msg body
SOPC_ReturnStatus SOPC_MsgBodyType_Read(SOPC_Buffer* buf, SOPC_EncodeableType** receivedEncType);

SOPC_ReturnStatus SOPC_DecodeMsg_HeaderOrBody(SOPC_Buffer* buffer,
                                              SOPC_EncodeableType* msgEncType,
                                              void** encodeableObj);

/**
 *  \brief Generic built-in type encoding. Provides all the generic service
 *  functions for encoding and decoding a built-in type.
 */
typedef struct SOPC_BuiltInType_Encoding
{
    SOPC_EncodeableObject_PfnEncode* encode;
    SOPC_EncodeableObject_PfnDecode* decode;
} SOPC_BuiltInType_Encoding;

/**
 *  \brief Table of all built-in type encoding. To be indexed with a \c
 *  SOPC_BuiltinId.  Provides all the generic encoding and decoding functions
 *  associated with each built-in type.
 */
extern const SOPC_BuiltInType_Encoding SOPC_BuiltInType_EncodingTable[SOPC_BUILTINID_MAX + 1];

#endif /* SOPC_ENCODER_H_ */
