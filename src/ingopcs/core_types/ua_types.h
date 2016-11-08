/* ========================================================================
 * Copyright (c) 2005-2016 The OPC Foundation, Inc. All rights reserved.
 *
 * OPC Foundation MIT License 1.00
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * The complete license agreement can be found here:
 * http://opcfoundation.org/License/MIT/1.00/
 * ======================================================================*/


#ifndef _UA_Types_H_
#define _UA_Types_H_ 1

//#ifndef OPCUA_FORCE_INT32_ENUMS
//# error OPCUA_FORCE_INT32_ENUMS must be defined!
//#endif /* OPCUA_FORCE_INT32_ENUMS */

#include <platform_deps.h>
#include <ua_msg_buffer.h>
#include <ua_builtintypes.h>
#include <ua_encodeable.h>

BEGIN_EXTERN_C

#ifndef OPCUA_EXCLUDE_IdType
/*============================================================================
 * The IdType enumeration.
 *===========================================================================*/
typedef enum _OpcUa_IdType
{
    OpcUa_IdType_Numeric = 0,
    OpcUa_IdType_String  = 1,
    OpcUa_IdType_Guid    = 2,
    OpcUa_IdType_Opaque  = 3
#if OPCUA_FORCE_INT32_ENUMS
    ,_OpcUa_IdType_MaxEnumerationValue = OpcUa_Int32_Max
#endif
}
OpcUa_IdType;

#endif

#ifndef OPCUA_EXCLUDE_NodeClass
/*============================================================================
 * The NodeClass enumeration.
 *===========================================================================*/
typedef enum _OpcUa_NodeClass
{
    OpcUa_NodeClass_Unspecified   = 0,
    OpcUa_NodeClass_Object        = 1,
    OpcUa_NodeClass_Variable      = 2,
    OpcUa_NodeClass_Method        = 4,
    OpcUa_NodeClass_ObjectType    = 8,
    OpcUa_NodeClass_VariableType  = 16,
    OpcUa_NodeClass_ReferenceType = 32,
    OpcUa_NodeClass_DataType      = 64,
    OpcUa_NodeClass_View          = 128
#if OPCUA_FORCE_INT32_ENUMS
    ,_OpcUa_NodeClass_MaxEnumerationValue = OpcUa_Int32_Max
#endif
}
OpcUa_NodeClass;

#endif

#ifndef OPCUA_EXCLUDE_ReferenceNode
/*============================================================================
 * The ReferenceNode structure.
 *===========================================================================*/
typedef struct _OpcUa_ReferenceNode
{
    UA_NodeId         ReferenceTypeId;
    UA_Boolean        IsInverse;
    UA_ExpandedNodeId TargetId;
}
OpcUa_ReferenceNode;

void OpcUa_ReferenceNode_Initialize(OpcUa_ReferenceNode* pValue);

void OpcUa_ReferenceNode_Clear(OpcUa_ReferenceNode* pValue);

//StatusCode OpcUa_ReferenceNode_GetSize(OpcUa_ReferenceNode* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_ReferenceNode_Encode(OpcUa_ReferenceNode* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ReferenceNode_Decode(OpcUa_ReferenceNode* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_ReferenceNode_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_Node
/*============================================================================
 * The Node structure.
 *===========================================================================*/
typedef struct _OpcUa_Node
{
    UA_NodeId            NodeId;
    OpcUa_NodeClass      NodeClass;
    UA_QualifiedName     BrowseName;
    UA_LocalizedText     DisplayName;
    UA_LocalizedText     Description;
    uint32_t             WriteMask;
    uint32_t             UserWriteMask;
    int32_t              NoOfReferences;
    OpcUa_ReferenceNode* References;
}
OpcUa_Node;

void OpcUa_Node_Initialize(OpcUa_Node* pValue);

void OpcUa_Node_Clear(OpcUa_Node* pValue);

//StatusCode OpcUa_Node_GetSize(OpcUa_Node* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_Node_Encode(OpcUa_Node* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_Node_Decode(OpcUa_Node* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_Node_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_InstanceNode
/*============================================================================
 * The InstanceNode structure.
 *===========================================================================*/
typedef struct _OpcUa_InstanceNode
{
    UA_NodeId            NodeId;
    OpcUa_NodeClass      NodeClass;
    UA_QualifiedName     BrowseName;
    UA_LocalizedText     DisplayName;
    UA_LocalizedText     Description;
    uint32_t             WriteMask;
    uint32_t             UserWriteMask;
    int32_t              NoOfReferences;
    OpcUa_ReferenceNode* References;
}
OpcUa_InstanceNode;

void OpcUa_InstanceNode_Initialize(OpcUa_InstanceNode* pValue);

void OpcUa_InstanceNode_Clear(OpcUa_InstanceNode* pValue);

//StatusCode OpcUa_InstanceNode_GetSize(OpcUa_InstanceNode* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_InstanceNode_Encode(OpcUa_InstanceNode* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_InstanceNode_Decode(OpcUa_InstanceNode* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_InstanceNode_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_TypeNode
/*============================================================================
 * The TypeNode structure.
 *===========================================================================*/
typedef struct _OpcUa_TypeNode
{
    UA_NodeId            NodeId;
    OpcUa_NodeClass      NodeClass;
    UA_QualifiedName     BrowseName;
    UA_LocalizedText     DisplayName;
    UA_LocalizedText     Description;
    uint32_t             WriteMask;
    uint32_t             UserWriteMask;
    int32_t              NoOfReferences;
    OpcUa_ReferenceNode* References;
}
OpcUa_TypeNode;

void OpcUa_TypeNode_Initialize(OpcUa_TypeNode* pValue);

void OpcUa_TypeNode_Clear(OpcUa_TypeNode* pValue);

//StatusCode OpcUa_TypeNode_GetSize(OpcUa_TypeNode* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_TypeNode_Encode(OpcUa_TypeNode* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_TypeNode_Decode(OpcUa_TypeNode* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_TypeNode_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ObjectNode
/*============================================================================
 * The ObjectNode structure.
 *===========================================================================*/
typedef struct _OpcUa_ObjectNode
{
    UA_NodeId            NodeId;
    OpcUa_NodeClass      NodeClass;
    UA_QualifiedName     BrowseName;
    UA_LocalizedText     DisplayName;
    UA_LocalizedText     Description;
    uint32_t             WriteMask;
    uint32_t             UserWriteMask;
    int32_t              NoOfReferences;
    OpcUa_ReferenceNode* References;
    UA_Byte              EventNotifier;
}
OpcUa_ObjectNode;

void OpcUa_ObjectNode_Initialize(OpcUa_ObjectNode* pValue);

void OpcUa_ObjectNode_Clear(OpcUa_ObjectNode* pValue);

//StatusCode OpcUa_ObjectNode_GetSize(OpcUa_ObjectNode* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_ObjectNode_Encode(OpcUa_ObjectNode* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ObjectNode_Decode(OpcUa_ObjectNode* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_ObjectNode_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ObjectTypeNode
/*============================================================================
 * The ObjectTypeNode structure.
 *===========================================================================*/
typedef struct _OpcUa_ObjectTypeNode
{
    UA_NodeId            NodeId;
    OpcUa_NodeClass      NodeClass;
    UA_QualifiedName     BrowseName;
    UA_LocalizedText     DisplayName;
    UA_LocalizedText     Description;
    uint32_t             WriteMask;
    uint32_t             UserWriteMask;
    int32_t              NoOfReferences;
    OpcUa_ReferenceNode* References;
    UA_Boolean           IsAbstract;
}
OpcUa_ObjectTypeNode;

void OpcUa_ObjectTypeNode_Initialize(OpcUa_ObjectTypeNode* pValue);

void OpcUa_ObjectTypeNode_Clear(OpcUa_ObjectTypeNode* pValue);

//StatusCode OpcUa_ObjectTypeNode_GetSize(OpcUa_ObjectTypeNode* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_ObjectTypeNode_Encode(OpcUa_ObjectTypeNode* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ObjectTypeNode_Decode(OpcUa_ObjectTypeNode* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_ObjectTypeNode_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_VariableNode
/*============================================================================
 * The VariableNode structure.
 *===========================================================================*/
typedef struct _OpcUa_VariableNode
{
    UA_NodeId            NodeId;
    OpcUa_NodeClass      NodeClass;
    UA_QualifiedName     BrowseName;
    UA_LocalizedText     DisplayName;
    UA_LocalizedText     Description;
    uint32_t             WriteMask;
    uint32_t             UserWriteMask;
    int32_t              NoOfReferences;
    OpcUa_ReferenceNode* References;
    UA_Variant           Value;
    UA_NodeId            DataType;
    int32_t              ValueRank;
    int32_t              NoOfArrayDimensions;
    uint32_t*            ArrayDimensions;
    UA_Byte              AccessLevel;
    UA_Byte              UserAccessLevel;
    double               MinimumSamplingInterval;
    UA_Boolean           Historizing;
}
OpcUa_VariableNode;

void OpcUa_VariableNode_Initialize(OpcUa_VariableNode* pValue);

void OpcUa_VariableNode_Clear(OpcUa_VariableNode* pValue);

//StatusCode OpcUa_VariableNode_GetSize(OpcUa_VariableNode* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_VariableNode_Encode(OpcUa_VariableNode* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_VariableNode_Decode(OpcUa_VariableNode* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_VariableNode_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_VariableTypeNode
/*============================================================================
 * The VariableTypeNode structure.
 *===========================================================================*/
typedef struct _OpcUa_VariableTypeNode
{
    UA_NodeId            NodeId;
    OpcUa_NodeClass      NodeClass;
    UA_QualifiedName     BrowseName;
    UA_LocalizedText     DisplayName;
    UA_LocalizedText     Description;
    uint32_t             WriteMask;
    uint32_t             UserWriteMask;
    int32_t              NoOfReferences;
    OpcUa_ReferenceNode* References;
    UA_Variant           Value;
    UA_NodeId            DataType;
    int32_t              ValueRank;
    int32_t              NoOfArrayDimensions;
    uint32_t*            ArrayDimensions;
    UA_Boolean           IsAbstract;
}
OpcUa_VariableTypeNode;

void OpcUa_VariableTypeNode_Initialize(OpcUa_VariableTypeNode* pValue);

void OpcUa_VariableTypeNode_Clear(OpcUa_VariableTypeNode* pValue);

//StatusCode OpcUa_VariableTypeNode_GetSize(OpcUa_VariableTypeNode* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_VariableTypeNode_Encode(OpcUa_VariableTypeNode* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_VariableTypeNode_Decode(OpcUa_VariableTypeNode* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_VariableTypeNode_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ReferenceTypeNode
/*============================================================================
 * The ReferenceTypeNode structure.
 *===========================================================================*/
typedef struct _OpcUa_ReferenceTypeNode
{
    UA_NodeId            NodeId;
    OpcUa_NodeClass      NodeClass;
    UA_QualifiedName     BrowseName;
    UA_LocalizedText     DisplayName;
    UA_LocalizedText     Description;
    uint32_t             WriteMask;
    uint32_t             UserWriteMask;
    int32_t              NoOfReferences;
    OpcUa_ReferenceNode* References;
    UA_Boolean           IsAbstract;
    UA_Boolean           Symmetric;
    UA_LocalizedText     InverseName;
}
OpcUa_ReferenceTypeNode;

void OpcUa_ReferenceTypeNode_Initialize(OpcUa_ReferenceTypeNode* pValue);

void OpcUa_ReferenceTypeNode_Clear(OpcUa_ReferenceTypeNode* pValue);

//StatusCode OpcUa_ReferenceTypeNode_GetSize(OpcUa_ReferenceTypeNode* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_ReferenceTypeNode_Encode(OpcUa_ReferenceTypeNode* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ReferenceTypeNode_Decode(OpcUa_ReferenceTypeNode* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_ReferenceTypeNode_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_MethodNode
/*============================================================================
 * The MethodNode structure.
 *===========================================================================*/
typedef struct _OpcUa_MethodNode
{
    UA_NodeId            NodeId;
    OpcUa_NodeClass      NodeClass;
    UA_QualifiedName     BrowseName;
    UA_LocalizedText     DisplayName;
    UA_LocalizedText     Description;
    uint32_t             WriteMask;
    uint32_t             UserWriteMask;
    int32_t              NoOfReferences;
    OpcUa_ReferenceNode* References;
    UA_Boolean           Executable;
    UA_Boolean           UserExecutable;
}
OpcUa_MethodNode;

void OpcUa_MethodNode_Initialize(OpcUa_MethodNode* pValue);

void OpcUa_MethodNode_Clear(OpcUa_MethodNode* pValue);

//StatusCode OpcUa_MethodNode_GetSize(OpcUa_MethodNode* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_MethodNode_Encode(OpcUa_MethodNode* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_MethodNode_Decode(OpcUa_MethodNode* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_MethodNode_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ViewNode
/*============================================================================
 * The ViewNode structure.
 *===========================================================================*/
typedef struct _OpcUa_ViewNode
{
    UA_NodeId            NodeId;
    OpcUa_NodeClass      NodeClass;
    UA_QualifiedName     BrowseName;
    UA_LocalizedText     DisplayName;
    UA_LocalizedText     Description;
    uint32_t             WriteMask;
    uint32_t             UserWriteMask;
    int32_t              NoOfReferences;
    OpcUa_ReferenceNode* References;
    UA_Boolean           ContainsNoLoops;
    UA_Byte              EventNotifier;
}
OpcUa_ViewNode;

void OpcUa_ViewNode_Initialize(OpcUa_ViewNode* pValue);

void OpcUa_ViewNode_Clear(OpcUa_ViewNode* pValue);

//StatusCode OpcUa_ViewNode_GetSize(OpcUa_ViewNode* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_ViewNode_Encode(OpcUa_ViewNode* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ViewNode_Decode(OpcUa_ViewNode* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_ViewNode_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_DataTypeNode
/*============================================================================
 * The DataTypeNode structure.
 *===========================================================================*/
typedef struct _OpcUa_DataTypeNode
{
    UA_NodeId            NodeId;
    OpcUa_NodeClass      NodeClass;
    UA_QualifiedName     BrowseName;
    UA_LocalizedText     DisplayName;
    UA_LocalizedText     Description;
    uint32_t             WriteMask;
    uint32_t             UserWriteMask;
    int32_t              NoOfReferences;
    OpcUa_ReferenceNode* References;
    UA_Boolean           IsAbstract;
}
OpcUa_DataTypeNode;

void OpcUa_DataTypeNode_Initialize(OpcUa_DataTypeNode* pValue);

void OpcUa_DataTypeNode_Clear(OpcUa_DataTypeNode* pValue);

//StatusCode OpcUa_DataTypeNode_GetSize(OpcUa_DataTypeNode* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_DataTypeNode_Encode(OpcUa_DataTypeNode* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_DataTypeNode_Decode(OpcUa_DataTypeNode* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_DataTypeNode_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_Argument
/*============================================================================
 * The Argument structure.
 *===========================================================================*/
typedef struct _OpcUa_Argument
{
    UA_String        Name;
    UA_NodeId        DataType;
    int32_t          ValueRank;
    int32_t          NoOfArrayDimensions;
    uint32_t*        ArrayDimensions;
    UA_LocalizedText Description;
}
OpcUa_Argument;

void OpcUa_Argument_Initialize(OpcUa_Argument* pValue);

void OpcUa_Argument_Clear(OpcUa_Argument* pValue);

//StatusCode OpcUa_Argument_GetSize(OpcUa_Argument* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_Argument_Encode(OpcUa_Argument* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_Argument_Decode(OpcUa_Argument* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_Argument_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_EnumValueType
/*============================================================================
 * The EnumValueType structure.
 *===========================================================================*/
typedef struct _OpcUa_EnumValueType
{
    int64_t          Value;
    UA_LocalizedText DisplayName;
    UA_LocalizedText Description;
}
OpcUa_EnumValueType;

void OpcUa_EnumValueType_Initialize(OpcUa_EnumValueType* pValue);

void OpcUa_EnumValueType_Clear(OpcUa_EnumValueType* pValue);

//StatusCode OpcUa_EnumValueType_GetSize(OpcUa_EnumValueType* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_EnumValueType_Encode(OpcUa_EnumValueType* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_EnumValueType_Decode(OpcUa_EnumValueType* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_EnumValueType_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_EnumField
/*============================================================================
 * The EnumField structure.
 *===========================================================================*/
typedef struct _OpcUa_EnumField
{
    int64_t          Value;
    UA_LocalizedText DisplayName;
    UA_LocalizedText Description;
    UA_String        Name;
}
OpcUa_EnumField;

void OpcUa_EnumField_Initialize(OpcUa_EnumField* pValue);

void OpcUa_EnumField_Clear(OpcUa_EnumField* pValue);

//StatusCode OpcUa_EnumField_GetSize(OpcUa_EnumField* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_EnumField_Encode(OpcUa_EnumField* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_EnumField_Decode(OpcUa_EnumField* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_EnumField_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_OptionSet
/*============================================================================
 * The OptionSet structure.
 *===========================================================================*/
typedef struct _OpcUa_OptionSet
{
    UA_ByteString Value;
    UA_ByteString ValidBits;
}
OpcUa_OptionSet;

void OpcUa_OptionSet_Initialize(OpcUa_OptionSet* pValue);

void OpcUa_OptionSet_Clear(OpcUa_OptionSet* pValue);

//StatusCode OpcUa_OptionSet_GetSize(OpcUa_OptionSet* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_OptionSet_Encode(OpcUa_OptionSet* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_OptionSet_Decode(OpcUa_OptionSet* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_OptionSet_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_TimeZoneDataType
/*============================================================================
 * The TimeZoneDataType structure.
 *===========================================================================*/
typedef struct _OpcUa_TimeZoneDataType
{
    int16_t    Offset;
    UA_Boolean DaylightSavingInOffset;
}
OpcUa_TimeZoneDataType;

void OpcUa_TimeZoneDataType_Initialize(OpcUa_TimeZoneDataType* pValue);

void OpcUa_TimeZoneDataType_Clear(OpcUa_TimeZoneDataType* pValue);

//StatusCode OpcUa_TimeZoneDataType_GetSize(OpcUa_TimeZoneDataType* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_TimeZoneDataType_Encode(OpcUa_TimeZoneDataType* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_TimeZoneDataType_Decode(OpcUa_TimeZoneDataType* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_TimeZoneDataType_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ApplicationType
/*============================================================================
 * The ApplicationType enumeration.
 *===========================================================================*/
typedef enum _OpcUa_ApplicationType
{
    OpcUa_ApplicationType_Server          = 0,
    OpcUa_ApplicationType_Client          = 1,
    OpcUa_ApplicationType_ClientAndServer = 2,
    OpcUa_ApplicationType_DiscoveryServer = 3
#if OPCUA_FORCE_INT32_ENUMS
    ,_OpcUa_ApplicationType_MaxEnumerationValue = OpcUa_Int32_Max
#endif
}
OpcUa_ApplicationType;

#endif

#ifndef OPCUA_EXCLUDE_ApplicationDescription
/*============================================================================
 * The ApplicationDescription structure.
 *===========================================================================*/
typedef struct _OpcUa_ApplicationDescription
{
    UA_String             ApplicationUri;
    UA_String             ProductUri;
    UA_LocalizedText      ApplicationName;
    OpcUa_ApplicationType ApplicationType;
    UA_String             GatewayServerUri;
    UA_String             DiscoveryProfileUri;
    int32_t               NoOfDiscoveryUrls;
    UA_String*            DiscoveryUrls;
}
OpcUa_ApplicationDescription;

void OpcUa_ApplicationDescription_Initialize(OpcUa_ApplicationDescription* pValue);

void OpcUa_ApplicationDescription_Clear(OpcUa_ApplicationDescription* pValue);

//StatusCode OpcUa_ApplicationDescription_GetSize(OpcUa_ApplicationDescription* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_ApplicationDescription_Encode(OpcUa_ApplicationDescription* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ApplicationDescription_Decode(OpcUa_ApplicationDescription* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_ApplicationDescription_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_RequestHeader
/*============================================================================
 * The RequestHeader structure.
 *===========================================================================*/
typedef struct _OpcUa_RequestHeader
{
    UA_NodeId          AuthenticationToken;
    UA_DateTime        Timestamp;
    uint32_t           RequestHandle;
    uint32_t           ReturnDiagnostics;
    UA_String          AuditEntryId;
    uint32_t           TimeoutHint;
    UA_ExtensionObject AdditionalHeader;
}
OpcUa_RequestHeader;

void OpcUa_RequestHeader_Initialize(OpcUa_RequestHeader* pValue);

void OpcUa_RequestHeader_Clear(OpcUa_RequestHeader* pValue);

//StatusCode OpcUa_RequestHeader_GetSize(OpcUa_RequestHeader* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_RequestHeader_Encode(OpcUa_RequestHeader* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_RequestHeader_Decode(OpcUa_RequestHeader* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_RequestHeader_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ResponseHeader
/*============================================================================
 * The ResponseHeader structure.
 *===========================================================================*/
typedef struct _OpcUa_ResponseHeader
{
    UA_DateTime        Timestamp;
    uint32_t           RequestHandle;
    SOPC_StatusCode         ServiceResult;
    UA_DiagnosticInfo  ServiceDiagnostics;
    int32_t            NoOfStringTable;
    UA_String*         StringTable;
    UA_ExtensionObject AdditionalHeader;
}
OpcUa_ResponseHeader;

void OpcUa_ResponseHeader_Initialize(OpcUa_ResponseHeader* pValue);

void OpcUa_ResponseHeader_Clear(OpcUa_ResponseHeader* pValue);

//StatusCode OpcUa_ResponseHeader_GetSize(OpcUa_ResponseHeader* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_ResponseHeader_Encode(OpcUa_ResponseHeader* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ResponseHeader_Decode(OpcUa_ResponseHeader* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_ResponseHeader_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ServiceFault
/*============================================================================
 * The ServiceFault structure.
 *===========================================================================*/
typedef struct _OpcUa_ServiceFault
{
    OpcUa_ResponseHeader ResponseHeader;
}
OpcUa_ServiceFault;

void OpcUa_ServiceFault_Initialize(OpcUa_ServiceFault* pValue);

void OpcUa_ServiceFault_Clear(OpcUa_ServiceFault* pValue);

//StatusCode OpcUa_ServiceFault_GetSize(OpcUa_ServiceFault* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_ServiceFault_Encode(OpcUa_ServiceFault* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ServiceFault_Decode(OpcUa_ServiceFault* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_ServiceFault_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_FindServers
#ifndef OPCUA_EXCLUDE_FindServersRequest
/*============================================================================
 * The FindServersRequest structure.
 *===========================================================================*/
typedef struct _OpcUa_FindServersRequest
{
    OpcUa_RequestHeader RequestHeader;
    UA_String           EndpointUrl;
    int32_t             NoOfLocaleIds;
    UA_String*          LocaleIds;
    int32_t             NoOfServerUris;
    UA_String*          ServerUris;
}
OpcUa_FindServersRequest;

void OpcUa_FindServersRequest_Initialize(OpcUa_FindServersRequest* pValue);

void OpcUa_FindServersRequest_Clear(OpcUa_FindServersRequest* pValue);

//StatusCode OpcUa_FindServersRequest_GetSize(OpcUa_FindServersRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_FindServersRequest_Encode(OpcUa_FindServersRequest* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_FindServersRequest_Decode(OpcUa_FindServersRequest* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_FindServersRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_FindServersResponse
/*============================================================================
 * The FindServersResponse structure.
 *===========================================================================*/
typedef struct _OpcUa_FindServersResponse
{
    OpcUa_ResponseHeader          ResponseHeader;
    int32_t                       NoOfServers;
    OpcUa_ApplicationDescription* Servers;
}
OpcUa_FindServersResponse;

void OpcUa_FindServersResponse_Initialize(OpcUa_FindServersResponse* pValue);

void OpcUa_FindServersResponse_Clear(OpcUa_FindServersResponse* pValue);

//StatusCode OpcUa_FindServersResponse_GetSize(OpcUa_FindServersResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_FindServersResponse_Encode(OpcUa_FindServersResponse* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_FindServersResponse_Decode(OpcUa_FindServersResponse* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_FindServersResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_ServerOnNetwork
/*============================================================================
 * The ServerOnNetwork structure.
 *===========================================================================*/
typedef struct _OpcUa_ServerOnNetwork
{
    uint32_t   RecordId;
    UA_String  ServerName;
    UA_String  DiscoveryUrl;
    int32_t    NoOfServerCapabilities;
    UA_String* ServerCapabilities;
}
OpcUa_ServerOnNetwork;

void OpcUa_ServerOnNetwork_Initialize(OpcUa_ServerOnNetwork* pValue);

void OpcUa_ServerOnNetwork_Clear(OpcUa_ServerOnNetwork* pValue);

//StatusCode OpcUa_ServerOnNetwork_GetSize(OpcUa_ServerOnNetwork* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_ServerOnNetwork_Encode(OpcUa_ServerOnNetwork* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ServerOnNetwork_Decode(OpcUa_ServerOnNetwork* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_ServerOnNetwork_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_FindServersOnNetwork
#ifndef OPCUA_EXCLUDE_FindServersOnNetworkRequest
/*============================================================================
 * The FindServersOnNetworkRequest structure.
 *===========================================================================*/
typedef struct _OpcUa_FindServersOnNetworkRequest
{
    OpcUa_RequestHeader RequestHeader;
    uint32_t            StartingRecordId;
    uint32_t            MaxRecordsToReturn;
    int32_t             NoOfServerCapabilityFilter;
    UA_String*          ServerCapabilityFilter;
}
OpcUa_FindServersOnNetworkRequest;

void OpcUa_FindServersOnNetworkRequest_Initialize(OpcUa_FindServersOnNetworkRequest* pValue);

void OpcUa_FindServersOnNetworkRequest_Clear(OpcUa_FindServersOnNetworkRequest* pValue);

//StatusCode OpcUa_FindServersOnNetworkRequest_GetSize(OpcUa_FindServersOnNetworkRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_FindServersOnNetworkRequest_Encode(OpcUa_FindServersOnNetworkRequest* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_FindServersOnNetworkRequest_Decode(OpcUa_FindServersOnNetworkRequest* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_FindServersOnNetworkRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_FindServersOnNetworkResponse
/*============================================================================
 * The FindServersOnNetworkResponse structure.
 *===========================================================================*/
typedef struct _OpcUa_FindServersOnNetworkResponse
{
    OpcUa_ResponseHeader   ResponseHeader;
    UA_DateTime            LastCounterResetTime;
    int32_t                NoOfServers;
    OpcUa_ServerOnNetwork* Servers;
}
OpcUa_FindServersOnNetworkResponse;

void OpcUa_FindServersOnNetworkResponse_Initialize(OpcUa_FindServersOnNetworkResponse* pValue);

void OpcUa_FindServersOnNetworkResponse_Clear(OpcUa_FindServersOnNetworkResponse* pValue);

//StatusCode OpcUa_FindServersOnNetworkResponse_GetSize(OpcUa_FindServersOnNetworkResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_FindServersOnNetworkResponse_Encode(OpcUa_FindServersOnNetworkResponse* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_FindServersOnNetworkResponse_Decode(OpcUa_FindServersOnNetworkResponse* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_FindServersOnNetworkResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_MessageSecurityMode
/*============================================================================
 * The MessageSecurityMode enumeration.
 *===========================================================================*/
typedef enum _OpcUa_MessageSecurityMode
{
    OpcUa_MessageSecurityMode_Invalid        = 0,
    OpcUa_MessageSecurityMode_None           = 1,
    OpcUa_MessageSecurityMode_Sign           = 2,
    OpcUa_MessageSecurityMode_SignAndEncrypt = 3
#if OPCUA_FORCE_INT32_ENUMS
    ,_OpcUa_MessageSecurityMode_MaxEnumerationValue = OpcUa_Int32_Max
#endif
}
OpcUa_MessageSecurityMode;

#endif

#ifndef OPCUA_EXCLUDE_UserTokenType
/*============================================================================
 * The UserTokenType enumeration.
 *===========================================================================*/
typedef enum _OpcUa_UserTokenType
{
    OpcUa_UserTokenType_Anonymous   = 0,
    OpcUa_UserTokenType_UserName    = 1,
    OpcUa_UserTokenType_Certificate = 2,
    OpcUa_UserTokenType_IssuedToken = 3,
    OpcUa_UserTokenType_Kerberos    = 4
#if OPCUA_FORCE_INT32_ENUMS
    ,_OpcUa_UserTokenType_MaxEnumerationValue = OpcUa_Int32_Max
#endif
}
OpcUa_UserTokenType;

#endif

