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

#ifndef SOPC_TYPES_WRAPPER_H_
#define SOPC_TYPES_WRAPPER_H_

#include <stdlib.h>
#include <stdint.h>

#include "opcua_statuscodes.h"
#include "sopc_sockets.h"
#include "sopc_builtintypes.h"
#include "sopc_encodeable.h"
#include "sopc_types.h"

#define OPCUA_BEGIN_EXTERN_C BEGIN_EXTERN_C
#define OPCUA_END_EXTERN_C END_EXTERN_C

BEGIN_EXTERN_C

typedef SOPC_Byte Byte;
typedef SOPC_SByte SByte;
typedef SOPC_Boolean Boolean;
typedef uint16_t UInt16;
typedef int16_t Int16;
typedef uint32_t UInt32;
typedef int32_t Int32;
typedef uint64_t UInt64;
typedef int64_t Int64;
typedef float Float;
typedef double Double;
typedef SOPC_String String;
typedef SOPC_ByteString ByteString;
typedef SOPC_XmlElement XmlElement;
typedef SOPC_DateTime DateTime;
typedef SOPC_NodeId NodeId;
typedef SOPC_ExpandedNodeId ExpandedNodeId;
typedef SOPC_DiagnosticInfo DiagnosticInfo;
typedef SOPC_ExtensionObject ExtensionObject;
typedef SOPC_Variant Variant;
typedef SOPC_DataValue DataValue;

typedef int OpcUa_Int;
typedef char* OpcUa_StringA;
typedef char  OpcUa_CharA;
typedef void* OpcUa_Handle;
typedef void* OpcUa_SocketManager;

#define OpcUa_StatusCode SOPC_StatusCode
#define uStatus status
#define OpcUa_Good STATUS_OK
#define OpcUa_Bad STATUS_NOK
#define OpcUa_Void void
#define OpcUa_BuiltInType SOPC_BuiltinId
#define OpcUa_Byte SOPC_Byte
#define OpcUa_SByte SOPC_SByte
#define OpcUa_Boolean SOPC_Boolean
#define OpcUa_Int16 int16_t
#define OpcUa_UInt16 uint16_t
#define OpcUa_Int32 int32_t
#define OpcUa_UInt32 uint32_t
#define OpcUa_Int64 int64_t
#define OpcUa_UInt64 uint64_t
#define OpcUa_Float float
#define OpcUa_Double double
#define OpcUa_ByteString SOPC_ByteString
#define OpcUa_String SOPC_String
#define OpcUa_XmlElement SOPC_XmlElement
#define OpcUa_Guid SOPC_Guid
#define OpcUa_IdentifierType SOPC_IdentifierType
#define OpcUa_NodeId SOPC_NodeId
#define OpcUa_ExpandedNodeId SOPC_ExpandedNodeId
#define OpcUa_DiagnosticInfo SOPC_DiagnosticInfo
#define OpcUa_QualifiedName SOPC_QualifiedName
#define OpcUa_LocalizedText SOPC_LocalizedText
#define OpcUa_ExtensionObject SOPC_ExtensionObject
#define OpcUa_Variant SOPC_Variant
#define OpcUa_DataValue SOPC_DataValue
#define OpcUa_Null NULL

#define OpcUa_Encoder SOPC_MsgBuffer
#define OpcUa_Decoder SOPC_MsgBuffer
#define a_pEncoder msgBuf
#define a_pDecoder msgBuf

#define OpcUa_True 1
#define OpcUa_False 0
#define OPCUA_INFINITE UINT32_MAX

