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

#ifndef SOPC_FX_CM_Types_H_
#define SOPC_FX_CM_Types_H_ 1

#include "fx_cm_enum_types.h"
#include "sopc_buffer.h"
#include "sopc_builtintypes.h"
#include "sopc_encodeabletype.h"

#define SOPC_FX_CM_NS_INDEX 4

#include "sopc_types.h"

#include "fx_data_types.h"

#ifndef OPCUA_EXCLUDE_FX_CM_AddressSelectionDataType
/*============================================================================
 * The FX_CM_AddressSelectionDataType structure.
 *===========================================================================*/
extern SOPC_EncodeableType OpcUa_FX_CM_AddressSelectionDataType_EncodeableType;

typedef struct _OpcUa_FX_CM_AddressSelectionDataType
{
    SOPC_EncodeableType* encodeableType;
    /* IMPORTANT NOTE: response header IN RESPONSE MSG BODY is kept only
     *  for giving a copy of the header to application.
     */
    SOPC_ExtensionObject Address;
    int32_t NoOfAddressSelection;
    SOPC_ExtensionObject* AddressSelection;
    SOPC_Boolean AddressModify;
} OpcUa_FX_CM_AddressSelectionDataType;

void OpcUa_FX_CM_AddressSelectionDataType_Initialize(void* pValue);

void OpcUa_FX_CM_AddressSelectionDataType_Clear(void* pValue);

#endif

#ifndef OPCUA_EXCLUDE_FX_CM_NodeIdentifier
/*============================================================================
 * The FX_CM_NodeIdentifier structure.
 *===========================================================================*/
extern SOPC_EncodeableType OpcUa_FX_CM_NodeIdentifier_EncodeableType;

typedef struct _OpcUa_FX_CM_NodeIdentifier
{
    SOPC_EncodeableType* encodeableType;
    /* IMPORTANT NOTE: response header IN RESPONSE MSG BODY is kept only
     *  for giving a copy of the header to application.
     */
    uint32_t SwitchField;
    union
    {
        SOPC_NodeId Node;
        SOPC_String Alias;
        OpcUa_RelativePath IdentifierBrowsePath;
    } Value;
} OpcUa_FX_CM_NodeIdentifier;

void OpcUa_FX_CM_NodeIdentifier_Initialize(void* pValue);

void OpcUa_FX_CM_NodeIdentifier_Clear(void* pValue);

#endif

#ifndef OPCUA_EXCLUDE_FX_CM_NodeIdentifierValuePair
/*============================================================================
 * The FX_CM_NodeIdentifierValuePair structure.
 *===========================================================================*/
extern SOPC_EncodeableType OpcUa_FX_CM_NodeIdentifierValuePair_EncodeableType;

typedef struct _OpcUa_FX_CM_NodeIdentifierValuePair
{
    SOPC_EncodeableType* encodeableType;
    /* IMPORTANT NOTE: response header IN RESPONSE MSG BODY is kept only
     *  for giving a copy of the header to application.
     */
    OpcUa_FX_CM_NodeIdentifier Key;
    int32_t NoOfArrayIndex;
    uint32_t* ArrayIndex;
    SOPC_Variant Value;
} OpcUa_FX_CM_NodeIdentifierValuePair;

void OpcUa_FX_CM_NodeIdentifierValuePair_Initialize(void* pValue);

void OpcUa_FX_CM_NodeIdentifierValuePair_Clear(void* pValue);

#endif

#ifndef OPCUA_EXCLUDE_FX_CM_AssetVerificationConfDataType
/*============================================================================
 * The FX_CM_AssetVerificationConfDataType structure.
 *===========================================================================*/
extern SOPC_EncodeableType OpcUa_FX_CM_AssetVerificationConfDataType_EncodeableType;

typedef struct _OpcUa_FX_CM_AssetVerificationConfDataType
{
    SOPC_EncodeableType* encodeableType;
    /* IMPORTANT NOTE: response header IN RESPONSE MSG BODY is kept only
     *  for giving a copy of the header to application.
     */
    OpcUa_FX_CM_NodeIdentifier AssetToVerify;
    int32_t VerificationMode;
    int32_t ExpectedVerificationResult;
    int32_t NoOfExpectedVerificationVariables;
    OpcUa_FX_CM_NodeIdentifierValuePair* ExpectedVerificationVariables;
    int32_t NoOfExpectedAdditionalVerificationVariables;
    OpcUa_FX_CM_NodeIdentifierValuePair* ExpectedAdditionalVerificationVariables;
    int32_t* NoOfAssetProperties;
    OpcUa_KeyValuePair* AssetProperties;
} OpcUa_FX_CM_AssetVerificationConfDataType;

void OpcUa_FX_CM_AssetVerificationConfDataType_Initialize(void* pValue);

void OpcUa_FX_CM_AssetVerificationConfDataType_Clear(void* pValue);

#endif

