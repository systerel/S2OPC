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

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "sopc_mem_alloc.h"
#include "sopc_pubsub_conf.h"

/**
 * Defines a source of Data Items
 */
typedef struct SOPC_PublishedDataItemsData
{
    uint16_t length;
    SOPC_PublishedVariable* publishedData;
} SOPC_PublishedDataItemsData;

struct SOPC_PublishedVariable
{
    SOPC_NodeId* nodeId;
    uint32_t attributeId;
    char* indexRange;
};

/**
 * Defines a source of Events
 */

struct SOPC_PubSubConfiguration
{
    uint32_t pub_connection_length;
    SOPC_PubSubConnection* pub_connection;

    uint32_t sub_connection_length;
    SOPC_PubSubConnection* sub_connection;

    uint32_t publishedDataSet_length;
    SOPC_PublishedDataSet* publishedDataSet;
};

struct SOPC_PubSubConnection
{
    SOPC_PubSubConnection_Type type;
    char* name;
    bool enabled;
    SOPC_Conf_PublisherId publisherId;
    // Give Network and Transport Protocol as URI
    char* transportProfileUri;
    // Network interface name (Ethernet only): mandatory for Publisher
    char* interfaceName;
    // NetworkAddressDataType is string
    char* address;
    uint16_t writerGroups_length;
    SOPC_WriterGroup* writerGroups;
    uint16_t readerGroups_length;
    SOPC_ReaderGroup* readerGroups;

    // For the next version:
    // uint32_t connectionPropertiesLength: not used;
    // KeyValuePair *connectionProperties: not used;
    // ConnectionTransportDataType transportSettings: not used;
};

struct SOPC_DataSetMetaData
{
    // char* name;
    uint16_t fields_length;
    SOPC_FieldMetaData* fields;

    // configurationVersion
};

struct SOPC_PublishedDataSet
{
    SOPC_DataSetMetaData dataSetMetaData;
    // SOPC_PublishedDataSetSource dataSetSource;
    SOPC_PublishedDataSetSourceType dataSetSourceType;

    // For the next version:
    // char **dataSetFolder
    // KeyValuePair *extensionFields
};

typedef struct SOPC_UadpWriterGroup
{
    uint32_t version;
    SOPC_UadpNetworkMessageContentMask contentMask;

    // For the next version:
    // dataSetOrdering
    // samplingOffset
    // publishingOffset

} SOPC_UadpWriterGroup;

struct SOPC_WriterGroup
{
    SOPC_PubSubConnection* parent;
    uint16_t id;
    double publishingIntervalMs;
    int32_t publishingOffsetMs;
    SOPC_UadpWriterGroup messageSettings;

    uint8_t dataSetWriters_length;
    SOPC_DataSetWriter* dataSetWriters;

    SOPC_SecurityMode_Type securityMode;

    // For the next version:
    // KeepAliveTime
    // Priority
    // LocaleIds
    // transportSettings
};

struct SOPC_DataSetWriter
{
    uint16_t id;
    // dataSetName in structure defined in Spec 1.4
    SOPC_PublishedDataSet*
        dataSet; /* IMPORTANT NOTE: memory management of field not managed here (see global config struct)*/
    // For the next version:
    // name
    // enabled
    // dataSetFieldContentMask
    // keyFrameCount
    // dataSetWriterProperties
    // transportSettings
    // messageSettings
};

struct SOPC_ReaderGroup
{
    SOPC_SecurityMode_Type securityMode;
    SOPC_Conf_PublisherId publisherId;

    uint8_t dataSetReaders_length;
    SOPC_DataSetReader* dataSetReaders;
    uint16_t groupId;
    uint32_t groupVersion;
    bool hasNonZeroWriterIds;
    bool hasZeroWriterIds;
};

struct SOPC_DataSetReader
{
    struct SOPC_ReaderGroup* group;
    uint16_t dataSetWriterId;
    SOPC_DataSetMetaData metaData;
    SOPC_SubscribedDataSetType targetType;
    double messageReceiveTimeout; /* ms */

    // These fields below are defined in Spec but not used
    // DataSetFieldContentMask

    // SecurityMode
    // SecurityGroupId
    // SecurityKeyServices
    // DataSetReaderProperties
    // transportSettings
    // messageSettings
};

struct SOPC_FieldTarget
{
    // numeric range format
    char* receiverIndexRange;
    SOPC_NodeId* nodeId;
    uint32_t attributeId;
    // numeric range format
    char* writeIndexRange;

    // override value is not managed
    // overrideValueHandling
    // overrideValue
};

struct SOPC_FieldMetaData
{
    // char* name;
    SOPC_BuiltinId builtinType;
    int32_t valueRank;

