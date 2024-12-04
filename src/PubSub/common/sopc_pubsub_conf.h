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

#ifndef SOPC_PUBSUB_CONF_H_
#define SOPC_PUBSUB_CONF_H_

#include "sopc_builtintypes.h"
#include "sopc_key_manager.h"
#include "sopc_pubsub_constants.h"
#include "sopc_types.h"

typedef struct SOPC_PubSubConfiguration SOPC_PubSubConfiguration;
typedef struct SOPC_PubSubConnection SOPC_PubSubConnection;
typedef struct SOPC_PublishedDataSet SOPC_PublishedDataSet;
typedef struct SOPC_WriterGroup SOPC_WriterGroup;
typedef struct SOPC_SecurityKeyServices SOPC_SecurityKeyServices;
typedef struct SOPC_DataSetWriter SOPC_DataSetWriter;
typedef struct SOPC_ReaderGroup SOPC_ReaderGroup;
typedef struct SOPC_DataSetReader SOPC_DataSetReader;
typedef struct SOPC_SubscribedDataSet SOPC_SubscribedDataSet;
typedef struct SOPC_DataSetMetaData SOPC_DataSetMetaData;
typedef struct SOPC_FieldMetaData SOPC_FieldMetaData;
typedef struct SOPC_FieldTarget SOPC_FieldTarget;

typedef struct SOPC_PubSub_ArrayDimension
{
    int32_t valueRank;         // -1 or >0 (Other values not supported)
    uint32_t* arrayDimensions; // NULL if valueRank is == -1, an array of length ::valueRank otherwise
} SOPC_PubSub_ArrayDimension;

typedef struct SOPC_UADP_Configuration
{
    bool PublisherIdFlag;
    bool GroupHeaderFlag;
    bool GroupIdFlag;
    bool GroupVersionFlag;
    bool NetworkMessageNumberFlag;
    bool SequenceNumberFlag;
    bool PayloadHeaderFlag;
    bool TimestampFlag;
    bool PicoSecondsFlag;
    bool DataSetClassIdFlag;
    bool SecurityFlag;
    bool PromotedFieldsFlag;

} SOPC_UADP_Configuration;

typedef SOPC_UADP_Configuration SOPC_UadpNetworkMessageContentMask;

typedef struct SOPC_PublishedVariable SOPC_PublishedVariable;

/**
 * Notifies a fatal error on a Pub or Sub connection, that is not automatically recoverable
 * @param userContext The user context.
 * @param message A textual information about the error
 */
typedef void SOPC_PubSub_OnFatalError(void* userContext, const char* message);

typedef enum
{
    SOPC_MessageEncodeUADP = 0,
    SOPC_MessageEncodeJSON
} SOPC_Pubsub_MessageEncodingType;

typedef enum
{
    SOPC_PubSubConnection_Pub = 0,
    SOPC_PubSubConnection_Sub = 1
} SOPC_PubSubConnection_Type;

/* There is no functional difference between SOPC_PublishedEventsDataType and SOPC_PublishedDataSetCustomSourceDataType
 * by now.*/
typedef enum
{
    SOPC_PublishedDataItemsDataType = 0,
    SOPC_PublishedEventsDataType = 1,
    SOPC_PublishedDataSetCustomSourceDataType = 2
} SOPC_PublishedDataSetSourceType;

typedef enum
{
    SOPC_TargetVariablesDataType = 0,
    SOPC_SubscribedDataSetMirrorDataType = 1
} SOPC_SubscribedDataSetType;

typedef enum
{
    SOPC_SecurityMode_Invalid = 0,
    SOPC_SecurityMode_None = 1,
    SOPC_SecurityMode_Sign = 2,
    SOPC_SecurityMode_SignAndEncrypt = 3
} SOPC_SecurityMode_Type;

typedef enum
{
    SOPC_Null_PublisherId = 0,
    SOPC_UInteger_PublisherId = 1,
    SOPC_String_PublisherId = 2,
} SOPC_Conf_PublisherId_Type;

typedef struct SOPC_Conf_PublisherId
{
    SOPC_Conf_PublisherId_Type type;
    union
    {
        uint64_t uint;
        SOPC_String string;
    } data;
} SOPC_Conf_PublisherId;

