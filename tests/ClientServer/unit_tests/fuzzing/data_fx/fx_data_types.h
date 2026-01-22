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

#ifndef SOPC_FX_DATA_Types_H_
#define SOPC_FX_DATA_Types_H_ 1

#include "fx_data_enum_types.h"
#include "sopc_buffer.h"
#include "sopc_builtintypes.h"
#include "sopc_encodeabletype.h"

#define SOPC_FX_DATA_NS_INDEX 3

#include "sopc_types.h"

#ifndef OPCUA_EXCLUDE_FX_Data_NodeIdArray
/*============================================================================
 * The FX_Data_NodeIdArray structure.
 *===========================================================================*/
extern SOPC_EncodeableType OpcUa_FX_Data_NodeIdArray_EncodeableType;

typedef struct _OpcUa_FX_Data_NodeIdArray
{
    SOPC_EncodeableType* encodeableType;
    /* IMPORTANT NOTE: response header IN RESPONSE MSG BODY is kept only
     *  for giving a copy of the header to application.
     */
    SOPC_NodeId Node;
    int32_t NoOfArrayIndex;
    uint32_t* ArrayIndex;
} OpcUa_FX_Data_NodeIdArray;

void OpcUa_FX_Data_NodeIdArray_Initialize(void* pValue);

void OpcUa_FX_Data_NodeIdArray_Clear(void* pValue);

#endif

#ifndef OPCUA_EXCLUDE_FX_Data_NodeIdValuePair
/*============================================================================
 * The FX_Data_NodeIdValuePair structure.
 *===========================================================================*/
extern SOPC_EncodeableType OpcUa_FX_Data_NodeIdValuePair_EncodeableType;

typedef struct _OpcUa_FX_Data_NodeIdValuePair
{
    SOPC_EncodeableType* encodeableType;
    /* IMPORTANT NOTE: response header IN RESPONSE MSG BODY is kept only
     *  for giving a copy of the header to application.
     */
    OpcUa_FX_Data_NodeIdArray Key;
    SOPC_Variant Value;
} OpcUa_FX_Data_NodeIdValuePair;

void OpcUa_FX_Data_NodeIdValuePair_Initialize(void* pValue);

void OpcUa_FX_Data_NodeIdValuePair_Clear(void* pValue);

#endif

#ifndef OPCUA_EXCLUDE_FX_Data_AssetVerificationDataType
/*============================================================================
 * The FX_Data_AssetVerificationDataType structure.
 *===========================================================================*/
extern SOPC_EncodeableType OpcUa_FX_Data_AssetVerificationDataType_EncodeableType;

typedef struct _OpcUa_FX_Data_AssetVerificationDataType
{
    SOPC_EncodeableType* encodeableType;
    /* IMPORTANT NOTE: response header IN RESPONSE MSG BODY is kept only
     *  for giving a copy of the header to application.
     */
    SOPC_NodeId AssetToVerify;
    OpcUa_FX_Data_AssetVerificationModeEnum VerificationMode;
    OpcUa_FX_Data_AssetVerificationResultEnum ExpectedVerificationResult;
    int32_t NoOfExpectedVerificationVariables;
    OpcUa_KeyValuePair* ExpectedVerificationVariables;
    int32_t NoOfExpectedAdditionalVerificationVariables;
    OpcUa_FX_Data_NodeIdValuePair* ExpectedAdditionalVerificationVariables;
} OpcUa_FX_Data_AssetVerificationDataType;

void OpcUa_FX_Data_AssetVerificationDataType_Initialize(void* pValue);

void OpcUa_FX_Data_AssetVerificationDataType_Clear(void* pValue);

#endif

#ifndef OPCUA_EXCLUDE_FX_Data_AssetVerificationResultDataType
/*============================================================================
 * The FX_Data_AssetVerificationResultDataType structure.
 *===========================================================================*/
extern SOPC_EncodeableType OpcUa_FX_Data_AssetVerificationResultDataType_EncodeableType;

typedef struct _OpcUa_FX_Data_AssetVerificationResultDataType
{
    SOPC_EncodeableType* encodeableType;
    /* IMPORTANT NOTE: response header IN RESPONSE MSG BODY is kept only
     *  for giving a copy of the header to application.
     */
    SOPC_StatusCode VerificationStatus;
    OpcUa_FX_Data_AssetVerificationResultEnum VerificationResult;
    int32_t NoOfVerificationVariablesErrors;
    SOPC_StatusCode* VerificationVariablesErrors;
    int32_t NoOfVerificationAdditionalVariablesErrors;
    SOPC_StatusCode* VerificationAdditionalVariablesErrors;
} OpcUa_FX_Data_AssetVerificationResultDataType;