#ifndef OPCUA_EXCLUDE_UserTokenPolicy
/*============================================================================
 * The UserTokenPolicy structure.
 *===========================================================================*/
typedef struct _OpcUa_UserTokenPolicy
{
    UA_String           PolicyId;
    OpcUa_UserTokenType TokenType;
    UA_String           IssuedTokenType;
    UA_String           IssuerEndpointUrl;
    UA_String           SecurityPolicyUri;
}
OpcUa_UserTokenPolicy;

void OpcUa_UserTokenPolicy_Initialize(OpcUa_UserTokenPolicy* pValue);

void OpcUa_UserTokenPolicy_Clear(OpcUa_UserTokenPolicy* pValue);

//StatusCode OpcUa_UserTokenPolicy_GetSize(OpcUa_UserTokenPolicy* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_UserTokenPolicy_Encode(OpcUa_UserTokenPolicy* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_UserTokenPolicy_Decode(OpcUa_UserTokenPolicy* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_UserTokenPolicy_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_EndpointDescription
/*============================================================================
 * The EndpointDescription structure.
 *===========================================================================*/
typedef struct _OpcUa_EndpointDescription
{
    UA_String                    EndpointUrl;
    OpcUa_ApplicationDescription Server;
    UA_ByteString                ServerCertificate;
    OpcUa_MessageSecurityMode    SecurityMode;
    UA_String                    SecurityPolicyUri;
    int32_t                      NoOfUserIdentityTokens;
    OpcUa_UserTokenPolicy*       UserIdentityTokens;
    UA_String                    TransportProfileUri;
    UA_Byte                      SecurityLevel;
}
OpcUa_EndpointDescription;

void OpcUa_EndpointDescription_Initialize(OpcUa_EndpointDescription* pValue);

void OpcUa_EndpointDescription_Clear(OpcUa_EndpointDescription* pValue);

//StatusCode OpcUa_EndpointDescription_GetSize(OpcUa_EndpointDescription* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_EndpointDescription_Encode(OpcUa_EndpointDescription* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_EndpointDescription_Decode(OpcUa_EndpointDescription* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_EndpointDescription_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_GetEndpoints
#ifndef OPCUA_EXCLUDE_GetEndpointsRequest
/*============================================================================
 * The GetEndpointsRequest structure.
 *===========================================================================*/
typedef struct _OpcUa_GetEndpointsRequest
{
    OpcUa_RequestHeader RequestHeader;
    UA_String           EndpointUrl;
    int32_t             NoOfLocaleIds;
    UA_String*          LocaleIds;
    int32_t             NoOfProfileUris;
    UA_String*          ProfileUris;
}
OpcUa_GetEndpointsRequest;

void OpcUa_GetEndpointsRequest_Initialize(OpcUa_GetEndpointsRequest* pValue);

void OpcUa_GetEndpointsRequest_Clear(OpcUa_GetEndpointsRequest* pValue);

//StatusCode OpcUa_GetEndpointsRequest_GetSize(OpcUa_GetEndpointsRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_GetEndpointsRequest_Encode(OpcUa_GetEndpointsRequest* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_GetEndpointsRequest_Decode(OpcUa_GetEndpointsRequest* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_GetEndpointsRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_GetEndpointsResponse
/*============================================================================
 * The GetEndpointsResponse structure.
 *===========================================================================*/
typedef struct _OpcUa_GetEndpointsResponse
{
    OpcUa_ResponseHeader       ResponseHeader;
    int32_t                    NoOfEndpoints;
    OpcUa_EndpointDescription* Endpoints;
}
OpcUa_GetEndpointsResponse;

void OpcUa_GetEndpointsResponse_Initialize(OpcUa_GetEndpointsResponse* pValue);

void OpcUa_GetEndpointsResponse_Clear(OpcUa_GetEndpointsResponse* pValue);

//StatusCode OpcUa_GetEndpointsResponse_GetSize(OpcUa_GetEndpointsResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_GetEndpointsResponse_Encode(OpcUa_GetEndpointsResponse* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_GetEndpointsResponse_Decode(OpcUa_GetEndpointsResponse* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_GetEndpointsResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_RegisteredServer
/*============================================================================
 * The RegisteredServer structure.
 *===========================================================================*/
typedef struct _OpcUa_RegisteredServer
{
    UA_String             ServerUri;
    UA_String             ProductUri;
    int32_t               NoOfServerNames;
    UA_LocalizedText*     ServerNames;
    OpcUa_ApplicationType ServerType;
    UA_String             GatewayServerUri;
    int32_t               NoOfDiscoveryUrls;
    UA_String*            DiscoveryUrls;
    UA_String             SemaphoreFilePath;
    UA_Boolean            IsOnline;
}
OpcUa_RegisteredServer;

void OpcUa_RegisteredServer_Initialize(OpcUa_RegisteredServer* pValue);

void OpcUa_RegisteredServer_Clear(OpcUa_RegisteredServer* pValue);

//StatusCode OpcUa_RegisteredServer_GetSize(OpcUa_RegisteredServer* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_RegisteredServer_Encode(OpcUa_RegisteredServer* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_RegisteredServer_Decode(OpcUa_RegisteredServer* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_RegisteredServer_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_RegisterServer
#ifndef OPCUA_EXCLUDE_RegisterServerRequest
/*============================================================================
 * The RegisterServerRequest structure.
 *===========================================================================*/
typedef struct _OpcUa_RegisterServerRequest
{
    OpcUa_RequestHeader    RequestHeader;
    OpcUa_RegisteredServer Server;
}
OpcUa_RegisterServerRequest;

void OpcUa_RegisterServerRequest_Initialize(OpcUa_RegisterServerRequest* pValue);

void OpcUa_RegisterServerRequest_Clear(OpcUa_RegisterServerRequest* pValue);

//StatusCode OpcUa_RegisterServerRequest_GetSize(OpcUa_RegisterServerRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_RegisterServerRequest_Encode(OpcUa_RegisterServerRequest* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_RegisterServerRequest_Decode(OpcUa_RegisterServerRequest* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_RegisterServerRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_RegisterServerResponse
/*============================================================================
 * The RegisterServerResponse structure.
 *===========================================================================*/
typedef struct _OpcUa_RegisterServerResponse
{
    OpcUa_ResponseHeader ResponseHeader;
}
OpcUa_RegisterServerResponse;

void OpcUa_RegisterServerResponse_Initialize(OpcUa_RegisterServerResponse* pValue);

void OpcUa_RegisterServerResponse_Clear(OpcUa_RegisterServerResponse* pValue);

//StatusCode OpcUa_RegisterServerResponse_GetSize(OpcUa_RegisterServerResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_RegisterServerResponse_Encode(OpcUa_RegisterServerResponse* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_RegisterServerResponse_Decode(OpcUa_RegisterServerResponse* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_RegisterServerResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_MdnsDiscoveryConfiguration
/*============================================================================
 * The MdnsDiscoveryConfiguration structure.
 *===========================================================================*/
typedef struct _OpcUa_MdnsDiscoveryConfiguration
{
    UA_String  MdnsServerName;
    int32_t    NoOfServerCapabilities;
    UA_String* ServerCapabilities;
}
OpcUa_MdnsDiscoveryConfiguration;

void OpcUa_MdnsDiscoveryConfiguration_Initialize(OpcUa_MdnsDiscoveryConfiguration* pValue);

void OpcUa_MdnsDiscoveryConfiguration_Clear(OpcUa_MdnsDiscoveryConfiguration* pValue);

//StatusCode OpcUa_MdnsDiscoveryConfiguration_GetSize(OpcUa_MdnsDiscoveryConfiguration* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_MdnsDiscoveryConfiguration_Encode(OpcUa_MdnsDiscoveryConfiguration* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_MdnsDiscoveryConfiguration_Decode(OpcUa_MdnsDiscoveryConfiguration* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_MdnsDiscoveryConfiguration_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_RegisterServer2
#ifndef OPCUA_EXCLUDE_RegisterServer2Request
/*============================================================================
 * The RegisterServer2Request structure.
 *===========================================================================*/
typedef struct _OpcUa_RegisterServer2Request
{
    OpcUa_RequestHeader    RequestHeader;
    OpcUa_RegisteredServer Server;
    int32_t                NoOfDiscoveryConfiguration;
    UA_ExtensionObject*    DiscoveryConfiguration;
}
OpcUa_RegisterServer2Request;

void OpcUa_RegisterServer2Request_Initialize(OpcUa_RegisterServer2Request* pValue);

void OpcUa_RegisterServer2Request_Clear(OpcUa_RegisterServer2Request* pValue);

//StatusCode OpcUa_RegisterServer2Request_GetSize(OpcUa_RegisterServer2Request* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_RegisterServer2Request_Encode(OpcUa_RegisterServer2Request* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_RegisterServer2Request_Decode(OpcUa_RegisterServer2Request* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_RegisterServer2Request_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_RegisterServer2Response
/*============================================================================
 * The RegisterServer2Response structure.
 *===========================================================================*/
typedef struct _OpcUa_RegisterServer2Response
{
    OpcUa_ResponseHeader ResponseHeader;
    int32_t              NoOfConfigurationResults;
    SOPC_StatusCode*          ConfigurationResults;
    int32_t              NoOfDiagnosticInfos;
    UA_DiagnosticInfo*   DiagnosticInfos;
}
OpcUa_RegisterServer2Response;

void OpcUa_RegisterServer2Response_Initialize(OpcUa_RegisterServer2Response* pValue);

void OpcUa_RegisterServer2Response_Clear(OpcUa_RegisterServer2Response* pValue);

//StatusCode OpcUa_RegisterServer2Response_GetSize(OpcUa_RegisterServer2Response* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_RegisterServer2Response_Encode(OpcUa_RegisterServer2Response* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_RegisterServer2Response_Decode(OpcUa_RegisterServer2Response* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_RegisterServer2Response_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_SecurityTokenRequestType
/*============================================================================
 * The SecurityTokenRequestType enumeration.
 *===========================================================================*/
typedef enum _OpcUa_SecurityTokenRequestType
{
    OpcUa_SecurityTokenRequestType_Issue = 0,
    OpcUa_SecurityTokenRequestType_Renew = 1
#if OPCUA_FORCE_INT32_ENUMS
    ,_OpcUa_SecurityTokenRequestType_MaxEnumerationValue = OpcUa_Int32_Max
#endif
}
OpcUa_SecurityTokenRequestType;

#endif

#ifndef OPCUA_EXCLUDE_ChannelSecurityToken
/*============================================================================
 * The ChannelSecurityToken structure.
 *===========================================================================*/
typedef struct _OpcUa_ChannelSecurityToken
{
    uint32_t    ChannelId;
    uint32_t    TokenId;
    UA_DateTime CreatedAt;
    uint32_t    RevisedLifetime;
}
OpcUa_ChannelSecurityToken;

void OpcUa_ChannelSecurityToken_Initialize(OpcUa_ChannelSecurityToken* pValue);

void OpcUa_ChannelSecurityToken_Clear(OpcUa_ChannelSecurityToken* pValue);

//StatusCode OpcUa_ChannelSecurityToken_GetSize(OpcUa_ChannelSecurityToken* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_ChannelSecurityToken_Encode(OpcUa_ChannelSecurityToken* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ChannelSecurityToken_Decode(OpcUa_ChannelSecurityToken* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_ChannelSecurityToken_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_OpenSecureChannel
#ifndef OPCUA_EXCLUDE_OpenSecureChannelRequest
/*============================================================================
 * The OpenSecureChannelRequest structure.
 *===========================================================================*/
typedef struct _OpcUa_OpenSecureChannelRequest
{
    OpcUa_RequestHeader            RequestHeader;
    uint32_t                       ClientProtocolVersion;
    OpcUa_SecurityTokenRequestType RequestType;
    OpcUa_MessageSecurityMode      SecurityMode;
    UA_ByteString                  ClientNonce;
    uint32_t                       RequestedLifetime;
}
OpcUa_OpenSecureChannelRequest;

void OpcUa_OpenSecureChannelRequest_Initialize(OpcUa_OpenSecureChannelRequest* pValue);

void OpcUa_OpenSecureChannelRequest_Clear(OpcUa_OpenSecureChannelRequest* pValue);

//StatusCode OpcUa_OpenSecureChannelRequest_GetSize(OpcUa_OpenSecureChannelRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_OpenSecureChannelRequest_Encode(OpcUa_OpenSecureChannelRequest* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_OpenSecureChannelRequest_Decode(OpcUa_OpenSecureChannelRequest* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_OpenSecureChannelRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_OpenSecureChannelResponse
/*============================================================================
 * The OpenSecureChannelResponse structure.
 *===========================================================================*/
typedef struct _OpcUa_OpenSecureChannelResponse
{
    OpcUa_ResponseHeader       ResponseHeader;
    uint32_t                   ServerProtocolVersion;
    OpcUa_ChannelSecurityToken SecurityToken;
    UA_ByteString              ServerNonce;
}
OpcUa_OpenSecureChannelResponse;

void OpcUa_OpenSecureChannelResponse_Initialize(OpcUa_OpenSecureChannelResponse* pValue);

void OpcUa_OpenSecureChannelResponse_Clear(OpcUa_OpenSecureChannelResponse* pValue);

//StatusCode OpcUa_OpenSecureChannelResponse_GetSize(OpcUa_OpenSecureChannelResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_OpenSecureChannelResponse_Encode(OpcUa_OpenSecureChannelResponse* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_OpenSecureChannelResponse_Decode(OpcUa_OpenSecureChannelResponse* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_OpenSecureChannelResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_CloseSecureChannel
#ifndef OPCUA_EXCLUDE_CloseSecureChannelRequest
/*============================================================================
 * The CloseSecureChannelRequest structure.
 *===========================================================================*/
typedef struct _OpcUa_CloseSecureChannelRequest
{
    OpcUa_RequestHeader RequestHeader;
}
OpcUa_CloseSecureChannelRequest;

void OpcUa_CloseSecureChannelRequest_Initialize(OpcUa_CloseSecureChannelRequest* pValue);

void OpcUa_CloseSecureChannelRequest_Clear(OpcUa_CloseSecureChannelRequest* pValue);

//StatusCode OpcUa_CloseSecureChannelRequest_GetSize(OpcUa_CloseSecureChannelRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_CloseSecureChannelRequest_Encode(OpcUa_CloseSecureChannelRequest* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_CloseSecureChannelRequest_Decode(OpcUa_CloseSecureChannelRequest* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_CloseSecureChannelRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_CloseSecureChannelResponse
/*============================================================================
 * The CloseSecureChannelResponse structure.
 *===========================================================================*/
typedef struct _OpcUa_CloseSecureChannelResponse
{
    OpcUa_ResponseHeader ResponseHeader;
}
OpcUa_CloseSecureChannelResponse;

void OpcUa_CloseSecureChannelResponse_Initialize(OpcUa_CloseSecureChannelResponse* pValue);

void OpcUa_CloseSecureChannelResponse_Clear(OpcUa_CloseSecureChannelResponse* pValue);

//StatusCode OpcUa_CloseSecureChannelResponse_GetSize(OpcUa_CloseSecureChannelResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_CloseSecureChannelResponse_Encode(OpcUa_CloseSecureChannelResponse* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_CloseSecureChannelResponse_Decode(OpcUa_CloseSecureChannelResponse* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_CloseSecureChannelResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_SignedSoftwareCertificate
/*============================================================================
 * The SignedSoftwareCertificate structure.
 *===========================================================================*/
typedef struct _OpcUa_SignedSoftwareCertificate
{
    UA_ByteString CertificateData;
    UA_ByteString Signature;
}
OpcUa_SignedSoftwareCertificate;

void OpcUa_SignedSoftwareCertificate_Initialize(OpcUa_SignedSoftwareCertificate* pValue);

void OpcUa_SignedSoftwareCertificate_Clear(OpcUa_SignedSoftwareCertificate* pValue);

//StatusCode OpcUa_SignedSoftwareCertificate_GetSize(OpcUa_SignedSoftwareCertificate* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_SignedSoftwareCertificate_Encode(OpcUa_SignedSoftwareCertificate* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_SignedSoftwareCertificate_Decode(OpcUa_SignedSoftwareCertificate* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_SignedSoftwareCertificate_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_SignatureData
/*============================================================================
 * The SignatureData structure.
 *===========================================================================*/
typedef struct _OpcUa_SignatureData
{
    UA_String     Algorithm;
    UA_ByteString Signature;
}
OpcUa_SignatureData;

void OpcUa_SignatureData_Initialize(OpcUa_SignatureData* pValue);

void OpcUa_SignatureData_Clear(OpcUa_SignatureData* pValue);

//StatusCode OpcUa_SignatureData_GetSize(OpcUa_SignatureData* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_SignatureData_Encode(OpcUa_SignatureData* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_SignatureData_Decode(OpcUa_SignatureData* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_SignatureData_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_CreateSession
#ifndef OPCUA_EXCLUDE_CreateSessionRequest
/*============================================================================
 * The CreateSessionRequest structure.
 *===========================================================================*/
typedef struct _OpcUa_CreateSessionRequest
{
    OpcUa_RequestHeader          RequestHeader;
    OpcUa_ApplicationDescription ClientDescription;
    UA_String                    ServerUri;
    UA_String                    EndpointUrl;
    UA_String                    SessionName;
    UA_ByteString                ClientNonce;
    UA_ByteString                ClientCertificate;
    double                       RequestedSessionTimeout;
    uint32_t                     MaxResponseMessageSize;
}
OpcUa_CreateSessionRequest;

void OpcUa_CreateSessionRequest_Initialize(OpcUa_CreateSessionRequest* pValue);

void OpcUa_CreateSessionRequest_Clear(OpcUa_CreateSessionRequest* pValue);

//StatusCode OpcUa_CreateSessionRequest_GetSize(OpcUa_CreateSessionRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_CreateSessionRequest_Encode(OpcUa_CreateSessionRequest* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_CreateSessionRequest_Decode(OpcUa_CreateSessionRequest* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_CreateSessionRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_CreateSessionResponse
/*============================================================================
 * The CreateSessionResponse structure.
 *===========================================================================*/
typedef struct _OpcUa_CreateSessionResponse
{
    OpcUa_ResponseHeader             ResponseHeader;
    UA_NodeId                        SessionId;
    UA_NodeId                        AuthenticationToken;
    double                           RevisedSessionTimeout;
    UA_ByteString                    ServerNonce;
    UA_ByteString                    ServerCertificate;
    int32_t                          NoOfServerEndpoints;
    OpcUa_EndpointDescription*       ServerEndpoints;
    int32_t                          NoOfServerSoftwareCertificates;
    OpcUa_SignedSoftwareCertificate* ServerSoftwareCertificates;
    OpcUa_SignatureData              ServerSignature;
    uint32_t                         MaxRequestMessageSize;
}
OpcUa_CreateSessionResponse;

void OpcUa_CreateSessionResponse_Initialize(OpcUa_CreateSessionResponse* pValue);

void OpcUa_CreateSessionResponse_Clear(OpcUa_CreateSessionResponse* pValue);

//StatusCode OpcUa_CreateSessionResponse_GetSize(OpcUa_CreateSessionResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_CreateSessionResponse_Encode(OpcUa_CreateSessionResponse* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_CreateSessionResponse_Decode(OpcUa_CreateSessionResponse* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_CreateSessionResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_UserIdentityToken
/*============================================================================
 * The UserIdentityToken structure.
 *===========================================================================*/
typedef struct _OpcUa_UserIdentityToken
{
    UA_String PolicyId;
}
OpcUa_UserIdentityToken;

void OpcUa_UserIdentityToken_Initialize(OpcUa_UserIdentityToken* pValue);

void OpcUa_UserIdentityToken_Clear(OpcUa_UserIdentityToken* pValue);

//StatusCode OpcUa_UserIdentityToken_GetSize(OpcUa_UserIdentityToken* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_UserIdentityToken_Encode(OpcUa_UserIdentityToken* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_UserIdentityToken_Decode(OpcUa_UserIdentityToken* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_UserIdentityToken_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_AnonymousIdentityToken
/*============================================================================
 * The AnonymousIdentityToken structure.
 *===========================================================================*/
typedef struct _OpcUa_AnonymousIdentityToken
{
    UA_String PolicyId;
}
OpcUa_AnonymousIdentityToken;

void OpcUa_AnonymousIdentityToken_Initialize(OpcUa_AnonymousIdentityToken* pValue);

void OpcUa_AnonymousIdentityToken_Clear(OpcUa_AnonymousIdentityToken* pValue);

//StatusCode OpcUa_AnonymousIdentityToken_GetSize(OpcUa_AnonymousIdentityToken* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_AnonymousIdentityToken_Encode(OpcUa_AnonymousIdentityToken* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_AnonymousIdentityToken_Decode(OpcUa_AnonymousIdentityToken* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_AnonymousIdentityToken_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_UserNameIdentityToken
/*============================================================================
 * The UserNameIdentityToken structure.
 *===========================================================================*/
typedef struct _OpcUa_UserNameIdentityToken
{
    UA_String     PolicyId;
    UA_String     UserName;
    UA_ByteString Password;
    UA_String     EncryptionAlgorithm;
}
OpcUa_UserNameIdentityToken;

void OpcUa_UserNameIdentityToken_Initialize(OpcUa_UserNameIdentityToken* pValue);

void OpcUa_UserNameIdentityToken_Clear(OpcUa_UserNameIdentityToken* pValue);

//StatusCode OpcUa_UserNameIdentityToken_GetSize(OpcUa_UserNameIdentityToken* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_UserNameIdentityToken_Encode(OpcUa_UserNameIdentityToken* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_UserNameIdentityToken_Decode(OpcUa_UserNameIdentityToken* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_UserNameIdentityToken_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_X509IdentityToken
/*============================================================================
 * The X509IdentityToken structure.
 *===========================================================================*/
typedef struct _OpcUa_X509IdentityToken
{
    UA_String     PolicyId;
    UA_ByteString CertificateData;
}
OpcUa_X509IdentityToken;

void OpcUa_X509IdentityToken_Initialize(OpcUa_X509IdentityToken* pValue);

void OpcUa_X509IdentityToken_Clear(OpcUa_X509IdentityToken* pValue);

//StatusCode OpcUa_X509IdentityToken_GetSize(OpcUa_X509IdentityToken* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_X509IdentityToken_Encode(OpcUa_X509IdentityToken* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_X509IdentityToken_Decode(OpcUa_X509IdentityToken* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_X509IdentityToken_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_KerberosIdentityToken
/*============================================================================
 * The KerberosIdentityToken structure.
 *===========================================================================*/
typedef struct _OpcUa_KerberosIdentityToken
{
    UA_String     PolicyId;
    UA_ByteString TicketData;
}
OpcUa_KerberosIdentityToken;

void OpcUa_KerberosIdentityToken_Initialize(OpcUa_KerberosIdentityToken* pValue);

void OpcUa_KerberosIdentityToken_Clear(OpcUa_KerberosIdentityToken* pValue);

//StatusCode OpcUa_KerberosIdentityToken_GetSize(OpcUa_KerberosIdentityToken* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_KerberosIdentityToken_Encode(OpcUa_KerberosIdentityToken* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_KerberosIdentityToken_Decode(OpcUa_KerberosIdentityToken* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_KerberosIdentityToken_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_IssuedIdentityToken
/*============================================================================
 * The IssuedIdentityToken structure.
 *===========================================================================*/
typedef struct _OpcUa_IssuedIdentityToken
{
    UA_String     PolicyId;
    UA_ByteString TokenData;
    UA_String     EncryptionAlgorithm;
}
OpcUa_IssuedIdentityToken;

void OpcUa_IssuedIdentityToken_Initialize(OpcUa_IssuedIdentityToken* pValue);

void OpcUa_IssuedIdentityToken_Clear(OpcUa_IssuedIdentityToken* pValue);

//StatusCode OpcUa_IssuedIdentityToken_GetSize(OpcUa_IssuedIdentityToken* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_IssuedIdentityToken_Encode(OpcUa_IssuedIdentityToken* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_IssuedIdentityToken_Decode(OpcUa_IssuedIdentityToken* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_IssuedIdentityToken_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ActivateSession
#ifndef OPCUA_EXCLUDE_ActivateSessionRequest
/*============================================================================
 * The ActivateSessionRequest structure.
 *===========================================================================*/
typedef struct _OpcUa_ActivateSessionRequest
{
    OpcUa_RequestHeader              RequestHeader;
    OpcUa_SignatureData              ClientSignature;
    int32_t                          NoOfClientSoftwareCertificates;
    OpcUa_SignedSoftwareCertificate* ClientSoftwareCertificates;
    int32_t                          NoOfLocaleIds;
    UA_String*                       LocaleIds;
    UA_ExtensionObject               UserIdentityToken;
    OpcUa_SignatureData              UserTokenSignature;
}
OpcUa_ActivateSessionRequest;

void OpcUa_ActivateSessionRequest_Initialize(OpcUa_ActivateSessionRequest* pValue);

void OpcUa_ActivateSessionRequest_Clear(OpcUa_ActivateSessionRequest* pValue);

//StatusCode OpcUa_ActivateSessionRequest_GetSize(OpcUa_ActivateSessionRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_ActivateSessionRequest_Encode(OpcUa_ActivateSessionRequest* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ActivateSessionRequest_Decode(OpcUa_ActivateSessionRequest* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_ActivateSessionRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ActivateSessionResponse
/*============================================================================
 * The ActivateSessionResponse structure.
 *===========================================================================*/
typedef struct _OpcUa_ActivateSessionResponse
{
    OpcUa_ResponseHeader ResponseHeader;
    UA_ByteString        ServerNonce;
    int32_t              NoOfResults;
    SOPC_StatusCode*          Results;
    int32_t              NoOfDiagnosticInfos;
    UA_DiagnosticInfo*   DiagnosticInfos;
}
OpcUa_ActivateSessionResponse;

void OpcUa_ActivateSessionResponse_Initialize(OpcUa_ActivateSessionResponse* pValue);

void OpcUa_ActivateSessionResponse_Clear(OpcUa_ActivateSessionResponse* pValue);

//StatusCode OpcUa_ActivateSessionResponse_GetSize(OpcUa_ActivateSessionResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_ActivateSessionResponse_Encode(OpcUa_ActivateSessionResponse* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ActivateSessionResponse_Decode(OpcUa_ActivateSessionResponse* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_ActivateSessionResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_CloseSession
#ifndef OPCUA_EXCLUDE_CloseSessionRequest
/*============================================================================
 * The CloseSessionRequest structure.
 *===========================================================================*/
typedef struct _OpcUa_CloseSessionRequest
{
    OpcUa_RequestHeader RequestHeader;
    UA_Boolean          DeleteSubscriptions;
}
OpcUa_CloseSessionRequest;

void OpcUa_CloseSessionRequest_Initialize(OpcUa_CloseSessionRequest* pValue);

void OpcUa_CloseSessionRequest_Clear(OpcUa_CloseSessionRequest* pValue);

//StatusCode OpcUa_CloseSessionRequest_GetSize(OpcUa_CloseSessionRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_CloseSessionRequest_Encode(OpcUa_CloseSessionRequest* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_CloseSessionRequest_Decode(OpcUa_CloseSessionRequest* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_CloseSessionRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_CloseSessionResponse
/*============================================================================
 * The CloseSessionResponse structure.
 *===========================================================================*/
typedef struct _OpcUa_CloseSessionResponse
{
    OpcUa_ResponseHeader ResponseHeader;
}
OpcUa_CloseSessionResponse;

void OpcUa_CloseSessionResponse_Initialize(OpcUa_CloseSessionResponse* pValue);

void OpcUa_CloseSessionResponse_Clear(OpcUa_CloseSessionResponse* pValue);

//StatusCode OpcUa_CloseSessionResponse_GetSize(OpcUa_CloseSessionResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_CloseSessionResponse_Encode(OpcUa_CloseSessionResponse* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_CloseSessionResponse_Decode(OpcUa_CloseSessionResponse* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_CloseSessionResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_Cancel
#ifndef OPCUA_EXCLUDE_CancelRequest
/*============================================================================
 * The CancelRequest structure.
 *===========================================================================*/
typedef struct _OpcUa_CancelRequest
{
    OpcUa_RequestHeader RequestHeader;
    uint32_t            RequestHandle;
}
OpcUa_CancelRequest;

void OpcUa_CancelRequest_Initialize(OpcUa_CancelRequest* pValue);

void OpcUa_CancelRequest_Clear(OpcUa_CancelRequest* pValue);

//StatusCode OpcUa_CancelRequest_GetSize(OpcUa_CancelRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_CancelRequest_Encode(OpcUa_CancelRequest* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_CancelRequest_Decode(OpcUa_CancelRequest* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_CancelRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_CancelResponse
/*============================================================================
 * The CancelResponse structure.
 *===========================================================================*/