/**
 * \brief Create the default MQTT topic described by the OPC UA standard (publisherId.groupId)
 *
 * \param publisherId A non-NULL pointer to the Publisher ID of the connection.
 *                    Only Integer-type publisher Id are supported currently
 * \param groupId     A uint16_t value to the Group ID of the message
 *
 * \return The defaultTopic as a string (char*)
 *         The return value shall be freed by caller after use
 */
char* SOPC_Allocate_MQTT_DefaultTopic(const SOPC_Conf_PublisherId* publisherId, uint16_t groupId);

/*************************/
/** PubSubConfiguration **/
/*************************/
SOPC_PubSubConfiguration* SOPC_PubSubConfiguration_Create(void);
void SOPC_PubSubConfiguration_Delete(SOPC_PubSubConfiguration* configuration);

// Publisher Connection
bool SOPC_PubSubConfiguration_Allocate_PubConnection_Array(SOPC_PubSubConfiguration* configuration, uint32_t nb);
uint32_t SOPC_PubSubConfiguration_Nb_PubConnection(const SOPC_PubSubConfiguration* configuration);
SOPC_PubSubConnection* SOPC_PubSubConfiguration_Get_PubConnection_At(const SOPC_PubSubConfiguration* configuration,
                                                                     uint32_t index);
// Subscriber Connection
bool SOPC_PubSubConfiguration_Allocate_SubConnection_Array(SOPC_PubSubConfiguration* configuration, uint32_t nb);
uint32_t SOPC_PubSubConfiguration_Nb_SubConnection(const SOPC_PubSubConfiguration* configuration);

SOPC_PubSubConnection* SOPC_PubSubConfiguration_Get_SubConnection_At(const SOPC_PubSubConfiguration* configuration,
                                                                     uint32_t index);
// Publisher Only
bool SOPC_PubSubConfiguration_Allocate_PublishedDataSet_Array(SOPC_PubSubConfiguration* configuration, uint32_t nb);
uint32_t SOPC_PubSubConfiguration_Nb_PublishedDataSet(const SOPC_PubSubConfiguration* configuration);
SOPC_PublishedDataSet* SOPC_PubSubConfiguration_Get_PublishedDataSet_At(const SOPC_PubSubConfiguration* configuration,
                                                                        uint16_t index);

/**********************/
/** PubSubConnection **/
/**********************/

const char* SOPC_PubSubConnection_Get_Name(const SOPC_PubSubConnection* connection);
// make a copy of name
bool SOPC_PubSubConnection_Set_Name(SOPC_PubSubConnection* connection, const char* name);
bool SOPC_PubSubConnection_Is_Enabled(const SOPC_PubSubConnection* connection);
void SOPC_PubSubConnection_Set_Enabled(SOPC_PubSubConnection* connection, bool enabled);

void SOPC_PubSubConfiguration_Set_FatalError_Callback(SOPC_PubSubConnection* connection,
                                                      SOPC_PubSub_OnFatalError* callback);
SOPC_PubSub_OnFatalError* SOPC_PubSubConfiguration_Get_FatalError_Callback(SOPC_PubSubConnection* connection);

/* Publisher only */
void SOPC_PubSubConnection_Set_AcyclicPublisher(SOPC_PubSubConnection* connection, bool acyclicPublisher);
bool SOPC_PubSubConnection_Get_AcyclicPublisher(const SOPC_PubSubConnection* connection);

const SOPC_Conf_PublisherId* SOPC_PubSubConnection_Get_PublisherId(const SOPC_PubSubConnection* connection);
void SOPC_PubSubConnection_Set_PublisherId_UInteger(SOPC_PubSubConnection* connection, uint64_t id);
// make a copy of id
bool SOPC_PubSubConnection_Set_PublisherId_String(SOPC_PubSubConnection* connection, const SOPC_String* id);

const char* SOPC_PubSubConnection_Get_TransportProfileUri(const SOPC_PubSubConnection* connection);
bool SOPC_PubSubConnection_Set_TransportProfileUri(SOPC_PubSubConnection* connection, const char* uri);