#ifndef OPCUA_EXCLUDE_FX_CM_AutomationComponentConfigurationConfDataType
/*============================================================================
 * The FX_CM_AutomationComponentConfigurationConfDataType structure.
 *===========================================================================*/
extern SOPC_EncodeableType OpcUa_FX_CM_AutomationComponentConfigurationConfDataType_EncodeableType;

typedef struct _OpcUa_FX_CM_AutomationComponentConfigurationConfDataType
{
    SOPC_EncodeableType* encodeableType;
    /* IMPORTANT NOTE: response header IN RESPONSE MSG BODY is kept only
     *  for giving a copy of the header to application.
     */
    SOPC_String BrowseName;
    OpcUa_FX_CM_NodeIdentifier AutomationComponentNode;
    int32_t NoOfAutomationComponentNodeSelection;
    OpcUa_FX_CM_NodeIdentifier* AutomationComponentNodeSelection;
    SOPC_Boolean AutomationComponentNodeModify;
    SOPC_Boolean CommandBundleRequired;
    int32_t NoOfAssetVerification;
    OpcUa_FX_CM_AssetVerificationConfDataType* AssetVerification;
    SOPC_ExtensionObject CommunicationModelConfig;
    int32_t NoOfAutomationComponentProperties;
    OpcUa_KeyValuePair* AutomationComponentProperties;
    int32_t ServerAddressIndex;
} OpcUa_FX_CM_AutomationComponentConfigurationConfDataType;

void OpcUa_FX_CM_AutomationComponentConfigurationConfDataType_Initialize(void* pValue);

void OpcUa_FX_CM_AutomationComponentConfigurationConfDataType_Clear(void* pValue);

#endif

#ifndef OPCUA_EXCLUDE_FX_CM_CommunicationFlowQosDataType
/*============================================================================
 * The FX_CM_CommunicationFlowQosDataType structure.
 *===========================================================================*/
extern SOPC_EncodeableType OpcUa_FX_CM_CommunicationFlowQosDataType_EncodeableType;

typedef struct _OpcUa_FX_CM_CommunicationFlowQosDataType
{
    SOPC_EncodeableType* encodeableType;
    /* IMPORTANT NOTE: response header IN RESPONSE MSG BODY is kept only
     *  for giving a copy of the header to application.
     */
    SOPC_String QosCategory;
    int32_t NoOfTransmitQos;
    SOPC_ExtensionObject* TransmitQos;
    int32_t NoOfReceiveQos;
    SOPC_ExtensionObject* ReceiveQos;
} OpcUa_FX_CM_CommunicationFlowQosDataType;

void OpcUa_FX_CM_CommunicationFlowQosDataType_Initialize(void* pValue);

void OpcUa_FX_CM_CommunicationFlowQosDataType_Clear(void* pValue);

#endif

#ifndef OPCUA_EXCLUDE_FX_CM_ReceiveQosSelectionDataType
/*============================================================================
 * The FX_CM_ReceiveQosSelectionDataType structure.
 *===========================================================================*/
extern SOPC_EncodeableType OpcUa_FX_CM_ReceiveQosSelectionDataType_EncodeableType;

typedef struct _OpcUa_FX_CM_ReceiveQosSelectionDataType
{
    SOPC_EncodeableType* encodeableType;
    /* IMPORTANT NOTE: response header IN RESPONSE MSG BODY is kept only
     *  for giving a copy of the header to application.
     */
    int32_t NoOfReceiveQos;
    SOPC_ExtensionObject* ReceiveQos;
    SOPC_Variant ReceiveQosSelection;
    SOPC_Boolean ReceiveQosModify;
} OpcUa_FX_CM_ReceiveQosSelectionDataType;

void OpcUa_FX_CM_ReceiveQosSelectionDataType_Initialize(void* pValue);

void OpcUa_FX_CM_ReceiveQosSelectionDataType_Clear(void* pValue);

#endif

#ifndef OPCUA_EXCLUDE_FX_CM_SubscriberConfigurationConfDataType
/*============================================================================
 * The FX_CM_SubscriberConfigurationConfDataType structure.
 *===========================================================================*/
extern SOPC_EncodeableType OpcUa_FX_CM_SubscriberConfigurationConfDataType_EncodeableType;

typedef struct _OpcUa_FX_CM_SubscriberConfigurationConfDataType
{
    SOPC_EncodeableType* encodeableType;
    /* IMPORTANT NOTE: response header IN RESPONSE MSG BODY is kept only
     *  for giving a copy of the header to application.
     */
    SOPC_String BrowseName;
    OpcUa_FX_CM_AddressSelectionDataType* Address;
    double MessageReceiveTimeout;
    int32_t* NoOfMessageReceiveTimeoutSelection;
    double* MessageReceiveTimeoutSelection;
    SOPC_Boolean* MessageReceiveTimeoutModify;
    OpcUa_FX_CM_ReceiveQosSelectionDataType* ReceiveQos;
    int32_t* NoOfSubscriberProperties;
    OpcUa_KeyValuePair* SubscriberProperties;
} OpcUa_FX_CM_SubscriberConfigurationConfDataType;

