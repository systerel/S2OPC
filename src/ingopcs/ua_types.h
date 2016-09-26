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

#include <ua_msg_buffer.h>
#include <ua_builtintypes.h>
#include <ua_encodeable.h>

OPCUA_BEGIN_EXTERN_C

#ifndef OPCUA_EXCLUDE_IdType
/*============================================================================
 * The IdType enumeration.
 *===========================================================================*/
typedef enum _UA_IdType
{
    UA_IdType_Numeric = 0,
    UA_IdType_String  = 1,
    UA_IdType_Guid    = 2,
    UA_IdType_Opaque  = 3
#if OPCUA_FORCE_INT32_ENUMS
    ,_UA_IdType_MaxEnumerationValue = OpcUa_Int32_Max
#endif
}
UA_IdType;

#endif

#ifndef OPCUA_EXCLUDE_NodeClass
/*============================================================================
 * The NodeClass enumeration.
 *===========================================================================*/
typedef enum _UA_NodeClass
{
    UA_NodeClass_Unspecified   = 0,
    UA_NodeClass_Object        = 1,
    UA_NodeClass_Variable      = 2,
    UA_NodeClass_Method        = 4,
    UA_NodeClass_ObjectType    = 8,
    UA_NodeClass_VariableType  = 16,
    UA_NodeClass_ReferenceType = 32,
    UA_NodeClass_DataType      = 64,
    UA_NodeClass_View          = 128
#if OPCUA_FORCE_INT32_ENUMS
    ,_UA_NodeClass_MaxEnumerationValue = OpcUa_Int32_Max
#endif
}
UA_NodeClass;

#endif

#ifndef OPCUA_EXCLUDE_ReferenceNode
/*============================================================================
 * The ReferenceNode structure.
 *===========================================================================*/
typedef struct _UA_ReferenceNode
{
    UA_NodeId         ReferenceTypeId;
    UA_Boolean        IsInverse;
    UA_ExpandedNodeId TargetId;
}
UA_ReferenceNode;

void UA_ReferenceNode_Initialize(UA_ReferenceNode* pValue);

void UA_ReferenceNode_Clear(UA_ReferenceNode* pValue);

//StatusCode UA_ReferenceNode_GetSize(UA_ReferenceNode* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_ReferenceNode_Encode(UA_MsgBuffer* msgBuf, UA_ReferenceNode* pValue);

StatusCode UA_ReferenceNode_Decode(UA_MsgBuffer* msgBuf, UA_ReferenceNode* pValue);

extern struct UA_EncodeableType UA_ReferenceNode_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_Node
/*============================================================================
 * The Node structure.
 *===========================================================================*/
typedef struct _UA_Node
{
    UA_NodeId         NodeId;
    UA_NodeClass      NodeClass;
    UA_QualifiedName  BrowseName;
    UA_LocalizedText  DisplayName;
    UA_LocalizedText  Description;
    uint32_t          WriteMask;
    uint32_t          UserWriteMask;
    int32_t           NoOfReferences;
    UA_ReferenceNode* References;
}
UA_Node;

void UA_Node_Initialize(UA_Node* pValue);

void UA_Node_Clear(UA_Node* pValue);

//StatusCode UA_Node_GetSize(UA_Node* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_Node_Encode(UA_MsgBuffer* msgBuf, UA_Node* pValue);

StatusCode UA_Node_Decode(UA_MsgBuffer* msgBuf, UA_Node* pValue);

extern struct UA_EncodeableType UA_Node_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_InstanceNode
/*============================================================================
 * The InstanceNode structure.
 *===========================================================================*/
typedef struct _UA_InstanceNode
{
    UA_NodeId         NodeId;
    UA_NodeClass      NodeClass;
    UA_QualifiedName  BrowseName;
    UA_LocalizedText  DisplayName;
    UA_LocalizedText  Description;
    uint32_t          WriteMask;
    uint32_t          UserWriteMask;
    int32_t           NoOfReferences;
    UA_ReferenceNode* References;
}
UA_InstanceNode;

void UA_InstanceNode_Initialize(UA_InstanceNode* pValue);

void UA_InstanceNode_Clear(UA_InstanceNode* pValue);

//StatusCode UA_InstanceNode_GetSize(UA_InstanceNode* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_InstanceNode_Encode(UA_MsgBuffer* msgBuf, UA_InstanceNode* pValue);

StatusCode UA_InstanceNode_Decode(UA_MsgBuffer* msgBuf, UA_InstanceNode* pValue);

extern struct UA_EncodeableType UA_InstanceNode_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_TypeNode
/*============================================================================
 * The TypeNode structure.
 *===========================================================================*/
typedef struct _UA_TypeNode
{
    UA_NodeId         NodeId;
    UA_NodeClass      NodeClass;
    UA_QualifiedName  BrowseName;
    UA_LocalizedText  DisplayName;
    UA_LocalizedText  Description;
    uint32_t          WriteMask;
    uint32_t          UserWriteMask;
    int32_t           NoOfReferences;
    UA_ReferenceNode* References;
}
UA_TypeNode;

void UA_TypeNode_Initialize(UA_TypeNode* pValue);

void UA_TypeNode_Clear(UA_TypeNode* pValue);

//StatusCode UA_TypeNode_GetSize(UA_TypeNode* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_TypeNode_Encode(UA_MsgBuffer* msgBuf, UA_TypeNode* pValue);

StatusCode UA_TypeNode_Decode(UA_MsgBuffer* msgBuf, UA_TypeNode* pValue);

extern struct UA_EncodeableType UA_TypeNode_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ObjectNode
/*============================================================================
 * The ObjectNode structure.
 *===========================================================================*/
typedef struct _UA_ObjectNode
{
    UA_NodeId         NodeId;
    UA_NodeClass      NodeClass;
    UA_QualifiedName  BrowseName;
    UA_LocalizedText  DisplayName;
    UA_LocalizedText  Description;
    uint32_t          WriteMask;
    uint32_t          UserWriteMask;
    int32_t           NoOfReferences;
    UA_ReferenceNode* References;
    UA_Byte           EventNotifier;
}
UA_ObjectNode;

void UA_ObjectNode_Initialize(UA_ObjectNode* pValue);

void UA_ObjectNode_Clear(UA_ObjectNode* pValue);

//StatusCode UA_ObjectNode_GetSize(UA_ObjectNode* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_ObjectNode_Encode(UA_MsgBuffer* msgBuf, UA_ObjectNode* pValue);

StatusCode UA_ObjectNode_Decode(UA_MsgBuffer* msgBuf, UA_ObjectNode* pValue);

extern struct UA_EncodeableType UA_ObjectNode_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ObjectTypeNode
/*============================================================================
 * The ObjectTypeNode structure.
 *===========================================================================*/
typedef struct _UA_ObjectTypeNode
{
    UA_NodeId         NodeId;
    UA_NodeClass      NodeClass;
    UA_QualifiedName  BrowseName;
    UA_LocalizedText  DisplayName;
    UA_LocalizedText  Description;
    uint32_t          WriteMask;
    uint32_t          UserWriteMask;
    int32_t           NoOfReferences;
    UA_ReferenceNode* References;
    UA_Boolean        IsAbstract;
}
UA_ObjectTypeNode;

void UA_ObjectTypeNode_Initialize(UA_ObjectTypeNode* pValue);

void UA_ObjectTypeNode_Clear(UA_ObjectTypeNode* pValue);

//StatusCode UA_ObjectTypeNode_GetSize(UA_ObjectTypeNode* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_ObjectTypeNode_Encode(UA_MsgBuffer* msgBuf, UA_ObjectTypeNode* pValue);

StatusCode UA_ObjectTypeNode_Decode(UA_MsgBuffer* msgBuf, UA_ObjectTypeNode* pValue);

extern struct UA_EncodeableType UA_ObjectTypeNode_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_VariableNode
/*============================================================================
 * The VariableNode structure.
 *===========================================================================*/
typedef struct _UA_VariableNode
{
    UA_NodeId         NodeId;
    UA_NodeClass      NodeClass;
    UA_QualifiedName  BrowseName;
    UA_LocalizedText  DisplayName;
    UA_LocalizedText  Description;
    uint32_t          WriteMask;
    uint32_t          UserWriteMask;
    int32_t           NoOfReferences;
    UA_ReferenceNode* References;
    UA_Variant        Value;
    UA_NodeId         DataType;
    int32_t           ValueRank;
    int32_t           NoOfArrayDimensions;
    uint32_t*         ArrayDimensions;
    UA_Byte           AccessLevel;
    UA_Byte           UserAccessLevel;
    double            MinimumSamplingInterval;
    UA_Boolean        Historizing;
}
UA_VariableNode;

void UA_VariableNode_Initialize(UA_VariableNode* pValue);

void UA_VariableNode_Clear(UA_VariableNode* pValue);

//StatusCode UA_VariableNode_GetSize(UA_VariableNode* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_VariableNode_Encode(UA_MsgBuffer* msgBuf, UA_VariableNode* pValue);

StatusCode UA_VariableNode_Decode(UA_MsgBuffer* msgBuf, UA_VariableNode* pValue);

extern struct UA_EncodeableType UA_VariableNode_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_VariableTypeNode
/*============================================================================
 * The VariableTypeNode structure.
 *===========================================================================*/
typedef struct _UA_VariableTypeNode
{
    UA_NodeId         NodeId;
    UA_NodeClass      NodeClass;
    UA_QualifiedName  BrowseName;
    UA_LocalizedText  DisplayName;
    UA_LocalizedText  Description;
    uint32_t          WriteMask;
    uint32_t          UserWriteMask;
    int32_t           NoOfReferences;
    UA_ReferenceNode* References;
    UA_Variant        Value;
    UA_NodeId         DataType;
    int32_t           ValueRank;
    int32_t           NoOfArrayDimensions;
    uint32_t*         ArrayDimensions;
    UA_Boolean        IsAbstract;
}
UA_VariableTypeNode;

void UA_VariableTypeNode_Initialize(UA_VariableTypeNode* pValue);

void UA_VariableTypeNode_Clear(UA_VariableTypeNode* pValue);

//StatusCode UA_VariableTypeNode_GetSize(UA_VariableTypeNode* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_VariableTypeNode_Encode(UA_MsgBuffer* msgBuf, UA_VariableTypeNode* pValue);

StatusCode UA_VariableTypeNode_Decode(UA_MsgBuffer* msgBuf, UA_VariableTypeNode* pValue);

extern struct UA_EncodeableType UA_VariableTypeNode_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ReferenceTypeNode
/*============================================================================
 * The ReferenceTypeNode structure.
 *===========================================================================*/
typedef struct _UA_ReferenceTypeNode
{
    UA_NodeId         NodeId;
    UA_NodeClass      NodeClass;
    UA_QualifiedName  BrowseName;
    UA_LocalizedText  DisplayName;
    UA_LocalizedText  Description;
    uint32_t          WriteMask;
    uint32_t          UserWriteMask;
    int32_t           NoOfReferences;
    UA_ReferenceNode* References;
    UA_Boolean        IsAbstract;
    UA_Boolean        Symmetric;
    UA_LocalizedText  InverseName;
}
UA_ReferenceTypeNode;

void UA_ReferenceTypeNode_Initialize(UA_ReferenceTypeNode* pValue);

void UA_ReferenceTypeNode_Clear(UA_ReferenceTypeNode* pValue);

//StatusCode UA_ReferenceTypeNode_GetSize(UA_ReferenceTypeNode* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_ReferenceTypeNode_Encode(UA_MsgBuffer* msgBuf, UA_ReferenceTypeNode* pValue);

StatusCode UA_ReferenceTypeNode_Decode(UA_MsgBuffer* msgBuf, UA_ReferenceTypeNode* pValue);

extern struct UA_EncodeableType UA_ReferenceTypeNode_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_MethodNode
/*============================================================================
 * The MethodNode structure.
 *===========================================================================*/
typedef struct _UA_MethodNode
{
    UA_NodeId         NodeId;
    UA_NodeClass      NodeClass;
    UA_QualifiedName  BrowseName;
    UA_LocalizedText  DisplayName;
    UA_LocalizedText  Description;
    uint32_t          WriteMask;
    uint32_t          UserWriteMask;
    int32_t           NoOfReferences;
    UA_ReferenceNode* References;
    UA_Boolean        Executable;
    UA_Boolean        UserExecutable;
}
UA_MethodNode;

void UA_MethodNode_Initialize(UA_MethodNode* pValue);

void UA_MethodNode_Clear(UA_MethodNode* pValue);

//StatusCode UA_MethodNode_GetSize(UA_MethodNode* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_MethodNode_Encode(UA_MsgBuffer* msgBuf, UA_MethodNode* pValue);

StatusCode UA_MethodNode_Decode(UA_MsgBuffer* msgBuf, UA_MethodNode* pValue);

extern struct UA_EncodeableType UA_MethodNode_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ViewNode
/*============================================================================
 * The ViewNode structure.
 *===========================================================================*/
typedef struct _UA_ViewNode
{
    UA_NodeId         NodeId;
    UA_NodeClass      NodeClass;
    UA_QualifiedName  BrowseName;
    UA_LocalizedText  DisplayName;
    UA_LocalizedText  Description;
    uint32_t          WriteMask;
    uint32_t          UserWriteMask;
    int32_t           NoOfReferences;
    UA_ReferenceNode* References;
    UA_Boolean        ContainsNoLoops;
    UA_Byte           EventNotifier;
}
UA_ViewNode;

void UA_ViewNode_Initialize(UA_ViewNode* pValue);

void UA_ViewNode_Clear(UA_ViewNode* pValue);

//StatusCode UA_ViewNode_GetSize(UA_ViewNode* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_ViewNode_Encode(UA_MsgBuffer* msgBuf, UA_ViewNode* pValue);

StatusCode UA_ViewNode_Decode(UA_MsgBuffer* msgBuf, UA_ViewNode* pValue);

extern struct UA_EncodeableType UA_ViewNode_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_DataTypeNode
/*============================================================================
 * The DataTypeNode structure.
 *===========================================================================*/
typedef struct _UA_DataTypeNode
{
    UA_NodeId         NodeId;
    UA_NodeClass      NodeClass;
    UA_QualifiedName  BrowseName;
    UA_LocalizedText  DisplayName;
    UA_LocalizedText  Description;
    uint32_t          WriteMask;
    uint32_t          UserWriteMask;
    int32_t           NoOfReferences;
    UA_ReferenceNode* References;
    UA_Boolean        IsAbstract;
}
UA_DataTypeNode;

void UA_DataTypeNode_Initialize(UA_DataTypeNode* pValue);

void UA_DataTypeNode_Clear(UA_DataTypeNode* pValue);

//StatusCode UA_DataTypeNode_GetSize(UA_DataTypeNode* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_DataTypeNode_Encode(UA_MsgBuffer* msgBuf, UA_DataTypeNode* pValue);

StatusCode UA_DataTypeNode_Decode(UA_MsgBuffer* msgBuf, UA_DataTypeNode* pValue);

extern struct UA_EncodeableType UA_DataTypeNode_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_Argument
/*============================================================================
 * The Argument structure.
 *===========================================================================*/
typedef struct _UA_Argument
{
    UA_String        Name;
    UA_NodeId        DataType;
    int32_t          ValueRank;
    int32_t          NoOfArrayDimensions;
    uint32_t*        ArrayDimensions;
    UA_LocalizedText Description;
}
UA_Argument;

void UA_Argument_Initialize(UA_Argument* pValue);

void UA_Argument_Clear(UA_Argument* pValue);

//StatusCode UA_Argument_GetSize(UA_Argument* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_Argument_Encode(UA_MsgBuffer* msgBuf, UA_Argument* pValue);

StatusCode UA_Argument_Decode(UA_MsgBuffer* msgBuf, UA_Argument* pValue);

extern struct UA_EncodeableType UA_Argument_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_EnumValueType
/*============================================================================
 * The EnumValueType structure.
 *===========================================================================*/
typedef struct _UA_EnumValueType
{
    int64_t          Value;
    UA_LocalizedText DisplayName;
    UA_LocalizedText Description;
}
UA_EnumValueType;

void UA_EnumValueType_Initialize(UA_EnumValueType* pValue);

void UA_EnumValueType_Clear(UA_EnumValueType* pValue);

//StatusCode UA_EnumValueType_GetSize(UA_EnumValueType* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_EnumValueType_Encode(UA_MsgBuffer* msgBuf, UA_EnumValueType* pValue);

StatusCode UA_EnumValueType_Decode(UA_MsgBuffer* msgBuf, UA_EnumValueType* pValue);

extern struct UA_EncodeableType UA_EnumValueType_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_EnumField
/*============================================================================
 * The EnumField structure.
 *===========================================================================*/
typedef struct _UA_EnumField
{
    int64_t          Value;
    UA_LocalizedText DisplayName;
    UA_LocalizedText Description;
    UA_String        Name;
}
UA_EnumField;

void UA_EnumField_Initialize(UA_EnumField* pValue);

void UA_EnumField_Clear(UA_EnumField* pValue);

//StatusCode UA_EnumField_GetSize(UA_EnumField* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_EnumField_Encode(UA_MsgBuffer* msgBuf, UA_EnumField* pValue);

StatusCode UA_EnumField_Decode(UA_MsgBuffer* msgBuf, UA_EnumField* pValue);

extern struct UA_EncodeableType UA_EnumField_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_OptionSet
/*============================================================================
 * The OptionSet structure.
 *===========================================================================*/
typedef struct _UA_OptionSet
{
    UA_ByteString Value;
    UA_ByteString ValidBits;
}
UA_OptionSet;

void UA_OptionSet_Initialize(UA_OptionSet* pValue);

void UA_OptionSet_Clear(UA_OptionSet* pValue);

//StatusCode UA_OptionSet_GetSize(UA_OptionSet* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_OptionSet_Encode(UA_MsgBuffer* msgBuf, UA_OptionSet* pValue);

StatusCode UA_OptionSet_Decode(UA_MsgBuffer* msgBuf, UA_OptionSet* pValue);

extern struct UA_EncodeableType UA_OptionSet_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_TimeZoneDataType
/*============================================================================
 * The TimeZoneDataType structure.
 *===========================================================================*/
typedef struct _UA_TimeZoneDataType
{
    int16_t    Offset;
    UA_Boolean DaylightSavingInOffset;
}
UA_TimeZoneDataType;

void UA_TimeZoneDataType_Initialize(UA_TimeZoneDataType* pValue);

void UA_TimeZoneDataType_Clear(UA_TimeZoneDataType* pValue);

//StatusCode UA_TimeZoneDataType_GetSize(UA_TimeZoneDataType* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_TimeZoneDataType_Encode(UA_MsgBuffer* msgBuf, UA_TimeZoneDataType* pValue);

StatusCode UA_TimeZoneDataType_Decode(UA_MsgBuffer* msgBuf, UA_TimeZoneDataType* pValue);

extern struct UA_EncodeableType UA_TimeZoneDataType_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ApplicationType
/*============================================================================
 * The ApplicationType enumeration.
 *===========================================================================*/
typedef enum _UA_ApplicationType
{
    UA_ApplicationType_Server          = 0,
    UA_ApplicationType_Client          = 1,
    UA_ApplicationType_ClientAndServer = 2,
    UA_ApplicationType_DiscoveryServer = 3
#if OPCUA_FORCE_INT32_ENUMS
    ,_UA_ApplicationType_MaxEnumerationValue = OpcUa_Int32_Max
#endif
}
UA_ApplicationType;

#endif

#ifndef OPCUA_EXCLUDE_ApplicationDescription
/*============================================================================
 * The ApplicationDescription structure.
 *===========================================================================*/
typedef struct _UA_ApplicationDescription
{
    UA_String          ApplicationUri;
    UA_String          ProductUri;
    UA_LocalizedText   ApplicationName;
    UA_ApplicationType ApplicationType;
    UA_String          GatewayServerUri;
    UA_String          DiscoveryProfileUri;
    int32_t            NoOfDiscoveryUrls;
    UA_String*         DiscoveryUrls;
}
UA_ApplicationDescription;

void UA_ApplicationDescription_Initialize(UA_ApplicationDescription* pValue);

void UA_ApplicationDescription_Clear(UA_ApplicationDescription* pValue);

//StatusCode UA_ApplicationDescription_GetSize(UA_ApplicationDescription* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_ApplicationDescription_Encode(UA_MsgBuffer* msgBuf, UA_ApplicationDescription* pValue);

StatusCode UA_ApplicationDescription_Decode(UA_MsgBuffer* msgBuf, UA_ApplicationDescription* pValue);

extern struct UA_EncodeableType UA_ApplicationDescription_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_RequestHeader
/*============================================================================
 * The RequestHeader structure.
 *===========================================================================*/
typedef struct _UA_RequestHeader
{
    UA_NodeId          AuthenticationToken;
    UA_DateTime        Timestamp;
    uint32_t           RequestHandle;
    uint32_t           ReturnDiagnostics;
    UA_String          AuditEntryId;
    uint32_t           TimeoutHint;
    UA_ExtensionObject AdditionalHeader;
}
UA_RequestHeader;

void UA_RequestHeader_Initialize(UA_RequestHeader* pValue);

void UA_RequestHeader_Clear(UA_RequestHeader* pValue);

//StatusCode UA_RequestHeader_GetSize(UA_RequestHeader* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_RequestHeader_Encode(UA_MsgBuffer* msgBuf, UA_RequestHeader* pValue);

StatusCode UA_RequestHeader_Decode(UA_MsgBuffer* msgBuf, UA_RequestHeader* pValue);

extern struct UA_EncodeableType UA_RequestHeader_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ResponseHeader
/*============================================================================
 * The ResponseHeader structure.
 *===========================================================================*/
typedef struct _UA_ResponseHeader
{
    UA_DateTime        Timestamp;
    uint32_t           RequestHandle;
    StatusCode         ServiceResult;
    UA_DiagnosticInfo  ServiceDiagnostics;
    int32_t            NoOfStringTable;
    UA_String*         StringTable;
    UA_ExtensionObject AdditionalHeader;
}
UA_ResponseHeader;

void UA_ResponseHeader_Initialize(UA_ResponseHeader* pValue);

void UA_ResponseHeader_Clear(UA_ResponseHeader* pValue);

//StatusCode UA_ResponseHeader_GetSize(UA_ResponseHeader* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_ResponseHeader_Encode(UA_MsgBuffer* msgBuf, UA_ResponseHeader* pValue);

StatusCode UA_ResponseHeader_Decode(UA_MsgBuffer* msgBuf, UA_ResponseHeader* pValue);

extern struct UA_EncodeableType UA_ResponseHeader_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ServiceFault
/*============================================================================
 * The ServiceFault structure.
 *===========================================================================*/
typedef struct _UA_ServiceFault
{
    UA_ResponseHeader ResponseHeader;
}
UA_ServiceFault;

void UA_ServiceFault_Initialize(UA_ServiceFault* pValue);

void UA_ServiceFault_Clear(UA_ServiceFault* pValue);

//StatusCode UA_ServiceFault_GetSize(UA_ServiceFault* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_ServiceFault_Encode(UA_MsgBuffer* msgBuf, UA_ServiceFault* pValue);

StatusCode UA_ServiceFault_Decode(UA_MsgBuffer* msgBuf, UA_ServiceFault* pValue);

extern struct UA_EncodeableType UA_ServiceFault_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_FindServers
#ifndef OPCUA_EXCLUDE_FindServersRequest
/*============================================================================
 * The FindServersRequest structure.
 *===========================================================================*/
typedef struct _UA_FindServersRequest
{
    UA_RequestHeader RequestHeader;
    UA_String        EndpointUrl;
    int32_t          NoOfLocaleIds;
    UA_String*       LocaleIds;
    int32_t          NoOfServerUris;
    UA_String*       ServerUris;
}
UA_FindServersRequest;

void UA_FindServersRequest_Initialize(UA_FindServersRequest* pValue);

void UA_FindServersRequest_Clear(UA_FindServersRequest* pValue);

//StatusCode UA_FindServersRequest_GetSize(UA_FindServersRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_FindServersRequest_Encode(UA_MsgBuffer* msgBuf, UA_FindServersRequest* pValue);

StatusCode UA_FindServersRequest_Decode(UA_MsgBuffer* msgBuf, UA_FindServersRequest* pValue);

extern struct UA_EncodeableType UA_FindServersRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_FindServersResponse
/*============================================================================
 * The FindServersResponse structure.
 *===========================================================================*/
typedef struct _UA_FindServersResponse
{
    UA_ResponseHeader          ResponseHeader;
    int32_t                    NoOfServers;
    UA_ApplicationDescription* Servers;
}
UA_FindServersResponse;

void UA_FindServersResponse_Initialize(UA_FindServersResponse* pValue);

void UA_FindServersResponse_Clear(UA_FindServersResponse* pValue);

//StatusCode UA_FindServersResponse_GetSize(UA_FindServersResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_FindServersResponse_Encode(UA_MsgBuffer* msgBuf, UA_FindServersResponse* pValue);

StatusCode UA_FindServersResponse_Decode(UA_MsgBuffer* msgBuf, UA_FindServersResponse* pValue);

extern struct UA_EncodeableType UA_FindServersResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_ServerOnNetwork
/*============================================================================
 * The ServerOnNetwork structure.
 *===========================================================================*/
typedef struct _UA_ServerOnNetwork
{
    uint32_t   RecordId;
    UA_String  ServerName;
    UA_String  DiscoveryUrl;
    int32_t    NoOfServerCapabilities;
    UA_String* ServerCapabilities;
}
UA_ServerOnNetwork;

void UA_ServerOnNetwork_Initialize(UA_ServerOnNetwork* pValue);

void UA_ServerOnNetwork_Clear(UA_ServerOnNetwork* pValue);

//StatusCode UA_ServerOnNetwork_GetSize(UA_ServerOnNetwork* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_ServerOnNetwork_Encode(UA_MsgBuffer* msgBuf, UA_ServerOnNetwork* pValue);

StatusCode UA_ServerOnNetwork_Decode(UA_MsgBuffer* msgBuf, UA_ServerOnNetwork* pValue);

extern struct UA_EncodeableType UA_ServerOnNetwork_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_FindServersOnNetwork
#ifndef OPCUA_EXCLUDE_FindServersOnNetworkRequest
/*============================================================================
 * The FindServersOnNetworkRequest structure.
 *===========================================================================*/
typedef struct _UA_FindServersOnNetworkRequest
{
    UA_RequestHeader RequestHeader;
    uint32_t         StartingRecordId;
    uint32_t         MaxRecordsToReturn;
    int32_t          NoOfServerCapabilityFilter;
    UA_String*       ServerCapabilityFilter;
}
UA_FindServersOnNetworkRequest;

void UA_FindServersOnNetworkRequest_Initialize(UA_FindServersOnNetworkRequest* pValue);

void UA_FindServersOnNetworkRequest_Clear(UA_FindServersOnNetworkRequest* pValue);

//StatusCode UA_FindServersOnNetworkRequest_GetSize(UA_FindServersOnNetworkRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_FindServersOnNetworkRequest_Encode(UA_MsgBuffer* msgBuf, UA_FindServersOnNetworkRequest* pValue);

StatusCode UA_FindServersOnNetworkRequest_Decode(UA_MsgBuffer* msgBuf, UA_FindServersOnNetworkRequest* pValue);

extern struct UA_EncodeableType UA_FindServersOnNetworkRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_FindServersOnNetworkResponse
/*============================================================================
 * The FindServersOnNetworkResponse structure.
 *===========================================================================*/
typedef struct _UA_FindServersOnNetworkResponse
{
    UA_ResponseHeader   ResponseHeader;
    UA_DateTime         LastCounterResetTime;
    int32_t             NoOfServers;
    UA_ServerOnNetwork* Servers;
}
UA_FindServersOnNetworkResponse;

void UA_FindServersOnNetworkResponse_Initialize(UA_FindServersOnNetworkResponse* pValue);

void UA_FindServersOnNetworkResponse_Clear(UA_FindServersOnNetworkResponse* pValue);

//StatusCode UA_FindServersOnNetworkResponse_GetSize(UA_FindServersOnNetworkResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_FindServersOnNetworkResponse_Encode(UA_MsgBuffer* msgBuf, UA_FindServersOnNetworkResponse* pValue);

StatusCode UA_FindServersOnNetworkResponse_Decode(UA_MsgBuffer* msgBuf, UA_FindServersOnNetworkResponse* pValue);

extern struct UA_EncodeableType UA_FindServersOnNetworkResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_MessageSecurityMode
/*============================================================================
 * The MessageSecurityMode enumeration.
 *===========================================================================*/
typedef enum _UA_MessageSecurityMode
{
    UA_MessageSecurityMode_Invalid        = 0,
    UA_MessageSecurityMode_None           = 1,
    UA_MessageSecurityMode_Sign           = 2,
    UA_MessageSecurityMode_SignAndEncrypt = 3
#if OPCUA_FORCE_INT32_ENUMS
    ,_UA_MessageSecurityMode_MaxEnumerationValue = OpcUa_Int32_Max
#endif
}
UA_MessageSecurityMode;

#endif

#ifndef OPCUA_EXCLUDE_UserTokenType
/*============================================================================
 * The UserTokenType enumeration.
 *===========================================================================*/
typedef enum _UA_UserTokenType
{
    UA_UserTokenType_Anonymous   = 0,
    UA_UserTokenType_UserName    = 1,
    UA_UserTokenType_Certificate = 2,
    UA_UserTokenType_IssuedToken = 3,
    UA_UserTokenType_Kerberos    = 4
#if OPCUA_FORCE_INT32_ENUMS
    ,_UA_UserTokenType_MaxEnumerationValue = OpcUa_Int32_Max
#endif
}
UA_UserTokenType;

#endif

#ifndef OPCUA_EXCLUDE_UserTokenPolicy
/*============================================================================
 * The UserTokenPolicy structure.
 *===========================================================================*/