void OpcUa_FX_Data_AssetVerificationResultDataType_Initialize(void* pValue);

void OpcUa_FX_Data_AssetVerificationResultDataType_Clear(void* pValue);

#endif

#ifndef OPCUA_EXCLUDE_FX_Data_PubSubCommunicationConfigurationDataType
/*============================================================================
 * The FX_Data_PubSubCommunicationConfigurationDataType structure.
 *===========================================================================*/
extern SOPC_EncodeableType OpcUa_FX_Data_PubSubCommunicationConfigurationDataType_EncodeableType;

typedef struct _OpcUa_FX_Data_PubSubCommunicationConfigurationDataType
{
    SOPC_EncodeableType* encodeableType;
    /* IMPORTANT NOTE: response header IN RESPONSE MSG BODY is kept only
     *  for giving a copy of the header to application.
     */
    OpcUa_PubSubConfiguration2DataType PubSubConfiguration;
    SOPC_Boolean RequireCompleteUpdate;
    int32_t NoOfConfigurationReferences;
    OpcUa_PubSubConfigurationRefDataType* ConfigurationReferences;
} OpcUa_FX_Data_PubSubCommunicationConfigurationDataType;

void OpcUa_FX_Data_PubSubCommunicationConfigurationDataType_Initialize(void* pValue);

void OpcUa_FX_Data_PubSubCommunicationConfigurationDataType_Clear(void* pValue);

#endif

#ifndef OPCUA_EXCLUDE_FX_Data_PubSubCommunicationConfigurationResultDataType
/*============================================================================
 * The FX_Data_PubSubCommunicationConfigurationResultDataType structure.
 *===========================================================================*/
extern SOPC_EncodeableType OpcUa_FX_Data_PubSubCommunicationConfigurationResultDataType_EncodeableType;

typedef struct _OpcUa_FX_Data_PubSubCommunicationConfigurationResultDataType
{
    SOPC_EncodeableType* encodeableType;
    /* IMPORTANT NOTE: response header IN RESPONSE MSG BODY is kept only
     *  for giving a copy of the header to application.
     */
    SOPC_StatusCode Result;
    SOPC_Boolean ChangesApplied;
    int32_t NoOfReferenceResults;
    SOPC_StatusCode* ReferenceResults;
    int32_t NoOfConfigurationValues;
    OpcUa_PubSubConfigurationValueDataType* ConfigurationValues;
    int32_t NoOfConfigurationObjects;
    SOPC_NodeId* ConfigurationObjects;
} OpcUa_FX_Data_PubSubCommunicationConfigurationResultDataType;

void OpcUa_FX_Data_PubSubCommunicationConfigurationResultDataType_Initialize(void* pValue);

void OpcUa_FX_Data_PubSubCommunicationConfigurationResultDataType_Clear(void* pValue);

#endif

#ifndef OPCUA_EXCLUDE_FX_Data_PubSubCommunicationLinkConfigurationDataType
/*============================================================================
 * The FX_Data_PubSubCommunicationLinkConfigurationDataType structure.
 *===========================================================================*/
extern SOPC_EncodeableType OpcUa_FX_Data_PubSubCommunicationLinkConfigurationDataType_EncodeableType;

typedef struct _OpcUa_FX_Data_PubSubCommunicationLinkConfigurationDataType
{
    SOPC_EncodeableType* encodeableType;
    /* IMPORTANT NOTE: response header IN RESPONSE MSG BODY is kept only
     *  for giving a copy of the header to application.
     */
    OpcUa_PubSubConfigurationRefDataType DataSetReaderRef;
    OpcUa_ConfigurationVersionDataType ExpectedSubscribedDataSetVersion;
    OpcUa_PubSubConfigurationRefDataType DataSetWriterRef;
    OpcUa_ConfigurationVersionDataType ExpectedPublishedDataSetVersion;
} OpcUa_FX_Data_PubSubCommunicationLinkConfigurationDataType;

void OpcUa_FX_Data_PubSubCommunicationLinkConfigurationDataType_Initialize(void* pValue);

void OpcUa_FX_Data_PubSubCommunicationLinkConfigurationDataType_Clear(void* pValue);