const char* SOPC_PubSubConnection_Get_Address(const SOPC_PubSubConnection* connection);
bool SOPC_PubSubConnection_Set_Address(SOPC_PubSubConnection* connection, const char* address);

// Used for OPC UA Ethernet only, mandatory for Publisher connection
const char* SOPC_PubSubConnection_Get_InterfaceName(const SOPC_PubSubConnection* connection);
bool SOPC_PubSubConnection_Set_InterfaceName(SOPC_PubSubConnection* connection, const char* interfaceName);

const char* SOPC_PubSubConnection_Get_MqttUsername(const SOPC_PubSubConnection* connection);
bool SOPC_PubSubConnection_Set_MqttUsername(SOPC_PubSubConnection* connection, const char* username);

const char* SOPC_PubSubConnection_Get_MqttPassword(const SOPC_PubSubConnection* connection);
bool SOPC_PubSubConnection_Set_MqttPassword(SOPC_PubSubConnection* connection, const char* password);

bool SOPC_PubSubConnection_Allocate_WriterGroup_Array(SOPC_PubSubConnection* connection, uint16_t nb);
uint16_t SOPC_PubSubConnection_Nb_WriterGroup(const SOPC_PubSubConnection* connection);
SOPC_WriterGroup* SOPC_PubSubConnection_Get_WriterGroup_At(const SOPC_PubSubConnection* connection, uint16_t index);

bool SOPC_PubSubConnection_Allocate_ReaderGroup_Array(SOPC_PubSubConnection* connection, uint16_t nb);
uint16_t SOPC_PubSubConnection_Nb_ReaderGroup(const SOPC_PubSubConnection* connection);
SOPC_ReaderGroup* SOPC_PubSubConnection_Get_ReaderGroup_At(const SOPC_PubSubConnection* connection, uint16_t index);

/******************/
/** Reader Group **/
/******************/
SOPC_SecurityMode_Type SOPC_ReaderGroup_Get_SecurityMode(const SOPC_ReaderGroup* group);
void SOPC_ReaderGroup_Set_SecurityMode(SOPC_ReaderGroup* group, SOPC_SecurityMode_Type mode);
// make a copy of id
bool SOPC_ReaderGroup_Set_SecurityGroupId(SOPC_ReaderGroup* group, char* securityGroupId);
bool SOPC_ReaderGroup_Allocate_SecurityKeyServices_Array(SOPC_ReaderGroup* group, uint32_t nb);
uint32_t SOPC_ReaderGroup_Nb_SecurityKeyServices(const SOPC_ReaderGroup* group);
SOPC_SecurityKeyServices* SOPC_ReaderGroup_Get_SecurityKeyServices_At(const SOPC_ReaderGroup* group, uint32_t index);

bool SOPC_ReaderGroup_Allocate_DataSetReader_Array(SOPC_ReaderGroup* group, uint8_t nb);
uint8_t SOPC_ReaderGroup_Nb_DataSetReader(const SOPC_ReaderGroup* group);
SOPC_DataSetReader* SOPC_ReaderGroup_Get_DataSetReader_At(const SOPC_ReaderGroup* group, uint8_t index);

uint16_t SOPC_ReaderGroup_Get_GroupId(const SOPC_ReaderGroup* group);
void SOPC_ReaderGroup_Set_GroupId(SOPC_ReaderGroup* group, uint16_t id);

uint32_t SOPC_ReaderGroup_Get_GroupVersion(const SOPC_ReaderGroup* group);
void SOPC_ReaderGroup_Set_GroupVersion(SOPC_ReaderGroup* group, uint32_t version);

const SOPC_Conf_PublisherId* SOPC_ReaderGroup_Get_PublisherId(SOPC_ReaderGroup* group);
void SOPC_ReaderGroup_Set_PublisherId_UInteger(SOPC_ReaderGroup* group, uint64_t id);
bool SOPC_ReaderGroup_Set_PublisherId_String(SOPC_ReaderGroup* group, const SOPC_String* id);

const char* SOPC_ReaderGroup_Get_MqttTopic(const SOPC_ReaderGroup* reader);

/**
 * Set MqttTopic on the ReaderGroup structure
 * If topic parameter is NULL, set mqttTopic to NULL
 * Otherwise, topic parameter is copied internally and may be freed/modified by caller after call.
 */