typedef struct _UA_UserTokenPolicy
{
    UA_String        PolicyId;
    UA_UserTokenType TokenType;
    UA_String        IssuedTokenType;
    UA_String        IssuerEndpointUrl;
    UA_String        SecurityPolicyUri;
}
UA_UserTokenPolicy;

void UA_UserTokenPolicy_Initialize(UA_UserTokenPolicy* pValue);

void UA_UserTokenPolicy_Clear(UA_UserTokenPolicy* pValue);

//StatusCode UA_UserTokenPolicy_GetSize(UA_UserTokenPolicy* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_UserTokenPolicy_Encode(UA_MsgBuffer* msgBuf, UA_UserTokenPolicy* pValue);

StatusCode UA_UserTokenPolicy_Decode(UA_MsgBuffer* msgBuf, UA_UserTokenPolicy* pValue);

extern struct UA_EncodeableType UA_UserTokenPolicy_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_EndpointDescription
/*============================================================================
 * The EndpointDescription structure.
 *===========================================================================*/
typedef struct _UA_EndpointDescription
{
    UA_String                 EndpointUrl;
    UA_ApplicationDescription Server;
    UA_ByteString             ServerCertificate;
    UA_MessageSecurityMode    SecurityMode;
    UA_String                 SecurityPolicyUri;
    int32_t                   NoOfUserIdentityTokens;
    UA_UserTokenPolicy*       UserIdentityTokens;
    UA_String                 TransportProfileUri;
    UA_Byte                   SecurityLevel;
}
UA_EndpointDescription;

void UA_EndpointDescription_Initialize(UA_EndpointDescription* pValue);

void UA_EndpointDescription_Clear(UA_EndpointDescription* pValue);

//StatusCode UA_EndpointDescription_GetSize(UA_EndpointDescription* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_EndpointDescription_Encode(UA_MsgBuffer* msgBuf, UA_EndpointDescription* pValue);

StatusCode UA_EndpointDescription_Decode(UA_MsgBuffer* msgBuf, UA_EndpointDescription* pValue);

extern struct UA_EncodeableType UA_EndpointDescription_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_GetEndpoints
#ifndef OPCUA_EXCLUDE_GetEndpointsRequest
/*============================================================================
 * The GetEndpointsRequest structure.
 *===========================================================================*/
typedef struct _UA_GetEndpointsRequest
{
    UA_RequestHeader RequestHeader;
    UA_String        EndpointUrl;
    int32_t          NoOfLocaleIds;
    UA_String*       LocaleIds;
    int32_t          NoOfProfileUris;
    UA_String*       ProfileUris;
}
UA_GetEndpointsRequest;

void UA_GetEndpointsRequest_Initialize(UA_GetEndpointsRequest* pValue);

void UA_GetEndpointsRequest_Clear(UA_GetEndpointsRequest* pValue);

//StatusCode UA_GetEndpointsRequest_GetSize(UA_GetEndpointsRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_GetEndpointsRequest_Encode(UA_MsgBuffer* msgBuf, UA_GetEndpointsRequest* pValue);

StatusCode UA_GetEndpointsRequest_Decode(UA_MsgBuffer* msgBuf, UA_GetEndpointsRequest* pValue);

extern struct UA_EncodeableType UA_GetEndpointsRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_GetEndpointsResponse
/*============================================================================
 * The GetEndpointsResponse structure.
 *===========================================================================*/
typedef struct _UA_GetEndpointsResponse
{
    UA_ResponseHeader       ResponseHeader;
    int32_t                 NoOfEndpoints;
    UA_EndpointDescription* Endpoints;
}
UA_GetEndpointsResponse;

void UA_GetEndpointsResponse_Initialize(UA_GetEndpointsResponse* pValue);

void UA_GetEndpointsResponse_Clear(UA_GetEndpointsResponse* pValue);

//StatusCode UA_GetEndpointsResponse_GetSize(UA_GetEndpointsResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_GetEndpointsResponse_Encode(UA_MsgBuffer* msgBuf, UA_GetEndpointsResponse* pValue);

StatusCode UA_GetEndpointsResponse_Decode(UA_MsgBuffer* msgBuf, UA_GetEndpointsResponse* pValue);

extern struct UA_EncodeableType UA_GetEndpointsResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_RegisteredServer
/*============================================================================
 * The RegisteredServer structure.
 *===========================================================================*/
typedef struct _UA_RegisteredServer
{
    UA_String          ServerUri;
    UA_String          ProductUri;
    int32_t            NoOfServerNames;
    UA_LocalizedText*  ServerNames;
    UA_ApplicationType ServerType;
    UA_String          GatewayServerUri;
    int32_t            NoOfDiscoveryUrls;
    UA_String*         DiscoveryUrls;
    UA_String          SemaphoreFilePath;
    UA_Boolean         IsOnline;
}
UA_RegisteredServer;

void UA_RegisteredServer_Initialize(UA_RegisteredServer* pValue);

void UA_RegisteredServer_Clear(UA_RegisteredServer* pValue);

//StatusCode UA_RegisteredServer_GetSize(UA_RegisteredServer* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_RegisteredServer_Encode(UA_MsgBuffer* msgBuf, UA_RegisteredServer* pValue);

StatusCode UA_RegisteredServer_Decode(UA_MsgBuffer* msgBuf, UA_RegisteredServer* pValue);

extern struct UA_EncodeableType UA_RegisteredServer_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_RegisterServer
#ifndef OPCUA_EXCLUDE_RegisterServerRequest
/*============================================================================
 * The RegisterServerRequest structure.
 *===========================================================================*/
typedef struct _UA_RegisterServerRequest
{
    UA_RequestHeader    RequestHeader;
    UA_RegisteredServer Server;
}
UA_RegisterServerRequest;

void UA_RegisterServerRequest_Initialize(UA_RegisterServerRequest* pValue);

void UA_RegisterServerRequest_Clear(UA_RegisterServerRequest* pValue);

//StatusCode UA_RegisterServerRequest_GetSize(UA_RegisterServerRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_RegisterServerRequest_Encode(UA_MsgBuffer* msgBuf, UA_RegisterServerRequest* pValue);

StatusCode UA_RegisterServerRequest_Decode(UA_MsgBuffer* msgBuf, UA_RegisterServerRequest* pValue);

extern struct UA_EncodeableType UA_RegisterServerRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_RegisterServerResponse
/*============================================================================
 * The RegisterServerResponse structure.
 *===========================================================================*/
typedef struct _UA_RegisterServerResponse
{
    UA_ResponseHeader ResponseHeader;
}
UA_RegisterServerResponse;

void UA_RegisterServerResponse_Initialize(UA_RegisterServerResponse* pValue);

void UA_RegisterServerResponse_Clear(UA_RegisterServerResponse* pValue);

//StatusCode UA_RegisterServerResponse_GetSize(UA_RegisterServerResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_RegisterServerResponse_Encode(UA_MsgBuffer* msgBuf, UA_RegisterServerResponse* pValue);

StatusCode UA_RegisterServerResponse_Decode(UA_MsgBuffer* msgBuf, UA_RegisterServerResponse* pValue);

extern struct UA_EncodeableType UA_RegisterServerResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_MdnsDiscoveryConfiguration
/*============================================================================
 * The MdnsDiscoveryConfiguration structure.
 *===========================================================================*/
typedef struct _UA_MdnsDiscoveryConfiguration
{
    UA_String  MdnsServerName;
    int32_t    NoOfServerCapabilities;
    UA_String* ServerCapabilities;
}
UA_MdnsDiscoveryConfiguration;

void UA_MdnsDiscoveryConfiguration_Initialize(UA_MdnsDiscoveryConfiguration* pValue);

void UA_MdnsDiscoveryConfiguration_Clear(UA_MdnsDiscoveryConfiguration* pValue);

//StatusCode UA_MdnsDiscoveryConfiguration_GetSize(UA_MdnsDiscoveryConfiguration* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_MdnsDiscoveryConfiguration_Encode(UA_MsgBuffer* msgBuf, UA_MdnsDiscoveryConfiguration* pValue);

StatusCode UA_MdnsDiscoveryConfiguration_Decode(UA_MsgBuffer* msgBuf, UA_MdnsDiscoveryConfiguration* pValue);

extern struct UA_EncodeableType UA_MdnsDiscoveryConfiguration_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_RegisterServer2
#ifndef OPCUA_EXCLUDE_RegisterServer2Request
/*============================================================================
 * The RegisterServer2Request structure.
 *===========================================================================*/
typedef struct _UA_RegisterServer2Request
{
    UA_RequestHeader    RequestHeader;
    UA_RegisteredServer Server;
    int32_t             NoOfDiscoveryConfiguration;
    UA_ExtensionObject* DiscoveryConfiguration;
}
UA_RegisterServer2Request;

void UA_RegisterServer2Request_Initialize(UA_RegisterServer2Request* pValue);

void UA_RegisterServer2Request_Clear(UA_RegisterServer2Request* pValue);

//StatusCode UA_RegisterServer2Request_GetSize(UA_RegisterServer2Request* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_RegisterServer2Request_Encode(UA_MsgBuffer* msgBuf, UA_RegisterServer2Request* pValue);

StatusCode UA_RegisterServer2Request_Decode(UA_MsgBuffer* msgBuf, UA_RegisterServer2Request* pValue);

extern struct UA_EncodeableType UA_RegisterServer2Request_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_RegisterServer2Response
/*============================================================================
 * The RegisterServer2Response structure.
 *===========================================================================*/
typedef struct _UA_RegisterServer2Response
{
    UA_ResponseHeader  ResponseHeader;
    int32_t            NoOfConfigurationResults;
    StatusCode*        ConfigurationResults;
    int32_t            NoOfDiagnosticInfos;
    UA_DiagnosticInfo* DiagnosticInfos;
}
UA_RegisterServer2Response;

void UA_RegisterServer2Response_Initialize(UA_RegisterServer2Response* pValue);

void UA_RegisterServer2Response_Clear(UA_RegisterServer2Response* pValue);

//StatusCode UA_RegisterServer2Response_GetSize(UA_RegisterServer2Response* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_RegisterServer2Response_Encode(UA_MsgBuffer* msgBuf, UA_RegisterServer2Response* pValue);

StatusCode UA_RegisterServer2Response_Decode(UA_MsgBuffer* msgBuf, UA_RegisterServer2Response* pValue);

extern struct UA_EncodeableType UA_RegisterServer2Response_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_SecurityTokenRequestType
/*============================================================================
 * The SecurityTokenRequestType enumeration.
 *===========================================================================*/
typedef enum _UA_SecurityTokenRequestType
{
    UA_SecurityTokenRequestType_Issue = 0,
    UA_SecurityTokenRequestType_Renew = 1
#if OPCUA_FORCE_INT32_ENUMS
    ,_UA_SecurityTokenRequestType_MaxEnumerationValue = OpcUa_Int32_Max
#endif
}
UA_SecurityTokenRequestType;

#endif

#ifndef OPCUA_EXCLUDE_ChannelSecurityToken
/*============================================================================
 * The ChannelSecurityToken structure.
 *===========================================================================*/
typedef struct _UA_ChannelSecurityToken
{
    uint32_t    ChannelId;
    uint32_t    TokenId;
    UA_DateTime CreatedAt;
    uint32_t    RevisedLifetime;
}
UA_ChannelSecurityToken;

void UA_ChannelSecurityToken_Initialize(UA_ChannelSecurityToken* pValue);

void UA_ChannelSecurityToken_Clear(UA_ChannelSecurityToken* pValue);

//StatusCode UA_ChannelSecurityToken_GetSize(UA_ChannelSecurityToken* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_ChannelSecurityToken_Encode(UA_MsgBuffer* msgBuf, UA_ChannelSecurityToken* pValue);

StatusCode UA_ChannelSecurityToken_Decode(UA_MsgBuffer* msgBuf, UA_ChannelSecurityToken* pValue);

extern struct UA_EncodeableType UA_ChannelSecurityToken_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_OpenSecureChannel
#ifndef OPCUA_EXCLUDE_OpenSecureChannelRequest
/*============================================================================
 * The OpenSecureChannelRequest structure.
 *===========================================================================*/
typedef struct _UA_OpenSecureChannelRequest
{
    UA_RequestHeader            RequestHeader;
    uint32_t                    ClientProtocolVersion;
    UA_SecurityTokenRequestType RequestType;
    UA_MessageSecurityMode      SecurityMode;
    UA_ByteString               ClientNonce;
    uint32_t                    RequestedLifetime;
}
UA_OpenSecureChannelRequest;

void UA_OpenSecureChannelRequest_Initialize(UA_OpenSecureChannelRequest* pValue);

void UA_OpenSecureChannelRequest_Clear(UA_OpenSecureChannelRequest* pValue);

//StatusCode UA_OpenSecureChannelRequest_GetSize(UA_OpenSecureChannelRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_OpenSecureChannelRequest_Encode(UA_MsgBuffer* msgBuf, UA_OpenSecureChannelRequest* pValue);

StatusCode UA_OpenSecureChannelRequest_Decode(UA_MsgBuffer* msgBuf, UA_OpenSecureChannelRequest* pValue);

extern struct UA_EncodeableType UA_OpenSecureChannelRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_OpenSecureChannelResponse
/*============================================================================
 * The OpenSecureChannelResponse structure.
 *===========================================================================*/
typedef struct _UA_OpenSecureChannelResponse
{
    UA_ResponseHeader       ResponseHeader;
    uint32_t                ServerProtocolVersion;
    UA_ChannelSecurityToken SecurityToken;
    UA_ByteString           ServerNonce;
}
UA_OpenSecureChannelResponse;

void UA_OpenSecureChannelResponse_Initialize(UA_OpenSecureChannelResponse* pValue);

void UA_OpenSecureChannelResponse_Clear(UA_OpenSecureChannelResponse* pValue);

//StatusCode UA_OpenSecureChannelResponse_GetSize(UA_OpenSecureChannelResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_OpenSecureChannelResponse_Encode(UA_MsgBuffer* msgBuf, UA_OpenSecureChannelResponse* pValue);

StatusCode UA_OpenSecureChannelResponse_Decode(UA_MsgBuffer* msgBuf, UA_OpenSecureChannelResponse* pValue);

extern struct UA_EncodeableType UA_OpenSecureChannelResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_CloseSecureChannel
#ifndef OPCUA_EXCLUDE_CloseSecureChannelRequest
/*============================================================================
 * The CloseSecureChannelRequest structure.
 *===========================================================================*/
typedef struct _UA_CloseSecureChannelRequest
{
    UA_RequestHeader RequestHeader;
}
UA_CloseSecureChannelRequest;

void UA_CloseSecureChannelRequest_Initialize(UA_CloseSecureChannelRequest* pValue);

void UA_CloseSecureChannelRequest_Clear(UA_CloseSecureChannelRequest* pValue);

//StatusCode UA_CloseSecureChannelRequest_GetSize(UA_CloseSecureChannelRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_CloseSecureChannelRequest_Encode(UA_MsgBuffer* msgBuf, UA_CloseSecureChannelRequest* pValue);

StatusCode UA_CloseSecureChannelRequest_Decode(UA_MsgBuffer* msgBuf, UA_CloseSecureChannelRequest* pValue);

extern struct UA_EncodeableType UA_CloseSecureChannelRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_CloseSecureChannelResponse
/*============================================================================
 * The CloseSecureChannelResponse structure.
 *===========================================================================*/
typedef struct _UA_CloseSecureChannelResponse
{
    UA_ResponseHeader ResponseHeader;
}
UA_CloseSecureChannelResponse;

void UA_CloseSecureChannelResponse_Initialize(UA_CloseSecureChannelResponse* pValue);

void UA_CloseSecureChannelResponse_Clear(UA_CloseSecureChannelResponse* pValue);

//StatusCode UA_CloseSecureChannelResponse_GetSize(UA_CloseSecureChannelResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_CloseSecureChannelResponse_Encode(UA_MsgBuffer* msgBuf, UA_CloseSecureChannelResponse* pValue);

StatusCode UA_CloseSecureChannelResponse_Decode(UA_MsgBuffer* msgBuf, UA_CloseSecureChannelResponse* pValue);

extern struct UA_EncodeableType UA_CloseSecureChannelResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_SignedSoftwareCertificate
/*============================================================================
 * The SignedSoftwareCertificate structure.
 *===========================================================================*/
typedef struct _UA_SignedSoftwareCertificate
{
    UA_ByteString CertificateData;
    UA_ByteString Signature;
}
UA_SignedSoftwareCertificate;

void UA_SignedSoftwareCertificate_Initialize(UA_SignedSoftwareCertificate* pValue);

void UA_SignedSoftwareCertificate_Clear(UA_SignedSoftwareCertificate* pValue);

//StatusCode UA_SignedSoftwareCertificate_GetSize(UA_SignedSoftwareCertificate* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_SignedSoftwareCertificate_Encode(UA_MsgBuffer* msgBuf, UA_SignedSoftwareCertificate* pValue);

StatusCode UA_SignedSoftwareCertificate_Decode(UA_MsgBuffer* msgBuf, UA_SignedSoftwareCertificate* pValue);

extern struct UA_EncodeableType UA_SignedSoftwareCertificate_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_SignatureData
/*============================================================================
 * The SignatureData structure.
 *===========================================================================*/
typedef struct _UA_SignatureData
{
    UA_String     Algorithm;
    UA_ByteString Signature;
}
UA_SignatureData;

void UA_SignatureData_Initialize(UA_SignatureData* pValue);

void UA_SignatureData_Clear(UA_SignatureData* pValue);

//StatusCode UA_SignatureData_GetSize(UA_SignatureData* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_SignatureData_Encode(UA_MsgBuffer* msgBuf, UA_SignatureData* pValue);

StatusCode UA_SignatureData_Decode(UA_MsgBuffer* msgBuf, UA_SignatureData* pValue);

extern struct UA_EncodeableType UA_SignatureData_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_CreateSession
#ifndef OPCUA_EXCLUDE_CreateSessionRequest
/*============================================================================
 * The CreateSessionRequest structure.
 *===========================================================================*/
typedef struct _UA_CreateSessionRequest
{
    UA_RequestHeader          RequestHeader;
    UA_ApplicationDescription ClientDescription;
    UA_String                 ServerUri;
    UA_String                 EndpointUrl;
    UA_String                 SessionName;
    UA_ByteString             ClientNonce;
    UA_ByteString             ClientCertificate;
    double                    RequestedSessionTimeout;
    uint32_t                  MaxResponseMessageSize;
}
UA_CreateSessionRequest;

void UA_CreateSessionRequest_Initialize(UA_CreateSessionRequest* pValue);

void UA_CreateSessionRequest_Clear(UA_CreateSessionRequest* pValue);

//StatusCode UA_CreateSessionRequest_GetSize(UA_CreateSessionRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_CreateSessionRequest_Encode(UA_MsgBuffer* msgBuf, UA_CreateSessionRequest* pValue);

StatusCode UA_CreateSessionRequest_Decode(UA_MsgBuffer* msgBuf, UA_CreateSessionRequest* pValue);

extern struct UA_EncodeableType UA_CreateSessionRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_CreateSessionResponse
/*============================================================================
 * The CreateSessionResponse structure.
 *===========================================================================*/
typedef struct _UA_CreateSessionResponse
{
    UA_ResponseHeader             ResponseHeader;
    UA_NodeId                     SessionId;
    UA_NodeId                     AuthenticationToken;
    double                        RevisedSessionTimeout;
    UA_ByteString                 ServerNonce;
    UA_ByteString                 ServerCertificate;
    int32_t                       NoOfServerEndpoints;
    UA_EndpointDescription*       ServerEndpoints;
    int32_t                       NoOfServerSoftwareCertificates;
    UA_SignedSoftwareCertificate* ServerSoftwareCertificates;
    UA_SignatureData              ServerSignature;
    uint32_t                      MaxRequestMessageSize;
}
UA_CreateSessionResponse;

void UA_CreateSessionResponse_Initialize(UA_CreateSessionResponse* pValue);

void UA_CreateSessionResponse_Clear(UA_CreateSessionResponse* pValue);

//StatusCode UA_CreateSessionResponse_GetSize(UA_CreateSessionResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_CreateSessionResponse_Encode(UA_MsgBuffer* msgBuf, UA_CreateSessionResponse* pValue);

StatusCode UA_CreateSessionResponse_Decode(UA_MsgBuffer* msgBuf, UA_CreateSessionResponse* pValue);

extern struct UA_EncodeableType UA_CreateSessionResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_UserIdentityToken
/*============================================================================
 * The UserIdentityToken structure.
 *===========================================================================*/
typedef struct _UA_UserIdentityToken
{
    UA_String PolicyId;
}
UA_UserIdentityToken;

void UA_UserIdentityToken_Initialize(UA_UserIdentityToken* pValue);

void UA_UserIdentityToken_Clear(UA_UserIdentityToken* pValue);

//StatusCode UA_UserIdentityToken_GetSize(UA_UserIdentityToken* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_UserIdentityToken_Encode(UA_MsgBuffer* msgBuf, UA_UserIdentityToken* pValue);

StatusCode UA_UserIdentityToken_Decode(UA_MsgBuffer* msgBuf, UA_UserIdentityToken* pValue);

extern struct UA_EncodeableType UA_UserIdentityToken_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_AnonymousIdentityToken
/*============================================================================
 * The AnonymousIdentityToken structure.
 *===========================================================================*/
typedef struct _UA_AnonymousIdentityToken
{
    UA_String PolicyId;
}
UA_AnonymousIdentityToken;

void UA_AnonymousIdentityToken_Initialize(UA_AnonymousIdentityToken* pValue);

void UA_AnonymousIdentityToken_Clear(UA_AnonymousIdentityToken* pValue);

//StatusCode UA_AnonymousIdentityToken_GetSize(UA_AnonymousIdentityToken* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_AnonymousIdentityToken_Encode(UA_MsgBuffer* msgBuf, UA_AnonymousIdentityToken* pValue);

StatusCode UA_AnonymousIdentityToken_Decode(UA_MsgBuffer* msgBuf, UA_AnonymousIdentityToken* pValue);

extern struct UA_EncodeableType UA_AnonymousIdentityToken_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_UserNameIdentityToken
/*============================================================================
 * The UserNameIdentityToken structure.
 *===========================================================================*/
typedef struct _UA_UserNameIdentityToken
{
    UA_String     PolicyId;
    UA_String     UserName;
    UA_ByteString Password;
    UA_String     EncryptionAlgorithm;
}
UA_UserNameIdentityToken;

void UA_UserNameIdentityToken_Initialize(UA_UserNameIdentityToken* pValue);

void UA_UserNameIdentityToken_Clear(UA_UserNameIdentityToken* pValue);

//StatusCode UA_UserNameIdentityToken_GetSize(UA_UserNameIdentityToken* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_UserNameIdentityToken_Encode(UA_MsgBuffer* msgBuf, UA_UserNameIdentityToken* pValue);

StatusCode UA_UserNameIdentityToken_Decode(UA_MsgBuffer* msgBuf, UA_UserNameIdentityToken* pValue);

extern struct UA_EncodeableType UA_UserNameIdentityToken_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_X509IdentityToken
/*============================================================================
 * The X509IdentityToken structure.
 *===========================================================================*/
typedef struct _UA_X509IdentityToken
{
    UA_String     PolicyId;
    UA_ByteString CertificateData;
}
UA_X509IdentityToken;

void UA_X509IdentityToken_Initialize(UA_X509IdentityToken* pValue);

void UA_X509IdentityToken_Clear(UA_X509IdentityToken* pValue);

//StatusCode UA_X509IdentityToken_GetSize(UA_X509IdentityToken* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_X509IdentityToken_Encode(UA_MsgBuffer* msgBuf, UA_X509IdentityToken* pValue);

StatusCode UA_X509IdentityToken_Decode(UA_MsgBuffer* msgBuf, UA_X509IdentityToken* pValue);

extern struct UA_EncodeableType UA_X509IdentityToken_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_KerberosIdentityToken
/*============================================================================
 * The KerberosIdentityToken structure.
 *===========================================================================*/
typedef struct _UA_KerberosIdentityToken
{
    UA_String     PolicyId;
    UA_ByteString TicketData;
}
UA_KerberosIdentityToken;

void UA_KerberosIdentityToken_Initialize(UA_KerberosIdentityToken* pValue);

void UA_KerberosIdentityToken_Clear(UA_KerberosIdentityToken* pValue);

//StatusCode UA_KerberosIdentityToken_GetSize(UA_KerberosIdentityToken* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_KerberosIdentityToken_Encode(UA_MsgBuffer* msgBuf, UA_KerberosIdentityToken* pValue);

StatusCode UA_KerberosIdentityToken_Decode(UA_MsgBuffer* msgBuf, UA_KerberosIdentityToken* pValue);

extern struct UA_EncodeableType UA_KerberosIdentityToken_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_IssuedIdentityToken
/*============================================================================
 * The IssuedIdentityToken structure.
 *===========================================================================*/
typedef struct _UA_IssuedIdentityToken
{
    UA_String     PolicyId;
    UA_ByteString TokenData;
    UA_String     EncryptionAlgorithm;
}
UA_IssuedIdentityToken;

void UA_IssuedIdentityToken_Initialize(UA_IssuedIdentityToken* pValue);

void UA_IssuedIdentityToken_Clear(UA_IssuedIdentityToken* pValue);

//StatusCode UA_IssuedIdentityToken_GetSize(UA_IssuedIdentityToken* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_IssuedIdentityToken_Encode(UA_MsgBuffer* msgBuf, UA_IssuedIdentityToken* pValue);

StatusCode UA_IssuedIdentityToken_Decode(UA_MsgBuffer* msgBuf, UA_IssuedIdentityToken* pValue);

extern struct UA_EncodeableType UA_IssuedIdentityToken_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ActivateSession
#ifndef OPCUA_EXCLUDE_ActivateSessionRequest
/*============================================================================
 * The ActivateSessionRequest structure.
 *===========================================================================*/
typedef struct _UA_ActivateSessionRequest
{
    UA_RequestHeader              RequestHeader;
    UA_SignatureData              ClientSignature;
    int32_t                       NoOfClientSoftwareCertificates;
    UA_SignedSoftwareCertificate* ClientSoftwareCertificates;
    int32_t                       NoOfLocaleIds;
    UA_String*                    LocaleIds;
    UA_ExtensionObject            UserIdentityToken;
    UA_SignatureData              UserTokenSignature;
}
UA_ActivateSessionRequest;

void UA_ActivateSessionRequest_Initialize(UA_ActivateSessionRequest* pValue);

void UA_ActivateSessionRequest_Clear(UA_ActivateSessionRequest* pValue);

//StatusCode UA_ActivateSessionRequest_GetSize(UA_ActivateSessionRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_ActivateSessionRequest_Encode(UA_MsgBuffer* msgBuf, UA_ActivateSessionRequest* pValue);

StatusCode UA_ActivateSessionRequest_Decode(UA_MsgBuffer* msgBuf, UA_ActivateSessionRequest* pValue);

extern struct UA_EncodeableType UA_ActivateSessionRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ActivateSessionResponse
/*============================================================================
 * The ActivateSessionResponse structure.
 *===========================================================================*/
typedef struct _UA_ActivateSessionResponse
{
    UA_ResponseHeader  ResponseHeader;
    UA_ByteString      ServerNonce;
    int32_t            NoOfResults;
    StatusCode*        Results;
    int32_t            NoOfDiagnosticInfos;
    UA_DiagnosticInfo* DiagnosticInfos;
}
UA_ActivateSessionResponse;

void UA_ActivateSessionResponse_Initialize(UA_ActivateSessionResponse* pValue);

void UA_ActivateSessionResponse_Clear(UA_ActivateSessionResponse* pValue);

//StatusCode UA_ActivateSessionResponse_GetSize(UA_ActivateSessionResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_ActivateSessionResponse_Encode(UA_MsgBuffer* msgBuf, UA_ActivateSessionResponse* pValue);

StatusCode UA_ActivateSessionResponse_Decode(UA_MsgBuffer* msgBuf, UA_ActivateSessionResponse* pValue);

extern struct UA_EncodeableType UA_ActivateSessionResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_CloseSession
#ifndef OPCUA_EXCLUDE_CloseSessionRequest
/*============================================================================
 * The CloseSessionRequest structure.
 *===========================================================================*/
typedef struct _UA_CloseSessionRequest
{
    UA_RequestHeader RequestHeader;
    UA_Boolean       DeleteSubscriptions;
}
UA_CloseSessionRequest;

void UA_CloseSessionRequest_Initialize(UA_CloseSessionRequest* pValue);

void UA_CloseSessionRequest_Clear(UA_CloseSessionRequest* pValue);

//StatusCode UA_CloseSessionRequest_GetSize(UA_CloseSessionRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_CloseSessionRequest_Encode(UA_MsgBuffer* msgBuf, UA_CloseSessionRequest* pValue);

StatusCode UA_CloseSessionRequest_Decode(UA_MsgBuffer* msgBuf, UA_CloseSessionRequest* pValue);

extern struct UA_EncodeableType UA_CloseSessionRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_CloseSessionResponse
/*============================================================================
 * The CloseSessionResponse structure.
 *===========================================================================*/
typedef struct _UA_CloseSessionResponse
{
    UA_ResponseHeader ResponseHeader;
}
UA_CloseSessionResponse;

void UA_CloseSessionResponse_Initialize(UA_CloseSessionResponse* pValue);

void UA_CloseSessionResponse_Clear(UA_CloseSessionResponse* pValue);

//StatusCode UA_CloseSessionResponse_GetSize(UA_CloseSessionResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_CloseSessionResponse_Encode(UA_MsgBuffer* msgBuf, UA_CloseSessionResponse* pValue);

StatusCode UA_CloseSessionResponse_Decode(UA_MsgBuffer* msgBuf, UA_CloseSessionResponse* pValue);