#endif

#ifndef OPCUA_EXCLUDE_FX_Data_ConnectionEndpointDefinitionDataType
/*============================================================================
 * The FX_Data_ConnectionEndpointDefinitionDataType structure.
 *===========================================================================*/
extern SOPC_EncodeableType OpcUa_FX_Data_ConnectionEndpointDefinitionDataType_EncodeableType;

typedef struct _OpcUa_FX_Data_ConnectionEndpointDefinitionDataType
{
    SOPC_EncodeableType* encodeableType;
    /* IMPORTANT NOTE: response header IN RESPONSE MSG BODY is kept only
     *  for giving a copy of the header to application.
     */
    uint32_t SwitchField;
    union
    {
        SOPC_ExtensionObject Parameter;
        SOPC_NodeId Node;
    } Value;
} OpcUa_FX_Data_ConnectionEndpointDefinitionDataType;

void OpcUa_FX_Data_ConnectionEndpointDefinitionDataType_Initialize(void* pValue);

void OpcUa_FX_Data_ConnectionEndpointDefinitionDataType_Clear(void* pValue);

#endif

#ifndef OPCUA_EXCLUDE_FX_Data_ConnectionEndpointConfigurationDataType
/*============================================================================
 * The FX_Data_ConnectionEndpointConfigurationDataType structure.
 *===========================================================================*/
extern SOPC_EncodeableType OpcUa_FX_Data_ConnectionEndpointConfigurationDataType_EncodeableType;

typedef struct _OpcUa_FX_Data_ConnectionEndpointConfigurationDataType
{
    SOPC_EncodeableType* encodeableType;
    /* IMPORTANT NOTE: response header IN RESPONSE MSG BODY is kept only
     *  for giving a copy of the header to application.
     */
    SOPC_NodeId FunctionalEntityNode;
    OpcUa_FX_Data_ConnectionEndpointDefinitionDataType ConnectionEndpoint;
    int32_t NoOfExpectedVerificationVariables;
    OpcUa_FX_Data_NodeIdValuePair* ExpectedVerificationVariables;
    int32_t NoOfControlGroups;
    SOPC_NodeId* ControlGroups;
    int32_t NoOfConfigurationData;
    OpcUa_FX_Data_NodeIdValuePair* ConfigurationData;
    SOPC_ExtensionObject CommunicationLinks;
} OpcUa_FX_Data_ConnectionEndpointConfigurationDataType;

void OpcUa_FX_Data_ConnectionEndpointConfigurationDataType_Initialize(void* pValue);

void OpcUa_FX_Data_ConnectionEndpointConfigurationDataType_Clear(void* pValue);

#endif

#ifndef OPCUA_EXCLUDE_FX_Data_ConnectionEndpointConfigurationResultDataType
/*============================================================================
 * The FX_Data_ConnectionEndpointConfigurationResultDataType structure.
 *===========================================================================*/
extern SOPC_EncodeableType OpcUa_FX_Data_ConnectionEndpointConfigurationResultDataType_EncodeableType;

typedef struct _OpcUa_FX_Data_ConnectionEndpointConfigurationResultDataType
{
    SOPC_EncodeableType* encodeableType;
    /* IMPORTANT NOTE: response header IN RESPONSE MSG BODY is kept only
     *  for giving a copy of the header to application.
     */
    SOPC_NodeId ConnectionEndpointId;
    SOPC_StatusCode FunctionalEntityNodeResult;
    SOPC_StatusCode ConnectionEndpointResult;
    OpcUa_FX_Data_FunctionalEntityVerificationResultEnum VerificationResult;
    SOPC_StatusCode VerificationStatus;
    int32_t NoOfVerificationVariablesErrors;
    SOPC_StatusCode* VerificationVariablesErrors;
    int32_t NoOfEstablishControlResult;
    SOPC_StatusCode* EstablishControlResult;
    int32_t NoOfConfigurationDataResult;
    SOPC_StatusCode* ConfigurationDataResult;
    int32_t NoOfReassignControlResult;
    SOPC_StatusCode* ReassignControlResult;
    SOPC_StatusCode CommunicationLinksResult;
    SOPC_StatusCode EnableCommunicationResult;
} OpcUa_FX_Data_ConnectionEndpointConfigurationResultDataType;

void OpcUa_FX_Data_ConnectionEndpointConfigurationResultDataType_Initialize(void* pValue);

