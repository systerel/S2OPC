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
 *
 * Modifications: adaptation for INGOPCS project
 * ======================================================================*/


#ifndef _SOPC_Types_H_
#define _SOPC_Types_H_ 1

//#ifndef OPCUA_FORCE_INT32_ENUMS
//# error OPCUA_FORCE_INT32_ENUMS must be defined!
//#endif /* OPCUA_FORCE_INT32_ENUMS */

#include "sopc_builtintypes.h"
#include "sopc_encodeabletype.h"

BEGIN_EXTERN_C

struct SOPC_MsgBuffer;

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
    SOPC_NodeId         ReferenceTypeId;
    SOPC_Boolean        IsInverse;
    SOPC_ExpandedNodeId TargetId;
}
OpcUa_ReferenceNode;

void OpcUa_ReferenceNode_Initialize(void* pValue);

void OpcUa_ReferenceNode_Clear(void* pValue);

SOPC_StatusCode OpcUa_ReferenceNode_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ReferenceNode_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_ReferenceNode_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_Node
/*============================================================================
 * The Node structure.
 *===========================================================================*/
typedef struct _OpcUa_Node
{
    SOPC_NodeId          NodeId;
    OpcUa_NodeClass      NodeClass;
    SOPC_QualifiedName   BrowseName;
    SOPC_LocalizedText   DisplayName;
    SOPC_LocalizedText   Description;
    uint32_t             WriteMask;
    uint32_t             UserWriteMask;
    int32_t              NoOfReferences;
    OpcUa_ReferenceNode* References;
}
OpcUa_Node;

void OpcUa_Node_Initialize(void* pValue);

void OpcUa_Node_Clear(void* pValue);

SOPC_StatusCode OpcUa_Node_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_Node_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_Node_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_InstanceNode
/*============================================================================
 * The InstanceNode structure.
 *===========================================================================*/
typedef struct _OpcUa_InstanceNode
{
    SOPC_NodeId          NodeId;
    OpcUa_NodeClass      NodeClass;
    SOPC_QualifiedName   BrowseName;
    SOPC_LocalizedText   DisplayName;
    SOPC_LocalizedText   Description;
    uint32_t             WriteMask;
    uint32_t             UserWriteMask;
    int32_t              NoOfReferences;
    OpcUa_ReferenceNode* References;
}
OpcUa_InstanceNode;

void OpcUa_InstanceNode_Initialize(void* pValue);

void OpcUa_InstanceNode_Clear(void* pValue);

SOPC_StatusCode OpcUa_InstanceNode_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_InstanceNode_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_InstanceNode_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_TypeNode
/*============================================================================
 * The TypeNode structure.
 *===========================================================================*/
typedef struct _OpcUa_TypeNode
{
    SOPC_NodeId          NodeId;
    OpcUa_NodeClass      NodeClass;
    SOPC_QualifiedName   BrowseName;
    SOPC_LocalizedText   DisplayName;
    SOPC_LocalizedText   Description;
    uint32_t             WriteMask;
    uint32_t             UserWriteMask;
    int32_t              NoOfReferences;
    OpcUa_ReferenceNode* References;
}
OpcUa_TypeNode;

void OpcUa_TypeNode_Initialize(void* pValue);

void OpcUa_TypeNode_Clear(void* pValue);

SOPC_StatusCode OpcUa_TypeNode_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_TypeNode_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_TypeNode_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ObjectNode
/*============================================================================
 * The ObjectNode structure.
 *===========================================================================*/
typedef struct _OpcUa_ObjectNode
{
    SOPC_NodeId          NodeId;
    OpcUa_NodeClass      NodeClass;
    SOPC_QualifiedName   BrowseName;
    SOPC_LocalizedText   DisplayName;
    SOPC_LocalizedText   Description;
    uint32_t             WriteMask;
    uint32_t             UserWriteMask;
    int32_t              NoOfReferences;
    OpcUa_ReferenceNode* References;
    SOPC_Byte            EventNotifier;
}
OpcUa_ObjectNode;

void OpcUa_ObjectNode_Initialize(void* pValue);

void OpcUa_ObjectNode_Clear(void* pValue);

SOPC_StatusCode OpcUa_ObjectNode_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ObjectNode_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_ObjectNode_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ObjectTypeNode
/*============================================================================
 * The ObjectTypeNode structure.
 *===========================================================================*/
typedef struct _OpcUa_ObjectTypeNode
{
    SOPC_NodeId          NodeId;
    OpcUa_NodeClass      NodeClass;
    SOPC_QualifiedName   BrowseName;
    SOPC_LocalizedText   DisplayName;
    SOPC_LocalizedText   Description;
    uint32_t             WriteMask;
    uint32_t             UserWriteMask;
    int32_t              NoOfReferences;
    OpcUa_ReferenceNode* References;
    SOPC_Boolean         IsAbstract;
}
OpcUa_ObjectTypeNode;

void OpcUa_ObjectTypeNode_Initialize(void* pValue);

void OpcUa_ObjectTypeNode_Clear(void* pValue);

SOPC_StatusCode OpcUa_ObjectTypeNode_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ObjectTypeNode_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_ObjectTypeNode_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_VariableNode
/*============================================================================
 * The VariableNode structure.
 *===========================================================================*/
typedef struct _OpcUa_VariableNode
{
    SOPC_NodeId          NodeId;
    OpcUa_NodeClass      NodeClass;
    SOPC_QualifiedName   BrowseName;
    SOPC_LocalizedText   DisplayName;
    SOPC_LocalizedText   Description;
    uint32_t             WriteMask;
    uint32_t             UserWriteMask;
    int32_t              NoOfReferences;
    OpcUa_ReferenceNode* References;
    SOPC_Variant         Value;
    SOPC_NodeId          DataType;
    int32_t              ValueRank;
    int32_t              NoOfArrayDimensions;
    uint32_t*            ArrayDimensions;
    SOPC_Byte            AccessLevel;
    SOPC_Byte            UserAccessLevel;
    double               MinimumSamplingInterval;
    SOPC_Boolean         Historizing;
}
OpcUa_VariableNode;

void OpcUa_VariableNode_Initialize(void* pValue);

void OpcUa_VariableNode_Clear(void* pValue);

SOPC_StatusCode OpcUa_VariableNode_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_VariableNode_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_VariableNode_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_VariableTypeNode
/*============================================================================
 * The VariableTypeNode structure.
 *===========================================================================*/
typedef struct _OpcUa_VariableTypeNode
{
    SOPC_NodeId          NodeId;
    OpcUa_NodeClass      NodeClass;
    SOPC_QualifiedName   BrowseName;
    SOPC_LocalizedText   DisplayName;
    SOPC_LocalizedText   Description;
    uint32_t             WriteMask;
    uint32_t             UserWriteMask;
    int32_t              NoOfReferences;
    OpcUa_ReferenceNode* References;
    SOPC_Variant         Value;
    SOPC_NodeId          DataType;
    int32_t              ValueRank;
    int32_t              NoOfArrayDimensions;
    uint32_t*            ArrayDimensions;
    SOPC_Boolean         IsAbstract;
}
OpcUa_VariableTypeNode;

void OpcUa_VariableTypeNode_Initialize(void* pValue);

void OpcUa_VariableTypeNode_Clear(void* pValue);

SOPC_StatusCode OpcUa_VariableTypeNode_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_VariableTypeNode_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_VariableTypeNode_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ReferenceTypeNode
/*============================================================================
 * The ReferenceTypeNode structure.
 *===========================================================================*/
typedef struct _OpcUa_ReferenceTypeNode
{
    SOPC_NodeId          NodeId;
    OpcUa_NodeClass      NodeClass;
    SOPC_QualifiedName   BrowseName;
    SOPC_LocalizedText   DisplayName;
    SOPC_LocalizedText   Description;
    uint32_t             WriteMask;
    uint32_t             UserWriteMask;
    int32_t              NoOfReferences;
    OpcUa_ReferenceNode* References;
    SOPC_Boolean         IsAbstract;
    SOPC_Boolean         Symmetric;
    SOPC_LocalizedText   InverseName;
}
OpcUa_ReferenceTypeNode;

void OpcUa_ReferenceTypeNode_Initialize(void* pValue);

void OpcUa_ReferenceTypeNode_Clear(void* pValue);

SOPC_StatusCode OpcUa_ReferenceTypeNode_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ReferenceTypeNode_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_ReferenceTypeNode_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_MethodNode
/*============================================================================
 * The MethodNode structure.
 *===========================================================================*/
typedef struct _OpcUa_MethodNode
{
    SOPC_NodeId          NodeId;
    OpcUa_NodeClass      NodeClass;
    SOPC_QualifiedName   BrowseName;
    SOPC_LocalizedText   DisplayName;
    SOPC_LocalizedText   Description;
    uint32_t             WriteMask;
    uint32_t             UserWriteMask;
    int32_t              NoOfReferences;
    OpcUa_ReferenceNode* References;
    SOPC_Boolean         Executable;
    SOPC_Boolean         UserExecutable;
}
OpcUa_MethodNode;

void OpcUa_MethodNode_Initialize(void* pValue);

void OpcUa_MethodNode_Clear(void* pValue);

SOPC_StatusCode OpcUa_MethodNode_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_MethodNode_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_MethodNode_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ViewNode
/*============================================================================
 * The ViewNode structure.
 *===========================================================================*/
typedef struct _OpcUa_ViewNode
{
    SOPC_NodeId          NodeId;
    OpcUa_NodeClass      NodeClass;
    SOPC_QualifiedName   BrowseName;
    SOPC_LocalizedText   DisplayName;
    SOPC_LocalizedText   Description;
    uint32_t             WriteMask;
    uint32_t             UserWriteMask;
    int32_t              NoOfReferences;
    OpcUa_ReferenceNode* References;
    SOPC_Boolean         ContainsNoLoops;
    SOPC_Byte            EventNotifier;
}
OpcUa_ViewNode;

void OpcUa_ViewNode_Initialize(void* pValue);

void OpcUa_ViewNode_Clear(void* pValue);

SOPC_StatusCode OpcUa_ViewNode_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ViewNode_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_ViewNode_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_DataTypeNode
/*============================================================================
 * The DataTypeNode structure.
 *===========================================================================*/
typedef struct _OpcUa_DataTypeNode
{
    SOPC_NodeId          NodeId;
    OpcUa_NodeClass      NodeClass;
    SOPC_QualifiedName   BrowseName;
    SOPC_LocalizedText   DisplayName;
    SOPC_LocalizedText   Description;
    uint32_t             WriteMask;
    uint32_t             UserWriteMask;
    int32_t              NoOfReferences;
    OpcUa_ReferenceNode* References;
    SOPC_Boolean         IsAbstract;
}
OpcUa_DataTypeNode;

void OpcUa_DataTypeNode_Initialize(void* pValue);

void OpcUa_DataTypeNode_Clear(void* pValue);

SOPC_StatusCode OpcUa_DataTypeNode_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_DataTypeNode_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_DataTypeNode_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_Argument
/*============================================================================
 * The Argument structure.
 *===========================================================================*/
typedef struct _OpcUa_Argument
{
    SOPC_String        Name;
    SOPC_NodeId        DataType;
    int32_t            ValueRank;
    int32_t            NoOfArrayDimensions;
    uint32_t*          ArrayDimensions;
    SOPC_LocalizedText Description;
}
OpcUa_Argument;

void OpcUa_Argument_Initialize(void* pValue);

void OpcUa_Argument_Clear(void* pValue);

SOPC_StatusCode OpcUa_Argument_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_Argument_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_Argument_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_EnumValueType
/*============================================================================
 * The EnumValueType structure.
 *===========================================================================*/
typedef struct _OpcUa_EnumValueType
{
    int64_t            Value;
    SOPC_LocalizedText DisplayName;
    SOPC_LocalizedText Description;
}
OpcUa_EnumValueType;

void OpcUa_EnumValueType_Initialize(void* pValue);

void OpcUa_EnumValueType_Clear(void* pValue);

SOPC_StatusCode OpcUa_EnumValueType_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_EnumValueType_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_EnumValueType_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_EnumField
/*============================================================================
 * The EnumField structure.
 *===========================================================================*/
typedef struct _OpcUa_EnumField
{
    int64_t            Value;
    SOPC_LocalizedText DisplayName;
    SOPC_LocalizedText Description;
    SOPC_String        Name;
}
OpcUa_EnumField;

void OpcUa_EnumField_Initialize(void* pValue);

void OpcUa_EnumField_Clear(void* pValue);

SOPC_StatusCode OpcUa_EnumField_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_EnumField_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_EnumField_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_OptionSet
/*============================================================================
 * The OptionSet structure.
 *===========================================================================*/
typedef struct _OpcUa_OptionSet
{
    SOPC_ByteString Value;
    SOPC_ByteString ValidBits;
}
OpcUa_OptionSet;

void OpcUa_OptionSet_Initialize(void* pValue);

void OpcUa_OptionSet_Clear(void* pValue);

SOPC_StatusCode OpcUa_OptionSet_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_OptionSet_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_OptionSet_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_TimeZoneDataType
/*============================================================================
 * The TimeZoneDataType structure.
 *===========================================================================*/
typedef struct _OpcUa_TimeZoneDataType
{
    int16_t      Offset;
    SOPC_Boolean DaylightSavingInOffset;
}
OpcUa_TimeZoneDataType;

void OpcUa_TimeZoneDataType_Initialize(void* pValue);

void OpcUa_TimeZoneDataType_Clear(void* pValue);

SOPC_StatusCode OpcUa_TimeZoneDataType_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_TimeZoneDataType_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_TimeZoneDataType_EncodeableType;
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
    SOPC_String           ApplicationUri;
    SOPC_String           ProductUri;
    SOPC_LocalizedText    ApplicationName;
    OpcUa_ApplicationType ApplicationType;
    SOPC_String           GatewayServerUri;
    SOPC_String           DiscoveryProfileUri;
    int32_t               NoOfDiscoveryUrls;
    SOPC_String*          DiscoveryUrls;
}
OpcUa_ApplicationDescription;

void OpcUa_ApplicationDescription_Initialize(void* pValue);

void OpcUa_ApplicationDescription_Clear(void* pValue);

SOPC_StatusCode OpcUa_ApplicationDescription_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ApplicationDescription_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_ApplicationDescription_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_RequestHeader
/*============================================================================
 * The RequestHeader structure.
 *===========================================================================*/
typedef struct _OpcUa_RequestHeader
{
    SOPC_NodeId          AuthenticationToken;
    SOPC_DateTime        Timestamp;
    uint32_t             RequestHandle;
    uint32_t             ReturnDiagnostics;
    SOPC_String          AuditEntryId;
    uint32_t             TimeoutHint;
    SOPC_ExtensionObject AdditionalHeader;
}
OpcUa_RequestHeader;

void OpcUa_RequestHeader_Initialize(void* pValue);

void OpcUa_RequestHeader_Clear(void* pValue);

SOPC_StatusCode OpcUa_RequestHeader_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_RequestHeader_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_RequestHeader_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ResponseHeader
/*============================================================================
 * The ResponseHeader structure.
 *===========================================================================*/
typedef struct _OpcUa_ResponseHeader
{
    SOPC_DateTime        Timestamp;
    uint32_t             RequestHandle;
    SOPC_StatusCode      ServiceResult;
    SOPC_DiagnosticInfo  ServiceDiagnostics;
    int32_t              NoOfStringTable;
    SOPC_String*         StringTable;
    SOPC_ExtensionObject AdditionalHeader;
}
OpcUa_ResponseHeader;

void OpcUa_ResponseHeader_Initialize(void* pValue);

void OpcUa_ResponseHeader_Clear(void* pValue);

SOPC_StatusCode OpcUa_ResponseHeader_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ResponseHeader_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_ResponseHeader_EncodeableType;
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