void OpcUa_FX_CM_SubscriberConfigurationConfDataType_Initialize(void* pValue);

void OpcUa_FX_CM_SubscriberConfigurationConfDataType_Clear(void* pValue);

#endif

#ifndef OPCUA_EXCLUDE_FX_CM_PubSubCommunicationFlowConfigurationConfDataType
/*============================================================================
 * The FX_CM_PubSubCommunicationFlowConfigurationConfDataType structure.
 *===========================================================================*/
extern SOPC_EncodeableType OpcUa_FX_CM_PubSubCommunicationFlowConfigurationConfDataType_EncodeableType;

typedef struct _OpcUa_FX_CM_PubSubCommunicationFlowConfigurationConfDataType
{
    SOPC_EncodeableType* encodeableType;
    /* IMPORTANT NOTE: response header IN RESPONSE MSG BODY is kept only
     *  for giving a copy of the header to application.
     */
    OpcUa_FX_CM_AddressSelectionDataType* Address;
    SOPC_String* TransportProfileUri;
    int32_t* NoOfTransportProfileUriSelection;
    SOPC_String* TransportProfileUriSelection;
    SOPC_Boolean* TransportProfileUriModify;
    SOPC_String* HeaderLayoutUri;
    int32_t* NoOfHeaderLayoutUriSelection;
    SOPC_String* HeaderLayoutUriSelection;
    SOPC_Boolean* HeaderLayoutUriModify;
    double* PublishingInterval;
    int32_t* NoOfPublishingIntervalSelection;
    double* PublishingIntervalSelection;
    SOPC_Boolean* PublishingIntervalModify;
    OpcUa_FX_CM_CommunicationFlowQosDataType* Qos;
    int32_t* NoOfQosSelection;
    OpcUa_FX_CM_CommunicationFlowQosDataType* QosSelection;
    SOPC_Boolean* QosModify;
    int32_t* SecurityMode;
    int32_t* NoOfSecurityModeSelection;
    int32_t* SecurityModeSelection;
    SOPC_Boolean* SecurityModeModify;
    SOPC_String* SecurityGroupId;
    int32_t* NoOfSecurityGroupIdSelection;
    SOPC_String* SecurityGroupIdSelection;
    SOPC_Boolean* SecurityGroupIdModify;
    int32_t* NoOfSubscriberConfigurations;
    OpcUa_FX_CM_SubscriberConfigurationConfDataType* SubscriberConfigurations;
} OpcUa_FX_CM_PubSubCommunicationFlowConfigurationConfDataType;

void OpcUa_FX_CM_PubSubCommunicationFlowConfigurationConfDataType_Initialize(void* pValue);

void OpcUa_FX_CM_PubSubCommunicationFlowConfigurationConfDataType_Clear(void* pValue);

#endif

#ifndef OPCUA_EXCLUDE_FX_CM_PortableRelativePathElement
/*============================================================================
 * The FX_CM_PortableRelativePathElement structure.
 *===========================================================================*/
extern SOPC_EncodeableType OpcUa_FX_CM_PortableRelativePathElement_EncodeableType;

typedef struct _OpcUa_FX_CM_PortableRelativePathElement
{
    SOPC_EncodeableType* encodeableType;
    /* IMPORTANT NOTE: response header IN RESPONSE MSG BODY is kept only
     *  for giving a copy of the header to application.
     */
    SOPC_Boolean IsInverse;
    SOPC_Boolean IncludeSubtypes;
} OpcUa_FX_CM_PortableRelativePathElement;

void OpcUa_FX_CM_PortableRelativePathElement_Initialize(void* pValue);

void OpcUa_FX_CM_PortableRelativePathElement_Clear(void* pValue);

#endif

#ifndef OPCUA_EXCLUDE_FX_CM_PortableRelativePath
/*============================================================================
 * The FX_CM_PortableRelativePath structure.
 *===========================================================================*/
extern SOPC_EncodeableType OpcUa_FX_CM_PortableRelativePath_EncodeableType;

typedef struct _OpcUa_FX_CM_PortableRelativePath
{
    SOPC_EncodeableType* encodeableType;
    /* IMPORTANT NOTE: response header IN RESPONSE MSG BODY is kept only
     *  for giving a copy of the header to application.
     */
    int32_t NoOfElements;
    OpcUa_FX_CM_PortableRelativePathElement* Elements;
} OpcUa_FX_CM_PortableRelativePath;

void OpcUa_FX_CM_PortableRelativePath_Initialize(void* pValue);

void OpcUa_FX_CM_PortableRelativePath_Clear(void* pValue);

#endif

#ifndef OPCUA_EXCLUDE_FX_CM_PortableNodeIdentifier
/*============================================================================
 * The FX_CM_PortableNodeIdentifier structure.
 *===========================================================================*/