void OpcUa_FX_Data_ConnectionEndpointConfigurationResultDataType_Clear(void* pValue);

#endif

#ifndef OPCUA_EXCLUDE_FX_Data_RelatedEndpointDataType
/*============================================================================
 * The FX_Data_RelatedEndpointDataType structure.
 *===========================================================================*/
extern SOPC_EncodeableType OpcUa_FX_Data_RelatedEndpointDataType_EncodeableType;

typedef struct _OpcUa_FX_Data_RelatedEndpointDataType
{
    SOPC_EncodeableType* encodeableType;
    /* IMPORTANT NOTE: response header IN RESPONSE MSG BODY is kept only
     *  for giving a copy of the header to application.
     */
    SOPC_String Address;
    int32_t NoOfConnectionEndpointPath;
    OpcUa_PortableQualifiedName* ConnectionEndpointPath;
    SOPC_String ConnectionEndpointName;
} OpcUa_FX_Data_RelatedEndpointDataType;

void OpcUa_FX_Data_RelatedEndpointDataType_Initialize(void* pValue);

void OpcUa_FX_Data_RelatedEndpointDataType_Clear(void* pValue);

#endif

#ifndef OPCUA_EXCLUDE_FX_Data_ConnectionEndpointParameterDataType
/*============================================================================
 * The FX_Data_ConnectionEndpointParameterDataType structure.
 *===========================================================================*/
extern SOPC_EncodeableType OpcUa_FX_Data_ConnectionEndpointParameterDataType_EncodeableType;

typedef struct _OpcUa_FX_Data_ConnectionEndpointParameterDataType
{
    SOPC_EncodeableType* encodeableType;
    /* IMPORTANT NOTE: response header IN RESPONSE MSG BODY is kept only
     *  for giving a copy of the header to application.
     */
    SOPC_String Name;
    SOPC_NodeId ConnectionEndpointTypeId;
    int32_t NoOfInputVariableIds;
    SOPC_NodeId* InputVariableIds;
    int32_t NoOfOutputVariableIds;
    SOPC_NodeId* OutputVariableIds;
    SOPC_Boolean IsPersistent;
    double CleanupTimeout;
    OpcUa_FX_Data_RelatedEndpointDataType RelatedEndpoint;
    SOPC_Boolean IsPreconfigured;
} OpcUa_FX_Data_ConnectionEndpointParameterDataType;

void OpcUa_FX_Data_ConnectionEndpointParameterDataType_Initialize(void* pValue);

void OpcUa_FX_Data_ConnectionEndpointParameterDataType_Clear(void* pValue);

#endif

#ifndef OPCUA_EXCLUDE_FX_Data_PubSubConnectionEndpointParameterDataType
/*============================================================================
 * The FX_Data_PubSubConnectionEndpointParameterDataType structure.
 *===========================================================================*/
extern SOPC_EncodeableType OpcUa_FX_Data_PubSubConnectionEndpointParameterDataType_EncodeableType;

typedef struct _OpcUa_FX_Data_PubSubConnectionEndpointParameterDataType
{
    SOPC_EncodeableType* encodeableType;
    /* IMPORTANT NOTE: response header IN RESPONSE MSG BODY is kept only
     *  for giving a copy of the header to application.
     */
    SOPC_String Name;
    SOPC_NodeId ConnectionEndpointTypeId;
    int32_t NoOfInputVariableIds;
    SOPC_NodeId* InputVariableIds;
    int32_t NoOfOutputVariableIds;
    SOPC_NodeId* OutputVariableIds;
    SOPC_Boolean IsPersistent;
    double CleanupTimeout;
    OpcUa_FX_Data_RelatedEndpointDataType RelatedEndpoint;
    SOPC_Boolean IsPreconfigured;
    OpcUa_FX_Data_PubSubConnectionEndpointModeEnum Mode;
} OpcUa_FX_Data_PubSubConnectionEndpointParameterDataType;

void OpcUa_FX_Data_PubSubConnectionEndpointParameterDataType_Initialize(void* pValue);

void OpcUa_FX_Data_PubSubConnectionEndpointParameterDataType_Clear(void* pValue);

#endif

#ifndef OPCUA_EXCLUDE_FX_Data_PubSubReserveCommunicationIdsDataType
/*============================================================================
 * The FX_Data_PubSubReserveCommunicationIdsDataType structure.
 *===========================================================================*/
extern SOPC_EncodeableType OpcUa_FX_Data_PubSubReserveCommunicationIdsDataType_EncodeableType;