    // other field are not used
    // only one element. used for subscriber
    SOPC_FieldTarget* targetVariable;
    // only one element. used for publisher
    SOPC_PublishedVariable* publishedVariable;
};

static bool SOPC_PubSubConfiguration_Allocate_PubSubConnection_Array(SOPC_PubSubConfiguration* configuration,
                                                                     uint32_t nb,
                                                                     SOPC_PubSubConnection** arrayPtr,
                                                                     uint32_t* lengthPtr,
                                                                     SOPC_PubSubConnection_Type type);

static char* SOPC_PubSub_String_Copy(const char* src);

static void SOPC_PubSubConnection_Clear(SOPC_PubSubConnection* connection);

static void SOPC_PublishedDataSet_Clear(SOPC_PublishedDataSet* dataset);

static void SOPC_WriterGroup_Clear(SOPC_WriterGroup* group);

static void SOPC_ReaderGroup_Clear(SOPC_ReaderGroup* group);

static void SOPC_DataSetMetaData_Clear(SOPC_DataSetMetaData* metaData);

static void SOPC_FieldTarget_Clear(SOPC_FieldTarget* target);

static void SOPC_PublishedVariable_Clear(SOPC_PublishedVariable* variable);

static void SOPC_Conf_PublisherId_Clear(SOPC_Conf_PublisherId* pubId);

SOPC_PubSubConfiguration* SOPC_PubSubConfiguration_Create(void)
{
    // free in SOPC_PubSubConfiguration_Delete
    SOPC_PubSubConfiguration* configuration = SOPC_Calloc(1, sizeof(SOPC_PubSubConfiguration));
    if (NULL == configuration)
    {
        return NULL;
    }
    return configuration;
}

void SOPC_PubSubConfiguration_Delete(SOPC_PubSubConfiguration* configuration)
{
    if (NULL == configuration)
    {
        return;
    }
    for (uint32_t i = 0; i < configuration->pub_connection_length; i++)
    {
        SOPC_PubSubConnection_Clear(&(configuration->pub_connection[i]));
    }
    SOPC_Free(configuration->pub_connection);
    for (uint32_t i = 0; i < configuration->sub_connection_length; i++)
    {
        SOPC_PubSubConnection_Clear(&(configuration->sub_connection[i]));
    }
    SOPC_Free(configuration->sub_connection);
    for (uint32_t i = 0; i < configuration->publishedDataSet_length; i++)
    {
        SOPC_PublishedDataSet_Clear(&(configuration->publishedDataSet[i]));
    }
    SOPC_Free(configuration->publishedDataSet);
    SOPC_Free(configuration);
}

static bool SOPC_PubSubConfiguration_Allocate_PubSubConnection_Array(SOPC_PubSubConfiguration* configuration,
                                                                     uint32_t nb,
                                                                     SOPC_PubSubConnection** arrayPtr,
                                                                     uint32_t* lengthPtr,
                                                                     SOPC_PubSubConnection_Type type)
{
    assert(NULL != configuration && NULL != arrayPtr && NULL != lengthPtr);
    // free in SOPC_PubSubConfiguration_Delete
    *arrayPtr = SOPC_Calloc(nb, sizeof(SOPC_PubSubConnection));
    if (NULL == *arrayPtr)
    {
        return false;
    }
    *lengthPtr = nb;
    for (uint32_t i = 0; i < nb; i++)
    {
        (*arrayPtr)[i].type = type;
        (*arrayPtr)[i].publisherId.type = SOPC_Null_PublisherId;
    }

    return true;
}

bool SOPC_PubSubConfiguration_Allocate_PubConnection_Array(SOPC_PubSubConfiguration* configuration, uint32_t nb)
{
    return SOPC_PubSubConfiguration_Allocate_PubSubConnection_Array(configuration, nb, &(configuration->pub_connection),
                                                                    &(configuration->pub_connection_length),
                                                                    SOPC_PubSubConnection_Pub);
}

uint32_t SOPC_PubSubConfiguration_Nb_PubConnection(const SOPC_PubSubConfiguration* configuration)
{
    assert(NULL != configuration);
    return configuration->pub_connection_length;
}

SOPC_PubSubConnection* SOPC_PubSubConfiguration_Get_PubConnection_At(const SOPC_PubSubConfiguration* configuration,
                                                                     uint32_t index)
{
    assert(NULL != configuration && index < configuration->pub_connection_length);
    return &(configuration->pub_connection[index]);
}

bool SOPC_PubSubConfiguration_Allocate_SubConnection_Array(SOPC_PubSubConfiguration* configuration, uint32_t nb)
{
    return SOPC_PubSubConfiguration_Allocate_PubSubConnection_Array(configuration, nb, &(configuration->sub_connection),
                                                                    &(configuration->sub_connection_length),
                                                                    SOPC_PubSubConnection_Sub);
}