extern SOPC_EncodeableType OpcUa_FX_CM_PortableNodeIdentifier_EncodeableType;

typedef struct _OpcUa_FX_CM_PortableNodeIdentifier
{
    SOPC_EncodeableType* encodeableType;
    /* IMPORTANT NOTE: response header IN RESPONSE MSG BODY is kept only
     *  for giving a copy of the header to application.
     */
    uint32_t SwitchField;
    union
    {
        SOPC_String Alias;
        OpcUa_FX_CM_PortableRelativePath IdentifierBrowsePath;
    } Value;
} OpcUa_FX_CM_PortableNodeIdentifier;

void OpcUa_FX_CM_PortableNodeIdentifier_Initialize(void* pValue);

void OpcUa_FX_CM_PortableNodeIdentifier_Clear(void* pValue);

#endif

#ifndef OPCUA_EXCLUDE_FX_CM_NodeIdTranslationDataType
/*============================================================================
 * The FX_CM_NodeIdTranslationDataType structure.
 *===========================================================================*/
extern SOPC_EncodeableType OpcUa_FX_CM_NodeIdTranslationDataType_EncodeableType;

typedef struct _OpcUa_FX_CM_NodeIdTranslationDataType
{
    SOPC_EncodeableType* encodeableType;
    /* IMPORTANT NOTE: response header IN RESPONSE MSG BODY is kept only
     *  for giving a copy of the header to application.
     */
    SOPC_NodeId NodePlaceholder;
    OpcUa_FX_CM_PortableNodeIdentifier PortableNode;
} OpcUa_FX_CM_NodeIdTranslationDataType;

void OpcUa_FX_CM_NodeIdTranslationDataType_Initialize(void* pValue);

void OpcUa_FX_CM_NodeIdTranslationDataType_Clear(void* pValue);

#endif

#ifndef OPCUA_EXCLUDE_FX_CM_PubSubCommunicationModelConfigurationDataType
/*============================================================================
 * The FX_CM_PubSubCommunicationModelConfigurationDataType structure.
 *===========================================================================*/
extern SOPC_EncodeableType OpcUa_FX_CM_PubSubCommunicationModelConfigurationDataType_EncodeableType;

typedef struct _OpcUa_FX_CM_PubSubCommunicationModelConfigurationDataType
{
    SOPC_EncodeableType* encodeableType;
    /* IMPORTANT NOTE: response header IN RESPONSE MSG BODY is kept only
     *  for giving a copy of the header to application.
     */
    int32_t NoOfTranslationTable;
    OpcUa_FX_CM_NodeIdTranslationDataType* TranslationTable;
} OpcUa_FX_CM_PubSubCommunicationModelConfigurationDataType;

void OpcUa_FX_CM_PubSubCommunicationModelConfigurationDataType_Initialize(void* pValue);

void OpcUa_FX_CM_PubSubCommunicationModelConfigurationDataType_Clear(void* pValue);

#endif

#ifndef OPCUA_EXCLUDE_FX_CM_ConnectionEndpointConfigurationConfDataType
/*============================================================================
 * The FX_CM_ConnectionEndpointConfigurationConfDataType structure.
 *===========================================================================*/
extern SOPC_EncodeableType OpcUa_FX_CM_ConnectionEndpointConfigurationConfDataType_EncodeableType;

typedef struct _OpcUa_FX_CM_ConnectionEndpointConfigurationConfDataType
{
    SOPC_EncodeableType* encodeableType;
    /* IMPORTANT NOTE: response header IN RESPONSE MSG BODY is kept only
     *  for giving a copy of the header to application.
     */
    OpcUa_FX_CM_NodeIdentifier FunctionalEntityNode;
    int32_t* NoOfFunctionalEntityNodeSelection;
    OpcUa_FX_CM_NodeIdentifier* FunctionalEntityNodeSelection;
    SOPC_Boolean* FunctionalEntityNodeModify;
    SOPC_String Name;
    int32_t* NoOfNameSelection;
    SOPC_String* NameSelection;
    SOPC_Boolean* NameModify;
    SOPC_NodeId ConnectionEndpointTypeId;
    int32_t* NoOfInputVariableIds;
    OpcUa_FX_CM_NodeIdentifier* InputVariableIds;
    int32_t* NoOfOutputVariableIds;
    OpcUa_FX_CM_NodeIdentifier* OutputVariableIds;
    SOPC_Boolean IsPersistent;
    double CleanupTimeout;
    SOPC_Boolean IsPreconfigured;
    SOPC_ExtensionObject* CommunicationLinks;
    SOPC_String* PreconfiguredPublishedDataSet;
    OpcUa_PublishedDataSetDataType* PublishedDataSetData;
    SOPC_String* PreconfiguredSubscribedDataSet;
    int32_t* NoOfExpectedVerificationVariables;
    OpcUa_FX_CM_NodeIdentifierValuePair* ExpectedVerificationVariables;
    int32_t* NoOfControlGroups;
    OpcUa_FX_CM_NodeIdentifier* ControlGroups;
    int32_t* NoOfConfigurationData;
    OpcUa_FX_CM_NodeIdentifierValuePair* ConfigurationData;
    int32_t* NoOfEndpointProperties;
    OpcUa_KeyValuePair* EndpointProperties;
    int32_t AutomationComponentIndex;
    int32_t* OutboundFlowIndex;
    int32_t* NoOfInboundFlowIndex;
    int32_t* InboundFlowIndex;
} OpcUa_FX_CM_ConnectionEndpointConfigurationConfDataType;