typedef struct _OpcUa_CancelResponse
{
    OpcUa_ResponseHeader ResponseHeader;
    uint32_t             CancelCount;
}
OpcUa_CancelResponse;

void OpcUa_CancelResponse_Initialize(OpcUa_CancelResponse* pValue);

void OpcUa_CancelResponse_Clear(OpcUa_CancelResponse* pValue);

//StatusCode OpcUa_CancelResponse_GetSize(OpcUa_CancelResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_CancelResponse_Encode(OpcUa_CancelResponse* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_CancelResponse_Decode(OpcUa_CancelResponse* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_CancelResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_NodeAttributesMask
/*============================================================================
 * The NodeAttributesMask enumeration.
 *===========================================================================*/
typedef enum _OpcUa_NodeAttributesMask
{
    OpcUa_NodeAttributesMask_None                    = 0,
    OpcUa_NodeAttributesMask_AccessLevel             = 1,
    OpcUa_NodeAttributesMask_ArrayDimensions         = 2,
    OpcUa_NodeAttributesMask_BrowseName              = 4,
    OpcUa_NodeAttributesMask_ContainsNoLoops         = 8,
    OpcUa_NodeAttributesMask_DataType                = 16,
    OpcUa_NodeAttributesMask_Description             = 32,
    OpcUa_NodeAttributesMask_DisplayName             = 64,
    OpcUa_NodeAttributesMask_EventNotifier           = 128,
    OpcUa_NodeAttributesMask_Executable              = 256,
    OpcUa_NodeAttributesMask_Historizing             = 512,
    OpcUa_NodeAttributesMask_InverseName             = 1024,
    OpcUa_NodeAttributesMask_IsAbstract              = 2048,
    OpcUa_NodeAttributesMask_MinimumSamplingInterval = 4096,
    OpcUa_NodeAttributesMask_NodeClass               = 8192,
    OpcUa_NodeAttributesMask_NodeId                  = 16384,
    OpcUa_NodeAttributesMask_Symmetric               = 32768,
    OpcUa_NodeAttributesMask_UserAccessLevel         = 65536,
    OpcUa_NodeAttributesMask_UserExecutable          = 131072,
    OpcUa_NodeAttributesMask_UserWriteMask           = 262144,
    OpcUa_NodeAttributesMask_ValueRank               = 524288,
    OpcUa_NodeAttributesMask_WriteMask               = 1048576,
    OpcUa_NodeAttributesMask_Value                   = 2097152,
    OpcUa_NodeAttributesMask_All                     = 4194303,
    OpcUa_NodeAttributesMask_BaseNode                = 1335396,
    OpcUa_NodeAttributesMask_Object                  = 1335524,
    OpcUa_NodeAttributesMask_ObjectTypeOrDataType    = 1337444,
    OpcUa_NodeAttributesMask_Variable                = 4026999,
    OpcUa_NodeAttributesMask_VariableType            = 3958902,
    OpcUa_NodeAttributesMask_Method                  = 1466724,
    OpcUa_NodeAttributesMask_ReferenceType           = 1371236,
    OpcUa_NodeAttributesMask_View                    = 1335532
#if OPCUA_FORCE_INT32_ENUMS
    ,_OpcUa_NodeAttributesMask_MaxEnumerationValue = OpcUa_Int32_Max
#endif
}
OpcUa_NodeAttributesMask;

#endif

#ifndef OPCUA_EXCLUDE_NodeAttributes
/*============================================================================
 * The NodeAttributes structure.
 *===========================================================================*/
typedef struct _OpcUa_NodeAttributes
{
    uint32_t         SpecifiedAttributes;
    UA_LocalizedText DisplayName;
    UA_LocalizedText Description;
    uint32_t         WriteMask;
    uint32_t         UserWriteMask;
}
OpcUa_NodeAttributes;

void OpcUa_NodeAttributes_Initialize(OpcUa_NodeAttributes* pValue);

void OpcUa_NodeAttributes_Clear(OpcUa_NodeAttributes* pValue);

//StatusCode OpcUa_NodeAttributes_GetSize(OpcUa_NodeAttributes* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_NodeAttributes_Encode(OpcUa_NodeAttributes* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_NodeAttributes_Decode(OpcUa_NodeAttributes* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_NodeAttributes_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ObjectAttributes
/*============================================================================
 * The ObjectAttributes structure.
 *===========================================================================*/
typedef struct _OpcUa_ObjectAttributes
{
    uint32_t         SpecifiedAttributes;
    UA_LocalizedText DisplayName;
    UA_LocalizedText Description;
    uint32_t         WriteMask;
    uint32_t         UserWriteMask;
    UA_Byte          EventNotifier;
}
OpcUa_ObjectAttributes;

void OpcUa_ObjectAttributes_Initialize(OpcUa_ObjectAttributes* pValue);

void OpcUa_ObjectAttributes_Clear(OpcUa_ObjectAttributes* pValue);

//StatusCode OpcUa_ObjectAttributes_GetSize(OpcUa_ObjectAttributes* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_ObjectAttributes_Encode(OpcUa_ObjectAttributes* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ObjectAttributes_Decode(OpcUa_ObjectAttributes* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_ObjectAttributes_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_VariableAttributes
/*============================================================================
 * The VariableAttributes structure.
 *===========================================================================*/
typedef struct _OpcUa_VariableAttributes
{
    uint32_t         SpecifiedAttributes;
    UA_LocalizedText DisplayName;
    UA_LocalizedText Description;
    uint32_t         WriteMask;
    uint32_t         UserWriteMask;
    UA_Variant       Value;
    UA_NodeId        DataType;
    int32_t          ValueRank;
    int32_t          NoOfArrayDimensions;
    uint32_t*        ArrayDimensions;
    UA_Byte          AccessLevel;
    UA_Byte          UserAccessLevel;
    double           MinimumSamplingInterval;
    UA_Boolean       Historizing;
}
OpcUa_VariableAttributes;

void OpcUa_VariableAttributes_Initialize(OpcUa_VariableAttributes* pValue);

void OpcUa_VariableAttributes_Clear(OpcUa_VariableAttributes* pValue);

//StatusCode OpcUa_VariableAttributes_GetSize(OpcUa_VariableAttributes* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_VariableAttributes_Encode(OpcUa_VariableAttributes* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_VariableAttributes_Decode(OpcUa_VariableAttributes* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_VariableAttributes_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_MethodAttributes
/*============================================================================
 * The MethodAttributes structure.
 *===========================================================================*/
typedef struct _OpcUa_MethodAttributes
{
    uint32_t         SpecifiedAttributes;
    UA_LocalizedText DisplayName;
    UA_LocalizedText Description;
    uint32_t         WriteMask;
    uint32_t         UserWriteMask;
    UA_Boolean       Executable;
    UA_Boolean       UserExecutable;
}
OpcUa_MethodAttributes;

void OpcUa_MethodAttributes_Initialize(OpcUa_MethodAttributes* pValue);

void OpcUa_MethodAttributes_Clear(OpcUa_MethodAttributes* pValue);

//StatusCode OpcUa_MethodAttributes_GetSize(OpcUa_MethodAttributes* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_MethodAttributes_Encode(OpcUa_MethodAttributes* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_MethodAttributes_Decode(OpcUa_MethodAttributes* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_MethodAttributes_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ObjectTypeAttributes
/*============================================================================
 * The ObjectTypeAttributes structure.
 *===========================================================================*/
typedef struct _OpcUa_ObjectTypeAttributes
{
    uint32_t         SpecifiedAttributes;
    UA_LocalizedText DisplayName;
    UA_LocalizedText Description;
    uint32_t         WriteMask;
    uint32_t         UserWriteMask;
    UA_Boolean       IsAbstract;
}
OpcUa_ObjectTypeAttributes;

void OpcUa_ObjectTypeAttributes_Initialize(OpcUa_ObjectTypeAttributes* pValue);

void OpcUa_ObjectTypeAttributes_Clear(OpcUa_ObjectTypeAttributes* pValue);

//StatusCode OpcUa_ObjectTypeAttributes_GetSize(OpcUa_ObjectTypeAttributes* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_ObjectTypeAttributes_Encode(OpcUa_ObjectTypeAttributes* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ObjectTypeAttributes_Decode(OpcUa_ObjectTypeAttributes* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_ObjectTypeAttributes_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_VariableTypeAttributes
/*============================================================================
 * The VariableTypeAttributes structure.
 *===========================================================================*/
typedef struct _OpcUa_VariableTypeAttributes
{
    uint32_t         SpecifiedAttributes;
    UA_LocalizedText DisplayName;
    UA_LocalizedText Description;
    uint32_t         WriteMask;
    uint32_t         UserWriteMask;
    UA_Variant       Value;
    UA_NodeId        DataType;
    int32_t          ValueRank;
    int32_t          NoOfArrayDimensions;
    uint32_t*        ArrayDimensions;
    UA_Boolean       IsAbstract;
}
OpcUa_VariableTypeAttributes;

void OpcUa_VariableTypeAttributes_Initialize(OpcUa_VariableTypeAttributes* pValue);

void OpcUa_VariableTypeAttributes_Clear(OpcUa_VariableTypeAttributes* pValue);

//StatusCode OpcUa_VariableTypeAttributes_GetSize(OpcUa_VariableTypeAttributes* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_VariableTypeAttributes_Encode(OpcUa_VariableTypeAttributes* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_VariableTypeAttributes_Decode(OpcUa_VariableTypeAttributes* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_VariableTypeAttributes_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ReferenceTypeAttributes
/*============================================================================
 * The ReferenceTypeAttributes structure.
 *===========================================================================*/
typedef struct _OpcUa_ReferenceTypeAttributes
{
    uint32_t         SpecifiedAttributes;
    UA_LocalizedText DisplayName;
    UA_LocalizedText Description;
    uint32_t         WriteMask;
    uint32_t         UserWriteMask;
    UA_Boolean       IsAbstract;
    UA_Boolean       Symmetric;
    UA_LocalizedText InverseName;
}
OpcUa_ReferenceTypeAttributes;

void OpcUa_ReferenceTypeAttributes_Initialize(OpcUa_ReferenceTypeAttributes* pValue);

void OpcUa_ReferenceTypeAttributes_Clear(OpcUa_ReferenceTypeAttributes* pValue);

//StatusCode OpcUa_ReferenceTypeAttributes_GetSize(OpcUa_ReferenceTypeAttributes* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_ReferenceTypeAttributes_Encode(OpcUa_ReferenceTypeAttributes* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ReferenceTypeAttributes_Decode(OpcUa_ReferenceTypeAttributes* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_ReferenceTypeAttributes_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_DataTypeAttributes
/*============================================================================
 * The DataTypeAttributes structure.
 *===========================================================================*/
typedef struct _OpcUa_DataTypeAttributes
{
    uint32_t         SpecifiedAttributes;
    UA_LocalizedText DisplayName;
    UA_LocalizedText Description;
    uint32_t         WriteMask;
    uint32_t         UserWriteMask;
    UA_Boolean       IsAbstract;
}
OpcUa_DataTypeAttributes;

void OpcUa_DataTypeAttributes_Initialize(OpcUa_DataTypeAttributes* pValue);

void OpcUa_DataTypeAttributes_Clear(OpcUa_DataTypeAttributes* pValue);

//StatusCode OpcUa_DataTypeAttributes_GetSize(OpcUa_DataTypeAttributes* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_DataTypeAttributes_Encode(OpcUa_DataTypeAttributes* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_DataTypeAttributes_Decode(OpcUa_DataTypeAttributes* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_DataTypeAttributes_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ViewAttributes
/*============================================================================
 * The ViewAttributes structure.
 *===========================================================================*/
typedef struct _OpcUa_ViewAttributes
{
    uint32_t         SpecifiedAttributes;
    UA_LocalizedText DisplayName;
    UA_LocalizedText Description;
    uint32_t         WriteMask;
    uint32_t         UserWriteMask;
    UA_Boolean       ContainsNoLoops;
    UA_Byte          EventNotifier;
}
OpcUa_ViewAttributes;

void OpcUa_ViewAttributes_Initialize(OpcUa_ViewAttributes* pValue);

void OpcUa_ViewAttributes_Clear(OpcUa_ViewAttributes* pValue);

//StatusCode OpcUa_ViewAttributes_GetSize(OpcUa_ViewAttributes* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_ViewAttributes_Encode(OpcUa_ViewAttributes* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ViewAttributes_Decode(OpcUa_ViewAttributes* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_ViewAttributes_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_AddNodesItem
/*============================================================================
 * The AddNodesItem structure.
 *===========================================================================*/
typedef struct _OpcUa_AddNodesItem
{
    UA_ExpandedNodeId  ParentNodeId;
    UA_NodeId          ReferenceTypeId;
    UA_ExpandedNodeId  RequestedNewNodeId;
    UA_QualifiedName   BrowseName;
    OpcUa_NodeClass    NodeClass;
    UA_ExtensionObject NodeAttributes;
    UA_ExpandedNodeId  TypeDefinition;
}
OpcUa_AddNodesItem;

void OpcUa_AddNodesItem_Initialize(OpcUa_AddNodesItem* pValue);

void OpcUa_AddNodesItem_Clear(OpcUa_AddNodesItem* pValue);

//StatusCode OpcUa_AddNodesItem_GetSize(OpcUa_AddNodesItem* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_AddNodesItem_Encode(OpcUa_AddNodesItem* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_AddNodesItem_Decode(OpcUa_AddNodesItem* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_AddNodesItem_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_AddNodesResult
/*============================================================================
 * The AddNodesResult structure.
 *===========================================================================*/
typedef struct _OpcUa_AddNodesResult
{
    SOPC_StatusCode StatusCode;
    UA_NodeId  AddedNodeId;
}
OpcUa_AddNodesResult;

void OpcUa_AddNodesResult_Initialize(OpcUa_AddNodesResult* pValue);

void OpcUa_AddNodesResult_Clear(OpcUa_AddNodesResult* pValue);

//StatusCode OpcUa_AddNodesResult_GetSize(OpcUa_AddNodesResult* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_AddNodesResult_Encode(OpcUa_AddNodesResult* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_AddNodesResult_Decode(OpcUa_AddNodesResult* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_AddNodesResult_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_AddNodes
#ifndef OPCUA_EXCLUDE_AddNodesRequest
/*============================================================================
 * The AddNodesRequest structure.
 *===========================================================================*/
typedef struct _OpcUa_AddNodesRequest
{
    OpcUa_RequestHeader RequestHeader;
    int32_t             NoOfNodesToAdd;
    OpcUa_AddNodesItem* NodesToAdd;
}
OpcUa_AddNodesRequest;

void OpcUa_AddNodesRequest_Initialize(OpcUa_AddNodesRequest* pValue);

void OpcUa_AddNodesRequest_Clear(OpcUa_AddNodesRequest* pValue);

//StatusCode OpcUa_AddNodesRequest_GetSize(OpcUa_AddNodesRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_AddNodesRequest_Encode(OpcUa_AddNodesRequest* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_AddNodesRequest_Decode(OpcUa_AddNodesRequest* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_AddNodesRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_AddNodesResponse
/*============================================================================
 * The AddNodesResponse structure.
 *===========================================================================*/
typedef struct _OpcUa_AddNodesResponse
{
    OpcUa_ResponseHeader  ResponseHeader;
    int32_t               NoOfResults;
    OpcUa_AddNodesResult* Results;
    int32_t               NoOfDiagnosticInfos;
    UA_DiagnosticInfo*    DiagnosticInfos;
}
OpcUa_AddNodesResponse;

void OpcUa_AddNodesResponse_Initialize(OpcUa_AddNodesResponse* pValue);

void OpcUa_AddNodesResponse_Clear(OpcUa_AddNodesResponse* pValue);

//StatusCode OpcUa_AddNodesResponse_GetSize(OpcUa_AddNodesResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_AddNodesResponse_Encode(OpcUa_AddNodesResponse* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_AddNodesResponse_Decode(OpcUa_AddNodesResponse* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_AddNodesResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_AddReferencesItem
/*============================================================================
 * The AddReferencesItem structure.
 *===========================================================================*/
typedef struct _OpcUa_AddReferencesItem
{
    UA_NodeId         SourceNodeId;
    UA_NodeId         ReferenceTypeId;
    UA_Boolean        IsForward;
    UA_String         TargetServerUri;
    UA_ExpandedNodeId TargetNodeId;
    OpcUa_NodeClass   TargetNodeClass;
}
OpcUa_AddReferencesItem;

void OpcUa_AddReferencesItem_Initialize(OpcUa_AddReferencesItem* pValue);

void OpcUa_AddReferencesItem_Clear(OpcUa_AddReferencesItem* pValue);

//StatusCode OpcUa_AddReferencesItem_GetSize(OpcUa_AddReferencesItem* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_AddReferencesItem_Encode(OpcUa_AddReferencesItem* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_AddReferencesItem_Decode(OpcUa_AddReferencesItem* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_AddReferencesItem_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_AddReferences
#ifndef OPCUA_EXCLUDE_AddReferencesRequest
/*============================================================================
 * The AddReferencesRequest structure.
 *===========================================================================*/
typedef struct _OpcUa_AddReferencesRequest
{
    OpcUa_RequestHeader      RequestHeader;
    int32_t                  NoOfReferencesToAdd;
    OpcUa_AddReferencesItem* ReferencesToAdd;
}
OpcUa_AddReferencesRequest;

void OpcUa_AddReferencesRequest_Initialize(OpcUa_AddReferencesRequest* pValue);

void OpcUa_AddReferencesRequest_Clear(OpcUa_AddReferencesRequest* pValue);

//StatusCode OpcUa_AddReferencesRequest_GetSize(OpcUa_AddReferencesRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_AddReferencesRequest_Encode(OpcUa_AddReferencesRequest* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_AddReferencesRequest_Decode(OpcUa_AddReferencesRequest* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_AddReferencesRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_AddReferencesResponse
/*============================================================================
 * The AddReferencesResponse structure.
 *===========================================================================*/
typedef struct _OpcUa_AddReferencesResponse
{
    OpcUa_ResponseHeader ResponseHeader;
    int32_t              NoOfResults;
    SOPC_StatusCode*          Results;
    int32_t              NoOfDiagnosticInfos;
    UA_DiagnosticInfo*   DiagnosticInfos;
}
OpcUa_AddReferencesResponse;

void OpcUa_AddReferencesResponse_Initialize(OpcUa_AddReferencesResponse* pValue);

void OpcUa_AddReferencesResponse_Clear(OpcUa_AddReferencesResponse* pValue);

//StatusCode OpcUa_AddReferencesResponse_GetSize(OpcUa_AddReferencesResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_AddReferencesResponse_Encode(OpcUa_AddReferencesResponse* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_AddReferencesResponse_Decode(OpcUa_AddReferencesResponse* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_AddReferencesResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_DeleteNodesItem
/*============================================================================
 * The DeleteNodesItem structure.
 *===========================================================================*/
typedef struct _OpcUa_DeleteNodesItem
{
    UA_NodeId  NodeId;
    UA_Boolean DeleteTargetReferences;
}
OpcUa_DeleteNodesItem;

void OpcUa_DeleteNodesItem_Initialize(OpcUa_DeleteNodesItem* pValue);

void OpcUa_DeleteNodesItem_Clear(OpcUa_DeleteNodesItem* pValue);

//StatusCode OpcUa_DeleteNodesItem_GetSize(OpcUa_DeleteNodesItem* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_DeleteNodesItem_Encode(OpcUa_DeleteNodesItem* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_DeleteNodesItem_Decode(OpcUa_DeleteNodesItem* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_DeleteNodesItem_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_DeleteNodes
#ifndef OPCUA_EXCLUDE_DeleteNodesRequest
/*============================================================================
 * The DeleteNodesRequest structure.
 *===========================================================================*/
typedef struct _OpcUa_DeleteNodesRequest
{
    OpcUa_RequestHeader    RequestHeader;
    int32_t                NoOfNodesToDelete;
    OpcUa_DeleteNodesItem* NodesToDelete;
}
OpcUa_DeleteNodesRequest;

void OpcUa_DeleteNodesRequest_Initialize(OpcUa_DeleteNodesRequest* pValue);

void OpcUa_DeleteNodesRequest_Clear(OpcUa_DeleteNodesRequest* pValue);

//StatusCode OpcUa_DeleteNodesRequest_GetSize(OpcUa_DeleteNodesRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_DeleteNodesRequest_Encode(OpcUa_DeleteNodesRequest* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_DeleteNodesRequest_Decode(OpcUa_DeleteNodesRequest* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_DeleteNodesRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_DeleteNodesResponse
/*============================================================================
 * The DeleteNodesResponse structure.
 *===========================================================================*/
typedef struct _OpcUa_DeleteNodesResponse
{
    OpcUa_ResponseHeader ResponseHeader;
    int32_t              NoOfResults;
    SOPC_StatusCode*          Results;
    int32_t              NoOfDiagnosticInfos;
    UA_DiagnosticInfo*   DiagnosticInfos;
}
OpcUa_DeleteNodesResponse;

void OpcUa_DeleteNodesResponse_Initialize(OpcUa_DeleteNodesResponse* pValue);

void OpcUa_DeleteNodesResponse_Clear(OpcUa_DeleteNodesResponse* pValue);

//StatusCode OpcUa_DeleteNodesResponse_GetSize(OpcUa_DeleteNodesResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_DeleteNodesResponse_Encode(OpcUa_DeleteNodesResponse* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_DeleteNodesResponse_Decode(OpcUa_DeleteNodesResponse* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_DeleteNodesResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_DeleteReferencesItem
/*============================================================================
 * The DeleteReferencesItem structure.
 *===========================================================================*/
typedef struct _OpcUa_DeleteReferencesItem
{
    UA_NodeId         SourceNodeId;
    UA_NodeId         ReferenceTypeId;
    UA_Boolean        IsForward;
    UA_ExpandedNodeId TargetNodeId;
    UA_Boolean        DeleteBidirectional;
}
OpcUa_DeleteReferencesItem;

void OpcUa_DeleteReferencesItem_Initialize(OpcUa_DeleteReferencesItem* pValue);

void OpcUa_DeleteReferencesItem_Clear(OpcUa_DeleteReferencesItem* pValue);

//StatusCode OpcUa_DeleteReferencesItem_GetSize(OpcUa_DeleteReferencesItem* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_DeleteReferencesItem_Encode(OpcUa_DeleteReferencesItem* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_DeleteReferencesItem_Decode(OpcUa_DeleteReferencesItem* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_DeleteReferencesItem_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_DeleteReferences
#ifndef OPCUA_EXCLUDE_DeleteReferencesRequest
/*============================================================================
 * The DeleteReferencesRequest structure.
 *===========================================================================*/
typedef struct _OpcUa_DeleteReferencesRequest
{
    OpcUa_RequestHeader         RequestHeader;
    int32_t                     NoOfReferencesToDelete;
    OpcUa_DeleteReferencesItem* ReferencesToDelete;
}
OpcUa_DeleteReferencesRequest;

void OpcUa_DeleteReferencesRequest_Initialize(OpcUa_DeleteReferencesRequest* pValue);

void OpcUa_DeleteReferencesRequest_Clear(OpcUa_DeleteReferencesRequest* pValue);

//StatusCode OpcUa_DeleteReferencesRequest_GetSize(OpcUa_DeleteReferencesRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_DeleteReferencesRequest_Encode(OpcUa_DeleteReferencesRequest* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_DeleteReferencesRequest_Decode(OpcUa_DeleteReferencesRequest* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_DeleteReferencesRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_DeleteReferencesResponse
/*============================================================================
 * The DeleteReferencesResponse structure.
 *===========================================================================*/
typedef struct _OpcUa_DeleteReferencesResponse
{
    OpcUa_ResponseHeader ResponseHeader;
    int32_t              NoOfResults;
    SOPC_StatusCode*          Results;
    int32_t              NoOfDiagnosticInfos;
    UA_DiagnosticInfo*   DiagnosticInfos;
}
OpcUa_DeleteReferencesResponse;

void OpcUa_DeleteReferencesResponse_Initialize(OpcUa_DeleteReferencesResponse* pValue);

void OpcUa_DeleteReferencesResponse_Clear(OpcUa_DeleteReferencesResponse* pValue);

//StatusCode OpcUa_DeleteReferencesResponse_GetSize(OpcUa_DeleteReferencesResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_DeleteReferencesResponse_Encode(OpcUa_DeleteReferencesResponse* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_DeleteReferencesResponse_Decode(OpcUa_DeleteReferencesResponse* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_DeleteReferencesResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_AttributeWriteMask
/*============================================================================
 * The AttributeWriteMask enumeration.
 *===========================================================================*/
typedef enum _OpcUa_AttributeWriteMask
{
    OpcUa_AttributeWriteMask_None                    = 0,
    OpcUa_AttributeWriteMask_AccessLevel             = 1,
    OpcUa_AttributeWriteMask_ArrayDimensions         = 2,
    OpcUa_AttributeWriteMask_BrowseName              = 4,
    OpcUa_AttributeWriteMask_ContainsNoLoops         = 8,
    OpcUa_AttributeWriteMask_DataType                = 16,
    OpcUa_AttributeWriteMask_Description             = 32,
    OpcUa_AttributeWriteMask_DisplayName             = 64,
    OpcUa_AttributeWriteMask_EventNotifier           = 128,
    OpcUa_AttributeWriteMask_Executable              = 256,
    OpcUa_AttributeWriteMask_Historizing             = 512,
    OpcUa_AttributeWriteMask_InverseName             = 1024,
    OpcUa_AttributeWriteMask_IsAbstract              = 2048,
    OpcUa_AttributeWriteMask_MinimumSamplingInterval = 4096,
    OpcUa_AttributeWriteMask_NodeClass               = 8192,
    OpcUa_AttributeWriteMask_NodeId                  = 16384,
    OpcUa_AttributeWriteMask_Symmetric               = 32768,
    OpcUa_AttributeWriteMask_UserAccessLevel         = 65536,
    OpcUa_AttributeWriteMask_UserExecutable          = 131072,
    OpcUa_AttributeWriteMask_UserWriteMask           = 262144,
    OpcUa_AttributeWriteMask_ValueRank               = 524288,
    OpcUa_AttributeWriteMask_WriteMask               = 1048576,
    OpcUa_AttributeWriteMask_ValueForVariableType    = 2097152
#if OPCUA_FORCE_INT32_ENUMS
    ,_OpcUa_AttributeWriteMask_MaxEnumerationValue = OpcUa_Int32_Max
#endif
}
OpcUa_AttributeWriteMask;

#endif

#ifndef OPCUA_EXCLUDE_BrowseDirection
/*============================================================================
 * The BrowseDirection enumeration.
 *===========================================================================*/
typedef enum _OpcUa_BrowseDirection
{
    OpcUa_BrowseDirection_Forward = 0,
    OpcUa_BrowseDirection_Inverse = 1,
    OpcUa_BrowseDirection_Both    = 2
#if OPCUA_FORCE_INT32_ENUMS
    ,_OpcUa_BrowseDirection_MaxEnumerationValue = OpcUa_Int32_Max
#endif
}
OpcUa_BrowseDirection;

#endif

#ifndef OPCUA_EXCLUDE_ViewDescription
/*============================================================================
 * The ViewDescription structure.
 *===========================================================================*/
typedef struct _OpcUa_ViewDescription
{
    UA_NodeId   ViewId;
    UA_DateTime Timestamp;
    uint32_t    ViewVersion;
}
OpcUa_ViewDescription;

void OpcUa_ViewDescription_Initialize(OpcUa_ViewDescription* pValue);

void OpcUa_ViewDescription_Clear(OpcUa_ViewDescription* pValue);

//StatusCode OpcUa_ViewDescription_GetSize(OpcUa_ViewDescription* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_ViewDescription_Encode(OpcUa_ViewDescription* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ViewDescription_Decode(OpcUa_ViewDescription* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_ViewDescription_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_BrowseDescription
/*============================================================================
 * The BrowseDescription structure.
 *===========================================================================*/
typedef struct _OpcUa_BrowseDescription
{
    UA_NodeId             NodeId;
    OpcUa_BrowseDirection BrowseDirection;
    UA_NodeId             ReferenceTypeId;
    UA_Boolean            IncludeSubtypes;
    uint32_t              NodeClassMask;
    uint32_t              ResultMask;
}
OpcUa_BrowseDescription;

void OpcUa_BrowseDescription_Initialize(OpcUa_BrowseDescription* pValue);

void OpcUa_BrowseDescription_Clear(OpcUa_BrowseDescription* pValue);