extern struct UA_EncodeableType UA_CloseSessionResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_Cancel
#ifndef OPCUA_EXCLUDE_CancelRequest
/*============================================================================
 * The CancelRequest structure.
 *===========================================================================*/
typedef struct _UA_CancelRequest
{
    UA_RequestHeader RequestHeader;
    uint32_t         RequestHandle;
}
UA_CancelRequest;

void UA_CancelRequest_Initialize(UA_CancelRequest* pValue);

void UA_CancelRequest_Clear(UA_CancelRequest* pValue);

//StatusCode UA_CancelRequest_GetSize(UA_CancelRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_CancelRequest_Encode(UA_MsgBuffer* msgBuf, UA_CancelRequest* pValue);

StatusCode UA_CancelRequest_Decode(UA_MsgBuffer* msgBuf, UA_CancelRequest* pValue);

extern struct UA_EncodeableType UA_CancelRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_CancelResponse
/*============================================================================
 * The CancelResponse structure.
 *===========================================================================*/
typedef struct _UA_CancelResponse
{
    UA_ResponseHeader ResponseHeader;
    uint32_t          CancelCount;
}
UA_CancelResponse;

void UA_CancelResponse_Initialize(UA_CancelResponse* pValue);

void UA_CancelResponse_Clear(UA_CancelResponse* pValue);

//StatusCode UA_CancelResponse_GetSize(UA_CancelResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_CancelResponse_Encode(UA_MsgBuffer* msgBuf, UA_CancelResponse* pValue);

StatusCode UA_CancelResponse_Decode(UA_MsgBuffer* msgBuf, UA_CancelResponse* pValue);

extern struct UA_EncodeableType UA_CancelResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_NodeAttributesMask
/*============================================================================
 * The NodeAttributesMask enumeration.
 *===========================================================================*/
typedef enum _UA_NodeAttributesMask
{
    UA_NodeAttributesMask_None                    = 0,
    UA_NodeAttributesMask_AccessLevel             = 1,
    UA_NodeAttributesMask_ArrayDimensions         = 2,
    UA_NodeAttributesMask_BrowseName              = 4,
    UA_NodeAttributesMask_ContainsNoLoops         = 8,
    UA_NodeAttributesMask_DataType                = 16,
    UA_NodeAttributesMask_Description             = 32,
    UA_NodeAttributesMask_DisplayName             = 64,
    UA_NodeAttributesMask_EventNotifier           = 128,
    UA_NodeAttributesMask_Executable              = 256,
    UA_NodeAttributesMask_Historizing             = 512,
    UA_NodeAttributesMask_InverseName             = 1024,
    UA_NodeAttributesMask_IsAbstract              = 2048,
    UA_NodeAttributesMask_MinimumSamplingInterval = 4096,
    UA_NodeAttributesMask_NodeClass               = 8192,
    UA_NodeAttributesMask_NodeId                  = 16384,
    UA_NodeAttributesMask_Symmetric               = 32768,
    UA_NodeAttributesMask_UserAccessLevel         = 65536,
    UA_NodeAttributesMask_UserExecutable          = 131072,
    UA_NodeAttributesMask_UserWriteMask           = 262144,
    UA_NodeAttributesMask_ValueRank               = 524288,
    UA_NodeAttributesMask_WriteMask               = 1048576,
    UA_NodeAttributesMask_Value                   = 2097152,
    UA_NodeAttributesMask_All                     = 4194303,
    UA_NodeAttributesMask_BaseNode                = 1335396,
    UA_NodeAttributesMask_Object                  = 1335524,
    UA_NodeAttributesMask_ObjectTypeOrDataType    = 1337444,
    UA_NodeAttributesMask_Variable                = 4026999,
    UA_NodeAttributesMask_VariableType            = 3958902,
    UA_NodeAttributesMask_Method                  = 1466724,
    UA_NodeAttributesMask_ReferenceType           = 1371236,
    UA_NodeAttributesMask_View                    = 1335532
#if OPCUA_FORCE_INT32_ENUMS
    ,_UA_NodeAttributesMask_MaxEnumerationValue = OpcUa_Int32_Max
#endif
}
UA_NodeAttributesMask;

#endif

#ifndef OPCUA_EXCLUDE_NodeAttributes
/*============================================================================
 * The NodeAttributes structure.
 *===========================================================================*/
typedef struct _UA_NodeAttributes
{
    uint32_t         SpecifiedAttributes;
    UA_LocalizedText DisplayName;
    UA_LocalizedText Description;
    uint32_t         WriteMask;
    uint32_t         UserWriteMask;
}
UA_NodeAttributes;

void UA_NodeAttributes_Initialize(UA_NodeAttributes* pValue);

void UA_NodeAttributes_Clear(UA_NodeAttributes* pValue);

//StatusCode UA_NodeAttributes_GetSize(UA_NodeAttributes* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_NodeAttributes_Encode(UA_MsgBuffer* msgBuf, UA_NodeAttributes* pValue);

StatusCode UA_NodeAttributes_Decode(UA_MsgBuffer* msgBuf, UA_NodeAttributes* pValue);

extern struct UA_EncodeableType UA_NodeAttributes_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ObjectAttributes
/*============================================================================
 * The ObjectAttributes structure.
 *===========================================================================*/
typedef struct _UA_ObjectAttributes
{
    uint32_t         SpecifiedAttributes;
    UA_LocalizedText DisplayName;
    UA_LocalizedText Description;
    uint32_t         WriteMask;
    uint32_t         UserWriteMask;
    UA_Byte          EventNotifier;
}
UA_ObjectAttributes;

void UA_ObjectAttributes_Initialize(UA_ObjectAttributes* pValue);

void UA_ObjectAttributes_Clear(UA_ObjectAttributes* pValue);

//StatusCode UA_ObjectAttributes_GetSize(UA_ObjectAttributes* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_ObjectAttributes_Encode(UA_MsgBuffer* msgBuf, UA_ObjectAttributes* pValue);

StatusCode UA_ObjectAttributes_Decode(UA_MsgBuffer* msgBuf, UA_ObjectAttributes* pValue);

extern struct UA_EncodeableType UA_ObjectAttributes_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_VariableAttributes
/*============================================================================
 * The VariableAttributes structure.
 *===========================================================================*/
typedef struct _UA_VariableAttributes
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
UA_VariableAttributes;

void UA_VariableAttributes_Initialize(UA_VariableAttributes* pValue);

void UA_VariableAttributes_Clear(UA_VariableAttributes* pValue);

//StatusCode UA_VariableAttributes_GetSize(UA_VariableAttributes* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_VariableAttributes_Encode(UA_MsgBuffer* msgBuf, UA_VariableAttributes* pValue);

StatusCode UA_VariableAttributes_Decode(UA_MsgBuffer* msgBuf, UA_VariableAttributes* pValue);

extern struct UA_EncodeableType UA_VariableAttributes_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_MethodAttributes
/*============================================================================
 * The MethodAttributes structure.
 *===========================================================================*/
typedef struct _UA_MethodAttributes
{
    uint32_t         SpecifiedAttributes;
    UA_LocalizedText DisplayName;
    UA_LocalizedText Description;
    uint32_t         WriteMask;
    uint32_t         UserWriteMask;
    UA_Boolean       Executable;
    UA_Boolean       UserExecutable;
}
UA_MethodAttributes;

void UA_MethodAttributes_Initialize(UA_MethodAttributes* pValue);

void UA_MethodAttributes_Clear(UA_MethodAttributes* pValue);

//StatusCode UA_MethodAttributes_GetSize(UA_MethodAttributes* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_MethodAttributes_Encode(UA_MsgBuffer* msgBuf, UA_MethodAttributes* pValue);

StatusCode UA_MethodAttributes_Decode(UA_MsgBuffer* msgBuf, UA_MethodAttributes* pValue);

extern struct UA_EncodeableType UA_MethodAttributes_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ObjectTypeAttributes
/*============================================================================
 * The ObjectTypeAttributes structure.
 *===========================================================================*/
typedef struct _UA_ObjectTypeAttributes
{
    uint32_t         SpecifiedAttributes;
    UA_LocalizedText DisplayName;
    UA_LocalizedText Description;
    uint32_t         WriteMask;
    uint32_t         UserWriteMask;
    UA_Boolean       IsAbstract;
}
UA_ObjectTypeAttributes;

void UA_ObjectTypeAttributes_Initialize(UA_ObjectTypeAttributes* pValue);

void UA_ObjectTypeAttributes_Clear(UA_ObjectTypeAttributes* pValue);

//StatusCode UA_ObjectTypeAttributes_GetSize(UA_ObjectTypeAttributes* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_ObjectTypeAttributes_Encode(UA_MsgBuffer* msgBuf, UA_ObjectTypeAttributes* pValue);

StatusCode UA_ObjectTypeAttributes_Decode(UA_MsgBuffer* msgBuf, UA_ObjectTypeAttributes* pValue);

extern struct UA_EncodeableType UA_ObjectTypeAttributes_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_VariableTypeAttributes
/*============================================================================
 * The VariableTypeAttributes structure.
 *===========================================================================*/
typedef struct _UA_VariableTypeAttributes
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
UA_VariableTypeAttributes;

void UA_VariableTypeAttributes_Initialize(UA_VariableTypeAttributes* pValue);

void UA_VariableTypeAttributes_Clear(UA_VariableTypeAttributes* pValue);

//StatusCode UA_VariableTypeAttributes_GetSize(UA_VariableTypeAttributes* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_VariableTypeAttributes_Encode(UA_MsgBuffer* msgBuf, UA_VariableTypeAttributes* pValue);

StatusCode UA_VariableTypeAttributes_Decode(UA_MsgBuffer* msgBuf, UA_VariableTypeAttributes* pValue);

extern struct UA_EncodeableType UA_VariableTypeAttributes_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ReferenceTypeAttributes
/*============================================================================
 * The ReferenceTypeAttributes structure.
 *===========================================================================*/
typedef struct _UA_ReferenceTypeAttributes
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
UA_ReferenceTypeAttributes;

void UA_ReferenceTypeAttributes_Initialize(UA_ReferenceTypeAttributes* pValue);

void UA_ReferenceTypeAttributes_Clear(UA_ReferenceTypeAttributes* pValue);

//StatusCode UA_ReferenceTypeAttributes_GetSize(UA_ReferenceTypeAttributes* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_ReferenceTypeAttributes_Encode(UA_MsgBuffer* msgBuf, UA_ReferenceTypeAttributes* pValue);

StatusCode UA_ReferenceTypeAttributes_Decode(UA_MsgBuffer* msgBuf, UA_ReferenceTypeAttributes* pValue);

extern struct UA_EncodeableType UA_ReferenceTypeAttributes_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_DataTypeAttributes
/*============================================================================
 * The DataTypeAttributes structure.
 *===========================================================================*/
typedef struct _UA_DataTypeAttributes
{
    uint32_t         SpecifiedAttributes;
    UA_LocalizedText DisplayName;
    UA_LocalizedText Description;
    uint32_t         WriteMask;
    uint32_t         UserWriteMask;
    UA_Boolean       IsAbstract;
}
UA_DataTypeAttributes;

void UA_DataTypeAttributes_Initialize(UA_DataTypeAttributes* pValue);

void UA_DataTypeAttributes_Clear(UA_DataTypeAttributes* pValue);

//StatusCode UA_DataTypeAttributes_GetSize(UA_DataTypeAttributes* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_DataTypeAttributes_Encode(UA_MsgBuffer* msgBuf, UA_DataTypeAttributes* pValue);

StatusCode UA_DataTypeAttributes_Decode(UA_MsgBuffer* msgBuf, UA_DataTypeAttributes* pValue);

extern struct UA_EncodeableType UA_DataTypeAttributes_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ViewAttributes
/*============================================================================
 * The ViewAttributes structure.
 *===========================================================================*/
typedef struct _UA_ViewAttributes
{
    uint32_t         SpecifiedAttributes;
    UA_LocalizedText DisplayName;
    UA_LocalizedText Description;
    uint32_t         WriteMask;
    uint32_t         UserWriteMask;
    UA_Boolean       ContainsNoLoops;
    UA_Byte          EventNotifier;
}
UA_ViewAttributes;

void UA_ViewAttributes_Initialize(UA_ViewAttributes* pValue);

void UA_ViewAttributes_Clear(UA_ViewAttributes* pValue);

//StatusCode UA_ViewAttributes_GetSize(UA_ViewAttributes* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_ViewAttributes_Encode(UA_MsgBuffer* msgBuf, UA_ViewAttributes* pValue);

StatusCode UA_ViewAttributes_Decode(UA_MsgBuffer* msgBuf, UA_ViewAttributes* pValue);

extern struct UA_EncodeableType UA_ViewAttributes_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_AddNodesItem
/*============================================================================
 * The AddNodesItem structure.
 *===========================================================================*/
typedef struct _UA_AddNodesItem
{
    UA_ExpandedNodeId  ParentNodeId;
    UA_NodeId          ReferenceTypeId;
    UA_ExpandedNodeId  RequestedNewNodeId;
    UA_QualifiedName   BrowseName;
    UA_NodeClass       NodeClass;
    UA_ExtensionObject NodeAttributes;
    UA_ExpandedNodeId  TypeDefinition;
}
UA_AddNodesItem;

void UA_AddNodesItem_Initialize(UA_AddNodesItem* pValue);

void UA_AddNodesItem_Clear(UA_AddNodesItem* pValue);

//StatusCode UA_AddNodesItem_GetSize(UA_AddNodesItem* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_AddNodesItem_Encode(UA_MsgBuffer* msgBuf, UA_AddNodesItem* pValue);

StatusCode UA_AddNodesItem_Decode(UA_MsgBuffer* msgBuf, UA_AddNodesItem* pValue);

extern struct UA_EncodeableType UA_AddNodesItem_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_AddNodesResult
/*============================================================================
 * The AddNodesResult structure.
 *===========================================================================*/
typedef struct _UA_AddNodesResult
{
    StatusCode StatusCode;
    UA_NodeId  AddedNodeId;
}
UA_AddNodesResult;

void UA_AddNodesResult_Initialize(UA_AddNodesResult* pValue);

void UA_AddNodesResult_Clear(UA_AddNodesResult* pValue);

//StatusCode UA_AddNodesResult_GetSize(UA_AddNodesResult* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_AddNodesResult_Encode(UA_MsgBuffer* msgBuf, UA_AddNodesResult* pValue);

StatusCode UA_AddNodesResult_Decode(UA_MsgBuffer* msgBuf, UA_AddNodesResult* pValue);

extern struct UA_EncodeableType UA_AddNodesResult_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_AddNodes
#ifndef OPCUA_EXCLUDE_AddNodesRequest
/*============================================================================
 * The AddNodesRequest structure.
 *===========================================================================*/
typedef struct _UA_AddNodesRequest
{
    UA_RequestHeader RequestHeader;
    int32_t          NoOfNodesToAdd;
    UA_AddNodesItem* NodesToAdd;
}
UA_AddNodesRequest;

void UA_AddNodesRequest_Initialize(UA_AddNodesRequest* pValue);

void UA_AddNodesRequest_Clear(UA_AddNodesRequest* pValue);

//StatusCode UA_AddNodesRequest_GetSize(UA_AddNodesRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_AddNodesRequest_Encode(UA_MsgBuffer* msgBuf, UA_AddNodesRequest* pValue);

StatusCode UA_AddNodesRequest_Decode(UA_MsgBuffer* msgBuf, UA_AddNodesRequest* pValue);

extern struct UA_EncodeableType UA_AddNodesRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_AddNodesResponse
/*============================================================================
 * The AddNodesResponse structure.
 *===========================================================================*/
typedef struct _UA_AddNodesResponse
{
    UA_ResponseHeader  ResponseHeader;
    int32_t            NoOfResults;
    UA_AddNodesResult* Results;
    int32_t            NoOfDiagnosticInfos;
    UA_DiagnosticInfo* DiagnosticInfos;
}
UA_AddNodesResponse;

void UA_AddNodesResponse_Initialize(UA_AddNodesResponse* pValue);

void UA_AddNodesResponse_Clear(UA_AddNodesResponse* pValue);

//StatusCode UA_AddNodesResponse_GetSize(UA_AddNodesResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_AddNodesResponse_Encode(UA_MsgBuffer* msgBuf, UA_AddNodesResponse* pValue);

StatusCode UA_AddNodesResponse_Decode(UA_MsgBuffer* msgBuf, UA_AddNodesResponse* pValue);

extern struct UA_EncodeableType UA_AddNodesResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_AddReferencesItem
/*============================================================================
 * The AddReferencesItem structure.
 *===========================================================================*/
typedef struct _UA_AddReferencesItem
{
    UA_NodeId         SourceNodeId;
    UA_NodeId         ReferenceTypeId;
    UA_Boolean        IsForward;
    UA_String         TargetServerUri;
    UA_ExpandedNodeId TargetNodeId;
    UA_NodeClass      TargetNodeClass;
}
UA_AddReferencesItem;

void UA_AddReferencesItem_Initialize(UA_AddReferencesItem* pValue);

void UA_AddReferencesItem_Clear(UA_AddReferencesItem* pValue);

//StatusCode UA_AddReferencesItem_GetSize(UA_AddReferencesItem* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_AddReferencesItem_Encode(UA_MsgBuffer* msgBuf, UA_AddReferencesItem* pValue);

StatusCode UA_AddReferencesItem_Decode(UA_MsgBuffer* msgBuf, UA_AddReferencesItem* pValue);

extern struct UA_EncodeableType UA_AddReferencesItem_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_AddReferences
#ifndef OPCUA_EXCLUDE_AddReferencesRequest
/*============================================================================
 * The AddReferencesRequest structure.
 *===========================================================================*/
typedef struct _UA_AddReferencesRequest
{
    UA_RequestHeader      RequestHeader;
    int32_t               NoOfReferencesToAdd;
    UA_AddReferencesItem* ReferencesToAdd;
}
UA_AddReferencesRequest;

void UA_AddReferencesRequest_Initialize(UA_AddReferencesRequest* pValue);

void UA_AddReferencesRequest_Clear(UA_AddReferencesRequest* pValue);

//StatusCode UA_AddReferencesRequest_GetSize(UA_AddReferencesRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_AddReferencesRequest_Encode(UA_MsgBuffer* msgBuf, UA_AddReferencesRequest* pValue);

StatusCode UA_AddReferencesRequest_Decode(UA_MsgBuffer* msgBuf, UA_AddReferencesRequest* pValue);

extern struct UA_EncodeableType UA_AddReferencesRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_AddReferencesResponse
/*============================================================================
 * The AddReferencesResponse structure.
 *===========================================================================*/
typedef struct _UA_AddReferencesResponse
{
    UA_ResponseHeader  ResponseHeader;
    int32_t            NoOfResults;
    StatusCode*        Results;
    int32_t            NoOfDiagnosticInfos;
    UA_DiagnosticInfo* DiagnosticInfos;
}
UA_AddReferencesResponse;

void UA_AddReferencesResponse_Initialize(UA_AddReferencesResponse* pValue);

void UA_AddReferencesResponse_Clear(UA_AddReferencesResponse* pValue);

//StatusCode UA_AddReferencesResponse_GetSize(UA_AddReferencesResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_AddReferencesResponse_Encode(UA_MsgBuffer* msgBuf, UA_AddReferencesResponse* pValue);

StatusCode UA_AddReferencesResponse_Decode(UA_MsgBuffer* msgBuf, UA_AddReferencesResponse* pValue);

extern struct UA_EncodeableType UA_AddReferencesResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_DeleteNodesItem
/*============================================================================
 * The DeleteNodesItem structure.
 *===========================================================================*/
typedef struct _UA_DeleteNodesItem
{
    UA_NodeId  NodeId;
    UA_Boolean DeleteTargetReferences;
}
UA_DeleteNodesItem;

void UA_DeleteNodesItem_Initialize(UA_DeleteNodesItem* pValue);

void UA_DeleteNodesItem_Clear(UA_DeleteNodesItem* pValue);

//StatusCode UA_DeleteNodesItem_GetSize(UA_DeleteNodesItem* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_DeleteNodesItem_Encode(UA_MsgBuffer* msgBuf, UA_DeleteNodesItem* pValue);

StatusCode UA_DeleteNodesItem_Decode(UA_MsgBuffer* msgBuf, UA_DeleteNodesItem* pValue);

extern struct UA_EncodeableType UA_DeleteNodesItem_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_DeleteNodes
#ifndef OPCUA_EXCLUDE_DeleteNodesRequest
/*============================================================================
 * The DeleteNodesRequest structure.
 *===========================================================================*/
typedef struct _UA_DeleteNodesRequest
{
    UA_RequestHeader    RequestHeader;
    int32_t             NoOfNodesToDelete;
    UA_DeleteNodesItem* NodesToDelete;
}
UA_DeleteNodesRequest;

void UA_DeleteNodesRequest_Initialize(UA_DeleteNodesRequest* pValue);

void UA_DeleteNodesRequest_Clear(UA_DeleteNodesRequest* pValue);

//StatusCode UA_DeleteNodesRequest_GetSize(UA_DeleteNodesRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_DeleteNodesRequest_Encode(UA_MsgBuffer* msgBuf, UA_DeleteNodesRequest* pValue);

StatusCode UA_DeleteNodesRequest_Decode(UA_MsgBuffer* msgBuf, UA_DeleteNodesRequest* pValue);

extern struct UA_EncodeableType UA_DeleteNodesRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_DeleteNodesResponse
/*============================================================================
 * The DeleteNodesResponse structure.
 *===========================================================================*/
typedef struct _UA_DeleteNodesResponse
{
    UA_ResponseHeader  ResponseHeader;
    int32_t            NoOfResults;
    StatusCode*        Results;
    int32_t            NoOfDiagnosticInfos;
    UA_DiagnosticInfo* DiagnosticInfos;
}
UA_DeleteNodesResponse;

void UA_DeleteNodesResponse_Initialize(UA_DeleteNodesResponse* pValue);

void UA_DeleteNodesResponse_Clear(UA_DeleteNodesResponse* pValue);

//StatusCode UA_DeleteNodesResponse_GetSize(UA_DeleteNodesResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_DeleteNodesResponse_Encode(UA_MsgBuffer* msgBuf, UA_DeleteNodesResponse* pValue);

StatusCode UA_DeleteNodesResponse_Decode(UA_MsgBuffer* msgBuf, UA_DeleteNodesResponse* pValue);

extern struct UA_EncodeableType UA_DeleteNodesResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_DeleteReferencesItem
/*============================================================================
 * The DeleteReferencesItem structure.
 *===========================================================================*/
typedef struct _UA_DeleteReferencesItem
{
    UA_NodeId         SourceNodeId;
    UA_NodeId         ReferenceTypeId;
    UA_Boolean        IsForward;
    UA_ExpandedNodeId TargetNodeId;
    UA_Boolean        DeleteBidirectional;
}
UA_DeleteReferencesItem;

void UA_DeleteReferencesItem_Initialize(UA_DeleteReferencesItem* pValue);

void UA_DeleteReferencesItem_Clear(UA_DeleteReferencesItem* pValue);

//StatusCode UA_DeleteReferencesItem_GetSize(UA_DeleteReferencesItem* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_DeleteReferencesItem_Encode(UA_MsgBuffer* msgBuf, UA_DeleteReferencesItem* pValue);

StatusCode UA_DeleteReferencesItem_Decode(UA_MsgBuffer* msgBuf, UA_DeleteReferencesItem* pValue);

extern struct UA_EncodeableType UA_DeleteReferencesItem_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_DeleteReferences
#ifndef OPCUA_EXCLUDE_DeleteReferencesRequest
/*============================================================================
 * The DeleteReferencesRequest structure.
 *===========================================================================*/
typedef struct _UA_DeleteReferencesRequest
{
    UA_RequestHeader         RequestHeader;
    int32_t                  NoOfReferencesToDelete;
    UA_DeleteReferencesItem* ReferencesToDelete;
}
UA_DeleteReferencesRequest;

void UA_DeleteReferencesRequest_Initialize(UA_DeleteReferencesRequest* pValue);

void UA_DeleteReferencesRequest_Clear(UA_DeleteReferencesRequest* pValue);

//StatusCode UA_DeleteReferencesRequest_GetSize(UA_DeleteReferencesRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_DeleteReferencesRequest_Encode(UA_MsgBuffer* msgBuf, UA_DeleteReferencesRequest* pValue);

StatusCode UA_DeleteReferencesRequest_Decode(UA_MsgBuffer* msgBuf, UA_DeleteReferencesRequest* pValue);

extern struct UA_EncodeableType UA_DeleteReferencesRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_DeleteReferencesResponse
/*============================================================================
 * The DeleteReferencesResponse structure.
 *===========================================================================*/
typedef struct _UA_DeleteReferencesResponse
{
    UA_ResponseHeader  ResponseHeader;
    int32_t            NoOfResults;
    StatusCode*        Results;
    int32_t            NoOfDiagnosticInfos;
    UA_DiagnosticInfo* DiagnosticInfos;
}
UA_DeleteReferencesResponse;

void UA_DeleteReferencesResponse_Initialize(UA_DeleteReferencesResponse* pValue);

void UA_DeleteReferencesResponse_Clear(UA_DeleteReferencesResponse* pValue);

//StatusCode UA_DeleteReferencesResponse_GetSize(UA_DeleteReferencesResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_DeleteReferencesResponse_Encode(UA_MsgBuffer* msgBuf, UA_DeleteReferencesResponse* pValue);

StatusCode UA_DeleteReferencesResponse_Decode(UA_MsgBuffer* msgBuf, UA_DeleteReferencesResponse* pValue);

extern struct UA_EncodeableType UA_DeleteReferencesResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_AttributeWriteMask
/*============================================================================
 * The AttributeWriteMask enumeration.
 *===========================================================================*/
typedef enum _UA_AttributeWriteMask
{
    UA_AttributeWriteMask_None                    = 0,
    UA_AttributeWriteMask_AccessLevel             = 1,
    UA_AttributeWriteMask_ArrayDimensions         = 2,
    UA_AttributeWriteMask_BrowseName              = 4,
    UA_AttributeWriteMask_ContainsNoLoops         = 8,
    UA_AttributeWriteMask_DataType                = 16,
    UA_AttributeWriteMask_Description             = 32,
    UA_AttributeWriteMask_DisplayName             = 64,
    UA_AttributeWriteMask_EventNotifier           = 128,
    UA_AttributeWriteMask_Executable              = 256,
    UA_AttributeWriteMask_Historizing             = 512,
    UA_AttributeWriteMask_InverseName             = 1024,
    UA_AttributeWriteMask_IsAbstract              = 2048,
    UA_AttributeWriteMask_MinimumSamplingInterval = 4096,
    UA_AttributeWriteMask_NodeClass               = 8192,
    UA_AttributeWriteMask_NodeId                  = 16384,
    UA_AttributeWriteMask_Symmetric               = 32768,
    UA_AttributeWriteMask_UserAccessLevel         = 65536,
    UA_AttributeWriteMask_UserExecutable          = 131072,
    UA_AttributeWriteMask_UserWriteMask           = 262144,
    UA_AttributeWriteMask_ValueRank               = 524288,
    UA_AttributeWriteMask_WriteMask               = 1048576,
    UA_AttributeWriteMask_ValueForVariableType    = 2097152
#if OPCUA_FORCE_INT32_ENUMS
    ,_UA_AttributeWriteMask_MaxEnumerationValue = OpcUa_Int32_Max
#endif
}
UA_AttributeWriteMask;

#endif

#ifndef OPCUA_EXCLUDE_BrowseDirection
/*============================================================================
 * The BrowseDirection enumeration.
 *===========================================================================*/
typedef enum _UA_BrowseDirection
{
    UA_BrowseDirection_Forward = 0,
    UA_BrowseDirection_Inverse = 1,
    UA_BrowseDirection_Both    = 2
#if OPCUA_FORCE_INT32_ENUMS
    ,_UA_BrowseDirection_MaxEnumerationValue = OpcUa_Int32_Max
#endif
}
UA_BrowseDirection;

#endif

#ifndef OPCUA_EXCLUDE_ViewDescription
/*============================================================================
 * The ViewDescription structure.
 *===========================================================================*/
typedef struct _UA_ViewDescription
{
    UA_NodeId   ViewId;
    UA_DateTime Timestamp;
    uint32_t    ViewVersion;
}
UA_ViewDescription;

void UA_ViewDescription_Initialize(UA_ViewDescription* pValue);

void UA_ViewDescription_Clear(UA_ViewDescription* pValue);

//StatusCode UA_ViewDescription_GetSize(UA_ViewDescription* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_ViewDescription_Encode(UA_MsgBuffer* msgBuf, UA_ViewDescription* pValue);

StatusCode UA_ViewDescription_Decode(UA_MsgBuffer* msgBuf, UA_ViewDescription* pValue);

extern struct UA_EncodeableType UA_ViewDescription_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_BrowseDescription
/*============================================================================
 * The BrowseDescription structure.
 *===========================================================================*/
typedef struct _UA_BrowseDescription
{
    UA_NodeId          NodeId;
    UA_BrowseDirection BrowseDirection;
    UA_NodeId          ReferenceTypeId;
    UA_Boolean         IncludeSubtypes;
    uint32_t           NodeClassMask;
    uint32_t           ResultMask;
}
UA_BrowseDescription;

void UA_BrowseDescription_Initialize(UA_BrowseDescription* pValue);

void UA_BrowseDescription_Clear(UA_BrowseDescription* pValue);

//StatusCode UA_BrowseDescription_GetSize(UA_BrowseDescription* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_BrowseDescription_Encode(UA_MsgBuffer* msgBuf, UA_BrowseDescription* pValue);

StatusCode UA_BrowseDescription_Decode(UA_MsgBuffer* msgBuf, UA_BrowseDescription* pValue);

extern struct UA_EncodeableType UA_BrowseDescription_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_BrowseResultMask
/*============================================================================
 * The BrowseResultMask enumeration.
 *===========================================================================*/