void OpcUa_FX_CM_ConnectionEndpointConfigurationConfDataType_Initialize(void* pValue);

void OpcUa_FX_CM_ConnectionEndpointConfigurationConfDataType_Clear(void* pValue);

#endif

#ifndef OPCUA_EXCLUDE_FX_CM_ConnectionConfigurationConfDataType
/*============================================================================
 * The FX_CM_ConnectionConfigurationConfDataType structure.
 *===========================================================================*/
extern SOPC_EncodeableType OpcUa_FX_CM_ConnectionConfigurationConfDataType_EncodeableType;

typedef struct _OpcUa_FX_CM_ConnectionConfigurationConfDataType
{
    SOPC_EncodeableType* encodeableType;
    /* IMPORTANT NOTE: response header IN RESPONSE MSG BODY is kept only
     *  for giving a copy of the header to application.
     */
    SOPC_String BrowseName;
    OpcUa_FX_CM_ConnectionEndpointConfigurationConfDataType Endpoint1;
    OpcUa_FX_CM_ConnectionEndpointConfigurationConfDataType* Endpoint2;
    int32_t* NoOfConnectionProperties;
    OpcUa_KeyValuePair* ConnectionProperties;
} OpcUa_FX_CM_ConnectionConfigurationConfDataType;

void OpcUa_FX_CM_ConnectionConfigurationConfDataType_Initialize(void* pValue);

void OpcUa_FX_CM_ConnectionConfigurationConfDataType_Clear(void* pValue);

#endif

#ifndef OPCUA_EXCLUDE_FX_CM_ServerAddressConfDataType
/*============================================================================
 * The FX_CM_ServerAddressConfDataType structure.
 *===========================================================================*/
extern SOPC_EncodeableType OpcUa_FX_CM_ServerAddressConfDataType_EncodeableType;

typedef struct _OpcUa_FX_CM_ServerAddressConfDataType
{
    SOPC_EncodeableType* encodeableType;
    /* IMPORTANT NOTE: response header IN RESPONSE MSG BODY is kept only
     *  for giving a copy of the header to application.
     */
    SOPC_String BrowseName;
    SOPC_String Address;
    int32_t* NoOfAddressSelection;
    SOPC_String* AddressSelection;
    SOPC_Boolean* AddressModify;
    int32_t SecurityMode;
    int32_t* NoOfSecurityModeSelection;
    int32_t* SecurityModeSelection;
    SOPC_Boolean* SecurityModeModify;
    SOPC_String SecurityPolicyUri;
    int32_t* NoOfSecurityPolicyUriSelection;
    SOPC_String* SecurityPolicyUriSelection;
    SOPC_Boolean* SecurityPolicyUriModify;
    SOPC_String ServerUri;
    int32_t* NoOfServerUriSelection;
    SOPC_String* ServerUriSelection;
    SOPC_Boolean* ServerUriModify;
    int32_t* NoOfServerProperties;
    OpcUa_KeyValuePair* ServerProperties;
    int32_t NoOfNamespaces;
    SOPC_String* Namespaces;
} OpcUa_FX_CM_ServerAddressConfDataType;

void OpcUa_FX_CM_ServerAddressConfDataType_Initialize(void* pValue);

void OpcUa_FX_CM_ServerAddressConfDataType_Clear(void* pValue);

#endif

#ifndef OPCUA_EXCLUDE_FX_CM_SecurityKeyServerAddressConfDataType
/*============================================================================
 * The FX_CM_SecurityKeyServerAddressConfDataType structure.
 *===========================================================================*/
extern SOPC_EncodeableType OpcUa_FX_CM_SecurityKeyServerAddressConfDataType_EncodeableType;

typedef struct _OpcUa_FX_CM_SecurityKeyServerAddressConfDataType
{
    SOPC_EncodeableType* encodeableType;
    /* IMPORTANT NOTE: response header IN RESPONSE MSG BODY is kept only
     *  for giving a copy of the header to application.
     */
    SOPC_String Address;
    int32_t* NoOfAddressSelection;
    SOPC_String* AddressSelection;
    SOPC_Boolean* AddressModify;
    SOPC_String SecurityPolicyUri;
    int32_t* NoOfSecurityPolicyUriSelection;
    SOPC_String* SecurityPolicyUriSelection;
    SOPC_Boolean* SecurityPolicyUriModify;
    SOPC_String ServerUri;
    int32_t* NoOfServerUriSelection;
    SOPC_String* ServerUriSelection;
    SOPC_Boolean* ServerUriModify;
    SOPC_Boolean UsePushModel;
    int32_t* NoOfSksProperties;
    OpcUa_KeyValuePair* SksProperties;
} OpcUa_FX_CM_SecurityKeyServerAddressConfDataType;

