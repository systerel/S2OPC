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

/**
 * This file is an excerpt from sopc_types.h.
 * It should not be included in a generic project.
 * See s2opc_headers.h
 */

typedef enum
{
    OpcUa_MessageSecurityMode_Invalid = 0,
    OpcUa_MessageSecurityMode_None = 1,
    OpcUa_MessageSecurityMode_Sign = 2,
    OpcUa_MessageSecurityMode_SignAndEncrypt = 3,
    OpcUa_MessageSecurityMode_SizeOf = INT32_MAX
} OpcUa_MessageSecurityMode;

extern SOPC_EncodeableType OpcUa_ResponseHeader_EncodeableType;
typedef struct _OpcUa_ResponseHeader
{
    SOPC_EncodeableType* encodeableType;
    SOPC_DateTime Timestamp;
    uint32_t RequestHandle;
    SOPC_StatusCode ServiceResult;
    SOPC_DiagnosticInfo ServiceDiagnostics;
    int32_t NoOfStringTable;
    SOPC_String* StringTable;
    SOPC_ExtensionObject AdditionalHeader;
} OpcUa_ResponseHeader;

typedef enum _OpcUa_TimestampsToReturn
{
    OpcUa_TimestampsToReturn_Source = 0,
    OpcUa_TimestampsToReturn_Server = 1,
    OpcUa_TimestampsToReturn_Both = 2,
    OpcUa_TimestampsToReturn_Neither = 3,
    OpcUa_TimestampsToReturn_SizeOf = INT32_MAX
} OpcUa_TimestampsToReturn;

typedef enum _OpcUa_NodeClass
{
    OpcUa_NodeClass_Unspecified = 0,
    OpcUa_NodeClass_Object = 1,
    OpcUa_NodeClass_Variable = 2,
    OpcUa_NodeClass_Method = 4,
    OpcUa_NodeClass_ObjectType = 8,
    OpcUa_NodeClass_VariableType = 16,
    OpcUa_NodeClass_ReferenceType = 32,
    OpcUa_NodeClass_DataType = 64,
    OpcUa_NodeClass_View = 128,
    OpcUa_NodeClass_SizeOf = INT32_MAX
} OpcUa_NodeClass;

/* API structures */

typedef enum _OpcUa_UserTokenType
{
    OpcUa_UserTokenType_Anonymous = 0,
    OpcUa_UserTokenType_UserName = 1,
    OpcUa_UserTokenType_Certificate = 2,
    OpcUa_UserTokenType_IssuedToken = 3,
    OpcUa_UserTokenType_Kerberos = 4,
    OpcUa_UserTokenType_SizeOf = INT32_MAX
} OpcUa_UserTokenType;

typedef struct _OpcUa_UserTokenPolicy
{
    SOPC_EncodeableType* encodeableType;
    SOPC_String PolicyId;
    OpcUa_UserTokenType TokenType;
    SOPC_String IssuedTokenType;
    SOPC_String IssuerEndpointUrl;
    SOPC_String SecurityPolicyUri;
} OpcUa_UserTokenPolicy;

typedef enum _OpcUa_ApplicationType
{
    OpcUa_ApplicationType_Server = 0,
    OpcUa_ApplicationType_Client = 1,
    OpcUa_ApplicationType_ClientAndServer = 2,
    OpcUa_ApplicationType_DiscoveryServer = 3,
    OpcUa_ApplicationType_SizeOf = INT32_MAX
} OpcUa_ApplicationType;

typedef struct _OpcUa_ApplicationDescription
{
    SOPC_EncodeableType* encodeableType;
    SOPC_String ApplicationUri;
    SOPC_String ProductUri;
    SOPC_LocalizedText ApplicationName;
    OpcUa_ApplicationType ApplicationType;
    SOPC_String GatewayServerUri;
    SOPC_String DiscoveryProfileUri;
    int32_t NoOfDiscoveryUrls;
    SOPC_String* DiscoveryUrls;
} OpcUa_ApplicationDescription;