void OpcUa_ServiceFault_Initialize(void* pValue);

void OpcUa_ServiceFault_Clear(void* pValue);

SOPC_StatusCode OpcUa_ServiceFault_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ServiceFault_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_ServiceFault_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_FindServers
#ifndef OPCUA_EXCLUDE_FindServersRequest
/*============================================================================
 * The FindServersRequest structure.
 *===========================================================================*/
typedef struct _OpcUa_FindServersRequest
{
    OpcUa_RequestHeader RequestHeader;
    SOPC_String         EndpointUrl;
    int32_t             NoOfLocaleIds;
    SOPC_String*        LocaleIds;
    int32_t             NoOfServerUris;
    SOPC_String*        ServerUris;
}
OpcUa_FindServersRequest;

void OpcUa_FindServersRequest_Initialize(void* pValue);

void OpcUa_FindServersRequest_Clear(void* pValue);

SOPC_StatusCode OpcUa_FindServersRequest_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_FindServersRequest_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_FindServersRequest_EncodeableType;
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

void OpcUa_FindServersResponse_Initialize(void* pValue);

void OpcUa_FindServersResponse_Clear(void* pValue);

SOPC_StatusCode OpcUa_FindServersResponse_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_FindServersResponse_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_FindServersResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_ServerOnNetwork
/*============================================================================
 * The ServerOnNetwork structure.
 *===========================================================================*/
typedef struct _OpcUa_ServerOnNetwork
{
    uint32_t     RecordId;
    SOPC_String  ServerName;
    SOPC_String  DiscoveryUrl;
    int32_t      NoOfServerCapabilities;
    SOPC_String* ServerCapabilities;
}
OpcUa_ServerOnNetwork;

void OpcUa_ServerOnNetwork_Initialize(void* pValue);

void OpcUa_ServerOnNetwork_Clear(void* pValue);

SOPC_StatusCode OpcUa_ServerOnNetwork_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ServerOnNetwork_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_ServerOnNetwork_EncodeableType;
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
    SOPC_String*        ServerCapabilityFilter;
}
OpcUa_FindServersOnNetworkRequest;

void OpcUa_FindServersOnNetworkRequest_Initialize(void* pValue);

void OpcUa_FindServersOnNetworkRequest_Clear(void* pValue);

SOPC_StatusCode OpcUa_FindServersOnNetworkRequest_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_FindServersOnNetworkRequest_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_FindServersOnNetworkRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_FindServersOnNetworkResponse
/*============================================================================
 * The FindServersOnNetworkResponse structure.
 *===========================================================================*/
typedef struct _OpcUa_FindServersOnNetworkResponse
{
    OpcUa_ResponseHeader   ResponseHeader;
    SOPC_DateTime          LastCounterResetTime;
    int32_t                NoOfServers;
    OpcUa_ServerOnNetwork* Servers;
}
OpcUa_FindServersOnNetworkResponse;

void OpcUa_FindServersOnNetworkResponse_Initialize(void* pValue);

void OpcUa_FindServersOnNetworkResponse_Clear(void* pValue);

SOPC_StatusCode OpcUa_FindServersOnNetworkResponse_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_FindServersOnNetworkResponse_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_FindServersOnNetworkResponse_EncodeableType;
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
    SOPC_String         PolicyId;
    OpcUa_UserTokenType TokenType;
    SOPC_String         IssuedTokenType;
    SOPC_String         IssuerEndpointUrl;
    SOPC_String         SecurityPolicyUri;
}
OpcUa_UserTokenPolicy;

void OpcUa_UserTokenPolicy_Initialize(void* pValue);

void OpcUa_UserTokenPolicy_Clear(void* pValue);

SOPC_StatusCode OpcUa_UserTokenPolicy_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_UserTokenPolicy_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_UserTokenPolicy_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_EndpointDescription
/*============================================================================
 * The EndpointDescription structure.
 *===========================================================================*/
typedef struct _OpcUa_EndpointDescription
{
    SOPC_String                  EndpointUrl;
    OpcUa_ApplicationDescription Server;
    SOPC_ByteString              ServerCertificate;
    OpcUa_MessageSecurityMode    SecurityMode;
    SOPC_String                  SecurityPolicyUri;
    int32_t                      NoOfUserIdentityTokens;
    OpcUa_UserTokenPolicy*       UserIdentityTokens;
    SOPC_String                  TransportProfileUri;
    SOPC_Byte                    SecurityLevel;
}
OpcUa_EndpointDescription;

void OpcUa_EndpointDescription_Initialize(void* pValue);

void OpcUa_EndpointDescription_Clear(void* pValue);

SOPC_StatusCode OpcUa_EndpointDescription_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_EndpointDescription_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_EndpointDescription_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_GetEndpoints
#ifndef OPCUA_EXCLUDE_GetEndpointsRequest
/*============================================================================
 * The GetEndpointsRequest structure.
 *===========================================================================*/
typedef struct _OpcUa_GetEndpointsRequest
{
    OpcUa_RequestHeader RequestHeader;
    SOPC_String         EndpointUrl;
    int32_t             NoOfLocaleIds;
    SOPC_String*        LocaleIds;
    int32_t             NoOfProfileUris;
    SOPC_String*        ProfileUris;
}
OpcUa_GetEndpointsRequest;

void OpcUa_GetEndpointsRequest_Initialize(void* pValue);

void OpcUa_GetEndpointsRequest_Clear(void* pValue);

SOPC_StatusCode OpcUa_GetEndpointsRequest_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_GetEndpointsRequest_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_GetEndpointsRequest_EncodeableType;
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

void OpcUa_GetEndpointsResponse_Initialize(void* pValue);

void OpcUa_GetEndpointsResponse_Clear(void* pValue);

SOPC_StatusCode OpcUa_GetEndpointsResponse_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_GetEndpointsResponse_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_GetEndpointsResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_RegisteredServer
/*============================================================================
 * The RegisteredServer structure.
 *===========================================================================*/
typedef struct _OpcUa_RegisteredServer
{
    SOPC_String           ServerUri;
    SOPC_String           ProductUri;
    int32_t               NoOfServerNames;
    SOPC_LocalizedText*   ServerNames;
    OpcUa_ApplicationType ServerType;
    SOPC_String           GatewayServerUri;
    int32_t               NoOfDiscoveryUrls;
    SOPC_String*          DiscoveryUrls;
    SOPC_String           SemaphoreFilePath;
    SOPC_Boolean          IsOnline;
}
OpcUa_RegisteredServer;

void OpcUa_RegisteredServer_Initialize(void* pValue);

void OpcUa_RegisteredServer_Clear(void* pValue);

SOPC_StatusCode OpcUa_RegisteredServer_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_RegisteredServer_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_RegisteredServer_EncodeableType;
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

void OpcUa_RegisterServerRequest_Initialize(void* pValue);

void OpcUa_RegisterServerRequest_Clear(void* pValue);

SOPC_StatusCode OpcUa_RegisterServerRequest_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_RegisterServerRequest_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_RegisterServerRequest_EncodeableType;
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

void OpcUa_RegisterServerResponse_Initialize(void* pValue);

void OpcUa_RegisterServerResponse_Clear(void* pValue);

SOPC_StatusCode OpcUa_RegisterServerResponse_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_RegisterServerResponse_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_RegisterServerResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_MdnsDiscoveryConfiguration
/*============================================================================
 * The MdnsDiscoveryConfiguration structure.
 *===========================================================================*/
typedef struct _OpcUa_MdnsDiscoveryConfiguration
{
    SOPC_String  MdnsServerName;
    int32_t      NoOfServerCapabilities;
    SOPC_String* ServerCapabilities;
}
OpcUa_MdnsDiscoveryConfiguration;

void OpcUa_MdnsDiscoveryConfiguration_Initialize(void* pValue);

void OpcUa_MdnsDiscoveryConfiguration_Clear(void* pValue);

SOPC_StatusCode OpcUa_MdnsDiscoveryConfiguration_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_MdnsDiscoveryConfiguration_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_MdnsDiscoveryConfiguration_EncodeableType;
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
    SOPC_ExtensionObject*  DiscoveryConfiguration;
}
OpcUa_RegisterServer2Request;

void OpcUa_RegisterServer2Request_Initialize(void* pValue);

void OpcUa_RegisterServer2Request_Clear(void* pValue);

SOPC_StatusCode OpcUa_RegisterServer2Request_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_RegisterServer2Request_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_RegisterServer2Request_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_RegisterServer2Response
/*============================================================================
 * The RegisterServer2Response structure.
 *===========================================================================*/
typedef struct _OpcUa_RegisterServer2Response
{
    OpcUa_ResponseHeader ResponseHeader;
    int32_t              NoOfConfigurationResults;
    SOPC_StatusCode*     ConfigurationResults;
    int32_t              NoOfDiagnosticInfos;
    SOPC_DiagnosticInfo* DiagnosticInfos;
}
OpcUa_RegisterServer2Response;

void OpcUa_RegisterServer2Response_Initialize(void* pValue);

void OpcUa_RegisterServer2Response_Clear(void* pValue);

SOPC_StatusCode OpcUa_RegisterServer2Response_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_RegisterServer2Response_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_RegisterServer2Response_EncodeableType;
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
    uint32_t      ChannelId;
    uint32_t      TokenId;
    SOPC_DateTime CreatedAt;
    uint32_t      RevisedLifetime;
}
OpcUa_ChannelSecurityToken;

void OpcUa_ChannelSecurityToken_Initialize(void* pValue);

void OpcUa_ChannelSecurityToken_Clear(void* pValue);

SOPC_StatusCode OpcUa_ChannelSecurityToken_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ChannelSecurityToken_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_ChannelSecurityToken_EncodeableType;
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
    SOPC_ByteString                ClientNonce;
    uint32_t                       RequestedLifetime;
}
OpcUa_OpenSecureChannelRequest;

void OpcUa_OpenSecureChannelRequest_Initialize(void* pValue);

void OpcUa_OpenSecureChannelRequest_Clear(void* pValue);

SOPC_StatusCode OpcUa_OpenSecureChannelRequest_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_OpenSecureChannelRequest_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_OpenSecureChannelRequest_EncodeableType;
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
    SOPC_ByteString            ServerNonce;
}
OpcUa_OpenSecureChannelResponse;

void OpcUa_OpenSecureChannelResponse_Initialize(void* pValue);

void OpcUa_OpenSecureChannelResponse_Clear(void* pValue);

SOPC_StatusCode OpcUa_OpenSecureChannelResponse_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_OpenSecureChannelResponse_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_OpenSecureChannelResponse_EncodeableType;
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

void OpcUa_CloseSecureChannelRequest_Initialize(void* pValue);

void OpcUa_CloseSecureChannelRequest_Clear(void* pValue);

SOPC_StatusCode OpcUa_CloseSecureChannelRequest_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_CloseSecureChannelRequest_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_CloseSecureChannelRequest_EncodeableType;
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

void OpcUa_CloseSecureChannelResponse_Initialize(void* pValue);

void OpcUa_CloseSecureChannelResponse_Clear(void* pValue);

SOPC_StatusCode OpcUa_CloseSecureChannelResponse_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_CloseSecureChannelResponse_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_CloseSecureChannelResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_SignedSoftwareCertificate
/*============================================================================
 * The SignedSoftwareCertificate structure.
 *===========================================================================*/
typedef struct _OpcUa_SignedSoftwareCertificate
{
    SOPC_ByteString CertificateData;
    SOPC_ByteString Signature;
}
OpcUa_SignedSoftwareCertificate;

void OpcUa_SignedSoftwareCertificate_Initialize(void* pValue);

void OpcUa_SignedSoftwareCertificate_Clear(void* pValue);

SOPC_StatusCode OpcUa_SignedSoftwareCertificate_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_SignedSoftwareCertificate_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_SignedSoftwareCertificate_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_SignatureData
/*============================================================================
 * The SignatureData structure.
 *===========================================================================*/
typedef struct _OpcUa_SignatureData
{
    SOPC_String     Algorithm;
    SOPC_ByteString Signature;
}
OpcUa_SignatureData;

void OpcUa_SignatureData_Initialize(void* pValue);

void OpcUa_SignatureData_Clear(void* pValue);

SOPC_StatusCode OpcUa_SignatureData_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_SignatureData_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_SignatureData_EncodeableType;
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
    SOPC_String                  ServerUri;
    SOPC_String                  EndpointUrl;
    SOPC_String                  SessionName;
    SOPC_ByteString              ClientNonce;
    SOPC_ByteString              ClientCertificate;
    double                       RequestedSessionTimeout;
    uint32_t                     MaxResponseMessageSize;
}
OpcUa_CreateSessionRequest;

void OpcUa_CreateSessionRequest_Initialize(void* pValue);

void OpcUa_CreateSessionRequest_Clear(void* pValue);

SOPC_StatusCode OpcUa_CreateSessionRequest_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_CreateSessionRequest_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_CreateSessionRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_CreateSessionResponse
/*============================================================================
 * The CreateSessionResponse structure.
 *===========================================================================*/
typedef struct _OpcUa_CreateSessionResponse
{
    OpcUa_ResponseHeader             ResponseHeader;
    SOPC_NodeId                      SessionId;
    SOPC_NodeId                      AuthenticationToken;
    double                           RevisedSessionTimeout;
    SOPC_ByteString                  ServerNonce;
    SOPC_ByteString                  ServerCertificate;
    int32_t                          NoOfServerEndpoints;
    OpcUa_EndpointDescription*       ServerEndpoints;
    int32_t                          NoOfServerSoftwareCertificates;
    OpcUa_SignedSoftwareCertificate* ServerSoftwareCertificates;
    OpcUa_SignatureData              ServerSignature;
    uint32_t                         MaxRequestMessageSize;
}
OpcUa_CreateSessionResponse;

void OpcUa_CreateSessionResponse_Initialize(void* pValue);

void OpcUa_CreateSessionResponse_Clear(void* pValue);

SOPC_StatusCode OpcUa_CreateSessionResponse_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_CreateSessionResponse_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_CreateSessionResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_UserIdentityToken
/*============================================================================
 * The UserIdentityToken structure.
 *===========================================================================*/
typedef struct _OpcUa_UserIdentityToken
{
    SOPC_String PolicyId;
}
OpcUa_UserIdentityToken;

void OpcUa_UserIdentityToken_Initialize(void* pValue);

void OpcUa_UserIdentityToken_Clear(void* pValue);

SOPC_StatusCode OpcUa_UserIdentityToken_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_UserIdentityToken_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_UserIdentityToken_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_AnonymousIdentityToken
/*============================================================================
 * The AnonymousIdentityToken structure.
 *===========================================================================*/
typedef struct _OpcUa_AnonymousIdentityToken
{
    SOPC_String PolicyId;
}
OpcUa_AnonymousIdentityToken;

void OpcUa_AnonymousIdentityToken_Initialize(void* pValue);

void OpcUa_AnonymousIdentityToken_Clear(void* pValue);

SOPC_StatusCode OpcUa_AnonymousIdentityToken_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_AnonymousIdentityToken_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_AnonymousIdentityToken_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_UserNameIdentityToken
/*============================================================================
 * The UserNameIdentityToken structure.
 *===========================================================================*/
typedef struct _OpcUa_UserNameIdentityToken
{
    SOPC_String     PolicyId;
    SOPC_String     UserName;
    SOPC_ByteString Password;
    SOPC_String     EncryptionAlgorithm;
}
OpcUa_UserNameIdentityToken;

void OpcUa_UserNameIdentityToken_Initialize(void* pValue);

void OpcUa_UserNameIdentityToken_Clear(void* pValue);

SOPC_StatusCode OpcUa_UserNameIdentityToken_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_UserNameIdentityToken_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_UserNameIdentityToken_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_X509IdentityToken
/*============================================================================
 * The X509IdentityToken structure.
 *===========================================================================*/