typedef enum _UA_BrowseResultMask
{
    UA_BrowseResultMask_None              = 0,
    UA_BrowseResultMask_ReferenceTypeId   = 1,
    UA_BrowseResultMask_IsForward         = 2,
    UA_BrowseResultMask_NodeClass         = 4,
    UA_BrowseResultMask_BrowseName        = 8,
    UA_BrowseResultMask_DisplayName       = 16,
    UA_BrowseResultMask_TypeDefinition    = 32,
    UA_BrowseResultMask_All               = 63,
    UA_BrowseResultMask_ReferenceTypeInfo = 3,
    UA_BrowseResultMask_TargetInfo        = 60
#if OPCUA_FORCE_INT32_ENUMS
    ,_UA_BrowseResultMask_MaxEnumerationValue = OpcUa_Int32_Max
#endif
}
UA_BrowseResultMask;

#endif

#ifndef OPCUA_EXCLUDE_ReferenceDescription
/*============================================================================
 * The ReferenceDescription structure.
 *===========================================================================*/
typedef struct _UA_ReferenceDescription
{
    UA_NodeId         ReferenceTypeId;
    UA_Boolean        IsForward;
    UA_ExpandedNodeId NodeId;
    UA_QualifiedName  BrowseName;
    UA_LocalizedText  DisplayName;
    UA_NodeClass      NodeClass;
    UA_ExpandedNodeId TypeDefinition;
}
UA_ReferenceDescription;

void UA_ReferenceDescription_Initialize(UA_ReferenceDescription* pValue);

void UA_ReferenceDescription_Clear(UA_ReferenceDescription* pValue);

//StatusCode UA_ReferenceDescription_GetSize(UA_ReferenceDescription* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_ReferenceDescription_Encode(UA_MsgBuffer* msgBuf, UA_ReferenceDescription* pValue);

StatusCode UA_ReferenceDescription_Decode(UA_MsgBuffer* msgBuf, UA_ReferenceDescription* pValue);

extern struct UA_EncodeableType UA_ReferenceDescription_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_BrowseResult
/*============================================================================
 * The BrowseResult structure.
 *===========================================================================*/
typedef struct _UA_BrowseResult
{
    StatusCode               StatusCode;
    UA_ByteString            ContinuationPoint;
    int32_t                  NoOfReferences;
    UA_ReferenceDescription* References;
}
UA_BrowseResult;

void UA_BrowseResult_Initialize(UA_BrowseResult* pValue);

void UA_BrowseResult_Clear(UA_BrowseResult* pValue);

//StatusCode UA_BrowseResult_GetSize(UA_BrowseResult* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_BrowseResult_Encode(UA_MsgBuffer* msgBuf, UA_BrowseResult* pValue);

StatusCode UA_BrowseResult_Decode(UA_MsgBuffer* msgBuf, UA_BrowseResult* pValue);

extern struct UA_EncodeableType UA_BrowseResult_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_Browse
#ifndef OPCUA_EXCLUDE_BrowseRequest
/*============================================================================
 * The BrowseRequest structure.
 *===========================================================================*/
typedef struct _UA_BrowseRequest
{
    UA_RequestHeader      RequestHeader;
    UA_ViewDescription    View;
    uint32_t              RequestedMaxReferencesPerNode;
    int32_t               NoOfNodesToBrowse;
    UA_BrowseDescription* NodesToBrowse;
}
UA_BrowseRequest;

void UA_BrowseRequest_Initialize(UA_BrowseRequest* pValue);

void UA_BrowseRequest_Clear(UA_BrowseRequest* pValue);

//StatusCode UA_BrowseRequest_GetSize(UA_BrowseRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_BrowseRequest_Encode(UA_MsgBuffer* msgBuf, UA_BrowseRequest* pValue);

StatusCode UA_BrowseRequest_Decode(UA_MsgBuffer* msgBuf, UA_BrowseRequest* pValue);

extern struct UA_EncodeableType UA_BrowseRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_BrowseResponse
/*============================================================================
 * The BrowseResponse structure.
 *===========================================================================*/
typedef struct _UA_BrowseResponse
{
    UA_ResponseHeader  ResponseHeader;
    int32_t            NoOfResults;
    UA_BrowseResult*   Results;
    int32_t            NoOfDiagnosticInfos;
    UA_DiagnosticInfo* DiagnosticInfos;
}
UA_BrowseResponse;

void UA_BrowseResponse_Initialize(UA_BrowseResponse* pValue);

void UA_BrowseResponse_Clear(UA_BrowseResponse* pValue);

//StatusCode UA_BrowseResponse_GetSize(UA_BrowseResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_BrowseResponse_Encode(UA_MsgBuffer* msgBuf, UA_BrowseResponse* pValue);

StatusCode UA_BrowseResponse_Decode(UA_MsgBuffer* msgBuf, UA_BrowseResponse* pValue);

extern struct UA_EncodeableType UA_BrowseResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_BrowseNext
#ifndef OPCUA_EXCLUDE_BrowseNextRequest
/*============================================================================
 * The BrowseNextRequest structure.
 *===========================================================================*/
typedef struct _UA_BrowseNextRequest
{
    UA_RequestHeader RequestHeader;
    UA_Boolean       ReleaseContinuationPoints;
    int32_t          NoOfContinuationPoints;
    UA_ByteString*   ContinuationPoints;
}
UA_BrowseNextRequest;

void UA_BrowseNextRequest_Initialize(UA_BrowseNextRequest* pValue);

void UA_BrowseNextRequest_Clear(UA_BrowseNextRequest* pValue);

//StatusCode UA_BrowseNextRequest_GetSize(UA_BrowseNextRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_BrowseNextRequest_Encode(UA_MsgBuffer* msgBuf, UA_BrowseNextRequest* pValue);

StatusCode UA_BrowseNextRequest_Decode(UA_MsgBuffer* msgBuf, UA_BrowseNextRequest* pValue);

extern struct UA_EncodeableType UA_BrowseNextRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_BrowseNextResponse
/*============================================================================
 * The BrowseNextResponse structure.
 *===========================================================================*/
typedef struct _UA_BrowseNextResponse
{
    UA_ResponseHeader  ResponseHeader;
    int32_t            NoOfResults;
    UA_BrowseResult*   Results;
    int32_t            NoOfDiagnosticInfos;
    UA_DiagnosticInfo* DiagnosticInfos;
}
UA_BrowseNextResponse;

void UA_BrowseNextResponse_Initialize(UA_BrowseNextResponse* pValue);

void UA_BrowseNextResponse_Clear(UA_BrowseNextResponse* pValue);

//StatusCode UA_BrowseNextResponse_GetSize(UA_BrowseNextResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_BrowseNextResponse_Encode(UA_MsgBuffer* msgBuf, UA_BrowseNextResponse* pValue);

StatusCode UA_BrowseNextResponse_Decode(UA_MsgBuffer* msgBuf, UA_BrowseNextResponse* pValue);

extern struct UA_EncodeableType UA_BrowseNextResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_RelativePathElement
/*============================================================================
 * The RelativePathElement structure.
 *===========================================================================*/
typedef struct _UA_RelativePathElement
{
    UA_NodeId        ReferenceTypeId;
    UA_Boolean       IsInverse;
    UA_Boolean       IncludeSubtypes;
    UA_QualifiedName TargetName;
}
UA_RelativePathElement;

void UA_RelativePathElement_Initialize(UA_RelativePathElement* pValue);

void UA_RelativePathElement_Clear(UA_RelativePathElement* pValue);

//StatusCode UA_RelativePathElement_GetSize(UA_RelativePathElement* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_RelativePathElement_Encode(UA_MsgBuffer* msgBuf, UA_RelativePathElement* pValue);

StatusCode UA_RelativePathElement_Decode(UA_MsgBuffer* msgBuf, UA_RelativePathElement* pValue);

extern struct UA_EncodeableType UA_RelativePathElement_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_RelativePath
/*============================================================================
 * The RelativePath structure.
 *===========================================================================*/
typedef struct _UA_RelativePath
{
    int32_t                 NoOfElements;
    UA_RelativePathElement* Elements;
}
UA_RelativePath;

void UA_RelativePath_Initialize(UA_RelativePath* pValue);

void UA_RelativePath_Clear(UA_RelativePath* pValue);

//StatusCode UA_RelativePath_GetSize(UA_RelativePath* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_RelativePath_Encode(UA_MsgBuffer* msgBuf, UA_RelativePath* pValue);

StatusCode UA_RelativePath_Decode(UA_MsgBuffer* msgBuf, UA_RelativePath* pValue);

extern struct UA_EncodeableType UA_RelativePath_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_BrowsePath
/*============================================================================
 * The BrowsePath structure.
 *===========================================================================*/
typedef struct _UA_BrowsePath
{
    UA_NodeId       StartingNode;
    UA_RelativePath RelativePath;
}
UA_BrowsePath;

void UA_BrowsePath_Initialize(UA_BrowsePath* pValue);

void UA_BrowsePath_Clear(UA_BrowsePath* pValue);

//StatusCode UA_BrowsePath_GetSize(UA_BrowsePath* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_BrowsePath_Encode(UA_MsgBuffer* msgBuf, UA_BrowsePath* pValue);

StatusCode UA_BrowsePath_Decode(UA_MsgBuffer* msgBuf, UA_BrowsePath* pValue);

extern struct UA_EncodeableType UA_BrowsePath_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_BrowsePathTarget
/*============================================================================
 * The BrowsePathTarget structure.
 *===========================================================================*/
typedef struct _UA_BrowsePathTarget
{
    UA_ExpandedNodeId TargetId;
    uint32_t          RemainingPathIndex;
}
UA_BrowsePathTarget;

void UA_BrowsePathTarget_Initialize(UA_BrowsePathTarget* pValue);

void UA_BrowsePathTarget_Clear(UA_BrowsePathTarget* pValue);

//StatusCode UA_BrowsePathTarget_GetSize(UA_BrowsePathTarget* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_BrowsePathTarget_Encode(UA_MsgBuffer* msgBuf, UA_BrowsePathTarget* pValue);

StatusCode UA_BrowsePathTarget_Decode(UA_MsgBuffer* msgBuf, UA_BrowsePathTarget* pValue);

extern struct UA_EncodeableType UA_BrowsePathTarget_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_BrowsePathResult
/*============================================================================
 * The BrowsePathResult structure.
 *===========================================================================*/
typedef struct _UA_BrowsePathResult
{
    StatusCode           StatusCode;
    int32_t              NoOfTargets;
    UA_BrowsePathTarget* Targets;
}
UA_BrowsePathResult;

void UA_BrowsePathResult_Initialize(UA_BrowsePathResult* pValue);

void UA_BrowsePathResult_Clear(UA_BrowsePathResult* pValue);

//StatusCode UA_BrowsePathResult_GetSize(UA_BrowsePathResult* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_BrowsePathResult_Encode(UA_MsgBuffer* msgBuf, UA_BrowsePathResult* pValue);

StatusCode UA_BrowsePathResult_Decode(UA_MsgBuffer* msgBuf, UA_BrowsePathResult* pValue);

extern struct UA_EncodeableType UA_BrowsePathResult_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_TranslateBrowsePathsToNodeIds
#ifndef OPCUA_EXCLUDE_TranslateBrowsePathsToNodeIdsRequest
/*============================================================================
 * The TranslateBrowsePathsToNodeIdsRequest structure.
 *===========================================================================*/
typedef struct _UA_TranslateBrowsePathsToNodeIdsRequest
{
    UA_RequestHeader RequestHeader;
    int32_t          NoOfBrowsePaths;
    UA_BrowsePath*   BrowsePaths;
}
UA_TranslateBrowsePathsToNodeIdsRequest;

void UA_TranslateBrowsePathsToNodeIdsRequest_Initialize(UA_TranslateBrowsePathsToNodeIdsRequest* pValue);

void UA_TranslateBrowsePathsToNodeIdsRequest_Clear(UA_TranslateBrowsePathsToNodeIdsRequest* pValue);

//StatusCode UA_TranslateBrowsePathsToNodeIdsRequest_GetSize(UA_TranslateBrowsePathsToNodeIdsRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_TranslateBrowsePathsToNodeIdsRequest_Encode(UA_MsgBuffer* msgBuf, UA_TranslateBrowsePathsToNodeIdsRequest* pValue);

StatusCode UA_TranslateBrowsePathsToNodeIdsRequest_Decode(UA_MsgBuffer* msgBuf, UA_TranslateBrowsePathsToNodeIdsRequest* pValue);

extern struct UA_EncodeableType UA_TranslateBrowsePathsToNodeIdsRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_TranslateBrowsePathsToNodeIdsResponse
/*============================================================================
 * The TranslateBrowsePathsToNodeIdsResponse structure.
 *===========================================================================*/
typedef struct _UA_TranslateBrowsePathsToNodeIdsResponse
{
    UA_ResponseHeader    ResponseHeader;
    int32_t              NoOfResults;
    UA_BrowsePathResult* Results;
    int32_t              NoOfDiagnosticInfos;
    UA_DiagnosticInfo*   DiagnosticInfos;
}
UA_TranslateBrowsePathsToNodeIdsResponse;

void UA_TranslateBrowsePathsToNodeIdsResponse_Initialize(UA_TranslateBrowsePathsToNodeIdsResponse* pValue);

void UA_TranslateBrowsePathsToNodeIdsResponse_Clear(UA_TranslateBrowsePathsToNodeIdsResponse* pValue);

//StatusCode UA_TranslateBrowsePathsToNodeIdsResponse_GetSize(UA_TranslateBrowsePathsToNodeIdsResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_TranslateBrowsePathsToNodeIdsResponse_Encode(UA_MsgBuffer* msgBuf, UA_TranslateBrowsePathsToNodeIdsResponse* pValue);

StatusCode UA_TranslateBrowsePathsToNodeIdsResponse_Decode(UA_MsgBuffer* msgBuf, UA_TranslateBrowsePathsToNodeIdsResponse* pValue);

extern struct UA_EncodeableType UA_TranslateBrowsePathsToNodeIdsResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_RegisterNodes
#ifndef OPCUA_EXCLUDE_RegisterNodesRequest
/*============================================================================
 * The RegisterNodesRequest structure.
 *===========================================================================*/
typedef struct _UA_RegisterNodesRequest
{
    UA_RequestHeader RequestHeader;
    int32_t          NoOfNodesToRegister;
    UA_NodeId*       NodesToRegister;
}
UA_RegisterNodesRequest;

void UA_RegisterNodesRequest_Initialize(UA_RegisterNodesRequest* pValue);

void UA_RegisterNodesRequest_Clear(UA_RegisterNodesRequest* pValue);

//StatusCode UA_RegisterNodesRequest_GetSize(UA_RegisterNodesRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_RegisterNodesRequest_Encode(UA_MsgBuffer* msgBuf, UA_RegisterNodesRequest* pValue);

StatusCode UA_RegisterNodesRequest_Decode(UA_MsgBuffer* msgBuf, UA_RegisterNodesRequest* pValue);

extern struct UA_EncodeableType UA_RegisterNodesRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_RegisterNodesResponse
/*============================================================================
 * The RegisterNodesResponse structure.
 *===========================================================================*/
typedef struct _UA_RegisterNodesResponse
{
    UA_ResponseHeader ResponseHeader;
    int32_t           NoOfRegisteredNodeIds;
    UA_NodeId*        RegisteredNodeIds;
}
UA_RegisterNodesResponse;

void UA_RegisterNodesResponse_Initialize(UA_RegisterNodesResponse* pValue);

void UA_RegisterNodesResponse_Clear(UA_RegisterNodesResponse* pValue);

//StatusCode UA_RegisterNodesResponse_GetSize(UA_RegisterNodesResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_RegisterNodesResponse_Encode(UA_MsgBuffer* msgBuf, UA_RegisterNodesResponse* pValue);

StatusCode UA_RegisterNodesResponse_Decode(UA_MsgBuffer* msgBuf, UA_RegisterNodesResponse* pValue);

extern struct UA_EncodeableType UA_RegisterNodesResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_UnregisterNodes
#ifndef OPCUA_EXCLUDE_UnregisterNodesRequest
/*============================================================================
 * The UnregisterNodesRequest structure.
 *===========================================================================*/
typedef struct _UA_UnregisterNodesRequest
{
    UA_RequestHeader RequestHeader;
    int32_t          NoOfNodesToUnregister;
    UA_NodeId*       NodesToUnregister;
}
UA_UnregisterNodesRequest;

void UA_UnregisterNodesRequest_Initialize(UA_UnregisterNodesRequest* pValue);

void UA_UnregisterNodesRequest_Clear(UA_UnregisterNodesRequest* pValue);

//StatusCode UA_UnregisterNodesRequest_GetSize(UA_UnregisterNodesRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_UnregisterNodesRequest_Encode(UA_MsgBuffer* msgBuf, UA_UnregisterNodesRequest* pValue);

StatusCode UA_UnregisterNodesRequest_Decode(UA_MsgBuffer* msgBuf, UA_UnregisterNodesRequest* pValue);

extern struct UA_EncodeableType UA_UnregisterNodesRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_UnregisterNodesResponse
/*============================================================================
 * The UnregisterNodesResponse structure.
 *===========================================================================*/
typedef struct _UA_UnregisterNodesResponse
{
    UA_ResponseHeader ResponseHeader;
}
UA_UnregisterNodesResponse;

void UA_UnregisterNodesResponse_Initialize(UA_UnregisterNodesResponse* pValue);

void UA_UnregisterNodesResponse_Clear(UA_UnregisterNodesResponse* pValue);

//StatusCode UA_UnregisterNodesResponse_GetSize(UA_UnregisterNodesResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_UnregisterNodesResponse_Encode(UA_MsgBuffer* msgBuf, UA_UnregisterNodesResponse* pValue);

StatusCode UA_UnregisterNodesResponse_Decode(UA_MsgBuffer* msgBuf, UA_UnregisterNodesResponse* pValue);

extern struct UA_EncodeableType UA_UnregisterNodesResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_EndpointConfiguration
/*============================================================================
 * The EndpointConfiguration structure.
 *===========================================================================*/
typedef struct _UA_EndpointConfiguration
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
UA_EndpointConfiguration;

void UA_EndpointConfiguration_Initialize(UA_EndpointConfiguration* pValue);

void UA_EndpointConfiguration_Clear(UA_EndpointConfiguration* pValue);

//StatusCode UA_EndpointConfiguration_GetSize(UA_EndpointConfiguration* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_EndpointConfiguration_Encode(UA_MsgBuffer* msgBuf, UA_EndpointConfiguration* pValue);

StatusCode UA_EndpointConfiguration_Decode(UA_MsgBuffer* msgBuf, UA_EndpointConfiguration* pValue);

extern struct UA_EncodeableType UA_EndpointConfiguration_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ComplianceLevel
/*============================================================================
 * The ComplianceLevel enumeration.
 *===========================================================================*/
typedef enum _UA_ComplianceLevel
{
    UA_ComplianceLevel_Untested   = 0,
    UA_ComplianceLevel_Partial    = 1,
    UA_ComplianceLevel_SelfTested = 2,
    UA_ComplianceLevel_Certified  = 3
#if OPCUA_FORCE_INT32_ENUMS
    ,_UA_ComplianceLevel_MaxEnumerationValue = OpcUa_Int32_Max
#endif
}
UA_ComplianceLevel;

#endif

#ifndef OPCUA_EXCLUDE_SupportedProfile
/*============================================================================
 * The SupportedProfile structure.
 *===========================================================================*/
typedef struct _UA_SupportedProfile
{
    UA_String          OrganizationUri;
    UA_String          ProfileId;
    UA_String          ComplianceTool;
    UA_DateTime        ComplianceDate;
    UA_ComplianceLevel ComplianceLevel;
    int32_t            NoOfUnsupportedUnitIds;
    UA_String*         UnsupportedUnitIds;
}
UA_SupportedProfile;

void UA_SupportedProfile_Initialize(UA_SupportedProfile* pValue);

void UA_SupportedProfile_Clear(UA_SupportedProfile* pValue);

//StatusCode UA_SupportedProfile_GetSize(UA_SupportedProfile* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_SupportedProfile_Encode(UA_MsgBuffer* msgBuf, UA_SupportedProfile* pValue);

StatusCode UA_SupportedProfile_Decode(UA_MsgBuffer* msgBuf, UA_SupportedProfile* pValue);

extern struct UA_EncodeableType UA_SupportedProfile_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_SoftwareCertificate
/*============================================================================
 * The SoftwareCertificate structure.
 *===========================================================================*/
typedef struct _UA_SoftwareCertificate
{
    UA_String            ProductName;
    UA_String            ProductUri;
    UA_String            VendorName;
    UA_ByteString        VendorProductCertificate;
    UA_String            SoftwareVersion;
    UA_String            BuildNumber;
    UA_DateTime          BuildDate;
    UA_String            IssuedBy;
    UA_DateTime          IssueDate;
    int32_t              NoOfSupportedProfiles;
    UA_SupportedProfile* SupportedProfiles;
}
UA_SoftwareCertificate;

void UA_SoftwareCertificate_Initialize(UA_SoftwareCertificate* pValue);

void UA_SoftwareCertificate_Clear(UA_SoftwareCertificate* pValue);

//StatusCode UA_SoftwareCertificate_GetSize(UA_SoftwareCertificate* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_SoftwareCertificate_Encode(UA_MsgBuffer* msgBuf, UA_SoftwareCertificate* pValue);

StatusCode UA_SoftwareCertificate_Decode(UA_MsgBuffer* msgBuf, UA_SoftwareCertificate* pValue);

extern struct UA_EncodeableType UA_SoftwareCertificate_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_QueryDataDescription
/*============================================================================
 * The QueryDataDescription structure.
 *===========================================================================*/
typedef struct _UA_QueryDataDescription
{
    UA_RelativePath RelativePath;
    uint32_t        AttributeId;
    UA_String       IndexRange;
}
UA_QueryDataDescription;

void UA_QueryDataDescription_Initialize(UA_QueryDataDescription* pValue);

void UA_QueryDataDescription_Clear(UA_QueryDataDescription* pValue);

//StatusCode UA_QueryDataDescription_GetSize(UA_QueryDataDescription* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_QueryDataDescription_Encode(UA_MsgBuffer* msgBuf, UA_QueryDataDescription* pValue);

StatusCode UA_QueryDataDescription_Decode(UA_MsgBuffer* msgBuf, UA_QueryDataDescription* pValue);

extern struct UA_EncodeableType UA_QueryDataDescription_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_NodeTypeDescription
/*============================================================================
 * The NodeTypeDescription structure.
 *===========================================================================*/
typedef struct _UA_NodeTypeDescription
{
    UA_ExpandedNodeId        TypeDefinitionNode;
    UA_Boolean               IncludeSubTypes;
    int32_t                  NoOfDataToReturn;
    UA_QueryDataDescription* DataToReturn;
}
UA_NodeTypeDescription;

void UA_NodeTypeDescription_Initialize(UA_NodeTypeDescription* pValue);

void UA_NodeTypeDescription_Clear(UA_NodeTypeDescription* pValue);

//StatusCode UA_NodeTypeDescription_GetSize(UA_NodeTypeDescription* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_NodeTypeDescription_Encode(UA_MsgBuffer* msgBuf, UA_NodeTypeDescription* pValue);

StatusCode UA_NodeTypeDescription_Decode(UA_MsgBuffer* msgBuf, UA_NodeTypeDescription* pValue);

extern struct UA_EncodeableType UA_NodeTypeDescription_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_FilterOperator
/*============================================================================
 * The FilterOperator enumeration.
 *===========================================================================*/
typedef enum _UA_FilterOperator
{
    UA_FilterOperator_Equals             = 0,
    UA_FilterOperator_IsNull             = 1,
    UA_FilterOperator_GreaterThan        = 2,
    UA_FilterOperator_LessThan           = 3,
    UA_FilterOperator_GreaterThanOrEqual = 4,
    UA_FilterOperator_LessThanOrEqual    = 5,
    UA_FilterOperator_Like               = 6,
    UA_FilterOperator_Not                = 7,
    UA_FilterOperator_Between            = 8,
    UA_FilterOperator_InList             = 9,
    UA_FilterOperator_And                = 10,
    UA_FilterOperator_Or                 = 11,
    UA_FilterOperator_Cast               = 12,
    UA_FilterOperator_InView             = 13,
    UA_FilterOperator_OfType             = 14,
    UA_FilterOperator_RelatedTo          = 15,
    UA_FilterOperator_BitwiseAnd         = 16,
    UA_FilterOperator_BitwiseOr          = 17
#if OPCUA_FORCE_INT32_ENUMS
    ,_UA_FilterOperator_MaxEnumerationValue = OpcUa_Int32_Max
#endif
}
UA_FilterOperator;

#endif

#ifndef OPCUA_EXCLUDE_QueryDataSet
/*============================================================================
 * The QueryDataSet structure.
 *===========================================================================*/
typedef struct _UA_QueryDataSet
{
    UA_ExpandedNodeId NodeId;
    UA_ExpandedNodeId TypeDefinitionNode;
    int32_t           NoOfValues;
    UA_Variant*       Values;
}
UA_QueryDataSet;

void UA_QueryDataSet_Initialize(UA_QueryDataSet* pValue);

void UA_QueryDataSet_Clear(UA_QueryDataSet* pValue);

//StatusCode UA_QueryDataSet_GetSize(UA_QueryDataSet* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_QueryDataSet_Encode(UA_MsgBuffer* msgBuf, UA_QueryDataSet* pValue);

StatusCode UA_QueryDataSet_Decode(UA_MsgBuffer* msgBuf, UA_QueryDataSet* pValue);

extern struct UA_EncodeableType UA_QueryDataSet_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_NodeReference
/*============================================================================
 * The NodeReference structure.
 *===========================================================================*/
typedef struct _UA_NodeReference
{
    UA_NodeId  NodeId;
    UA_NodeId  ReferenceTypeId;
    UA_Boolean IsForward;
    int32_t    NoOfReferencedNodeIds;
    UA_NodeId* ReferencedNodeIds;
}
UA_NodeReference;

void UA_NodeReference_Initialize(UA_NodeReference* pValue);

void UA_NodeReference_Clear(UA_NodeReference* pValue);

//StatusCode UA_NodeReference_GetSize(UA_NodeReference* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_NodeReference_Encode(UA_MsgBuffer* msgBuf, UA_NodeReference* pValue);

StatusCode UA_NodeReference_Decode(UA_MsgBuffer* msgBuf, UA_NodeReference* pValue);

extern struct UA_EncodeableType UA_NodeReference_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ContentFilterElement
/*============================================================================
 * The ContentFilterElement structure.
 *===========================================================================*/
typedef struct _UA_ContentFilterElement
{
    UA_FilterOperator   FilterOperator;
    int32_t             NoOfFilterOperands;
    UA_ExtensionObject* FilterOperands;
}
UA_ContentFilterElement;

void UA_ContentFilterElement_Initialize(UA_ContentFilterElement* pValue);

void UA_ContentFilterElement_Clear(UA_ContentFilterElement* pValue);

//StatusCode UA_ContentFilterElement_GetSize(UA_ContentFilterElement* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_ContentFilterElement_Encode(UA_MsgBuffer* msgBuf, UA_ContentFilterElement* pValue);

StatusCode UA_ContentFilterElement_Decode(UA_MsgBuffer* msgBuf, UA_ContentFilterElement* pValue);

extern struct UA_EncodeableType UA_ContentFilterElement_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ContentFilter
/*============================================================================
 * The ContentFilter structure.
 *===========================================================================*/
typedef struct _UA_ContentFilter
{
    int32_t                  NoOfElements;
    UA_ContentFilterElement* Elements;
}
UA_ContentFilter;

void UA_ContentFilter_Initialize(UA_ContentFilter* pValue);

void UA_ContentFilter_Clear(UA_ContentFilter* pValue);

//StatusCode UA_ContentFilter_GetSize(UA_ContentFilter* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_ContentFilter_Encode(UA_MsgBuffer* msgBuf, UA_ContentFilter* pValue);

StatusCode UA_ContentFilter_Decode(UA_MsgBuffer* msgBuf, UA_ContentFilter* pValue);

extern struct UA_EncodeableType UA_ContentFilter_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ElementOperand
/*============================================================================
 * The ElementOperand structure.
 *===========================================================================*/
typedef struct _UA_ElementOperand
{
    uint32_t Index;
}
UA_ElementOperand;

void UA_ElementOperand_Initialize(UA_ElementOperand* pValue);

void UA_ElementOperand_Clear(UA_ElementOperand* pValue);

//StatusCode UA_ElementOperand_GetSize(UA_ElementOperand* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_ElementOperand_Encode(UA_MsgBuffer* msgBuf, UA_ElementOperand* pValue);

StatusCode UA_ElementOperand_Decode(UA_MsgBuffer* msgBuf, UA_ElementOperand* pValue);

extern struct UA_EncodeableType UA_ElementOperand_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_LiteralOperand
/*============================================================================
 * The LiteralOperand structure.
 *===========================================================================*/
typedef struct _UA_LiteralOperand
{
    UA_Variant Value;
}
UA_LiteralOperand;

void UA_LiteralOperand_Initialize(UA_LiteralOperand* pValue);

void UA_LiteralOperand_Clear(UA_LiteralOperand* pValue);

//StatusCode UA_LiteralOperand_GetSize(UA_LiteralOperand* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_LiteralOperand_Encode(UA_MsgBuffer* msgBuf, UA_LiteralOperand* pValue);

StatusCode UA_LiteralOperand_Decode(UA_MsgBuffer* msgBuf, UA_LiteralOperand* pValue);

extern struct UA_EncodeableType UA_LiteralOperand_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_AttributeOperand
/*============================================================================
 * The AttributeOperand structure.
 *===========================================================================*/