/* GetEndpoitns response */
typedef struct _OpcUa_EndpointDescription
{
    SOPC_EncodeableType* encodeableType;
    /* IMPORTANT NOTE: response header IN RESPONSE MSG BODY is kept only
     *  for giving a copy of the header to application.
     */
    SOPC_String EndpointUrl;
    OpcUa_ApplicationDescription Server;
    SOPC_ByteString ServerCertificate;
    OpcUa_MessageSecurityMode SecurityMode;
    SOPC_String SecurityPolicyUri;
    int32_t NoOfUserIdentityTokens;
    OpcUa_UserTokenPolicy* UserIdentityTokens;
    SOPC_String TransportProfileUri;
    SOPC_Byte SecurityLevel;
} OpcUa_EndpointDescription;

typedef struct _OpcUa_GetEndpointsResponse
{
    SOPC_EncodeableType* encodeableType;
    /* IMPORTANT NOTE: response header IN RESPONSE MSG BODY is kept only
     *  for giving a copy of the header to application.
     */
    OpcUa_ResponseHeader ResponseHeader;
    int32_t NoOfEndpoints;
    OpcUa_EndpointDescription* Endpoints;
} OpcUa_GetEndpointsResponse;

/* Read structures */

extern SOPC_EncodeableType OpcUa_ReadValueId_EncodeableType;
typedef struct _OpcUa_ReadValueId
{
    SOPC_EncodeableType* encodeableType;
    SOPC_NodeId NodeId;
    uint32_t AttributeId;
    SOPC_String IndexRange;
    SOPC_QualifiedName DataEncoding;
} OpcUa_ReadValueId;

extern SOPC_EncodeableType OpcUa_ReadRequest_EncodeableType;
typedef struct _OpcUa_ReadRequest
{
    SOPC_EncodeableType* encodeableType;
    double MaxAge;
    OpcUa_TimestampsToReturn TimestampsToReturn;
    int32_t NoOfNodesToRead;
    OpcUa_ReadValueId* NodesToRead;
} OpcUa_ReadRequest;

extern SOPC_EncodeableType OpcUa_ReadResponse_EncodeableType;
typedef struct _OpcUa_ReadResponse
{
    SOPC_EncodeableType* encodeableType;
    OpcUa_ResponseHeader ResponseHeader;
    int32_t NoOfResults;
    SOPC_DataValue* Results;
    int32_t NoOfDiagnosticInfos;
    SOPC_DiagnosticInfo* DiagnosticInfos;
} OpcUa_ReadResponse;

/* Write structures */

extern SOPC_EncodeableType OpcUa_WriteValue_EncodeableType;
typedef struct _OpcUa_WriteValue
{
    SOPC_EncodeableType* encodeableType;
    SOPC_NodeId NodeId;
    uint32_t AttributeId;
    SOPC_String IndexRange;
    SOPC_DataValue Value;
} OpcUa_WriteValue;

extern SOPC_EncodeableType OpcUa_WriteRequest_EncodeableType;
typedef struct _OpcUa_WriteRequest
{
    SOPC_EncodeableType* encodeableType;
    int32_t NoOfNodesToWrite;
    OpcUa_WriteValue* NodesToWrite;
} OpcUa_WriteRequest;

extern SOPC_EncodeableType OpcUa_WriteResponse_EncodeableType;
typedef struct _OpcUa_WriteResponse
{
    SOPC_EncodeableType* encodeableType;
    OpcUa_ResponseHeader ResponseHeader;
    int32_t NoOfResults;
    SOPC_StatusCode* Results;
    int32_t NoOfDiagnosticInfos;
    SOPC_DiagnosticInfo* DiagnosticInfos;
} OpcUa_WriteResponse;

/* Browse structures */

typedef enum _OpcUa_BrowseDirection
{
    OpcUa_BrowseDirection_Forward = 0,
    OpcUa_BrowseDirection_Inverse = 1,
    OpcUa_BrowseDirection_Both = 2,
    OpcUa_BrowseDirection_SizeOf = INT32_MAX
} OpcUa_BrowseDirection;

extern SOPC_EncodeableType OpcUa_ViewDescription_EncodeableType;
typedef struct _OpcUa_ViewDescription
{
    SOPC_EncodeableType* encodeableType;
    SOPC_NodeId ViewId;
    SOPC_DateTime Timestamp;
    uint32_t ViewVersion;
} OpcUa_ViewDescription;