void OpcUa_FX_CM_SecurityKeyServerAddressConfDataType_Initialize(void* pValue);

void OpcUa_FX_CM_SecurityKeyServerAddressConfDataType_Clear(void* pValue);

#endif

#ifndef OPCUA_EXCLUDE_FX_CM_ConnectionConfigurationSetConfDataType
/*============================================================================
 * The FX_CM_ConnectionConfigurationSetConfDataType structure.
 *===========================================================================*/
extern SOPC_EncodeableType OpcUa_FX_CM_ConnectionConfigurationSetConfDataType_EncodeableType;

typedef struct _OpcUa_FX_CM_ConnectionConfigurationSetConfDataType
{
    SOPC_EncodeableType* encodeableType;
    /* IMPORTANT NOTE: response header IN RESPONSE MSG BODY is kept only
     *  for giving a copy of the header to application.
     */
    SOPC_String BrowseName;
    int32_t NoOfConnectionConfigurationSetFolder;
    SOPC_String* ConnectionConfigurationSetFolder;
    int32_t NoOfConnections;
    OpcUa_FX_CM_ConnectionConfigurationConfDataType* Connections;
    int32_t NoOfCommunicationFlows;
    SOPC_ExtensionObject* CommunicationFlows;
    int32_t NoOfServerAddresses;
    OpcUa_FX_CM_ServerAddressConfDataType* ServerAddresses;
    int32_t NoOfAutomationComponentConfigurations;
    OpcUa_FX_CM_AutomationComponentConfigurationConfDataType* AutomationComponentConfigurations;
    SOPC_Boolean RollbackOnError;
    OpcUa_FX_CM_SecurityKeyServerAddressConfDataType SecurityKeyServer;
    uint32_t Version;
    int32_t NoOfConnectionConfigurationSetProperties;
    OpcUa_KeyValuePair* ConnectionConfigurationSetProperties;
} OpcUa_FX_CM_ConnectionConfigurationSetConfDataType;

void OpcUa_FX_CM_ConnectionConfigurationSetConfDataType_Initialize(void* pValue);

void OpcUa_FX_CM_ConnectionConfigurationSetConfDataType_Clear(void* pValue);

#endif

#ifndef OPCUA_EXCLUDE_FX_CM_ConnectionDiagnosticsDataType
/*============================================================================
 * The FX_CM_ConnectionDiagnosticsDataType structure.
 *===========================================================================*/
extern SOPC_EncodeableType OpcUa_FX_CM_ConnectionDiagnosticsDataType_EncodeableType;

typedef struct _OpcUa_FX_CM_ConnectionDiagnosticsDataType
{
    SOPC_EncodeableType* encodeableType;
    /* IMPORTANT NOTE: response header IN RESPONSE MSG BODY is kept only
     *  for giving a copy of the header to application.
     */
    SOPC_QualifiedName Name;
    /* ::OpcUa_FX_CM_LastActivityMask */ uint16_t LastActivity;
    OpcUa_FX_CM_ConnectionStateEnum ConnectionState;
    OpcUa_FX_CM_FxErrorEnum ErrorEndpoint1;
    SOPC_StatusCode Endpoint1Status;
    OpcUa_FX_CM_FxErrorEnum ErrorEndpoint2;
    SOPC_StatusCode Endpoint2Status;
} OpcUa_FX_CM_ConnectionDiagnosticsDataType;

void OpcUa_FX_CM_ConnectionDiagnosticsDataType_Initialize(void* pValue);

void OpcUa_FX_CM_ConnectionDiagnosticsDataType_Clear(void* pValue);

#endif

#ifndef OPCUA_EXCLUDE_FX_CM_NodeIdTranslationConfDataType
/*============================================================================
 * The FX_CM_NodeIdTranslationConfDataType structure.
 *===========================================================================*/
extern SOPC_EncodeableType OpcUa_FX_CM_NodeIdTranslationConfDataType_EncodeableType;

typedef struct _OpcUa_FX_CM_NodeIdTranslationConfDataType
{
    SOPC_EncodeableType* encodeableType;
    /* IMPORTANT NOTE: response header IN RESPONSE MSG BODY is kept only
     *  for giving a copy of the header to application.
     */
    SOPC_NodeId NodePlaceholder;
    OpcUa_FX_CM_NodeIdentifier Node;
} OpcUa_FX_CM_NodeIdTranslationConfDataType;

void OpcUa_FX_CM_NodeIdTranslationConfDataType_Initialize(void* pValue);