typedef struct _UA_AttributeOperand
{
    UA_NodeId       NodeId;
    UA_String       Alias;
    UA_RelativePath BrowsePath;
    uint32_t        AttributeId;
    UA_String       IndexRange;
}
UA_AttributeOperand;

void UA_AttributeOperand_Initialize(UA_AttributeOperand* pValue);

void UA_AttributeOperand_Clear(UA_AttributeOperand* pValue);

//StatusCode UA_AttributeOperand_GetSize(UA_AttributeOperand* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_AttributeOperand_Encode(UA_MsgBuffer* msgBuf, UA_AttributeOperand* pValue);

StatusCode UA_AttributeOperand_Decode(UA_MsgBuffer* msgBuf, UA_AttributeOperand* pValue);

extern struct UA_EncodeableType UA_AttributeOperand_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_SimpleAttributeOperand
/*============================================================================
 * The SimpleAttributeOperand structure.
 *===========================================================================*/
typedef struct _UA_SimpleAttributeOperand
{
    UA_NodeId         TypeDefinitionId;
    int32_t           NoOfBrowsePath;
    UA_QualifiedName* BrowsePath;
    uint32_t          AttributeId;
    UA_String         IndexRange;
}
UA_SimpleAttributeOperand;

void UA_SimpleAttributeOperand_Initialize(UA_SimpleAttributeOperand* pValue);

void UA_SimpleAttributeOperand_Clear(UA_SimpleAttributeOperand* pValue);

//StatusCode UA_SimpleAttributeOperand_GetSize(UA_SimpleAttributeOperand* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_SimpleAttributeOperand_Encode(UA_MsgBuffer* msgBuf, UA_SimpleAttributeOperand* pValue);

StatusCode UA_SimpleAttributeOperand_Decode(UA_MsgBuffer* msgBuf, UA_SimpleAttributeOperand* pValue);

extern struct UA_EncodeableType UA_SimpleAttributeOperand_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ContentFilterElementResult
/*============================================================================
 * The ContentFilterElementResult structure.
 *===========================================================================*/
typedef struct _UA_ContentFilterElementResult
{
    StatusCode         StatusCode;
    int32_t            NoOfOperandStatusCodes;
    StatusCode*        OperandStatusCodes;
    int32_t            NoOfOperandDiagnosticInfos;
    UA_DiagnosticInfo* OperandDiagnosticInfos;
}
UA_ContentFilterElementResult;

void UA_ContentFilterElementResult_Initialize(UA_ContentFilterElementResult* pValue);

void UA_ContentFilterElementResult_Clear(UA_ContentFilterElementResult* pValue);

//StatusCode UA_ContentFilterElementResult_GetSize(UA_ContentFilterElementResult* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_ContentFilterElementResult_Encode(UA_MsgBuffer* msgBuf, UA_ContentFilterElementResult* pValue);

StatusCode UA_ContentFilterElementResult_Decode(UA_MsgBuffer* msgBuf, UA_ContentFilterElementResult* pValue);

extern struct UA_EncodeableType UA_ContentFilterElementResult_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ContentFilterResult
/*============================================================================
 * The ContentFilterResult structure.
 *===========================================================================*/
typedef struct _UA_ContentFilterResult
{
    int32_t                        NoOfElementResults;
    UA_ContentFilterElementResult* ElementResults;
    int32_t                        NoOfElementDiagnosticInfos;
    UA_DiagnosticInfo*             ElementDiagnosticInfos;
}
UA_ContentFilterResult;

void UA_ContentFilterResult_Initialize(UA_ContentFilterResult* pValue);

void UA_ContentFilterResult_Clear(UA_ContentFilterResult* pValue);

//StatusCode UA_ContentFilterResult_GetSize(UA_ContentFilterResult* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_ContentFilterResult_Encode(UA_MsgBuffer* msgBuf, UA_ContentFilterResult* pValue);

StatusCode UA_ContentFilterResult_Decode(UA_MsgBuffer* msgBuf, UA_ContentFilterResult* pValue);

extern struct UA_EncodeableType UA_ContentFilterResult_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ParsingResult
/*============================================================================
 * The ParsingResult structure.
 *===========================================================================*/
typedef struct _UA_ParsingResult
{
    StatusCode         StatusCode;
    int32_t            NoOfDataStatusCodes;
    StatusCode*        DataStatusCodes;
    int32_t            NoOfDataDiagnosticInfos;
    UA_DiagnosticInfo* DataDiagnosticInfos;
}
UA_ParsingResult;

void UA_ParsingResult_Initialize(UA_ParsingResult* pValue);

void UA_ParsingResult_Clear(UA_ParsingResult* pValue);

//StatusCode UA_ParsingResult_GetSize(UA_ParsingResult* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_ParsingResult_Encode(UA_MsgBuffer* msgBuf, UA_ParsingResult* pValue);

StatusCode UA_ParsingResult_Decode(UA_MsgBuffer* msgBuf, UA_ParsingResult* pValue);

extern struct UA_EncodeableType UA_ParsingResult_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_QueryFirst
#ifndef OPCUA_EXCLUDE_QueryFirstRequest
/*============================================================================
 * The QueryFirstRequest structure.
 *===========================================================================*/
typedef struct _UA_QueryFirstRequest
{
    UA_RequestHeader        RequestHeader;
    UA_ViewDescription      View;
    int32_t                 NoOfNodeTypes;
    UA_NodeTypeDescription* NodeTypes;
    UA_ContentFilter        Filter;
    uint32_t                MaxDataSetsToReturn;
    uint32_t                MaxReferencesToReturn;
}
UA_QueryFirstRequest;

void UA_QueryFirstRequest_Initialize(UA_QueryFirstRequest* pValue);

void UA_QueryFirstRequest_Clear(UA_QueryFirstRequest* pValue);

//StatusCode UA_QueryFirstRequest_GetSize(UA_QueryFirstRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_QueryFirstRequest_Encode(UA_MsgBuffer* msgBuf, UA_QueryFirstRequest* pValue);

StatusCode UA_QueryFirstRequest_Decode(UA_MsgBuffer* msgBuf, UA_QueryFirstRequest* pValue);

extern struct UA_EncodeableType UA_QueryFirstRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_QueryFirstResponse
/*============================================================================
 * The QueryFirstResponse structure.
 *===========================================================================*/
typedef struct _UA_QueryFirstResponse
{
    UA_ResponseHeader      ResponseHeader;
    int32_t                NoOfQueryDataSets;
    UA_QueryDataSet*       QueryDataSets;
    UA_ByteString          ContinuationPoint;
    int32_t                NoOfParsingResults;
    UA_ParsingResult*      ParsingResults;
    int32_t                NoOfDiagnosticInfos;
    UA_DiagnosticInfo*     DiagnosticInfos;
    UA_ContentFilterResult FilterResult;
}
UA_QueryFirstResponse;

void UA_QueryFirstResponse_Initialize(UA_QueryFirstResponse* pValue);

void UA_QueryFirstResponse_Clear(UA_QueryFirstResponse* pValue);

//StatusCode UA_QueryFirstResponse_GetSize(UA_QueryFirstResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_QueryFirstResponse_Encode(UA_MsgBuffer* msgBuf, UA_QueryFirstResponse* pValue);

StatusCode UA_QueryFirstResponse_Decode(UA_MsgBuffer* msgBuf, UA_QueryFirstResponse* pValue);

extern struct UA_EncodeableType UA_QueryFirstResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_QueryNext
#ifndef OPCUA_EXCLUDE_QueryNextRequest
/*============================================================================
 * The QueryNextRequest structure.
 *===========================================================================*/
typedef struct _UA_QueryNextRequest
{
    UA_RequestHeader RequestHeader;
    UA_Boolean       ReleaseContinuationPoint;
    UA_ByteString    ContinuationPoint;
}
UA_QueryNextRequest;

void UA_QueryNextRequest_Initialize(UA_QueryNextRequest* pValue);

void UA_QueryNextRequest_Clear(UA_QueryNextRequest* pValue);

//StatusCode UA_QueryNextRequest_GetSize(UA_QueryNextRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_QueryNextRequest_Encode(UA_MsgBuffer* msgBuf, UA_QueryNextRequest* pValue);

StatusCode UA_QueryNextRequest_Decode(UA_MsgBuffer* msgBuf, UA_QueryNextRequest* pValue);

extern struct UA_EncodeableType UA_QueryNextRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_QueryNextResponse
/*============================================================================
 * The QueryNextResponse structure.
 *===========================================================================*/
typedef struct _UA_QueryNextResponse
{
    UA_ResponseHeader ResponseHeader;
    int32_t           NoOfQueryDataSets;
    UA_QueryDataSet*  QueryDataSets;
    UA_ByteString     RevisedContinuationPoint;
}
UA_QueryNextResponse;

void UA_QueryNextResponse_Initialize(UA_QueryNextResponse* pValue);

void UA_QueryNextResponse_Clear(UA_QueryNextResponse* pValue);

//StatusCode UA_QueryNextResponse_GetSize(UA_QueryNextResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_QueryNextResponse_Encode(UA_MsgBuffer* msgBuf, UA_QueryNextResponse* pValue);

StatusCode UA_QueryNextResponse_Decode(UA_MsgBuffer* msgBuf, UA_QueryNextResponse* pValue);

extern struct UA_EncodeableType UA_QueryNextResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_TimestampsToReturn
/*============================================================================
 * The TimestampsToReturn enumeration.
 *===========================================================================*/
typedef enum _UA_TimestampsToReturn
{
    UA_TimestampsToReturn_Source  = 0,
    UA_TimestampsToReturn_Server  = 1,
    UA_TimestampsToReturn_Both    = 2,
    UA_TimestampsToReturn_Neither = 3
#if OPCUA_FORCE_INT32_ENUMS
    ,_UA_TimestampsToReturn_MaxEnumerationValue = OpcUa_Int32_Max
#endif
}
UA_TimestampsToReturn;

#endif

#ifndef OPCUA_EXCLUDE_ReadValueId
/*============================================================================
 * The ReadValueId structure.
 *===========================================================================*/
typedef struct _UA_ReadValueId
{
    UA_NodeId        NodeId;
    uint32_t         AttributeId;
    UA_String        IndexRange;
    UA_QualifiedName DataEncoding;
}
UA_ReadValueId;

void UA_ReadValueId_Initialize(UA_ReadValueId* pValue);

void UA_ReadValueId_Clear(UA_ReadValueId* pValue);

//StatusCode UA_ReadValueId_GetSize(UA_ReadValueId* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_ReadValueId_Encode(UA_MsgBuffer* msgBuf, UA_ReadValueId* pValue);

StatusCode UA_ReadValueId_Decode(UA_MsgBuffer* msgBuf, UA_ReadValueId* pValue);

extern struct UA_EncodeableType UA_ReadValueId_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_Read
#ifndef OPCUA_EXCLUDE_ReadRequest
/*============================================================================
 * The ReadRequest structure.
 *===========================================================================*/
typedef struct _UA_ReadRequest
{
    UA_RequestHeader      RequestHeader;
    double                MaxAge;
    UA_TimestampsToReturn TimestampsToReturn;
    int32_t               NoOfNodesToRead;
    UA_ReadValueId*       NodesToRead;
}
UA_ReadRequest;

void UA_ReadRequest_Initialize(UA_ReadRequest* pValue);

void UA_ReadRequest_Clear(UA_ReadRequest* pValue);

//StatusCode UA_ReadRequest_GetSize(UA_ReadRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_ReadRequest_Encode(UA_MsgBuffer* msgBuf, UA_ReadRequest* pValue);

StatusCode UA_ReadRequest_Decode(UA_MsgBuffer* msgBuf, UA_ReadRequest* pValue);

extern struct UA_EncodeableType UA_ReadRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ReadResponse
/*============================================================================
 * The ReadResponse structure.
 *===========================================================================*/
typedef struct _UA_ReadResponse
{
    UA_ResponseHeader  ResponseHeader;
    int32_t            NoOfResults;
    UA_DataValue*      Results;
    int32_t            NoOfDiagnosticInfos;
    UA_DiagnosticInfo* DiagnosticInfos;
}
UA_ReadResponse;

void UA_ReadResponse_Initialize(UA_ReadResponse* pValue);

void UA_ReadResponse_Clear(UA_ReadResponse* pValue);

//StatusCode UA_ReadResponse_GetSize(UA_ReadResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_ReadResponse_Encode(UA_MsgBuffer* msgBuf, UA_ReadResponse* pValue);

StatusCode UA_ReadResponse_Decode(UA_MsgBuffer* msgBuf, UA_ReadResponse* pValue);

extern struct UA_EncodeableType UA_ReadResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_HistoryReadValueId
/*============================================================================
 * The HistoryReadValueId structure.
 *===========================================================================*/
typedef struct _UA_HistoryReadValueId
{
    UA_NodeId        NodeId;
    UA_String        IndexRange;
    UA_QualifiedName DataEncoding;
    UA_ByteString    ContinuationPoint;
}
UA_HistoryReadValueId;

void UA_HistoryReadValueId_Initialize(UA_HistoryReadValueId* pValue);

void UA_HistoryReadValueId_Clear(UA_HistoryReadValueId* pValue);

//StatusCode UA_HistoryReadValueId_GetSize(UA_HistoryReadValueId* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_HistoryReadValueId_Encode(UA_MsgBuffer* msgBuf, UA_HistoryReadValueId* pValue);

StatusCode UA_HistoryReadValueId_Decode(UA_MsgBuffer* msgBuf, UA_HistoryReadValueId* pValue);

extern struct UA_EncodeableType UA_HistoryReadValueId_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_HistoryReadResult
/*============================================================================
 * The HistoryReadResult structure.
 *===========================================================================*/
typedef struct _UA_HistoryReadResult
{
    StatusCode         StatusCode;
    UA_ByteString      ContinuationPoint;
    UA_ExtensionObject HistoryData;
}
UA_HistoryReadResult;

void UA_HistoryReadResult_Initialize(UA_HistoryReadResult* pValue);

void UA_HistoryReadResult_Clear(UA_HistoryReadResult* pValue);

//StatusCode UA_HistoryReadResult_GetSize(UA_HistoryReadResult* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_HistoryReadResult_Encode(UA_MsgBuffer* msgBuf, UA_HistoryReadResult* pValue);

StatusCode UA_HistoryReadResult_Decode(UA_MsgBuffer* msgBuf, UA_HistoryReadResult* pValue);

extern struct UA_EncodeableType UA_HistoryReadResult_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_EventFilter
/*============================================================================
 * The EventFilter structure.
 *===========================================================================*/
typedef struct _UA_EventFilter
{
    int32_t                    NoOfSelectClauses;
    UA_SimpleAttributeOperand* SelectClauses;
    UA_ContentFilter           WhereClause;
}
UA_EventFilter;

void UA_EventFilter_Initialize(UA_EventFilter* pValue);

void UA_EventFilter_Clear(UA_EventFilter* pValue);

//StatusCode UA_EventFilter_GetSize(UA_EventFilter* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_EventFilter_Encode(UA_MsgBuffer* msgBuf, UA_EventFilter* pValue);

StatusCode UA_EventFilter_Decode(UA_MsgBuffer* msgBuf, UA_EventFilter* pValue);

extern struct UA_EncodeableType UA_EventFilter_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ReadEventDetails
/*============================================================================
 * The ReadEventDetails structure.
 *===========================================================================*/
typedef struct _UA_ReadEventDetails
{
    uint32_t       NumValuesPerNode;
    UA_DateTime    StartTime;
    UA_DateTime    EndTime;
    UA_EventFilter Filter;
}
UA_ReadEventDetails;

void UA_ReadEventDetails_Initialize(UA_ReadEventDetails* pValue);

void UA_ReadEventDetails_Clear(UA_ReadEventDetails* pValue);

//StatusCode UA_ReadEventDetails_GetSize(UA_ReadEventDetails* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_ReadEventDetails_Encode(UA_MsgBuffer* msgBuf, UA_ReadEventDetails* pValue);

StatusCode UA_ReadEventDetails_Decode(UA_MsgBuffer* msgBuf, UA_ReadEventDetails* pValue);

extern struct UA_EncodeableType UA_ReadEventDetails_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ReadRawModifiedDetails
/*============================================================================
 * The ReadRawModifiedDetails structure.
 *===========================================================================*/
typedef struct _UA_ReadRawModifiedDetails
{
    UA_Boolean  IsReadModified;
    UA_DateTime StartTime;
    UA_DateTime EndTime;
    uint32_t    NumValuesPerNode;
    UA_Boolean  ReturnBounds;
}
UA_ReadRawModifiedDetails;

void UA_ReadRawModifiedDetails_Initialize(UA_ReadRawModifiedDetails* pValue);

void UA_ReadRawModifiedDetails_Clear(UA_ReadRawModifiedDetails* pValue);

//StatusCode UA_ReadRawModifiedDetails_GetSize(UA_ReadRawModifiedDetails* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_ReadRawModifiedDetails_Encode(UA_MsgBuffer* msgBuf, UA_ReadRawModifiedDetails* pValue);

StatusCode UA_ReadRawModifiedDetails_Decode(UA_MsgBuffer* msgBuf, UA_ReadRawModifiedDetails* pValue);

extern struct UA_EncodeableType UA_ReadRawModifiedDetails_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_AggregateConfiguration
/*============================================================================
 * The AggregateConfiguration structure.
 *===========================================================================*/
typedef struct _UA_AggregateConfiguration
{
    UA_Boolean UseServerCapabilitiesDefaults;
    UA_Boolean TreatUncertainAsBad;
    UA_Byte    PercentDataBad;
    UA_Byte    PercentDataGood;
    UA_Boolean UseSlopedExtrapolation;
}
UA_AggregateConfiguration;

void UA_AggregateConfiguration_Initialize(UA_AggregateConfiguration* pValue);

void UA_AggregateConfiguration_Clear(UA_AggregateConfiguration* pValue);

//StatusCode UA_AggregateConfiguration_GetSize(UA_AggregateConfiguration* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_AggregateConfiguration_Encode(UA_MsgBuffer* msgBuf, UA_AggregateConfiguration* pValue);

StatusCode UA_AggregateConfiguration_Decode(UA_MsgBuffer* msgBuf, UA_AggregateConfiguration* pValue);

extern struct UA_EncodeableType UA_AggregateConfiguration_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ReadProcessedDetails
/*============================================================================
 * The ReadProcessedDetails structure.
 *===========================================================================*/
typedef struct _UA_ReadProcessedDetails
{
    UA_DateTime               StartTime;
    UA_DateTime               EndTime;
    double                    ProcessingInterval;
    int32_t                   NoOfAggregateType;
    UA_NodeId*                AggregateType;
    UA_AggregateConfiguration AggregateConfiguration;
}
UA_ReadProcessedDetails;

void UA_ReadProcessedDetails_Initialize(UA_ReadProcessedDetails* pValue);

void UA_ReadProcessedDetails_Clear(UA_ReadProcessedDetails* pValue);

//StatusCode UA_ReadProcessedDetails_GetSize(UA_ReadProcessedDetails* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_ReadProcessedDetails_Encode(UA_MsgBuffer* msgBuf, UA_ReadProcessedDetails* pValue);

StatusCode UA_ReadProcessedDetails_Decode(UA_MsgBuffer* msgBuf, UA_ReadProcessedDetails* pValue);

extern struct UA_EncodeableType UA_ReadProcessedDetails_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ReadAtTimeDetails
/*============================================================================
 * The ReadAtTimeDetails structure.
 *===========================================================================*/
typedef struct _UA_ReadAtTimeDetails
{
    int32_t      NoOfReqTimes;
    UA_DateTime* ReqTimes;
    UA_Boolean   UseSimpleBounds;
}
UA_ReadAtTimeDetails;

void UA_ReadAtTimeDetails_Initialize(UA_ReadAtTimeDetails* pValue);

void UA_ReadAtTimeDetails_Clear(UA_ReadAtTimeDetails* pValue);

//StatusCode UA_ReadAtTimeDetails_GetSize(UA_ReadAtTimeDetails* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_ReadAtTimeDetails_Encode(UA_MsgBuffer* msgBuf, UA_ReadAtTimeDetails* pValue);

StatusCode UA_ReadAtTimeDetails_Decode(UA_MsgBuffer* msgBuf, UA_ReadAtTimeDetails* pValue);

extern struct UA_EncodeableType UA_ReadAtTimeDetails_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_HistoryData
/*============================================================================
 * The HistoryData structure.
 *===========================================================================*/
typedef struct _UA_HistoryData
{
    int32_t       NoOfDataValues;
    UA_DataValue* DataValues;
}
UA_HistoryData;

void UA_HistoryData_Initialize(UA_HistoryData* pValue);

void UA_HistoryData_Clear(UA_HistoryData* pValue);

//StatusCode UA_HistoryData_GetSize(UA_HistoryData* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_HistoryData_Encode(UA_MsgBuffer* msgBuf, UA_HistoryData* pValue);

StatusCode UA_HistoryData_Decode(UA_MsgBuffer* msgBuf, UA_HistoryData* pValue);

extern struct UA_EncodeableType UA_HistoryData_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_HistoryUpdateType
/*============================================================================
 * The HistoryUpdateType enumeration.
 *===========================================================================*/
typedef enum _UA_HistoryUpdateType
{
    UA_HistoryUpdateType_Insert  = 1,
    UA_HistoryUpdateType_Replace = 2,
    UA_HistoryUpdateType_Update  = 3,
    UA_HistoryUpdateType_Delete  = 4
#if OPCUA_FORCE_INT32_ENUMS
    ,_UA_HistoryUpdateType_MaxEnumerationValue = OpcUa_Int32_Max
#endif
}
UA_HistoryUpdateType;

#endif

#ifndef OPCUA_EXCLUDE_ModificationInfo
/*============================================================================
 * The ModificationInfo structure.
 *===========================================================================*/
typedef struct _UA_ModificationInfo
{
    UA_DateTime          ModificationTime;
    UA_HistoryUpdateType UpdateType;
    UA_String            UserName;
}
UA_ModificationInfo;

void UA_ModificationInfo_Initialize(UA_ModificationInfo* pValue);

void UA_ModificationInfo_Clear(UA_ModificationInfo* pValue);

//StatusCode UA_ModificationInfo_GetSize(UA_ModificationInfo* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_ModificationInfo_Encode(UA_MsgBuffer* msgBuf, UA_ModificationInfo* pValue);

StatusCode UA_ModificationInfo_Decode(UA_MsgBuffer* msgBuf, UA_ModificationInfo* pValue);

extern struct UA_EncodeableType UA_ModificationInfo_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_HistoryModifiedData
/*============================================================================
 * The HistoryModifiedData structure.
 *===========================================================================*/
typedef struct _UA_HistoryModifiedData
{
    int32_t              NoOfDataValues;
    UA_DataValue*        DataValues;
    int32_t              NoOfModificationInfos;
    UA_ModificationInfo* ModificationInfos;
}
UA_HistoryModifiedData;

void UA_HistoryModifiedData_Initialize(UA_HistoryModifiedData* pValue);

void UA_HistoryModifiedData_Clear(UA_HistoryModifiedData* pValue);

//StatusCode UA_HistoryModifiedData_GetSize(UA_HistoryModifiedData* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_HistoryModifiedData_Encode(UA_MsgBuffer* msgBuf, UA_HistoryModifiedData* pValue);

StatusCode UA_HistoryModifiedData_Decode(UA_MsgBuffer* msgBuf, UA_HistoryModifiedData* pValue);

extern struct UA_EncodeableType UA_HistoryModifiedData_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_HistoryEventFieldList
/*============================================================================
 * The HistoryEventFieldList structure.
 *===========================================================================*/
typedef struct _UA_HistoryEventFieldList
{
    int32_t     NoOfEventFields;
    UA_Variant* EventFields;
}
UA_HistoryEventFieldList;

void UA_HistoryEventFieldList_Initialize(UA_HistoryEventFieldList* pValue);

void UA_HistoryEventFieldList_Clear(UA_HistoryEventFieldList* pValue);

//StatusCode UA_HistoryEventFieldList_GetSize(UA_HistoryEventFieldList* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_HistoryEventFieldList_Encode(UA_MsgBuffer* msgBuf, UA_HistoryEventFieldList* pValue);

StatusCode UA_HistoryEventFieldList_Decode(UA_MsgBuffer* msgBuf, UA_HistoryEventFieldList* pValue);

extern struct UA_EncodeableType UA_HistoryEventFieldList_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_HistoryEvent
/*============================================================================
 * The HistoryEvent structure.
 *===========================================================================*/
typedef struct _UA_HistoryEvent
{
    int32_t                   NoOfEvents;
    UA_HistoryEventFieldList* Events;
}
UA_HistoryEvent;

void UA_HistoryEvent_Initialize(UA_HistoryEvent* pValue);

void UA_HistoryEvent_Clear(UA_HistoryEvent* pValue);

//StatusCode UA_HistoryEvent_GetSize(UA_HistoryEvent* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_HistoryEvent_Encode(UA_MsgBuffer* msgBuf, UA_HistoryEvent* pValue);

StatusCode UA_HistoryEvent_Decode(UA_MsgBuffer* msgBuf, UA_HistoryEvent* pValue);

extern struct UA_EncodeableType UA_HistoryEvent_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_HistoryRead
#ifndef OPCUA_EXCLUDE_HistoryReadRequest
/*============================================================================
 * The HistoryReadRequest structure.
 *===========================================================================*/
typedef struct _UA_HistoryReadRequest
{
    UA_RequestHeader       RequestHeader;
    UA_ExtensionObject     HistoryReadDetails;
    UA_TimestampsToReturn  TimestampsToReturn;
    UA_Boolean             ReleaseContinuationPoints;
    int32_t                NoOfNodesToRead;
    UA_HistoryReadValueId* NodesToRead;
}
UA_HistoryReadRequest;

void UA_HistoryReadRequest_Initialize(UA_HistoryReadRequest* pValue);

void UA_HistoryReadRequest_Clear(UA_HistoryReadRequest* pValue);

//StatusCode UA_HistoryReadRequest_GetSize(UA_HistoryReadRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_HistoryReadRequest_Encode(UA_MsgBuffer* msgBuf, UA_HistoryReadRequest* pValue);

StatusCode UA_HistoryReadRequest_Decode(UA_MsgBuffer* msgBuf, UA_HistoryReadRequest* pValue);

extern struct UA_EncodeableType UA_HistoryReadRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_HistoryReadResponse
/*============================================================================
 * The HistoryReadResponse structure.
 *===========================================================================*/
typedef struct _UA_HistoryReadResponse
{
    UA_ResponseHeader     ResponseHeader;
    int32_t               NoOfResults;
    UA_HistoryReadResult* Results;
    int32_t               NoOfDiagnosticInfos;
    UA_DiagnosticInfo*    DiagnosticInfos;
}
UA_HistoryReadResponse;

void UA_HistoryReadResponse_Initialize(UA_HistoryReadResponse* pValue);

void UA_HistoryReadResponse_Clear(UA_HistoryReadResponse* pValue);

//StatusCode UA_HistoryReadResponse_GetSize(UA_HistoryReadResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_HistoryReadResponse_Encode(UA_MsgBuffer* msgBuf, UA_HistoryReadResponse* pValue);

StatusCode UA_HistoryReadResponse_Decode(UA_MsgBuffer* msgBuf, UA_HistoryReadResponse* pValue);

extern struct UA_EncodeableType UA_HistoryReadResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_WriteValue
/*============================================================================
 * The WriteValue structure.
 *===========================================================================*/
typedef struct _UA_WriteValue
{
    UA_NodeId    NodeId;
    uint32_t     AttributeId;
    UA_String    IndexRange;
    UA_DataValue Value;
}
UA_WriteValue;

void UA_WriteValue_Initialize(UA_WriteValue* pValue);

void UA_WriteValue_Clear(UA_WriteValue* pValue);

//StatusCode UA_WriteValue_GetSize(UA_WriteValue* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_WriteValue_Encode(UA_MsgBuffer* msgBuf, UA_WriteValue* pValue);

StatusCode UA_WriteValue_Decode(UA_MsgBuffer* msgBuf, UA_WriteValue* pValue);

extern struct UA_EncodeableType UA_WriteValue_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_Write
#ifndef OPCUA_EXCLUDE_WriteRequest
/*============================================================================
 * The WriteRequest structure.
 *===========================================================================*/
typedef struct _UA_WriteRequest
{
    UA_RequestHeader RequestHeader;
    int32_t          NoOfNodesToWrite;
    UA_WriteValue*   NodesToWrite;
}
UA_WriteRequest;

void UA_WriteRequest_Initialize(UA_WriteRequest* pValue);

void UA_WriteRequest_Clear(UA_WriteRequest* pValue);

//StatusCode UA_WriteRequest_GetSize(UA_WriteRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_WriteRequest_Encode(UA_MsgBuffer* msgBuf, UA_WriteRequest* pValue);

StatusCode UA_WriteRequest_Decode(UA_MsgBuffer* msgBuf, UA_WriteRequest* pValue);

extern struct UA_EncodeableType UA_WriteRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_WriteResponse
/*============================================================================
 * The WriteResponse structure.
 *===========================================================================*/
typedef struct _UA_WriteResponse
{
    UA_ResponseHeader  ResponseHeader;
    int32_t            NoOfResults;
    StatusCode*        Results;
    int32_t            NoOfDiagnosticInfos;
    UA_DiagnosticInfo* DiagnosticInfos;
}
UA_WriteResponse;

void UA_WriteResponse_Initialize(UA_WriteResponse* pValue);

void UA_WriteResponse_Clear(UA_WriteResponse* pValue);

//StatusCode UA_WriteResponse_GetSize(UA_WriteResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_WriteResponse_Encode(UA_MsgBuffer* msgBuf, UA_WriteResponse* pValue);

StatusCode UA_WriteResponse_Decode(UA_MsgBuffer* msgBuf, UA_WriteResponse* pValue);