void SOPC_ReaderGroup_Set_MqttTopic(SOPC_ReaderGroup* reader, const char* topic);

/*******************/
/** DataSetReader **/
/*******************/

SOPC_ReaderGroup* SOPC_DataSetReader_Get_ReaderGroup(const SOPC_DataSetReader* reader);
uint16_t SOPC_DataSetReader_Get_DataSetWriterId(const SOPC_DataSetReader* reader);
void SOPC_DataSetReader_Set_DataSetWriterId(SOPC_DataSetReader* reader, uint16_t id);
SOPC_SubscribedDataSetType SOPC_DataSetReader_Get_DataSet_TargetType(const SOPC_DataSetReader* reader);
// void SOPC_DataSetReader_Set_DataSet_TargetType(SOPC_DataSetReader* reader, SOPC_SubscribedDataSetType type);

double SOPC_DataSetReader_Get_ReceiveTimeout(const SOPC_DataSetReader* reader);
void SOPC_DataSetReader_Set_ReceiveTimeout(SOPC_DataSetReader* reader, double timeout_ms);

bool SOPC_DataSetReader_Allocate_FieldMetaData_Array(SOPC_DataSetReader* reader,
                                                     SOPC_SubscribedDataSetType type,
                                                     uint16_t nb);
uint16_t SOPC_DataSetReader_Nb_FieldMetaData(const SOPC_DataSetReader* reader);
SOPC_FieldMetaData* SOPC_DataSetReader_Get_FieldMetaData_At(const SOPC_DataSetReader* reader, uint16_t index);

/**********************/
/** SOPC_FieldTarget **/
/**********************/

// return receiver index range. Type is numeric range format
const char* SOPC_FieldTarget_Get_SourceIndexRange(const SOPC_FieldTarget* target);
// copy the given string
bool SOPC_FieldTarget_Set_SourceIndexRange(SOPC_FieldTarget* target, const char* sourceIndexRange);
const SOPC_NodeId* SOPC_FieldTarget_Get_NodeId(const SOPC_FieldTarget* target);
/**
 * Set the NodeId of a field target
 * The field target is now the ownership of the NodeId.
 * Deleting the given field target will delete this NodeId too.
 */
void SOPC_FieldTarget_Set_NodeId(SOPC_FieldTarget* target, SOPC_NodeId* nodeId);

uint32_t SOPC_FieldTarget_Get_AttributeId(const SOPC_FieldTarget* target);
void SOPC_FieldTarget_Set_AttributeId(SOPC_FieldTarget* target, uint32_t attributeId);

// return write index range. Type is numeric range format
const char* SOPC_FieldTarget_Get_TargetIndexRange(const SOPC_FieldTarget* target);
// copy the given string
bool SOPC_FieldTarget_Set_TargetIndexRange(SOPC_FieldTarget* target, const char* sourceIndexRange);

/******************/
/** Writer Group **/
/******************/
typedef struct SOPC_WriterGroup_Options
{
    bool useFixedSizeBuffer;
} SOPC_WriterGroup_Options;

const SOPC_PubSubConnection* SOPC_WriterGroup_Get_Connection(const SOPC_WriterGroup* group);

const SOPC_WriterGroup_Options* SOPC_WriterGroup_Get_Options(const SOPC_WriterGroup* group);
void SOPC_WriterGroup_Set_Options(SOPC_WriterGroup* group, const SOPC_WriterGroup_Options* options);

uint16_t SOPC_WriterGroup_Get_Id(const SOPC_WriterGroup* group);
void SOPC_WriterGroup_Set_Id(SOPC_WriterGroup* group, uint16_t id);

uint32_t SOPC_WriterGroup_Get_Version(const SOPC_WriterGroup* group);
void SOPC_WriterGroup_Set_Version(SOPC_WriterGroup* group, uint32_t version);

/* Expected to be negative for acyclic Publisher */
double SOPC_WriterGroup_Get_PublishingInterval(const SOPC_WriterGroup* group);
void SOPC_WriterGroup_Set_PublishingInterval(SOPC_WriterGroup* group, double interval_ms);