typedef enum _OpcUa_BrowseResultMask
{
    OpcUa_BrowseResultMask_None = 0,
    OpcUa_BrowseResultMask_ReferenceTypeId = 1,
    OpcUa_BrowseResultMask_IsForward = 2,
    OpcUa_BrowseResultMask_NodeClass = 4,
    OpcUa_BrowseResultMask_BrowseName = 8,
    OpcUa_BrowseResultMask_DisplayName = 16,
    OpcUa_BrowseResultMask_TypeDefinition = 32,
    OpcUa_BrowseResultMask_All = 63,
    OpcUa_BrowseResultMask_ReferenceTypeInfo = 3,
    OpcUa_BrowseResultMask_TargetInfo = 60,
    OpcUa_BrowseResultMask_SizeOf = INT32_MAX
} OpcUa_BrowseResultMask;

extern SOPC_EncodeableType OpcUa_BrowseDescription_EncodeableType;
typedef struct _OpcUa_BrowseDescription
{
    SOPC_EncodeableType* encodeableType;
    SOPC_NodeId NodeId;
    OpcUa_BrowseDirection BrowseDirection;
    SOPC_NodeId ReferenceTypeId;
    SOPC_Boolean IncludeSubtypes;
    uint32_t NodeClassMask;
    uint32_t ResultMask;
} OpcUa_BrowseDescription;

typedef struct _OpcUa_BrowseRequest
{
    SOPC_EncodeableType* encodeableType;
    OpcUa_ViewDescription View;
    uint32_t RequestedMaxReferencesPerNode;
    int32_t NoOfNodesToBrowse;
    OpcUa_BrowseDescription* NodesToBrowse;
} OpcUa_BrowseRequest;
extern SOPC_EncodeableType OpcUa_BrowseResponse_EncodeableType;

extern SOPC_EncodeableType OpcUa_ReferenceDescription_EncodeableType;
typedef struct _OpcUa_ReferenceDescription
{
    SOPC_EncodeableType* encodeableType;
    SOPC_NodeId ReferenceTypeId;
    SOPC_Boolean IsForward;
    SOPC_ExpandedNodeId NodeId;
    SOPC_QualifiedName BrowseName;
    SOPC_LocalizedText DisplayName;
    OpcUa_NodeClass NodeClass;
    SOPC_ExpandedNodeId TypeDefinition;
} OpcUa_ReferenceDescription;

extern SOPC_EncodeableType OpcUa_BrowseResult_EncodeableType;
typedef struct _OpcUa_BrowseResult
{
    SOPC_EncodeableType* encodeableType;
    SOPC_StatusCode StatusCode;
    SOPC_ByteString ContinuationPoint;
    int32_t NoOfReferences;
    OpcUa_ReferenceDescription* References;
} OpcUa_BrowseResult;

extern SOPC_EncodeableType OpcUa_BrowseRequest_EncodeableType;
typedef struct _OpcUa_BrowseResponse
{
    SOPC_EncodeableType* encodeableType;
    OpcUa_ResponseHeader ResponseHeader;
    int32_t NoOfResults;
    OpcUa_BrowseResult* Results;
    int32_t NoOfDiagnosticInfos;
    SOPC_DiagnosticInfo* DiagnosticInfos;
} OpcUa_BrowseResponse;

/* Misc */
extern SOPC_EncodeableType OpcUa_Range_EncodeableType;
typedef struct _OpcUa_Range
{
    SOPC_EncodeableType* encodeableType;
    double Low;
    double High;
} OpcUa_Range;

extern SOPC_EncodeableType OpcUa_EUInformation_EncodeableType;
typedef struct _OpcUa_EUInformation
{
    SOPC_EncodeableType* encodeableType;
    SOPC_String NamespaceUri;
    int32_t UnitId;
    SOPC_LocalizedText DisplayName;
    SOPC_LocalizedText Description;
} OpcUa_EUInformation;

extern const OpcUa_UserTokenPolicy SOPC_UserTokenPolicy_Anonymous;

extern const OpcUa_UserTokenPolicy SOPC_UserTokenPolicy_UserName_NoneSecurityPolicy;

extern const OpcUa_UserTokenPolicy SOPC_UserTokenPolicy_UserName_DefaultSecurityPolicy;