typedef struct _OpcUa_FX_Data_PubSubReserveCommunicationIdsDataType
{
    SOPC_EncodeableType* encodeableType;
    /* IMPORTANT NOTE: response header IN RESPONSE MSG BODY is kept only
     *  for giving a copy of the header to application.
     */
    SOPC_String TransportProfileUri;
    uint16_t NumReqWriterGroupIds;
    uint16_t NumReqDataSetWriterIds;
} OpcUa_FX_Data_PubSubReserveCommunicationIdsDataType;

void OpcUa_FX_Data_PubSubReserveCommunicationIdsDataType_Initialize(void* pValue);

void OpcUa_FX_Data_PubSubReserveCommunicationIdsDataType_Clear(void* pValue);

#endif

#ifndef OPCUA_EXCLUDE_FX_Data_PubSubReserveCommunicationIds2DataType
/*============================================================================
 * The FX_Data_PubSubReserveCommunicationIds2DataType structure.
 *===========================================================================*/
extern SOPC_EncodeableType OpcUa_FX_Data_PubSubReserveCommunicationIds2DataType_EncodeableType;

typedef struct _OpcUa_FX_Data_PubSubReserveCommunicationIds2DataType
{
    SOPC_EncodeableType* encodeableType;
    /* IMPORTANT NOTE: response header IN RESPONSE MSG BODY is kept only
     *  for giving a copy of the header to application.
     */
    SOPC_String TransportProfileUri;
    uint16_t NumReqWriterGroupIds;
    uint16_t NumReqDataSetWriterIds;
    SOPC_Boolean RequestTransportSpecificInfo;
} OpcUa_FX_Data_PubSubReserveCommunicationIds2DataType;

void OpcUa_FX_Data_PubSubReserveCommunicationIds2DataType_Initialize(void* pValue);

void OpcUa_FX_Data_PubSubReserveCommunicationIds2DataType_Clear(void* pValue);

#endif

#ifndef OPCUA_EXCLUDE_FX_Data_PubSubReserveCommunicationIdsResultDataType
/*============================================================================
 * The FX_Data_PubSubReserveCommunicationIdsResultDataType structure.
 *===========================================================================*/
extern SOPC_EncodeableType OpcUa_FX_Data_PubSubReserveCommunicationIdsResultDataType_EncodeableType;

typedef struct _OpcUa_FX_Data_PubSubReserveCommunicationIdsResultDataType
{
    SOPC_EncodeableType* encodeableType;
    /* IMPORTANT NOTE: response header IN RESPONSE MSG BODY is kept only
     *  for giving a copy of the header to application.
     */
    SOPC_StatusCode Result;
    SOPC_Variant DefaultPublisherId;
    int32_t NoOfWriterGroupIds;
    uint16_t* WriterGroupIds;
    int32_t NoOfDataSetWriterIds;
    uint16_t* DataSetWriterIds;
} OpcUa_FX_Data_PubSubReserveCommunicationIdsResultDataType;

void OpcUa_FX_Data_PubSubReserveCommunicationIdsResultDataType_Initialize(void* pValue);

void OpcUa_FX_Data_PubSubReserveCommunicationIdsResultDataType_Clear(void* pValue);

#endif

#ifndef OPCUA_EXCLUDE_FX_Data_PubSubReserveCommunicationIdsResult2DataType
/*============================================================================
 * The FX_Data_PubSubReserveCommunicationIdsResult2DataType structure.
 *===========================================================================*/
extern SOPC_EncodeableType OpcUa_FX_Data_PubSubReserveCommunicationIdsResult2DataType_EncodeableType;

typedef struct _OpcUa_FX_Data_PubSubReserveCommunicationIdsResult2DataType
{
    SOPC_EncodeableType* encodeableType;
    /* IMPORTANT NOTE: response header IN RESPONSE MSG BODY is kept only
     *  for giving a copy of the header to application.
     */
    SOPC_StatusCode Result;
    SOPC_Variant DefaultPublisherId;
    int32_t NoOfWriterGroupIds;
    uint16_t* WriterGroupIds;
    int32_t NoOfDataSetWriterIds;
    uint16_t* DataSetWriterIds;
    SOPC_Variant TransportSpecificInfo;
} OpcUa_FX_Data_PubSubReserveCommunicationIdsResult2DataType;

void OpcUa_FX_Data_PubSubReserveCommunicationIdsResult2DataType_Initialize(void* pValue);