uint32_t SOPC_PubSubConfiguration_Nb_SubConnection(const SOPC_PubSubConfiguration* configuration)
{
    assert(NULL != configuration);
    return configuration->sub_connection_length;
}

SOPC_PubSubConnection* SOPC_PubSubConfiguration_Get_SubConnection_At(const SOPC_PubSubConfiguration* configuration,
                                                                     uint32_t index)
{
    assert(NULL != configuration && index < configuration->sub_connection_length);
    return &(configuration->sub_connection[index]);
}

bool SOPC_PubSubConfiguration_Allocate_PublishedDataSet_Array(SOPC_PubSubConfiguration* configuration, uint32_t nb)
{
    assert(NULL != configuration);
    // free in SOPC_PubSubConfiguration_Delete
    configuration->publishedDataSet = SOPC_Calloc(nb, sizeof(SOPC_PublishedDataSet));
    if (NULL == configuration->publishedDataSet)
    {
        return false;
    }
    configuration->publishedDataSet_length = nb;
    return true;
}

uint32_t SOPC_PubSubConfiguration_Nb_PublishedDataSet(const SOPC_PubSubConfiguration* configuration)
{
    assert(NULL != configuration);
    return configuration->publishedDataSet_length;
}

SOPC_PublishedDataSet* SOPC_PubSubConfiguration_Get_PublishedDataSet_At(const SOPC_PubSubConfiguration* configuration,
                                                                        uint16_t index)
{
    assert(NULL != configuration && index < configuration->publishedDataSet_length);
    return &(configuration->publishedDataSet[index]);
}

static void SOPC_PubSubConnection_Clear(SOPC_PubSubConnection* connection)
{
    if (NULL != connection)
    {
        SOPC_Free(connection->name);
        SOPC_Conf_PublisherId_Clear(&connection->publisherId);
        SOPC_Free(connection->address);
        SOPC_Free(connection->interfaceName);
        SOPC_Free(connection->transportProfileUri);
        for (int i = 0; i < connection->writerGroups_length; i++)
        {
            SOPC_WriterGroup_Clear(&(connection->writerGroups[i]));
        }
        SOPC_Free(connection->writerGroups);
        for (int i = 0; i < connection->readerGroups_length; i++)
        {
            SOPC_ReaderGroup_Clear(&(connection->readerGroups[i]));
        }
        SOPC_Free(connection->readerGroups);
    }
}

const char* SOPC_PubSubConnection_Get_Name(const SOPC_PubSubConnection* connection)
{
    return connection->name;
}

bool SOPC_PubSubConnection_Set_Name(SOPC_PubSubConnection* connection, const char* name)
{
    assert(NULL != connection);
    // free in SOPC_PubSubConnection_Clear
    connection->name = SOPC_PubSub_String_Copy(name);
    return (NULL != connection->name);
}

bool SOPC_PubSubConnection_Is_Enabled(const SOPC_PubSubConnection* connection)
{
    assert(NULL != connection);
    return connection->enabled;
}

void SOPC_PubSubConnection_Set_Enabled(SOPC_PubSubConnection* connection, bool enabled)
{
    assert(NULL != connection);
    connection->enabled = enabled;
}

// PublisherId

static void SOPC_Conf_PublisherId_Clear(SOPC_Conf_PublisherId* pubId)
{
    if (NULL != pubId && SOPC_String_PublisherId == pubId->type)
    {
        SOPC_String_Clear(&pubId->data.string);
    }
}

const SOPC_Conf_PublisherId* SOPC_PubSubConnection_Get_PublisherId(const SOPC_PubSubConnection* connection)
{
    assert(NULL != connection);
    return &(connection->publisherId);
}

void SOPC_PubSubConnection_Set_PublisherId_UInteger(SOPC_PubSubConnection* connection, uint64_t id)
{
    assert(NULL != connection && SOPC_PubSubConnection_Pub == connection->type);
    connection->publisherId.type = SOPC_UInteger_PublisherId;
    connection->publisherId.data.uint = id;
}

bool SOPC_PubSubConnection_Set_PublisherId_String(SOPC_PubSubConnection* connection, const char* id)
{
    assert(NULL != connection && NULL != id && SOPC_PubSubConnection_Pub == connection->type);
    connection->publisherId.type = SOPC_String_PublisherId;
    SOPC_ReturnStatus status = SOPC_String_CopyFromCString(&connection->publisherId.data.string, id);
    return (SOPC_STATUS_OK == status);
}

const char* SOPC_PubSubConnection_Get_TransportProfileUri(const SOPC_PubSubConnection* connection)
{
    assert(NULL != connection);
    return connection->transportProfileUri;
}