/* Expected to be negative for acyclic Publisher */
int32_t SOPC_WriterGroup_Get_PublishingOffset(const SOPC_WriterGroup* group);
void SOPC_WriterGroup_Set_PublishingOffset(SOPC_WriterGroup* group, int32_t offset_ms);

/* Expected only for acyclic publisher */
double SOPC_WriterGroup_Get_KeepAlive(const SOPC_WriterGroup* group);
void SOPC_WriterGroup_Set_KeepAlive(SOPC_WriterGroup* group, double keepAlive_ms);

SOPC_Pubsub_MessageEncodingType SOPC_WriterGroup_Get_Encoding(const SOPC_WriterGroup* group);
void SOPC_WriterGroup_Set_Encoding(SOPC_WriterGroup* group, SOPC_Pubsub_MessageEncodingType encoding);

SOPC_UadpNetworkMessageContentMask SOPC_WriterGroup_Get_NetworkMessageContentMask(const SOPC_WriterGroup* group);
void SOPC_WriterGroup_Set_NetworkMessageContentMask(SOPC_WriterGroup* group,
                                                    SOPC_UadpNetworkMessageContentMask contentMask);

SOPC_SecurityMode_Type SOPC_WriterGroup_Get_SecurityMode(const SOPC_WriterGroup* group);
void SOPC_WriterGroup_Set_SecurityMode(SOPC_WriterGroup* group, SOPC_SecurityMode_Type mode);
// make a copy of id
bool SOPC_WriterGroup_Set_SecurityGroupId(SOPC_WriterGroup* group, char* securityGroupId);
bool SOPC_WriterGroup_Allocate_SecurityKeyServices_Array(SOPC_WriterGroup* group, uint32_t nb);
uint32_t SOPC_WriterGroup_Nb_SecurityKeyServices(const SOPC_WriterGroup* group);
SOPC_SecurityKeyServices* SOPC_WriterGroup_Get_SecurityKeyServices_At(const SOPC_WriterGroup* group, uint32_t index);

const char* SOPC_WriterGroup_Get_MqttTopic(const SOPC_WriterGroup* writer);

/**
 * Set MqttTopic on the WriterGroup structure
 * If topic parameter is NULL, set mqttTopic to NULL
 * Otherwise, topic parameter is copied internally and may be freed/modified by caller after call.
 */
void SOPC_WriterGroup_Set_MqttTopic(SOPC_WriterGroup* writer, const char* topic);

bool SOPC_WriterGroup_Allocate_DataSetWriter_Array(SOPC_WriterGroup* group, uint8_t nb);
uint8_t SOPC_WriterGroup_Nb_DataSetWriter(const SOPC_WriterGroup* group);
SOPC_DataSetWriter* SOPC_WriterGroup_Get_DataSetWriter_At(const SOPC_WriterGroup* group, uint8_t index);

/*******************/
/** DataSetWriter **/
/*******************/
typedef struct SOPC_DataSetWriter_Options
{
    // Negative logic to get default value with 0-fill initializer (Calloc)
    bool noUseSeqNum;
    bool noTimestamp;
    bool disableEmission;
} SOPC_DataSetWriter_Options;

uint16_t SOPC_DataSetWriter_Get_Id(const SOPC_DataSetWriter* writer);
void SOPC_DataSetWriter_Set_Id(SOPC_DataSetWriter* writer, uint16_t id);
const SOPC_DataSetWriter_Options* SOPC_DataSetWriter_Get_Options(const SOPC_DataSetWriter* writer);
void SOPC_DataSetWriter_Set_Options(SOPC_DataSetWriter* writer, const SOPC_DataSetWriter_Options* options);

const SOPC_PublishedDataSet* SOPC_DataSetWriter_Get_DataSet(const SOPC_DataSetWriter* writer);
void SOPC_DataSetWriter_Set_DataSet(SOPC_DataSetWriter* writer, SOPC_PublishedDataSet* dataset);

/** SOPC_PublishedDataSet **/
bool SOPC_PublishedDataSet_Init(SOPC_PublishedDataSet* dataset,
                                SOPC_PublishedDataSetSourceType type,
                                uint16_t nbFields);