void OpcUa_FX_Data_PubSubReserveCommunicationIdsResult2DataType_Clear(void* pValue);

#endif

/*============================================================================
 * Indexes in the table of known encodeable types.
 *
 * The enumerated values are indexes in the sopc_FX_Data_KnownEncodeableTypes array.
 *===========================================================================*/
typedef enum _SOPC_FX_Data_TypeInternalIndex
{
#ifndef OPCUA_EXCLUDE_FX_Data_NodeIdArray
    SOPC_TypeInternalIndex_FX_Data_NodeIdArray,
#endif
#ifndef OPCUA_EXCLUDE_FX_Data_NodeIdValuePair
    SOPC_TypeInternalIndex_FX_Data_NodeIdValuePair,
#endif
#ifndef OPCUA_EXCLUDE_FX_Data_AssetVerificationDataType
    SOPC_TypeInternalIndex_FX_Data_AssetVerificationDataType,
#endif
#ifndef OPCUA_EXCLUDE_FX_Data_AssetVerificationResultDataType
    SOPC_TypeInternalIndex_FX_Data_AssetVerificationResultDataType,
#endif
#ifndef OPCUA_EXCLUDE_FX_Data_PubSubCommunicationConfigurationDataType
    SOPC_TypeInternalIndex_FX_Data_PubSubCommunicationConfigurationDataType,
#endif
#ifndef OPCUA_EXCLUDE_FX_Data_PubSubCommunicationConfigurationResultDataType
    SOPC_TypeInternalIndex_FX_Data_PubSubCommunicationConfigurationResultDataType,
#endif
#ifndef OPCUA_EXCLUDE_FX_Data_PubSubCommunicationLinkConfigurationDataType
    SOPC_TypeInternalIndex_FX_Data_PubSubCommunicationLinkConfigurationDataType,
#endif
#ifndef OPCUA_EXCLUDE_FX_Data_ConnectionEndpointDefinitionDataType
    SOPC_TypeInternalIndex_FX_Data_ConnectionEndpointDefinitionDataType,
#endif
#ifndef OPCUA_EXCLUDE_FX_Data_ConnectionEndpointConfigurationDataType
    SOPC_TypeInternalIndex_FX_Data_ConnectionEndpointConfigurationDataType,
#endif
#ifndef OPCUA_EXCLUDE_FX_Data_ConnectionEndpointConfigurationResultDataType
    SOPC_TypeInternalIndex_FX_Data_ConnectionEndpointConfigurationResultDataType,
#endif
#ifndef OPCUA_EXCLUDE_FX_Data_RelatedEndpointDataType
    SOPC_TypeInternalIndex_FX_Data_RelatedEndpointDataType,
#endif
#ifndef OPCUA_EXCLUDE_FX_Data_ConnectionEndpointParameterDataType
    SOPC_TypeInternalIndex_FX_Data_ConnectionEndpointParameterDataType,
#endif
#ifndef OPCUA_EXCLUDE_FX_Data_PubSubConnectionEndpointParameterDataType
    SOPC_TypeInternalIndex_FX_Data_PubSubConnectionEndpointParameterDataType,
#endif
#ifndef OPCUA_EXCLUDE_FX_Data_PubSubReserveCommunicationIdsDataType
    SOPC_TypeInternalIndex_FX_Data_PubSubReserveCommunicationIdsDataType,
#endif
#ifndef OPCUA_EXCLUDE_FX_Data_PubSubReserveCommunicationIds2DataType
    SOPC_TypeInternalIndex_FX_Data_PubSubReserveCommunicationIds2DataType,
#endif
#ifndef OPCUA_EXCLUDE_FX_Data_PubSubReserveCommunicationIdsResultDataType
    SOPC_TypeInternalIndex_FX_Data_PubSubReserveCommunicationIdsResultDataType,
#endif
#ifndef OPCUA_EXCLUDE_FX_Data_PubSubReserveCommunicationIdsResult2DataType
    SOPC_TypeInternalIndex_FX_Data_PubSubReserveCommunicationIdsResult2DataType,
#endif
    SOPC_FX_Data_TypeInternalIndex_SIZE
} SOPC_FX_Data_TypeInternalIndex;

/*============================================================================
 * Table of known types.
 *===========================================================================*/
extern SOPC_EncodeableType** sopc_FX_Data_KnownEncodeableTypes;

#endif
/* This is the last line of an autogenerated file. */