bool SOPC_PubSubConnection_Set_TransportProfileUri(SOPC_PubSubConnection* connection, const char* uri)
{
    assert(NULL != connection);
    // free in SOPC_PubSubConnection_Clear
    connection->transportProfileUri = SOPC_PubSub_String_Copy(uri);
    return (NULL != connection->transportProfileUri);
}

const char* SOPC_PubSubConnection_Get_Address(const SOPC_PubSubConnection* connection)
{
    assert(NULL != connection);
    return connection->address;
}

bool SOPC_PubSubConnection_Set_Address(SOPC_PubSubConnection* connection, const char* address)
{
    assert(NULL != connection);
    // free in SOPC_PubSubConnection_Clear
    connection->address = SOPC_PubSub_String_Copy(address);
    return (NULL != connection->address);
}

const char* SOPC_PubSubConnection_Get_InterfaceName(const SOPC_PubSubConnection* connection)
{
    assert(NULL != connection);
    return connection->interfaceName;
}

bool SOPC_PubSubConnection_Set_InterfaceName(SOPC_PubSubConnection* connection, const char* interfaceName)
{
    assert(NULL != connection);
    // free in SOPC_PubSubConnection_Clear
    connection->interfaceName = SOPC_PubSub_String_Copy(interfaceName);
    return (NULL != connection->interfaceName);
}

bool SOPC_PubSubConnection_Allocate_WriterGroup_Array(SOPC_PubSubConnection* connection, uint16_t nb)
{
    assert(NULL != connection && SOPC_PubSubConnection_Pub == connection->type);
    // free in SOPC_PubSubConnection_Clear
    connection->writerGroups = SOPC_Calloc(nb, sizeof(SOPC_WriterGroup));
    if (NULL == connection->writerGroups)
    {
        return false;
    }
    connection->writerGroups_length = nb;
    for (int i = 0; i < nb; i++)
    {
        connection->writerGroups[i].parent = connection;
        // As publishingOffsetMs is optional, it shall be initialized here
        connection->writerGroups[i].publishingOffsetMs = -1;
    }
    return true;
}

uint16_t SOPC_PubSubConnection_Nb_WriterGroup(const SOPC_PubSubConnection* connection)
{
    assert(NULL != connection);
    return connection->writerGroups_length;
}

SOPC_WriterGroup* SOPC_PubSubConnection_Get_WriterGroup_At(const SOPC_PubSubConnection* connection, uint16_t index)
{
    assert(NULL != connection && index < connection->writerGroups_length);
    return &connection->writerGroups[index];
}

bool SOPC_PubSubConnection_Allocate_ReaderGroup_Array(SOPC_PubSubConnection* connection, uint16_t nb)
{
    assert(NULL != connection && SOPC_PubSubConnection_Sub == connection->type);
    // free in SOPC_PubSubConnection_Clear
    connection->readerGroups = SOPC_Calloc(nb, sizeof(SOPC_ReaderGroup));
    if (NULL == connection->readerGroups)
    {
        return false;
    }
    connection->readerGroups_length = nb;
    for (int i = 0; i < nb; i++)
    {
        memset(&connection->readerGroups[i], 0, sizeof(SOPC_ReaderGroup));
        connection->readerGroups[i].publisherId.type = SOPC_Null_PublisherId;
    }
    return true;
}

uint16_t SOPC_PubSubConnection_Nb_ReaderGroup(const SOPC_PubSubConnection* connection)
{
    assert(NULL != connection);
    return connection->readerGroups_length;
}

SOPC_ReaderGroup* SOPC_PubSubConnection_Get_ReaderGroup_At(const SOPC_PubSubConnection* connection, uint16_t index)
{
    assert(NULL != connection && index < connection->readerGroups_length);
    return &connection->readerGroups[index];
}

/** Reader Group **/

SOPC_SecurityMode_Type SOPC_ReaderGroup_Get_SecurityMode(const SOPC_ReaderGroup* group)
{
    assert(NULL != group);
    return group->securityMode;
}

void SOPC_ReaderGroup_Set_SecurityMode(SOPC_ReaderGroup* group, SOPC_SecurityMode_Type mode)
{
    assert(NULL != group);
    group->securityMode = mode;
}

bool SOPC_ReaderGroup_Allocate_DataSetReader_Array(SOPC_ReaderGroup* group, uint8_t nb)
{
    assert(NULL != group);
    // free in SOPC_ReaderGroup_Clear
    group->dataSetReaders = SOPC_Calloc(nb, sizeof(SOPC_DataSetReader));
    if (NULL == group->dataSetReaders)
    {
        return false;
    }
    for (size_t i = 0; i < nb; i++)
    {
        group->dataSetReaders[i].group = group;
    }
    group->dataSetReaders_length = nb;
    return true;
}