typedef struct _OpcUa_X509IdentityToken
{
    SOPC_String     PolicyId;
    SOPC_ByteString CertificateData;
}
OpcUa_X509IdentityToken;

void OpcUa_X509IdentityToken_Initialize(void* pValue);

void OpcUa_X509IdentityToken_Clear(void* pValue);

SOPC_StatusCode OpcUa_X509IdentityToken_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_X509IdentityToken_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_X509IdentityToken_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_KerberosIdentityToken
/*============================================================================
 * The KerberosIdentityToken structure.
 *===========================================================================*/
typedef struct _OpcUa_KerberosIdentityToken
{
    SOPC_String     PolicyId;
    SOPC_ByteString TicketData;
}
OpcUa_KerberosIdentityToken;

void OpcUa_KerberosIdentityToken_Initialize(void* pValue);

void OpcUa_KerberosIdentityToken_Clear(void* pValue);

SOPC_StatusCode OpcUa_KerberosIdentityToken_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_KerberosIdentityToken_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_KerberosIdentityToken_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_IssuedIdentityToken
/*============================================================================
 * The IssuedIdentityToken structure.
 *===========================================================================*/
typedef struct _OpcUa_IssuedIdentityToken
{
    SOPC_String     PolicyId;
    SOPC_ByteString TokenData;
    SOPC_String     EncryptionAlgorithm;
}
OpcUa_IssuedIdentityToken;

void OpcUa_IssuedIdentityToken_Initialize(void* pValue);

void OpcUa_IssuedIdentityToken_Clear(void* pValue);

SOPC_StatusCode OpcUa_IssuedIdentityToken_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_IssuedIdentityToken_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_IssuedIdentityToken_EncodeableType;
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
    SOPC_String*                     LocaleIds;
    SOPC_ExtensionObject             UserIdentityToken;
    OpcUa_SignatureData              UserTokenSignature;
}
OpcUa_ActivateSessionRequest;

void OpcUa_ActivateSessionRequest_Initialize(void* pValue);

void OpcUa_ActivateSessionRequest_Clear(void* pValue);

SOPC_StatusCode OpcUa_ActivateSessionRequest_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ActivateSessionRequest_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_ActivateSessionRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ActivateSessionResponse
/*============================================================================
 * The ActivateSessionResponse structure.
 *===========================================================================*/
typedef struct _OpcUa_ActivateSessionResponse
{
    OpcUa_ResponseHeader ResponseHeader;
    SOPC_ByteString      ServerNonce;
    int32_t              NoOfResults;
    SOPC_StatusCode*     Results;
    int32_t              NoOfDiagnosticInfos;
    SOPC_DiagnosticInfo* DiagnosticInfos;
}
OpcUa_ActivateSessionResponse;

void OpcUa_ActivateSessionResponse_Initialize(void* pValue);

void OpcUa_ActivateSessionResponse_Clear(void* pValue);

SOPC_StatusCode OpcUa_ActivateSessionResponse_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ActivateSessionResponse_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_ActivateSessionResponse_EncodeableType;
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
    SOPC_Boolean        DeleteSubscriptions;
}
OpcUa_CloseSessionRequest;

void OpcUa_CloseSessionRequest_Initialize(void* pValue);

void OpcUa_CloseSessionRequest_Clear(void* pValue);

SOPC_StatusCode OpcUa_CloseSessionRequest_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_CloseSessionRequest_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_CloseSessionRequest_EncodeableType;
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

void OpcUa_CloseSessionResponse_Initialize(void* pValue);

void OpcUa_CloseSessionResponse_Clear(void* pValue);

SOPC_StatusCode OpcUa_CloseSessionResponse_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_CloseSessionResponse_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_CloseSessionResponse_EncodeableType;
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

void OpcUa_CancelRequest_Initialize(void* pValue);

void OpcUa_CancelRequest_Clear(void* pValue);

SOPC_StatusCode OpcUa_CancelRequest_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_CancelRequest_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_CancelRequest_EncodeableType;
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

void OpcUa_CancelResponse_Initialize(void* pValue);

void OpcUa_CancelResponse_Clear(void* pValue);

SOPC_StatusCode OpcUa_CancelResponse_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_CancelResponse_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_CancelResponse_EncodeableType;
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
    uint32_t           SpecifiedAttributes;
    SOPC_LocalizedText DisplayName;
    SOPC_LocalizedText Description;
    uint32_t           WriteMask;
    uint32_t           UserWriteMask;
}
OpcUa_NodeAttributes;

void OpcUa_NodeAttributes_Initialize(void* pValue);

void OpcUa_NodeAttributes_Clear(void* pValue);

SOPC_StatusCode OpcUa_NodeAttributes_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_NodeAttributes_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_NodeAttributes_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ObjectAttributes
/*============================================================================
 * The ObjectAttributes structure.
 *===========================================================================*/
typedef struct _OpcUa_ObjectAttributes
{
    uint32_t           SpecifiedAttributes;
    SOPC_LocalizedText DisplayName;
    SOPC_LocalizedText Description;
    uint32_t           WriteMask;
    uint32_t           UserWriteMask;
    SOPC_Byte          EventNotifier;
}
OpcUa_ObjectAttributes;

void OpcUa_ObjectAttributes_Initialize(void* pValue);

void OpcUa_ObjectAttributes_Clear(void* pValue);

SOPC_StatusCode OpcUa_ObjectAttributes_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ObjectAttributes_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_ObjectAttributes_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_VariableAttributes
/*============================================================================
 * The VariableAttributes structure.
 *===========================================================================*/
typedef struct _OpcUa_VariableAttributes
{
    uint32_t           SpecifiedAttributes;
    SOPC_LocalizedText DisplayName;
    SOPC_LocalizedText Description;
    uint32_t           WriteMask;
    uint32_t           UserWriteMask;
    SOPC_Variant       Value;
    SOPC_NodeId        DataType;
    int32_t            ValueRank;
    int32_t            NoOfArrayDimensions;
    uint32_t*          ArrayDimensions;
    SOPC_Byte          AccessLevel;
    SOPC_Byte          UserAccessLevel;
    double             MinimumSamplingInterval;
    SOPC_Boolean       Historizing;
}
OpcUa_VariableAttributes;

void OpcUa_VariableAttributes_Initialize(void* pValue);

void OpcUa_VariableAttributes_Clear(void* pValue);

SOPC_StatusCode OpcUa_VariableAttributes_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_VariableAttributes_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_VariableAttributes_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_MethodAttributes
/*============================================================================
 * The MethodAttributes structure.
 *===========================================================================*/
typedef struct _OpcUa_MethodAttributes
{
    uint32_t           SpecifiedAttributes;
    SOPC_LocalizedText DisplayName;
    SOPC_LocalizedText Description;
    uint32_t           WriteMask;
    uint32_t           UserWriteMask;
    SOPC_Boolean       Executable;
    SOPC_Boolean       UserExecutable;
}
OpcUa_MethodAttributes;

void OpcUa_MethodAttributes_Initialize(void* pValue);

void OpcUa_MethodAttributes_Clear(void* pValue);

SOPC_StatusCode OpcUa_MethodAttributes_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_MethodAttributes_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_MethodAttributes_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ObjectTypeAttributes
/*============================================================================
 * The ObjectTypeAttributes structure.
 *===========================================================================*/
typedef struct _OpcUa_ObjectTypeAttributes
{
    uint32_t           SpecifiedAttributes;
    SOPC_LocalizedText DisplayName;
    SOPC_LocalizedText Description;
    uint32_t           WriteMask;
    uint32_t           UserWriteMask;
    SOPC_Boolean       IsAbstract;
}
OpcUa_ObjectTypeAttributes;

void OpcUa_ObjectTypeAttributes_Initialize(void* pValue);

void OpcUa_ObjectTypeAttributes_Clear(void* pValue);

SOPC_StatusCode OpcUa_ObjectTypeAttributes_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ObjectTypeAttributes_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_ObjectTypeAttributes_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_VariableTypeAttributes
/*============================================================================
 * The VariableTypeAttributes structure.
 *===========================================================================*/
typedef struct _OpcUa_VariableTypeAttributes
{
    uint32_t           SpecifiedAttributes;
    SOPC_LocalizedText DisplayName;
    SOPC_LocalizedText Description;
    uint32_t           WriteMask;
    uint32_t           UserWriteMask;
    SOPC_Variant       Value;
    SOPC_NodeId        DataType;
    int32_t            ValueRank;
    int32_t            NoOfArrayDimensions;
    uint32_t*          ArrayDimensions;
    SOPC_Boolean       IsAbstract;
}
OpcUa_VariableTypeAttributes;

void OpcUa_VariableTypeAttributes_Initialize(void* pValue);

void OpcUa_VariableTypeAttributes_Clear(void* pValue);

SOPC_StatusCode OpcUa_VariableTypeAttributes_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_VariableTypeAttributes_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_VariableTypeAttributes_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ReferenceTypeAttributes
/*============================================================================
 * The ReferenceTypeAttributes structure.
 *===========================================================================*/
typedef struct _OpcUa_ReferenceTypeAttributes
{
    uint32_t           SpecifiedAttributes;
    SOPC_LocalizedText DisplayName;
    SOPC_LocalizedText Description;
    uint32_t           WriteMask;
    uint32_t           UserWriteMask;
    SOPC_Boolean       IsAbstract;
    SOPC_Boolean       Symmetric;
    SOPC_LocalizedText InverseName;
}
OpcUa_ReferenceTypeAttributes;

void OpcUa_ReferenceTypeAttributes_Initialize(void* pValue);

void OpcUa_ReferenceTypeAttributes_Clear(void* pValue);

SOPC_StatusCode OpcUa_ReferenceTypeAttributes_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ReferenceTypeAttributes_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_ReferenceTypeAttributes_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_DataTypeAttributes
/*============================================================================
 * The DataTypeAttributes structure.
 *===========================================================================*/
typedef struct _OpcUa_DataTypeAttributes
{
    uint32_t           SpecifiedAttributes;
    SOPC_LocalizedText DisplayName;
    SOPC_LocalizedText Description;
    uint32_t           WriteMask;
    uint32_t           UserWriteMask;
    SOPC_Boolean       IsAbstract;
}
OpcUa_DataTypeAttributes;

void OpcUa_DataTypeAttributes_Initialize(void* pValue);

void OpcUa_DataTypeAttributes_Clear(void* pValue);

SOPC_StatusCode OpcUa_DataTypeAttributes_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_DataTypeAttributes_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_DataTypeAttributes_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ViewAttributes
/*============================================================================
 * The ViewAttributes structure.
 *===========================================================================*/
typedef struct _OpcUa_ViewAttributes
{
    uint32_t           SpecifiedAttributes;
    SOPC_LocalizedText DisplayName;
    SOPC_LocalizedText Description;
    uint32_t           WriteMask;
    uint32_t           UserWriteMask;
    SOPC_Boolean       ContainsNoLoops;
    SOPC_Byte          EventNotifier;
}
OpcUa_ViewAttributes;

void OpcUa_ViewAttributes_Initialize(void* pValue);

void OpcUa_ViewAttributes_Clear(void* pValue);

SOPC_StatusCode OpcUa_ViewAttributes_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ViewAttributes_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_ViewAttributes_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_AddNodesItem
/*============================================================================
 * The AddNodesItem structure.
 *===========================================================================*/
typedef struct _OpcUa_AddNodesItem
{
    SOPC_ExpandedNodeId  ParentNodeId;
    SOPC_NodeId          ReferenceTypeId;
    SOPC_ExpandedNodeId  RequestedNewNodeId;
    SOPC_QualifiedName   BrowseName;
    OpcUa_NodeClass      NodeClass;
    SOPC_ExtensionObject NodeAttributes;
    SOPC_ExpandedNodeId  TypeDefinition;
}
OpcUa_AddNodesItem;

void OpcUa_AddNodesItem_Initialize(void* pValue);

void OpcUa_AddNodesItem_Clear(void* pValue);

SOPC_StatusCode OpcUa_AddNodesItem_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_AddNodesItem_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_AddNodesItem_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_AddNodesResult
/*============================================================================
 * The AddNodesResult structure.
 *===========================================================================*/
typedef struct _OpcUa_AddNodesResult
{
    SOPC_StatusCode StatusCode;
    SOPC_NodeId     AddedNodeId;
}
OpcUa_AddNodesResult;

void OpcUa_AddNodesResult_Initialize(void* pValue);

void OpcUa_AddNodesResult_Clear(void* pValue);

SOPC_StatusCode OpcUa_AddNodesResult_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_AddNodesResult_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_AddNodesResult_EncodeableType;
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

void OpcUa_AddNodesRequest_Initialize(void* pValue);

void OpcUa_AddNodesRequest_Clear(void* pValue);

SOPC_StatusCode OpcUa_AddNodesRequest_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_AddNodesRequest_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_AddNodesRequest_EncodeableType;
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
    SOPC_DiagnosticInfo*  DiagnosticInfos;
}
OpcUa_AddNodesResponse;

void OpcUa_AddNodesResponse_Initialize(void* pValue);

void OpcUa_AddNodesResponse_Clear(void* pValue);

SOPC_StatusCode OpcUa_AddNodesResponse_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_AddNodesResponse_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_AddNodesResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_AddReferencesItem
/*============================================================================
 * The AddReferencesItem structure.
 *===========================================================================*/
typedef struct _OpcUa_AddReferencesItem
{
    SOPC_NodeId         SourceNodeId;
    SOPC_NodeId         ReferenceTypeId;
    SOPC_Boolean        IsForward;
    SOPC_String         TargetServerUri;
    SOPC_ExpandedNodeId TargetNodeId;
    OpcUa_NodeClass     TargetNodeClass;
}
OpcUa_AddReferencesItem;

void OpcUa_AddReferencesItem_Initialize(void* pValue);

void OpcUa_AddReferencesItem_Clear(void* pValue);

SOPC_StatusCode OpcUa_AddReferencesItem_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_AddReferencesItem_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_AddReferencesItem_EncodeableType;
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

void OpcUa_AddReferencesRequest_Initialize(void* pValue);

void OpcUa_AddReferencesRequest_Clear(void* pValue);

SOPC_StatusCode OpcUa_AddReferencesRequest_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_AddReferencesRequest_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_AddReferencesRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_AddReferencesResponse
/*============================================================================
 * The AddReferencesResponse structure.
 *===========================================================================*/
typedef struct _OpcUa_AddReferencesResponse
{
    OpcUa_ResponseHeader ResponseHeader;
    int32_t              NoOfResults;
    SOPC_StatusCode*     Results;
    int32_t              NoOfDiagnosticInfos;
    SOPC_DiagnosticInfo* DiagnosticInfos;
}
OpcUa_AddReferencesResponse;

void OpcUa_AddReferencesResponse_Initialize(void* pValue);

void OpcUa_AddReferencesResponse_Clear(void* pValue);

SOPC_StatusCode OpcUa_AddReferencesResponse_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_AddReferencesResponse_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_AddReferencesResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_DeleteNodesItem
/*============================================================================
 * The DeleteNodesItem structure.
 *===========================================================================*/
typedef struct _OpcUa_DeleteNodesItem
{
    SOPC_NodeId  NodeId;
    SOPC_Boolean DeleteTargetReferences;
}
OpcUa_DeleteNodesItem;

void OpcUa_DeleteNodesItem_Initialize(void* pValue);

void OpcUa_DeleteNodesItem_Clear(void* pValue);

SOPC_StatusCode OpcUa_DeleteNodesItem_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_DeleteNodesItem_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_DeleteNodesItem_EncodeableType;
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

void OpcUa_DeleteNodesRequest_Initialize(void* pValue);

void OpcUa_DeleteNodesRequest_Clear(void* pValue);