extern struct UA_EncodeableType UA_WriteResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_HistoryUpdateDetails
/*============================================================================
 * The HistoryUpdateDetails structure.
 *===========================================================================*/
typedef struct _UA_HistoryUpdateDetails
{
    UA_NodeId NodeId;
}
UA_HistoryUpdateDetails;

void UA_HistoryUpdateDetails_Initialize(UA_HistoryUpdateDetails* pValue);

void UA_HistoryUpdateDetails_Clear(UA_HistoryUpdateDetails* pValue);

//StatusCode UA_HistoryUpdateDetails_GetSize(UA_HistoryUpdateDetails* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_HistoryUpdateDetails_Encode(UA_MsgBuffer* msgBuf, UA_HistoryUpdateDetails* pValue);

StatusCode UA_HistoryUpdateDetails_Decode(UA_MsgBuffer* msgBuf, UA_HistoryUpdateDetails* pValue);

extern struct UA_EncodeableType UA_HistoryUpdateDetails_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_PerformUpdateType
/*============================================================================
 * The PerformUpdateType enumeration.
 *===========================================================================*/
typedef enum _UA_PerformUpdateType
{
    UA_PerformUpdateType_Insert  = 1,
    UA_PerformUpdateType_Replace = 2,
    UA_PerformUpdateType_Update  = 3,
    UA_PerformUpdateType_Remove  = 4
#if OPCUA_FORCE_INT32_ENUMS
    ,_UA_PerformUpdateType_MaxEnumerationValue = OpcUa_Int32_Max
#endif
}
UA_PerformUpdateType;

#endif

#ifndef OPCUA_EXCLUDE_UpdateDataDetails
/*============================================================================
 * The UpdateDataDetails structure.
 *===========================================================================*/
typedef struct _UA_UpdateDataDetails
{
    UA_NodeId            NodeId;
    UA_PerformUpdateType PerformInsertReplace;
    int32_t              NoOfUpdateValues;
    UA_DataValue*        UpdateValues;
}
UA_UpdateDataDetails;

void UA_UpdateDataDetails_Initialize(UA_UpdateDataDetails* pValue);

void UA_UpdateDataDetails_Clear(UA_UpdateDataDetails* pValue);

//StatusCode UA_UpdateDataDetails_GetSize(UA_UpdateDataDetails* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_UpdateDataDetails_Encode(UA_MsgBuffer* msgBuf, UA_UpdateDataDetails* pValue);

StatusCode UA_UpdateDataDetails_Decode(UA_MsgBuffer* msgBuf, UA_UpdateDataDetails* pValue);

extern struct UA_EncodeableType UA_UpdateDataDetails_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_UpdateStructureDataDetails
/*============================================================================
 * The UpdateStructureDataDetails structure.
 *===========================================================================*/
typedef struct _UA_UpdateStructureDataDetails
{
    UA_NodeId            NodeId;
    UA_PerformUpdateType PerformInsertReplace;
    int32_t              NoOfUpdateValues;
    UA_DataValue*        UpdateValues;
}
UA_UpdateStructureDataDetails;

void UA_UpdateStructureDataDetails_Initialize(UA_UpdateStructureDataDetails* pValue);

void UA_UpdateStructureDataDetails_Clear(UA_UpdateStructureDataDetails* pValue);

//StatusCode UA_UpdateStructureDataDetails_GetSize(UA_UpdateStructureDataDetails* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_UpdateStructureDataDetails_Encode(UA_MsgBuffer* msgBuf, UA_UpdateStructureDataDetails* pValue);

StatusCode UA_UpdateStructureDataDetails_Decode(UA_MsgBuffer* msgBuf, UA_UpdateStructureDataDetails* pValue);

extern struct UA_EncodeableType UA_UpdateStructureDataDetails_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_UpdateEventDetails
/*============================================================================
 * The UpdateEventDetails structure.
 *===========================================================================*/
typedef struct _UA_UpdateEventDetails
{
    UA_NodeId                 NodeId;
    UA_PerformUpdateType      PerformInsertReplace;
    UA_EventFilter            Filter;
    int32_t                   NoOfEventData;
    UA_HistoryEventFieldList* EventData;
}
UA_UpdateEventDetails;

void UA_UpdateEventDetails_Initialize(UA_UpdateEventDetails* pValue);

void UA_UpdateEventDetails_Clear(UA_UpdateEventDetails* pValue);

//StatusCode UA_UpdateEventDetails_GetSize(UA_UpdateEventDetails* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_UpdateEventDetails_Encode(UA_MsgBuffer* msgBuf, UA_UpdateEventDetails* pValue);

StatusCode UA_UpdateEventDetails_Decode(UA_MsgBuffer* msgBuf, UA_UpdateEventDetails* pValue);

extern struct UA_EncodeableType UA_UpdateEventDetails_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_DeleteRawModifiedDetails
/*============================================================================
 * The DeleteRawModifiedDetails structure.
 *===========================================================================*/
typedef struct _UA_DeleteRawModifiedDetails
{
    UA_NodeId   NodeId;
    UA_Boolean  IsDeleteModified;
    UA_DateTime StartTime;
    UA_DateTime EndTime;
}
UA_DeleteRawModifiedDetails;

void UA_DeleteRawModifiedDetails_Initialize(UA_DeleteRawModifiedDetails* pValue);

void UA_DeleteRawModifiedDetails_Clear(UA_DeleteRawModifiedDetails* pValue);

//StatusCode UA_DeleteRawModifiedDetails_GetSize(UA_DeleteRawModifiedDetails* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_DeleteRawModifiedDetails_Encode(UA_MsgBuffer* msgBuf, UA_DeleteRawModifiedDetails* pValue);

StatusCode UA_DeleteRawModifiedDetails_Decode(UA_MsgBuffer* msgBuf, UA_DeleteRawModifiedDetails* pValue);

extern struct UA_EncodeableType UA_DeleteRawModifiedDetails_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_DeleteAtTimeDetails
/*============================================================================
 * The DeleteAtTimeDetails structure.
 *===========================================================================*/
typedef struct _UA_DeleteAtTimeDetails
{
    UA_NodeId    NodeId;
    int32_t      NoOfReqTimes;
    UA_DateTime* ReqTimes;
}
UA_DeleteAtTimeDetails;

void UA_DeleteAtTimeDetails_Initialize(UA_DeleteAtTimeDetails* pValue);

void UA_DeleteAtTimeDetails_Clear(UA_DeleteAtTimeDetails* pValue);

//StatusCode UA_DeleteAtTimeDetails_GetSize(UA_DeleteAtTimeDetails* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_DeleteAtTimeDetails_Encode(UA_MsgBuffer* msgBuf, UA_DeleteAtTimeDetails* pValue);

StatusCode UA_DeleteAtTimeDetails_Decode(UA_MsgBuffer* msgBuf, UA_DeleteAtTimeDetails* pValue);

extern struct UA_EncodeableType UA_DeleteAtTimeDetails_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_DeleteEventDetails
/*============================================================================
 * The DeleteEventDetails structure.
 *===========================================================================*/
typedef struct _UA_DeleteEventDetails
{
    UA_NodeId      NodeId;
    int32_t        NoOfEventIds;
    UA_ByteString* EventIds;
}
UA_DeleteEventDetails;

void UA_DeleteEventDetails_Initialize(UA_DeleteEventDetails* pValue);

void UA_DeleteEventDetails_Clear(UA_DeleteEventDetails* pValue);

//StatusCode UA_DeleteEventDetails_GetSize(UA_DeleteEventDetails* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_DeleteEventDetails_Encode(UA_MsgBuffer* msgBuf, UA_DeleteEventDetails* pValue);

StatusCode UA_DeleteEventDetails_Decode(UA_MsgBuffer* msgBuf, UA_DeleteEventDetails* pValue);

extern struct UA_EncodeableType UA_DeleteEventDetails_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_HistoryUpdateResult
/*============================================================================
 * The HistoryUpdateResult structure.
 *===========================================================================*/
typedef struct _UA_HistoryUpdateResult
{
    StatusCode         StatusCode;
    int32_t            NoOfOperationResults;
    StatusCode*        OperationResults;
    int32_t            NoOfDiagnosticInfos;
    UA_DiagnosticInfo* DiagnosticInfos;
}
UA_HistoryUpdateResult;

void UA_HistoryUpdateResult_Initialize(UA_HistoryUpdateResult* pValue);

void UA_HistoryUpdateResult_Clear(UA_HistoryUpdateResult* pValue);

//StatusCode UA_HistoryUpdateResult_GetSize(UA_HistoryUpdateResult* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_HistoryUpdateResult_Encode(UA_MsgBuffer* msgBuf, UA_HistoryUpdateResult* pValue);

StatusCode UA_HistoryUpdateResult_Decode(UA_MsgBuffer* msgBuf, UA_HistoryUpdateResult* pValue);

extern struct UA_EncodeableType UA_HistoryUpdateResult_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_HistoryUpdate
#ifndef OPCUA_EXCLUDE_HistoryUpdateRequest
/*============================================================================
 * The HistoryUpdateRequest structure.
 *===========================================================================*/
typedef struct _UA_HistoryUpdateRequest
{
    UA_RequestHeader    RequestHeader;
    int32_t             NoOfHistoryUpdateDetails;
    UA_ExtensionObject* HistoryUpdateDetails;
}
UA_HistoryUpdateRequest;

void UA_HistoryUpdateRequest_Initialize(UA_HistoryUpdateRequest* pValue);

void UA_HistoryUpdateRequest_Clear(UA_HistoryUpdateRequest* pValue);

//StatusCode UA_HistoryUpdateRequest_GetSize(UA_HistoryUpdateRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_HistoryUpdateRequest_Encode(UA_MsgBuffer* msgBuf, UA_HistoryUpdateRequest* pValue);

StatusCode UA_HistoryUpdateRequest_Decode(UA_MsgBuffer* msgBuf, UA_HistoryUpdateRequest* pValue);

extern struct UA_EncodeableType UA_HistoryUpdateRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_HistoryUpdateResponse
/*============================================================================
 * The HistoryUpdateResponse structure.
 *===========================================================================*/
typedef struct _UA_HistoryUpdateResponse
{
    UA_ResponseHeader       ResponseHeader;
    int32_t                 NoOfResults;
    UA_HistoryUpdateResult* Results;
    int32_t                 NoOfDiagnosticInfos;
    UA_DiagnosticInfo*      DiagnosticInfos;
}
UA_HistoryUpdateResponse;

void UA_HistoryUpdateResponse_Initialize(UA_HistoryUpdateResponse* pValue);

void UA_HistoryUpdateResponse_Clear(UA_HistoryUpdateResponse* pValue);

//StatusCode UA_HistoryUpdateResponse_GetSize(UA_HistoryUpdateResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_HistoryUpdateResponse_Encode(UA_MsgBuffer* msgBuf, UA_HistoryUpdateResponse* pValue);

StatusCode UA_HistoryUpdateResponse_Decode(UA_MsgBuffer* msgBuf, UA_HistoryUpdateResponse* pValue);

extern struct UA_EncodeableType UA_HistoryUpdateResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_CallMethodRequest
/*============================================================================
 * The CallMethodRequest structure.
 *===========================================================================*/
typedef struct _UA_CallMethodRequest
{
    UA_NodeId   ObjectId;
    UA_NodeId   MethodId;
    int32_t     NoOfInputArguments;
    UA_Variant* InputArguments;
}
UA_CallMethodRequest;

void UA_CallMethodRequest_Initialize(UA_CallMethodRequest* pValue);

void UA_CallMethodRequest_Clear(UA_CallMethodRequest* pValue);

//StatusCode UA_CallMethodRequest_GetSize(UA_CallMethodRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_CallMethodRequest_Encode(UA_MsgBuffer* msgBuf, UA_CallMethodRequest* pValue);

StatusCode UA_CallMethodRequest_Decode(UA_MsgBuffer* msgBuf, UA_CallMethodRequest* pValue);

extern struct UA_EncodeableType UA_CallMethodRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_CallMethodResult
/*============================================================================
 * The CallMethodResult structure.
 *===========================================================================*/
typedef struct _UA_CallMethodResult
{
    StatusCode         StatusCode;
    int32_t            NoOfInputArgumentResults;
    StatusCode*        InputArgumentResults;
    int32_t            NoOfInputArgumentDiagnosticInfos;
    UA_DiagnosticInfo* InputArgumentDiagnosticInfos;
    int32_t            NoOfOutputArguments;
    UA_Variant*        OutputArguments;
}
UA_CallMethodResult;

void UA_CallMethodResult_Initialize(UA_CallMethodResult* pValue);

void UA_CallMethodResult_Clear(UA_CallMethodResult* pValue);

//StatusCode UA_CallMethodResult_GetSize(UA_CallMethodResult* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_CallMethodResult_Encode(UA_MsgBuffer* msgBuf, UA_CallMethodResult* pValue);

StatusCode UA_CallMethodResult_Decode(UA_MsgBuffer* msgBuf, UA_CallMethodResult* pValue);

extern struct UA_EncodeableType UA_CallMethodResult_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_Call
#ifndef OPCUA_EXCLUDE_CallRequest
/*============================================================================
 * The CallRequest structure.
 *===========================================================================*/
typedef struct _UA_CallRequest
{
    UA_RequestHeader      RequestHeader;
    int32_t               NoOfMethodsToCall;
    UA_CallMethodRequest* MethodsToCall;
}
UA_CallRequest;

void UA_CallRequest_Initialize(UA_CallRequest* pValue);

void UA_CallRequest_Clear(UA_CallRequest* pValue);

//StatusCode UA_CallRequest_GetSize(UA_CallRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_CallRequest_Encode(UA_MsgBuffer* msgBuf, UA_CallRequest* pValue);

StatusCode UA_CallRequest_Decode(UA_MsgBuffer* msgBuf, UA_CallRequest* pValue);

extern struct UA_EncodeableType UA_CallRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_CallResponse
/*============================================================================
 * The CallResponse structure.
 *===========================================================================*/
typedef struct _UA_CallResponse
{
    UA_ResponseHeader    ResponseHeader;
    int32_t              NoOfResults;
    UA_CallMethodResult* Results;
    int32_t              NoOfDiagnosticInfos;
    UA_DiagnosticInfo*   DiagnosticInfos;
}
UA_CallResponse;

void UA_CallResponse_Initialize(UA_CallResponse* pValue);

void UA_CallResponse_Clear(UA_CallResponse* pValue);

//StatusCode UA_CallResponse_GetSize(UA_CallResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_CallResponse_Encode(UA_MsgBuffer* msgBuf, UA_CallResponse* pValue);

StatusCode UA_CallResponse_Decode(UA_MsgBuffer* msgBuf, UA_CallResponse* pValue);

extern struct UA_EncodeableType UA_CallResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_MonitoringMode
/*============================================================================
 * The MonitoringMode enumeration.
 *===========================================================================*/
typedef enum _UA_MonitoringMode
{
    UA_MonitoringMode_Disabled  = 0,
    UA_MonitoringMode_Sampling  = 1,
    UA_MonitoringMode_Reporting = 2
#if OPCUA_FORCE_INT32_ENUMS
    ,_UA_MonitoringMode_MaxEnumerationValue = OpcUa_Int32_Max
#endif
}
UA_MonitoringMode;

#endif

#ifndef OPCUA_EXCLUDE_DataChangeTrigger
/*============================================================================
 * The DataChangeTrigger enumeration.
 *===========================================================================*/
typedef enum _UA_DataChangeTrigger
{
    UA_DataChangeTrigger_Status               = 0,
    UA_DataChangeTrigger_StatusValue          = 1,
    UA_DataChangeTrigger_StatusValueTimestamp = 2
#if OPCUA_FORCE_INT32_ENUMS
    ,_UA_DataChangeTrigger_MaxEnumerationValue = OpcUa_Int32_Max
#endif
}
UA_DataChangeTrigger;

#endif

#ifndef OPCUA_EXCLUDE_DeadbandType
/*============================================================================
 * The DeadbandType enumeration.
 *===========================================================================*/
typedef enum _UA_DeadbandType
{
    UA_DeadbandType_None     = 0,
    UA_DeadbandType_Absolute = 1,
    UA_DeadbandType_Percent  = 2
#if OPCUA_FORCE_INT32_ENUMS
    ,_UA_DeadbandType_MaxEnumerationValue = OpcUa_Int32_Max
#endif
}
UA_DeadbandType;

#endif

#ifndef OPCUA_EXCLUDE_DataChangeFilter
/*============================================================================
 * The DataChangeFilter structure.
 *===========================================================================*/
typedef struct _UA_DataChangeFilter
{
    UA_DataChangeTrigger Trigger;
    uint32_t             DeadbandType;
    double               DeadbandValue;
}
UA_DataChangeFilter;

void UA_DataChangeFilter_Initialize(UA_DataChangeFilter* pValue);

void UA_DataChangeFilter_Clear(UA_DataChangeFilter* pValue);

//StatusCode UA_DataChangeFilter_GetSize(UA_DataChangeFilter* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_DataChangeFilter_Encode(UA_MsgBuffer* msgBuf, UA_DataChangeFilter* pValue);

StatusCode UA_DataChangeFilter_Decode(UA_MsgBuffer* msgBuf, UA_DataChangeFilter* pValue);

extern struct UA_EncodeableType UA_DataChangeFilter_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_AggregateFilter
/*============================================================================
 * The AggregateFilter structure.
 *===========================================================================*/
typedef struct _UA_AggregateFilter
{
    UA_DateTime               StartTime;
    UA_NodeId                 AggregateType;
    double                    ProcessingInterval;
    UA_AggregateConfiguration AggregateConfiguration;
}
UA_AggregateFilter;

void UA_AggregateFilter_Initialize(UA_AggregateFilter* pValue);

void UA_AggregateFilter_Clear(UA_AggregateFilter* pValue);

//StatusCode UA_AggregateFilter_GetSize(UA_AggregateFilter* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_AggregateFilter_Encode(UA_MsgBuffer* msgBuf, UA_AggregateFilter* pValue);

StatusCode UA_AggregateFilter_Decode(UA_MsgBuffer* msgBuf, UA_AggregateFilter* pValue);

extern struct UA_EncodeableType UA_AggregateFilter_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_EventFilterResult
/*============================================================================
 * The EventFilterResult structure.
 *===========================================================================*/
typedef struct _UA_EventFilterResult
{
    int32_t                NoOfSelectClauseResults;
    StatusCode*            SelectClauseResults;
    int32_t                NoOfSelectClauseDiagnosticInfos;
    UA_DiagnosticInfo*     SelectClauseDiagnosticInfos;
    UA_ContentFilterResult WhereClauseResult;
}
UA_EventFilterResult;

void UA_EventFilterResult_Initialize(UA_EventFilterResult* pValue);

void UA_EventFilterResult_Clear(UA_EventFilterResult* pValue);

//StatusCode UA_EventFilterResult_GetSize(UA_EventFilterResult* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_EventFilterResult_Encode(UA_MsgBuffer* msgBuf, UA_EventFilterResult* pValue);

StatusCode UA_EventFilterResult_Decode(UA_MsgBuffer* msgBuf, UA_EventFilterResult* pValue);

extern struct UA_EncodeableType UA_EventFilterResult_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_AggregateFilterResult
/*============================================================================
 * The AggregateFilterResult structure.
 *===========================================================================*/
typedef struct _UA_AggregateFilterResult
{
    UA_DateTime               RevisedStartTime;
    double                    RevisedProcessingInterval;
    UA_AggregateConfiguration RevisedAggregateConfiguration;
}
UA_AggregateFilterResult;

void UA_AggregateFilterResult_Initialize(UA_AggregateFilterResult* pValue);

void UA_AggregateFilterResult_Clear(UA_AggregateFilterResult* pValue);

//StatusCode UA_AggregateFilterResult_GetSize(UA_AggregateFilterResult* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_AggregateFilterResult_Encode(UA_MsgBuffer* msgBuf, UA_AggregateFilterResult* pValue);

StatusCode UA_AggregateFilterResult_Decode(UA_MsgBuffer* msgBuf, UA_AggregateFilterResult* pValue);

extern struct UA_EncodeableType UA_AggregateFilterResult_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_MonitoringParameters
/*============================================================================
 * The MonitoringParameters structure.
 *===========================================================================*/
typedef struct _UA_MonitoringParameters
{
    uint32_t           ClientHandle;
    double             SamplingInterval;
    UA_ExtensionObject Filter;
    uint32_t           QueueSize;
    UA_Boolean         DiscardOldest;
}
UA_MonitoringParameters;

void UA_MonitoringParameters_Initialize(UA_MonitoringParameters* pValue);

void UA_MonitoringParameters_Clear(UA_MonitoringParameters* pValue);

//StatusCode UA_MonitoringParameters_GetSize(UA_MonitoringParameters* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_MonitoringParameters_Encode(UA_MsgBuffer* msgBuf, UA_MonitoringParameters* pValue);

StatusCode UA_MonitoringParameters_Decode(UA_MsgBuffer* msgBuf, UA_MonitoringParameters* pValue);

extern struct UA_EncodeableType UA_MonitoringParameters_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_MonitoredItemCreateRequest
/*============================================================================
 * The MonitoredItemCreateRequest structure.
 *===========================================================================*/
typedef struct _UA_MonitoredItemCreateRequest
{
    UA_ReadValueId          ItemToMonitor;
    UA_MonitoringMode       MonitoringMode;
    UA_MonitoringParameters RequestedParameters;
}
UA_MonitoredItemCreateRequest;

void UA_MonitoredItemCreateRequest_Initialize(UA_MonitoredItemCreateRequest* pValue);

void UA_MonitoredItemCreateRequest_Clear(UA_MonitoredItemCreateRequest* pValue);

//StatusCode UA_MonitoredItemCreateRequest_GetSize(UA_MonitoredItemCreateRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_MonitoredItemCreateRequest_Encode(UA_MsgBuffer* msgBuf, UA_MonitoredItemCreateRequest* pValue);

StatusCode UA_MonitoredItemCreateRequest_Decode(UA_MsgBuffer* msgBuf, UA_MonitoredItemCreateRequest* pValue);

extern struct UA_EncodeableType UA_MonitoredItemCreateRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_MonitoredItemCreateResult
/*============================================================================
 * The MonitoredItemCreateResult structure.
 *===========================================================================*/
typedef struct _UA_MonitoredItemCreateResult
{
    StatusCode         StatusCode;
    uint32_t           MonitoredItemId;
    double             RevisedSamplingInterval;
    uint32_t           RevisedQueueSize;
    UA_ExtensionObject FilterResult;
}
UA_MonitoredItemCreateResult;

void UA_MonitoredItemCreateResult_Initialize(UA_MonitoredItemCreateResult* pValue);

void UA_MonitoredItemCreateResult_Clear(UA_MonitoredItemCreateResult* pValue);

//StatusCode UA_MonitoredItemCreateResult_GetSize(UA_MonitoredItemCreateResult* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_MonitoredItemCreateResult_Encode(UA_MsgBuffer* msgBuf, UA_MonitoredItemCreateResult* pValue);

StatusCode UA_MonitoredItemCreateResult_Decode(UA_MsgBuffer* msgBuf, UA_MonitoredItemCreateResult* pValue);

extern struct UA_EncodeableType UA_MonitoredItemCreateResult_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_CreateMonitoredItems
#ifndef OPCUA_EXCLUDE_CreateMonitoredItemsRequest
/*============================================================================
 * The CreateMonitoredItemsRequest structure.
 *===========================================================================*/
typedef struct _UA_CreateMonitoredItemsRequest
{
    UA_RequestHeader               RequestHeader;
    uint32_t                       SubscriptionId;
    UA_TimestampsToReturn          TimestampsToReturn;
    int32_t                        NoOfItemsToCreate;
    UA_MonitoredItemCreateRequest* ItemsToCreate;
}
UA_CreateMonitoredItemsRequest;

void UA_CreateMonitoredItemsRequest_Initialize(UA_CreateMonitoredItemsRequest* pValue);

void UA_CreateMonitoredItemsRequest_Clear(UA_CreateMonitoredItemsRequest* pValue);

//StatusCode UA_CreateMonitoredItemsRequest_GetSize(UA_CreateMonitoredItemsRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_CreateMonitoredItemsRequest_Encode(UA_MsgBuffer* msgBuf, UA_CreateMonitoredItemsRequest* pValue);

StatusCode UA_CreateMonitoredItemsRequest_Decode(UA_MsgBuffer* msgBuf, UA_CreateMonitoredItemsRequest* pValue);

extern struct UA_EncodeableType UA_CreateMonitoredItemsRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_CreateMonitoredItemsResponse
/*============================================================================
 * The CreateMonitoredItemsResponse structure.
 *===========================================================================*/
typedef struct _UA_CreateMonitoredItemsResponse
{
    UA_ResponseHeader             ResponseHeader;
    int32_t                       NoOfResults;
    UA_MonitoredItemCreateResult* Results;
    int32_t                       NoOfDiagnosticInfos;
    UA_DiagnosticInfo*            DiagnosticInfos;
}
UA_CreateMonitoredItemsResponse;

void UA_CreateMonitoredItemsResponse_Initialize(UA_CreateMonitoredItemsResponse* pValue);

void UA_CreateMonitoredItemsResponse_Clear(UA_CreateMonitoredItemsResponse* pValue);

//StatusCode UA_CreateMonitoredItemsResponse_GetSize(UA_CreateMonitoredItemsResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_CreateMonitoredItemsResponse_Encode(UA_MsgBuffer* msgBuf, UA_CreateMonitoredItemsResponse* pValue);

StatusCode UA_CreateMonitoredItemsResponse_Decode(UA_MsgBuffer* msgBuf, UA_CreateMonitoredItemsResponse* pValue);

extern struct UA_EncodeableType UA_CreateMonitoredItemsResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_MonitoredItemModifyRequest
/*============================================================================
 * The MonitoredItemModifyRequest structure.
 *===========================================================================*/
typedef struct _UA_MonitoredItemModifyRequest
{
    uint32_t                MonitoredItemId;
    UA_MonitoringParameters RequestedParameters;
}
UA_MonitoredItemModifyRequest;

void UA_MonitoredItemModifyRequest_Initialize(UA_MonitoredItemModifyRequest* pValue);

void UA_MonitoredItemModifyRequest_Clear(UA_MonitoredItemModifyRequest* pValue);

//StatusCode UA_MonitoredItemModifyRequest_GetSize(UA_MonitoredItemModifyRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_MonitoredItemModifyRequest_Encode(UA_MsgBuffer* msgBuf, UA_MonitoredItemModifyRequest* pValue);

StatusCode UA_MonitoredItemModifyRequest_Decode(UA_MsgBuffer* msgBuf, UA_MonitoredItemModifyRequest* pValue);

extern struct UA_EncodeableType UA_MonitoredItemModifyRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_MonitoredItemModifyResult
/*============================================================================
 * The MonitoredItemModifyResult structure.
 *===========================================================================*/
typedef struct _UA_MonitoredItemModifyResult
{
    StatusCode         StatusCode;
    double             RevisedSamplingInterval;
    uint32_t           RevisedQueueSize;
    UA_ExtensionObject FilterResult;
}
UA_MonitoredItemModifyResult;

void UA_MonitoredItemModifyResult_Initialize(UA_MonitoredItemModifyResult* pValue);

void UA_MonitoredItemModifyResult_Clear(UA_MonitoredItemModifyResult* pValue);

//StatusCode UA_MonitoredItemModifyResult_GetSize(UA_MonitoredItemModifyResult* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_MonitoredItemModifyResult_Encode(UA_MsgBuffer* msgBuf, UA_MonitoredItemModifyResult* pValue);

StatusCode UA_MonitoredItemModifyResult_Decode(UA_MsgBuffer* msgBuf, UA_MonitoredItemModifyResult* pValue);

extern struct UA_EncodeableType UA_MonitoredItemModifyResult_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ModifyMonitoredItems
#ifndef OPCUA_EXCLUDE_ModifyMonitoredItemsRequest
/*============================================================================
 * The ModifyMonitoredItemsRequest structure.
 *===========================================================================*/
typedef struct _UA_ModifyMonitoredItemsRequest
{
    UA_RequestHeader               RequestHeader;
    uint32_t                       SubscriptionId;
    UA_TimestampsToReturn          TimestampsToReturn;
    int32_t                        NoOfItemsToModify;
    UA_MonitoredItemModifyRequest* ItemsToModify;
}
UA_ModifyMonitoredItemsRequest;

void UA_ModifyMonitoredItemsRequest_Initialize(UA_ModifyMonitoredItemsRequest* pValue);

void UA_ModifyMonitoredItemsRequest_Clear(UA_ModifyMonitoredItemsRequest* pValue);

//StatusCode UA_ModifyMonitoredItemsRequest_GetSize(UA_ModifyMonitoredItemsRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_ModifyMonitoredItemsRequest_Encode(UA_MsgBuffer* msgBuf, UA_ModifyMonitoredItemsRequest* pValue);

StatusCode UA_ModifyMonitoredItemsRequest_Decode(UA_MsgBuffer* msgBuf, UA_ModifyMonitoredItemsRequest* pValue);

extern struct UA_EncodeableType UA_ModifyMonitoredItemsRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ModifyMonitoredItemsResponse
/*============================================================================
 * The ModifyMonitoredItemsResponse structure.
 *===========================================================================*/
typedef struct _UA_ModifyMonitoredItemsResponse
{
    UA_ResponseHeader             ResponseHeader;
    int32_t                       NoOfResults;
    UA_MonitoredItemModifyResult* Results;
    int32_t                       NoOfDiagnosticInfos;
    UA_DiagnosticInfo*            DiagnosticInfos;
}
UA_ModifyMonitoredItemsResponse;

void UA_ModifyMonitoredItemsResponse_Initialize(UA_ModifyMonitoredItemsResponse* pValue);

void UA_ModifyMonitoredItemsResponse_Clear(UA_ModifyMonitoredItemsResponse* pValue);