#define OpcUa_SecurityPolicy_None           "http://opcfoundation.org/UA/SecurityPolicy#None"
#define OpcUa_SecurityPolicy_Basic128       "http://opcfoundation.org/UA/SecurityPolicy#Basic128"
#define OpcUa_SecurityPolicy_Basic128Rsa15  "http://opcfoundation.org/UA/SecurityPolicy#Basic128Rsa15"
#define OpcUa_SecurityPolicy_Basic192       "http://opcfoundation.org/UA/SecurityPolicy#Basic192"
#define OpcUa_SecurityPolicy_Basic192Rsa15  "http://opcfoundation.org/UA/SecurityPolicy#Basic192Rsa15"
#define OpcUa_SecurityPolicy_Basic256       "http://opcfoundation.org/UA/SecurityPolicy#Basic256"
#define OpcUa_SecurityPolicy_Basic256Rsa15  "http://opcfoundation.org/UA/SecurityPolicy#Basic256Rsa15"
#define OpcUa_SecurityPolicy_Basic256Sha256 "http://opcfoundation.org/UA/SecurityPolicy#Basic256Sha256"

#define OpcUa_EncodeableType SOPC_EncodeableType
#define _OpcUa_EncodeableType SOPC_EncodeableType
#define OpcUa_EncodeableObject_PfnInitialize SOPC_EncodeableObject_PfnInitialize
#define OpcUa_EncodeableObject_PfnClear SOPC_EncodeableObject_PfnClear
#define OpcUa_EncodeableObject_PfnGetSize SOPC_EncodeableObject_PfnGetSize
#define OpcUa_EncodeableObject_PfnEncode SOPC_EncodeableObject_PfnEncode
#define OpcUa_EncodeableObject_PfnDecode SOPC_EncodeableObject_PfnDecode

#define OpcUa_ByteString_Initialize SOPC_ByteString_Initialize
#define OpcUa_ByteString_Clear SOPC_ByteString_Clear

#define OpcUa_Free free
#define OpcUa_MemSet memset
#define OpcUa_Alloc malloc

#define OpcUa_InitializeStatus(xModule, xName) \
    SOPC_StatusCode status = STATUS_OK

#define OpcUa_GotoErrorIfBad(xStatus) \
    if(((xStatus) & 0x80000000) != 0) \
        goto Error;

#define OpcUa_ReturnStatusCode return status
#define OpcUa_BeginErrorHandling Error: 
#define OpcUa_FinishErrorHandling OpcUa_ReturnStatusCode
#define OpcUa_ReturnErrorIfArgumentNull(xValue) \
    if(xValue == NULL) \
        return OpcUa_BadInvalidArgument
       
#define OpcUa_Field_Initialize(xType, xName) \
    xType##_Initialize(&a_pValue->xName)