SOPC_StatusCode OpcUa_DeleteNodesRequest_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_DeleteNodesRequest_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_DeleteNodesRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_DeleteNodesResponse
/*============================================================================
 * The DeleteNodesResponse structure.
 *===========================================================================*/
typedef struct _OpcUa_DeleteNodesResponse
{
    OpcUa_ResponseHeader ResponseHeader;
    int32_t              NoOfResults;
    SOPC_StatusCode*     Results;
    int32_t              NoOfDiagnosticInfos;
    SOPC_DiagnosticInfo* DiagnosticInfos;
}
OpcUa_DeleteNodesResponse;

void OpcUa_DeleteNodesResponse_Initialize(void* pValue);

void OpcUa_DeleteNodesResponse_Clear(void* pValue);

SOPC_StatusCode OpcUa_DeleteNodesResponse_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_DeleteNodesResponse_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_DeleteNodesResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_DeleteReferencesItem
/*============================================================================
 * The DeleteReferencesItem structure.
 *===========================================================================*/
typedef struct _OpcUa_DeleteReferencesItem
{
    SOPC_NodeId         SourceNodeId;
    SOPC_NodeId         ReferenceTypeId;
    SOPC_Boolean        IsForward;
    SOPC_ExpandedNodeId TargetNodeId;
    SOPC_Boolean        DeleteBidirectional;
}
OpcUa_DeleteReferencesItem;

void OpcUa_DeleteReferencesItem_Initialize(void* pValue);

void OpcUa_DeleteReferencesItem_Clear(void* pValue);

SOPC_StatusCode OpcUa_DeleteReferencesItem_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_DeleteReferencesItem_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_DeleteReferencesItem_EncodeableType;
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

void OpcUa_DeleteReferencesRequest_Initialize(void* pValue);

void OpcUa_DeleteReferencesRequest_Clear(void* pValue);

SOPC_StatusCode OpcUa_DeleteReferencesRequest_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_DeleteReferencesRequest_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_DeleteReferencesRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_DeleteReferencesResponse
/*============================================================================
 * The DeleteReferencesResponse structure.
 *===========================================================================*/
typedef struct _OpcUa_DeleteReferencesResponse
{
    OpcUa_ResponseHeader ResponseHeader;
    int32_t              NoOfResults;
    SOPC_StatusCode*     Results;
    int32_t              NoOfDiagnosticInfos;
    SOPC_DiagnosticInfo* DiagnosticInfos;
}
OpcUa_DeleteReferencesResponse;

void OpcUa_DeleteReferencesResponse_Initialize(void* pValue);

void OpcUa_DeleteReferencesResponse_Clear(void* pValue);

SOPC_StatusCode OpcUa_DeleteReferencesResponse_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_DeleteReferencesResponse_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_DeleteReferencesResponse_EncodeableType;
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
    SOPC_NodeId   ViewId;
    SOPC_DateTime Timestamp;
    uint32_t      ViewVersion;
}
OpcUa_ViewDescription;

void OpcUa_ViewDescription_Initialize(void* pValue);

void OpcUa_ViewDescription_Clear(void* pValue);

SOPC_StatusCode OpcUa_ViewDescription_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ViewDescription_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_ViewDescription_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_BrowseDescription
/*============================================================================
 * The BrowseDescription structure.
 *===========================================================================*/
typedef struct _OpcUa_BrowseDescription
{
    SOPC_NodeId           NodeId;
    OpcUa_BrowseDirection BrowseDirection;
    SOPC_NodeId           ReferenceTypeId;
    SOPC_Boolean          IncludeSubtypes;
    uint32_t              NodeClassMask;
    uint32_t              ResultMask;
}
OpcUa_BrowseDescription;

void OpcUa_BrowseDescription_Initialize(void* pValue);

void OpcUa_BrowseDescription_Clear(void* pValue);

SOPC_StatusCode OpcUa_BrowseDescription_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_BrowseDescription_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_BrowseDescription_EncodeableType;
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
    SOPC_NodeId         ReferenceTypeId;
    SOPC_Boolean        IsForward;
    SOPC_ExpandedNodeId NodeId;
    SOPC_QualifiedName  BrowseName;
    SOPC_LocalizedText  DisplayName;
    OpcUa_NodeClass     NodeClass;
    SOPC_ExpandedNodeId TypeDefinition;
}
OpcUa_ReferenceDescription;

void OpcUa_ReferenceDescription_Initialize(void* pValue);

void OpcUa_ReferenceDescription_Clear(void* pValue);

SOPC_StatusCode OpcUa_ReferenceDescription_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ReferenceDescription_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_ReferenceDescription_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_BrowseResult
/*============================================================================
 * The BrowseResult structure.
 *===========================================================================*/
typedef struct _OpcUa_BrowseResult
{
    SOPC_StatusCode             StatusCode;
    SOPC_ByteString             ContinuationPoint;
    int32_t                     NoOfReferences;
    OpcUa_ReferenceDescription* References;
}
OpcUa_BrowseResult;

void OpcUa_BrowseResult_Initialize(void* pValue);

void OpcUa_BrowseResult_Clear(void* pValue);

SOPC_StatusCode OpcUa_BrowseResult_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_BrowseResult_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_BrowseResult_EncodeableType;
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

void OpcUa_BrowseRequest_Initialize(void* pValue);

void OpcUa_BrowseRequest_Clear(void* pValue);

SOPC_StatusCode OpcUa_BrowseRequest_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_BrowseRequest_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_BrowseRequest_EncodeableType;
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
    SOPC_DiagnosticInfo* DiagnosticInfos;
}
OpcUa_BrowseResponse;

void OpcUa_BrowseResponse_Initialize(void* pValue);

void OpcUa_BrowseResponse_Clear(void* pValue);

SOPC_StatusCode OpcUa_BrowseResponse_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_BrowseResponse_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_BrowseResponse_EncodeableType;
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
    SOPC_Boolean        ReleaseContinuationPoints;
    int32_t             NoOfContinuationPoints;
    SOPC_ByteString*    ContinuationPoints;
}
OpcUa_BrowseNextRequest;

void OpcUa_BrowseNextRequest_Initialize(void* pValue);

void OpcUa_BrowseNextRequest_Clear(void* pValue);

SOPC_StatusCode OpcUa_BrowseNextRequest_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_BrowseNextRequest_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_BrowseNextRequest_EncodeableType;
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
    SOPC_DiagnosticInfo* DiagnosticInfos;
}
OpcUa_BrowseNextResponse;

void OpcUa_BrowseNextResponse_Initialize(void* pValue);

void OpcUa_BrowseNextResponse_Clear(void* pValue);

SOPC_StatusCode OpcUa_BrowseNextResponse_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_BrowseNextResponse_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_BrowseNextResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_RelativePathElement
/*============================================================================
 * The RelativePathElement structure.
 *===========================================================================*/
typedef struct _OpcUa_RelativePathElement
{
    SOPC_NodeId        ReferenceTypeId;
    SOPC_Boolean       IsInverse;
    SOPC_Boolean       IncludeSubtypes;
    SOPC_QualifiedName TargetName;
}
OpcUa_RelativePathElement;

void OpcUa_RelativePathElement_Initialize(void* pValue);

void OpcUa_RelativePathElement_Clear(void* pValue);

SOPC_StatusCode OpcUa_RelativePathElement_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_RelativePathElement_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_RelativePathElement_EncodeableType;
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

void OpcUa_RelativePath_Initialize(void* pValue);

void OpcUa_RelativePath_Clear(void* pValue);

SOPC_StatusCode OpcUa_RelativePath_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_RelativePath_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_RelativePath_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_BrowsePath
/*============================================================================
 * The BrowsePath structure.
 *===========================================================================*/
typedef struct _OpcUa_BrowsePath
{
    SOPC_NodeId        StartingNode;
    OpcUa_RelativePath RelativePath;
}
OpcUa_BrowsePath;

void OpcUa_BrowsePath_Initialize(void* pValue);

void OpcUa_BrowsePath_Clear(void* pValue);

SOPC_StatusCode OpcUa_BrowsePath_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_BrowsePath_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_BrowsePath_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_BrowsePathTarget
/*============================================================================
 * The BrowsePathTarget structure.
 *===========================================================================*/
typedef struct _OpcUa_BrowsePathTarget
{
    SOPC_ExpandedNodeId TargetId;
    uint32_t            RemainingPathIndex;
}
OpcUa_BrowsePathTarget;

void OpcUa_BrowsePathTarget_Initialize(void* pValue);

void OpcUa_BrowsePathTarget_Clear(void* pValue);

SOPC_StatusCode OpcUa_BrowsePathTarget_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_BrowsePathTarget_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_BrowsePathTarget_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_BrowsePathResult
/*============================================================================
 * The BrowsePathResult structure.
 *===========================================================================*/
typedef struct _OpcUa_BrowsePathResult
{
    SOPC_StatusCode         StatusCode;
    int32_t                 NoOfTargets;
    OpcUa_BrowsePathTarget* Targets;
}
OpcUa_BrowsePathResult;

void OpcUa_BrowsePathResult_Initialize(void* pValue);

void OpcUa_BrowsePathResult_Clear(void* pValue);

SOPC_StatusCode OpcUa_BrowsePathResult_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_BrowsePathResult_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_BrowsePathResult_EncodeableType;
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

void OpcUa_TranslateBrowsePathsToNodeIdsRequest_Initialize(void* pValue);

void OpcUa_TranslateBrowsePathsToNodeIdsRequest_Clear(void* pValue);

SOPC_StatusCode OpcUa_TranslateBrowsePathsToNodeIdsRequest_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_TranslateBrowsePathsToNodeIdsRequest_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_TranslateBrowsePathsToNodeIdsRequest_EncodeableType;
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
    SOPC_DiagnosticInfo*    DiagnosticInfos;
}
OpcUa_TranslateBrowsePathsToNodeIdsResponse;

void OpcUa_TranslateBrowsePathsToNodeIdsResponse_Initialize(void* pValue);

void OpcUa_TranslateBrowsePathsToNodeIdsResponse_Clear(void* pValue);

SOPC_StatusCode OpcUa_TranslateBrowsePathsToNodeIdsResponse_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_TranslateBrowsePathsToNodeIdsResponse_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_TranslateBrowsePathsToNodeIdsResponse_EncodeableType;
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
    SOPC_NodeId*        NodesToRegister;
}
OpcUa_RegisterNodesRequest;

void OpcUa_RegisterNodesRequest_Initialize(void* pValue);

void OpcUa_RegisterNodesRequest_Clear(void* pValue);

SOPC_StatusCode OpcUa_RegisterNodesRequest_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_RegisterNodesRequest_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_RegisterNodesRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_RegisterNodesResponse
/*============================================================================
 * The RegisterNodesResponse structure.
 *===========================================================================*/
typedef struct _OpcUa_RegisterNodesResponse
{
    OpcUa_ResponseHeader ResponseHeader;
    int32_t              NoOfRegisteredNodeIds;
    SOPC_NodeId*         RegisteredNodeIds;
}
OpcUa_RegisterNodesResponse;

void OpcUa_RegisterNodesResponse_Initialize(void* pValue);

void OpcUa_RegisterNodesResponse_Clear(void* pValue);

SOPC_StatusCode OpcUa_RegisterNodesResponse_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_RegisterNodesResponse_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_RegisterNodesResponse_EncodeableType;
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
    SOPC_NodeId*        NodesToUnregister;
}
OpcUa_UnregisterNodesRequest;

void OpcUa_UnregisterNodesRequest_Initialize(void* pValue);

void OpcUa_UnregisterNodesRequest_Clear(void* pValue);

SOPC_StatusCode OpcUa_UnregisterNodesRequest_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_UnregisterNodesRequest_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_UnregisterNodesRequest_EncodeableType;
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

void OpcUa_UnregisterNodesResponse_Initialize(void* pValue);

void OpcUa_UnregisterNodesResponse_Clear(void* pValue);

SOPC_StatusCode OpcUa_UnregisterNodesResponse_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_UnregisterNodesResponse_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_UnregisterNodesResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_EndpointConfiguration
/*============================================================================
 * The EndpointConfiguration structure.
 *===========================================================================*/
typedef struct _OpcUa_EndpointConfiguration
{
    int32_t      OperationTimeout;
    SOPC_Boolean UseBinaryEncoding;
    int32_t      MaxStringLength;
    int32_t      MaxByteStringLength;
    int32_t      MaxArrayLength;
    int32_t      MaxMessageSize;
    int32_t      MaxBufferSize;
    int32_t      ChannelLifetime;
    int32_t      SecurityTokenLifetime;
}
OpcUa_EndpointConfiguration;

void OpcUa_EndpointConfiguration_Initialize(void* pValue);

void OpcUa_EndpointConfiguration_Clear(void* pValue);

SOPC_StatusCode OpcUa_EndpointConfiguration_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_EndpointConfiguration_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_EndpointConfiguration_EncodeableType;
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
    SOPC_String           OrganizationUri;
    SOPC_String           ProfileId;
    SOPC_String           ComplianceTool;
    SOPC_DateTime         ComplianceDate;
    OpcUa_ComplianceLevel ComplianceLevel;
    int32_t               NoOfUnsupportedUnitIds;
    SOPC_String*          UnsupportedUnitIds;
}
OpcUa_SupportedProfile;

void OpcUa_SupportedProfile_Initialize(void* pValue);

void OpcUa_SupportedProfile_Clear(void* pValue);

SOPC_StatusCode OpcUa_SupportedProfile_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_SupportedProfile_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_SupportedProfile_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_SoftwareCertificate
/*============================================================================
 * The SoftwareCertificate structure.
 *===========================================================================*/
typedef struct _OpcUa_SoftwareCertificate
{
    SOPC_String             ProductName;
    SOPC_String             ProductUri;
    SOPC_String             VendorName;
    SOPC_ByteString         VendorProductCertificate;
    SOPC_String             SoftwareVersion;
    SOPC_String             BuildNumber;
    SOPC_DateTime           BuildDate;
    SOPC_String             IssuedBy;
    SOPC_DateTime           IssueDate;
    int32_t                 NoOfSupportedProfiles;
    OpcUa_SupportedProfile* SupportedProfiles;
}
OpcUa_SoftwareCertificate;

void OpcUa_SoftwareCertificate_Initialize(void* pValue);

void OpcUa_SoftwareCertificate_Clear(void* pValue);

SOPC_StatusCode OpcUa_SoftwareCertificate_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_SoftwareCertificate_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_SoftwareCertificate_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_QueryDataDescription
/*============================================================================
 * The QueryDataDescription structure.
 *===========================================================================*/
typedef struct _OpcUa_QueryDataDescription
{
    OpcUa_RelativePath RelativePath;
    uint32_t           AttributeId;
    SOPC_String        IndexRange;
}
OpcUa_QueryDataDescription;

void OpcUa_QueryDataDescription_Initialize(void* pValue);

void OpcUa_QueryDataDescription_Clear(void* pValue);

SOPC_StatusCode OpcUa_QueryDataDescription_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_QueryDataDescription_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_QueryDataDescription_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_NodeTypeDescription
/*============================================================================
 * The NodeTypeDescription structure.
 *===========================================================================*/
typedef struct _OpcUa_NodeTypeDescription
{
    SOPC_ExpandedNodeId         TypeDefinitionNode;
    SOPC_Boolean                IncludeSubTypes;
    int32_t                     NoOfDataToReturn;
    OpcUa_QueryDataDescription* DataToReturn;
}
OpcUa_NodeTypeDescription;

void OpcUa_NodeTypeDescription_Initialize(void* pValue);

void OpcUa_NodeTypeDescription_Clear(void* pValue);

SOPC_StatusCode OpcUa_NodeTypeDescription_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_NodeTypeDescription_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_NodeTypeDescription_EncodeableType;
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
    SOPC_ExpandedNodeId NodeId;
    SOPC_ExpandedNodeId TypeDefinitionNode;
    int32_t             NoOfValues;
    SOPC_Variant*       Values;
}
OpcUa_QueryDataSet;