//StatusCode OpcUa_BrowseDescription_GetSize(OpcUa_BrowseDescription* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_BrowseDescription_Encode(OpcUa_BrowseDescription* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_BrowseDescription_Decode(OpcUa_BrowseDescription* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_BrowseDescription_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_BrowseResultMask
/*============================================================================
 * The BrowseResultMask enumeration.
 *===========================================================================*/
typedef enum _OpcUa_BrowseResultMask
{
    OpcUa_BrowseResultMask_None              = 0,
    OpcUa_BrowseResultMask_ReferenceTypeId   = 1,
    OpcUa_BrowseResultMask_IsForward         = 2,
    OpcUa_BrowseResultMask_NodeClass         = 4,
    OpcUa_BrowseResultMask_BrowseName        = 8,
    OpcUa_BrowseResultMask_DisplayName       = 16,
    OpcUa_BrowseResultMask_TypeDefinition    = 32,
    OpcUa_BrowseResultMask_All               = 63,
    OpcUa_BrowseResultMask_ReferenceTypeInfo = 3,
    OpcUa_BrowseResultMask_TargetInfo        = 60
#if OPCUA_FORCE_INT32_ENUMS
    ,_OpcUa_BrowseResultMask_MaxEnumerationValue = OpcUa_Int32_Max
#endif
}
OpcUa_BrowseResultMask;

#endif

#ifndef OPCUA_EXCLUDE_ReferenceDescription
/*============================================================================
 * The ReferenceDescription structure.
 *===========================================================================*/
typedef struct _OpcUa_ReferenceDescription
{
    UA_NodeId         ReferenceTypeId;
    UA_Boolean        IsForward;
    UA_ExpandedNodeId NodeId;
    UA_QualifiedName  BrowseName;
    UA_LocalizedText  DisplayName;
    OpcUa_NodeClass   NodeClass;
    UA_ExpandedNodeId TypeDefinition;
}
OpcUa_ReferenceDescription;

void OpcUa_ReferenceDescription_Initialize(OpcUa_ReferenceDescription* pValue);

void OpcUa_ReferenceDescription_Clear(OpcUa_ReferenceDescription* pValue);

//StatusCode OpcUa_ReferenceDescription_GetSize(OpcUa_ReferenceDescription* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_ReferenceDescription_Encode(OpcUa_ReferenceDescription* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ReferenceDescription_Decode(OpcUa_ReferenceDescription* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_ReferenceDescription_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_BrowseResult
/*============================================================================
 * The BrowseResult structure.
 *===========================================================================*/
typedef struct _OpcUa_BrowseResult
{
    SOPC_StatusCode                  StatusCode;
    UA_ByteString               ContinuationPoint;
    int32_t                     NoOfReferences;
    OpcUa_ReferenceDescription* References;
}
OpcUa_BrowseResult;

void OpcUa_BrowseResult_Initialize(OpcUa_BrowseResult* pValue);

void OpcUa_BrowseResult_Clear(OpcUa_BrowseResult* pValue);

//StatusCode OpcUa_BrowseResult_GetSize(OpcUa_BrowseResult* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_BrowseResult_Encode(OpcUa_BrowseResult* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_BrowseResult_Decode(OpcUa_BrowseResult* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_BrowseResult_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_Browse
#ifndef OPCUA_EXCLUDE_BrowseRequest
/*============================================================================
 * The BrowseRequest structure.
 *===========================================================================*/
typedef struct _OpcUa_BrowseRequest
{
    OpcUa_RequestHeader      RequestHeader;
    OpcUa_ViewDescription    View;
    uint32_t                 RequestedMaxReferencesPerNode;
    int32_t                  NoOfNodesToBrowse;
    OpcUa_BrowseDescription* NodesToBrowse;
}
OpcUa_BrowseRequest;

void OpcUa_BrowseRequest_Initialize(OpcUa_BrowseRequest* pValue);

void OpcUa_BrowseRequest_Clear(OpcUa_BrowseRequest* pValue);

//StatusCode OpcUa_BrowseRequest_GetSize(OpcUa_BrowseRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_BrowseRequest_Encode(OpcUa_BrowseRequest* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_BrowseRequest_Decode(OpcUa_BrowseRequest* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_BrowseRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_BrowseResponse
/*============================================================================
 * The BrowseResponse structure.
 *===========================================================================*/
typedef struct _OpcUa_BrowseResponse
{
    OpcUa_ResponseHeader ResponseHeader;
    int32_t              NoOfResults;
    OpcUa_BrowseResult*  Results;
    int32_t              NoOfDiagnosticInfos;
    UA_DiagnosticInfo*   DiagnosticInfos;
}
OpcUa_BrowseResponse;

void OpcUa_BrowseResponse_Initialize(OpcUa_BrowseResponse* pValue);

void OpcUa_BrowseResponse_Clear(OpcUa_BrowseResponse* pValue);

//StatusCode OpcUa_BrowseResponse_GetSize(OpcUa_BrowseResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_BrowseResponse_Encode(OpcUa_BrowseResponse* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_BrowseResponse_Decode(OpcUa_BrowseResponse* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_BrowseResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_BrowseNext
#ifndef OPCUA_EXCLUDE_BrowseNextRequest
/*============================================================================
 * The BrowseNextRequest structure.
 *===========================================================================*/
typedef struct _OpcUa_BrowseNextRequest
{
    OpcUa_RequestHeader RequestHeader;
    UA_Boolean          ReleaseContinuationPoints;
    int32_t             NoOfContinuationPoints;
    UA_ByteString*      ContinuationPoints;
}
OpcUa_BrowseNextRequest;

void OpcUa_BrowseNextRequest_Initialize(OpcUa_BrowseNextRequest* pValue);

void OpcUa_BrowseNextRequest_Clear(OpcUa_BrowseNextRequest* pValue);

//StatusCode OpcUa_BrowseNextRequest_GetSize(OpcUa_BrowseNextRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_BrowseNextRequest_Encode(OpcUa_BrowseNextRequest* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_BrowseNextRequest_Decode(OpcUa_BrowseNextRequest* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_BrowseNextRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_BrowseNextResponse
/*============================================================================
 * The BrowseNextResponse structure.
 *===========================================================================*/
typedef struct _OpcUa_BrowseNextResponse
{
    OpcUa_ResponseHeader ResponseHeader;
    int32_t              NoOfResults;
    OpcUa_BrowseResult*  Results;
    int32_t              NoOfDiagnosticInfos;
    UA_DiagnosticInfo*   DiagnosticInfos;
}
OpcUa_BrowseNextResponse;

void OpcUa_BrowseNextResponse_Initialize(OpcUa_BrowseNextResponse* pValue);

void OpcUa_BrowseNextResponse_Clear(OpcUa_BrowseNextResponse* pValue);

//StatusCode OpcUa_BrowseNextResponse_GetSize(OpcUa_BrowseNextResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_BrowseNextResponse_Encode(OpcUa_BrowseNextResponse* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_BrowseNextResponse_Decode(OpcUa_BrowseNextResponse* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_BrowseNextResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_RelativePathElement
/*============================================================================
 * The RelativePathElement structure.
 *===========================================================================*/
typedef struct _OpcUa_RelativePathElement
{
    UA_NodeId        ReferenceTypeId;
    UA_Boolean       IsInverse;
    UA_Boolean       IncludeSubtypes;
    UA_QualifiedName TargetName;
}
OpcUa_RelativePathElement;

void OpcUa_RelativePathElement_Initialize(OpcUa_RelativePathElement* pValue);

void OpcUa_RelativePathElement_Clear(OpcUa_RelativePathElement* pValue);

//StatusCode OpcUa_RelativePathElement_GetSize(OpcUa_RelativePathElement* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_RelativePathElement_Encode(OpcUa_RelativePathElement* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_RelativePathElement_Decode(OpcUa_RelativePathElement* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_RelativePathElement_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_RelativePath
/*============================================================================
 * The RelativePath structure.
 *===========================================================================*/
typedef struct _OpcUa_RelativePath
{
    int32_t                    NoOfElements;
    OpcUa_RelativePathElement* Elements;
}
OpcUa_RelativePath;

void OpcUa_RelativePath_Initialize(OpcUa_RelativePath* pValue);

void OpcUa_RelativePath_Clear(OpcUa_RelativePath* pValue);

//StatusCode OpcUa_RelativePath_GetSize(OpcUa_RelativePath* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_RelativePath_Encode(OpcUa_RelativePath* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_RelativePath_Decode(OpcUa_RelativePath* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_RelativePath_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_BrowsePath
/*============================================================================
 * The BrowsePath structure.
 *===========================================================================*/
typedef struct _OpcUa_BrowsePath
{
    UA_NodeId          StartingNode;
    OpcUa_RelativePath RelativePath;
}
OpcUa_BrowsePath;

void OpcUa_BrowsePath_Initialize(OpcUa_BrowsePath* pValue);

void OpcUa_BrowsePath_Clear(OpcUa_BrowsePath* pValue);

//StatusCode OpcUa_BrowsePath_GetSize(OpcUa_BrowsePath* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_BrowsePath_Encode(OpcUa_BrowsePath* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_BrowsePath_Decode(OpcUa_BrowsePath* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_BrowsePath_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_BrowsePathTarget
/*============================================================================
 * The BrowsePathTarget structure.
 *===========================================================================*/
typedef struct _OpcUa_BrowsePathTarget
{
    UA_ExpandedNodeId TargetId;
    uint32_t          RemainingPathIndex;
}
OpcUa_BrowsePathTarget;

void OpcUa_BrowsePathTarget_Initialize(OpcUa_BrowsePathTarget* pValue);

void OpcUa_BrowsePathTarget_Clear(OpcUa_BrowsePathTarget* pValue);

//StatusCode OpcUa_BrowsePathTarget_GetSize(OpcUa_BrowsePathTarget* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_BrowsePathTarget_Encode(OpcUa_BrowsePathTarget* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_BrowsePathTarget_Decode(OpcUa_BrowsePathTarget* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_BrowsePathTarget_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_BrowsePathResult
/*============================================================================
 * The BrowsePathResult structure.
 *===========================================================================*/
typedef struct _OpcUa_BrowsePathResult
{
    SOPC_StatusCode              StatusCode;
    int32_t                 NoOfTargets;
    OpcUa_BrowsePathTarget* Targets;
}
OpcUa_BrowsePathResult;

void OpcUa_BrowsePathResult_Initialize(OpcUa_BrowsePathResult* pValue);

void OpcUa_BrowsePathResult_Clear(OpcUa_BrowsePathResult* pValue);

//StatusCode OpcUa_BrowsePathResult_GetSize(OpcUa_BrowsePathResult* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_BrowsePathResult_Encode(OpcUa_BrowsePathResult* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_BrowsePathResult_Decode(OpcUa_BrowsePathResult* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_BrowsePathResult_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_TranslateBrowsePathsToNodeIds
#ifndef OPCUA_EXCLUDE_TranslateBrowsePathsToNodeIdsRequest
/*============================================================================
 * The TranslateBrowsePathsToNodeIdsRequest structure.
 *===========================================================================*/
typedef struct _OpcUa_TranslateBrowsePathsToNodeIdsRequest
{
    OpcUa_RequestHeader RequestHeader;
    int32_t             NoOfBrowsePaths;
    OpcUa_BrowsePath*   BrowsePaths;
}
OpcUa_TranslateBrowsePathsToNodeIdsRequest;

void OpcUa_TranslateBrowsePathsToNodeIdsRequest_Initialize(OpcUa_TranslateBrowsePathsToNodeIdsRequest* pValue);

void OpcUa_TranslateBrowsePathsToNodeIdsRequest_Clear(OpcUa_TranslateBrowsePathsToNodeIdsRequest* pValue);

//StatusCode OpcUa_TranslateBrowsePathsToNodeIdsRequest_GetSize(OpcUa_TranslateBrowsePathsToNodeIdsRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_TranslateBrowsePathsToNodeIdsRequest_Encode(OpcUa_TranslateBrowsePathsToNodeIdsRequest* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_TranslateBrowsePathsToNodeIdsRequest_Decode(OpcUa_TranslateBrowsePathsToNodeIdsRequest* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_TranslateBrowsePathsToNodeIdsRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_TranslateBrowsePathsToNodeIdsResponse
/*============================================================================
 * The TranslateBrowsePathsToNodeIdsResponse structure.
 *===========================================================================*/
typedef struct _OpcUa_TranslateBrowsePathsToNodeIdsResponse
{
    OpcUa_ResponseHeader    ResponseHeader;
    int32_t                 NoOfResults;
    OpcUa_BrowsePathResult* Results;
    int32_t                 NoOfDiagnosticInfos;
    UA_DiagnosticInfo*      DiagnosticInfos;
}
OpcUa_TranslateBrowsePathsToNodeIdsResponse;

void OpcUa_TranslateBrowsePathsToNodeIdsResponse_Initialize(OpcUa_TranslateBrowsePathsToNodeIdsResponse* pValue);

void OpcUa_TranslateBrowsePathsToNodeIdsResponse_Clear(OpcUa_TranslateBrowsePathsToNodeIdsResponse* pValue);

//StatusCode OpcUa_TranslateBrowsePathsToNodeIdsResponse_GetSize(OpcUa_TranslateBrowsePathsToNodeIdsResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_TranslateBrowsePathsToNodeIdsResponse_Encode(OpcUa_TranslateBrowsePathsToNodeIdsResponse* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_TranslateBrowsePathsToNodeIdsResponse_Decode(OpcUa_TranslateBrowsePathsToNodeIdsResponse* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_TranslateBrowsePathsToNodeIdsResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_RegisterNodes
#ifndef OPCUA_EXCLUDE_RegisterNodesRequest
/*============================================================================
 * The RegisterNodesRequest structure.
 *===========================================================================*/
typedef struct _OpcUa_RegisterNodesRequest
{
    OpcUa_RequestHeader RequestHeader;
    int32_t             NoOfNodesToRegister;
    UA_NodeId*          NodesToRegister;
}
OpcUa_RegisterNodesRequest;

void OpcUa_RegisterNodesRequest_Initialize(OpcUa_RegisterNodesRequest* pValue);

void OpcUa_RegisterNodesRequest_Clear(OpcUa_RegisterNodesRequest* pValue);

//StatusCode OpcUa_RegisterNodesRequest_GetSize(OpcUa_RegisterNodesRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_RegisterNodesRequest_Encode(OpcUa_RegisterNodesRequest* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_RegisterNodesRequest_Decode(OpcUa_RegisterNodesRequest* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_RegisterNodesRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_RegisterNodesResponse
/*============================================================================
 * The RegisterNodesResponse structure.
 *===========================================================================*/
typedef struct _OpcUa_RegisterNodesResponse
{
    OpcUa_ResponseHeader ResponseHeader;
    int32_t              NoOfRegisteredNodeIds;
    UA_NodeId*           RegisteredNodeIds;
}
OpcUa_RegisterNodesResponse;

void OpcUa_RegisterNodesResponse_Initialize(OpcUa_RegisterNodesResponse* pValue);

void OpcUa_RegisterNodesResponse_Clear(OpcUa_RegisterNodesResponse* pValue);

//StatusCode OpcUa_RegisterNodesResponse_GetSize(OpcUa_RegisterNodesResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_RegisterNodesResponse_Encode(OpcUa_RegisterNodesResponse* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_RegisterNodesResponse_Decode(OpcUa_RegisterNodesResponse* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_RegisterNodesResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_UnregisterNodes
#ifndef OPCUA_EXCLUDE_UnregisterNodesRequest
/*============================================================================
 * The UnregisterNodesRequest structure.
 *===========================================================================*/
typedef struct _OpcUa_UnregisterNodesRequest
{
    OpcUa_RequestHeader RequestHeader;
    int32_t             NoOfNodesToUnregister;
    UA_NodeId*          NodesToUnregister;
}
OpcUa_UnregisterNodesRequest;

void OpcUa_UnregisterNodesRequest_Initialize(OpcUa_UnregisterNodesRequest* pValue);

void OpcUa_UnregisterNodesRequest_Clear(OpcUa_UnregisterNodesRequest* pValue);

//StatusCode OpcUa_UnregisterNodesRequest_GetSize(OpcUa_UnregisterNodesRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_UnregisterNodesRequest_Encode(OpcUa_UnregisterNodesRequest* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_UnregisterNodesRequest_Decode(OpcUa_UnregisterNodesRequest* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_UnregisterNodesRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_UnregisterNodesResponse
/*============================================================================
 * The UnregisterNodesResponse structure.
 *===========================================================================*/
typedef struct _OpcUa_UnregisterNodesResponse
{
    OpcUa_ResponseHeader ResponseHeader;
}
OpcUa_UnregisterNodesResponse;

void OpcUa_UnregisterNodesResponse_Initialize(OpcUa_UnregisterNodesResponse* pValue);

void OpcUa_UnregisterNodesResponse_Clear(OpcUa_UnregisterNodesResponse* pValue);

//StatusCode OpcUa_UnregisterNodesResponse_GetSize(OpcUa_UnregisterNodesResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_UnregisterNodesResponse_Encode(OpcUa_UnregisterNodesResponse* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_UnregisterNodesResponse_Decode(OpcUa_UnregisterNodesResponse* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_UnregisterNodesResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_EndpointConfiguration
/*============================================================================
 * The EndpointConfiguration structure.
 *===========================================================================*/
typedef struct _OpcUa_EndpointConfiguration
{
    int32_t    OperationTimeout;
    UA_Boolean UseBinaryEncoding;
    int32_t    MaxStringLength;
    int32_t    MaxByteStringLength;
    int32_t    MaxArrayLength;
    int32_t    MaxMessageSize;
    int32_t    MaxBufferSize;
    int32_t    ChannelLifetime;
    int32_t    SecurityTokenLifetime;
}
OpcUa_EndpointConfiguration;

void OpcUa_EndpointConfiguration_Initialize(OpcUa_EndpointConfiguration* pValue);

void OpcUa_EndpointConfiguration_Clear(OpcUa_EndpointConfiguration* pValue);

//StatusCode OpcUa_EndpointConfiguration_GetSize(OpcUa_EndpointConfiguration* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_EndpointConfiguration_Encode(OpcUa_EndpointConfiguration* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_EndpointConfiguration_Decode(OpcUa_EndpointConfiguration* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_EndpointConfiguration_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ComplianceLevel
/*============================================================================
 * The ComplianceLevel enumeration.
 *===========================================================================*/
typedef enum _OpcUa_ComplianceLevel
{
    OpcUa_ComplianceLevel_Untested   = 0,
    OpcUa_ComplianceLevel_Partial    = 1,
    OpcUa_ComplianceLevel_SelfTested = 2,
    OpcUa_ComplianceLevel_Certified  = 3
#if OPCUA_FORCE_INT32_ENUMS
    ,_OpcUa_ComplianceLevel_MaxEnumerationValue = OpcUa_Int32_Max
#endif
}
OpcUa_ComplianceLevel;

#endif

#ifndef OPCUA_EXCLUDE_SupportedProfile
/*============================================================================
 * The SupportedProfile structure.
 *===========================================================================*/
typedef struct _OpcUa_SupportedProfile
{
    UA_String             OrganizationUri;
    UA_String             ProfileId;
    UA_String             ComplianceTool;
    UA_DateTime           ComplianceDate;
    OpcUa_ComplianceLevel ComplianceLevel;
    int32_t               NoOfUnsupportedUnitIds;
    UA_String*            UnsupportedUnitIds;
}
OpcUa_SupportedProfile;

void OpcUa_SupportedProfile_Initialize(OpcUa_SupportedProfile* pValue);

void OpcUa_SupportedProfile_Clear(OpcUa_SupportedProfile* pValue);

//StatusCode OpcUa_SupportedProfile_GetSize(OpcUa_SupportedProfile* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_SupportedProfile_Encode(OpcUa_SupportedProfile* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_SupportedProfile_Decode(OpcUa_SupportedProfile* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_SupportedProfile_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_SoftwareCertificate
/*============================================================================
 * The SoftwareCertificate structure.
 *===========================================================================*/
typedef struct _OpcUa_SoftwareCertificate
{
    UA_String               ProductName;
    UA_String               ProductUri;
    UA_String               VendorName;
    UA_ByteString           VendorProductCertificate;
    UA_String               SoftwareVersion;
    UA_String               BuildNumber;
    UA_DateTime             BuildDate;
    UA_String               IssuedBy;
    UA_DateTime             IssueDate;
    int32_t                 NoOfSupportedProfiles;
    OpcUa_SupportedProfile* SupportedProfiles;
}
OpcUa_SoftwareCertificate;

void OpcUa_SoftwareCertificate_Initialize(OpcUa_SoftwareCertificate* pValue);

void OpcUa_SoftwareCertificate_Clear(OpcUa_SoftwareCertificate* pValue);

//StatusCode OpcUa_SoftwareCertificate_GetSize(OpcUa_SoftwareCertificate* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_SoftwareCertificate_Encode(OpcUa_SoftwareCertificate* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_SoftwareCertificate_Decode(OpcUa_SoftwareCertificate* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_SoftwareCertificate_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_QueryDataDescription
/*============================================================================
 * The QueryDataDescription structure.
 *===========================================================================*/
typedef struct _OpcUa_QueryDataDescription
{
    OpcUa_RelativePath RelativePath;
    uint32_t           AttributeId;
    UA_String          IndexRange;
}
OpcUa_QueryDataDescription;

void OpcUa_QueryDataDescription_Initialize(OpcUa_QueryDataDescription* pValue);

void OpcUa_QueryDataDescription_Clear(OpcUa_QueryDataDescription* pValue);

//StatusCode OpcUa_QueryDataDescription_GetSize(OpcUa_QueryDataDescription* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_QueryDataDescription_Encode(OpcUa_QueryDataDescription* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_QueryDataDescription_Decode(OpcUa_QueryDataDescription* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_QueryDataDescription_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_NodeTypeDescription
/*============================================================================
 * The NodeTypeDescription structure.
 *===========================================================================*/
typedef struct _OpcUa_NodeTypeDescription
{
    UA_ExpandedNodeId           TypeDefinitionNode;
    UA_Boolean                  IncludeSubTypes;
    int32_t                     NoOfDataToReturn;
    OpcUa_QueryDataDescription* DataToReturn;
}
OpcUa_NodeTypeDescription;

void OpcUa_NodeTypeDescription_Initialize(OpcUa_NodeTypeDescription* pValue);

void OpcUa_NodeTypeDescription_Clear(OpcUa_NodeTypeDescription* pValue);

//StatusCode OpcUa_NodeTypeDescription_GetSize(OpcUa_NodeTypeDescription* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_NodeTypeDescription_Encode(OpcUa_NodeTypeDescription* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_NodeTypeDescription_Decode(OpcUa_NodeTypeDescription* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_NodeTypeDescription_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_FilterOperator
/*============================================================================
 * The FilterOperator enumeration.
 *===========================================================================*/
typedef enum _OpcUa_FilterOperator
{
    OpcUa_FilterOperator_Equals             = 0,
    OpcUa_FilterOperator_IsNull             = 1,
    OpcUa_FilterOperator_GreaterThan        = 2,
    OpcUa_FilterOperator_LessThan           = 3,
    OpcUa_FilterOperator_GreaterThanOrEqual = 4,
    OpcUa_FilterOperator_LessThanOrEqual    = 5,
    OpcUa_FilterOperator_Like               = 6,
    OpcUa_FilterOperator_Not                = 7,
    OpcUa_FilterOperator_Between            = 8,
    OpcUa_FilterOperator_InList             = 9,
    OpcUa_FilterOperator_And                = 10,
    OpcUa_FilterOperator_Or                 = 11,
    OpcUa_FilterOperator_Cast               = 12,
    OpcUa_FilterOperator_InView             = 13,
    OpcUa_FilterOperator_OfType             = 14,
    OpcUa_FilterOperator_RelatedTo          = 15,
    OpcUa_FilterOperator_BitwiseAnd         = 16,
    OpcUa_FilterOperator_BitwiseOr          = 17
#if OPCUA_FORCE_INT32_ENUMS
    ,_OpcUa_FilterOperator_MaxEnumerationValue = OpcUa_Int32_Max
#endif
}
OpcUa_FilterOperator;

#endif

#ifndef OPCUA_EXCLUDE_QueryDataSet
/*============================================================================
 * The QueryDataSet structure.
 *===========================================================================*/
typedef struct _OpcUa_QueryDataSet
{
    UA_ExpandedNodeId NodeId;
    UA_ExpandedNodeId TypeDefinitionNode;
    int32_t           NoOfValues;
    UA_Variant*       Values;
}
OpcUa_QueryDataSet;

void OpcUa_QueryDataSet_Initialize(OpcUa_QueryDataSet* pValue);

void OpcUa_QueryDataSet_Clear(OpcUa_QueryDataSet* pValue);

//StatusCode OpcUa_QueryDataSet_GetSize(OpcUa_QueryDataSet* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_QueryDataSet_Encode(OpcUa_QueryDataSet* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_QueryDataSet_Decode(OpcUa_QueryDataSet* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_QueryDataSet_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_NodeReference
/*============================================================================
 * The NodeReference structure.
 *===========================================================================*/
typedef struct _OpcUa_NodeReference
{
    UA_NodeId  NodeId;
    UA_NodeId  ReferenceTypeId;
    UA_Boolean IsForward;
    int32_t    NoOfReferencedNodeIds;
    UA_NodeId* ReferencedNodeIds;
}
OpcUa_NodeReference;

void OpcUa_NodeReference_Initialize(OpcUa_NodeReference* pValue);

void OpcUa_NodeReference_Clear(OpcUa_NodeReference* pValue);

//StatusCode OpcUa_NodeReference_GetSize(OpcUa_NodeReference* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_NodeReference_Encode(OpcUa_NodeReference* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_NodeReference_Decode(OpcUa_NodeReference* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_NodeReference_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ContentFilterElement
/*============================================================================
 * The ContentFilterElement structure.
 *===========================================================================*/
typedef struct _OpcUa_ContentFilterElement
{
    OpcUa_FilterOperator FilterOperator;
    int32_t              NoOfFilterOperands;
    UA_ExtensionObject*  FilterOperands;
}
OpcUa_ContentFilterElement;

void OpcUa_ContentFilterElement_Initialize(OpcUa_ContentFilterElement* pValue);

void OpcUa_ContentFilterElement_Clear(OpcUa_ContentFilterElement* pValue);

//StatusCode OpcUa_ContentFilterElement_GetSize(OpcUa_ContentFilterElement* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_ContentFilterElement_Encode(OpcUa_ContentFilterElement* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ContentFilterElement_Decode(OpcUa_ContentFilterElement* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_ContentFilterElement_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ContentFilter
/*============================================================================
 * The ContentFilter structure.
 *===========================================================================*/
typedef struct _OpcUa_ContentFilter
{
    int32_t                     NoOfElements;
    OpcUa_ContentFilterElement* Elements;
}
OpcUa_ContentFilter;

void OpcUa_ContentFilter_Initialize(OpcUa_ContentFilter* pValue);

void OpcUa_ContentFilter_Clear(OpcUa_ContentFilter* pValue);

//StatusCode OpcUa_ContentFilter_GetSize(OpcUa_ContentFilter* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_ContentFilter_Encode(OpcUa_ContentFilter* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ContentFilter_Decode(OpcUa_ContentFilter* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_ContentFilter_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ElementOperand
/*============================================================================
 * The ElementOperand structure.
 *===========================================================================*/
typedef struct _OpcUa_ElementOperand
{
    uint32_t Index;
}
OpcUa_ElementOperand;

void OpcUa_ElementOperand_Initialize(OpcUa_ElementOperand* pValue);

void OpcUa_ElementOperand_Clear(OpcUa_ElementOperand* pValue);

//StatusCode OpcUa_ElementOperand_GetSize(OpcUa_ElementOperand* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_ElementOperand_Encode(OpcUa_ElementOperand* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ElementOperand_Decode(OpcUa_ElementOperand* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_ElementOperand_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_LiteralOperand
/*============================================================================
 * The LiteralOperand structure.
 *===========================================================================*/
typedef struct _OpcUa_LiteralOperand
{
    UA_Variant Value;
}
OpcUa_LiteralOperand;

void OpcUa_LiteralOperand_Initialize(OpcUa_LiteralOperand* pValue);

void OpcUa_LiteralOperand_Clear(OpcUa_LiteralOperand* pValue);

//StatusCode OpcUa_LiteralOperand_GetSize(OpcUa_LiteralOperand* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_LiteralOperand_Encode(OpcUa_LiteralOperand* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_LiteralOperand_Decode(OpcUa_LiteralOperand* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_LiteralOperand_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_AttributeOperand
/*============================================================================
 * The AttributeOperand structure.
 *===========================================================================*/