//StatusCode UA_ModifyMonitoredItemsResponse_GetSize(UA_ModifyMonitoredItemsResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_ModifyMonitoredItemsResponse_Encode(UA_MsgBuffer* msgBuf, UA_ModifyMonitoredItemsResponse* pValue);

StatusCode UA_ModifyMonitoredItemsResponse_Decode(UA_MsgBuffer* msgBuf, UA_ModifyMonitoredItemsResponse* pValue);

extern struct UA_EncodeableType UA_ModifyMonitoredItemsResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_SetMonitoringMode
#ifndef OPCUA_EXCLUDE_SetMonitoringModeRequest
/*============================================================================
 * The SetMonitoringModeRequest structure.
 *===========================================================================*/
typedef struct _UA_SetMonitoringModeRequest
{
    UA_RequestHeader  RequestHeader;
    uint32_t          SubscriptionId;
    UA_MonitoringMode MonitoringMode;
    int32_t           NoOfMonitoredItemIds;
    uint32_t*         MonitoredItemIds;
}
UA_SetMonitoringModeRequest;

void UA_SetMonitoringModeRequest_Initialize(UA_SetMonitoringModeRequest* pValue);

void UA_SetMonitoringModeRequest_Clear(UA_SetMonitoringModeRequest* pValue);

//StatusCode UA_SetMonitoringModeRequest_GetSize(UA_SetMonitoringModeRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_SetMonitoringModeRequest_Encode(UA_MsgBuffer* msgBuf, UA_SetMonitoringModeRequest* pValue);

StatusCode UA_SetMonitoringModeRequest_Decode(UA_MsgBuffer* msgBuf, UA_SetMonitoringModeRequest* pValue);

extern struct UA_EncodeableType UA_SetMonitoringModeRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_SetMonitoringModeResponse
/*============================================================================
 * The SetMonitoringModeResponse structure.
 *===========================================================================*/
typedef struct _UA_SetMonitoringModeResponse
{
    UA_ResponseHeader  ResponseHeader;
    int32_t            NoOfResults;
    StatusCode*        Results;
    int32_t            NoOfDiagnosticInfos;
    UA_DiagnosticInfo* DiagnosticInfos;
}
UA_SetMonitoringModeResponse;

void UA_SetMonitoringModeResponse_Initialize(UA_SetMonitoringModeResponse* pValue);

void UA_SetMonitoringModeResponse_Clear(UA_SetMonitoringModeResponse* pValue);

//StatusCode UA_SetMonitoringModeResponse_GetSize(UA_SetMonitoringModeResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_SetMonitoringModeResponse_Encode(UA_MsgBuffer* msgBuf, UA_SetMonitoringModeResponse* pValue);

StatusCode UA_SetMonitoringModeResponse_Decode(UA_MsgBuffer* msgBuf, UA_SetMonitoringModeResponse* pValue);

extern struct UA_EncodeableType UA_SetMonitoringModeResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_SetTriggering
#ifndef OPCUA_EXCLUDE_SetTriggeringRequest
/*============================================================================
 * The SetTriggeringRequest structure.
 *===========================================================================*/
typedef struct _UA_SetTriggeringRequest
{
    UA_RequestHeader RequestHeader;
    uint32_t         SubscriptionId;
    uint32_t         TriggeringItemId;
    int32_t          NoOfLinksToAdd;
    uint32_t*        LinksToAdd;
    int32_t          NoOfLinksToRemove;
    uint32_t*        LinksToRemove;
}
UA_SetTriggeringRequest;

void UA_SetTriggeringRequest_Initialize(UA_SetTriggeringRequest* pValue);

void UA_SetTriggeringRequest_Clear(UA_SetTriggeringRequest* pValue);

//StatusCode UA_SetTriggeringRequest_GetSize(UA_SetTriggeringRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_SetTriggeringRequest_Encode(UA_MsgBuffer* msgBuf, UA_SetTriggeringRequest* pValue);

StatusCode UA_SetTriggeringRequest_Decode(UA_MsgBuffer* msgBuf, UA_SetTriggeringRequest* pValue);

extern struct UA_EncodeableType UA_SetTriggeringRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_SetTriggeringResponse
/*============================================================================
 * The SetTriggeringResponse structure.
 *===========================================================================*/
typedef struct _UA_SetTriggeringResponse
{
    UA_ResponseHeader  ResponseHeader;
    int32_t            NoOfAddResults;
    StatusCode*        AddResults;
    int32_t            NoOfAddDiagnosticInfos;
    UA_DiagnosticInfo* AddDiagnosticInfos;
    int32_t            NoOfRemoveResults;
    StatusCode*        RemoveResults;
    int32_t            NoOfRemoveDiagnosticInfos;
    UA_DiagnosticInfo* RemoveDiagnosticInfos;
}
UA_SetTriggeringResponse;

void UA_SetTriggeringResponse_Initialize(UA_SetTriggeringResponse* pValue);

void UA_SetTriggeringResponse_Clear(UA_SetTriggeringResponse* pValue);

//StatusCode UA_SetTriggeringResponse_GetSize(UA_SetTriggeringResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_SetTriggeringResponse_Encode(UA_MsgBuffer* msgBuf, UA_SetTriggeringResponse* pValue);

StatusCode UA_SetTriggeringResponse_Decode(UA_MsgBuffer* msgBuf, UA_SetTriggeringResponse* pValue);

extern struct UA_EncodeableType UA_SetTriggeringResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_DeleteMonitoredItems
#ifndef OPCUA_EXCLUDE_DeleteMonitoredItemsRequest
/*============================================================================
 * The DeleteMonitoredItemsRequest structure.
 *===========================================================================*/
typedef struct _UA_DeleteMonitoredItemsRequest
{
    UA_RequestHeader RequestHeader;
    uint32_t         SubscriptionId;
    int32_t          NoOfMonitoredItemIds;
    uint32_t*        MonitoredItemIds;
}
UA_DeleteMonitoredItemsRequest;

void UA_DeleteMonitoredItemsRequest_Initialize(UA_DeleteMonitoredItemsRequest* pValue);

void UA_DeleteMonitoredItemsRequest_Clear(UA_DeleteMonitoredItemsRequest* pValue);

//StatusCode UA_DeleteMonitoredItemsRequest_GetSize(UA_DeleteMonitoredItemsRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_DeleteMonitoredItemsRequest_Encode(UA_MsgBuffer* msgBuf, UA_DeleteMonitoredItemsRequest* pValue);

StatusCode UA_DeleteMonitoredItemsRequest_Decode(UA_MsgBuffer* msgBuf, UA_DeleteMonitoredItemsRequest* pValue);

extern struct UA_EncodeableType UA_DeleteMonitoredItemsRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_DeleteMonitoredItemsResponse
/*============================================================================
 * The DeleteMonitoredItemsResponse structure.
 *===========================================================================*/
typedef struct _UA_DeleteMonitoredItemsResponse
{
    UA_ResponseHeader  ResponseHeader;
    int32_t            NoOfResults;
    StatusCode*        Results;
    int32_t            NoOfDiagnosticInfos;
    UA_DiagnosticInfo* DiagnosticInfos;
}
UA_DeleteMonitoredItemsResponse;

void UA_DeleteMonitoredItemsResponse_Initialize(UA_DeleteMonitoredItemsResponse* pValue);

void UA_DeleteMonitoredItemsResponse_Clear(UA_DeleteMonitoredItemsResponse* pValue);

//StatusCode UA_DeleteMonitoredItemsResponse_GetSize(UA_DeleteMonitoredItemsResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_DeleteMonitoredItemsResponse_Encode(UA_MsgBuffer* msgBuf, UA_DeleteMonitoredItemsResponse* pValue);

StatusCode UA_DeleteMonitoredItemsResponse_Decode(UA_MsgBuffer* msgBuf, UA_DeleteMonitoredItemsResponse* pValue);

extern struct UA_EncodeableType UA_DeleteMonitoredItemsResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_CreateSubscription
#ifndef OPCUA_EXCLUDE_CreateSubscriptionRequest
/*============================================================================
 * The CreateSubscriptionRequest structure.
 *===========================================================================*/
typedef struct _UA_CreateSubscriptionRequest
{
    UA_RequestHeader RequestHeader;
    double           RequestedPublishingInterval;
    uint32_t         RequestedLifetimeCount;
    uint32_t         RequestedMaxKeepAliveCount;
    uint32_t         MaxNotificationsPerPublish;
    UA_Boolean       PublishingEnabled;
    UA_Byte          Priority;
}
UA_CreateSubscriptionRequest;

void UA_CreateSubscriptionRequest_Initialize(UA_CreateSubscriptionRequest* pValue);

void UA_CreateSubscriptionRequest_Clear(UA_CreateSubscriptionRequest* pValue);

//StatusCode UA_CreateSubscriptionRequest_GetSize(UA_CreateSubscriptionRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_CreateSubscriptionRequest_Encode(UA_MsgBuffer* msgBuf, UA_CreateSubscriptionRequest* pValue);

StatusCode UA_CreateSubscriptionRequest_Decode(UA_MsgBuffer* msgBuf, UA_CreateSubscriptionRequest* pValue);

extern struct UA_EncodeableType UA_CreateSubscriptionRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_CreateSubscriptionResponse
/*============================================================================
 * The CreateSubscriptionResponse structure.
 *===========================================================================*/
typedef struct _UA_CreateSubscriptionResponse
{
    UA_ResponseHeader ResponseHeader;
    uint32_t          SubscriptionId;
    double            RevisedPublishingInterval;
    uint32_t          RevisedLifetimeCount;
    uint32_t          RevisedMaxKeepAliveCount;
}
UA_CreateSubscriptionResponse;

void UA_CreateSubscriptionResponse_Initialize(UA_CreateSubscriptionResponse* pValue);

void UA_CreateSubscriptionResponse_Clear(UA_CreateSubscriptionResponse* pValue);

//StatusCode UA_CreateSubscriptionResponse_GetSize(UA_CreateSubscriptionResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_CreateSubscriptionResponse_Encode(UA_MsgBuffer* msgBuf, UA_CreateSubscriptionResponse* pValue);

StatusCode UA_CreateSubscriptionResponse_Decode(UA_MsgBuffer* msgBuf, UA_CreateSubscriptionResponse* pValue);

extern struct UA_EncodeableType UA_CreateSubscriptionResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_ModifySubscription
#ifndef OPCUA_EXCLUDE_ModifySubscriptionRequest
/*============================================================================
 * The ModifySubscriptionRequest structure.
 *===========================================================================*/
typedef struct _UA_ModifySubscriptionRequest
{
    UA_RequestHeader RequestHeader;
    uint32_t         SubscriptionId;
    double           RequestedPublishingInterval;
    uint32_t         RequestedLifetimeCount;
    uint32_t         RequestedMaxKeepAliveCount;
    uint32_t         MaxNotificationsPerPublish;
    UA_Byte          Priority;
}
UA_ModifySubscriptionRequest;

void UA_ModifySubscriptionRequest_Initialize(UA_ModifySubscriptionRequest* pValue);

void UA_ModifySubscriptionRequest_Clear(UA_ModifySubscriptionRequest* pValue);

//StatusCode UA_ModifySubscriptionRequest_GetSize(UA_ModifySubscriptionRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_ModifySubscriptionRequest_Encode(UA_MsgBuffer* msgBuf, UA_ModifySubscriptionRequest* pValue);

StatusCode UA_ModifySubscriptionRequest_Decode(UA_MsgBuffer* msgBuf, UA_ModifySubscriptionRequest* pValue);

extern struct UA_EncodeableType UA_ModifySubscriptionRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ModifySubscriptionResponse
/*============================================================================
 * The ModifySubscriptionResponse structure.
 *===========================================================================*/
typedef struct _UA_ModifySubscriptionResponse
{
    UA_ResponseHeader ResponseHeader;
    double            RevisedPublishingInterval;
    uint32_t          RevisedLifetimeCount;
    uint32_t          RevisedMaxKeepAliveCount;
}
UA_ModifySubscriptionResponse;

void UA_ModifySubscriptionResponse_Initialize(UA_ModifySubscriptionResponse* pValue);

void UA_ModifySubscriptionResponse_Clear(UA_ModifySubscriptionResponse* pValue);

//StatusCode UA_ModifySubscriptionResponse_GetSize(UA_ModifySubscriptionResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_ModifySubscriptionResponse_Encode(UA_MsgBuffer* msgBuf, UA_ModifySubscriptionResponse* pValue);

StatusCode UA_ModifySubscriptionResponse_Decode(UA_MsgBuffer* msgBuf, UA_ModifySubscriptionResponse* pValue);

extern struct UA_EncodeableType UA_ModifySubscriptionResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_SetPublishingMode
#ifndef OPCUA_EXCLUDE_SetPublishingModeRequest
/*============================================================================
 * The SetPublishingModeRequest structure.
 *===========================================================================*/
typedef struct _UA_SetPublishingModeRequest
{
    UA_RequestHeader RequestHeader;
    UA_Boolean       PublishingEnabled;
    int32_t          NoOfSubscriptionIds;
    uint32_t*        SubscriptionIds;
}
UA_SetPublishingModeRequest;

void UA_SetPublishingModeRequest_Initialize(UA_SetPublishingModeRequest* pValue);

void UA_SetPublishingModeRequest_Clear(UA_SetPublishingModeRequest* pValue);

//StatusCode UA_SetPublishingModeRequest_GetSize(UA_SetPublishingModeRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_SetPublishingModeRequest_Encode(UA_MsgBuffer* msgBuf, UA_SetPublishingModeRequest* pValue);

StatusCode UA_SetPublishingModeRequest_Decode(UA_MsgBuffer* msgBuf, UA_SetPublishingModeRequest* pValue);

extern struct UA_EncodeableType UA_SetPublishingModeRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_SetPublishingModeResponse
/*============================================================================
 * The SetPublishingModeResponse structure.
 *===========================================================================*/
typedef struct _UA_SetPublishingModeResponse
{
    UA_ResponseHeader  ResponseHeader;
    int32_t            NoOfResults;
    StatusCode*        Results;
    int32_t            NoOfDiagnosticInfos;
    UA_DiagnosticInfo* DiagnosticInfos;
}
UA_SetPublishingModeResponse;

void UA_SetPublishingModeResponse_Initialize(UA_SetPublishingModeResponse* pValue);

void UA_SetPublishingModeResponse_Clear(UA_SetPublishingModeResponse* pValue);

//StatusCode UA_SetPublishingModeResponse_GetSize(UA_SetPublishingModeResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_SetPublishingModeResponse_Encode(UA_MsgBuffer* msgBuf, UA_SetPublishingModeResponse* pValue);

StatusCode UA_SetPublishingModeResponse_Decode(UA_MsgBuffer* msgBuf, UA_SetPublishingModeResponse* pValue);

extern struct UA_EncodeableType UA_SetPublishingModeResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_NotificationMessage
/*============================================================================
 * The NotificationMessage structure.
 *===========================================================================*/
typedef struct _UA_NotificationMessage
{
    uint32_t            SequenceNumber;
    UA_DateTime         PublishTime;
    int32_t             NoOfNotificationData;
    UA_ExtensionObject* NotificationData;
}
UA_NotificationMessage;

void UA_NotificationMessage_Initialize(UA_NotificationMessage* pValue);

void UA_NotificationMessage_Clear(UA_NotificationMessage* pValue);

//StatusCode UA_NotificationMessage_GetSize(UA_NotificationMessage* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_NotificationMessage_Encode(UA_MsgBuffer* msgBuf, UA_NotificationMessage* pValue);

StatusCode UA_NotificationMessage_Decode(UA_MsgBuffer* msgBuf, UA_NotificationMessage* pValue);

extern struct UA_EncodeableType UA_NotificationMessage_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_MonitoredItemNotification
/*============================================================================
 * The MonitoredItemNotification structure.
 *===========================================================================*/
typedef struct _UA_MonitoredItemNotification
{
    uint32_t     ClientHandle;
    UA_DataValue Value;
}
UA_MonitoredItemNotification;

void UA_MonitoredItemNotification_Initialize(UA_MonitoredItemNotification* pValue);

void UA_MonitoredItemNotification_Clear(UA_MonitoredItemNotification* pValue);

//StatusCode UA_MonitoredItemNotification_GetSize(UA_MonitoredItemNotification* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_MonitoredItemNotification_Encode(UA_MsgBuffer* msgBuf, UA_MonitoredItemNotification* pValue);

StatusCode UA_MonitoredItemNotification_Decode(UA_MsgBuffer* msgBuf, UA_MonitoredItemNotification* pValue);

extern struct UA_EncodeableType UA_MonitoredItemNotification_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_DataChangeNotification
/*============================================================================
 * The DataChangeNotification structure.
 *===========================================================================*/
typedef struct _UA_DataChangeNotification
{
    int32_t                       NoOfMonitoredItems;
    UA_MonitoredItemNotification* MonitoredItems;
    int32_t                       NoOfDiagnosticInfos;
    UA_DiagnosticInfo*            DiagnosticInfos;
}
UA_DataChangeNotification;

void UA_DataChangeNotification_Initialize(UA_DataChangeNotification* pValue);

void UA_DataChangeNotification_Clear(UA_DataChangeNotification* pValue);

//StatusCode UA_DataChangeNotification_GetSize(UA_DataChangeNotification* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_DataChangeNotification_Encode(UA_MsgBuffer* msgBuf, UA_DataChangeNotification* pValue);

StatusCode UA_DataChangeNotification_Decode(UA_MsgBuffer* msgBuf, UA_DataChangeNotification* pValue);

extern struct UA_EncodeableType UA_DataChangeNotification_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_EventFieldList
/*============================================================================
 * The EventFieldList structure.
 *===========================================================================*/
typedef struct _UA_EventFieldList
{
    uint32_t    ClientHandle;
    int32_t     NoOfEventFields;
    UA_Variant* EventFields;
}
UA_EventFieldList;

void UA_EventFieldList_Initialize(UA_EventFieldList* pValue);

void UA_EventFieldList_Clear(UA_EventFieldList* pValue);

//StatusCode UA_EventFieldList_GetSize(UA_EventFieldList* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_EventFieldList_Encode(UA_MsgBuffer* msgBuf, UA_EventFieldList* pValue);

StatusCode UA_EventFieldList_Decode(UA_MsgBuffer* msgBuf, UA_EventFieldList* pValue);

extern struct UA_EncodeableType UA_EventFieldList_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_EventNotificationList
/*============================================================================
 * The EventNotificationList structure.
 *===========================================================================*/
typedef struct _UA_EventNotificationList
{
    int32_t            NoOfEvents;
    UA_EventFieldList* Events;
}
UA_EventNotificationList;

void UA_EventNotificationList_Initialize(UA_EventNotificationList* pValue);

void UA_EventNotificationList_Clear(UA_EventNotificationList* pValue);

//StatusCode UA_EventNotificationList_GetSize(UA_EventNotificationList* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_EventNotificationList_Encode(UA_MsgBuffer* msgBuf, UA_EventNotificationList* pValue);

StatusCode UA_EventNotificationList_Decode(UA_MsgBuffer* msgBuf, UA_EventNotificationList* pValue);

extern struct UA_EncodeableType UA_EventNotificationList_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_StatusChangeNotification
/*============================================================================
 * The StatusChangeNotification structure.
 *===========================================================================*/
typedef struct _UA_StatusChangeNotification
{
    StatusCode        Status;
    UA_DiagnosticInfo DiagnosticInfo;
}
UA_StatusChangeNotification;

void UA_StatusChangeNotification_Initialize(UA_StatusChangeNotification* pValue);

void UA_StatusChangeNotification_Clear(UA_StatusChangeNotification* pValue);

//StatusCode UA_StatusChangeNotification_GetSize(UA_StatusChangeNotification* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_StatusChangeNotification_Encode(UA_MsgBuffer* msgBuf, UA_StatusChangeNotification* pValue);

StatusCode UA_StatusChangeNotification_Decode(UA_MsgBuffer* msgBuf, UA_StatusChangeNotification* pValue);

extern struct UA_EncodeableType UA_StatusChangeNotification_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_SubscriptionAcknowledgement
/*============================================================================
 * The SubscriptionAcknowledgement structure.
 *===========================================================================*/
typedef struct _UA_SubscriptionAcknowledgement
{
    uint32_t SubscriptionId;
    uint32_t SequenceNumber;
}
UA_SubscriptionAcknowledgement;

void UA_SubscriptionAcknowledgement_Initialize(UA_SubscriptionAcknowledgement* pValue);

void UA_SubscriptionAcknowledgement_Clear(UA_SubscriptionAcknowledgement* pValue);

//StatusCode UA_SubscriptionAcknowledgement_GetSize(UA_SubscriptionAcknowledgement* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_SubscriptionAcknowledgement_Encode(UA_MsgBuffer* msgBuf, UA_SubscriptionAcknowledgement* pValue);

StatusCode UA_SubscriptionAcknowledgement_Decode(UA_MsgBuffer* msgBuf, UA_SubscriptionAcknowledgement* pValue);

extern struct UA_EncodeableType UA_SubscriptionAcknowledgement_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_Publish
#ifndef OPCUA_EXCLUDE_PublishRequest
/*============================================================================
 * The PublishRequest structure.
 *===========================================================================*/
typedef struct _UA_PublishRequest
{
    UA_RequestHeader                RequestHeader;
    int32_t                         NoOfSubscriptionAcknowledgements;
    UA_SubscriptionAcknowledgement* SubscriptionAcknowledgements;
}
UA_PublishRequest;

void UA_PublishRequest_Initialize(UA_PublishRequest* pValue);

void UA_PublishRequest_Clear(UA_PublishRequest* pValue);

//StatusCode UA_PublishRequest_GetSize(UA_PublishRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_PublishRequest_Encode(UA_MsgBuffer* msgBuf, UA_PublishRequest* pValue);

StatusCode UA_PublishRequest_Decode(UA_MsgBuffer* msgBuf, UA_PublishRequest* pValue);

extern struct UA_EncodeableType UA_PublishRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_PublishResponse
/*============================================================================
 * The PublishResponse structure.
 *===========================================================================*/
typedef struct _UA_PublishResponse
{
    UA_ResponseHeader      ResponseHeader;
    uint32_t               SubscriptionId;
    int32_t                NoOfAvailableSequenceNumbers;
    uint32_t*              AvailableSequenceNumbers;
    UA_Boolean             MoreNotifications;
    UA_NotificationMessage NotificationMessage;
    int32_t                NoOfResults;
    StatusCode*            Results;
    int32_t                NoOfDiagnosticInfos;
    UA_DiagnosticInfo*     DiagnosticInfos;
}
UA_PublishResponse;

void UA_PublishResponse_Initialize(UA_PublishResponse* pValue);

void UA_PublishResponse_Clear(UA_PublishResponse* pValue);

//StatusCode UA_PublishResponse_GetSize(UA_PublishResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_PublishResponse_Encode(UA_MsgBuffer* msgBuf, UA_PublishResponse* pValue);

StatusCode UA_PublishResponse_Decode(UA_MsgBuffer* msgBuf, UA_PublishResponse* pValue);

extern struct UA_EncodeableType UA_PublishResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_Republish
#ifndef OPCUA_EXCLUDE_RepublishRequest
/*============================================================================
 * The RepublishRequest structure.
 *===========================================================================*/
typedef struct _UA_RepublishRequest
{
    UA_RequestHeader RequestHeader;
    uint32_t         SubscriptionId;
    uint32_t         RetransmitSequenceNumber;
}
UA_RepublishRequest;

void UA_RepublishRequest_Initialize(UA_RepublishRequest* pValue);

void UA_RepublishRequest_Clear(UA_RepublishRequest* pValue);

//StatusCode UA_RepublishRequest_GetSize(UA_RepublishRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_RepublishRequest_Encode(UA_MsgBuffer* msgBuf, UA_RepublishRequest* pValue);

StatusCode UA_RepublishRequest_Decode(UA_MsgBuffer* msgBuf, UA_RepublishRequest* pValue);

extern struct UA_EncodeableType UA_RepublishRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_RepublishResponse
/*============================================================================
 * The RepublishResponse structure.
 *===========================================================================*/
typedef struct _UA_RepublishResponse
{
    UA_ResponseHeader      ResponseHeader;
    UA_NotificationMessage NotificationMessage;
}
UA_RepublishResponse;

void UA_RepublishResponse_Initialize(UA_RepublishResponse* pValue);

void UA_RepublishResponse_Clear(UA_RepublishResponse* pValue);

//StatusCode UA_RepublishResponse_GetSize(UA_RepublishResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_RepublishResponse_Encode(UA_MsgBuffer* msgBuf, UA_RepublishResponse* pValue);

StatusCode UA_RepublishResponse_Decode(UA_MsgBuffer* msgBuf, UA_RepublishResponse* pValue);

extern struct UA_EncodeableType UA_RepublishResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_TransferResult
/*============================================================================
 * The TransferResult structure.
 *===========================================================================*/
typedef struct _UA_TransferResult
{
    StatusCode StatusCode;
    int32_t    NoOfAvailableSequenceNumbers;
    uint32_t*  AvailableSequenceNumbers;
}
UA_TransferResult;

void UA_TransferResult_Initialize(UA_TransferResult* pValue);

void UA_TransferResult_Clear(UA_TransferResult* pValue);

//StatusCode UA_TransferResult_GetSize(UA_TransferResult* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_TransferResult_Encode(UA_MsgBuffer* msgBuf, UA_TransferResult* pValue);

StatusCode UA_TransferResult_Decode(UA_MsgBuffer* msgBuf, UA_TransferResult* pValue);

extern struct UA_EncodeableType UA_TransferResult_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_TransferSubscriptions
#ifndef OPCUA_EXCLUDE_TransferSubscriptionsRequest
/*============================================================================
 * The TransferSubscriptionsRequest structure.
 *===========================================================================*/
typedef struct _UA_TransferSubscriptionsRequest
{
    UA_RequestHeader RequestHeader;
    int32_t          NoOfSubscriptionIds;
    uint32_t*        SubscriptionIds;
    UA_Boolean       SendInitialValues;
}
UA_TransferSubscriptionsRequest;

void UA_TransferSubscriptionsRequest_Initialize(UA_TransferSubscriptionsRequest* pValue);

void UA_TransferSubscriptionsRequest_Clear(UA_TransferSubscriptionsRequest* pValue);

//StatusCode UA_TransferSubscriptionsRequest_GetSize(UA_TransferSubscriptionsRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_TransferSubscriptionsRequest_Encode(UA_MsgBuffer* msgBuf, UA_TransferSubscriptionsRequest* pValue);

StatusCode UA_TransferSubscriptionsRequest_Decode(UA_MsgBuffer* msgBuf, UA_TransferSubscriptionsRequest* pValue);

extern struct UA_EncodeableType UA_TransferSubscriptionsRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_TransferSubscriptionsResponse
/*============================================================================
 * The TransferSubscriptionsResponse structure.
 *===========================================================================*/
typedef struct _UA_TransferSubscriptionsResponse
{
    UA_ResponseHeader  ResponseHeader;
    int32_t            NoOfResults;
    UA_TransferResult* Results;
    int32_t            NoOfDiagnosticInfos;
    UA_DiagnosticInfo* DiagnosticInfos;
}
UA_TransferSubscriptionsResponse;

void UA_TransferSubscriptionsResponse_Initialize(UA_TransferSubscriptionsResponse* pValue);

void UA_TransferSubscriptionsResponse_Clear(UA_TransferSubscriptionsResponse* pValue);

//StatusCode UA_TransferSubscriptionsResponse_GetSize(UA_TransferSubscriptionsResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_TransferSubscriptionsResponse_Encode(UA_MsgBuffer* msgBuf, UA_TransferSubscriptionsResponse* pValue);

StatusCode UA_TransferSubscriptionsResponse_Decode(UA_MsgBuffer* msgBuf, UA_TransferSubscriptionsResponse* pValue);

extern struct UA_EncodeableType UA_TransferSubscriptionsResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_DeleteSubscriptions
#ifndef OPCUA_EXCLUDE_DeleteSubscriptionsRequest
/*============================================================================
 * The DeleteSubscriptionsRequest structure.
 *===========================================================================*/
typedef struct _UA_DeleteSubscriptionsRequest
{
    UA_RequestHeader RequestHeader;
    int32_t          NoOfSubscriptionIds;
    uint32_t*        SubscriptionIds;
}
UA_DeleteSubscriptionsRequest;

void UA_DeleteSubscriptionsRequest_Initialize(UA_DeleteSubscriptionsRequest* pValue);

void UA_DeleteSubscriptionsRequest_Clear(UA_DeleteSubscriptionsRequest* pValue);

//StatusCode UA_DeleteSubscriptionsRequest_GetSize(UA_DeleteSubscriptionsRequest* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_DeleteSubscriptionsRequest_Encode(UA_MsgBuffer* msgBuf, UA_DeleteSubscriptionsRequest* pValue);

StatusCode UA_DeleteSubscriptionsRequest_Decode(UA_MsgBuffer* msgBuf, UA_DeleteSubscriptionsRequest* pValue);

extern struct UA_EncodeableType UA_DeleteSubscriptionsRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_DeleteSubscriptionsResponse
/*============================================================================
 * The DeleteSubscriptionsResponse structure.
 *===========================================================================*/
typedef struct _UA_DeleteSubscriptionsResponse
{
    UA_ResponseHeader  ResponseHeader;
    int32_t            NoOfResults;
    StatusCode*        Results;
    int32_t            NoOfDiagnosticInfos;
    UA_DiagnosticInfo* DiagnosticInfos;
}
UA_DeleteSubscriptionsResponse;

void UA_DeleteSubscriptionsResponse_Initialize(UA_DeleteSubscriptionsResponse* pValue);

void UA_DeleteSubscriptionsResponse_Clear(UA_DeleteSubscriptionsResponse* pValue);