void OpcUa_QueryDataSet_Initialize(void* pValue);

void OpcUa_QueryDataSet_Clear(void* pValue);

SOPC_StatusCode OpcUa_QueryDataSet_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_QueryDataSet_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_QueryDataSet_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_NodeReference
/*============================================================================
 * The NodeReference structure.
 *===========================================================================*/
typedef struct _OpcUa_NodeReference
{
    SOPC_NodeId  NodeId;
    SOPC_NodeId  ReferenceTypeId;
    SOPC_Boolean IsForward;
    int32_t      NoOfReferencedNodeIds;
    SOPC_NodeId* ReferencedNodeIds;
}
OpcUa_NodeReference;

void OpcUa_NodeReference_Initialize(void* pValue);

void OpcUa_NodeReference_Clear(void* pValue);

SOPC_StatusCode OpcUa_NodeReference_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_NodeReference_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_NodeReference_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ContentFilterElement
/*============================================================================
 * The ContentFilterElement structure.
 *===========================================================================*/
typedef struct _OpcUa_ContentFilterElement
{
    OpcUa_FilterOperator  FilterOperator;
    int32_t               NoOfFilterOperands;
    SOPC_ExtensionObject* FilterOperands;
}
OpcUa_ContentFilterElement;

void OpcUa_ContentFilterElement_Initialize(void* pValue);

void OpcUa_ContentFilterElement_Clear(void* pValue);

SOPC_StatusCode OpcUa_ContentFilterElement_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ContentFilterElement_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_ContentFilterElement_EncodeableType;
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

void OpcUa_ContentFilter_Initialize(void* pValue);

void OpcUa_ContentFilter_Clear(void* pValue);

SOPC_StatusCode OpcUa_ContentFilter_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ContentFilter_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_ContentFilter_EncodeableType;
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

void OpcUa_ElementOperand_Initialize(void* pValue);

void OpcUa_ElementOperand_Clear(void* pValue);

SOPC_StatusCode OpcUa_ElementOperand_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ElementOperand_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_ElementOperand_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_LiteralOperand
/*============================================================================
 * The LiteralOperand structure.
 *===========================================================================*/
typedef struct _OpcUa_LiteralOperand
{
    SOPC_Variant Value;
}
OpcUa_LiteralOperand;

void OpcUa_LiteralOperand_Initialize(void* pValue);

void OpcUa_LiteralOperand_Clear(void* pValue);

SOPC_StatusCode OpcUa_LiteralOperand_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_LiteralOperand_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_LiteralOperand_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_AttributeOperand
/*============================================================================
 * The AttributeOperand structure.
 *===========================================================================*/
typedef struct _OpcUa_AttributeOperand
{
    SOPC_NodeId        NodeId;
    SOPC_String        Alias;
    OpcUa_RelativePath BrowsePath;
    uint32_t           AttributeId;
    SOPC_String        IndexRange;
}
OpcUa_AttributeOperand;

void OpcUa_AttributeOperand_Initialize(void* pValue);

void OpcUa_AttributeOperand_Clear(void* pValue);

SOPC_StatusCode OpcUa_AttributeOperand_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_AttributeOperand_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_AttributeOperand_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_SimpleAttributeOperand
/*============================================================================
 * The SimpleAttributeOperand structure.
 *===========================================================================*/
typedef struct _OpcUa_SimpleAttributeOperand
{
    SOPC_NodeId         TypeDefinitionId;
    int32_t             NoOfBrowsePath;
    SOPC_QualifiedName* BrowsePath;
    uint32_t            AttributeId;
    SOPC_String         IndexRange;
}
OpcUa_SimpleAttributeOperand;

void OpcUa_SimpleAttributeOperand_Initialize(void* pValue);

void OpcUa_SimpleAttributeOperand_Clear(void* pValue);

SOPC_StatusCode OpcUa_SimpleAttributeOperand_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_SimpleAttributeOperand_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_SimpleAttributeOperand_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ContentFilterElementResult
/*============================================================================
 * The ContentFilterElementResult structure.
 *===========================================================================*/
typedef struct _OpcUa_ContentFilterElementResult
{
    SOPC_StatusCode      StatusCode;
    int32_t              NoOfOperandStatusCodes;
    SOPC_StatusCode*     OperandStatusCodes;
    int32_t              NoOfOperandDiagnosticInfos;
    SOPC_DiagnosticInfo* OperandDiagnosticInfos;
}
OpcUa_ContentFilterElementResult;

void OpcUa_ContentFilterElementResult_Initialize(void* pValue);

void OpcUa_ContentFilterElementResult_Clear(void* pValue);

SOPC_StatusCode OpcUa_ContentFilterElementResult_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ContentFilterElementResult_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_ContentFilterElementResult_EncodeableType;
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
    SOPC_DiagnosticInfo*              ElementDiagnosticInfos;
}
OpcUa_ContentFilterResult;

void OpcUa_ContentFilterResult_Initialize(void* pValue);

void OpcUa_ContentFilterResult_Clear(void* pValue);

SOPC_StatusCode OpcUa_ContentFilterResult_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ContentFilterResult_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_ContentFilterResult_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ParsingResult
/*============================================================================
 * The ParsingResult structure.
 *===========================================================================*/
typedef struct _OpcUa_ParsingResult
{
    SOPC_StatusCode      StatusCode;
    int32_t              NoOfDataStatusCodes;
    SOPC_StatusCode*     DataStatusCodes;
    int32_t              NoOfDataDiagnosticInfos;
    SOPC_DiagnosticInfo* DataDiagnosticInfos;
}
OpcUa_ParsingResult;

void OpcUa_ParsingResult_Initialize(void* pValue);

void OpcUa_ParsingResult_Clear(void* pValue);

SOPC_StatusCode OpcUa_ParsingResult_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ParsingResult_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_ParsingResult_EncodeableType;
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

void OpcUa_QueryFirstRequest_Initialize(void* pValue);

void OpcUa_QueryFirstRequest_Clear(void* pValue);

SOPC_StatusCode OpcUa_QueryFirstRequest_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_QueryFirstRequest_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_QueryFirstRequest_EncodeableType;
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
    SOPC_ByteString           ContinuationPoint;
    int32_t                   NoOfParsingResults;
    OpcUa_ParsingResult*      ParsingResults;
    int32_t                   NoOfDiagnosticInfos;
    SOPC_DiagnosticInfo*      DiagnosticInfos;
    OpcUa_ContentFilterResult FilterResult;
}
OpcUa_QueryFirstResponse;

void OpcUa_QueryFirstResponse_Initialize(void* pValue);

void OpcUa_QueryFirstResponse_Clear(void* pValue);

SOPC_StatusCode OpcUa_QueryFirstResponse_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_QueryFirstResponse_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_QueryFirstResponse_EncodeableType;
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
    SOPC_Boolean        ReleaseContinuationPoint;
    SOPC_ByteString     ContinuationPoint;
}
OpcUa_QueryNextRequest;

void OpcUa_QueryNextRequest_Initialize(void* pValue);

void OpcUa_QueryNextRequest_Clear(void* pValue);

SOPC_StatusCode OpcUa_QueryNextRequest_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_QueryNextRequest_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_QueryNextRequest_EncodeableType;
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
    SOPC_ByteString      RevisedContinuationPoint;
}
OpcUa_QueryNextResponse;

void OpcUa_QueryNextResponse_Initialize(void* pValue);

void OpcUa_QueryNextResponse_Clear(void* pValue);

SOPC_StatusCode OpcUa_QueryNextResponse_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_QueryNextResponse_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_QueryNextResponse_EncodeableType;
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
    SOPC_NodeId        NodeId;
    uint32_t           AttributeId;
    SOPC_String        IndexRange;
    SOPC_QualifiedName DataEncoding;
}
OpcUa_ReadValueId;

void OpcUa_ReadValueId_Initialize(void* pValue);

void OpcUa_ReadValueId_Clear(void* pValue);

SOPC_StatusCode OpcUa_ReadValueId_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ReadValueId_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_ReadValueId_EncodeableType;
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

void OpcUa_ReadRequest_Initialize(void* pValue);

void OpcUa_ReadRequest_Clear(void* pValue);

SOPC_StatusCode OpcUa_ReadRequest_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ReadRequest_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_ReadRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ReadResponse
/*============================================================================
 * The ReadResponse structure.
 *===========================================================================*/
typedef struct _OpcUa_ReadResponse
{
    OpcUa_ResponseHeader ResponseHeader;
    int32_t              NoOfResults;
    SOPC_DataValue*      Results;
    int32_t              NoOfDiagnosticInfos;
    SOPC_DiagnosticInfo* DiagnosticInfos;
}
OpcUa_ReadResponse;

void OpcUa_ReadResponse_Initialize(void* pValue);

void OpcUa_ReadResponse_Clear(void* pValue);

SOPC_StatusCode OpcUa_ReadResponse_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ReadResponse_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_ReadResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_HistoryReadValueId
/*============================================================================
 * The HistoryReadValueId structure.
 *===========================================================================*/
typedef struct _OpcUa_HistoryReadValueId
{
    SOPC_NodeId        NodeId;
    SOPC_String        IndexRange;
    SOPC_QualifiedName DataEncoding;
    SOPC_ByteString    ContinuationPoint;
}
OpcUa_HistoryReadValueId;

void OpcUa_HistoryReadValueId_Initialize(void* pValue);

void OpcUa_HistoryReadValueId_Clear(void* pValue);

SOPC_StatusCode OpcUa_HistoryReadValueId_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_HistoryReadValueId_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_HistoryReadValueId_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_HistoryReadResult
/*============================================================================
 * The HistoryReadResult structure.
 *===========================================================================*/
typedef struct _OpcUa_HistoryReadResult
{
    SOPC_StatusCode      StatusCode;
    SOPC_ByteString      ContinuationPoint;
    SOPC_ExtensionObject HistoryData;
}
OpcUa_HistoryReadResult;

void OpcUa_HistoryReadResult_Initialize(void* pValue);

void OpcUa_HistoryReadResult_Clear(void* pValue);

SOPC_StatusCode OpcUa_HistoryReadResult_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_HistoryReadResult_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_HistoryReadResult_EncodeableType;
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

void OpcUa_EventFilter_Initialize(void* pValue);

void OpcUa_EventFilter_Clear(void* pValue);

SOPC_StatusCode OpcUa_EventFilter_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_EventFilter_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_EventFilter_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ReadEventDetails
/*============================================================================
 * The ReadEventDetails structure.
 *===========================================================================*/
typedef struct _OpcUa_ReadEventDetails
{
    uint32_t          NumValuesPerNode;
    SOPC_DateTime     StartTime;
    SOPC_DateTime     EndTime;
    OpcUa_EventFilter Filter;
}
OpcUa_ReadEventDetails;

void OpcUa_ReadEventDetails_Initialize(void* pValue);

void OpcUa_ReadEventDetails_Clear(void* pValue);

SOPC_StatusCode OpcUa_ReadEventDetails_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ReadEventDetails_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_ReadEventDetails_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ReadRawModifiedDetails
/*============================================================================
 * The ReadRawModifiedDetails structure.
 *===========================================================================*/
typedef struct _OpcUa_ReadRawModifiedDetails
{
    SOPC_Boolean  IsReadModified;
    SOPC_DateTime StartTime;
    SOPC_DateTime EndTime;
    uint32_t      NumValuesPerNode;
    SOPC_Boolean  ReturnBounds;
}
OpcUa_ReadRawModifiedDetails;

void OpcUa_ReadRawModifiedDetails_Initialize(void* pValue);

void OpcUa_ReadRawModifiedDetails_Clear(void* pValue);

SOPC_StatusCode OpcUa_ReadRawModifiedDetails_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ReadRawModifiedDetails_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_ReadRawModifiedDetails_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_AggregateConfiguration
/*============================================================================
 * The AggregateConfiguration structure.
 *===========================================================================*/
typedef struct _OpcUa_AggregateConfiguration
{
    SOPC_Boolean UseServerCapabilitiesDefaults;
    SOPC_Boolean TreatUncertainAsBad;
    SOPC_Byte    PercentDataBad;
    SOPC_Byte    PercentDataGood;
    SOPC_Boolean UseSlopedExtrapolation;
}
OpcUa_AggregateConfiguration;

void OpcUa_AggregateConfiguration_Initialize(void* pValue);

void OpcUa_AggregateConfiguration_Clear(void* pValue);

SOPC_StatusCode OpcUa_AggregateConfiguration_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_AggregateConfiguration_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_AggregateConfiguration_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ReadProcessedDetails
/*============================================================================
 * The ReadProcessedDetails structure.
 *===========================================================================*/
typedef struct _OpcUa_ReadProcessedDetails
{
    SOPC_DateTime                StartTime;
    SOPC_DateTime                EndTime;
    double                       ProcessingInterval;
    int32_t                      NoOfAggregateType;
    SOPC_NodeId*                 AggregateType;
    OpcUa_AggregateConfiguration AggregateConfiguration;
}
OpcUa_ReadProcessedDetails;

void OpcUa_ReadProcessedDetails_Initialize(void* pValue);

void OpcUa_ReadProcessedDetails_Clear(void* pValue);

SOPC_StatusCode OpcUa_ReadProcessedDetails_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ReadProcessedDetails_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_ReadProcessedDetails_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ReadAtTimeDetails
/*============================================================================
 * The ReadAtTimeDetails structure.
 *===========================================================================*/
typedef struct _OpcUa_ReadAtTimeDetails
{
    int32_t        NoOfReqTimes;
    SOPC_DateTime* ReqTimes;
    SOPC_Boolean   UseSimpleBounds;
}
OpcUa_ReadAtTimeDetails;

void OpcUa_ReadAtTimeDetails_Initialize(void* pValue);

void OpcUa_ReadAtTimeDetails_Clear(void* pValue);

SOPC_StatusCode OpcUa_ReadAtTimeDetails_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ReadAtTimeDetails_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_ReadAtTimeDetails_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_HistoryData
/*============================================================================
 * The HistoryData structure.
 *===========================================================================*/
typedef struct _OpcUa_HistoryData
{
    int32_t         NoOfDataValues;
    SOPC_DataValue* DataValues;
}
OpcUa_HistoryData;

void OpcUa_HistoryData_Initialize(void* pValue);

void OpcUa_HistoryData_Clear(void* pValue);

SOPC_StatusCode OpcUa_HistoryData_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_HistoryData_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_HistoryData_EncodeableType;
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
    SOPC_DateTime           ModificationTime;
    OpcUa_HistoryUpdateType UpdateType;
    SOPC_String             UserName;
}
OpcUa_ModificationInfo;

void OpcUa_ModificationInfo_Initialize(void* pValue);

void OpcUa_ModificationInfo_Clear(void* pValue);

SOPC_StatusCode OpcUa_ModificationInfo_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ModificationInfo_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_ModificationInfo_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_HistoryModifiedData
/*============================================================================
 * The HistoryModifiedData structure.
 *===========================================================================*/
typedef struct _OpcUa_HistoryModifiedData
{
    int32_t                 NoOfDataValues;
    SOPC_DataValue*         DataValues;
    int32_t                 NoOfModificationInfos;
    OpcUa_ModificationInfo* ModificationInfos;
}
OpcUa_HistoryModifiedData;

void OpcUa_HistoryModifiedData_Initialize(void* pValue);

void OpcUa_HistoryModifiedData_Clear(void* pValue);

SOPC_StatusCode OpcUa_HistoryModifiedData_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_HistoryModifiedData_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_HistoryModifiedData_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_HistoryEventFieldList
/*============================================================================
 * The HistoryEventFieldList structure.
 *===========================================================================*/
typedef struct _OpcUa_HistoryEventFieldList
{
    int32_t       NoOfEventFields;
    SOPC_Variant* EventFields;
}
OpcUa_HistoryEventFieldList;