typedef struct _OpcUa_AttributeOperand
{
    UA_NodeId          NodeId;
    UA_String          Alias;
    OpcUa_RelativePath BrowsePath;
    uint32_t           AttributeId;
    UA_String          IndexRange;
}
OpcUa_AttributeOperand;

void OpcUa_AttributeOperand_Initialize(OpcUa_AttributeOperand* pValue);

void OpcUa_AttributeOperand_Clear(OpcUa_AttributeOperand* pValue);

//StatusCode OpcUa_AttributeOperand_GetSize(OpcUa_AttributeOperand* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_AttributeOperand_Encode(OpcUa_AttributeOperand* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_AttributeOperand_Decode(OpcUa_AttributeOperand* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_AttributeOperand_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_SimpleAttributeOperand
/*============================================================================
 * The SimpleAttributeOperand structure.
 *===========================================================================*/
typedef struct _OpcUa_SimpleAttributeOperand
{
    UA_NodeId         TypeDefinitionId;
    int32_t           NoOfBrowsePath;
    UA_QualifiedName* BrowsePath;
    uint32_t          AttributeId;
    UA_String         IndexRange;
}
OpcUa_SimpleAttributeOperand;

void OpcUa_SimpleAttributeOperand_Initialize(OpcUa_SimpleAttributeOperand* pValue);

void OpcUa_SimpleAttributeOperand_Clear(OpcUa_SimpleAttributeOperand* pValue);

//StatusCode OpcUa_SimpleAttributeOperand_GetSize(OpcUa_SimpleAttributeOperand* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_SimpleAttributeOperand_Encode(OpcUa_SimpleAttributeOperand* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_SimpleAttributeOperand_Decode(OpcUa_SimpleAttributeOperand* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_SimpleAttributeOperand_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ContentFilterElementResult
/*============================================================================
 * The ContentFilterElementResult structure.
 *===========================================================================*/
typedef struct _OpcUa_ContentFilterElementResult
{
    SOPC_StatusCode         StatusCode;
    int32_t            NoOfOperandStatusCodes;
    SOPC_StatusCode*        OperandStatusCodes;
    int32_t            NoOfOperandDiagnosticInfos;
    UA_DiagnosticInfo* OperandDiagnosticInfos;
}
OpcUa_ContentFilterElementResult;

void OpcUa_ContentFilterElementResult_Initialize(OpcUa_ContentFilterElementResult* pValue);

void OpcUa_ContentFilterElementResult_Clear(OpcUa_ContentFilterElementResult* pValue);

//StatusCode OpcUa_ContentFilterElementResult_GetSize(OpcUa_ContentFilterElementResult* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_ContentFilterElementResult_Encode(OpcUa_ContentFilterElementResult* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ContentFilterElementResult_Decode(OpcUa_ContentFilterElementResult* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_ContentFilterElementResult_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ContentFilterResult
/*============================================================================
 * The ContentFilterResult structure.
 *===========================================================================*/
typedef struct _OpcUa_ContentFilterResult
{
    int32_t                           NoOfElementResults;
    OpcUa_ContentFilterElementResult* ElementResults;
    int32_t                           NoOfElementDiagnosticInfos;
    UA_DiagnosticInfo*                ElementDiagnosticInfos;
}
OpcUa_ContentFilterResult;

void OpcUa_ContentFilterResult_Initialize(OpcUa_ContentFilterResult* pValue);

void OpcUa_ContentFilterResult_Clear(OpcUa_ContentFilterResult* pValue);

//StatusCode OpcUa_ContentFilterResult_GetSize(OpcUa_ContentFilterResult* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_ContentFilterResult_Encode(OpcUa_ContentFilterResult* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ContentFilterResult_Decode(OpcUa_ContentFilterResult* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_ContentFilterResult_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ParsingResult
/*============================================================================
 * The ParsingResult structure.
 *===========================================================================*/
typedef struct _OpcUa_ParsingResult
{
    SOPC_StatusCode         StatusCode;
    int32_t            NoOfDataStatusCodes;
    SOPC_StatusCode*        DataStatusCodes;
    int32_t            NoOfDataDiagnosticInfos;
    UA_DiagnosticInfo* DataDiagnosticInfos;
}
OpcUa_ParsingResult;

void OpcUa_ParsingResult_Initialize(OpcUa_ParsingResult* pValue);

void OpcUa_ParsingResult_Clear(OpcUa_ParsingResult* pValue);

//StatusCode OpcUa_ParsingResult_GetSize(OpcUa_ParsingResult* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_ParsingResult_Encode(OpcUa_ParsingResult* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ParsingResult_Decode(OpcUa_ParsingResult* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_ParsingResult_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_QueryFirst
#ifndef OPCUA_EXCLUDE_QueryFirstRequest
/*============================================================================
 * The QueryFirstRequest structure.
 *===========================================================================*/
typedef struct _OpcUa_QueryFirstRequest
{
    OpcUa_RequestHeader        RequestHeader;
    OpcUa_ViewDescription      View;
    int32_t                    NoOfNodeTypes;
    OpcUa_NodeTypeDescription* NodeTypes;
    OpcUa_ContentFilter        Filter;
    uint32_t                   MaxDataSetsToReturn;
    uint32_t                   MaxReferencesToReturn;
}
OpcUa_QueryFirstRequest;

void OpcUa_QueryFirstRequest_Initialize(OpcUa_QueryFirstRequest* pValue);

void OpcUa_QueryFirstRequest_Clear(OpcUa_QueryFirstRequest* pValue);

//StatusCode OpcUa_QueryFirstRequest_GetSize(OpcUa_QueryFirstRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_QueryFirstRequest_Encode(OpcUa_QueryFirstRequest* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_QueryFirstRequest_Decode(OpcUa_QueryFirstRequest* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_QueryFirstRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_QueryFirstResponse
/*============================================================================
 * The QueryFirstResponse structure.
 *===========================================================================*/
typedef struct _OpcUa_QueryFirstResponse
{
    OpcUa_ResponseHeader      ResponseHeader;
    int32_t                   NoOfQueryDataSets;
    OpcUa_QueryDataSet*       QueryDataSets;
    UA_ByteString             ContinuationPoint;
    int32_t                   NoOfParsingResults;
    OpcUa_ParsingResult*      ParsingResults;
    int32_t                   NoOfDiagnosticInfos;
    UA_DiagnosticInfo*        DiagnosticInfos;
    OpcUa_ContentFilterResult FilterResult;
}
OpcUa_QueryFirstResponse;

void OpcUa_QueryFirstResponse_Initialize(OpcUa_QueryFirstResponse* pValue);

void OpcUa_QueryFirstResponse_Clear(OpcUa_QueryFirstResponse* pValue);

//StatusCode OpcUa_QueryFirstResponse_GetSize(OpcUa_QueryFirstResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_QueryFirstResponse_Encode(OpcUa_QueryFirstResponse* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_QueryFirstResponse_Decode(OpcUa_QueryFirstResponse* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_QueryFirstResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_QueryNext
#ifndef OPCUA_EXCLUDE_QueryNextRequest
/*============================================================================
 * The QueryNextRequest structure.
 *===========================================================================*/
typedef struct _OpcUa_QueryNextRequest
{
    OpcUa_RequestHeader RequestHeader;
    UA_Boolean          ReleaseContinuationPoint;
    UA_ByteString       ContinuationPoint;
}
OpcUa_QueryNextRequest;

void OpcUa_QueryNextRequest_Initialize(OpcUa_QueryNextRequest* pValue);

void OpcUa_QueryNextRequest_Clear(OpcUa_QueryNextRequest* pValue);

//StatusCode OpcUa_QueryNextRequest_GetSize(OpcUa_QueryNextRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_QueryNextRequest_Encode(OpcUa_QueryNextRequest* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_QueryNextRequest_Decode(OpcUa_QueryNextRequest* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_QueryNextRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_QueryNextResponse
/*============================================================================
 * The QueryNextResponse structure.
 *===========================================================================*/
typedef struct _OpcUa_QueryNextResponse
{
    OpcUa_ResponseHeader ResponseHeader;
    int32_t              NoOfQueryDataSets;
    OpcUa_QueryDataSet*  QueryDataSets;
    UA_ByteString        RevisedContinuationPoint;
}
OpcUa_QueryNextResponse;

void OpcUa_QueryNextResponse_Initialize(OpcUa_QueryNextResponse* pValue);

void OpcUa_QueryNextResponse_Clear(OpcUa_QueryNextResponse* pValue);

//StatusCode OpcUa_QueryNextResponse_GetSize(OpcUa_QueryNextResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_QueryNextResponse_Encode(OpcUa_QueryNextResponse* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_QueryNextResponse_Decode(OpcUa_QueryNextResponse* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_QueryNextResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_TimestampsToReturn
/*============================================================================
 * The TimestampsToReturn enumeration.
 *===========================================================================*/
typedef enum _OpcUa_TimestampsToReturn
{
    OpcUa_TimestampsToReturn_Source  = 0,
    OpcUa_TimestampsToReturn_Server  = 1,
    OpcUa_TimestampsToReturn_Both    = 2,
    OpcUa_TimestampsToReturn_Neither = 3
#if OPCUA_FORCE_INT32_ENUMS
    ,_OpcUa_TimestampsToReturn_MaxEnumerationValue = OpcUa_Int32_Max
#endif
}
OpcUa_TimestampsToReturn;

#endif

#ifndef OPCUA_EXCLUDE_ReadValueId
/*============================================================================
 * The ReadValueId structure.
 *===========================================================================*/
typedef struct _OpcUa_ReadValueId
{
    UA_NodeId        NodeId;
    uint32_t         AttributeId;
    UA_String        IndexRange;
    UA_QualifiedName DataEncoding;
}
OpcUa_ReadValueId;

void OpcUa_ReadValueId_Initialize(OpcUa_ReadValueId* pValue);

void OpcUa_ReadValueId_Clear(OpcUa_ReadValueId* pValue);

//StatusCode OpcUa_ReadValueId_GetSize(OpcUa_ReadValueId* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_ReadValueId_Encode(OpcUa_ReadValueId* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ReadValueId_Decode(OpcUa_ReadValueId* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_ReadValueId_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_Read
#ifndef OPCUA_EXCLUDE_ReadRequest
/*============================================================================
 * The ReadRequest structure.
 *===========================================================================*/
typedef struct _OpcUa_ReadRequest
{
    OpcUa_RequestHeader      RequestHeader;
    double                   MaxAge;
    OpcUa_TimestampsToReturn TimestampsToReturn;
    int32_t                  NoOfNodesToRead;
    OpcUa_ReadValueId*       NodesToRead;
}
OpcUa_ReadRequest;

void OpcUa_ReadRequest_Initialize(OpcUa_ReadRequest* pValue);

void OpcUa_ReadRequest_Clear(OpcUa_ReadRequest* pValue);

//StatusCode OpcUa_ReadRequest_GetSize(OpcUa_ReadRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_ReadRequest_Encode(OpcUa_ReadRequest* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ReadRequest_Decode(OpcUa_ReadRequest* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_ReadRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ReadResponse
/*============================================================================
 * The ReadResponse structure.
 *===========================================================================*/
typedef struct _OpcUa_ReadResponse
{
    OpcUa_ResponseHeader ResponseHeader;
    int32_t              NoOfResults;
    UA_DataValue*        Results;
    int32_t              NoOfDiagnosticInfos;
    UA_DiagnosticInfo*   DiagnosticInfos;
}
OpcUa_ReadResponse;

void OpcUa_ReadResponse_Initialize(OpcUa_ReadResponse* pValue);

void OpcUa_ReadResponse_Clear(OpcUa_ReadResponse* pValue);

//StatusCode OpcUa_ReadResponse_GetSize(OpcUa_ReadResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_ReadResponse_Encode(OpcUa_ReadResponse* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ReadResponse_Decode(OpcUa_ReadResponse* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_ReadResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_HistoryReadValueId
/*============================================================================
 * The HistoryReadValueId structure.
 *===========================================================================*/
typedef struct _OpcUa_HistoryReadValueId
{
    UA_NodeId        NodeId;
    UA_String        IndexRange;
    UA_QualifiedName DataEncoding;
    UA_ByteString    ContinuationPoint;
}
OpcUa_HistoryReadValueId;

void OpcUa_HistoryReadValueId_Initialize(OpcUa_HistoryReadValueId* pValue);

void OpcUa_HistoryReadValueId_Clear(OpcUa_HistoryReadValueId* pValue);

//StatusCode OpcUa_HistoryReadValueId_GetSize(OpcUa_HistoryReadValueId* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_HistoryReadValueId_Encode(OpcUa_HistoryReadValueId* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_HistoryReadValueId_Decode(OpcUa_HistoryReadValueId* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_HistoryReadValueId_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_HistoryReadResult
/*============================================================================
 * The HistoryReadResult structure.
 *===========================================================================*/
typedef struct _OpcUa_HistoryReadResult
{
    SOPC_StatusCode         StatusCode;
    UA_ByteString      ContinuationPoint;
    UA_ExtensionObject HistoryData;
}
OpcUa_HistoryReadResult;

void OpcUa_HistoryReadResult_Initialize(OpcUa_HistoryReadResult* pValue);

void OpcUa_HistoryReadResult_Clear(OpcUa_HistoryReadResult* pValue);

//StatusCode OpcUa_HistoryReadResult_GetSize(OpcUa_HistoryReadResult* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_HistoryReadResult_Encode(OpcUa_HistoryReadResult* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_HistoryReadResult_Decode(OpcUa_HistoryReadResult* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_HistoryReadResult_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_EventFilter
/*============================================================================
 * The EventFilter structure.
 *===========================================================================*/
typedef struct _OpcUa_EventFilter
{
    int32_t                       NoOfSelectClauses;
    OpcUa_SimpleAttributeOperand* SelectClauses;
    OpcUa_ContentFilter           WhereClause;
}
OpcUa_EventFilter;

void OpcUa_EventFilter_Initialize(OpcUa_EventFilter* pValue);

void OpcUa_EventFilter_Clear(OpcUa_EventFilter* pValue);

//StatusCode OpcUa_EventFilter_GetSize(OpcUa_EventFilter* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_EventFilter_Encode(OpcUa_EventFilter* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_EventFilter_Decode(OpcUa_EventFilter* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_EventFilter_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ReadEventDetails
/*============================================================================
 * The ReadEventDetails structure.
 *===========================================================================*/
typedef struct _OpcUa_ReadEventDetails
{
    uint32_t          NumValuesPerNode;
    UA_DateTime       StartTime;
    UA_DateTime       EndTime;
    OpcUa_EventFilter Filter;
}
OpcUa_ReadEventDetails;

void OpcUa_ReadEventDetails_Initialize(OpcUa_ReadEventDetails* pValue);

void OpcUa_ReadEventDetails_Clear(OpcUa_ReadEventDetails* pValue);

//StatusCode OpcUa_ReadEventDetails_GetSize(OpcUa_ReadEventDetails* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_ReadEventDetails_Encode(OpcUa_ReadEventDetails* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ReadEventDetails_Decode(OpcUa_ReadEventDetails* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_ReadEventDetails_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ReadRawModifiedDetails
/*============================================================================
 * The ReadRawModifiedDetails structure.
 *===========================================================================*/
typedef struct _OpcUa_ReadRawModifiedDetails
{
    UA_Boolean  IsReadModified;
    UA_DateTime StartTime;
    UA_DateTime EndTime;
    uint32_t    NumValuesPerNode;
    UA_Boolean  ReturnBounds;
}
OpcUa_ReadRawModifiedDetails;

void OpcUa_ReadRawModifiedDetails_Initialize(OpcUa_ReadRawModifiedDetails* pValue);

void OpcUa_ReadRawModifiedDetails_Clear(OpcUa_ReadRawModifiedDetails* pValue);

//StatusCode OpcUa_ReadRawModifiedDetails_GetSize(OpcUa_ReadRawModifiedDetails* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_ReadRawModifiedDetails_Encode(OpcUa_ReadRawModifiedDetails* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ReadRawModifiedDetails_Decode(OpcUa_ReadRawModifiedDetails* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_ReadRawModifiedDetails_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_AggregateConfiguration
/*============================================================================
 * The AggregateConfiguration structure.
 *===========================================================================*/
typedef struct _OpcUa_AggregateConfiguration
{
    UA_Boolean UseServerCapabilitiesDefaults;
    UA_Boolean TreatUncertainAsBad;
    UA_Byte    PercentDataBad;
    UA_Byte    PercentDataGood;
    UA_Boolean UseSlopedExtrapolation;
}
OpcUa_AggregateConfiguration;

void OpcUa_AggregateConfiguration_Initialize(OpcUa_AggregateConfiguration* pValue);

void OpcUa_AggregateConfiguration_Clear(OpcUa_AggregateConfiguration* pValue);

//StatusCode OpcUa_AggregateConfiguration_GetSize(OpcUa_AggregateConfiguration* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_AggregateConfiguration_Encode(OpcUa_AggregateConfiguration* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_AggregateConfiguration_Decode(OpcUa_AggregateConfiguration* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_AggregateConfiguration_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ReadProcessedDetails
/*============================================================================
 * The ReadProcessedDetails structure.
 *===========================================================================*/
typedef struct _OpcUa_ReadProcessedDetails
{
    UA_DateTime                  StartTime;
    UA_DateTime                  EndTime;
    double                       ProcessingInterval;
    int32_t                      NoOfAggregateType;
    UA_NodeId*                   AggregateType;
    OpcUa_AggregateConfiguration AggregateConfiguration;
}
OpcUa_ReadProcessedDetails;

void OpcUa_ReadProcessedDetails_Initialize(OpcUa_ReadProcessedDetails* pValue);

void OpcUa_ReadProcessedDetails_Clear(OpcUa_ReadProcessedDetails* pValue);

//StatusCode OpcUa_ReadProcessedDetails_GetSize(OpcUa_ReadProcessedDetails* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_ReadProcessedDetails_Encode(OpcUa_ReadProcessedDetails* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ReadProcessedDetails_Decode(OpcUa_ReadProcessedDetails* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_ReadProcessedDetails_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ReadAtTimeDetails
/*============================================================================
 * The ReadAtTimeDetails structure.
 *===========================================================================*/
typedef struct _OpcUa_ReadAtTimeDetails
{
    int32_t      NoOfReqTimes;
    UA_DateTime* ReqTimes;
    UA_Boolean   UseSimpleBounds;
}
OpcUa_ReadAtTimeDetails;

void OpcUa_ReadAtTimeDetails_Initialize(OpcUa_ReadAtTimeDetails* pValue);

void OpcUa_ReadAtTimeDetails_Clear(OpcUa_ReadAtTimeDetails* pValue);

//StatusCode OpcUa_ReadAtTimeDetails_GetSize(OpcUa_ReadAtTimeDetails* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_ReadAtTimeDetails_Encode(OpcUa_ReadAtTimeDetails* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ReadAtTimeDetails_Decode(OpcUa_ReadAtTimeDetails* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_ReadAtTimeDetails_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_HistoryData
/*============================================================================
 * The HistoryData structure.
 *===========================================================================*/
typedef struct _OpcUa_HistoryData
{
    int32_t       NoOfDataValues;
    UA_DataValue* DataValues;
}
OpcUa_HistoryData;

void OpcUa_HistoryData_Initialize(OpcUa_HistoryData* pValue);

void OpcUa_HistoryData_Clear(OpcUa_HistoryData* pValue);

//StatusCode OpcUa_HistoryData_GetSize(OpcUa_HistoryData* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_HistoryData_Encode(OpcUa_HistoryData* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_HistoryData_Decode(OpcUa_HistoryData* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_HistoryData_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_HistoryUpdateType
/*============================================================================
 * The HistoryUpdateType enumeration.
 *===========================================================================*/
typedef enum _OpcUa_HistoryUpdateType
{
    OpcUa_HistoryUpdateType_Insert  = 1,
    OpcUa_HistoryUpdateType_Replace = 2,
    OpcUa_HistoryUpdateType_Update  = 3,
    OpcUa_HistoryUpdateType_Delete  = 4
#if OPCUA_FORCE_INT32_ENUMS
    ,_OpcUa_HistoryUpdateType_MaxEnumerationValue = OpcUa_Int32_Max
#endif
}
OpcUa_HistoryUpdateType;

#endif

#ifndef OPCUA_EXCLUDE_ModificationInfo
/*============================================================================
 * The ModificationInfo structure.
 *===========================================================================*/
typedef struct _OpcUa_ModificationInfo
{
    UA_DateTime             ModificationTime;
    OpcUa_HistoryUpdateType UpdateType;
    UA_String               UserName;
}
OpcUa_ModificationInfo;

void OpcUa_ModificationInfo_Initialize(OpcUa_ModificationInfo* pValue);

void OpcUa_ModificationInfo_Clear(OpcUa_ModificationInfo* pValue);

//StatusCode OpcUa_ModificationInfo_GetSize(OpcUa_ModificationInfo* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_ModificationInfo_Encode(OpcUa_ModificationInfo* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ModificationInfo_Decode(OpcUa_ModificationInfo* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_ModificationInfo_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_HistoryModifiedData
/*============================================================================
 * The HistoryModifiedData structure.
 *===========================================================================*/
typedef struct _OpcUa_HistoryModifiedData
{
    int32_t                 NoOfDataValues;
    UA_DataValue*           DataValues;
    int32_t                 NoOfModificationInfos;
    OpcUa_ModificationInfo* ModificationInfos;
}
OpcUa_HistoryModifiedData;

void OpcUa_HistoryModifiedData_Initialize(OpcUa_HistoryModifiedData* pValue);

void OpcUa_HistoryModifiedData_Clear(OpcUa_HistoryModifiedData* pValue);

//StatusCode OpcUa_HistoryModifiedData_GetSize(OpcUa_HistoryModifiedData* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_HistoryModifiedData_Encode(OpcUa_HistoryModifiedData* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_HistoryModifiedData_Decode(OpcUa_HistoryModifiedData* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_HistoryModifiedData_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_HistoryEventFieldList
/*============================================================================
 * The HistoryEventFieldList structure.
 *===========================================================================*/
typedef struct _OpcUa_HistoryEventFieldList
{
    int32_t     NoOfEventFields;
    UA_Variant* EventFields;
}
OpcUa_HistoryEventFieldList;

void OpcUa_HistoryEventFieldList_Initialize(OpcUa_HistoryEventFieldList* pValue);

void OpcUa_HistoryEventFieldList_Clear(OpcUa_HistoryEventFieldList* pValue);

//StatusCode OpcUa_HistoryEventFieldList_GetSize(OpcUa_HistoryEventFieldList* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_HistoryEventFieldList_Encode(OpcUa_HistoryEventFieldList* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_HistoryEventFieldList_Decode(OpcUa_HistoryEventFieldList* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_HistoryEventFieldList_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_HistoryEvent
/*============================================================================
 * The HistoryEvent structure.
 *===========================================================================*/
typedef struct _OpcUa_HistoryEvent
{
    int32_t                      NoOfEvents;
    OpcUa_HistoryEventFieldList* Events;
}
OpcUa_HistoryEvent;

void OpcUa_HistoryEvent_Initialize(OpcUa_HistoryEvent* pValue);

void OpcUa_HistoryEvent_Clear(OpcUa_HistoryEvent* pValue);

//StatusCode OpcUa_HistoryEvent_GetSize(OpcUa_HistoryEvent* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_HistoryEvent_Encode(OpcUa_HistoryEvent* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_HistoryEvent_Decode(OpcUa_HistoryEvent* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_HistoryEvent_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_HistoryRead
#ifndef OPCUA_EXCLUDE_HistoryReadRequest
/*============================================================================
 * The HistoryReadRequest structure.
 *===========================================================================*/
typedef struct _OpcUa_HistoryReadRequest
{
    OpcUa_RequestHeader       RequestHeader;
    UA_ExtensionObject        HistoryReadDetails;
    OpcUa_TimestampsToReturn  TimestampsToReturn;
    UA_Boolean                ReleaseContinuationPoints;
    int32_t                   NoOfNodesToRead;
    OpcUa_HistoryReadValueId* NodesToRead;
}
OpcUa_HistoryReadRequest;

void OpcUa_HistoryReadRequest_Initialize(OpcUa_HistoryReadRequest* pValue);

void OpcUa_HistoryReadRequest_Clear(OpcUa_HistoryReadRequest* pValue);

//StatusCode OpcUa_HistoryReadRequest_GetSize(OpcUa_HistoryReadRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_HistoryReadRequest_Encode(OpcUa_HistoryReadRequest* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_HistoryReadRequest_Decode(OpcUa_HistoryReadRequest* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_HistoryReadRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_HistoryReadResponse
/*============================================================================
 * The HistoryReadResponse structure.
 *===========================================================================*/
typedef struct _OpcUa_HistoryReadResponse
{
    OpcUa_ResponseHeader     ResponseHeader;
    int32_t                  NoOfResults;
    OpcUa_HistoryReadResult* Results;
    int32_t                  NoOfDiagnosticInfos;
    UA_DiagnosticInfo*       DiagnosticInfos;
}
OpcUa_HistoryReadResponse;

void OpcUa_HistoryReadResponse_Initialize(OpcUa_HistoryReadResponse* pValue);

void OpcUa_HistoryReadResponse_Clear(OpcUa_HistoryReadResponse* pValue);

//StatusCode OpcUa_HistoryReadResponse_GetSize(OpcUa_HistoryReadResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_HistoryReadResponse_Encode(OpcUa_HistoryReadResponse* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_HistoryReadResponse_Decode(OpcUa_HistoryReadResponse* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_HistoryReadResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_WriteValue
/*============================================================================
 * The WriteValue structure.
 *===========================================================================*/
typedef struct _OpcUa_WriteValue
{
    UA_NodeId    NodeId;
    uint32_t     AttributeId;
    UA_String    IndexRange;
    UA_DataValue Value;
}
OpcUa_WriteValue;

void OpcUa_WriteValue_Initialize(OpcUa_WriteValue* pValue);

void OpcUa_WriteValue_Clear(OpcUa_WriteValue* pValue);

//StatusCode OpcUa_WriteValue_GetSize(OpcUa_WriteValue* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_WriteValue_Encode(OpcUa_WriteValue* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_WriteValue_Decode(OpcUa_WriteValue* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_WriteValue_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_Write
#ifndef OPCUA_EXCLUDE_WriteRequest
/*============================================================================
 * The WriteRequest structure.
 *===========================================================================*/
typedef struct _OpcUa_WriteRequest
{
    OpcUa_RequestHeader RequestHeader;
    int32_t             NoOfNodesToWrite;
    OpcUa_WriteValue*   NodesToWrite;
}
OpcUa_WriteRequest;

void OpcUa_WriteRequest_Initialize(OpcUa_WriteRequest* pValue);

void OpcUa_WriteRequest_Clear(OpcUa_WriteRequest* pValue);

//StatusCode OpcUa_WriteRequest_GetSize(OpcUa_WriteRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_WriteRequest_Encode(OpcUa_WriteRequest* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_WriteRequest_Decode(OpcUa_WriteRequest* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_WriteRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_WriteResponse
/*============================================================================
 * The WriteResponse structure.
 *===========================================================================*/
typedef struct _OpcUa_WriteResponse
{
    OpcUa_ResponseHeader ResponseHeader;
    int32_t              NoOfResults;
    SOPC_StatusCode*          Results;
    int32_t              NoOfDiagnosticInfos;
    UA_DiagnosticInfo*   DiagnosticInfos;
}
OpcUa_WriteResponse;

void OpcUa_WriteResponse_Initialize(OpcUa_WriteResponse* pValue);

void OpcUa_WriteResponse_Clear(OpcUa_WriteResponse* pValue);

//StatusCode OpcUa_WriteResponse_GetSize(OpcUa_WriteResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_WriteResponse_Encode(OpcUa_WriteResponse* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_WriteResponse_Decode(OpcUa_WriteResponse* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_WriteResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_HistoryUpdateDetails
/*============================================================================
 * The HistoryUpdateDetails structure.
 *===========================================================================*/
typedef struct _OpcUa_HistoryUpdateDetails
{
    UA_NodeId NodeId;
}
OpcUa_HistoryUpdateDetails;

void OpcUa_HistoryUpdateDetails_Initialize(OpcUa_HistoryUpdateDetails* pValue);

void OpcUa_HistoryUpdateDetails_Clear(OpcUa_HistoryUpdateDetails* pValue);

//StatusCode OpcUa_HistoryUpdateDetails_GetSize(OpcUa_HistoryUpdateDetails* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_HistoryUpdateDetails_Encode(OpcUa_HistoryUpdateDetails* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_HistoryUpdateDetails_Decode(OpcUa_HistoryUpdateDetails* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_HistoryUpdateDetails_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_PerformUpdateType
/*============================================================================
 * The PerformUpdateType enumeration.
 *===========================================================================*/