uint16_t SOPC_PublishedDataSet_Nb_FieldMetaData(const SOPC_PublishedDataSet* dataset);
SOPC_FieldMetaData* SOPC_PublishedDataSet_Get_FieldMetaData_At(const SOPC_PublishedDataSet* dataset, uint16_t index);
SOPC_PublishedDataSetSourceType SOPC_PublishedDataSet_Get_DataSet_SourceType(const SOPC_PublishedDataSet* dataset);

/****************************/
/** SOPC_PublishedVariable **/
/****************************/

const SOPC_NodeId* SOPC_PublishedVariable_Get_NodeId(const SOPC_PublishedVariable* variable);
/**
 * Set the NodeId of a PublishedVariable
 * The PublishedVariable is now the ownership of the NodeId.
 * Deleting the given PublishedVariable will delete this NodeId too.
 */
void SOPC_PublishedVariable_Set_NodeId(SOPC_PublishedVariable* variable, SOPC_NodeId* nodeId);

uint32_t SOPC_PublishedVariable_Get_AttributeId(const SOPC_PublishedVariable* variable);
void SOPC_PublishedVariable_Set_AttributeId(SOPC_PublishedVariable* variable, uint32_t attributeId);
// return index range. Type is numeric range format
const char* SOPC_PublishedVariable_Get_IndexRange(const SOPC_PublishedVariable* variable);
// copy the given string
bool SOPC_PublishedVariable_Set_IndexRange(SOPC_PublishedVariable* variable, const char* indexRange);

/************************/
/** SOPC_FieldMetaData **/
/************************/

/* n > 1: the dataType is an array with the specified number of dimensions.
 * OneDimension (1)
 * OneOrMoreDimensions (0)
 * Scalar (−1)
 * Any (−2)
 * ScalarOrOneDimension (−3)
 */
int32_t SOPC_FieldMetaData_Get_ValueRank(const SOPC_FieldMetaData* metadata);
SOPC_BuiltinId SOPC_FieldMetaData_Get_BuiltinType(const SOPC_FieldMetaData* metadata);
void SOPC_FieldMetaData_Set_BuiltinType(SOPC_FieldMetaData* metadata, SOPC_BuiltinId builtinType);
/* ValueRank define dimension of the array ArrayDimensions. ValueRank should be set before allocating ArrayDimensions */
const SOPC_PubSub_ArrayDimension* SOPC_FieldMetaData_Get_ArrayDimension(const SOPC_FieldMetaData* metadata);
void SOPC_FieldMetaData_ArrayDimension_Move(SOPC_FieldMetaData* metadata, SOPC_PubSub_ArrayDimension* arrayDimension);
bool SOPC_FieldMetaDeta_SetCopy_ArrayDimension(SOPC_FieldMetaData* metadata,
                                               const SOPC_PubSub_ArrayDimension* arrayDimensions);

// only for Subscriber
SOPC_FieldTarget* SOPC_FieldMetaData_Get_TargetVariable(const SOPC_FieldMetaData* fieldMetaData);
// only for Publisher
SOPC_PublishedVariable* SOPC_FieldMetaData_Get_PublishedVariable(const SOPC_FieldMetaData* fieldMetaData);

/** SOPC_SecurityKeyServices **/
const char* SOPC_SecurityKeyServices_Get_EndpointUrl(const SOPC_SecurityKeyServices* sks);
/* Data are copied */
bool SOPC_SecurityKeyServices_Set_EndpointUrl(SOPC_SecurityKeyServices* sks, const char* uri);
SOPC_SerializedCertificate* SOPC_SecurityKeyServices_Get_ServerCertificate(const SOPC_SecurityKeyServices* sks);
/* ServerCertificate data ownership moves to SOPC_SecurityKeyServices */
bool SOPC_SecurityKeyServices_Set_ServerCertificate(SOPC_SecurityKeyServices* sks,
                                                    SOPC_SerializedCertificate* serverCertificate);

/* Creates an endpoint description and set the url. Other field values are 0
   returns True if url is NULL or EndpointDescription is created and initialized.
   Otherwise returns false ( out of memory )
*/
bool SOPC_EndpointDescription_Create_From_URL(char* url, OpcUa_EndpointDescription** endpoint_out);

#endif /* SOPC_PUBSUB_CONF_H_ */