void OpcUa_HistoryEventFieldList_Initialize(void* pValue);

void OpcUa_HistoryEventFieldList_Clear(void* pValue);

SOPC_StatusCode OpcUa_HistoryEventFieldList_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_HistoryEventFieldList_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_HistoryEventFieldList_EncodeableType;
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

void OpcUa_HistoryEvent_Initialize(void* pValue);

void OpcUa_HistoryEvent_Clear(void* pValue);

SOPC_StatusCode OpcUa_HistoryEvent_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_HistoryEvent_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_HistoryEvent_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_HistoryRead
#ifndef OPCUA_EXCLUDE_HistoryReadRequest
/*============================================================================
 * The HistoryReadRequest structure.
 *===========================================================================*/
typedef struct _OpcUa_HistoryReadRequest
{
    OpcUa_RequestHeader       RequestHeader;
    SOPC_ExtensionObject      HistoryReadDetails;
    OpcUa_TimestampsToReturn  TimestampsToReturn;
    SOPC_Boolean              ReleaseContinuationPoints;
    int32_t                   NoOfNodesToRead;
    OpcUa_HistoryReadValueId* NodesToRead;
}
OpcUa_HistoryReadRequest;

void OpcUa_HistoryReadRequest_Initialize(void* pValue);

void OpcUa_HistoryReadRequest_Clear(void* pValue);

SOPC_StatusCode OpcUa_HistoryReadRequest_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_HistoryReadRequest_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_HistoryReadRequest_EncodeableType;
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
    SOPC_DiagnosticInfo*     DiagnosticInfos;
}
OpcUa_HistoryReadResponse;

void OpcUa_HistoryReadResponse_Initialize(void* pValue);

void OpcUa_HistoryReadResponse_Clear(void* pValue);

SOPC_StatusCode OpcUa_HistoryReadResponse_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_HistoryReadResponse_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_HistoryReadResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_WriteValue
/*============================================================================
 * The WriteValue structure.
 *===========================================================================*/
typedef struct _OpcUa_WriteValue
{
    SOPC_NodeId    NodeId;
    uint32_t       AttributeId;
    SOPC_String    IndexRange;
    SOPC_DataValue Value;
}
OpcUa_WriteValue;

void OpcUa_WriteValue_Initialize(void* pValue);

void OpcUa_WriteValue_Clear(void* pValue);

SOPC_StatusCode OpcUa_WriteValue_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_WriteValue_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_WriteValue_EncodeableType;
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

void OpcUa_WriteRequest_Initialize(void* pValue);

void OpcUa_WriteRequest_Clear(void* pValue);

SOPC_StatusCode OpcUa_WriteRequest_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_WriteRequest_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_WriteRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_WriteResponse
/*============================================================================
 * The WriteResponse structure.
 *===========================================================================*/
typedef struct _OpcUa_WriteResponse
{
    OpcUa_ResponseHeader ResponseHeader;
    int32_t              NoOfResults;
    SOPC_StatusCode*     Results;
    int32_t              NoOfDiagnosticInfos;
    SOPC_DiagnosticInfo* DiagnosticInfos;
}
OpcUa_WriteResponse;

void OpcUa_WriteResponse_Initialize(void* pValue);

void OpcUa_WriteResponse_Clear(void* pValue);

SOPC_StatusCode OpcUa_WriteResponse_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_WriteResponse_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_WriteResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_HistoryUpdateDetails
/*============================================================================
 * The HistoryUpdateDetails structure.
 *===========================================================================*/
typedef struct _OpcUa_HistoryUpdateDetails
{
    SOPC_NodeId NodeId;
}
OpcUa_HistoryUpdateDetails;

void OpcUa_HistoryUpdateDetails_Initialize(void* pValue);

void OpcUa_HistoryUpdateDetails_Clear(void* pValue);

SOPC_StatusCode OpcUa_HistoryUpdateDetails_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_HistoryUpdateDetails_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_HistoryUpdateDetails_EncodeableType;
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
    SOPC_NodeId             NodeId;
    OpcUa_PerformUpdateType PerformInsertReplace;
    int32_t                 NoOfUpdateValues;
    SOPC_DataValue*         UpdateValues;
}
OpcUa_UpdateDataDetails;

void OpcUa_UpdateDataDetails_Initialize(void* pValue);

void OpcUa_UpdateDataDetails_Clear(void* pValue);

SOPC_StatusCode OpcUa_UpdateDataDetails_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_UpdateDataDetails_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_UpdateDataDetails_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_UpdateStructureDataDetails
/*============================================================================
 * The UpdateStructureDataDetails structure.
 *===========================================================================*/
typedef struct _OpcUa_UpdateStructureDataDetails
{
    SOPC_NodeId             NodeId;
    OpcUa_PerformUpdateType PerformInsertReplace;
    int32_t                 NoOfUpdateValues;
    SOPC_DataValue*         UpdateValues;
}
OpcUa_UpdateStructureDataDetails;

void OpcUa_UpdateStructureDataDetails_Initialize(void* pValue);

void OpcUa_UpdateStructureDataDetails_Clear(void* pValue);

SOPC_StatusCode OpcUa_UpdateStructureDataDetails_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_UpdateStructureDataDetails_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_UpdateStructureDataDetails_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_UpdateEventDetails
/*============================================================================
 * The UpdateEventDetails structure.
 *===========================================================================*/
typedef struct _OpcUa_UpdateEventDetails
{
    SOPC_NodeId                  NodeId;
    OpcUa_PerformUpdateType      PerformInsertReplace;
    OpcUa_EventFilter            Filter;
    int32_t                      NoOfEventData;
    OpcUa_HistoryEventFieldList* EventData;
}
OpcUa_UpdateEventDetails;

void OpcUa_UpdateEventDetails_Initialize(void* pValue);

void OpcUa_UpdateEventDetails_Clear(void* pValue);

SOPC_StatusCode OpcUa_UpdateEventDetails_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_UpdateEventDetails_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_UpdateEventDetails_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_DeleteRawModifiedDetails
/*============================================================================
 * The DeleteRawModifiedDetails structure.
 *===========================================================================*/
typedef struct _OpcUa_DeleteRawModifiedDetails
{
    SOPC_NodeId   NodeId;
    SOPC_Boolean  IsDeleteModified;
    SOPC_DateTime StartTime;
    SOPC_DateTime EndTime;
}
OpcUa_DeleteRawModifiedDetails;

void OpcUa_DeleteRawModifiedDetails_Initialize(void* pValue);

void OpcUa_DeleteRawModifiedDetails_Clear(void* pValue);

SOPC_StatusCode OpcUa_DeleteRawModifiedDetails_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_DeleteRawModifiedDetails_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_DeleteRawModifiedDetails_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_DeleteAtTimeDetails
/*============================================================================
 * The DeleteAtTimeDetails structure.
 *===========================================================================*/
typedef struct _OpcUa_DeleteAtTimeDetails
{
    SOPC_NodeId    NodeId;
    int32_t        NoOfReqTimes;
    SOPC_DateTime* ReqTimes;
}
OpcUa_DeleteAtTimeDetails;

void OpcUa_DeleteAtTimeDetails_Initialize(void* pValue);

void OpcUa_DeleteAtTimeDetails_Clear(void* pValue);

SOPC_StatusCode OpcUa_DeleteAtTimeDetails_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_DeleteAtTimeDetails_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_DeleteAtTimeDetails_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_DeleteEventDetails
/*============================================================================
 * The DeleteEventDetails structure.
 *===========================================================================*/
typedef struct _OpcUa_DeleteEventDetails
{
    SOPC_NodeId      NodeId;
    int32_t          NoOfEventIds;
    SOPC_ByteString* EventIds;
}
OpcUa_DeleteEventDetails;

void OpcUa_DeleteEventDetails_Initialize(void* pValue);

void OpcUa_DeleteEventDetails_Clear(void* pValue);

SOPC_StatusCode OpcUa_DeleteEventDetails_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_DeleteEventDetails_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_DeleteEventDetails_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_HistoryUpdateResult
/*============================================================================
 * The HistoryUpdateResult structure.
 *===========================================================================*/
typedef struct _OpcUa_HistoryUpdateResult
{
    SOPC_StatusCode      StatusCode;
    int32_t              NoOfOperationResults;
    SOPC_StatusCode*     OperationResults;
    int32_t              NoOfDiagnosticInfos;
    SOPC_DiagnosticInfo* DiagnosticInfos;
}
OpcUa_HistoryUpdateResult;

void OpcUa_HistoryUpdateResult_Initialize(void* pValue);

void OpcUa_HistoryUpdateResult_Clear(void* pValue);

SOPC_StatusCode OpcUa_HistoryUpdateResult_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_HistoryUpdateResult_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_HistoryUpdateResult_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_HistoryUpdate
#ifndef OPCUA_EXCLUDE_HistoryUpdateRequest
/*============================================================================
 * The HistoryUpdateRequest structure.
 *===========================================================================*/
typedef struct _OpcUa_HistoryUpdateRequest
{
    OpcUa_RequestHeader   RequestHeader;
    int32_t               NoOfHistoryUpdateDetails;
    SOPC_ExtensionObject* HistoryUpdateDetails;
}
OpcUa_HistoryUpdateRequest;

void OpcUa_HistoryUpdateRequest_Initialize(void* pValue);

void OpcUa_HistoryUpdateRequest_Clear(void* pValue);

SOPC_StatusCode OpcUa_HistoryUpdateRequest_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_HistoryUpdateRequest_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_HistoryUpdateRequest_EncodeableType;
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
    SOPC_DiagnosticInfo*       DiagnosticInfos;
}
OpcUa_HistoryUpdateResponse;

void OpcUa_HistoryUpdateResponse_Initialize(void* pValue);

void OpcUa_HistoryUpdateResponse_Clear(void* pValue);

SOPC_StatusCode OpcUa_HistoryUpdateResponse_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_HistoryUpdateResponse_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_HistoryUpdateResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_CallMethodRequest
/*============================================================================
 * The CallMethodRequest structure.
 *===========================================================================*/
typedef struct _OpcUa_CallMethodRequest
{
    SOPC_NodeId   ObjectId;
    SOPC_NodeId   MethodId;
    int32_t       NoOfInputArguments;
    SOPC_Variant* InputArguments;
}
OpcUa_CallMethodRequest;

void OpcUa_CallMethodRequest_Initialize(void* pValue);

void OpcUa_CallMethodRequest_Clear(void* pValue);

SOPC_StatusCode OpcUa_CallMethodRequest_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_CallMethodRequest_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_CallMethodRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_CallMethodResult
/*============================================================================
 * The CallMethodResult structure.
 *===========================================================================*/
typedef struct _OpcUa_CallMethodResult
{
    SOPC_StatusCode      StatusCode;
    int32_t              NoOfInputArgumentResults;
    SOPC_StatusCode*     InputArgumentResults;
    int32_t              NoOfInputArgumentDiagnosticInfos;
    SOPC_DiagnosticInfo* InputArgumentDiagnosticInfos;
    int32_t              NoOfOutputArguments;
    SOPC_Variant*        OutputArguments;
}
OpcUa_CallMethodResult;

void OpcUa_CallMethodResult_Initialize(void* pValue);

void OpcUa_CallMethodResult_Clear(void* pValue);

SOPC_StatusCode OpcUa_CallMethodResult_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_CallMethodResult_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_CallMethodResult_EncodeableType;
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

void OpcUa_CallRequest_Initialize(void* pValue);

void OpcUa_CallRequest_Clear(void* pValue);

SOPC_StatusCode OpcUa_CallRequest_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_CallRequest_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_CallRequest_EncodeableType;
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
    SOPC_DiagnosticInfo*    DiagnosticInfos;
}
OpcUa_CallResponse;

void OpcUa_CallResponse_Initialize(void* pValue);

void OpcUa_CallResponse_Clear(void* pValue);

SOPC_StatusCode OpcUa_CallResponse_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_CallResponse_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_CallResponse_EncodeableType;
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

void OpcUa_DataChangeFilter_Initialize(void* pValue);

void OpcUa_DataChangeFilter_Clear(void* pValue);

SOPC_StatusCode OpcUa_DataChangeFilter_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_DataChangeFilter_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_DataChangeFilter_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_AggregateFilter
/*============================================================================
 * The AggregateFilter structure.
 *===========================================================================*/
typedef struct _OpcUa_AggregateFilter
{
    SOPC_DateTime                StartTime;
    SOPC_NodeId                  AggregateType;
    double                       ProcessingInterval;
    OpcUa_AggregateConfiguration AggregateConfiguration;
}
OpcUa_AggregateFilter;

void OpcUa_AggregateFilter_Initialize(void* pValue);

void OpcUa_AggregateFilter_Clear(void* pValue);

SOPC_StatusCode OpcUa_AggregateFilter_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_AggregateFilter_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_AggregateFilter_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_EventFilterResult
/*============================================================================
 * The EventFilterResult structure.
 *===========================================================================*/
typedef struct _OpcUa_EventFilterResult
{
    int32_t                   NoOfSelectClauseResults;
    SOPC_StatusCode*          SelectClauseResults;
    int32_t                   NoOfSelectClauseDiagnosticInfos;
    SOPC_DiagnosticInfo*      SelectClauseDiagnosticInfos;
    OpcUa_ContentFilterResult WhereClauseResult;
}
OpcUa_EventFilterResult;

void OpcUa_EventFilterResult_Initialize(void* pValue);

void OpcUa_EventFilterResult_Clear(void* pValue);

SOPC_StatusCode OpcUa_EventFilterResult_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_EventFilterResult_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_EventFilterResult_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_AggregateFilterResult
/*============================================================================
 * The AggregateFilterResult structure.
 *===========================================================================*/
typedef struct _OpcUa_AggregateFilterResult
{
    SOPC_DateTime                RevisedStartTime;
    double                       RevisedProcessingInterval;
    OpcUa_AggregateConfiguration RevisedAggregateConfiguration;
}
OpcUa_AggregateFilterResult;

void OpcUa_AggregateFilterResult_Initialize(void* pValue);

void OpcUa_AggregateFilterResult_Clear(void* pValue);

SOPC_StatusCode OpcUa_AggregateFilterResult_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_AggregateFilterResult_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_AggregateFilterResult_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_MonitoringParameters
/*============================================================================
 * The MonitoringParameters structure.
 *===========================================================================*/
typedef struct _OpcUa_MonitoringParameters
{
    uint32_t             ClientHandle;
    double               SamplingInterval;
    SOPC_ExtensionObject Filter;
    uint32_t             QueueSize;
    SOPC_Boolean         DiscardOldest;
}
OpcUa_MonitoringParameters;

void OpcUa_MonitoringParameters_Initialize(void* pValue);

void OpcUa_MonitoringParameters_Clear(void* pValue);

SOPC_StatusCode OpcUa_MonitoringParameters_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_MonitoringParameters_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_MonitoringParameters_EncodeableType;
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

void OpcUa_MonitoredItemCreateRequest_Initialize(void* pValue);

void OpcUa_MonitoredItemCreateRequest_Clear(void* pValue);

SOPC_StatusCode OpcUa_MonitoredItemCreateRequest_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_MonitoredItemCreateRequest_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_MonitoredItemCreateRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_MonitoredItemCreateResult
/*============================================================================
 * The MonitoredItemCreateResult structure.
 *===========================================================================*/
typedef struct _OpcUa_MonitoredItemCreateResult
{
    SOPC_StatusCode      StatusCode;
    uint32_t             MonitoredItemId;
    double               RevisedSamplingInterval;
    uint32_t             RevisedQueueSize;
    SOPC_ExtensionObject FilterResult;
}
OpcUa_MonitoredItemCreateResult;

void OpcUa_MonitoredItemCreateResult_Initialize(void* pValue);