uint8_t SOPC_ReaderGroup_Nb_DataSetReader(const SOPC_ReaderGroup* group)
{
    assert(NULL != group);
    return group->dataSetReaders_length;
}

SOPC_DataSetReader* SOPC_ReaderGroup_Get_DataSetReader_At(const SOPC_ReaderGroup* group, uint8_t index)
{
    assert(NULL != group && index < group->dataSetReaders_length);
    return &group->dataSetReaders[index];
}

uint16_t SOPC_ReaderGroup_Get_GroupId(const SOPC_ReaderGroup* group)
{
    assert(NULL != group);
    return group->groupId;
}

void SOPC_ReaderGroup_Set_GroupId(SOPC_ReaderGroup* group, uint16_t id)
{
    assert(NULL != group);
    group->groupId = id;
}

uint32_t SOPC_ReaderGroup_Get_GroupVersion(const SOPC_ReaderGroup* group)
{
    assert(NULL != group);
    return group->groupVersion;
}

void SOPC_ReaderGroup_Set_GroupVersion(SOPC_ReaderGroup* group, uint32_t version)
{
    assert(NULL != group);
    group->groupVersion = version;
}

const SOPC_Conf_PublisherId* SOPC_ReaderGroup_Get_PublisherId(SOPC_ReaderGroup* group)
{
    assert(NULL != group);
    return &(group->publisherId);
}

void SOPC_ReaderGroup_Set_PublisherId_UInteger(SOPC_ReaderGroup* group, uint64_t id)
{
    assert(NULL != group);
    group->publisherId.type = SOPC_UInteger_PublisherId;
    group->publisherId.data.uint = id;
}

bool SOPC_ReaderGroup_Set_PublisherId_String(SOPC_ReaderGroup* group, const char* id)
{
    assert(NULL != group && NULL != id);
    group->publisherId.type = SOPC_String_PublisherId;
    SOPC_ReturnStatus status = SOPC_String_CopyFromCString(&group->publisherId.data.string, id);
    return (SOPC_STATUS_OK == status);
}

bool SOPC_ReaderGroup_HasNonZeroDataSetWriterId(const SOPC_ReaderGroup* group)
{
    assert(NULL != group);
    return group->hasNonZeroWriterIds;
}

/** DataSetReader **/

static void SOPC_DataSetMetaData_Clear(SOPC_DataSetMetaData* metaData)
{
    for (int i = 0; i < metaData->fields_length; i++)
    {
        SOPC_FieldTarget_Clear(metaData->fields[i].targetVariable);
        SOPC_Free(metaData->fields[i].targetVariable);
        SOPC_PublishedVariable_Clear(metaData->fields[i].publishedVariable);
        SOPC_Free(metaData->fields[i].publishedVariable);
    }
    metaData->fields_length = 0;
    SOPC_Free(metaData->fields);
}

SOPC_ReaderGroup* SOPC_DataSetReader_Get_ReaderGroup(const SOPC_DataSetReader* reader)
{
    assert(NULL != reader);
    return reader->group;
}

uint16_t SOPC_DataSetReader_Get_DataSetWriterId(const SOPC_DataSetReader* reader)
{
    assert(NULL != reader);
    return reader->dataSetWriterId;
}

void SOPC_DataSetReader_Set_DataSetWriterId(SOPC_DataSetReader* reader, uint16_t id)
{
    assert(NULL != reader);
    if (id != 0)
    {
        reader->group->hasNonZeroWriterIds = true;
    }
    else
    {
        reader->group->hasZeroWriterIds = true;
    }
    assert(!(reader->group->hasNonZeroWriterIds && reader->group->hasZeroWriterIds));

    reader->dataSetWriterId = id;
}

bool SOPC_DataSetReader_Allocate_FieldMetaData_Array(SOPC_DataSetReader* reader,
                                                     SOPC_SubscribedDataSetType type,
                                                     uint16_t nb)
{
    assert(NULL != reader);
    reader->targetType = type;
    // free in SOPC_DataSetMetaData_Clear
    reader->metaData.fields = SOPC_Calloc(nb, sizeof(SOPC_FieldMetaData));
    if (NULL == reader->metaData.fields)
    {
        return false;
    }
    reader->metaData.fields_length = nb;
    for (int i = 0; i < nb; i++)
    {
        // free in SOPC_DataSetMetaData_Clear
        SOPC_FieldTarget* target = SOPC_Calloc(1, sizeof(SOPC_FieldTarget));
        if (NULL == target)
        {
            SOPC_DataSetMetaData_Clear(&(reader->metaData));
            return false;
        }
        reader->metaData.fields[i].targetVariable = target;
    }
    return true;
}

uint16_t SOPC_DataSetReader_Nb_FieldMetaData(const SOPC_DataSetReader* reader)
{
    assert(NULL != reader);
    return reader->metaData.fields_length;
}