typedef enum _OpcUa_PerformUpdateType
{
    OpcUa_PerformUpdateType_Insert  = 1,
    OpcUa_PerformUpdateType_Replace = 2,
    OpcUa_PerformUpdateType_Update  = 3,
    OpcUa_PerformUpdateType_Remove  = 4
#if OPCUA_FORCE_INT32_ENUMS
    ,_OpcUa_PerformUpdateType_MaxEnumerationValue = OpcUa_Int32_Max
#endif
}
OpcUa_PerformUpdateType;

#endif

#ifndef OPCUA_EXCLUDE_UpdateDataDetails
/*============================================================================
 * The UpdateDataDetails structure.
 *===========================================================================*/
typedef struct _OpcUa_UpdateDataDetails
{
    UA_NodeId               NodeId;
    OpcUa_PerformUpdateType PerformInsertReplace;
    int32_t                 NoOfUpdateValues;
    UA_DataValue*           UpdateValues;
}
OpcUa_UpdateDataDetails;

void OpcUa_UpdateDataDetails_Initialize(OpcUa_UpdateDataDetails* pValue);

void OpcUa_UpdateDataDetails_Clear(OpcUa_UpdateDataDetails* pValue);

//StatusCode OpcUa_UpdateDataDetails_GetSize(OpcUa_UpdateDataDetails* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_UpdateDataDetails_Encode(OpcUa_UpdateDataDetails* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_UpdateDataDetails_Decode(OpcUa_UpdateDataDetails* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_UpdateDataDetails_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_UpdateStructureDataDetails
/*============================================================================
 * The UpdateStructureDataDetails structure.
 *===========================================================================*/
typedef struct _OpcUa_UpdateStructureDataDetails
{
    UA_NodeId               NodeId;
    OpcUa_PerformUpdateType PerformInsertReplace;
    int32_t                 NoOfUpdateValues;
    UA_DataValue*           UpdateValues;
}
OpcUa_UpdateStructureDataDetails;

void OpcUa_UpdateStructureDataDetails_Initialize(OpcUa_UpdateStructureDataDetails* pValue);

void OpcUa_UpdateStructureDataDetails_Clear(OpcUa_UpdateStructureDataDetails* pValue);

//StatusCode OpcUa_UpdateStructureDataDetails_GetSize(OpcUa_UpdateStructureDataDetails* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_UpdateStructureDataDetails_Encode(OpcUa_UpdateStructureDataDetails* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_UpdateStructureDataDetails_Decode(OpcUa_UpdateStructureDataDetails* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_UpdateStructureDataDetails_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_UpdateEventDetails
/*============================================================================
 * The UpdateEventDetails structure.
 *===========================================================================*/
typedef struct _OpcUa_UpdateEventDetails
{
    UA_NodeId                    NodeId;
    OpcUa_PerformUpdateType      PerformInsertReplace;
    OpcUa_EventFilter            Filter;
    int32_t                      NoOfEventData;
    OpcUa_HistoryEventFieldList* EventData;
}
OpcUa_UpdateEventDetails;

void OpcUa_UpdateEventDetails_Initialize(OpcUa_UpdateEventDetails* pValue);

void OpcUa_UpdateEventDetails_Clear(OpcUa_UpdateEventDetails* pValue);

//StatusCode OpcUa_UpdateEventDetails_GetSize(OpcUa_UpdateEventDetails* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_UpdateEventDetails_Encode(OpcUa_UpdateEventDetails* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_UpdateEventDetails_Decode(OpcUa_UpdateEventDetails* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_UpdateEventDetails_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_DeleteRawModifiedDetails
/*============================================================================
 * The DeleteRawModifiedDetails structure.
 *===========================================================================*/
typedef struct _OpcUa_DeleteRawModifiedDetails
{
    UA_NodeId   NodeId;
    UA_Boolean  IsDeleteModified;
    UA_DateTime StartTime;
    UA_DateTime EndTime;
}
OpcUa_DeleteRawModifiedDetails;

void OpcUa_DeleteRawModifiedDetails_Initialize(OpcUa_DeleteRawModifiedDetails* pValue);

void OpcUa_DeleteRawModifiedDetails_Clear(OpcUa_DeleteRawModifiedDetails* pValue);

//StatusCode OpcUa_DeleteRawModifiedDetails_GetSize(OpcUa_DeleteRawModifiedDetails* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_DeleteRawModifiedDetails_Encode(OpcUa_DeleteRawModifiedDetails* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_DeleteRawModifiedDetails_Decode(OpcUa_DeleteRawModifiedDetails* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_DeleteRawModifiedDetails_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_DeleteAtTimeDetails
/*============================================================================
 * The DeleteAtTimeDetails structure.
 *===========================================================================*/
typedef struct _OpcUa_DeleteAtTimeDetails
{
    UA_NodeId    NodeId;
    int32_t      NoOfReqTimes;
    UA_DateTime* ReqTimes;
}
OpcUa_DeleteAtTimeDetails;

void OpcUa_DeleteAtTimeDetails_Initialize(OpcUa_DeleteAtTimeDetails* pValue);

void OpcUa_DeleteAtTimeDetails_Clear(OpcUa_DeleteAtTimeDetails* pValue);

//StatusCode OpcUa_DeleteAtTimeDetails_GetSize(OpcUa_DeleteAtTimeDetails* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_DeleteAtTimeDetails_Encode(OpcUa_DeleteAtTimeDetails* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_DeleteAtTimeDetails_Decode(OpcUa_DeleteAtTimeDetails* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_DeleteAtTimeDetails_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_DeleteEventDetails
/*============================================================================
 * The DeleteEventDetails structure.
 *===========================================================================*/
typedef struct _OpcUa_DeleteEventDetails
{
    UA_NodeId      NodeId;
    int32_t        NoOfEventIds;
    UA_ByteString* EventIds;
}
OpcUa_DeleteEventDetails;

void OpcUa_DeleteEventDetails_Initialize(OpcUa_DeleteEventDetails* pValue);

void OpcUa_DeleteEventDetails_Clear(OpcUa_DeleteEventDetails* pValue);

//StatusCode OpcUa_DeleteEventDetails_GetSize(OpcUa_DeleteEventDetails* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_DeleteEventDetails_Encode(OpcUa_DeleteEventDetails* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_DeleteEventDetails_Decode(OpcUa_DeleteEventDetails* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_DeleteEventDetails_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_HistoryUpdateResult
/*============================================================================
 * The HistoryUpdateResult structure.
 *===========================================================================*/
typedef struct _OpcUa_HistoryUpdateResult
{
    SOPC_StatusCode         StatusCode;
    int32_t            NoOfOperationResults;
    SOPC_StatusCode*        OperationResults;
    int32_t            NoOfDiagnosticInfos;
    UA_DiagnosticInfo* DiagnosticInfos;
}
OpcUa_HistoryUpdateResult;

void OpcUa_HistoryUpdateResult_Initialize(OpcUa_HistoryUpdateResult* pValue);

void OpcUa_HistoryUpdateResult_Clear(OpcUa_HistoryUpdateResult* pValue);

//StatusCode OpcUa_HistoryUpdateResult_GetSize(OpcUa_HistoryUpdateResult* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_HistoryUpdateResult_Encode(OpcUa_HistoryUpdateResult* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_HistoryUpdateResult_Decode(OpcUa_HistoryUpdateResult* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_HistoryUpdateResult_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_HistoryUpdate
#ifndef OPCUA_EXCLUDE_HistoryUpdateRequest
/*============================================================================
 * The HistoryUpdateRequest structure.
 *===========================================================================*/
typedef struct _OpcUa_HistoryUpdateRequest
{
    OpcUa_RequestHeader RequestHeader;
    int32_t             NoOfHistoryUpdateDetails;
    UA_ExtensionObject* HistoryUpdateDetails;
}
OpcUa_HistoryUpdateRequest;

void OpcUa_HistoryUpdateRequest_Initialize(OpcUa_HistoryUpdateRequest* pValue);

void OpcUa_HistoryUpdateRequest_Clear(OpcUa_HistoryUpdateRequest* pValue);

//StatusCode OpcUa_HistoryUpdateRequest_GetSize(OpcUa_HistoryUpdateRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_HistoryUpdateRequest_Encode(OpcUa_HistoryUpdateRequest* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_HistoryUpdateRequest_Decode(OpcUa_HistoryUpdateRequest* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_HistoryUpdateRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_HistoryUpdateResponse
/*============================================================================
 * The HistoryUpdateResponse structure.
 *===========================================================================*/
typedef struct _OpcUa_HistoryUpdateResponse
{
    OpcUa_ResponseHeader       ResponseHeader;
    int32_t                    NoOfResults;
    OpcUa_HistoryUpdateResult* Results;
    int32_t                    NoOfDiagnosticInfos;
    UA_DiagnosticInfo*         DiagnosticInfos;
}
OpcUa_HistoryUpdateResponse;

void OpcUa_HistoryUpdateResponse_Initialize(OpcUa_HistoryUpdateResponse* pValue);

void OpcUa_HistoryUpdateResponse_Clear(OpcUa_HistoryUpdateResponse* pValue);

//StatusCode OpcUa_HistoryUpdateResponse_GetSize(OpcUa_HistoryUpdateResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_HistoryUpdateResponse_Encode(OpcUa_HistoryUpdateResponse* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_HistoryUpdateResponse_Decode(OpcUa_HistoryUpdateResponse* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_HistoryUpdateResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_CallMethodRequest
/*============================================================================
 * The CallMethodRequest structure.
 *===========================================================================*/
typedef struct _OpcUa_CallMethodRequest
{
    UA_NodeId   ObjectId;
    UA_NodeId   MethodId;
    int32_t     NoOfInputArguments;
    UA_Variant* InputArguments;
}
OpcUa_CallMethodRequest;

void OpcUa_CallMethodRequest_Initialize(OpcUa_CallMethodRequest* pValue);

void OpcUa_CallMethodRequest_Clear(OpcUa_CallMethodRequest* pValue);

//StatusCode OpcUa_CallMethodRequest_GetSize(OpcUa_CallMethodRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_CallMethodRequest_Encode(OpcUa_CallMethodRequest* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_CallMethodRequest_Decode(OpcUa_CallMethodRequest* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_CallMethodRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_CallMethodResult
/*============================================================================
 * The CallMethodResult structure.
 *===========================================================================*/
typedef struct _OpcUa_CallMethodResult
{
    SOPC_StatusCode         StatusCode;
    int32_t            NoOfInputArgumentResults;
    SOPC_StatusCode*        InputArgumentResults;
    int32_t            NoOfInputArgumentDiagnosticInfos;
    UA_DiagnosticInfo* InputArgumentDiagnosticInfos;
    int32_t            NoOfOutputArguments;
    UA_Variant*        OutputArguments;
}
OpcUa_CallMethodResult;

void OpcUa_CallMethodResult_Initialize(OpcUa_CallMethodResult* pValue);

void OpcUa_CallMethodResult_Clear(OpcUa_CallMethodResult* pValue);

//StatusCode OpcUa_CallMethodResult_GetSize(OpcUa_CallMethodResult* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_CallMethodResult_Encode(OpcUa_CallMethodResult* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_CallMethodResult_Decode(OpcUa_CallMethodResult* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_CallMethodResult_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_Call
#ifndef OPCUA_EXCLUDE_CallRequest
/*============================================================================
 * The CallRequest structure.
 *===========================================================================*/
typedef struct _OpcUa_CallRequest
{
    OpcUa_RequestHeader      RequestHeader;
    int32_t                  NoOfMethodsToCall;
    OpcUa_CallMethodRequest* MethodsToCall;
}
OpcUa_CallRequest;

void OpcUa_CallRequest_Initialize(OpcUa_CallRequest* pValue);

void OpcUa_CallRequest_Clear(OpcUa_CallRequest* pValue);

//StatusCode OpcUa_CallRequest_GetSize(OpcUa_CallRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_CallRequest_Encode(OpcUa_CallRequest* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_CallRequest_Decode(OpcUa_CallRequest* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_CallRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_CallResponse
/*============================================================================
 * The CallResponse structure.
 *===========================================================================*/
typedef struct _OpcUa_CallResponse
{
    OpcUa_ResponseHeader    ResponseHeader;
    int32_t                 NoOfResults;
    OpcUa_CallMethodResult* Results;
    int32_t                 NoOfDiagnosticInfos;
    UA_DiagnosticInfo*      DiagnosticInfos;
}
OpcUa_CallResponse;

void OpcUa_CallResponse_Initialize(OpcUa_CallResponse* pValue);

void OpcUa_CallResponse_Clear(OpcUa_CallResponse* pValue);

//StatusCode OpcUa_CallResponse_GetSize(OpcUa_CallResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_CallResponse_Encode(OpcUa_CallResponse* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_CallResponse_Decode(OpcUa_CallResponse* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_CallResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_MonitoringMode
/*============================================================================
 * The MonitoringMode enumeration.
 *===========================================================================*/
typedef enum _OpcUa_MonitoringMode
{
    OpcUa_MonitoringMode_Disabled  = 0,
    OpcUa_MonitoringMode_Sampling  = 1,
    OpcUa_MonitoringMode_Reporting = 2
#if OPCUA_FORCE_INT32_ENUMS
    ,_OpcUa_MonitoringMode_MaxEnumerationValue = OpcUa_Int32_Max
#endif
}
OpcUa_MonitoringMode;

#endif

#ifndef OPCUA_EXCLUDE_DataChangeTrigger
/*============================================================================
 * The DataChangeTrigger enumeration.
 *===========================================================================*/
typedef enum _OpcUa_DataChangeTrigger
{
    OpcUa_DataChangeTrigger_Status               = 0,
    OpcUa_DataChangeTrigger_StatusValue          = 1,
    OpcUa_DataChangeTrigger_StatusValueTimestamp = 2
#if OPCUA_FORCE_INT32_ENUMS
    ,_OpcUa_DataChangeTrigger_MaxEnumerationValue = OpcUa_Int32_Max
#endif
}
OpcUa_DataChangeTrigger;

#endif

#ifndef OPCUA_EXCLUDE_DeadbandType
/*============================================================================
 * The DeadbandType enumeration.
 *===========================================================================*/
typedef enum _OpcUa_DeadbandType
{
    OpcUa_DeadbandType_None     = 0,
    OpcUa_DeadbandType_Absolute = 1,
    OpcUa_DeadbandType_Percent  = 2
#if OPCUA_FORCE_INT32_ENUMS
    ,_OpcUa_DeadbandType_MaxEnumerationValue = OpcUa_Int32_Max
#endif
}
OpcUa_DeadbandType;

#endif

#ifndef OPCUA_EXCLUDE_DataChangeFilter
/*============================================================================
 * The DataChangeFilter structure.
 *===========================================================================*/
typedef struct _OpcUa_DataChangeFilter
{
    OpcUa_DataChangeTrigger Trigger;
    uint32_t                DeadbandType;
    double                  DeadbandValue;
}
OpcUa_DataChangeFilter;

void OpcUa_DataChangeFilter_Initialize(OpcUa_DataChangeFilter* pValue);

void OpcUa_DataChangeFilter_Clear(OpcUa_DataChangeFilter* pValue);

//StatusCode OpcUa_DataChangeFilter_GetSize(OpcUa_DataChangeFilter* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_DataChangeFilter_Encode(OpcUa_DataChangeFilter* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_DataChangeFilter_Decode(OpcUa_DataChangeFilter* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_DataChangeFilter_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_AggregateFilter
/*============================================================================
 * The AggregateFilter structure.
 *===========================================================================*/
typedef struct _OpcUa_AggregateFilter
{
    UA_DateTime                  StartTime;
    UA_NodeId                    AggregateType;
    double                       ProcessingInterval;
    OpcUa_AggregateConfiguration AggregateConfiguration;
}
OpcUa_AggregateFilter;

void OpcUa_AggregateFilter_Initialize(OpcUa_AggregateFilter* pValue);

void OpcUa_AggregateFilter_Clear(OpcUa_AggregateFilter* pValue);

//StatusCode OpcUa_AggregateFilter_GetSize(OpcUa_AggregateFilter* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_AggregateFilter_Encode(OpcUa_AggregateFilter* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_AggregateFilter_Decode(OpcUa_AggregateFilter* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_AggregateFilter_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_EventFilterResult
/*============================================================================
 * The EventFilterResult structure.
 *===========================================================================*/
typedef struct _OpcUa_EventFilterResult
{
    int32_t                   NoOfSelectClauseResults;
    SOPC_StatusCode*               SelectClauseResults;
    int32_t                   NoOfSelectClauseDiagnosticInfos;
    UA_DiagnosticInfo*        SelectClauseDiagnosticInfos;
    OpcUa_ContentFilterResult WhereClauseResult;
}
OpcUa_EventFilterResult;

void OpcUa_EventFilterResult_Initialize(OpcUa_EventFilterResult* pValue);

void OpcUa_EventFilterResult_Clear(OpcUa_EventFilterResult* pValue);

//StatusCode OpcUa_EventFilterResult_GetSize(OpcUa_EventFilterResult* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_EventFilterResult_Encode(OpcUa_EventFilterResult* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_EventFilterResult_Decode(OpcUa_EventFilterResult* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_EventFilterResult_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_AggregateFilterResult
/*============================================================================
 * The AggregateFilterResult structure.
 *===========================================================================*/
typedef struct _OpcUa_AggregateFilterResult
{
    UA_DateTime                  RevisedStartTime;
    double                       RevisedProcessingInterval;
    OpcUa_AggregateConfiguration RevisedAggregateConfiguration;
}
OpcUa_AggregateFilterResult;

void OpcUa_AggregateFilterResult_Initialize(OpcUa_AggregateFilterResult* pValue);

void OpcUa_AggregateFilterResult_Clear(OpcUa_AggregateFilterResult* pValue);

//StatusCode OpcUa_AggregateFilterResult_GetSize(OpcUa_AggregateFilterResult* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_AggregateFilterResult_Encode(OpcUa_AggregateFilterResult* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_AggregateFilterResult_Decode(OpcUa_AggregateFilterResult* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_AggregateFilterResult_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_MonitoringParameters
/*============================================================================
 * The MonitoringParameters structure.
 *===========================================================================*/
typedef struct _OpcUa_MonitoringParameters
{
    uint32_t           ClientHandle;
    double             SamplingInterval;
    UA_ExtensionObject Filter;
    uint32_t           QueueSize;
    UA_Boolean         DiscardOldest;
}
OpcUa_MonitoringParameters;

void OpcUa_MonitoringParameters_Initialize(OpcUa_MonitoringParameters* pValue);

void OpcUa_MonitoringParameters_Clear(OpcUa_MonitoringParameters* pValue);

//StatusCode OpcUa_MonitoringParameters_GetSize(OpcUa_MonitoringParameters* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_MonitoringParameters_Encode(OpcUa_MonitoringParameters* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_MonitoringParameters_Decode(OpcUa_MonitoringParameters* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_MonitoringParameters_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_MonitoredItemCreateRequest
/*============================================================================
 * The MonitoredItemCreateRequest structure.
 *===========================================================================*/
typedef struct _OpcUa_MonitoredItemCreateRequest
{
    OpcUa_ReadValueId          ItemToMonitor;
    OpcUa_MonitoringMode       MonitoringMode;
    OpcUa_MonitoringParameters RequestedParameters;
}
OpcUa_MonitoredItemCreateRequest;

void OpcUa_MonitoredItemCreateRequest_Initialize(OpcUa_MonitoredItemCreateRequest* pValue);

void OpcUa_MonitoredItemCreateRequest_Clear(OpcUa_MonitoredItemCreateRequest* pValue);

//StatusCode OpcUa_MonitoredItemCreateRequest_GetSize(OpcUa_MonitoredItemCreateRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_MonitoredItemCreateRequest_Encode(OpcUa_MonitoredItemCreateRequest* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_MonitoredItemCreateRequest_Decode(OpcUa_MonitoredItemCreateRequest* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_MonitoredItemCreateRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_MonitoredItemCreateResult
/*============================================================================
 * The MonitoredItemCreateResult structure.
 *===========================================================================*/
typedef struct _OpcUa_MonitoredItemCreateResult
{
    SOPC_StatusCode         StatusCode;
    uint32_t           MonitoredItemId;
    double             RevisedSamplingInterval;
    uint32_t           RevisedQueueSize;
    UA_ExtensionObject FilterResult;
}
OpcUa_MonitoredItemCreateResult;

void OpcUa_MonitoredItemCreateResult_Initialize(OpcUa_MonitoredItemCreateResult* pValue);

void OpcUa_MonitoredItemCreateResult_Clear(OpcUa_MonitoredItemCreateResult* pValue);

//StatusCode OpcUa_MonitoredItemCreateResult_GetSize(OpcUa_MonitoredItemCreateResult* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_MonitoredItemCreateResult_Encode(OpcUa_MonitoredItemCreateResult* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_MonitoredItemCreateResult_Decode(OpcUa_MonitoredItemCreateResult* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_MonitoredItemCreateResult_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_CreateMonitoredItems
#ifndef OPCUA_EXCLUDE_CreateMonitoredItemsRequest
/*============================================================================
 * The CreateMonitoredItemsRequest structure.
 *===========================================================================*/
typedef struct _OpcUa_CreateMonitoredItemsRequest
{
    OpcUa_RequestHeader               RequestHeader;
    uint32_t                          SubscriptionId;
    OpcUa_TimestampsToReturn          TimestampsToReturn;
    int32_t                           NoOfItemsToCreate;
    OpcUa_MonitoredItemCreateRequest* ItemsToCreate;
}
OpcUa_CreateMonitoredItemsRequest;

void OpcUa_CreateMonitoredItemsRequest_Initialize(OpcUa_CreateMonitoredItemsRequest* pValue);

void OpcUa_CreateMonitoredItemsRequest_Clear(OpcUa_CreateMonitoredItemsRequest* pValue);

//StatusCode OpcUa_CreateMonitoredItemsRequest_GetSize(OpcUa_CreateMonitoredItemsRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_CreateMonitoredItemsRequest_Encode(OpcUa_CreateMonitoredItemsRequest* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_CreateMonitoredItemsRequest_Decode(OpcUa_CreateMonitoredItemsRequest* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_CreateMonitoredItemsRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_CreateMonitoredItemsResponse
/*============================================================================
 * The CreateMonitoredItemsResponse structure.
 *===========================================================================*/
typedef struct _OpcUa_CreateMonitoredItemsResponse
{
    OpcUa_ResponseHeader             ResponseHeader;
    int32_t                          NoOfResults;
    OpcUa_MonitoredItemCreateResult* Results;
    int32_t                          NoOfDiagnosticInfos;
    UA_DiagnosticInfo*               DiagnosticInfos;
}
OpcUa_CreateMonitoredItemsResponse;

void OpcUa_CreateMonitoredItemsResponse_Initialize(OpcUa_CreateMonitoredItemsResponse* pValue);

void OpcUa_CreateMonitoredItemsResponse_Clear(OpcUa_CreateMonitoredItemsResponse* pValue);

//StatusCode OpcUa_CreateMonitoredItemsResponse_GetSize(OpcUa_CreateMonitoredItemsResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_CreateMonitoredItemsResponse_Encode(OpcUa_CreateMonitoredItemsResponse* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_CreateMonitoredItemsResponse_Decode(OpcUa_CreateMonitoredItemsResponse* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_CreateMonitoredItemsResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_MonitoredItemModifyRequest
/*============================================================================
 * The MonitoredItemModifyRequest structure.
 *===========================================================================*/
typedef struct _OpcUa_MonitoredItemModifyRequest
{
    uint32_t                   MonitoredItemId;
    OpcUa_MonitoringParameters RequestedParameters;
}
OpcUa_MonitoredItemModifyRequest;

void OpcUa_MonitoredItemModifyRequest_Initialize(OpcUa_MonitoredItemModifyRequest* pValue);

void OpcUa_MonitoredItemModifyRequest_Clear(OpcUa_MonitoredItemModifyRequest* pValue);

//StatusCode OpcUa_MonitoredItemModifyRequest_GetSize(OpcUa_MonitoredItemModifyRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_MonitoredItemModifyRequest_Encode(OpcUa_MonitoredItemModifyRequest* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_MonitoredItemModifyRequest_Decode(OpcUa_MonitoredItemModifyRequest* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_MonitoredItemModifyRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_MonitoredItemModifyResult
/*============================================================================
 * The MonitoredItemModifyResult structure.
 *===========================================================================*/
typedef struct _OpcUa_MonitoredItemModifyResult
{
    SOPC_StatusCode         StatusCode;
    double             RevisedSamplingInterval;
    uint32_t           RevisedQueueSize;
    UA_ExtensionObject FilterResult;
}
OpcUa_MonitoredItemModifyResult;

void OpcUa_MonitoredItemModifyResult_Initialize(OpcUa_MonitoredItemModifyResult* pValue);

void OpcUa_MonitoredItemModifyResult_Clear(OpcUa_MonitoredItemModifyResult* pValue);

//StatusCode OpcUa_MonitoredItemModifyResult_GetSize(OpcUa_MonitoredItemModifyResult* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_MonitoredItemModifyResult_Encode(OpcUa_MonitoredItemModifyResult* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_MonitoredItemModifyResult_Decode(OpcUa_MonitoredItemModifyResult* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_MonitoredItemModifyResult_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ModifyMonitoredItems
#ifndef OPCUA_EXCLUDE_ModifyMonitoredItemsRequest
/*============================================================================
 * The ModifyMonitoredItemsRequest structure.
 *===========================================================================*/
typedef struct _OpcUa_ModifyMonitoredItemsRequest
{
    OpcUa_RequestHeader               RequestHeader;
    uint32_t                          SubscriptionId;
    OpcUa_TimestampsToReturn          TimestampsToReturn;
    int32_t                           NoOfItemsToModify;
    OpcUa_MonitoredItemModifyRequest* ItemsToModify;
}
OpcUa_ModifyMonitoredItemsRequest;

void OpcUa_ModifyMonitoredItemsRequest_Initialize(OpcUa_ModifyMonitoredItemsRequest* pValue);

void OpcUa_ModifyMonitoredItemsRequest_Clear(OpcUa_ModifyMonitoredItemsRequest* pValue);

//StatusCode OpcUa_ModifyMonitoredItemsRequest_GetSize(OpcUa_ModifyMonitoredItemsRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_ModifyMonitoredItemsRequest_Encode(OpcUa_ModifyMonitoredItemsRequest* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ModifyMonitoredItemsRequest_Decode(OpcUa_ModifyMonitoredItemsRequest* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_ModifyMonitoredItemsRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ModifyMonitoredItemsResponse
/*============================================================================
 * The ModifyMonitoredItemsResponse structure.
 *===========================================================================*/
typedef struct _OpcUa_ModifyMonitoredItemsResponse
{
    OpcUa_ResponseHeader             ResponseHeader;
    int32_t                          NoOfResults;
    OpcUa_MonitoredItemModifyResult* Results;
    int32_t                          NoOfDiagnosticInfos;
    UA_DiagnosticInfo*               DiagnosticInfos;
}
OpcUa_ModifyMonitoredItemsResponse;

void OpcUa_ModifyMonitoredItemsResponse_Initialize(OpcUa_ModifyMonitoredItemsResponse* pValue);

void OpcUa_ModifyMonitoredItemsResponse_Clear(OpcUa_ModifyMonitoredItemsResponse* pValue);

//StatusCode OpcUa_ModifyMonitoredItemsResponse_GetSize(OpcUa_ModifyMonitoredItemsResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_ModifyMonitoredItemsResponse_Encode(OpcUa_ModifyMonitoredItemsResponse* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ModifyMonitoredItemsResponse_Decode(OpcUa_ModifyMonitoredItemsResponse* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_ModifyMonitoredItemsResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_SetMonitoringMode
#ifndef OPCUA_EXCLUDE_SetMonitoringModeRequest
/*============================================================================
 * The SetMonitoringModeRequest structure.
 *===========================================================================*/
typedef struct _OpcUa_SetMonitoringModeRequest
{
    OpcUa_RequestHeader  RequestHeader;
    uint32_t             SubscriptionId;
    OpcUa_MonitoringMode MonitoringMode;
    int32_t              NoOfMonitoredItemIds;
    uint32_t*            MonitoredItemIds;
}
OpcUa_SetMonitoringModeRequest;

void OpcUa_SetMonitoringModeRequest_Initialize(OpcUa_SetMonitoringModeRequest* pValue);

void OpcUa_SetMonitoringModeRequest_Clear(OpcUa_SetMonitoringModeRequest* pValue);