void OpcUa_FX_CM_NodeIdTranslationConfDataType_Clear(void* pValue);

#endif

#ifndef OPCUA_EXCLUDE_FX_CM_PortableKeyValuePair
/*============================================================================
 * The FX_CM_PortableKeyValuePair structure.
 *===========================================================================*/
extern SOPC_EncodeableType OpcUa_FX_CM_PortableKeyValuePair_EncodeableType;

typedef struct _OpcUa_FX_CM_PortableKeyValuePair
{
    SOPC_EncodeableType* encodeableType;
    /* IMPORTANT NOTE: response header IN RESPONSE MSG BODY is kept only
     *  for giving a copy of the header to application.
     */
    SOPC_Variant Value;
} OpcUa_FX_CM_PortableKeyValuePair;

void OpcUa_FX_CM_PortableKeyValuePair_Initialize(void* pValue);

void OpcUa_FX_CM_PortableKeyValuePair_Clear(void* pValue);

#endif

#ifndef OPCUA_EXCLUDE_FX_CM_PortableNodeIdentifierValuePair
/*============================================================================
 * The FX_CM_PortableNodeIdentifierValuePair structure.
 *===========================================================================*/
extern SOPC_EncodeableType OpcUa_FX_CM_PortableNodeIdentifierValuePair_EncodeableType;

typedef struct _OpcUa_FX_CM_PortableNodeIdentifierValuePair
{
    SOPC_EncodeableType* encodeableType;
    /* IMPORTANT NOTE: response header IN RESPONSE MSG BODY is kept only
     *  for giving a copy of the header to application.
     */
    OpcUa_FX_CM_PortableNodeIdentifier Key;
    int32_t NoOfArrayIndex;
    uint32_t* ArrayIndex;
    SOPC_Variant Value;
} OpcUa_FX_CM_PortableNodeIdentifierValuePair;

void OpcUa_FX_CM_PortableNodeIdentifierValuePair_Initialize(void* pValue);

void OpcUa_FX_CM_PortableNodeIdentifierValuePair_Clear(void* pValue);

#endif

#ifndef OPCUA_EXCLUDE_FX_CM_SecurityKeyServerAddressDataType
/*============================================================================
 * The FX_CM_SecurityKeyServerAddressDataType structure.
 *===========================================================================*/
extern SOPC_EncodeableType OpcUa_FX_CM_SecurityKeyServerAddressDataType_EncodeableType;

typedef struct _OpcUa_FX_CM_SecurityKeyServerAddressDataType
{
    SOPC_EncodeableType* encodeableType;
    /* IMPORTANT NOTE: response header IN RESPONSE MSG BODY is kept only
     *  for giving a copy of the header to application.
     */
    SOPC_String Address;
    SOPC_String SecurityPolicyUri;
    SOPC_String ServerUri;
    SOPC_Boolean UsePushModel;
} OpcUa_FX_CM_SecurityKeyServerAddressDataType;

void OpcUa_FX_CM_SecurityKeyServerAddressDataType_Initialize(void* pValue);

void OpcUa_FX_CM_SecurityKeyServerAddressDataType_Clear(void* pValue);

#endif

#ifndef OPCUA_EXCLUDE_FX_CM_ServerAddressDataType
/*============================================================================
 * The FX_CM_ServerAddressDataType structure.
 *===========================================================================*/
extern SOPC_EncodeableType OpcUa_FX_CM_ServerAddressDataType_EncodeableType;

typedef struct _OpcUa_FX_CM_ServerAddressDataType
{
    SOPC_EncodeableType* encodeableType;
    /* IMPORTANT NOTE: response header IN RESPONSE MSG BODY is kept only
     *  for giving a copy of the header to application.
     */
    SOPC_String Address;
    int32_t SecurityMode;
    SOPC_String SecurityPolicyUri;
    SOPC_String ServerUri;
} OpcUa_FX_CM_ServerAddressDataType;

void OpcUa_FX_CM_ServerAddressDataType_Initialize(void* pValue);

void OpcUa_FX_CM_ServerAddressDataType_Clear(void* pValue);

#endif

/*============================================================================
 * Indexes in the table of known encodeable types.
 *
 * The enumerated values are indexes in the sopc_FX_CM_KnownEncodeableTypes array.
 *===========================================================================*/