SOPC_FieldMetaData* SOPC_DataSetReader_Get_FieldMetaData_At(const SOPC_DataSetReader* reader, uint16_t index)
{
    assert(NULL != reader);
    assert(index < reader->metaData.fields_length);
    return &(reader->metaData.fields[index]);
}

SOPC_SubscribedDataSetType SOPC_DataSetReader_Get_DataSet_TargetType(const SOPC_DataSetReader* reader)
{
    assert(NULL != reader);
    return reader->targetType;
}

double SOPC_DataSetReader_Get_ReceiveTimeout(const SOPC_DataSetReader* reader)
{
    assert(NULL != reader);
    return reader->messageReceiveTimeout;
}

void SOPC_DataSetReader_Set_ReceiveTimeout(SOPC_DataSetReader* reader, double timeout_ms)
{
    assert(NULL != reader);
    reader->messageReceiveTimeout = timeout_ms;
}

/** SOPC_FieldTarget **/

const char* SOPC_FieldTarget_Get_SourceIndexRange(const SOPC_FieldTarget* target)
{
    return target->receiverIndexRange;
}

bool SOPC_FieldTarget_Set_SourceIndexRange(SOPC_FieldTarget* target, const char* sourceIndexRange)
{
    assert(NULL != target && NULL != sourceIndexRange);
    // free in SOPC_FieldTarget_Clear
    target->receiverIndexRange = SOPC_PubSub_String_Copy(sourceIndexRange);
    return (NULL != target->receiverIndexRange);
}

const SOPC_NodeId* SOPC_FieldTarget_Get_NodeId(const SOPC_FieldTarget* target)
{
    assert(NULL != target);
    return target->nodeId;
}

void SOPC_FieldTarget_Set_NodeId(SOPC_FieldTarget* target, SOPC_NodeId* nodeId)
{
    assert(NULL != target && NULL != nodeId);
    // free in SOPC_FieldTarget_Clear
    target->nodeId = nodeId;
}

uint32_t SOPC_FieldTarget_Get_AttributeId(const SOPC_FieldTarget* target)
{
    assert(NULL != target);
    return target->attributeId;
}

void SOPC_FieldTarget_Set_AttributeId(SOPC_FieldTarget* target, uint32_t attributeId)
{
    assert(NULL != target);
    target->attributeId = attributeId;
}

const char* SOPC_FieldTarget_Get_TargetIndexRange(const SOPC_FieldTarget* target)
{
    return target->writeIndexRange;
}

bool SOPC_FieldTarget_Set_TargetIndexRange(SOPC_FieldTarget* target, const char* targetIndexRange)
{
    assert(NULL != target && NULL != targetIndexRange);
    // free in SOPC_FieldTarget_Clear
    target->writeIndexRange = SOPC_PubSub_String_Copy(targetIndexRange);
    return (NULL != target->writeIndexRange);
}

/** Writer Group **/
const SOPC_PubSubConnection* SOPC_WriterGroup_Get_Connection(const SOPC_WriterGroup* group)
{
    assert(NULL != group);
    return group->parent;
}

uint16_t SOPC_WriterGroup_Get_Id(const SOPC_WriterGroup* group)
{
    assert(NULL != group);
    return group->id;
}

void SOPC_WriterGroup_Set_Id(SOPC_WriterGroup* group, uint16_t id)
{
    assert(NULL != group);
    group->id = id;
}

uint32_t SOPC_WriterGroup_Get_Version(const SOPC_WriterGroup* group)
{
    assert(NULL != group);
    return group->messageSettings.version;
}

void SOPC_WriterGroup_Set_Version(SOPC_WriterGroup* group, uint32_t version)
{
    assert(NULL != group);
    group->messageSettings.version = version;
}

double SOPC_WriterGroup_Get_PublishingInterval(const SOPC_WriterGroup* group)
{
    assert(NULL != group);
    return group->publishingIntervalMs;
}

int32_t SOPC_WriterGroup_Get_PublishingOffset(const SOPC_WriterGroup* group)
{
    assert(NULL != group);
    return group->publishingOffsetMs;
}

void SOPC_WriterGroup_Set_PublishingInterval(SOPC_WriterGroup* group, double interval_ms)
{
    assert(NULL != group);
    group->publishingIntervalMs = interval_ms;
}

void SOPC_WriterGroup_Set_PublishingOffset(SOPC_WriterGroup* group, int32_t offset_ms)
{
    assert(NULL != group);
    group->publishingOffsetMs = offset_ms;
}

SOPC_UadpNetworkMessageContentMask SOPC_WriterGroup_Get_NetworkMessageContentMask(const SOPC_WriterGroup* group)
{
    assert(NULL != group);
    return group->messageSettings.contentMask;
}