//StatusCode OpcUa_SetMonitoringModeRequest_GetSize(OpcUa_SetMonitoringModeRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_SetMonitoringModeRequest_Encode(OpcUa_SetMonitoringModeRequest* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_SetMonitoringModeRequest_Decode(OpcUa_SetMonitoringModeRequest* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_SetMonitoringModeRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_SetMonitoringModeResponse
/*============================================================================
 * The SetMonitoringModeResponse structure.
 *===========================================================================*/
typedef struct _OpcUa_SetMonitoringModeResponse
{
    OpcUa_ResponseHeader ResponseHeader;
    int32_t              NoOfResults;
    SOPC_StatusCode*          Results;
    int32_t              NoOfDiagnosticInfos;
    UA_DiagnosticInfo*   DiagnosticInfos;
}
OpcUa_SetMonitoringModeResponse;

void OpcUa_SetMonitoringModeResponse_Initialize(OpcUa_SetMonitoringModeResponse* pValue);

void OpcUa_SetMonitoringModeResponse_Clear(OpcUa_SetMonitoringModeResponse* pValue);

//StatusCode OpcUa_SetMonitoringModeResponse_GetSize(OpcUa_SetMonitoringModeResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_SetMonitoringModeResponse_Encode(OpcUa_SetMonitoringModeResponse* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_SetMonitoringModeResponse_Decode(OpcUa_SetMonitoringModeResponse* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_SetMonitoringModeResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_SetTriggering
#ifndef OPCUA_EXCLUDE_SetTriggeringRequest
/*============================================================================
 * The SetTriggeringRequest structure.
 *===========================================================================*/
typedef struct _OpcUa_SetTriggeringRequest
{
    OpcUa_RequestHeader RequestHeader;
    uint32_t            SubscriptionId;
    uint32_t            TriggeringItemId;
    int32_t             NoOfLinksToAdd;
    uint32_t*           LinksToAdd;
    int32_t             NoOfLinksToRemove;
    uint32_t*           LinksToRemove;
}
OpcUa_SetTriggeringRequest;

void OpcUa_SetTriggeringRequest_Initialize(OpcUa_SetTriggeringRequest* pValue);

void OpcUa_SetTriggeringRequest_Clear(OpcUa_SetTriggeringRequest* pValue);

//StatusCode OpcUa_SetTriggeringRequest_GetSize(OpcUa_SetTriggeringRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_SetTriggeringRequest_Encode(OpcUa_SetTriggeringRequest* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_SetTriggeringRequest_Decode(OpcUa_SetTriggeringRequest* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_SetTriggeringRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_SetTriggeringResponse
/*============================================================================
 * The SetTriggeringResponse structure.
 *===========================================================================*/
typedef struct _OpcUa_SetTriggeringResponse
{
    OpcUa_ResponseHeader ResponseHeader;
    int32_t              NoOfAddResults;
    SOPC_StatusCode*          AddResults;
    int32_t              NoOfAddDiagnosticInfos;
    UA_DiagnosticInfo*   AddDiagnosticInfos;
    int32_t              NoOfRemoveResults;
    SOPC_StatusCode*          RemoveResults;
    int32_t              NoOfRemoveDiagnosticInfos;
    UA_DiagnosticInfo*   RemoveDiagnosticInfos;
}
OpcUa_SetTriggeringResponse;

void OpcUa_SetTriggeringResponse_Initialize(OpcUa_SetTriggeringResponse* pValue);

void OpcUa_SetTriggeringResponse_Clear(OpcUa_SetTriggeringResponse* pValue);

//StatusCode OpcUa_SetTriggeringResponse_GetSize(OpcUa_SetTriggeringResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_SetTriggeringResponse_Encode(OpcUa_SetTriggeringResponse* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_SetTriggeringResponse_Decode(OpcUa_SetTriggeringResponse* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_SetTriggeringResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_DeleteMonitoredItems
#ifndef OPCUA_EXCLUDE_DeleteMonitoredItemsRequest
/*============================================================================
 * The DeleteMonitoredItemsRequest structure.
 *===========================================================================*/
typedef struct _OpcUa_DeleteMonitoredItemsRequest
{
    OpcUa_RequestHeader RequestHeader;
    uint32_t            SubscriptionId;
    int32_t             NoOfMonitoredItemIds;
    uint32_t*           MonitoredItemIds;
}
OpcUa_DeleteMonitoredItemsRequest;

void OpcUa_DeleteMonitoredItemsRequest_Initialize(OpcUa_DeleteMonitoredItemsRequest* pValue);

void OpcUa_DeleteMonitoredItemsRequest_Clear(OpcUa_DeleteMonitoredItemsRequest* pValue);

//StatusCode OpcUa_DeleteMonitoredItemsRequest_GetSize(OpcUa_DeleteMonitoredItemsRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_DeleteMonitoredItemsRequest_Encode(OpcUa_DeleteMonitoredItemsRequest* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_DeleteMonitoredItemsRequest_Decode(OpcUa_DeleteMonitoredItemsRequest* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_DeleteMonitoredItemsRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_DeleteMonitoredItemsResponse
/*============================================================================
 * The DeleteMonitoredItemsResponse structure.
 *===========================================================================*/
typedef struct _OpcUa_DeleteMonitoredItemsResponse
{
    OpcUa_ResponseHeader ResponseHeader;
    int32_t              NoOfResults;
    SOPC_StatusCode*          Results;
    int32_t              NoOfDiagnosticInfos;
    UA_DiagnosticInfo*   DiagnosticInfos;
}
OpcUa_DeleteMonitoredItemsResponse;

void OpcUa_DeleteMonitoredItemsResponse_Initialize(OpcUa_DeleteMonitoredItemsResponse* pValue);

void OpcUa_DeleteMonitoredItemsResponse_Clear(OpcUa_DeleteMonitoredItemsResponse* pValue);

//StatusCode OpcUa_DeleteMonitoredItemsResponse_GetSize(OpcUa_DeleteMonitoredItemsResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_DeleteMonitoredItemsResponse_Encode(OpcUa_DeleteMonitoredItemsResponse* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_DeleteMonitoredItemsResponse_Decode(OpcUa_DeleteMonitoredItemsResponse* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_DeleteMonitoredItemsResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_CreateSubscription
#ifndef OPCUA_EXCLUDE_CreateSubscriptionRequest
/*============================================================================
 * The CreateSubscriptionRequest structure.
 *===========================================================================*/
typedef struct _OpcUa_CreateSubscriptionRequest
{
    OpcUa_RequestHeader RequestHeader;
    double              RequestedPublishingInterval;
    uint32_t            RequestedLifetimeCount;
    uint32_t            RequestedMaxKeepAliveCount;
    uint32_t            MaxNotificationsPerPublish;
    UA_Boolean          PublishingEnabled;
    UA_Byte             Priority;
}
OpcUa_CreateSubscriptionRequest;

void OpcUa_CreateSubscriptionRequest_Initialize(OpcUa_CreateSubscriptionRequest* pValue);

void OpcUa_CreateSubscriptionRequest_Clear(OpcUa_CreateSubscriptionRequest* pValue);

//StatusCode OpcUa_CreateSubscriptionRequest_GetSize(OpcUa_CreateSubscriptionRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_CreateSubscriptionRequest_Encode(OpcUa_CreateSubscriptionRequest* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_CreateSubscriptionRequest_Decode(OpcUa_CreateSubscriptionRequest* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_CreateSubscriptionRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_CreateSubscriptionResponse
/*============================================================================
 * The CreateSubscriptionResponse structure.
 *===========================================================================*/
typedef struct _OpcUa_CreateSubscriptionResponse
{
    OpcUa_ResponseHeader ResponseHeader;
    uint32_t             SubscriptionId;
    double               RevisedPublishingInterval;
    uint32_t             RevisedLifetimeCount;
    uint32_t             RevisedMaxKeepAliveCount;
}
OpcUa_CreateSubscriptionResponse;

void OpcUa_CreateSubscriptionResponse_Initialize(OpcUa_CreateSubscriptionResponse* pValue);

void OpcUa_CreateSubscriptionResponse_Clear(OpcUa_CreateSubscriptionResponse* pValue);

//StatusCode OpcUa_CreateSubscriptionResponse_GetSize(OpcUa_CreateSubscriptionResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_CreateSubscriptionResponse_Encode(OpcUa_CreateSubscriptionResponse* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_CreateSubscriptionResponse_Decode(OpcUa_CreateSubscriptionResponse* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_CreateSubscriptionResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_ModifySubscription
#ifndef OPCUA_EXCLUDE_ModifySubscriptionRequest
/*============================================================================
 * The ModifySubscriptionRequest structure.
 *===========================================================================*/
typedef struct _OpcUa_ModifySubscriptionRequest
{
    OpcUa_RequestHeader RequestHeader;
    uint32_t            SubscriptionId;
    double              RequestedPublishingInterval;
    uint32_t            RequestedLifetimeCount;
    uint32_t            RequestedMaxKeepAliveCount;
    uint32_t            MaxNotificationsPerPublish;
    UA_Byte             Priority;
}
OpcUa_ModifySubscriptionRequest;

void OpcUa_ModifySubscriptionRequest_Initialize(OpcUa_ModifySubscriptionRequest* pValue);

void OpcUa_ModifySubscriptionRequest_Clear(OpcUa_ModifySubscriptionRequest* pValue);

//StatusCode OpcUa_ModifySubscriptionRequest_GetSize(OpcUa_ModifySubscriptionRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_ModifySubscriptionRequest_Encode(OpcUa_ModifySubscriptionRequest* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ModifySubscriptionRequest_Decode(OpcUa_ModifySubscriptionRequest* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_ModifySubscriptionRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ModifySubscriptionResponse
/*============================================================================
 * The ModifySubscriptionResponse structure.
 *===========================================================================*/
typedef struct _OpcUa_ModifySubscriptionResponse
{
    OpcUa_ResponseHeader ResponseHeader;
    double               RevisedPublishingInterval;
    uint32_t             RevisedLifetimeCount;
    uint32_t             RevisedMaxKeepAliveCount;
}
OpcUa_ModifySubscriptionResponse;

void OpcUa_ModifySubscriptionResponse_Initialize(OpcUa_ModifySubscriptionResponse* pValue);

void OpcUa_ModifySubscriptionResponse_Clear(OpcUa_ModifySubscriptionResponse* pValue);

//StatusCode OpcUa_ModifySubscriptionResponse_GetSize(OpcUa_ModifySubscriptionResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_ModifySubscriptionResponse_Encode(OpcUa_ModifySubscriptionResponse* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ModifySubscriptionResponse_Decode(OpcUa_ModifySubscriptionResponse* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_ModifySubscriptionResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_SetPublishingMode
#ifndef OPCUA_EXCLUDE_SetPublishingModeRequest
/*============================================================================
 * The SetPublishingModeRequest structure.
 *===========================================================================*/
typedef struct _OpcUa_SetPublishingModeRequest
{
    OpcUa_RequestHeader RequestHeader;
    UA_Boolean          PublishingEnabled;
    int32_t             NoOfSubscriptionIds;
    uint32_t*           SubscriptionIds;
}
OpcUa_SetPublishingModeRequest;

void OpcUa_SetPublishingModeRequest_Initialize(OpcUa_SetPublishingModeRequest* pValue);

void OpcUa_SetPublishingModeRequest_Clear(OpcUa_SetPublishingModeRequest* pValue);

//StatusCode OpcUa_SetPublishingModeRequest_GetSize(OpcUa_SetPublishingModeRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_SetPublishingModeRequest_Encode(OpcUa_SetPublishingModeRequest* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_SetPublishingModeRequest_Decode(OpcUa_SetPublishingModeRequest* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_SetPublishingModeRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_SetPublishingModeResponse
/*============================================================================
 * The SetPublishingModeResponse structure.
 *===========================================================================*/
typedef struct _OpcUa_SetPublishingModeResponse
{
    OpcUa_ResponseHeader ResponseHeader;
    int32_t              NoOfResults;
    SOPC_StatusCode*          Results;
    int32_t              NoOfDiagnosticInfos;
    UA_DiagnosticInfo*   DiagnosticInfos;
}
OpcUa_SetPublishingModeResponse;

void OpcUa_SetPublishingModeResponse_Initialize(OpcUa_SetPublishingModeResponse* pValue);

void OpcUa_SetPublishingModeResponse_Clear(OpcUa_SetPublishingModeResponse* pValue);

//StatusCode OpcUa_SetPublishingModeResponse_GetSize(OpcUa_SetPublishingModeResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_SetPublishingModeResponse_Encode(OpcUa_SetPublishingModeResponse* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_SetPublishingModeResponse_Decode(OpcUa_SetPublishingModeResponse* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_SetPublishingModeResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_NotificationMessage
/*============================================================================
 * The NotificationMessage structure.
 *===========================================================================*/
typedef struct _OpcUa_NotificationMessage
{
    uint32_t            SequenceNumber;
    UA_DateTime         PublishTime;
    int32_t             NoOfNotificationData;
    UA_ExtensionObject* NotificationData;
}
OpcUa_NotificationMessage;

void OpcUa_NotificationMessage_Initialize(OpcUa_NotificationMessage* pValue);

void OpcUa_NotificationMessage_Clear(OpcUa_NotificationMessage* pValue);

//StatusCode OpcUa_NotificationMessage_GetSize(OpcUa_NotificationMessage* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_NotificationMessage_Encode(OpcUa_NotificationMessage* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_NotificationMessage_Decode(OpcUa_NotificationMessage* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_NotificationMessage_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_MonitoredItemNotification
/*============================================================================
 * The MonitoredItemNotification structure.
 *===========================================================================*/
typedef struct _OpcUa_MonitoredItemNotification
{
    uint32_t     ClientHandle;
    UA_DataValue Value;
}
OpcUa_MonitoredItemNotification;

void OpcUa_MonitoredItemNotification_Initialize(OpcUa_MonitoredItemNotification* pValue);

void OpcUa_MonitoredItemNotification_Clear(OpcUa_MonitoredItemNotification* pValue);

//StatusCode OpcUa_MonitoredItemNotification_GetSize(OpcUa_MonitoredItemNotification* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_MonitoredItemNotification_Encode(OpcUa_MonitoredItemNotification* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_MonitoredItemNotification_Decode(OpcUa_MonitoredItemNotification* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_MonitoredItemNotification_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_DataChangeNotification
/*============================================================================
 * The DataChangeNotification structure.
 *===========================================================================*/
typedef struct _OpcUa_DataChangeNotification
{
    int32_t                          NoOfMonitoredItems;
    OpcUa_MonitoredItemNotification* MonitoredItems;
    int32_t                          NoOfDiagnosticInfos;
    UA_DiagnosticInfo*               DiagnosticInfos;
}
OpcUa_DataChangeNotification;

void OpcUa_DataChangeNotification_Initialize(OpcUa_DataChangeNotification* pValue);

void OpcUa_DataChangeNotification_Clear(OpcUa_DataChangeNotification* pValue);

//StatusCode OpcUa_DataChangeNotification_GetSize(OpcUa_DataChangeNotification* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_DataChangeNotification_Encode(OpcUa_DataChangeNotification* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_DataChangeNotification_Decode(OpcUa_DataChangeNotification* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_DataChangeNotification_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_EventFieldList
/*============================================================================
 * The EventFieldList structure.
 *===========================================================================*/
typedef struct _OpcUa_EventFieldList
{
    uint32_t    ClientHandle;
    int32_t     NoOfEventFields;
    UA_Variant* EventFields;
}
OpcUa_EventFieldList;

void OpcUa_EventFieldList_Initialize(OpcUa_EventFieldList* pValue);

void OpcUa_EventFieldList_Clear(OpcUa_EventFieldList* pValue);

//StatusCode OpcUa_EventFieldList_GetSize(OpcUa_EventFieldList* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_EventFieldList_Encode(OpcUa_EventFieldList* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_EventFieldList_Decode(OpcUa_EventFieldList* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_EventFieldList_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_EventNotificationList
/*============================================================================
 * The EventNotificationList structure.
 *===========================================================================*/
typedef struct _OpcUa_EventNotificationList
{
    int32_t               NoOfEvents;
    OpcUa_EventFieldList* Events;
}
OpcUa_EventNotificationList;

void OpcUa_EventNotificationList_Initialize(OpcUa_EventNotificationList* pValue);

void OpcUa_EventNotificationList_Clear(OpcUa_EventNotificationList* pValue);

//StatusCode OpcUa_EventNotificationList_GetSize(OpcUa_EventNotificationList* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_EventNotificationList_Encode(OpcUa_EventNotificationList* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_EventNotificationList_Decode(OpcUa_EventNotificationList* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_EventNotificationList_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_StatusChangeNotification
/*============================================================================
 * The StatusChangeNotification structure.
 *===========================================================================*/
typedef struct _OpcUa_StatusChangeNotification
{
    SOPC_StatusCode        Status;
    UA_DiagnosticInfo DiagnosticInfo;
}
OpcUa_StatusChangeNotification;

void OpcUa_StatusChangeNotification_Initialize(OpcUa_StatusChangeNotification* pValue);

void OpcUa_StatusChangeNotification_Clear(OpcUa_StatusChangeNotification* pValue);

//StatusCode OpcUa_StatusChangeNotification_GetSize(OpcUa_StatusChangeNotification* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_StatusChangeNotification_Encode(OpcUa_StatusChangeNotification* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_StatusChangeNotification_Decode(OpcUa_StatusChangeNotification* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_StatusChangeNotification_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_SubscriptionAcknowledgement
/*============================================================================
 * The SubscriptionAcknowledgement structure.
 *===========================================================================*/
typedef struct _OpcUa_SubscriptionAcknowledgement
{
    uint32_t SubscriptionId;
    uint32_t SequenceNumber;
}
OpcUa_SubscriptionAcknowledgement;

void OpcUa_SubscriptionAcknowledgement_Initialize(OpcUa_SubscriptionAcknowledgement* pValue);

void OpcUa_SubscriptionAcknowledgement_Clear(OpcUa_SubscriptionAcknowledgement* pValue);

//StatusCode OpcUa_SubscriptionAcknowledgement_GetSize(OpcUa_SubscriptionAcknowledgement* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_SubscriptionAcknowledgement_Encode(OpcUa_SubscriptionAcknowledgement* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_SubscriptionAcknowledgement_Decode(OpcUa_SubscriptionAcknowledgement* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_SubscriptionAcknowledgement_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_Publish
#ifndef OPCUA_EXCLUDE_PublishRequest
/*============================================================================
 * The PublishRequest structure.
 *===========================================================================*/
typedef struct _OpcUa_PublishRequest
{
    OpcUa_RequestHeader                RequestHeader;
    int32_t                            NoOfSubscriptionAcknowledgements;
    OpcUa_SubscriptionAcknowledgement* SubscriptionAcknowledgements;
}
OpcUa_PublishRequest;

void OpcUa_PublishRequest_Initialize(OpcUa_PublishRequest* pValue);

void OpcUa_PublishRequest_Clear(OpcUa_PublishRequest* pValue);

//StatusCode OpcUa_PublishRequest_GetSize(OpcUa_PublishRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_PublishRequest_Encode(OpcUa_PublishRequest* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_PublishRequest_Decode(OpcUa_PublishRequest* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_PublishRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_PublishResponse
/*============================================================================
 * The PublishResponse structure.
 *===========================================================================*/
typedef struct _OpcUa_PublishResponse
{
    OpcUa_ResponseHeader      ResponseHeader;
    uint32_t                  SubscriptionId;
    int32_t                   NoOfAvailableSequenceNumbers;
    uint32_t*                 AvailableSequenceNumbers;
    UA_Boolean                MoreNotifications;
    OpcUa_NotificationMessage NotificationMessage;
    int32_t                   NoOfResults;
    SOPC_StatusCode*               Results;
    int32_t                   NoOfDiagnosticInfos;
    UA_DiagnosticInfo*        DiagnosticInfos;
}
OpcUa_PublishResponse;

void OpcUa_PublishResponse_Initialize(OpcUa_PublishResponse* pValue);

void OpcUa_PublishResponse_Clear(OpcUa_PublishResponse* pValue);

//StatusCode OpcUa_PublishResponse_GetSize(OpcUa_PublishResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_PublishResponse_Encode(OpcUa_PublishResponse* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_PublishResponse_Decode(OpcUa_PublishResponse* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_PublishResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_Republish
#ifndef OPCUA_EXCLUDE_RepublishRequest
/*============================================================================
 * The RepublishRequest structure.
 *===========================================================================*/
typedef struct _OpcUa_RepublishRequest
{
    OpcUa_RequestHeader RequestHeader;
    uint32_t            SubscriptionId;
    uint32_t            RetransmitSequenceNumber;
}
OpcUa_RepublishRequest;

void OpcUa_RepublishRequest_Initialize(OpcUa_RepublishRequest* pValue);

void OpcUa_RepublishRequest_Clear(OpcUa_RepublishRequest* pValue);

//StatusCode OpcUa_RepublishRequest_GetSize(OpcUa_RepublishRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_RepublishRequest_Encode(OpcUa_RepublishRequest* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_RepublishRequest_Decode(OpcUa_RepublishRequest* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_RepublishRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_RepublishResponse
/*============================================================================
 * The RepublishResponse structure.
 *===========================================================================*/
typedef struct _OpcUa_RepublishResponse
{
    OpcUa_ResponseHeader      ResponseHeader;
    OpcUa_NotificationMessage NotificationMessage;
}
OpcUa_RepublishResponse;

void OpcUa_RepublishResponse_Initialize(OpcUa_RepublishResponse* pValue);

void OpcUa_RepublishResponse_Clear(OpcUa_RepublishResponse* pValue);

//StatusCode OpcUa_RepublishResponse_GetSize(OpcUa_RepublishResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_RepublishResponse_Encode(OpcUa_RepublishResponse* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_RepublishResponse_Decode(OpcUa_RepublishResponse* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_RepublishResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_TransferResult
/*============================================================================
 * The TransferResult structure.
 *===========================================================================*/
typedef struct _OpcUa_TransferResult
{
    SOPC_StatusCode StatusCode;
    int32_t    NoOfAvailableSequenceNumbers;
    uint32_t*  AvailableSequenceNumbers;
}
OpcUa_TransferResult;

void OpcUa_TransferResult_Initialize(OpcUa_TransferResult* pValue);

void OpcUa_TransferResult_Clear(OpcUa_TransferResult* pValue);

//StatusCode OpcUa_TransferResult_GetSize(OpcUa_TransferResult* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_TransferResult_Encode(OpcUa_TransferResult* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_TransferResult_Decode(OpcUa_TransferResult* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_TransferResult_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_TransferSubscriptions
#ifndef OPCUA_EXCLUDE_TransferSubscriptionsRequest
/*============================================================================
 * The TransferSubscriptionsRequest structure.
 *===========================================================================*/
typedef struct _OpcUa_TransferSubscriptionsRequest
{
    OpcUa_RequestHeader RequestHeader;
    int32_t             NoOfSubscriptionIds;
    uint32_t*           SubscriptionIds;
    UA_Boolean          SendInitialValues;
}
OpcUa_TransferSubscriptionsRequest;

void OpcUa_TransferSubscriptionsRequest_Initialize(OpcUa_TransferSubscriptionsRequest* pValue);

void OpcUa_TransferSubscriptionsRequest_Clear(OpcUa_TransferSubscriptionsRequest* pValue);

//StatusCode OpcUa_TransferSubscriptionsRequest_GetSize(OpcUa_TransferSubscriptionsRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_TransferSubscriptionsRequest_Encode(OpcUa_TransferSubscriptionsRequest* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_TransferSubscriptionsRequest_Decode(OpcUa_TransferSubscriptionsRequest* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_TransferSubscriptionsRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_TransferSubscriptionsResponse
/*============================================================================
 * The TransferSubscriptionsResponse structure.
 *===========================================================================*/
typedef struct _OpcUa_TransferSubscriptionsResponse
{
    OpcUa_ResponseHeader  ResponseHeader;
    int32_t               NoOfResults;
    OpcUa_TransferResult* Results;
    int32_t               NoOfDiagnosticInfos;
    UA_DiagnosticInfo*    DiagnosticInfos;
}
OpcUa_TransferSubscriptionsResponse;

void OpcUa_TransferSubscriptionsResponse_Initialize(OpcUa_TransferSubscriptionsResponse* pValue);

void OpcUa_TransferSubscriptionsResponse_Clear(OpcUa_TransferSubscriptionsResponse* pValue);

//StatusCode OpcUa_TransferSubscriptionsResponse_GetSize(OpcUa_TransferSubscriptionsResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_TransferSubscriptionsResponse_Encode(OpcUa_TransferSubscriptionsResponse* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_TransferSubscriptionsResponse_Decode(OpcUa_TransferSubscriptionsResponse* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_TransferSubscriptionsResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_DeleteSubscriptions
#ifndef OPCUA_EXCLUDE_DeleteSubscriptionsRequest
/*============================================================================
 * The DeleteSubscriptionsRequest structure.
 *===========================================================================*/
typedef struct _OpcUa_DeleteSubscriptionsRequest
{
    OpcUa_RequestHeader RequestHeader;
    int32_t             NoOfSubscriptionIds;
    uint32_t*           SubscriptionIds;
}
OpcUa_DeleteSubscriptionsRequest;

void OpcUa_DeleteSubscriptionsRequest_Initialize(OpcUa_DeleteSubscriptionsRequest* pValue);

void OpcUa_DeleteSubscriptionsRequest_Clear(OpcUa_DeleteSubscriptionsRequest* pValue);

//StatusCode OpcUa_DeleteSubscriptionsRequest_GetSize(OpcUa_DeleteSubscriptionsRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_DeleteSubscriptionsRequest_Encode(OpcUa_DeleteSubscriptionsRequest* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_DeleteSubscriptionsRequest_Decode(OpcUa_DeleteSubscriptionsRequest* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_DeleteSubscriptionsRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_DeleteSubscriptionsResponse
/*============================================================================
 * The DeleteSubscriptionsResponse structure.
 *===========================================================================*/
typedef struct _OpcUa_DeleteSubscriptionsResponse
{
    OpcUa_ResponseHeader ResponseHeader;
    int32_t              NoOfResults;
    SOPC_StatusCode*          Results;
    int32_t              NoOfDiagnosticInfos;
    UA_DiagnosticInfo*   DiagnosticInfos;
}
OpcUa_DeleteSubscriptionsResponse;

void OpcUa_DeleteSubscriptionsResponse_Initialize(OpcUa_DeleteSubscriptionsResponse* pValue);

void OpcUa_DeleteSubscriptionsResponse_Clear(OpcUa_DeleteSubscriptionsResponse* pValue);

//StatusCode OpcUa_DeleteSubscriptionsResponse_GetSize(OpcUa_DeleteSubscriptionsResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_DeleteSubscriptionsResponse_Encode(OpcUa_DeleteSubscriptionsResponse* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_DeleteSubscriptionsResponse_Decode(OpcUa_DeleteSubscriptionsResponse* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_DeleteSubscriptionsResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_EnumeratedTestType
/*============================================================================
 * The EnumeratedTestType enumeration.
 *===========================================================================*/
typedef enum _OpcUa_EnumeratedTestType
{
    OpcUa_EnumeratedTestType_Red    = 1,
    OpcUa_EnumeratedTestType_Yellow = 4,
    OpcUa_EnumeratedTestType_Green  = 5
#if OPCUA_FORCE_INT32_ENUMS
    ,_OpcUa_EnumeratedTestType_MaxEnumerationValue = OpcUa_Int32_Max
#endif
}
OpcUa_EnumeratedTestType;

#endif

#ifndef OPCUA_EXCLUDE_BuildInfo
/*============================================================================
 * The BuildInfo structure.
 *===========================================================================*/
typedef struct _OpcUa_BuildInfo
{
    UA_String   ProductUri;
    UA_String   ManufacturerName;
    UA_String   ProductName;
    UA_String   SoftwareVersion;
    UA_String   BuildNumber;
    UA_DateTime BuildDate;
}
OpcUa_BuildInfo;

void OpcUa_BuildInfo_Initialize(OpcUa_BuildInfo* pValue);

void OpcUa_BuildInfo_Clear(OpcUa_BuildInfo* pValue);

//StatusCode OpcUa_BuildInfo_GetSize(OpcUa_BuildInfo* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_BuildInfo_Encode(OpcUa_BuildInfo* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_BuildInfo_Decode(OpcUa_BuildInfo* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_BuildInfo_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_RedundancySupport
/*============================================================================
 * The RedundancySupport enumeration.
 *===========================================================================*/