//StatusCode UA_DeleteSubscriptionsResponse_GetSize(UA_DeleteSubscriptionsResponse* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_DeleteSubscriptionsResponse_Encode(UA_MsgBuffer* msgBuf, UA_DeleteSubscriptionsResponse* pValue);

StatusCode UA_DeleteSubscriptionsResponse_Decode(UA_MsgBuffer* msgBuf, UA_DeleteSubscriptionsResponse* pValue);

extern struct UA_EncodeableType UA_DeleteSubscriptionsResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_EnumeratedTestType
/*============================================================================
 * The EnumeratedTestType enumeration.
 *===========================================================================*/
typedef enum _UA_EnumeratedTestType
{
    UA_EnumeratedTestType_Red    = 1,
    UA_EnumeratedTestType_Yellow = 4,
    UA_EnumeratedTestType_Green  = 5
#if OPCUA_FORCE_INT32_ENUMS
    ,_UA_EnumeratedTestType_MaxEnumerationValue = OpcUa_Int32_Max
#endif
}
UA_EnumeratedTestType;

#endif

#ifndef OPCUA_EXCLUDE_BuildInfo
/*============================================================================
 * The BuildInfo structure.
 *===========================================================================*/
typedef struct _UA_BuildInfo
{
    UA_String   ProductUri;
    UA_String   ManufacturerName;
    UA_String   ProductName;
    UA_String   SoftwareVersion;
    UA_String   BuildNumber;
    UA_DateTime BuildDate;
}
UA_BuildInfo;

void UA_BuildInfo_Initialize(UA_BuildInfo* pValue);

void UA_BuildInfo_Clear(UA_BuildInfo* pValue);

//StatusCode UA_BuildInfo_GetSize(UA_BuildInfo* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_BuildInfo_Encode(UA_MsgBuffer* msgBuf, UA_BuildInfo* pValue);

StatusCode UA_BuildInfo_Decode(UA_MsgBuffer* msgBuf, UA_BuildInfo* pValue);

extern struct UA_EncodeableType UA_BuildInfo_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_RedundancySupport
/*============================================================================
 * The RedundancySupport enumeration.
 *===========================================================================*/
typedef enum _UA_RedundancySupport
{
    UA_RedundancySupport_None           = 0,
    UA_RedundancySupport_Cold           = 1,
    UA_RedundancySupport_Warm           = 2,
    UA_RedundancySupport_Hot            = 3,
    UA_RedundancySupport_Transparent    = 4,
    UA_RedundancySupport_HotAndMirrored = 5
#if OPCUA_FORCE_INT32_ENUMS
    ,_UA_RedundancySupport_MaxEnumerationValue = OpcUa_Int32_Max
#endif
}
UA_RedundancySupport;

#endif

#ifndef OPCUA_EXCLUDE_ServerState
/*============================================================================
 * The ServerState enumeration.
 *===========================================================================*/
typedef enum _UA_ServerState
{
    UA_ServerState_Running            = 0,
    UA_ServerState_Failed             = 1,
    UA_ServerState_NoConfiguration    = 2,
    UA_ServerState_Suspended          = 3,
    UA_ServerState_Shutdown           = 4,
    UA_ServerState_Test               = 5,
    UA_ServerState_CommunicationFault = 6,
    UA_ServerState_Unknown            = 7
#if OPCUA_FORCE_INT32_ENUMS
    ,_UA_ServerState_MaxEnumerationValue = OpcUa_Int32_Max
#endif
}
UA_ServerState;

#endif

#ifndef OPCUA_EXCLUDE_RedundantServerDataType
/*============================================================================
 * The RedundantServerDataType structure.
 *===========================================================================*/
typedef struct _UA_RedundantServerDataType
{
    UA_String      ServerId;
    UA_Byte        ServiceLevel;
    UA_ServerState ServerState;
}
UA_RedundantServerDataType;

void UA_RedundantServerDataType_Initialize(UA_RedundantServerDataType* pValue);

void UA_RedundantServerDataType_Clear(UA_RedundantServerDataType* pValue);

//StatusCode UA_RedundantServerDataType_GetSize(UA_RedundantServerDataType* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_RedundantServerDataType_Encode(UA_MsgBuffer* msgBuf, UA_RedundantServerDataType* pValue);

StatusCode UA_RedundantServerDataType_Decode(UA_MsgBuffer* msgBuf, UA_RedundantServerDataType* pValue);

extern struct UA_EncodeableType UA_RedundantServerDataType_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_EndpointUrlListDataType
/*============================================================================
 * The EndpointUrlListDataType structure.
 *===========================================================================*/
typedef struct _UA_EndpointUrlListDataType
{
    int32_t    NoOfEndpointUrlList;
    UA_String* EndpointUrlList;
}
UA_EndpointUrlListDataType;

void UA_EndpointUrlListDataType_Initialize(UA_EndpointUrlListDataType* pValue);

void UA_EndpointUrlListDataType_Clear(UA_EndpointUrlListDataType* pValue);

//StatusCode UA_EndpointUrlListDataType_GetSize(UA_EndpointUrlListDataType* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_EndpointUrlListDataType_Encode(UA_MsgBuffer* msgBuf, UA_EndpointUrlListDataType* pValue);

StatusCode UA_EndpointUrlListDataType_Decode(UA_MsgBuffer* msgBuf, UA_EndpointUrlListDataType* pValue);

extern struct UA_EncodeableType UA_EndpointUrlListDataType_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_NetworkGroupDataType
/*============================================================================
 * The NetworkGroupDataType structure.
 *===========================================================================*/
typedef struct _UA_NetworkGroupDataType
{
    UA_String                   ServerUri;
    int32_t                     NoOfNetworkPaths;
    UA_EndpointUrlListDataType* NetworkPaths;
}
UA_NetworkGroupDataType;

void UA_NetworkGroupDataType_Initialize(UA_NetworkGroupDataType* pValue);

void UA_NetworkGroupDataType_Clear(UA_NetworkGroupDataType* pValue);

//StatusCode UA_NetworkGroupDataType_GetSize(UA_NetworkGroupDataType* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_NetworkGroupDataType_Encode(UA_MsgBuffer* msgBuf, UA_NetworkGroupDataType* pValue);

StatusCode UA_NetworkGroupDataType_Decode(UA_MsgBuffer* msgBuf, UA_NetworkGroupDataType* pValue);

extern struct UA_EncodeableType UA_NetworkGroupDataType_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_SamplingIntervalDiagnosticsDataType
/*============================================================================
 * The SamplingIntervalDiagnosticsDataType structure.
 *===========================================================================*/
typedef struct _UA_SamplingIntervalDiagnosticsDataType
{
    double   SamplingInterval;
    uint32_t MonitoredItemCount;
    uint32_t MaxMonitoredItemCount;
    uint32_t DisabledMonitoredItemCount;
}
UA_SamplingIntervalDiagnosticsDataType;

void UA_SamplingIntervalDiagnosticsDataType_Initialize(UA_SamplingIntervalDiagnosticsDataType* pValue);

void UA_SamplingIntervalDiagnosticsDataType_Clear(UA_SamplingIntervalDiagnosticsDataType* pValue);

//StatusCode UA_SamplingIntervalDiagnosticsDataType_GetSize(UA_SamplingIntervalDiagnosticsDataType* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_SamplingIntervalDiagnosticsDataType_Encode(UA_MsgBuffer* msgBuf, UA_SamplingIntervalDiagnosticsDataType* pValue);

StatusCode UA_SamplingIntervalDiagnosticsDataType_Decode(UA_MsgBuffer* msgBuf, UA_SamplingIntervalDiagnosticsDataType* pValue);

extern struct UA_EncodeableType UA_SamplingIntervalDiagnosticsDataType_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ServerDiagnosticsSummaryDataType
/*============================================================================
 * The ServerDiagnosticsSummaryDataType structure.
 *===========================================================================*/
typedef struct _UA_ServerDiagnosticsSummaryDataType
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
UA_ServerDiagnosticsSummaryDataType;

void UA_ServerDiagnosticsSummaryDataType_Initialize(UA_ServerDiagnosticsSummaryDataType* pValue);

void UA_ServerDiagnosticsSummaryDataType_Clear(UA_ServerDiagnosticsSummaryDataType* pValue);

//StatusCode UA_ServerDiagnosticsSummaryDataType_GetSize(UA_ServerDiagnosticsSummaryDataType* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_ServerDiagnosticsSummaryDataType_Encode(UA_MsgBuffer* msgBuf, UA_ServerDiagnosticsSummaryDataType* pValue);

StatusCode UA_ServerDiagnosticsSummaryDataType_Decode(UA_MsgBuffer* msgBuf, UA_ServerDiagnosticsSummaryDataType* pValue);

extern struct UA_EncodeableType UA_ServerDiagnosticsSummaryDataType_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ServerStatusDataType
/*============================================================================
 * The ServerStatusDataType structure.
 *===========================================================================*/
typedef struct _UA_ServerStatusDataType
{
    UA_DateTime      StartTime;
    UA_DateTime      CurrentTime;
    UA_ServerState   State;
    UA_BuildInfo     BuildInfo;
    uint32_t         SecondsTillShutdown;
    UA_LocalizedText ShutdownReason;
}
UA_ServerStatusDataType;

void UA_ServerStatusDataType_Initialize(UA_ServerStatusDataType* pValue);

void UA_ServerStatusDataType_Clear(UA_ServerStatusDataType* pValue);

//StatusCode UA_ServerStatusDataType_GetSize(UA_ServerStatusDataType* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_ServerStatusDataType_Encode(UA_MsgBuffer* msgBuf, UA_ServerStatusDataType* pValue);

StatusCode UA_ServerStatusDataType_Decode(UA_MsgBuffer* msgBuf, UA_ServerStatusDataType* pValue);

extern struct UA_EncodeableType UA_ServerStatusDataType_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ServiceCounterDataType
/*============================================================================
 * The ServiceCounterDataType structure.
 *===========================================================================*/
typedef struct _UA_ServiceCounterDataType
{
    uint32_t TotalCount;
    uint32_t ErrorCount;
}
UA_ServiceCounterDataType;

void UA_ServiceCounterDataType_Initialize(UA_ServiceCounterDataType* pValue);

void UA_ServiceCounterDataType_Clear(UA_ServiceCounterDataType* pValue);

//StatusCode UA_ServiceCounterDataType_GetSize(UA_ServiceCounterDataType* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_ServiceCounterDataType_Encode(UA_MsgBuffer* msgBuf, UA_ServiceCounterDataType* pValue);

StatusCode UA_ServiceCounterDataType_Decode(UA_MsgBuffer* msgBuf, UA_ServiceCounterDataType* pValue);

extern struct UA_EncodeableType UA_ServiceCounterDataType_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_SessionDiagnosticsDataType
/*============================================================================
 * The SessionDiagnosticsDataType structure.
 *===========================================================================*/
typedef struct _UA_SessionDiagnosticsDataType
{
    UA_NodeId                 SessionId;
    UA_String                 SessionName;
    UA_ApplicationDescription ClientDescription;
    UA_String                 ServerUri;
    UA_String                 EndpointUrl;
    int32_t                   NoOfLocaleIds;
    UA_String*                LocaleIds;
    double                    ActualSessionTimeout;
    uint32_t                  MaxResponseMessageSize;
    UA_DateTime               ClientConnectionTime;
    UA_DateTime               ClientLastContactTime;
    uint32_t                  CurrentSubscriptionsCount;
    uint32_t                  CurrentMonitoredItemsCount;
    uint32_t                  CurrentPublishRequestsInQueue;
    UA_ServiceCounterDataType TotalRequestCount;
    uint32_t                  UnauthorizedRequestCount;
    UA_ServiceCounterDataType ReadCount;
    UA_ServiceCounterDataType HistoryReadCount;
    UA_ServiceCounterDataType WriteCount;
    UA_ServiceCounterDataType HistoryUpdateCount;
    UA_ServiceCounterDataType CallCount;
    UA_ServiceCounterDataType CreateMonitoredItemsCount;
    UA_ServiceCounterDataType ModifyMonitoredItemsCount;
    UA_ServiceCounterDataType SetMonitoringModeCount;
    UA_ServiceCounterDataType SetTriggeringCount;
    UA_ServiceCounterDataType DeleteMonitoredItemsCount;
    UA_ServiceCounterDataType CreateSubscriptionCount;
    UA_ServiceCounterDataType ModifySubscriptionCount;
    UA_ServiceCounterDataType SetPublishingModeCount;
    UA_ServiceCounterDataType PublishCount;
    UA_ServiceCounterDataType RepublishCount;
    UA_ServiceCounterDataType TransferSubscriptionsCount;
    UA_ServiceCounterDataType DeleteSubscriptionsCount;
    UA_ServiceCounterDataType AddNodesCount;
    UA_ServiceCounterDataType AddReferencesCount;
    UA_ServiceCounterDataType DeleteNodesCount;
    UA_ServiceCounterDataType DeleteReferencesCount;
    UA_ServiceCounterDataType BrowseCount;
    UA_ServiceCounterDataType BrowseNextCount;
    UA_ServiceCounterDataType TranslateBrowsePathsToNodeIdsCount;
    UA_ServiceCounterDataType QueryFirstCount;
    UA_ServiceCounterDataType QueryNextCount;
    UA_ServiceCounterDataType RegisterNodesCount;
    UA_ServiceCounterDataType UnregisterNodesCount;
}
UA_SessionDiagnosticsDataType;

void UA_SessionDiagnosticsDataType_Initialize(UA_SessionDiagnosticsDataType* pValue);

void UA_SessionDiagnosticsDataType_Clear(UA_SessionDiagnosticsDataType* pValue);

//StatusCode UA_SessionDiagnosticsDataType_GetSize(UA_SessionDiagnosticsDataType* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_SessionDiagnosticsDataType_Encode(UA_MsgBuffer* msgBuf, UA_SessionDiagnosticsDataType* pValue);

StatusCode UA_SessionDiagnosticsDataType_Decode(UA_MsgBuffer* msgBuf, UA_SessionDiagnosticsDataType* pValue);

extern struct UA_EncodeableType UA_SessionDiagnosticsDataType_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_SessionSecurityDiagnosticsDataType
/*============================================================================
 * The SessionSecurityDiagnosticsDataType structure.
 *===========================================================================*/
typedef struct _UA_SessionSecurityDiagnosticsDataType
{
    UA_NodeId              SessionId;
    UA_String              ClientUserIdOfSession;
    int32_t                NoOfClientUserIdHistory;
    UA_String*             ClientUserIdHistory;
    UA_String              AuthenticationMechanism;
    UA_String              Encoding;
    UA_String              TransportProtocol;
    UA_MessageSecurityMode SecurityMode;
    UA_String              SecurityPolicyUri;
    UA_ByteString          ClientCertificate;
}
UA_SessionSecurityDiagnosticsDataType;

void UA_SessionSecurityDiagnosticsDataType_Initialize(UA_SessionSecurityDiagnosticsDataType* pValue);

void UA_SessionSecurityDiagnosticsDataType_Clear(UA_SessionSecurityDiagnosticsDataType* pValue);

//StatusCode UA_SessionSecurityDiagnosticsDataType_GetSize(UA_SessionSecurityDiagnosticsDataType* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_SessionSecurityDiagnosticsDataType_Encode(UA_MsgBuffer* msgBuf, UA_SessionSecurityDiagnosticsDataType* pValue);

StatusCode UA_SessionSecurityDiagnosticsDataType_Decode(UA_MsgBuffer* msgBuf, UA_SessionSecurityDiagnosticsDataType* pValue);

extern struct UA_EncodeableType UA_SessionSecurityDiagnosticsDataType_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_StatusResult
/*============================================================================
 * The StatusResult structure.
 *===========================================================================*/
typedef struct _UA_StatusResult
{
    StatusCode        StatusCode;
    UA_DiagnosticInfo DiagnosticInfo;
}
UA_StatusResult;

void UA_StatusResult_Initialize(UA_StatusResult* pValue);

void UA_StatusResult_Clear(UA_StatusResult* pValue);

//StatusCode UA_StatusResult_GetSize(UA_StatusResult* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_StatusResult_Encode(UA_MsgBuffer* msgBuf, UA_StatusResult* pValue);

StatusCode UA_StatusResult_Decode(UA_MsgBuffer* msgBuf, UA_StatusResult* pValue);

extern struct UA_EncodeableType UA_StatusResult_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_SubscriptionDiagnosticsDataType
/*============================================================================
 * The SubscriptionDiagnosticsDataType structure.
 *===========================================================================*/
typedef struct _UA_SubscriptionDiagnosticsDataType
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
UA_SubscriptionDiagnosticsDataType;

void UA_SubscriptionDiagnosticsDataType_Initialize(UA_SubscriptionDiagnosticsDataType* pValue);

void UA_SubscriptionDiagnosticsDataType_Clear(UA_SubscriptionDiagnosticsDataType* pValue);

//StatusCode UA_SubscriptionDiagnosticsDataType_GetSize(UA_SubscriptionDiagnosticsDataType* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_SubscriptionDiagnosticsDataType_Encode(UA_MsgBuffer* msgBuf, UA_SubscriptionDiagnosticsDataType* pValue);

StatusCode UA_SubscriptionDiagnosticsDataType_Decode(UA_MsgBuffer* msgBuf, UA_SubscriptionDiagnosticsDataType* pValue);

extern struct UA_EncodeableType UA_SubscriptionDiagnosticsDataType_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ModelChangeStructureVerbMask
/*============================================================================
 * The ModelChangeStructureVerbMask enumeration.
 *===========================================================================*/
typedef enum _UA_ModelChangeStructureVerbMask
{
    UA_ModelChangeStructureVerbMask_NodeAdded        = 1,
    UA_ModelChangeStructureVerbMask_NodeDeleted      = 2,
    UA_ModelChangeStructureVerbMask_ReferenceAdded   = 4,
    UA_ModelChangeStructureVerbMask_ReferenceDeleted = 8,
    UA_ModelChangeStructureVerbMask_DataTypeChanged  = 16
#if OPCUA_FORCE_INT32_ENUMS
    ,_UA_ModelChangeStructureVerbMask_MaxEnumerationValue = OpcUa_Int32_Max
#endif
}
UA_ModelChangeStructureVerbMask;

#endif

#ifndef OPCUA_EXCLUDE_ModelChangeStructureDataType
/*============================================================================
 * The ModelChangeStructureDataType structure.
 *===========================================================================*/
typedef struct _UA_ModelChangeStructureDataType
{
    UA_NodeId Affected;
    UA_NodeId AffectedType;
    UA_Byte   Verb;
}
UA_ModelChangeStructureDataType;

void UA_ModelChangeStructureDataType_Initialize(UA_ModelChangeStructureDataType* pValue);

void UA_ModelChangeStructureDataType_Clear(UA_ModelChangeStructureDataType* pValue);

//StatusCode UA_ModelChangeStructureDataType_GetSize(UA_ModelChangeStructureDataType* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_ModelChangeStructureDataType_Encode(UA_MsgBuffer* msgBuf, UA_ModelChangeStructureDataType* pValue);

StatusCode UA_ModelChangeStructureDataType_Decode(UA_MsgBuffer* msgBuf, UA_ModelChangeStructureDataType* pValue);

extern struct UA_EncodeableType UA_ModelChangeStructureDataType_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_SemanticChangeStructureDataType
/*============================================================================
 * The SemanticChangeStructureDataType structure.
 *===========================================================================*/
typedef struct _UA_SemanticChangeStructureDataType
{
    UA_NodeId Affected;
    UA_NodeId AffectedType;
}
UA_SemanticChangeStructureDataType;

void UA_SemanticChangeStructureDataType_Initialize(UA_SemanticChangeStructureDataType* pValue);

void UA_SemanticChangeStructureDataType_Clear(UA_SemanticChangeStructureDataType* pValue);

//StatusCode UA_SemanticChangeStructureDataType_GetSize(UA_SemanticChangeStructureDataType* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_SemanticChangeStructureDataType_Encode(UA_MsgBuffer* msgBuf, UA_SemanticChangeStructureDataType* pValue);

StatusCode UA_SemanticChangeStructureDataType_Decode(UA_MsgBuffer* msgBuf, UA_SemanticChangeStructureDataType* pValue);

extern struct UA_EncodeableType UA_SemanticChangeStructureDataType_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_Range
/*============================================================================
 * The Range structure.
 *===========================================================================*/
typedef struct _UA_Range
{
    double Low;
    double High;
}
UA_Range;

void UA_Range_Initialize(UA_Range* pValue);

void UA_Range_Clear(UA_Range* pValue);

//StatusCode UA_Range_GetSize(UA_Range* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_Range_Encode(UA_MsgBuffer* msgBuf, UA_Range* pValue);

StatusCode UA_Range_Decode(UA_MsgBuffer* msgBuf, UA_Range* pValue);

extern struct UA_EncodeableType UA_Range_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_EUInformation
/*============================================================================
 * The EUInformation structure.
 *===========================================================================*/
typedef struct _UA_EUInformation
{
    UA_String        NamespaceUri;
    int32_t          UnitId;
    UA_LocalizedText DisplayName;
    UA_LocalizedText Description;
}
UA_EUInformation;

void UA_EUInformation_Initialize(UA_EUInformation* pValue);

void UA_EUInformation_Clear(UA_EUInformation* pValue);

//StatusCode UA_EUInformation_GetSize(UA_EUInformation* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_EUInformation_Encode(UA_MsgBuffer* msgBuf, UA_EUInformation* pValue);

StatusCode UA_EUInformation_Decode(UA_MsgBuffer* msgBuf, UA_EUInformation* pValue);

extern struct UA_EncodeableType UA_EUInformation_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_AxisScaleEnumeration
/*============================================================================
 * The AxisScaleEnumeration enumeration.
 *===========================================================================*/
typedef enum _UA_AxisScaleEnumeration
{
    UA_AxisScaleEnumeration_Linear = 0,
    UA_AxisScaleEnumeration_Log    = 1,
    UA_AxisScaleEnumeration_Ln     = 2
#if OPCUA_FORCE_INT32_ENUMS
    ,_UA_AxisScaleEnumeration_MaxEnumerationValue = OpcUa_Int32_Max
#endif
}
UA_AxisScaleEnumeration;

#endif

#ifndef OPCUA_EXCLUDE_ComplexNumberType
/*============================================================================
 * The ComplexNumberType structure.
 *===========================================================================*/
typedef struct _UA_ComplexNumberType
{
    float Real;
    float Imaginary;
}
UA_ComplexNumberType;

void UA_ComplexNumberType_Initialize(UA_ComplexNumberType* pValue);

void UA_ComplexNumberType_Clear(UA_ComplexNumberType* pValue);

//StatusCode UA_ComplexNumberType_GetSize(UA_ComplexNumberType* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_ComplexNumberType_Encode(UA_MsgBuffer* msgBuf, UA_ComplexNumberType* pValue);

StatusCode UA_ComplexNumberType_Decode(UA_MsgBuffer* msgBuf, UA_ComplexNumberType* pValue);

extern struct UA_EncodeableType UA_ComplexNumberType_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_DoubleComplexNumberType
/*============================================================================
 * The DoubleComplexNumberType structure.
 *===========================================================================*/
typedef struct _UA_DoubleComplexNumberType
{
    double Real;
    double Imaginary;
}
UA_DoubleComplexNumberType;

void UA_DoubleComplexNumberType_Initialize(UA_DoubleComplexNumberType* pValue);

void UA_DoubleComplexNumberType_Clear(UA_DoubleComplexNumberType* pValue);

//StatusCode UA_DoubleComplexNumberType_GetSize(UA_DoubleComplexNumberType* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_DoubleComplexNumberType_Encode(UA_MsgBuffer* msgBuf, UA_DoubleComplexNumberType* pValue);

StatusCode UA_DoubleComplexNumberType_Decode(UA_MsgBuffer* msgBuf, UA_DoubleComplexNumberType* pValue);

extern struct UA_EncodeableType UA_DoubleComplexNumberType_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_AxisInformation
/*============================================================================
 * The AxisInformation structure.
 *===========================================================================*/
typedef struct _UA_AxisInformation
{
    UA_EUInformation        EngineeringUnits;
    UA_Range                EURange;
    UA_LocalizedText        Title;
    UA_AxisScaleEnumeration AxisScaleType;
    int32_t                 NoOfAxisSteps;
    double*                 AxisSteps;
}
UA_AxisInformation;

void UA_AxisInformation_Initialize(UA_AxisInformation* pValue);

void UA_AxisInformation_Clear(UA_AxisInformation* pValue);

//StatusCode UA_AxisInformation_GetSize(UA_AxisInformation* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_AxisInformation_Encode(UA_MsgBuffer* msgBuf, UA_AxisInformation* pValue);

StatusCode UA_AxisInformation_Decode(UA_MsgBuffer* msgBuf, UA_AxisInformation* pValue);

extern struct UA_EncodeableType UA_AxisInformation_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_XVType
/*============================================================================
 * The XVType structure.
 *===========================================================================*/
typedef struct _UA_XVType
{
    double X;
    float  Value;
}
UA_XVType;

void UA_XVType_Initialize(UA_XVType* pValue);

void UA_XVType_Clear(UA_XVType* pValue);

//StatusCode UA_XVType_GetSize(UA_XVType* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_XVType_Encode(UA_MsgBuffer* msgBuf, UA_XVType* pValue);

StatusCode UA_XVType_Decode(UA_MsgBuffer* msgBuf, UA_XVType* pValue);

extern struct UA_EncodeableType UA_XVType_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ProgramDiagnosticDataType
/*============================================================================
 * The ProgramDiagnosticDataType structure.
 *===========================================================================*/
typedef struct _UA_ProgramDiagnosticDataType
{
    UA_NodeId       CreateSessionId;
    UA_String       CreateClientName;
    UA_DateTime     InvocationCreationTime;
    UA_DateTime     LastTransitionTime;
    UA_String       LastMethodCall;
    UA_NodeId       LastMethodSessionId;
    int32_t         NoOfLastMethodInputArguments;
    UA_Argument*    LastMethodInputArguments;
    int32_t         NoOfLastMethodOutputArguments;
    UA_Argument*    LastMethodOutputArguments;
    UA_DateTime     LastMethodCallTime;
    UA_StatusResult LastMethodReturnStatus;
}
UA_ProgramDiagnosticDataType;

void UA_ProgramDiagnosticDataType_Initialize(UA_ProgramDiagnosticDataType* pValue);

void UA_ProgramDiagnosticDataType_Clear(UA_ProgramDiagnosticDataType* pValue);

//StatusCode UA_ProgramDiagnosticDataType_GetSize(UA_ProgramDiagnosticDataType* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_ProgramDiagnosticDataType_Encode(UA_MsgBuffer* msgBuf, UA_ProgramDiagnosticDataType* pValue);

StatusCode UA_ProgramDiagnosticDataType_Decode(UA_MsgBuffer* msgBuf, UA_ProgramDiagnosticDataType* pValue);

extern struct UA_EncodeableType UA_ProgramDiagnosticDataType_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_Annotation
/*============================================================================
 * The Annotation structure.
 *===========================================================================*/
typedef struct _UA_Annotation
{
    UA_String   Message;
    UA_String   UserName;
    UA_DateTime AnnotationTime;
}
UA_Annotation;

void UA_Annotation_Initialize(UA_Annotation* pValue);

void UA_Annotation_Clear(UA_Annotation* pValue);

//StatusCode UA_Annotation_GetSize(UA_Annotation* pValue, struct _OpcUa_Encoder* pEncoder, OpcUa_Int32* pSize);

StatusCode UA_Annotation_Encode(UA_MsgBuffer* msgBuf, UA_Annotation* pValue);

StatusCode UA_Annotation_Decode(UA_MsgBuffer* msgBuf, UA_Annotation* pValue);

extern struct UA_EncodeableType UA_Annotation_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ExceptionDeviationFormat
/*============================================================================
 * The ExceptionDeviationFormat enumeration.
 *===========================================================================*/
typedef enum _UA_ExceptionDeviationFormat
{
    UA_ExceptionDeviationFormat_AbsoluteValue    = 0,
    UA_ExceptionDeviationFormat_PercentOfValue   = 1,
    UA_ExceptionDeviationFormat_PercentOfRange   = 2,
    UA_ExceptionDeviationFormat_PercentOfEURange = 3,
    UA_ExceptionDeviationFormat_Unknown          = 4
#if OPCUA_FORCE_INT32_ENUMS
    ,_UA_ExceptionDeviationFormat_MaxEnumerationValue = OpcUa_Int32_Max
#endif
}
UA_ExceptionDeviationFormat;

#endif

void UA_Initialize_EnumeratedType(int32_t* enumerationValue);
void UA_Initialize_Array(int32_t* noOfElts, void** eltsArray, size_t sizeOfElt,
                         UA_EncodeableObject_PfnInitialize* initFct);

void UA_Clear_EnumeratedType(int32_t* enumerationValue);
void UA_Clear_Array(int32_t* noOfElts, void** eltsArray, size_t sizeOfElt,
                    UA_EncodeableObject_PfnClear* clearFct);
                    

StatusCode UA_Read_EnumeratedType(UA_MsgBuffer* msgBuf, int32_t* enumerationValue);
StatusCode UA_Read_Array(UA_MsgBuffer* msgBuf, int32_t* noOfElts, void** eltsArray,
                         size_t sizeOfElt, UA_EncodeableObject_PfnDecode* decodeFct);
                    

StatusCode UA_Write_EnumeratedType(UA_MsgBuffer* msgBuf, int32_t* enumerationValue);
StatusCode UA_Write_Array(UA_MsgBuffer* msgBuf, int32_t* noOfElts, void** eltsArray,
                          size_t sizeOfElt, UA_EncodeableObject_PfnEncode* encodeFct);

/*============================================================================
 * Table of known types.
 *===========================================================================*/
extern struct UA_EncodeableType** UA_KnownEncodeableTypes;

OPCUA_END_EXTERN_C

#endif
/* This is the last line of an autogenerated file. */