void SOPC_WriterGroup_Set_NetworkMessageContentMask(SOPC_WriterGroup* group,
                                                    SOPC_UadpNetworkMessageContentMask contentMask)
{
    assert(NULL != group);
    group->messageSettings.contentMask = contentMask;
}

SOPC_SecurityMode_Type SOPC_WriterGroup_Get_SecurityMode(const SOPC_WriterGroup* group)
{
    assert(NULL != group);
    return group->securityMode;
}

void SOPC_WriterGroup_Set_SecurityMode(SOPC_WriterGroup* group, SOPC_SecurityMode_Type mode)
{
    assert(NULL != group);
    group->securityMode = mode;
}

bool SOPC_WriterGroup_Allocate_DataSetWriter_Array(SOPC_WriterGroup* group, uint8_t nb)
{
    assert(NULL != group);
    // free in SOPC_WriterGroup_Clear
    group->dataSetWriters = SOPC_Calloc(nb, sizeof(SOPC_DataSetWriter));
    if (NULL == group->dataSetWriters)
    {
        return false;
    }
    group->dataSetWriters_length = nb;
    return true;
}

uint8_t SOPC_WriterGroup_Nb_DataSetWriter(const SOPC_WriterGroup* group)
{
    assert(NULL != group);
    return group->dataSetWriters_length;
}

SOPC_DataSetWriter* SOPC_WriterGroup_Get_DataSetWriter_At(const SOPC_WriterGroup* group, uint8_t index)
{
    assert(NULL != group && index < group->dataSetWriters_length);
    return &group->dataSetWriters[index];
}

/** DataSetWriter **/
uint16_t SOPC_DataSetWriter_Get_Id(const SOPC_DataSetWriter* writer)
{
    assert(NULL != writer);
    return writer->id;
}

void SOPC_DataSetWriter_Set_Id(SOPC_DataSetWriter* writer, uint16_t id)
{
    assert(NULL != writer);
    writer->id = id;
}

void SOPC_DataSetWriter_Set_DataSet(SOPC_DataSetWriter* writer, SOPC_PublishedDataSet* dataset)
{
    assert(NULL != writer);
    writer->dataSet = dataset;
}

const SOPC_PublishedDataSet* SOPC_DataSetWriter_Get_DataSet(const SOPC_DataSetWriter* writer)
{
    assert(NULL != writer && NULL != writer->dataSet);
    return writer->dataSet;
}

/* SOPC_FieldMetaData */

SOPC_FieldTarget* SOPC_FieldMetaData_Get_TargetVariable(const SOPC_FieldMetaData* fieldMetaData)
{
    assert(NULL != fieldMetaData);
    return fieldMetaData->targetVariable;
}

SOPC_PublishedVariable* SOPC_FieldMetaData_Get_PublishedVariable(const SOPC_FieldMetaData* fieldMetaData)
{
    assert(NULL != fieldMetaData);
    return fieldMetaData->publishedVariable;
}

int32_t SOPC_FieldMetaData_Get_ValueRank(const SOPC_FieldMetaData* metadata)
{
    assert(NULL != metadata);
    return metadata->valueRank;
}

void SOPC_FieldMetaData_Set_ValueRank(SOPC_FieldMetaData* metadata, int32_t valueRank)
{
    assert(NULL != metadata);
    metadata->valueRank = valueRank;
}

SOPC_BuiltinId SOPC_FieldMetaData_Get_BuiltinType(const SOPC_FieldMetaData* metadata)
{
    assert(NULL != metadata);
    return metadata->builtinType;
}

void SOPC_FieldMetaData_Set_BuiltinType(SOPC_FieldMetaData* metadata, SOPC_BuiltinId builtinType)
{
    assert(NULL != metadata);
    metadata->builtinType = builtinType;
}

/* SOPC_PublishedDataSet */

bool SOPC_PublishedDataSet_Init(SOPC_PublishedDataSet* dataset, SOPC_PublishedDataSetSourceType type, uint16_t nbFields)
{
    assert(NULL != dataset);
    assert(SOPC_PublishedDataItemsDataType == type);
    dataset->dataSetSourceType = type;
    // free in SOPC_DataSetMetaData_Clear
    dataset->dataSetMetaData.fields = SOPC_Calloc(nbFields, sizeof(SOPC_FieldMetaData));
    if (NULL == dataset->dataSetMetaData.fields)
    {
        return false;
    }
    dataset->dataSetMetaData.fields_length = nbFields;
    for (int i = 0; i < nbFields; i++)
    {
        // free in SOPC_DataSetMetaData_Clear
        SOPC_PublishedVariable* variable = SOPC_Calloc(1, sizeof(SOPC_PublishedVariable));
        if (NULL == variable)
        {
            SOPC_DataSetMetaData_Clear(&(dataset->dataSetMetaData));
            return false;
        }
        dataset->dataSetMetaData.fields[i].publishedVariable = variable;
    }
    return true;

    ////dataset->dataSetSource.type = type;
    // dataset->dataSetSource.source.dataItems.length = nbFields;
    // dataset->dataSetSource.source.dataItems.publishedData = SOPC_Calloc(nbFields, sizeof(SOPC_PublishedVariable));
}