typedef enum _OpcUa_RedundancySupport
{
    OpcUa_RedundancySupport_None           = 0,
    OpcUa_RedundancySupport_Cold           = 1,
    OpcUa_RedundancySupport_Warm           = 2,
    OpcUa_RedundancySupport_Hot            = 3,
    OpcUa_RedundancySupport_Transparent    = 4,
    OpcUa_RedundancySupport_HotAndMirrored = 5
#if OPCUA_FORCE_INT32_ENUMS
    ,_OpcUa_RedundancySupport_MaxEnumerationValue = OpcUa_Int32_Max
#endif
}
OpcUa_RedundancySupport;

#endif

#ifndef OPCUA_EXCLUDE_ServerState
/*============================================================================
 * The ServerState enumeration.
 *===========================================================================*/
typedef enum _OpcUa_ServerState
{
    OpcUa_ServerState_Running            = 0,
    OpcUa_ServerState_Failed             = 1,
    OpcUa_ServerState_NoConfiguration    = 2,
    OpcUa_ServerState_Suspended          = 3,
    OpcUa_ServerState_Shutdown           = 4,
    OpcUa_ServerState_Test               = 5,
    OpcUa_ServerState_CommunicationFault = 6,
    OpcUa_ServerState_Unknown            = 7
#if OPCUA_FORCE_INT32_ENUMS
    ,_OpcUa_ServerState_MaxEnumerationValue = OpcUa_Int32_Max
#endif
}
OpcUa_ServerState;

#endif

#ifndef OPCUA_EXCLUDE_RedundantServerDataType
/*============================================================================
 * The RedundantServerDataType structure.
 *===========================================================================*/
typedef struct _OpcUa_RedundantServerDataType
{
    UA_String         ServerId;
    UA_Byte           ServiceLevel;
    OpcUa_ServerState ServerState;
}
OpcUa_RedundantServerDataType;

void OpcUa_RedundantServerDataType_Initialize(OpcUa_RedundantServerDataType* pValue);

void OpcUa_RedundantServerDataType_Clear(OpcUa_RedundantServerDataType* pValue);

//StatusCode OpcUa_RedundantServerDataType_GetSize(OpcUa_RedundantServerDataType* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_RedundantServerDataType_Encode(OpcUa_RedundantServerDataType* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_RedundantServerDataType_Decode(OpcUa_RedundantServerDataType* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_RedundantServerDataType_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_EndpointUrlListDataType
/*============================================================================
 * The EndpointUrlListDataType structure.
 *===========================================================================*/
typedef struct _OpcUa_EndpointUrlListDataType
{
    int32_t    NoOfEndpointUrlList;
    UA_String* EndpointUrlList;
}
OpcUa_EndpointUrlListDataType;

void OpcUa_EndpointUrlListDataType_Initialize(OpcUa_EndpointUrlListDataType* pValue);

void OpcUa_EndpointUrlListDataType_Clear(OpcUa_EndpointUrlListDataType* pValue);

//StatusCode OpcUa_EndpointUrlListDataType_GetSize(OpcUa_EndpointUrlListDataType* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_EndpointUrlListDataType_Encode(OpcUa_EndpointUrlListDataType* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_EndpointUrlListDataType_Decode(OpcUa_EndpointUrlListDataType* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_EndpointUrlListDataType_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_NetworkGroupDataType
/*============================================================================
 * The NetworkGroupDataType structure.
 *===========================================================================*/
typedef struct _OpcUa_NetworkGroupDataType
{
    UA_String                      ServerUri;
    int32_t                        NoOfNetworkPaths;
    OpcUa_EndpointUrlListDataType* NetworkPaths;
}
OpcUa_NetworkGroupDataType;

void OpcUa_NetworkGroupDataType_Initialize(OpcUa_NetworkGroupDataType* pValue);

void OpcUa_NetworkGroupDataType_Clear(OpcUa_NetworkGroupDataType* pValue);

//StatusCode OpcUa_NetworkGroupDataType_GetSize(OpcUa_NetworkGroupDataType* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_NetworkGroupDataType_Encode(OpcUa_NetworkGroupDataType* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_NetworkGroupDataType_Decode(OpcUa_NetworkGroupDataType* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_NetworkGroupDataType_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_SamplingIntervalDiagnosticsDataType
/*============================================================================
 * The SamplingIntervalDiagnosticsDataType structure.
 *===========================================================================*/
typedef struct _OpcUa_SamplingIntervalDiagnosticsDataType
{
    double   SamplingInterval;
    uint32_t MonitoredItemCount;
    uint32_t MaxMonitoredItemCount;
    uint32_t DisabledMonitoredItemCount;
}
OpcUa_SamplingIntervalDiagnosticsDataType;

void OpcUa_SamplingIntervalDiagnosticsDataType_Initialize(OpcUa_SamplingIntervalDiagnosticsDataType* pValue);

void OpcUa_SamplingIntervalDiagnosticsDataType_Clear(OpcUa_SamplingIntervalDiagnosticsDataType* pValue);

//StatusCode OpcUa_SamplingIntervalDiagnosticsDataType_GetSize(OpcUa_SamplingIntervalDiagnosticsDataType* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_SamplingIntervalDiagnosticsDataType_Encode(OpcUa_SamplingIntervalDiagnosticsDataType* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_SamplingIntervalDiagnosticsDataType_Decode(OpcUa_SamplingIntervalDiagnosticsDataType* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_SamplingIntervalDiagnosticsDataType_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ServerDiagnosticsSummaryDataType
/*============================================================================
 * The ServerDiagnosticsSummaryDataType structure.
 *===========================================================================*/
typedef struct _OpcUa_ServerDiagnosticsSummaryDataType
{
    uint32_t ServerViewCount;
    uint32_t CurrentSessionCount;
    uint32_t CumulatedSessionCount;
    uint32_t SecurityRejectedSessionCount;
    uint32_t RejectedSessionCount;
    uint32_t SessionTimeoutCount;
    uint32_t SessionAbortCount;
    uint32_t CurrentSubscriptionCount;
    uint32_t CumulatedSubscriptionCount;
    uint32_t PublishingIntervalCount;
    uint32_t SecurityRejectedRequestsCount;
    uint32_t RejectedRequestsCount;
}
OpcUa_ServerDiagnosticsSummaryDataType;

void OpcUa_ServerDiagnosticsSummaryDataType_Initialize(OpcUa_ServerDiagnosticsSummaryDataType* pValue);

void OpcUa_ServerDiagnosticsSummaryDataType_Clear(OpcUa_ServerDiagnosticsSummaryDataType* pValue);

//StatusCode OpcUa_ServerDiagnosticsSummaryDataType_GetSize(OpcUa_ServerDiagnosticsSummaryDataType* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_ServerDiagnosticsSummaryDataType_Encode(OpcUa_ServerDiagnosticsSummaryDataType* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ServerDiagnosticsSummaryDataType_Decode(OpcUa_ServerDiagnosticsSummaryDataType* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_ServerDiagnosticsSummaryDataType_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ServerStatusDataType
/*============================================================================
 * The ServerStatusDataType structure.
 *===========================================================================*/
typedef struct _OpcUa_ServerStatusDataType
{
    UA_DateTime       StartTime;
    UA_DateTime       CurrentTime;
    OpcUa_ServerState State;
    OpcUa_BuildInfo   BuildInfo;
    uint32_t          SecondsTillShutdown;
    UA_LocalizedText  ShutdownReason;
}
OpcUa_ServerStatusDataType;

void OpcUa_ServerStatusDataType_Initialize(OpcUa_ServerStatusDataType* pValue);

void OpcUa_ServerStatusDataType_Clear(OpcUa_ServerStatusDataType* pValue);

//StatusCode OpcUa_ServerStatusDataType_GetSize(OpcUa_ServerStatusDataType* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_ServerStatusDataType_Encode(OpcUa_ServerStatusDataType* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ServerStatusDataType_Decode(OpcUa_ServerStatusDataType* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_ServerStatusDataType_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ServiceCounterDataType
/*============================================================================
 * The ServiceCounterDataType structure.
 *===========================================================================*/
typedef struct _OpcUa_ServiceCounterDataType
{
    uint32_t TotalCount;
    uint32_t ErrorCount;
}
OpcUa_ServiceCounterDataType;

void OpcUa_ServiceCounterDataType_Initialize(OpcUa_ServiceCounterDataType* pValue);

void OpcUa_ServiceCounterDataType_Clear(OpcUa_ServiceCounterDataType* pValue);

//StatusCode OpcUa_ServiceCounterDataType_GetSize(OpcUa_ServiceCounterDataType* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_ServiceCounterDataType_Encode(OpcUa_ServiceCounterDataType* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ServiceCounterDataType_Decode(OpcUa_ServiceCounterDataType* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_ServiceCounterDataType_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_SessionDiagnosticsDataType
/*============================================================================
 * The SessionDiagnosticsDataType structure.
 *===========================================================================*/
typedef struct _OpcUa_SessionDiagnosticsDataType
{
    UA_NodeId                    SessionId;
    UA_String                    SessionName;
    OpcUa_ApplicationDescription ClientDescription;
    UA_String                    ServerUri;
    UA_String                    EndpointUrl;
    int32_t                      NoOfLocaleIds;
    UA_String*                   LocaleIds;
    double                       ActualSessionTimeout;
    uint32_t                     MaxResponseMessageSize;
    UA_DateTime                  ClientConnectionTime;
    UA_DateTime                  ClientLastContactTime;
    uint32_t                     CurrentSubscriptionsCount;
    uint32_t                     CurrentMonitoredItemsCount;
    uint32_t                     CurrentPublishRequestsInQueue;
    OpcUa_ServiceCounterDataType TotalRequestCount;
    uint32_t                     UnauthorizedRequestCount;
    OpcUa_ServiceCounterDataType ReadCount;
    OpcUa_ServiceCounterDataType HistoryReadCount;
    OpcUa_ServiceCounterDataType WriteCount;
    OpcUa_ServiceCounterDataType HistoryUpdateCount;
    OpcUa_ServiceCounterDataType CallCount;
    OpcUa_ServiceCounterDataType CreateMonitoredItemsCount;
    OpcUa_ServiceCounterDataType ModifyMonitoredItemsCount;
    OpcUa_ServiceCounterDataType SetMonitoringModeCount;
    OpcUa_ServiceCounterDataType SetTriggeringCount;
    OpcUa_ServiceCounterDataType DeleteMonitoredItemsCount;
    OpcUa_ServiceCounterDataType CreateSubscriptionCount;
    OpcUa_ServiceCounterDataType ModifySubscriptionCount;
    OpcUa_ServiceCounterDataType SetPublishingModeCount;
    OpcUa_ServiceCounterDataType PublishCount;
    OpcUa_ServiceCounterDataType RepublishCount;
    OpcUa_ServiceCounterDataType TransferSubscriptionsCount;
    OpcUa_ServiceCounterDataType DeleteSubscriptionsCount;
    OpcUa_ServiceCounterDataType AddNodesCount;
    OpcUa_ServiceCounterDataType AddReferencesCount;
    OpcUa_ServiceCounterDataType DeleteNodesCount;
    OpcUa_ServiceCounterDataType DeleteReferencesCount;
    OpcUa_ServiceCounterDataType BrowseCount;
    OpcUa_ServiceCounterDataType BrowseNextCount;
    OpcUa_ServiceCounterDataType TranslateBrowsePathsToNodeIdsCount;
    OpcUa_ServiceCounterDataType QueryFirstCount;
    OpcUa_ServiceCounterDataType QueryNextCount;
    OpcUa_ServiceCounterDataType RegisterNodesCount;
    OpcUa_ServiceCounterDataType UnregisterNodesCount;
}
OpcUa_SessionDiagnosticsDataType;

void OpcUa_SessionDiagnosticsDataType_Initialize(OpcUa_SessionDiagnosticsDataType* pValue);

void OpcUa_SessionDiagnosticsDataType_Clear(OpcUa_SessionDiagnosticsDataType* pValue);

//StatusCode OpcUa_SessionDiagnosticsDataType_GetSize(OpcUa_SessionDiagnosticsDataType* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_SessionDiagnosticsDataType_Encode(OpcUa_SessionDiagnosticsDataType* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_SessionDiagnosticsDataType_Decode(OpcUa_SessionDiagnosticsDataType* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_SessionDiagnosticsDataType_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_SessionSecurityDiagnosticsDataType
/*============================================================================
 * The SessionSecurityDiagnosticsDataType structure.
 *===========================================================================*/
typedef struct _OpcUa_SessionSecurityDiagnosticsDataType
{
    UA_NodeId                 SessionId;
    UA_String                 ClientUserIdOfSession;
    int32_t                   NoOfClientUserIdHistory;
    UA_String*                ClientUserIdHistory;
    UA_String                 AuthenticationMechanism;
    UA_String                 Encoding;
    UA_String                 TransportProtocol;
    OpcUa_MessageSecurityMode SecurityMode;
    UA_String                 SecurityPolicyUri;
    UA_ByteString             ClientCertificate;
}
OpcUa_SessionSecurityDiagnosticsDataType;

void OpcUa_SessionSecurityDiagnosticsDataType_Initialize(OpcUa_SessionSecurityDiagnosticsDataType* pValue);

void OpcUa_SessionSecurityDiagnosticsDataType_Clear(OpcUa_SessionSecurityDiagnosticsDataType* pValue);

//StatusCode OpcUa_SessionSecurityDiagnosticsDataType_GetSize(OpcUa_SessionSecurityDiagnosticsDataType* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_SessionSecurityDiagnosticsDataType_Encode(OpcUa_SessionSecurityDiagnosticsDataType* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_SessionSecurityDiagnosticsDataType_Decode(OpcUa_SessionSecurityDiagnosticsDataType* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_SessionSecurityDiagnosticsDataType_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_StatusResult
/*============================================================================
 * The StatusResult structure.
 *===========================================================================*/
typedef struct _OpcUa_StatusResult
{
    SOPC_StatusCode        StatusCode;
    UA_DiagnosticInfo DiagnosticInfo;
}
OpcUa_StatusResult;

void OpcUa_StatusResult_Initialize(OpcUa_StatusResult* pValue);

void OpcUa_StatusResult_Clear(OpcUa_StatusResult* pValue);

//StatusCode OpcUa_StatusResult_GetSize(OpcUa_StatusResult* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_StatusResult_Encode(OpcUa_StatusResult* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_StatusResult_Decode(OpcUa_StatusResult* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_StatusResult_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_SubscriptionDiagnosticsDataType
/*============================================================================
 * The SubscriptionDiagnosticsDataType structure.
 *===========================================================================*/
typedef struct _OpcUa_SubscriptionDiagnosticsDataType
{
    UA_NodeId  SessionId;
    uint32_t   SubscriptionId;
    UA_Byte    Priority;
    double     PublishingInterval;
    uint32_t   MaxKeepAliveCount;
    uint32_t   MaxLifetimeCount;
    uint32_t   MaxNotificationsPerPublish;
    UA_Boolean PublishingEnabled;
    uint32_t   ModifyCount;
    uint32_t   EnableCount;
    uint32_t   DisableCount;
    uint32_t   RepublishRequestCount;
    uint32_t   RepublishMessageRequestCount;
    uint32_t   RepublishMessageCount;
    uint32_t   TransferRequestCount;
    uint32_t   TransferredToAltClientCount;
    uint32_t   TransferredToSameClientCount;
    uint32_t   PublishRequestCount;
    uint32_t   DataChangeNotificationsCount;
    uint32_t   EventNotificationsCount;
    uint32_t   NotificationsCount;
    uint32_t   LatePublishRequestCount;
    uint32_t   CurrentKeepAliveCount;
    uint32_t   CurrentLifetimeCount;
    uint32_t   UnacknowledgedMessageCount;
    uint32_t   DiscardedMessageCount;
    uint32_t   MonitoredItemCount;
    uint32_t   DisabledMonitoredItemCount;
    uint32_t   MonitoringQueueOverflowCount;
    uint32_t   NextSequenceNumber;
    uint32_t   EventQueueOverFlowCount;
}
OpcUa_SubscriptionDiagnosticsDataType;

void OpcUa_SubscriptionDiagnosticsDataType_Initialize(OpcUa_SubscriptionDiagnosticsDataType* pValue);

void OpcUa_SubscriptionDiagnosticsDataType_Clear(OpcUa_SubscriptionDiagnosticsDataType* pValue);

//StatusCode OpcUa_SubscriptionDiagnosticsDataType_GetSize(OpcUa_SubscriptionDiagnosticsDataType* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_SubscriptionDiagnosticsDataType_Encode(OpcUa_SubscriptionDiagnosticsDataType* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_SubscriptionDiagnosticsDataType_Decode(OpcUa_SubscriptionDiagnosticsDataType* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_SubscriptionDiagnosticsDataType_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ModelChangeStructureVerbMask
/*============================================================================
 * The ModelChangeStructureVerbMask enumeration.
 *===========================================================================*/
typedef enum _OpcUa_ModelChangeStructureVerbMask
{
    OpcUa_ModelChangeStructureVerbMask_NodeAdded        = 1,
    OpcUa_ModelChangeStructureVerbMask_NodeDeleted      = 2,
    OpcUa_ModelChangeStructureVerbMask_ReferenceAdded   = 4,
    OpcUa_ModelChangeStructureVerbMask_ReferenceDeleted = 8,
    OpcUa_ModelChangeStructureVerbMask_DataTypeChanged  = 16
#if OPCUA_FORCE_INT32_ENUMS
    ,_OpcUa_ModelChangeStructureVerbMask_MaxEnumerationValue = OpcUa_Int32_Max
#endif
}
OpcUa_ModelChangeStructureVerbMask;

#endif

#ifndef OPCUA_EXCLUDE_ModelChangeStructureDataType
/*============================================================================
 * The ModelChangeStructureDataType structure.
 *===========================================================================*/
typedef struct _OpcUa_ModelChangeStructureDataType
{
    UA_NodeId Affected;
    UA_NodeId AffectedType;
    UA_Byte   Verb;
}
OpcUa_ModelChangeStructureDataType;

void OpcUa_ModelChangeStructureDataType_Initialize(OpcUa_ModelChangeStructureDataType* pValue);

void OpcUa_ModelChangeStructureDataType_Clear(OpcUa_ModelChangeStructureDataType* pValue);

//StatusCode OpcUa_ModelChangeStructureDataType_GetSize(OpcUa_ModelChangeStructureDataType* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_ModelChangeStructureDataType_Encode(OpcUa_ModelChangeStructureDataType* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ModelChangeStructureDataType_Decode(OpcUa_ModelChangeStructureDataType* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_ModelChangeStructureDataType_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_SemanticChangeStructureDataType
/*============================================================================
 * The SemanticChangeStructureDataType structure.
 *===========================================================================*/
typedef struct _OpcUa_SemanticChangeStructureDataType
{
    UA_NodeId Affected;
    UA_NodeId AffectedType;
}
OpcUa_SemanticChangeStructureDataType;

void OpcUa_SemanticChangeStructureDataType_Initialize(OpcUa_SemanticChangeStructureDataType* pValue);

void OpcUa_SemanticChangeStructureDataType_Clear(OpcUa_SemanticChangeStructureDataType* pValue);

//StatusCode OpcUa_SemanticChangeStructureDataType_GetSize(OpcUa_SemanticChangeStructureDataType* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_SemanticChangeStructureDataType_Encode(OpcUa_SemanticChangeStructureDataType* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_SemanticChangeStructureDataType_Decode(OpcUa_SemanticChangeStructureDataType* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_SemanticChangeStructureDataType_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_Range
/*============================================================================
 * The Range structure.
 *===========================================================================*/
typedef struct _OpcUa_Range
{
    double Low;
    double High;
}
OpcUa_Range;

void OpcUa_Range_Initialize(OpcUa_Range* pValue);

void OpcUa_Range_Clear(OpcUa_Range* pValue);

//StatusCode OpcUa_Range_GetSize(OpcUa_Range* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_Range_Encode(OpcUa_Range* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_Range_Decode(OpcUa_Range* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_Range_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_EUInformation
/*============================================================================
 * The EUInformation structure.
 *===========================================================================*/
typedef struct _OpcUa_EUInformation
{
    UA_String        NamespaceUri;
    int32_t          UnitId;
    UA_LocalizedText DisplayName;
    UA_LocalizedText Description;
}
OpcUa_EUInformation;

void OpcUa_EUInformation_Initialize(OpcUa_EUInformation* pValue);

void OpcUa_EUInformation_Clear(OpcUa_EUInformation* pValue);

//StatusCode OpcUa_EUInformation_GetSize(OpcUa_EUInformation* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_EUInformation_Encode(OpcUa_EUInformation* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_EUInformation_Decode(OpcUa_EUInformation* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_EUInformation_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_AxisScaleEnumeration
/*============================================================================
 * The AxisScaleEnumeration enumeration.
 *===========================================================================*/
typedef enum _OpcUa_AxisScaleEnumeration
{
    OpcUa_AxisScaleEnumeration_Linear = 0,
    OpcUa_AxisScaleEnumeration_Log    = 1,
    OpcUa_AxisScaleEnumeration_Ln     = 2
#if OPCUA_FORCE_INT32_ENUMS
    ,_OpcUa_AxisScaleEnumeration_MaxEnumerationValue = OpcUa_Int32_Max
#endif
}
OpcUa_AxisScaleEnumeration;

#endif

#ifndef OPCUA_EXCLUDE_ComplexNumberType
/*============================================================================
 * The ComplexNumberType structure.
 *===========================================================================*/
typedef struct _OpcUa_ComplexNumberType
{
    float Real;
    float Imaginary;
}
OpcUa_ComplexNumberType;

void OpcUa_ComplexNumberType_Initialize(OpcUa_ComplexNumberType* pValue);

void OpcUa_ComplexNumberType_Clear(OpcUa_ComplexNumberType* pValue);

//StatusCode OpcUa_ComplexNumberType_GetSize(OpcUa_ComplexNumberType* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_ComplexNumberType_Encode(OpcUa_ComplexNumberType* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ComplexNumberType_Decode(OpcUa_ComplexNumberType* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_ComplexNumberType_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_DoubleComplexNumberType
/*============================================================================
 * The DoubleComplexNumberType structure.
 *===========================================================================*/
typedef struct _OpcUa_DoubleComplexNumberType
{
    double Real;
    double Imaginary;
}
OpcUa_DoubleComplexNumberType;

void OpcUa_DoubleComplexNumberType_Initialize(OpcUa_DoubleComplexNumberType* pValue);

void OpcUa_DoubleComplexNumberType_Clear(OpcUa_DoubleComplexNumberType* pValue);

//StatusCode OpcUa_DoubleComplexNumberType_GetSize(OpcUa_DoubleComplexNumberType* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_DoubleComplexNumberType_Encode(OpcUa_DoubleComplexNumberType* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_DoubleComplexNumberType_Decode(OpcUa_DoubleComplexNumberType* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_DoubleComplexNumberType_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_AxisInformation
/*============================================================================
 * The AxisInformation structure.
 *===========================================================================*/
typedef struct _OpcUa_AxisInformation
{
    OpcUa_EUInformation        EngineeringUnits;
    OpcUa_Range                EURange;
    UA_LocalizedText           Title;
    OpcUa_AxisScaleEnumeration AxisScaleType;
    int32_t                    NoOfAxisSteps;
    double*                    AxisSteps;
}
OpcUa_AxisInformation;

void OpcUa_AxisInformation_Initialize(OpcUa_AxisInformation* pValue);

void OpcUa_AxisInformation_Clear(OpcUa_AxisInformation* pValue);

//StatusCode OpcUa_AxisInformation_GetSize(OpcUa_AxisInformation* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_AxisInformation_Encode(OpcUa_AxisInformation* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_AxisInformation_Decode(OpcUa_AxisInformation* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_AxisInformation_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_XVType
/*============================================================================
 * The XVType structure.
 *===========================================================================*/
typedef struct _OpcUa_XVType
{
    double X;
    float  Value;
}
OpcUa_XVType;

void OpcUa_XVType_Initialize(OpcUa_XVType* pValue);

void OpcUa_XVType_Clear(OpcUa_XVType* pValue);

//StatusCode OpcUa_XVType_GetSize(OpcUa_XVType* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_XVType_Encode(OpcUa_XVType* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_XVType_Decode(OpcUa_XVType* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_XVType_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ProgramDiagnosticDataType
/*============================================================================
 * The ProgramDiagnosticDataType structure.
 *===========================================================================*/
typedef struct _OpcUa_ProgramDiagnosticDataType
{
    UA_NodeId          CreateSessionId;
    UA_String          CreateClientName;
    UA_DateTime        InvocationCreationTime;
    UA_DateTime        LastTransitionTime;
    UA_String          LastMethodCall;
    UA_NodeId          LastMethodSessionId;
    int32_t            NoOfLastMethodInputArguments;
    OpcUa_Argument*    LastMethodInputArguments;
    int32_t            NoOfLastMethodOutputArguments;
    OpcUa_Argument*    LastMethodOutputArguments;
    UA_DateTime        LastMethodCallTime;
    OpcUa_StatusResult LastMethodReturnStatus;
}
OpcUa_ProgramDiagnosticDataType;

void OpcUa_ProgramDiagnosticDataType_Initialize(OpcUa_ProgramDiagnosticDataType* pValue);

void OpcUa_ProgramDiagnosticDataType_Clear(OpcUa_ProgramDiagnosticDataType* pValue);

//StatusCode OpcUa_ProgramDiagnosticDataType_GetSize(OpcUa_ProgramDiagnosticDataType* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_ProgramDiagnosticDataType_Encode(OpcUa_ProgramDiagnosticDataType* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ProgramDiagnosticDataType_Decode(OpcUa_ProgramDiagnosticDataType* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_ProgramDiagnosticDataType_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_Annotation
/*============================================================================
 * The Annotation structure.
 *===========================================================================*/
typedef struct _OpcUa_Annotation
{
    UA_String   Message;
    UA_String   UserName;
    UA_DateTime AnnotationTime;
}
OpcUa_Annotation;

void OpcUa_Annotation_Initialize(OpcUa_Annotation* pValue);

void OpcUa_Annotation_Clear(OpcUa_Annotation* pValue);

//StatusCode OpcUa_Annotation_GetSize(OpcUa_Annotation* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

SOPC_StatusCode OpcUa_Annotation_Encode(OpcUa_Annotation* pValue, UA_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_Annotation_Decode(OpcUa_Annotation* pValue, UA_MsgBuffer* msgBuf);

extern struct UA_EncodeableType OpcUa_Annotation_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ExceptionDeviationFormat
/*============================================================================
 * The ExceptionDeviationFormat enumeration.
 *===========================================================================*/
typedef enum _OpcUa_ExceptionDeviationFormat
{
    OpcUa_ExceptionDeviationFormat_AbsoluteValue    = 0,
    OpcUa_ExceptionDeviationFormat_PercentOfValue   = 1,
    OpcUa_ExceptionDeviationFormat_PercentOfRange   = 2,
    OpcUa_ExceptionDeviationFormat_PercentOfEURange = 3,
    OpcUa_ExceptionDeviationFormat_Unknown          = 4
#if OPCUA_FORCE_INT32_ENUMS
    ,_OpcUa_ExceptionDeviationFormat_MaxEnumerationValue = OpcUa_Int32_Max
#endif
}
OpcUa_ExceptionDeviationFormat;

#endif

void UA_Initialize_EnumeratedType(int32_t* enumerationValue);
void UA_Initialize_Array(int32_t* noOfElts, void** eltsArray, size_t sizeOfElt,
                         UA_EncodeableObject_PfnInitialize* initFct);

void UA_Clear_EnumeratedType(int32_t* enumerationValue);
void UA_Clear_Array(int32_t* noOfElts, void** eltsArray, size_t sizeOfElt,
                    UA_EncodeableObject_PfnClear* clearFct);
                    

SOPC_StatusCode UA_Read_EnumeratedType(UA_MsgBuffer* msgBuf, int32_t* enumerationValue);
SOPC_StatusCode UA_Read_Array(UA_MsgBuffer* msgBuf, int32_t* noOfElts, void** eltsArray,
                         size_t sizeOfElt, UA_EncodeableObject_PfnDecode* decodeFct);
                    

SOPC_StatusCode UA_Write_EnumeratedType(UA_MsgBuffer* msgBuf, int32_t* enumerationValue);
SOPC_StatusCode UA_Write_Array(UA_MsgBuffer* msgBuf, int32_t* noOfElts, void** eltsArray,
                          size_t sizeOfElt, UA_EncodeableObject_PfnEncode* encodeFct);

/*============================================================================
 * Table of known types.
 *===========================================================================*/
extern struct UA_EncodeableType** UA_KnownEncodeableTypes;

END_EXTERN_C

#endif
/* This is the last line of an autogenerated file. */