typedef enum _SOPC_FX_CM_TypeInternalIndex
{
#ifndef OPCUA_EXCLUDE_FX_CM_AddressSelectionDataType
    SOPC_TypeInternalIndex_FX_CM_AddressSelectionDataType,
#endif
#ifndef OPCUA_EXCLUDE_FX_CM_NodeIdentifier
    SOPC_TypeInternalIndex_FX_CM_NodeIdentifier,
#endif
#ifndef OPCUA_EXCLUDE_FX_CM_NodeIdentifierValuePair
    SOPC_TypeInternalIndex_FX_CM_NodeIdentifierValuePair,
#endif
#ifndef OPCUA_EXCLUDE_FX_CM_AssetVerificationConfDataType
    SOPC_TypeInternalIndex_FX_CM_AssetVerificationConfDataType,
#endif
#ifndef OPCUA_EXCLUDE_FX_CM_AutomationComponentConfigurationConfDataType
    SOPC_TypeInternalIndex_FX_CM_AutomationComponentConfigurationConfDataType,
#endif
#ifndef OPCUA_EXCLUDE_FX_CM_CommunicationFlowQosDataType
    SOPC_TypeInternalIndex_FX_CM_CommunicationFlowQosDataType,
#endif
#ifndef OPCUA_EXCLUDE_FX_CM_ReceiveQosSelectionDataType
    SOPC_TypeInternalIndex_FX_CM_ReceiveQosSelectionDataType,
#endif
#ifndef OPCUA_EXCLUDE_FX_CM_SubscriberConfigurationConfDataType
    SOPC_TypeInternalIndex_FX_CM_SubscriberConfigurationConfDataType,
#endif
#ifndef OPCUA_EXCLUDE_FX_CM_PubSubCommunicationFlowConfigurationConfDataType
    SOPC_TypeInternalIndex_FX_CM_PubSubCommunicationFlowConfigurationConfDataType,
#endif
#ifndef OPCUA_EXCLUDE_FX_CM_PortableRelativePathElement
    SOPC_TypeInternalIndex_FX_CM_PortableRelativePathElement,
#endif
#ifndef OPCUA_EXCLUDE_FX_CM_PortableRelativePath
    SOPC_TypeInternalIndex_FX_CM_PortableRelativePath,
#endif
#ifndef OPCUA_EXCLUDE_FX_CM_PortableNodeIdentifier
    SOPC_TypeInternalIndex_FX_CM_PortableNodeIdentifier,
#endif
#ifndef OPCUA_EXCLUDE_FX_CM_NodeIdTranslationDataType
    SOPC_TypeInternalIndex_FX_CM_NodeIdTranslationDataType,
#endif
#ifndef OPCUA_EXCLUDE_FX_CM_PubSubCommunicationModelConfigurationDataType
    SOPC_TypeInternalIndex_FX_CM_PubSubCommunicationModelConfigurationDataType,
#endif
#ifndef OPCUA_EXCLUDE_FX_CM_ConnectionEndpointConfigurationConfDataType
    SOPC_TypeInternalIndex_FX_CM_ConnectionEndpointConfigurationConfDataType,
#endif
#ifndef OPCUA_EXCLUDE_FX_CM_ConnectionConfigurationConfDataType
    SOPC_TypeInternalIndex_FX_CM_ConnectionConfigurationConfDataType,
#endif
#ifndef OPCUA_EXCLUDE_FX_CM_ServerAddressConfDataType
    SOPC_TypeInternalIndex_FX_CM_ServerAddressConfDataType,
#endif
#ifndef OPCUA_EXCLUDE_FX_CM_SecurityKeyServerAddressConfDataType
    SOPC_TypeInternalIndex_FX_CM_SecurityKeyServerAddressConfDataType,
#endif
#ifndef OPCUA_EXCLUDE_FX_CM_ConnectionConfigurationSetConfDataType
    SOPC_TypeInternalIndex_FX_CM_ConnectionConfigurationSetConfDataType,
#endif
#ifndef OPCUA_EXCLUDE_FX_CM_ConnectionDiagnosticsDataType
    SOPC_TypeInternalIndex_FX_CM_ConnectionDiagnosticsDataType,
#endif
#ifndef OPCUA_EXCLUDE_FX_CM_NodeIdTranslationConfDataType
    SOPC_TypeInternalIndex_FX_CM_NodeIdTranslationConfDataType,
#endif
#ifndef OPCUA_EXCLUDE_FX_CM_PortableKeyValuePair
    SOPC_TypeInternalIndex_FX_CM_PortableKeyValuePair,
#endif
#ifndef OPCUA_EXCLUDE_FX_CM_PortableNodeIdentifierValuePair
    SOPC_TypeInternalIndex_FX_CM_PortableNodeIdentifierValuePair,
#endif
#ifndef OPCUA_EXCLUDE_FX_CM_SecurityKeyServerAddressDataType
    SOPC_TypeInternalIndex_FX_CM_SecurityKeyServerAddressDataType,
#endif
#ifndef OPCUA_EXCLUDE_FX_CM_ServerAddressDataType
    SOPC_TypeInternalIndex_FX_CM_ServerAddressDataType,
#endif
    SOPC_FX_CM_TypeInternalIndex_SIZE
} SOPC_FX_CM_TypeInternalIndex;

/*============================================================================
 * Table of known types.
 *===========================================================================*/
extern SOPC_EncodeableType** sopc_FX_CM_KnownEncodeableTypes;

#endif
/* This is the last line of an autogenerated file. */