static void SOPC_PublishedDataSet_Clear(SOPC_PublishedDataSet* dataset)
{
    if (NULL == dataset)
    {
        return;
    }
    assert(SOPC_PublishedDataItemsDataType == dataset->dataSetSourceType);

    SOPC_DataSetMetaData_Clear(&(dataset->dataSetMetaData));
    // SOPC_Free(dataset->dataSetSource.source.dataItems.publishedData);
}

uint16_t SOPC_PublishedDataSet_Nb_FieldMetaData(const SOPC_PublishedDataSet* dataset)
{
    assert(NULL != dataset);
    return dataset->dataSetMetaData.fields_length;
}

SOPC_FieldMetaData* SOPC_PublishedDataSet_Get_FieldMetaData_At(const SOPC_PublishedDataSet* dataset, uint16_t index)
{
    assert(NULL != dataset);
    assert(index < dataset->dataSetMetaData.fields_length);
    return &(dataset->dataSetMetaData.fields[index]);
}

SOPC_PublishedDataSetSourceType SOPC_PublishedDataSet_Get_DataSet_SourceType(const SOPC_PublishedDataSet* dataset)
{
    assert(NULL != dataset);
    return dataset->dataSetSourceType;
}

/** SOPC_PublishedVariable **/

const SOPC_NodeId* SOPC_PublishedVariable_Get_NodeId(const SOPC_PublishedVariable* variable)
{
    assert(NULL != variable);
    return variable->nodeId;
}

void SOPC_PublishedVariable_Set_NodeId(SOPC_PublishedVariable* variable, SOPC_NodeId* nodeId)
{
    assert(NULL != variable && NULL != nodeId);
    // free in SOPC_PublishedVariable_Clear
    variable->nodeId = nodeId;
}

uint32_t SOPC_PublishedVariable_Get_AttributeId(const SOPC_PublishedVariable* variable)
{
    assert(NULL != variable);
    return variable->attributeId;
}

void SOPC_PublishedVariable_Set_AttributeId(SOPC_PublishedVariable* variable, uint32_t attributeId)
{
    assert(NULL != variable);
    variable->attributeId = attributeId;
}

const char* SOPC_PublishedVariable_Get_IndexRange(const SOPC_PublishedVariable* variable)
{
    return variable->indexRange;
}

bool SOPC_PublishedVariable_Set_IndexRange(SOPC_PublishedVariable* variable, const char* indexRange)
{
    assert(NULL != variable && NULL != indexRange);
    // free in SOPC_FieldTarget_Clear
    variable->indexRange = SOPC_PubSub_String_Copy(indexRange);
    return (NULL != variable->indexRange);
}

/* Private */

static char* SOPC_PubSub_String_Copy(const char* src)
{
    assert(NULL != src);
    char* dst = SOPC_Malloc((strlen(src) + 1) * sizeof(char));
    if (NULL != dst)
    {
        strcpy(dst, src);
    }
    return dst;
}

static void SOPC_WriterGroup_Clear(SOPC_WriterGroup* group)
{
    if (NULL != group)
    {
        group->dataSetWriters_length = 0;
        /* IMPORTANT NOTE: No memory management to do in dataSetWriter(s), publishedDataSet managed in config struct
         * => no need to iterate on array */
        SOPC_Free(group->dataSetWriters);
    }
}

static void SOPC_ReaderGroup_Clear(SOPC_ReaderGroup* group)
{
    if (NULL != group)
    {
        for (int i = 0; i < group->dataSetReaders_length; i++)
        {
            SOPC_DataSetMetaData_Clear(&(group->dataSetReaders[i].metaData));
        }
        group->dataSetReaders_length = 0;
        SOPC_Free(group->dataSetReaders);
    }
}

static void SOPC_FieldTarget_Clear(SOPC_FieldTarget* target)
{
    if (NULL != target)
    {
        SOPC_Free(target->receiverIndexRange);
        SOPC_NodeId_Clear(target->nodeId);
        SOPC_Free(target->nodeId);
        SOPC_Free(target->writeIndexRange);
    }
}

static void SOPC_PublishedVariable_Clear(SOPC_PublishedVariable* variable)
{
    if (NULL != variable)
    {
        SOPC_NodeId_Clear(variable->nodeId);
        SOPC_Free(variable->nodeId);
    }
}