void OpcUa_MonitoredItemCreateResult_Clear(void* pValue);

SOPC_StatusCode OpcUa_MonitoredItemCreateResult_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_MonitoredItemCreateResult_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_MonitoredItemCreateResult_EncodeableType;
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

void OpcUa_CreateMonitoredItemsRequest_Initialize(void* pValue);

void OpcUa_CreateMonitoredItemsRequest_Clear(void* pValue);

SOPC_StatusCode OpcUa_CreateMonitoredItemsRequest_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_CreateMonitoredItemsRequest_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_CreateMonitoredItemsRequest_EncodeableType;
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
    SOPC_DiagnosticInfo*             DiagnosticInfos;
}
OpcUa_CreateMonitoredItemsResponse;

void OpcUa_CreateMonitoredItemsResponse_Initialize(void* pValue);

void OpcUa_CreateMonitoredItemsResponse_Clear(void* pValue);

SOPC_StatusCode OpcUa_CreateMonitoredItemsResponse_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_CreateMonitoredItemsResponse_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_CreateMonitoredItemsResponse_EncodeableType;
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

void OpcUa_MonitoredItemModifyRequest_Initialize(void* pValue);

void OpcUa_MonitoredItemModifyRequest_Clear(void* pValue);

SOPC_StatusCode OpcUa_MonitoredItemModifyRequest_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_MonitoredItemModifyRequest_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_MonitoredItemModifyRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_MonitoredItemModifyResult
/*============================================================================
 * The MonitoredItemModifyResult structure.
 *===========================================================================*/
typedef struct _OpcUa_MonitoredItemModifyResult
{
    SOPC_StatusCode      StatusCode;
    double               RevisedSamplingInterval;
    uint32_t             RevisedQueueSize;
    SOPC_ExtensionObject FilterResult;
}
OpcUa_MonitoredItemModifyResult;

void OpcUa_MonitoredItemModifyResult_Initialize(void* pValue);

void OpcUa_MonitoredItemModifyResult_Clear(void* pValue);

SOPC_StatusCode OpcUa_MonitoredItemModifyResult_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_MonitoredItemModifyResult_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_MonitoredItemModifyResult_EncodeableType;
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

void OpcUa_ModifyMonitoredItemsRequest_Initialize(void* pValue);

void OpcUa_ModifyMonitoredItemsRequest_Clear(void* pValue);

SOPC_StatusCode OpcUa_ModifyMonitoredItemsRequest_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ModifyMonitoredItemsRequest_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_ModifyMonitoredItemsRequest_EncodeableType;
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
    SOPC_DiagnosticInfo*             DiagnosticInfos;
}
OpcUa_ModifyMonitoredItemsResponse;

void OpcUa_ModifyMonitoredItemsResponse_Initialize(void* pValue);

void OpcUa_ModifyMonitoredItemsResponse_Clear(void* pValue);

SOPC_StatusCode OpcUa_ModifyMonitoredItemsResponse_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ModifyMonitoredItemsResponse_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_ModifyMonitoredItemsResponse_EncodeableType;
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

void OpcUa_SetMonitoringModeRequest_Initialize(void* pValue);

void OpcUa_SetMonitoringModeRequest_Clear(void* pValue);

SOPC_StatusCode OpcUa_SetMonitoringModeRequest_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_SetMonitoringModeRequest_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_SetMonitoringModeRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_SetMonitoringModeResponse
/*============================================================================
 * The SetMonitoringModeResponse structure.
 *===========================================================================*/
typedef struct _OpcUa_SetMonitoringModeResponse
{
    OpcUa_ResponseHeader ResponseHeader;
    int32_t              NoOfResults;
    SOPC_StatusCode*     Results;
    int32_t              NoOfDiagnosticInfos;
    SOPC_DiagnosticInfo* DiagnosticInfos;
}
OpcUa_SetMonitoringModeResponse;

void OpcUa_SetMonitoringModeResponse_Initialize(void* pValue);

void OpcUa_SetMonitoringModeResponse_Clear(void* pValue);

SOPC_StatusCode OpcUa_SetMonitoringModeResponse_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_SetMonitoringModeResponse_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_SetMonitoringModeResponse_EncodeableType;
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

void OpcUa_SetTriggeringRequest_Initialize(void* pValue);

void OpcUa_SetTriggeringRequest_Clear(void* pValue);

SOPC_StatusCode OpcUa_SetTriggeringRequest_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_SetTriggeringRequest_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_SetTriggeringRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_SetTriggeringResponse
/*============================================================================
 * The SetTriggeringResponse structure.
 *===========================================================================*/
typedef struct _OpcUa_SetTriggeringResponse
{
    OpcUa_ResponseHeader ResponseHeader;
    int32_t              NoOfAddResults;
    SOPC_StatusCode*     AddResults;
    int32_t              NoOfAddDiagnosticInfos;
    SOPC_DiagnosticInfo* AddDiagnosticInfos;
    int32_t              NoOfRemoveResults;
    SOPC_StatusCode*     RemoveResults;
    int32_t              NoOfRemoveDiagnosticInfos;
    SOPC_DiagnosticInfo* RemoveDiagnosticInfos;
}
OpcUa_SetTriggeringResponse;

void OpcUa_SetTriggeringResponse_Initialize(void* pValue);

void OpcUa_SetTriggeringResponse_Clear(void* pValue);

SOPC_StatusCode OpcUa_SetTriggeringResponse_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_SetTriggeringResponse_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_SetTriggeringResponse_EncodeableType;
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

void OpcUa_DeleteMonitoredItemsRequest_Initialize(void* pValue);

void OpcUa_DeleteMonitoredItemsRequest_Clear(void* pValue);

SOPC_StatusCode OpcUa_DeleteMonitoredItemsRequest_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_DeleteMonitoredItemsRequest_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_DeleteMonitoredItemsRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_DeleteMonitoredItemsResponse
/*============================================================================
 * The DeleteMonitoredItemsResponse structure.
 *===========================================================================*/
typedef struct _OpcUa_DeleteMonitoredItemsResponse
{
    OpcUa_ResponseHeader ResponseHeader;
    int32_t              NoOfResults;
    SOPC_StatusCode*     Results;
    int32_t              NoOfDiagnosticInfos;
    SOPC_DiagnosticInfo* DiagnosticInfos;
}
OpcUa_DeleteMonitoredItemsResponse;

void OpcUa_DeleteMonitoredItemsResponse_Initialize(void* pValue);

void OpcUa_DeleteMonitoredItemsResponse_Clear(void* pValue);

SOPC_StatusCode OpcUa_DeleteMonitoredItemsResponse_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_DeleteMonitoredItemsResponse_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_DeleteMonitoredItemsResponse_EncodeableType;
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
    SOPC_Boolean        PublishingEnabled;
    SOPC_Byte           Priority;
}
OpcUa_CreateSubscriptionRequest;

void OpcUa_CreateSubscriptionRequest_Initialize(void* pValue);

void OpcUa_CreateSubscriptionRequest_Clear(void* pValue);

SOPC_StatusCode OpcUa_CreateSubscriptionRequest_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_CreateSubscriptionRequest_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_CreateSubscriptionRequest_EncodeableType;
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

void OpcUa_CreateSubscriptionResponse_Initialize(void* pValue);

void OpcUa_CreateSubscriptionResponse_Clear(void* pValue);

SOPC_StatusCode OpcUa_CreateSubscriptionResponse_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_CreateSubscriptionResponse_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_CreateSubscriptionResponse_EncodeableType;
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
    SOPC_Byte           Priority;
}
OpcUa_ModifySubscriptionRequest;

void OpcUa_ModifySubscriptionRequest_Initialize(void* pValue);

void OpcUa_ModifySubscriptionRequest_Clear(void* pValue);

SOPC_StatusCode OpcUa_ModifySubscriptionRequest_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ModifySubscriptionRequest_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_ModifySubscriptionRequest_EncodeableType;
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

void OpcUa_ModifySubscriptionResponse_Initialize(void* pValue);

void OpcUa_ModifySubscriptionResponse_Clear(void* pValue);

SOPC_StatusCode OpcUa_ModifySubscriptionResponse_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ModifySubscriptionResponse_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_ModifySubscriptionResponse_EncodeableType;
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
    SOPC_Boolean        PublishingEnabled;
    int32_t             NoOfSubscriptionIds;
    uint32_t*           SubscriptionIds;
}
OpcUa_SetPublishingModeRequest;

void OpcUa_SetPublishingModeRequest_Initialize(void* pValue);

void OpcUa_SetPublishingModeRequest_Clear(void* pValue);

SOPC_StatusCode OpcUa_SetPublishingModeRequest_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_SetPublishingModeRequest_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_SetPublishingModeRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_SetPublishingModeResponse
/*============================================================================
 * The SetPublishingModeResponse structure.
 *===========================================================================*/
typedef struct _OpcUa_SetPublishingModeResponse
{
    OpcUa_ResponseHeader ResponseHeader;
    int32_t              NoOfResults;
    SOPC_StatusCode*     Results;
    int32_t              NoOfDiagnosticInfos;
    SOPC_DiagnosticInfo* DiagnosticInfos;
}
OpcUa_SetPublishingModeResponse;

void OpcUa_SetPublishingModeResponse_Initialize(void* pValue);

void OpcUa_SetPublishingModeResponse_Clear(void* pValue);

SOPC_StatusCode OpcUa_SetPublishingModeResponse_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_SetPublishingModeResponse_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_SetPublishingModeResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_NotificationMessage
/*============================================================================
 * The NotificationMessage structure.
 *===========================================================================*/
typedef struct _OpcUa_NotificationMessage
{
    uint32_t              SequenceNumber;
    SOPC_DateTime         PublishTime;
    int32_t               NoOfNotificationData;
    SOPC_ExtensionObject* NotificationData;
}
OpcUa_NotificationMessage;

void OpcUa_NotificationMessage_Initialize(void* pValue);

void OpcUa_NotificationMessage_Clear(void* pValue);

SOPC_StatusCode OpcUa_NotificationMessage_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_NotificationMessage_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_NotificationMessage_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_MonitoredItemNotification
/*============================================================================
 * The MonitoredItemNotification structure.
 *===========================================================================*/
typedef struct _OpcUa_MonitoredItemNotification
{
    uint32_t       ClientHandle;
    SOPC_DataValue Value;
}
OpcUa_MonitoredItemNotification;

void OpcUa_MonitoredItemNotification_Initialize(void* pValue);

void OpcUa_MonitoredItemNotification_Clear(void* pValue);

SOPC_StatusCode OpcUa_MonitoredItemNotification_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_MonitoredItemNotification_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_MonitoredItemNotification_EncodeableType;
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
    SOPC_DiagnosticInfo*             DiagnosticInfos;
}
OpcUa_DataChangeNotification;

void OpcUa_DataChangeNotification_Initialize(void* pValue);

void OpcUa_DataChangeNotification_Clear(void* pValue);

SOPC_StatusCode OpcUa_DataChangeNotification_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_DataChangeNotification_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_DataChangeNotification_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_EventFieldList
/*============================================================================
 * The EventFieldList structure.
 *===========================================================================*/
typedef struct _OpcUa_EventFieldList
{
    uint32_t      ClientHandle;
    int32_t       NoOfEventFields;
    SOPC_Variant* EventFields;
}
OpcUa_EventFieldList;

void OpcUa_EventFieldList_Initialize(void* pValue);

void OpcUa_EventFieldList_Clear(void* pValue);

SOPC_StatusCode OpcUa_EventFieldList_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_EventFieldList_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_EventFieldList_EncodeableType;
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

void OpcUa_EventNotificationList_Initialize(void* pValue);

void OpcUa_EventNotificationList_Clear(void* pValue);

SOPC_StatusCode OpcUa_EventNotificationList_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_EventNotificationList_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_EventNotificationList_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_StatusChangeNotification
/*============================================================================
 * The StatusChangeNotification structure.
 *===========================================================================*/
typedef struct _OpcUa_StatusChangeNotification
{
    SOPC_StatusCode     Status;
    SOPC_DiagnosticInfo DiagnosticInfo;
}
OpcUa_StatusChangeNotification;

void OpcUa_StatusChangeNotification_Initialize(void* pValue);

void OpcUa_StatusChangeNotification_Clear(void* pValue);

SOPC_StatusCode OpcUa_StatusChangeNotification_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_StatusChangeNotification_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_StatusChangeNotification_EncodeableType;
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

void OpcUa_SubscriptionAcknowledgement_Initialize(void* pValue);

void OpcUa_SubscriptionAcknowledgement_Clear(void* pValue);

SOPC_StatusCode OpcUa_SubscriptionAcknowledgement_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_SubscriptionAcknowledgement_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_SubscriptionAcknowledgement_EncodeableType;
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

void OpcUa_PublishRequest_Initialize(void* pValue);

void OpcUa_PublishRequest_Clear(void* pValue);

SOPC_StatusCode OpcUa_PublishRequest_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_PublishRequest_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_PublishRequest_EncodeableType;
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
    SOPC_Boolean              MoreNotifications;
    OpcUa_NotificationMessage NotificationMessage;
    int32_t                   NoOfResults;
    SOPC_StatusCode*          Results;
    int32_t                   NoOfDiagnosticInfos;
    SOPC_DiagnosticInfo*      DiagnosticInfos;
}
OpcUa_PublishResponse;

void OpcUa_PublishResponse_Initialize(void* pValue);

void OpcUa_PublishResponse_Clear(void* pValue);

SOPC_StatusCode OpcUa_PublishResponse_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_PublishResponse_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_PublishResponse_EncodeableType;
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

void OpcUa_RepublishRequest_Initialize(void* pValue);

void OpcUa_RepublishRequest_Clear(void* pValue);

SOPC_StatusCode OpcUa_RepublishRequest_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_RepublishRequest_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_RepublishRequest_EncodeableType;
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

void OpcUa_RepublishResponse_Initialize(void* pValue);

void OpcUa_RepublishResponse_Clear(void* pValue);

SOPC_StatusCode OpcUa_RepublishResponse_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_RepublishResponse_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_RepublishResponse_EncodeableType;
#endif
#endif

#ifndef OPCUA_EXCLUDE_TransferResult
/*============================================================================
 * The TransferResult structure.
 *===========================================================================*/
typedef struct _OpcUa_TransferResult
{
    SOPC_StatusCode StatusCode;
    int32_t         NoOfAvailableSequenceNumbers;
    uint32_t*       AvailableSequenceNumbers;
}
OpcUa_TransferResult;

void OpcUa_TransferResult_Initialize(void* pValue);

void OpcUa_TransferResult_Clear(void* pValue);

SOPC_StatusCode OpcUa_TransferResult_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_TransferResult_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_TransferResult_EncodeableType;
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
    SOPC_Boolean        SendInitialValues;
}
OpcUa_TransferSubscriptionsRequest;

void OpcUa_TransferSubscriptionsRequest_Initialize(void* pValue);

void OpcUa_TransferSubscriptionsRequest_Clear(void* pValue);

SOPC_StatusCode OpcUa_TransferSubscriptionsRequest_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_TransferSubscriptionsRequest_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_TransferSubscriptionsRequest_EncodeableType;
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
    SOPC_DiagnosticInfo*  DiagnosticInfos;
}
OpcUa_TransferSubscriptionsResponse;

void OpcUa_TransferSubscriptionsResponse_Initialize(void* pValue);

void OpcUa_TransferSubscriptionsResponse_Clear(void* pValue);

SOPC_StatusCode OpcUa_TransferSubscriptionsResponse_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_TransferSubscriptionsResponse_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_TransferSubscriptionsResponse_EncodeableType;
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

void OpcUa_DeleteSubscriptionsRequest_Initialize(void* pValue);

void OpcUa_DeleteSubscriptionsRequest_Clear(void* pValue);