#define OpcUa_Field_InitializeArray(xType, xName) \
    SOPC_Initialize_Array(&a_pValue->NoOf##xName, (void**) &a_pValue->xName, \
                        sizeof(xType), (SOPC_EncodeableObject_PfnInitialize*) xType##_Initialize)


#define OpcUa_Field_InitializeEncodeableArray(xType, xName) OpcUa_Field_InitializeArray(xType, xName)
#define OpcUa_Field_InitializeEnumeratedArray(xType, xName) \
    SOPC_Initialize_Array(&a_pValue->NoOf##xName, (void**) &a_pValue->xName, \
                        sizeof(xType), (SOPC_EncodeableObject_PfnInitialize*) SOPC_Initialize_EnumeratedType)

#define OpcUa_Field_InitializeEncodeable(xType, xName) \
    xType##_Initialize(&a_pValue->xName)

#define OpcUa_Field_InitializeEnumerated(xType, xName) \
    SOPC_Initialize_EnumeratedType((int32_t*) &a_pValue->xName)

#define OpcUa_Field_Clear(xType, xName) \
    xType##_Clear(&a_pValue->xName)

#define OpcUa_Field_ClearArray(xType, xName) \
    SOPC_Clear_Array(&a_pValue->NoOf##xName, (void**) &a_pValue->xName, \
                   sizeof(xType), (SOPC_EncodeableObject_PfnClear*) xType##_Clear)

#define OpcUa_Field_ClearEncodeableArray(xType, xName) OpcUa_Field_ClearArray(xType, xName)
#define OpcUa_Field_ClearEnumeratedArray(xType, xName) \
    SOPC_Initialize_Array(&a_pValue->NoOf##xName, (void**) &a_pValue->xName, \
                        sizeof(xType), (SOPC_EncodeableObject_PfnInitialize*) SOPC_Clear_EnumeratedType)

#define OpcUa_Field_ClearEncodeable(xType, xName) \
    xType##_Clear(&a_pValue->xName)

#define OpcUa_Field_ClearEnumerated(xType, xName) \
    SOPC_Clear_EnumeratedType((int32_t*) &a_pValue->xName)

#define OpcUa_Field_Write(xType, xName) \
    if(STATUS_OK == status) status = xType##_Write(&a_pValue->xName, msgBuf)

#define OpcUa_Field_WriteArray(xType, xName) \
    if(STATUS_OK == status) status = SOPC_Write_Array(msgBuf, &a_pValue->NoOf##xName, (void**) &a_pValue->xName, \
                                                      sizeof(xType), (SOPC_EncodeableObject_PfnEncode*) xType##_Write)

#define OpcUa_Field_WriteEncodeableArray(xType, xName) \
    if(STATUS_OK == status) status = SOPC_Write_Array(msgBuf, &a_pValue->NoOf##xName, (void**) &a_pValue->xName, \
                                                      sizeof(xType), (SOPC_EncodeableObject_PfnEncode*) xType##_Encode)

#define OpcUa_Field_WriteEnumeratedArray(xType, xName) \
    if(STATUS_OK == status) status = SOPC_Write_Array(msgBuf, &a_pValue->NoOf##xName, (void**) &a_pValue->xName, \
                                                      sizeof(xType), (SOPC_EncodeableObject_PfnEncode*) SOPC_Write_EnumeratedType)

#define OpcUa_Field_WriteEncodeable(xType, xName) \
    if(STATUS_OK == status) status = xType##_Encode(&a_pValue->xName, msgBuf)

#define OpcUa_Field_WriteEnumerated(xType, xName) \
    if(STATUS_OK == status) status = SOPC_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->xName)

#define OpcUa_Field_Read(xType, xName) \
    if(STATUS_OK == status) status = xType##_Read(&a_pValue->xName, msgBuf)

#define OpcUa_Field_ReadArray(xType, xName) \
    if(STATUS_OK == status) status = SOPC_Read_Array(msgBuf, &a_pValue->NoOf##xName, (void**) &a_pValue->xName, \
                                                     sizeof(xType), (SOPC_EncodeableObject_PfnDecode*) xType##_Read)

#define OpcUa_Field_ReadEncodeableArray(xType, xName) \
    if(STATUS_OK == status) status = SOPC_Read_Array(msgBuf, &a_pValue->NoOf##xName, (void**) &a_pValue->xName, \
                                                     sizeof(xType), (SOPC_EncodeableObject_PfnDecode*) xType##_Decode)

#define OpcUa_Field_ReadEnumeratedArray(xType, xName) \
    if(STATUS_OK == status) status = SOPC_Read_Array(msgBuf, &a_pValue->NoOf##xName, (void**) &a_pValue->xName, \
                                                     sizeof(xType), (SOPC_EncodeableObject_PfnDecode*) SOPC_Read_EnumeratedType)

#define OpcUa_Field_ReadEncodeable(xType, xName) \
    if(STATUS_OK == status) status = xType##_Decode(&a_pValue->xName, msgBuf)

#define OpcUa_Field_ReadEnumerated(xType, xName) \
    if(STATUS_OK == status) status = SOPC_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->xName)

#define OpcUa_Field_GetSize(xType, xName)
#define OpcUa_Field_GetSizeArray(xType, xName)
#define OpcUa_Field_GetSizeEncodeableArray(xType, xName)
#define OpcUa_Field_GetSizeEnumeratedArray(xType, xName)
#define OpcUa_Field_GetSizeEncodeable(xType, xName)
#define OpcUa_Field_GetSizeEnumerated(xType, xName)

#define OpcUa_DeclareErrorTraceModule(xModule)

END_EXTERN_C

#endif // SOPC_TYPES_WRAPPER_H_