SOPC_StatusCode OpcUa_DeleteSubscriptionsRequest_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_DeleteSubscriptionsRequest_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_DeleteSubscriptionsRequest_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_DeleteSubscriptionsResponse
/*============================================================================
 * The DeleteSubscriptionsResponse structure.
 *===========================================================================*/
typedef struct _OpcUa_DeleteSubscriptionsResponse
{
    OpcUa_ResponseHeader ResponseHeader;
    int32_t              NoOfResults;
    SOPC_StatusCode*     Results;
    int32_t              NoOfDiagnosticInfos;
    SOPC_DiagnosticInfo* DiagnosticInfos;
}
OpcUa_DeleteSubscriptionsResponse;

void OpcUa_DeleteSubscriptionsResponse_Initialize(void* pValue);

void OpcUa_DeleteSubscriptionsResponse_Clear(void* pValue);

SOPC_StatusCode OpcUa_DeleteSubscriptionsResponse_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_DeleteSubscriptionsResponse_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_DeleteSubscriptionsResponse_EncodeableType;
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
    SOPC_String   ProductUri;
    SOPC_String   ManufacturerName;
    SOPC_String   ProductName;
    SOPC_String   SoftwareVersion;
    SOPC_String   BuildNumber;
    SOPC_DateTime BuildDate;
}
OpcUa_BuildInfo;

void OpcUa_BuildInfo_Initialize(void* pValue);

void OpcUa_BuildInfo_Clear(void* pValue);

SOPC_StatusCode OpcUa_BuildInfo_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_BuildInfo_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_BuildInfo_EncodeableType;
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
    SOPC_String       ServerId;
    SOPC_Byte         ServiceLevel;
    OpcUa_ServerState ServerState;
}
OpcUa_RedundantServerDataType;

void OpcUa_RedundantServerDataType_Initialize(void* pValue);

void OpcUa_RedundantServerDataType_Clear(void* pValue);

SOPC_StatusCode OpcUa_RedundantServerDataType_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_RedundantServerDataType_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_RedundantServerDataType_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_EndpointUrlListDataType
/*============================================================================
 * The EndpointUrlListDataType structure.
 *===========================================================================*/
typedef struct _OpcUa_EndpointUrlListDataType
{
    int32_t      NoOfEndpointUrlList;
    SOPC_String* EndpointUrlList;
}
OpcUa_EndpointUrlListDataType;

void OpcUa_EndpointUrlListDataType_Initialize(void* pValue);

void OpcUa_EndpointUrlListDataType_Clear(void* pValue);

SOPC_StatusCode OpcUa_EndpointUrlListDataType_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_EndpointUrlListDataType_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_EndpointUrlListDataType_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_NetworkGroupDataType
/*============================================================================
 * The NetworkGroupDataType structure.
 *===========================================================================*/
typedef struct _OpcUa_NetworkGroupDataType
{
    SOPC_String                    ServerUri;
    int32_t                        NoOfNetworkPaths;
    OpcUa_EndpointUrlListDataType* NetworkPaths;
}
OpcUa_NetworkGroupDataType;

void OpcUa_NetworkGroupDataType_Initialize(void* pValue);

void OpcUa_NetworkGroupDataType_Clear(void* pValue);

SOPC_StatusCode OpcUa_NetworkGroupDataType_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_NetworkGroupDataType_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_NetworkGroupDataType_EncodeableType;
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

void OpcUa_SamplingIntervalDiagnosticsDataType_Initialize(void* pValue);

void OpcUa_SamplingIntervalDiagnosticsDataType_Clear(void* pValue);

SOPC_StatusCode OpcUa_SamplingIntervalDiagnosticsDataType_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_SamplingIntervalDiagnosticsDataType_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_SamplingIntervalDiagnosticsDataType_EncodeableType;
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

void OpcUa_ServerDiagnosticsSummaryDataType_Initialize(void* pValue);

void OpcUa_ServerDiagnosticsSummaryDataType_Clear(void* pValue);

SOPC_StatusCode OpcUa_ServerDiagnosticsSummaryDataType_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ServerDiagnosticsSummaryDataType_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_ServerDiagnosticsSummaryDataType_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ServerStatusDataType
/*============================================================================
 * The ServerStatusDataType structure.
 *===========================================================================*/
typedef struct _OpcUa_ServerStatusDataType
{
    SOPC_DateTime      StartTime;
    SOPC_DateTime      CurrentTime;
    OpcUa_ServerState  State;
    OpcUa_BuildInfo    BuildInfo;
    uint32_t           SecondsTillShutdown;
    SOPC_LocalizedText ShutdownReason;
}
OpcUa_ServerStatusDataType;

void OpcUa_ServerStatusDataType_Initialize(void* pValue);

void OpcUa_ServerStatusDataType_Clear(void* pValue);

SOPC_StatusCode OpcUa_ServerStatusDataType_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ServerStatusDataType_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_ServerStatusDataType_EncodeableType;
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

void OpcUa_ServiceCounterDataType_Initialize(void* pValue);

void OpcUa_ServiceCounterDataType_Clear(void* pValue);

SOPC_StatusCode OpcUa_ServiceCounterDataType_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ServiceCounterDataType_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_ServiceCounterDataType_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_SessionDiagnosticsDataType
/*============================================================================
 * The SessionDiagnosticsDataType structure.
 *===========================================================================*/
typedef struct _OpcUa_SessionDiagnosticsDataType
{
    SOPC_NodeId                  SessionId;
    SOPC_String                  SessionName;
    OpcUa_ApplicationDescription ClientDescription;
    SOPC_String                  ServerUri;
    SOPC_String                  EndpointUrl;
    int32_t                      NoOfLocaleIds;
    SOPC_String*                 LocaleIds;
    double                       ActualSessionTimeout;
    uint32_t                     MaxResponseMessageSize;
    SOPC_DateTime                ClientConnectionTime;
    SOPC_DateTime                ClientLastContactTime;
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

void OpcUa_SessionDiagnosticsDataType_Initialize(void* pValue);

void OpcUa_SessionDiagnosticsDataType_Clear(void* pValue);

SOPC_StatusCode OpcUa_SessionDiagnosticsDataType_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_SessionDiagnosticsDataType_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_SessionDiagnosticsDataType_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_SessionSecurityDiagnosticsDataType
/*============================================================================
 * The SessionSecurityDiagnosticsDataType structure.
 *===========================================================================*/
typedef struct _OpcUa_SessionSecurityDiagnosticsDataType
{
    SOPC_NodeId               SessionId;
    SOPC_String               ClientUserIdOfSession;
    int32_t                   NoOfClientUserIdHistory;
    SOPC_String*              ClientUserIdHistory;
    SOPC_String               AuthenticationMechanism;
    SOPC_String               Encoding;
    SOPC_String               TransportProtocol;
    OpcUa_MessageSecurityMode SecurityMode;
    SOPC_String               SecurityPolicyUri;
    SOPC_ByteString           ClientCertificate;
}
OpcUa_SessionSecurityDiagnosticsDataType;

void OpcUa_SessionSecurityDiagnosticsDataType_Initialize(void* pValue);

void OpcUa_SessionSecurityDiagnosticsDataType_Clear(void* pValue);

SOPC_StatusCode OpcUa_SessionSecurityDiagnosticsDataType_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_SessionSecurityDiagnosticsDataType_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_SessionSecurityDiagnosticsDataType_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_StatusResult
/*============================================================================
 * The StatusResult structure.
 *===========================================================================*/
typedef struct _OpcUa_StatusResult
{
    SOPC_StatusCode     StatusCode;
    SOPC_DiagnosticInfo DiagnosticInfo;
}
OpcUa_StatusResult;

void OpcUa_StatusResult_Initialize(void* pValue);

void OpcUa_StatusResult_Clear(void* pValue);

SOPC_StatusCode OpcUa_StatusResult_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_StatusResult_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_StatusResult_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_SubscriptionDiagnosticsDataType
/*============================================================================
 * The SubscriptionDiagnosticsDataType structure.
 *===========================================================================*/
typedef struct _OpcUa_SubscriptionDiagnosticsDataType
{
    SOPC_NodeId  SessionId;
    uint32_t     SubscriptionId;
    SOPC_Byte    Priority;
    double       PublishingInterval;
    uint32_t     MaxKeepAliveCount;
    uint32_t     MaxLifetimeCount;
    uint32_t     MaxNotificationsPerPublish;
    SOPC_Boolean PublishingEnabled;
    uint32_t     ModifyCount;
    uint32_t     EnableCount;
    uint32_t     DisableCount;
    uint32_t     RepublishRequestCount;
    uint32_t     RepublishMessageRequestCount;
    uint32_t     RepublishMessageCount;
    uint32_t     TransferRequestCount;
    uint32_t     TransferredToAltClientCount;
    uint32_t     TransferredToSameClientCount;
    uint32_t     PublishRequestCount;
    uint32_t     DataChangeNotificationsCount;
    uint32_t     EventNotificationsCount;
    uint32_t     NotificationsCount;
    uint32_t     LatePublishRequestCount;
    uint32_t     CurrentKeepAliveCount;
    uint32_t     CurrentLifetimeCount;
    uint32_t     UnacknowledgedMessageCount;
    uint32_t     DiscardedMessageCount;
    uint32_t     MonitoredItemCount;
    uint32_t     DisabledMonitoredItemCount;
    uint32_t     MonitoringQueueOverflowCount;
    uint32_t     NextSequenceNumber;
    uint32_t     EventQueueOverFlowCount;
}
OpcUa_SubscriptionDiagnosticsDataType;

void OpcUa_SubscriptionDiagnosticsDataType_Initialize(void* pValue);

void OpcUa_SubscriptionDiagnosticsDataType_Clear(void* pValue);

SOPC_StatusCode OpcUa_SubscriptionDiagnosticsDataType_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_SubscriptionDiagnosticsDataType_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_SubscriptionDiagnosticsDataType_EncodeableType;
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
    SOPC_NodeId Affected;
    SOPC_NodeId AffectedType;
    SOPC_Byte   Verb;
}
OpcUa_ModelChangeStructureDataType;

void OpcUa_ModelChangeStructureDataType_Initialize(void* pValue);

void OpcUa_ModelChangeStructureDataType_Clear(void* pValue);

SOPC_StatusCode OpcUa_ModelChangeStructureDataType_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ModelChangeStructureDataType_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_ModelChangeStructureDataType_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_SemanticChangeStructureDataType
/*============================================================================
 * The SemanticChangeStructureDataType structure.
 *===========================================================================*/
typedef struct _OpcUa_SemanticChangeStructureDataType
{
    SOPC_NodeId Affected;
    SOPC_NodeId AffectedType;
}
OpcUa_SemanticChangeStructureDataType;

void OpcUa_SemanticChangeStructureDataType_Initialize(void* pValue);

void OpcUa_SemanticChangeStructureDataType_Clear(void* pValue);

SOPC_StatusCode OpcUa_SemanticChangeStructureDataType_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_SemanticChangeStructureDataType_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_SemanticChangeStructureDataType_EncodeableType;
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

void OpcUa_Range_Initialize(void* pValue);

void OpcUa_Range_Clear(void* pValue);

SOPC_StatusCode OpcUa_Range_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_Range_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_Range_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_EUInformation
/*============================================================================
 * The EUInformation structure.
 *===========================================================================*/
typedef struct _OpcUa_EUInformation
{
    SOPC_String        NamespaceUri;
    int32_t            UnitId;
    SOPC_LocalizedText DisplayName;
    SOPC_LocalizedText Description;
}
OpcUa_EUInformation;

void OpcUa_EUInformation_Initialize(void* pValue);

void OpcUa_EUInformation_Clear(void* pValue);

SOPC_StatusCode OpcUa_EUInformation_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_EUInformation_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_EUInformation_EncodeableType;
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

void OpcUa_ComplexNumberType_Initialize(void* pValue);

void OpcUa_ComplexNumberType_Clear(void* pValue);

SOPC_StatusCode OpcUa_ComplexNumberType_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ComplexNumberType_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_ComplexNumberType_EncodeableType;
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

void OpcUa_DoubleComplexNumberType_Initialize(void* pValue);

void OpcUa_DoubleComplexNumberType_Clear(void* pValue);

SOPC_StatusCode OpcUa_DoubleComplexNumberType_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_DoubleComplexNumberType_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_DoubleComplexNumberType_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_AxisInformation
/*============================================================================
 * The AxisInformation structure.
 *===========================================================================*/
typedef struct _OpcUa_AxisInformation
{
    OpcUa_EUInformation        EngineeringUnits;
    OpcUa_Range                EURange;
    SOPC_LocalizedText         Title;
    OpcUa_AxisScaleEnumeration AxisScaleType;
    int32_t                    NoOfAxisSteps;
    double*                    AxisSteps;
}
OpcUa_AxisInformation;

void OpcUa_AxisInformation_Initialize(void* pValue);

void OpcUa_AxisInformation_Clear(void* pValue);

SOPC_StatusCode OpcUa_AxisInformation_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_AxisInformation_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_AxisInformation_EncodeableType;
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

void OpcUa_XVType_Initialize(void* pValue);

void OpcUa_XVType_Clear(void* pValue);

SOPC_StatusCode OpcUa_XVType_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_XVType_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_XVType_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_ProgramDiagnosticDataType
/*============================================================================
 * The ProgramDiagnosticDataType structure.
 *===========================================================================*/
typedef struct _OpcUa_ProgramDiagnosticDataType
{
    SOPC_NodeId        CreateSessionId;
    SOPC_String        CreateClientName;
    SOPC_DateTime      InvocationCreationTime;
    SOPC_DateTime      LastTransitionTime;
    SOPC_String        LastMethodCall;
    SOPC_NodeId        LastMethodSessionId;
    int32_t            NoOfLastMethodInputArguments;
    OpcUa_Argument*    LastMethodInputArguments;
    int32_t            NoOfLastMethodOutputArguments;
    OpcUa_Argument*    LastMethodOutputArguments;
    SOPC_DateTime      LastMethodCallTime;
    OpcUa_StatusResult LastMethodReturnStatus;
}
OpcUa_ProgramDiagnosticDataType;

void OpcUa_ProgramDiagnosticDataType_Initialize(void* pValue);

void OpcUa_ProgramDiagnosticDataType_Clear(void* pValue);

SOPC_StatusCode OpcUa_ProgramDiagnosticDataType_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_ProgramDiagnosticDataType_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_ProgramDiagnosticDataType_EncodeableType;
#endif

#ifndef OPCUA_EXCLUDE_Annotation
/*============================================================================
 * The Annotation structure.
 *===========================================================================*/
typedef struct _OpcUa_Annotation
{
    SOPC_String   Message;
    SOPC_String   UserName;
    SOPC_DateTime AnnotationTime;
}
OpcUa_Annotation;

void OpcUa_Annotation_Initialize(void* pValue);

void OpcUa_Annotation_Clear(void* pValue);

SOPC_StatusCode OpcUa_Annotation_Encode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

SOPC_StatusCode OpcUa_Annotation_Decode(void* pValue, struct SOPC_MsgBuffer* msgBuf);

extern struct SOPC_EncodeableType OpcUa_Annotation_EncodeableType;
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

void SOPC_Initialize_EnumeratedType(int32_t* enumerationValue);

void SOPC_Clear_EnumeratedType(int32_t* enumerationValue);

SOPC_StatusCode SOPC_Read_EnumeratedType(struct SOPC_MsgBuffer* msgBuf, int32_t* enumerationValue);
                    

SOPC_StatusCode SOPC_Write_EnumeratedType(struct SOPC_MsgBuffer* msgBuf, int32_t* enumerationValue);

/*============================================================================
 * Table of known types.
 *===========================================================================*/
extern struct SOPC_EncodeableType** SOPC_KnownEncodeableTypes;

END_EXTERN_C

#endif
/* This is the last line of an autogenerated file. */
