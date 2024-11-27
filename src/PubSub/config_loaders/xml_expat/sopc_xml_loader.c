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

#include "sopc_xml_loader.h"

#include <errno.h>
#include <float.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "expat.h"

#include "sopc_assert.h"
#include "sopc_builtintypes.h"
#include "sopc_helper_string.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"

/**
 * \brief
 *  This module provides an XML parsing for PubSub configurations.
 *  The input XML file is supposed to be previously checked towards 's2opc_pubsub_config.xsd'
 *    however, the code is robust to invalid XML or non-validatings schemas.
 *  It is possible to debug file parsing by defining the flag ::XML_CONFIG_LOADER_LOG
 */

// Default behavior is to show WML parsing errors
// This can be disabled by setting XML_CONFIG_LOADER_LOG to 0 in build flags
#ifndef XML_CONFIG_LOADER_LOG
#define XML_CONFIG_LOADER_LOG 1
#endif

#if XML_CONFIG_LOADER_LOG
#define LOG(str) fprintf(stderr, "XML_CONFIG_LOADER: %s:%d: %s\n", __FILE__, __LINE__, (str))
#define LOG_XML_ERROR(str)                                                                         \
    fprintf(stderr, "XML_CONFIG_LOADER: %s:%d: at line %lu, column %lu: %s\n", __FILE__, __LINE__, \
            XML_GetCurrentLineNumber(ctx->parser), XML_GetCurrentColumnNumber(ctx->parser), (str))

#define LOGF(format, ...) fprintf(stderr, "XML_CONFIG_LOADER: %s:%d: " format "\n", __FILE__, __LINE__, __VA_ARGS__)
#define LOG_XML_ERRORF(format, ...)                                                                        \
    fprintf(stderr, "XML_CONFIG_LOADER: %s:%d: at line %lu, column %lu: " format "\n", __FILE__, __LINE__, \
            XML_GetCurrentLineNumber(ctx->parser), XML_GetCurrentColumnNumber(ctx->parser), __VA_ARGS__)
#else
#define LOG(str)
#define LOG_XML_ERROR(str)
#define LOGF(format, ...)
#define LOG_XML_ERRORF(format, ...)
#endif

#define TEXT_EQUALS(x, y) (strcmp((x), (y)) == 0)
#define LOG_MEMORY_ALLOCATION_FAILURE LOG("Memory allocation failure")

#define TAG_PUBSUB "PubSub"
#define TAG_CONNECTION "connection"
#define TAG_MESSAGE "message"
#define TAG_DATASET "dataset"
#define TAG_VARIABLE "variable"
#define TAG_SKS_SERVER "skserver"

#define ATTR_CONNECTION_PUB_ID "publisherId" // TODO: remove Publisher Id from CONNECTION level
#define ATTR_CONNECTION_ADDR "address"
#define ATTR_CONNECTION_MODE "mode"
#define ATTR_CONNECTION_MODE_VAL_PUB "publisher"
#define ATTR_CONNECTION_MODE_VAL_SUB "subscriber"
#define ATTR_CONNECTION_IFNAME "interfaceName"
#define ATTR_CONNECTION_MQTTUSERNAME "mqttUsername"
#define ATTR_CONNECTION_MQTTPASSWORD "mqttPassword"
#define ATTR_CONNECTION_ACYCLIC_PUBLISHER "acyclicPublisher"

#define ATTR_MESSAGE_PUBLISHING_ITV "publishingInterval"
#define ATTR_MESSAGE_PUBLISHING_OFF "publishingOffset"
#define ATTR_MESSAGE_SECURITY "securityMode"
#define ATTR_MESSAGE_SECURITY_VAL_NONE "none"
#define ATTR_MESSAGE_SECURITY_VAL_SIGN "sign"
#define ATTR_MESSAGE_SECURITY_VAL_SIGNANDENCRYPT "signAndEncrypt"

#define ATTR_MESSAGE_PUBLISHER_ID "publisherId"
#define ATTR_MESSAGE_GROUP_ID "groupId"
#define ATTR_MESSAGE_GROUP_VERSION "groupVersion"
#define ATTR_MESSAGE_MQTT_TOPIC "mqttTopic"
#define ATTR_MESSAGE_KEEP_ALIVE "keepAliveTime"
#define ATTR_MESSAGE_ENCODING "encoding"
#define ATTR_MESSAGE_FIXED_SIZE "publisherFixedSize"

#define ATTR_DATASET_WRITER_ID "writerId"
#define ATTR_DATASET_SEQ_NUM "useSequenceNumber"
#define ATTR_DATASET_TIMESTAMP "timestamp"

#define ATTR_VARIABLE_NODE_ID "nodeId"
#define ATTR_VARIABLE_DISPLAY_NAME "displayName"
#define ATTR_VARIABLE_DATA_TYPE "dataType"
#define ATTR_VARIABLE_VALUE_RANK "valueRank"
#define ATTR_VARIABLE_ARRAY_DIMENSIONS "arrayDimensions"

#define SCALAR_ARRAY_RANK -1

#define ATTR_SKS_ENDPOINT_URL "endpointUrl"
#define ATTR_SKS_SERVER_CERT_PATH "serverCertPath"

typedef enum
{
    PARSE_START,      // Beginning of file
    PARSE_PUBSUB,     // In a PubSub
    PARSE_CONNECTION, // In a Connection
    PARSE_DATASET,    // In a DataSet
    PARSE_MESSAGE,    // In a connection message
    PARSE_VARIABLE,   // In a message variable
    PARSE_SKS,        // In a SKS Server
} parse_state_t;

typedef struct parse_arrayDimensions_t
{
    uint32_t* arrayDimensions; // Array of Dimensions
    int16_t len;               // len of sets dimensions. Value rank cannot exceed INT16_MAX
} parse_arrayDimensions_t;

struct sopc_xml_pubsub_variable_t
{
    SOPC_NodeId* nodeId;
    SOPC_LocalizedText displayName;
    bool has_displayName;
    SOPC_BuiltinId dataType;
    bool has_dataType;
    int16_t valueRank;
    parse_arrayDimensions_t arrayDimensions; // Used only when value rank is > 0
};

struct sopc_xml_pubsub_dataset_t
{
    uint16_t writer_id;
    bool useDsmSeqNum;
    bool useDsmTimestamp;
    uint16_t nb_variables;
    struct sopc_xml_pubsub_variable_t* variableArr;
};

/**
 * \brief Structure to defines access to a Security Keys Server
 */
struct sopc_xml_pubsub_sks_t
{
    char* endpointUrl;
    char* serverCertPath;
};

struct sopc_xml_pubsub_message_t
{
    double publishing_interval;
    int32_t publishing_offset;
    SOPC_Conf_PublisherId publisher_id;
    uint32_t version;
    SOPC_SecurityMode_Type security_mode;
    uint16_t nb_datasets;
    uint16_t groupId;
    uint32_t groupVersion;
    char* mqttTopic;
    SOPC_Pubsub_MessageEncodingType encoding;
    struct sopc_xml_pubsub_dataset_t* datasetArr;
    double keepAliveTime;
    bool publisherFixedSize;

    /* Array of to define Security Key Servers (SKS) that manage the security keys for the SecurityGroup
       assigned to the PubSubGroup. Null if the SecurityMode is None. */
    uint32_t nb_sks;
    struct sopc_xml_pubsub_sks_t* sksArr;
};

struct sopc_xml_pubsub_connection_t
{
    char* address;
    SOPC_Conf_PublisherId publisher_id;
    bool is_publisher;
    bool is_mode_set;
    char* interfaceName;
    uint16_t nb_messages;
    char* mqttUsername;
    char* mqttPassword;
    bool is_acyclic;
    struct sopc_xml_pubsub_message_t* messageArr;
};

struct parse_context_t
{
    XML_Parser parser;
    bool has_publisher;
    uint32_t nb_connections;
    uint32_t nb_pubconnections;
    uint16_t nb_datasets;
    uint16_t nb_messages;
    struct sopc_xml_pubsub_connection_t* connectionArr;
    struct sopc_xml_pubsub_message_t* currentMessage;
    parse_state_t state;
};

static bool parse(XML_Parser parser, FILE* fd)
{
    char buf[65365];
    if (NULL == fd)
    {
        LOG("Error: no input file provided");
        return false;
    }

    while (!feof(fd))
    {
        size_t r = fread(buf, sizeof(char), sizeof(buf) / sizeof(char), fd);

        if ((0 == r) && (ferror(fd) != 0))
        {
            LOGF("Error while reading input file: %s", strerror(errno));
            return false;
        }

        if (XML_Parse(parser, buf, (int) r, 0) != XML_STATUS_OK)
        {
            const enum XML_Error parser_error = XML_GetErrorCode(parser);

            if (parser_error != XML_ERROR_NONE)
            {
                fprintf(stderr, "XML parsing failed at line %lu, column %lu. Error code is %u.\n",
                        XML_GetCurrentLineNumber(parser), XML_GetCurrentColumnNumber(parser), XML_GetErrorCode(parser));
            }

            // else, the error comes from one of the callbacks, that log an error
            // themselves.

            return false;
        }
    }

    // Tell the parser that we are at the end of the file
    if (XML_Parse(parser, "", 0, 1) != XML_STATUS_OK)
    {
        return false;
    }

    return true;
}

static bool parse_signed64_value(const char* data, int64_t* dest)
{
    char buf[21];
    const size_t len = strlen(data);

    if (NULL == dest)
    {
        return false;
    }

    memcpy(buf, data, len);
    buf[len] = '\0';

    char* endptr;
    *dest = strtol(buf, &endptr, 10);

    if (endptr != (buf + len))
    {
        return false;
    }

    return true;
}

static bool parse_unsigned_value(const char* data, size_t len, uint8_t width, void* dest)
{
    char buf[21];

    if (len >= (sizeof(buf) / sizeof(char) - 1))
    {
        return false;
    }

    memcpy(buf, data, len);
    buf[len] = '\0';

    char* endptr;
    uint64_t val = strtoul(buf, &endptr, 10);

    if (endptr != (buf + len))
    {
        return false;
    }

    if (width == 8 && val <= UINT8_MAX)
    {
        *((uint8_t*) dest) = (uint8_t) val;
        return true;
    }
    else if (width == 16 && val <= UINT16_MAX)
    {
        *((uint16_t*) dest) = (uint16_t) val;
        return true;
    }
    else if (width == 32 && val <= UINT32_MAX)
    {
        *((uint32_t*) dest) = (uint32_t) val;
        return true;
    }
    else if (width == 64)
    {
        *((uint64_t*) dest) = (uint64_t) val;
        return true;
    }
    else
    {
        // Invalid width and/or out of bounds value
        return false;
    }
}

static bool parse_boolean(const char* data, size_t len, bool* dest)
{
    bool result = false;
    int res = SOPC_strncmp_ignore_case(data, "true", len);
    if (0 == res)
    {
        *dest = true;
        result = true;
    }
    else
    {
        res = SOPC_strncmp_ignore_case(data, "false", len);
        if (0 == res)
        {
            *dest = false;
            result = true;
        }
    }
    return result;
}

static inline bool check_arrayDimension_isValid(uint32_t arrayDimension)
{
    return arrayDimension <= INT32_MAX;
}

static bool parse_arrayDimensions(const char* data, parse_arrayDimensions_t* dest)
{
    bool result = true;
    size_t nbArrayDimensions = 1;
    size_t startIndex = 0;
    size_t len = strlen(data);
    int indexArrayDimension = 0;

    if (len <= 0)
    {
        result = false;
    }
    for (size_t i = 0; result && i < len; i++)
    {
        if (',' == data[i])
        {
            nbArrayDimensions++;
        }
        else if (data[i] < '0' || data[i] > '9')
        {
            result = false;
        }
    }
    SOPC_ASSERT(nbArrayDimensions < INT16_MAX);
    if (result)
    {
        dest->arrayDimensions = SOPC_Calloc(nbArrayDimensions, sizeof(uint32_t));
        dest->len = (int16_t) nbArrayDimensions;
        result = (NULL != dest->arrayDimensions);
    }

    for (size_t i = 0; result && i < len; i++)
    {
        if (',' == data[i])
        {
            SOPC_ASSERT(startIndex < len);
            result = (SOPC_STATUS_OK ==
                      SOPC_strtouint32_t(&data[startIndex], &dest->arrayDimensions[indexArrayDimension], 10, ','));
            result &= check_arrayDimension_isValid(dest->arrayDimensions[indexArrayDimension]);
            indexArrayDimension++;
            startIndex = i + 1;
        }
    }
    if (result)
    {
        result = (SOPC_STATUS_OK ==
                  SOPC_strtouint32_t(&data[startIndex], &dest->arrayDimensions[indexArrayDimension], 10, '\0'));
        result &= check_arrayDimension_isValid(dest->arrayDimensions[indexArrayDimension]);
    }

    if (!result && NULL != dest->arrayDimensions)
    {
        SOPC_Free(dest->arrayDimensions);
        dest->arrayDimensions = NULL;
        dest->len = 0;
    }
    return result;
}

static bool copy_any_string_attribute_value(char** to, const char* from)
{
    SOPC_ASSERT(to != NULL);
    bool result = false;
    *to = SOPC_Malloc(strlen(from) + 1);
    if (NULL == *to)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
    }
    else
    {
        strcpy(*to, from);
        result = true;
    }
    return result;
}

static bool parse_publisher_id(SOPC_Conf_PublisherId* publisherId, const char* from)
{
    SOPC_ASSERT(NULL != publisherId);
    bool result = false;

    if (0 == SOPC_strncmp_ignore_case("i=", &from[0], 2))
    {
        publisherId->type = SOPC_UInteger_PublisherId;
        result = parse_unsigned_value(&from[2], strlen(&from[2]), 64, &publisherId->data.uint);
    }
    else if (0 == SOPC_strncmp_ignore_case("s=", &from[0], 2))
    {
        publisherId->type = SOPC_String_PublisherId;
        result = (SOPC_STATUS_OK == SOPC_String_InitializeFromCString(&publisherId->data.string, &from[2]));
    }
    else
    {
        result = false;
    }
    return result;
}

static bool builtintype_id_from_tag(const char* typeName, SOPC_BuiltinId* type_id)
{
    static const struct
    {
        const char* name;
        SOPC_BuiltinId id;
    } TYPE_IDS[] = {{"Null", 0},           {"Boolean", 1},     {"SByte", 2},           {"Byte", 3},
                    {"Int16", 4},          {"UInt16", 5},      {"Int32", 6},           {"UInt32", 7},
                    {"Int64", 8},          {"UInt64", 9},      {"Float", 10},          {"Double", 11},
                    {"DateTime", 13},      {"String", 12},     {"ByteString", 15},     {"Guid", 14},
                    {"XmlElement", 16},    {"NodeId", 17},     {"ExpandedNodeId", 18}, {"QualifiedName", 20},
                    {"LocalizedText", 21}, {"StatusCode", 19}, {"Structure", 22},      {NULL, 0}};

    for (size_t i = 0; TYPE_IDS[i].name != NULL; ++i)
    {
        if (strcmp(typeName, TYPE_IDS[i].name) == 0)
        {
            *type_id = TYPE_IDS[i].id;
            return true;
        }
    }

    return false;
}

/**
 * /brief This callback is used to extract a given attribute (name and value)
 * /param attr_name The attribute name (non-NULL).
 * /param attr_val The attribute value (non-NULL).
 * /param user_param A user defined parameter
 * /return true if the parameter is correct and expected
 */
typedef bool attribute_parser_cb(const char* attr_name,
                                 const char* attr_val,
                                 struct parse_context_t* ctx,
                                 void* user_param);

/**
 * /brief Parse all attributes and calls the user callback for every valid attribute.
 * /param attrs A NULL-terminated array of attributes
 * /param ctx A non-NULL pointer to the parsing context.
 * /param callback A non-NULL pointer to the user parsing callback.
 * /param user_param The user parameter that will be passed to 'callback'
 * /return true if the parsing is successful
 */
static bool parse_attributes(const XML_Char** attrs,
                             attribute_parser_cb* callback,
                             struct parse_context_t* ctx,
                             void* user_param)
{
    bool result = true;
    SOPC_ASSERT(NULL != callback && NULL != attrs);
    for (size_t i = 0; attrs[i] && result; ++i)
    {
        // Current attribute name
        const char* attr_name = attrs[i];
        const char* attr_val = attrs[++i];
        if (NULL == attr_val)
        {
            LOG_XML_ERRORF("Missing value for attribute '%s", attr_name);
            result = false;
        }
        else
        {
            result = callback(attr_name, attr_val, ctx, user_param);
            if (!result)
            {
                LOG_XML_ERRORF("Invalid attribute argument : (%s='%s')", attr_name, attr_val);
            }
        }
    }
    return result;
}

static bool parse_connection_attributes(const char* attr_name,
                                        const char* attr_val,
                                        struct parse_context_t* ctx,
                                        void* user_param)
{
    bool result = false;
    SOPC_ASSERT(NULL != user_param);
    struct sopc_xml_pubsub_connection_t* connection = (struct sopc_xml_pubsub_connection_t*) user_param;

    if (TEXT_EQUALS(ATTR_CONNECTION_ADDR, attr_name))
    {
        result = copy_any_string_attribute_value(&connection->address, attr_val);
    }
    else if (TEXT_EQUALS(ATTR_CONNECTION_MODE, attr_name))
    {
        connection->is_mode_set = true;
        if (TEXT_EQUALS(ATTR_CONNECTION_MODE_VAL_PUB, attr_val))
        {
            connection->is_publisher = true;
            result = true;
            ctx->nb_pubconnections++;
            ctx->has_publisher = true;
        }
        else if (TEXT_EQUALS(ATTR_CONNECTION_MODE_VAL_SUB, attr_val))
        {
            connection->is_publisher = false;
            result = true;
        }
    }
    else if (TEXT_EQUALS(ATTR_CONNECTION_IFNAME, attr_name))
    {
        result = copy_any_string_attribute_value(&connection->interfaceName, attr_val);
    }
    else if (TEXT_EQUALS(ATTR_CONNECTION_PUB_ID, attr_name))
    {
        result = parse_publisher_id(&(connection->publisher_id), attr_val);
    }
    else if (TEXT_EQUALS(ATTR_CONNECTION_MQTTUSERNAME, attr_name))
    {
        result = copy_any_string_attribute_value(&connection->mqttUsername, attr_val);
    }
    else if (TEXT_EQUALS(ATTR_CONNECTION_MQTTPASSWORD, attr_name))
    {
        result = copy_any_string_attribute_value(&connection->mqttPassword, attr_val);
    }
    else if (TEXT_EQUALS(ATTR_CONNECTION_ACYCLIC_PUBLISHER, attr_name))
    {
        result = parse_boolean(attr_val, strlen(attr_val), &connection->is_acyclic);
    }
    else
    {
        LOG_XML_ERRORF("Unexpected 'connection' attribute <%s>", attr_name);
    }
    return result;
}

static bool start_connection(struct parse_context_t* ctx, const XML_Char** attrs)
{
    struct sopc_xml_pubsub_connection_t* connection = &ctx->connectionArr[ctx->nb_connections - 1];
    memset(connection, 0, sizeof *connection);

    bool result = parse_attributes(attrs, parse_connection_attributes, ctx, (void*) connection);

    if (result)
    {
        if (!connection->address)
        {
            LOG_XML_ERROR("Connection address is missing");
            result = false;
        }
        else if (!connection->is_mode_set)
        {
            LOG_XML_ERROR("Connection mode is missing");
            result = false;
        }
        else if (connection->is_publisher && SOPC_Null_PublisherId == connection->publisher_id.type)
        {
            LOG_XML_ERROR("Connection mode is 'publisher' but no 'publisherId' is defined");
            result = false;
        }
        else if (!connection->is_publisher && (SOPC_UInteger_PublisherId == connection->publisher_id.type ||
                                               SOPC_String_PublisherId == connection->publisher_id.type))
        {
            LOG_XML_ERROR("Connection mode is 'subscriber' and a 'publisherId' is defined");
            result = false;
        }
        if (connection->is_publisher && SOPC_UInteger_PublisherId == connection->publisher_id.type)
        {
            if (0 == connection->publisher_id.data.uint)
            {
                LOG_XML_ERROR("Publisher Id type uinteger cannot be equal to 0");
                result = false;
            }
        }
        else if (connection->is_publisher && SOPC_String_PublisherId == connection->publisher_id.type)
        {
            if (connection->publisher_id.data.string.Length <= 0)
            {
                LOG_XML_ERROR("Publisher Id type string cannot be empty");
                result = false;
            }
        }
        if (result)
        {
            ctx->state = PARSE_CONNECTION;
        }
    }
    return result;
}

static bool parse_message_attributes(const char* attr_name,
                                     const char* attr_val,
                                     struct parse_context_t* ctx,
                                     void* user_param)
{
    SOPC_UNUSED_ARG(ctx);
    bool result = false;
    SOPC_ASSERT(NULL != user_param);
    struct sopc_xml_pubsub_message_t* msg = (struct sopc_xml_pubsub_message_t*) user_param;

    if (TEXT_EQUALS(ATTR_MESSAGE_PUBLISHING_ITV, attr_name))
    {
        result = SOPC_strtodouble(attr_val, strlen(attr_val), 64, &msg->publishing_interval);
        result &= msg->publishing_interval > 0.;
    }
    else if (TEXT_EQUALS(ATTR_MESSAGE_PUBLISHING_OFF, attr_name))
    {
        result = SOPC_strtouint(attr_val, strlen(attr_val), 32, &msg->publishing_offset);
    }
    else if (TEXT_EQUALS(ATTR_MESSAGE_SECURITY, attr_name))
    {
        result = true;
        if (TEXT_EQUALS(ATTR_MESSAGE_SECURITY_VAL_NONE, attr_val))
        {
            msg->security_mode = SOPC_SecurityMode_None;
        }
        else if (TEXT_EQUALS(ATTR_MESSAGE_SECURITY_VAL_SIGN, attr_val))
        {
            msg->security_mode = SOPC_SecurityMode_Sign;
        }
        else if (TEXT_EQUALS(ATTR_MESSAGE_SECURITY_VAL_SIGNANDENCRYPT, attr_val))
        {
            msg->security_mode = SOPC_SecurityMode_SignAndEncrypt;
        }
        else
        {
            result = false;
        }
    }
    else if (TEXT_EQUALS(ATTR_MESSAGE_PUBLISHER_ID, attr_name))
    {
        result = parse_publisher_id(&msg->publisher_id, attr_val);
    }
    else if (TEXT_EQUALS(ATTR_MESSAGE_GROUP_ID, attr_name))
    {
        result = parse_unsigned_value(attr_val, strlen(attr_val), 16, &msg->groupId);
        result &= msg->groupId > 0;
    }
    else if (TEXT_EQUALS(ATTR_MESSAGE_GROUP_VERSION, attr_name))
    {
        result = parse_unsigned_value(attr_val, strlen(attr_val), 32, &msg->groupVersion);
        result &= msg->groupVersion > 0;
    }
    else if (TEXT_EQUALS(ATTR_MESSAGE_MQTT_TOPIC, attr_name))
    {
        result = copy_any_string_attribute_value(&msg->mqttTopic, attr_val);
    }
    else if (TEXT_EQUALS(ATTR_MESSAGE_KEEP_ALIVE, attr_name))
    {
        result = SOPC_strtodouble(attr_val, strlen(attr_val), sizeof(double) * 8, &msg->keepAliveTime);
    }
    else if (TEXT_EQUALS(ATTR_MESSAGE_ENCODING, attr_name))
    {
        result = true;
        if (0 == strcmp("uadp", attr_val))
        {
            msg->encoding = SOPC_MessageEncodeUADP;
        }
        else if (0 == strcmp("json", attr_val))
        {
            msg->encoding = SOPC_MessageEncodeJSON;
        }
        else
        {
            LOG_XML_ERRORF("Unexpected '%s' <%s>", ATTR_MESSAGE_ENCODING, attr_val);
            result = false;
        }
    }
    else if (TEXT_EQUALS(ATTR_MESSAGE_FIXED_SIZE, attr_name))
    {
        result = parse_boolean(attr_val, strlen(attr_val), &msg->publisherFixedSize);
    }
    else
    {
        LOG_XML_ERRORF("Unexpected 'message' attribute <%s>", attr_name);
    }
    return result;
}

static bool start_message(struct parse_context_t* ctx, struct sopc_xml_pubsub_message_t* msg, const XML_Char** attrs)
{
    memset(msg, 0, sizeof *msg);
    // Security is disabled if it is not configured
    msg->security_mode = SOPC_SecurityMode_None;
    msg->publishing_interval = 0.0;
    msg->publishing_offset = -1;
    msg->keepAliveTime = -1.0;
    msg->publisherFixedSize = false;
    bool result = parse_attributes(attrs, parse_message_attributes, ctx, (void*) msg);

    if (result)
    {
        if (msg->publishing_interval <= 0.0)
        {
            LOG_XML_ERROR("Message publishing interval is missing");
            result = false;
        }
        else if (msg->publishing_interval <= msg->publishing_offset)
        {
            LOG_XML_ERROR("Message publishOffset cannot be greater than publishInterval");
            result = false;
        }
        else if (msg->groupId == 0)
        {
            LOG_XML_ERROR("Message groupId is missing");
            result = false;
        }
        else if (msg->keepAliveTime <= 0.0 && ctx->connectionArr[ctx->nb_connections - 1].is_acyclic)
        {
            // An acyclic publisher shall provide a keep alive timer for all messages
            LOG_XML_ERROR("Message keepAliveTime is missing");
            result = false;
        }
        else if (ctx->connectionArr[ctx->nb_connections - 1].is_publisher &&
                 (SOPC_UInteger_PublisherId == msg->publisher_id.type ||
                  SOPC_String_PublisherId == msg->publisher_id.type))
        {
            // A publisher connection shall NOT provide a message publisherId source (itself here)
            LOG_XML_ERROR("Message publisherId shall not be provided in a connection in publisher mode");
            result = false;
        }
        if (!ctx->connectionArr[ctx->nb_connections - 1].is_publisher &&
            SOPC_UInteger_PublisherId == msg->publisher_id.type)
        {
            if (0 == msg->publisher_id.data.uint)
            {
                LOG_XML_ERROR("Publisher Id type uinteger cannot be equal to 0");
                result = false;
            }
        }
        else if (!ctx->connectionArr[ctx->nb_connections - 1].is_publisher &&
                 SOPC_String_PublisherId == msg->publisher_id.type)
        {
            if (msg->publisher_id.data.string.Length <= 0)
            {
                LOG_XML_ERROR("Publisher Id type string cannot be empty");
                result = false;
            }
        }
        if (!ctx->connectionArr[ctx->nb_connections - 1].is_publisher && true == msg->publisherFixedSize)
        {
            LOG_XML_ERROR("Fixed size buffer is an optimization only available for publisher");
            result = false;
        }
        if (result)
        {
            ctx->state = PARSE_MESSAGE;
        }
    }
    return result;
}

static bool parse_dataset_attributes(const char* attr_name,
                                     const char* attr_val,
                                     struct parse_context_t* ctx,
                                     void* user_param)
{
    SOPC_UNUSED_ARG(ctx);
    bool result = false;
    SOPC_ASSERT(NULL != user_param);
    struct sopc_xml_pubsub_dataset_t* ds = (struct sopc_xml_pubsub_dataset_t*) user_param;

    if (TEXT_EQUALS(ATTR_DATASET_WRITER_ID, attr_name))
    {
        // The writerId cannot be equal to 0
        result = parse_unsigned_value(attr_val, strlen(attr_val), 16, &ds->writer_id);
        result &= ds->writer_id > 0;
    }
    else if (TEXT_EQUALS(ATTR_DATASET_SEQ_NUM, attr_name))
    {
        result = parse_boolean(attr_val, strlen(attr_val), &ds->useDsmSeqNum);
    }
    else if (TEXT_EQUALS(ATTR_DATASET_TIMESTAMP, attr_name))
    {
        result = parse_boolean(attr_val, strlen(attr_val), &ds->useDsmTimestamp);
    }
    else
    {
        LOG_XML_ERRORF("Unexpected 'dataset' attribute <%s>", attr_name);
    }
    return result;
}

static bool start_dataset(struct parse_context_t* ctx, struct sopc_xml_pubsub_dataset_t* ds, const XML_Char** attrs)
{
    memset(ds, 0, sizeof *ds);
    SOPC_ASSERT(NULL != ctx->currentMessage);
    ds->writer_id = 0;
    ds->useDsmSeqNum = true;

    bool result = parse_attributes(attrs, parse_dataset_attributes, ctx, (void*) ds);

    if (ds->writer_id == 0)
    {
        LOG_XML_ERROR("WriterId missing in dataSet");
        result = false;
    }
    ctx->state = PARSE_DATASET;
    return result;
}

static bool parse_variable_attributes(const char* attr_name,
                                      const char* attr_val,
                                      struct parse_context_t* ctx,
                                      void* user_param)
{
    SOPC_UNUSED_ARG(ctx);
    bool result = false;
    SOPC_ASSERT(NULL != user_param);
    struct sopc_xml_pubsub_variable_t* var = (struct sopc_xml_pubsub_variable_t*) user_param;

    if (TEXT_EQUALS(ATTR_VARIABLE_NODE_ID, attr_name))
    {
        SOPC_ASSERT(strlen(attr_val) <= INT32_MAX);
        var->nodeId = SOPC_NodeId_FromCString(attr_val, (int32_t) strlen(attr_val));
        result = (NULL != var->nodeId);
    }
    else if (TEXT_EQUALS(ATTR_VARIABLE_DISPLAY_NAME, attr_name))
    {
        var->has_displayName = true;
        SOPC_LocalizedText_Initialize(&var->displayName);
        SOPC_ReturnStatus status = SOPC_String_CopyFromCString(&var->displayName.defaultText, attr_val);
        result = (SOPC_STATUS_OK == status);
        if (!result)
        {
            LOG_MEMORY_ALLOCATION_FAILURE;
        }
    }
    else if (TEXT_EQUALS(ATTR_VARIABLE_VALUE_RANK, attr_name))
    {
        int64_t rank;

        result = parse_signed64_value(attr_val, &rank) && rank >= -3 && rank <= 65535;
        if (result && (rank > 0 || rank == -1))
        {
            var->valueRank = (int16_t) rank;
        }
        else
        {
            result = false;
            LOG_XML_ERRORF("Value rank %" PRIi64 " not supported", rank);
        }
    }
    else if (TEXT_EQUALS(ATTR_VARIABLE_ARRAY_DIMENSIONS, attr_name))
    {
        parse_arrayDimensions_t arrayDimensions = {.arrayDimensions = NULL, .len = 0};
        result = parse_arrayDimensions(attr_val, &arrayDimensions);
        if (result && arrayDimensions.arrayDimensions != NULL)
        {
            var->arrayDimensions = arrayDimensions;
        }
    }
    else if (TEXT_EQUALS(ATTR_VARIABLE_DATA_TYPE, attr_name))
    {
        var->has_dataType = true;
        result = builtintype_id_from_tag(attr_val, &var->dataType);
        if (!result)
        {
            LOG_XML_ERRORF("DataType attribute value not recognized as BuiltInType: '%s", attr_val);
        }
    }
    else
    {
        LOG_XML_ERRORF("Unexpected 'variable' attribute <%s>", attr_name);
    }
    return result;
}

static bool start_variable(struct parse_context_t* ctx, struct sopc_xml_pubsub_variable_t* var, const XML_Char** attrs)
{
    memset(var, 0, sizeof *var);
    var->valueRank = SCALAR_ARRAY_RANK;
    var->nodeId = NULL;
    var->has_displayName = false;
    var->has_dataType = false;
    var->arrayDimensions.arrayDimensions = NULL;
    var->arrayDimensions.len = 0;

    bool result = parse_attributes(attrs, parse_variable_attributes, ctx, (void*) var);

    if (result)
    {
        // user don't set sufficient value for arrayDimensions compare to valueRank. Fill the rest with zeroes
        if (var->valueRank > 0)
        {
            if (var->arrayDimensions.len < var->valueRank)
            {
                parse_arrayDimensions_t arrDimensionTmp = var->arrayDimensions;
                var->arrayDimensions.arrayDimensions = SOPC_Calloc((size_t) var->valueRank, sizeof(uint32_t));
                if (NULL != arrDimensionTmp.arrayDimensions)
                {
                    memcpy(var->arrayDimensions.arrayDimensions, arrDimensionTmp.arrayDimensions,
                           sizeof(uint32_t) * (size_t) arrDimensionTmp.len);
                    SOPC_Free(arrDimensionTmp.arrayDimensions);
                }
                var->arrayDimensions.len = var->valueRank;
            }
        }
        if (!var->nodeId)
        {
            LOG_XML_ERROR("Variable nodeId is missing");
            result = false;
        }
        else if (!var->has_displayName)
        {
            LOG_XML_ERROR("Variable displayName is missing");
            result = false;
        }
        else if (!var->has_dataType)
        {
            LOG_XML_ERROR("Variable dataType is missing");
            result = false;
        }
        else
        {
            ctx->state = PARSE_VARIABLE;
        }
    }
    return result;
}

static bool start_sks(struct parse_context_t* ctx, struct sopc_xml_pubsub_sks_t* sks, const XML_Char** attrs)
{
    bool endpointUrl = false;
    bool serverCert = false;

    memset(sks, 0, sizeof *sks);

    for (size_t i = 0; attrs[i]; ++i)
    {
        // Current attribute name
        const char* attr = attrs[i];

        if (strcmp(ATTR_SKS_ENDPOINT_URL, attr) == 0)
        {
            const char* attr_val = attrs[++i];

            if (NULL == attr_val)
            {
                LOG_XML_ERROR("Missing value for sks endpointUrl");
                return false;
            }

            SOPC_ASSERT(strlen(attr_val) <= INT32_MAX);
            sks->endpointUrl = SOPC_Malloc(strlen(attr_val) + 1);
            if (NULL == sks->endpointUrl)
            {
                LOG_MEMORY_ALLOCATION_FAILURE;
                return false;
            }
            sks->endpointUrl = strcpy(sks->endpointUrl, attr_val);
            endpointUrl = true;
        }
        else if (strcmp(ATTR_SKS_SERVER_CERT_PATH, attr) == 0)
        {
            const char* attr_val = attrs[++i];

            if (NULL == attr_val)
            {
                LOG_XML_ERROR("Missing value for sks serverCertPath");
                return false;
            }

            SOPC_ASSERT(strlen(attr_val) <= INT32_MAX);
            sks->serverCertPath = SOPC_Malloc(strlen(attr_val) + 1);
            if (NULL == sks->serverCertPath)
            {
                LOG_MEMORY_ALLOCATION_FAILURE;
                return false;
            }
            sks->serverCertPath = strcpy(sks->serverCertPath, attr_val);
            serverCert = true;
        }
        else
        {
            ++i; // Skip value of unknown attribute
        }
    }

    if (!endpointUrl)
    {
        LOG_XML_ERROR("SKS endpointUrl is missing");
        return false;
    }
    else if (!serverCert)
    {
        LOG_XML_ERROR("SKS serverCertPath is missing");
        return false;
    }

    ctx->state = PARSE_SKS;
    return true;
}

static void start_element_handler(void* user_data, const XML_Char* name, const XML_Char** attrs)
{
    struct parse_context_t* ctx = user_data;
    struct sopc_xml_pubsub_connection_t* connection = NULL;
    struct sopc_xml_pubsub_message_t* msg = NULL;
    struct sopc_xml_pubsub_dataset_t* ds = NULL;

    bool isDataSet = false;
    bool isSkserver = false;

    switch (ctx->state)
    {
    case PARSE_START:
        if (strcmp(name, TAG_PUBSUB) != 0)
        {
            LOG_XML_ERRORF("Unexpected tag %s (expected " TAG_PUBSUB ")", name);
            XML_StopParser(ctx->parser, 0);
            return;
        }
        ctx->state = PARSE_PUBSUB;
        break;
    case PARSE_PUBSUB:
        if (strcmp(name, TAG_CONNECTION) != 0)
        {
            LOG_XML_ERRORF("Unexpected tag %s (expected " TAG_CONNECTION ")", name);
            XML_StopParser(ctx->parser, 0);
            return;
        }
        if (NULL == ctx->connectionArr)
        {
            SOPC_ASSERT(ctx->nb_connections == 0);
            ctx->connectionArr = SOPC_Malloc(sizeof *ctx->connectionArr);
            ctx->nb_connections = 1;
            SOPC_ASSERT(NULL != ctx->connectionArr);
        }
        else
        {
            SOPC_ASSERT(ctx->nb_connections > 0);
            ctx->nb_connections++;
            ctx->connectionArr =
                SOPC_Realloc(ctx->connectionArr, (size_t)(ctx->nb_connections - 1) * sizeof *ctx->connectionArr,
                             ctx->nb_connections * sizeof *ctx->connectionArr);
            SOPC_ASSERT(NULL != ctx->connectionArr);
        }
        if (!start_connection(ctx, attrs))
        {
            XML_StopParser(ctx->parser, 0);
            return;
        }
        break;
    case PARSE_CONNECTION:
        if (strcmp(name, TAG_MESSAGE) != 0)
        {
            LOG_XML_ERRORF("Unexpected tag %s (expected " TAG_MESSAGE ")", name);
            XML_StopParser(ctx->parser, 0);
            return;
        }
        connection = &ctx->connectionArr[ctx->nb_connections - 1];
        if (NULL == connection->messageArr)
        {
            SOPC_ASSERT(connection->nb_messages == 0);
            connection->messageArr = SOPC_Malloc(sizeof *connection->messageArr);
            connection->nb_messages = 1;
            SOPC_ASSERT(NULL != connection->messageArr);
        }
        else
        {
            SOPC_ASSERT(connection->nb_messages > 0);
            connection->nb_messages++;
            connection->messageArr = SOPC_Realloc(
                connection->messageArr, (size_t)(connection->nb_messages - 1) * sizeof *connection->messageArr,
                connection->nb_messages * sizeof *connection->messageArr);
            SOPC_ASSERT(NULL != connection->messageArr);
        }
        if (!start_message(ctx, &connection->messageArr[connection->nb_messages - 1], attrs))
        {
            XML_StopParser(ctx->parser, 0);
            return;
        }
        break;
    case PARSE_MESSAGE:
        isDataSet = (strcmp(name, TAG_DATASET) == 0);
        isSkserver = (strcmp(name, TAG_SKS_SERVER) == 0);
        if (!(isDataSet || isSkserver))
        {
            LOG_XML_ERRORF("Unexpected tag '%s' (expected '" TAG_DATASET "')", name);
            XML_StopParser(ctx->parser, 0);
            return;
        }
        connection = &ctx->connectionArr[ctx->nb_connections - 1];
        SOPC_ASSERT(NULL != connection);
        msg = &connection->messageArr[connection->nb_messages - 1];
        ctx->currentMessage = msg;

        if (isDataSet)
        {
            if (NULL == msg->datasetArr)
            {
                SOPC_ASSERT(msg->nb_datasets == 0);
                msg->datasetArr = SOPC_Malloc(sizeof *msg->datasetArr);
                msg->nb_datasets = 1;
                SOPC_ASSERT(NULL != msg->datasetArr);
            }
            else
            {
                SOPC_ASSERT(msg->nb_datasets > 0);
                msg->nb_datasets++;
                msg->datasetArr =
                    SOPC_Realloc(msg->datasetArr, (size_t)(msg->nb_datasets - 1) * sizeof *msg->datasetArr,
                                 msg->nb_datasets * sizeof *msg->datasetArr);
                SOPC_ASSERT(NULL != msg->datasetArr);
            }
            if (!start_dataset(ctx, &msg->datasetArr[msg->nb_datasets - 1], attrs))
            {
                XML_StopParser(ctx->parser, 0);
                return;
            }
        }
        else
        {
            if (NULL == msg->sksArr)
            {
                SOPC_ASSERT(msg->nb_sks == 0);
                msg->sksArr = SOPC_Malloc(sizeof *msg->sksArr);
                msg->nb_sks = 1;
                SOPC_ASSERT(NULL != msg->sksArr);
            }
            else
            {
                SOPC_ASSERT(msg->nb_sks > 0);
                msg->nb_sks++;
                msg->sksArr = SOPC_Realloc(msg->sksArr, (size_t)(msg->nb_sks - 1) * sizeof(*msg->sksArr),
                                           (size_t) msg->nb_sks * sizeof(*msg->sksArr));
                SOPC_ASSERT(NULL != msg->sksArr);
            }
            if (!start_sks(ctx, &msg->sksArr[msg->nb_sks - 1], attrs))
            {
                XML_StopParser(ctx->parser, 0);
                return;
            }
        }
        break;
    case PARSE_DATASET:
        if (strcmp(name, TAG_VARIABLE) != 0)
        {
            LOG_XML_ERRORF("Unexpected tag '%s' (expected '" TAG_VARIABLE "')", name);
            XML_StopParser(ctx->parser, 0);
            return;
        }
        connection = &ctx->connectionArr[ctx->nb_connections - 1];
        SOPC_ASSERT(NULL != connection);
        msg = &connection->messageArr[connection->nb_messages - 1];
        SOPC_ASSERT(NULL != msg);
        ds = &msg->datasetArr[msg->nb_datasets - 1];
        if (NULL == ds->variableArr)
        {
            SOPC_ASSERT(ds->nb_variables == 0);
            ds->variableArr = SOPC_Malloc(sizeof *ds->variableArr);
            ds->nb_variables = 1;
            SOPC_ASSERT(NULL != ds->variableArr);
        }
        else
        {
            SOPC_ASSERT(ds->nb_variables > 0);
            ds->nb_variables++;
            ds->variableArr = SOPC_Realloc(ds->variableArr, (size_t)(ds->nb_variables - 1) * sizeof *ds->variableArr,
                                           ds->nb_variables * sizeof *ds->variableArr);
            SOPC_ASSERT(NULL != ds->variableArr);
        }
        if (!start_variable(ctx, &ds->variableArr[ds->nb_variables - 1], attrs))
        {
            XML_StopParser(ctx->parser, 0);
            return;
        }
        break;
    default:
        return;
    }
}

static void end_element_handler(void* user_data, const XML_Char* name)
{
    SOPC_UNUSED_ARG(name);
    struct parse_context_t* ctx = user_data;
    struct sopc_xml_pubsub_connection_t* connection = NULL;
    struct sopc_xml_pubsub_message_t* msg = NULL;

    switch (ctx->state)
    {
    case PARSE_CONNECTION:
        ctx->state = PARSE_PUBSUB;
        connection = &ctx->connectionArr[ctx->nb_connections - 1];
        SOPC_ASSERT(NULL != connection);
        if ((NULL == connection->mqttPassword) != (NULL == connection->mqttUsername))
        {
            LOG("mqttPassword and mqttUsername must be set together.");
            XML_StopParser(ctx->parser, 0);
            return;
        }
        break;
    case PARSE_MESSAGE:
        connection = &ctx->connectionArr[ctx->nb_connections - 1];
        SOPC_ASSERT(NULL != connection);
        msg = &connection->messageArr[connection->nb_messages - 1];
        SOPC_ASSERT(NULL != msg);
        if (msg->nb_datasets == 0)
        {
            LOG("Message requires at least one DataSet.");
            XML_StopParser(ctx->parser, 0);
            return;
        }
        ctx->nb_messages++;
        ctx->currentMessage = NULL;
        ctx->state = PARSE_CONNECTION;
        break;
    case PARSE_VARIABLE:
        ctx->state = PARSE_DATASET;
        break;
    case PARSE_DATASET:
        connection = &ctx->connectionArr[ctx->nb_connections - 1];
        SOPC_ASSERT(NULL != connection);
        msg = &connection->messageArr[connection->nb_messages - 1];
        SOPC_ASSERT(NULL != msg);
        SOPC_ASSERT(msg->nb_datasets > 0);
        // Check that there is no duplicate of writerId for the same Group
        for (int jDs = 0; jDs < msg->nb_datasets - 1; ++jDs)
        {
            const uint16_t writerId = msg->datasetArr[msg->nb_datasets - 1].writer_id;
            const uint16_t jWriterId = msg->datasetArr[jDs].writer_id;
            if (writerId > 0 && writerId == jWriterId)
            {
                LOG_XML_ERRORF("Multiple definition of writerId = %d in message (Group=%d, version =%d)",
                               (int) writerId, (int) msg->groupId, (int) msg->groupVersion);
                XML_StopParser(ctx->parser, 0);
                return;
            }
        }

        ctx->nb_datasets++;
        ctx->state = PARSE_MESSAGE;
        break;
    case PARSE_SKS:
        ctx->state = PARSE_MESSAGE;
        break;
    case PARSE_PUBSUB:
        break;
    case PARSE_START:
        LOG_XML_ERROR("Got end_element callback when in PARSE_START state.");
        SOPC_ASSERT(false);
        break;
    default:
        LOG_XML_ERROR("Unknown state.");
        SOPC_ASSERT(false);
        break;
    }
}

static SOPC_PubSubConfiguration* build_pubsub_config(struct parse_context_t* ctx)
{
    SOPC_PubSubConfiguration* config = SOPC_PubSubConfiguration_Create();

    if (NULL == config)
    {
        return NULL;
    }
    bool allocSuccess = SOPC_PubSubConfiguration_Allocate_PubConnection_Array(config, ctx->nb_pubconnections);
    if (allocSuccess)
    {
        allocSuccess =
            SOPC_PubSubConfiguration_Allocate_SubConnection_Array(config, ctx->nb_connections - ctx->nb_pubconnections);
    }

    SOPC_ASSERT(ctx->nb_messages <= UINT16_MAX);

    if (ctx->has_publisher && allocSuccess)
    {
        allocSuccess = SOPC_PubSubConfiguration_Allocate_PublishedDataSet_Array(config, (uint16_t) ctx->nb_datasets);
    }
    uint16_t pubDataSetIndex = 0; // Index in "SOPC_PubSubConfiguration_Allocate_PublishedDataSet_Array"

    for (uint16_t icon = 0, pubicon = 0, subicon = 0; icon < ctx->nb_connections && allocSuccess; icon++)
    {
        SOPC_PubSubConnection* connection;
        struct sopc_xml_pubsub_connection_t* p_connection = &ctx->connectionArr[icon];

        if (p_connection->is_publisher)
        {
            SOPC_ASSERT(ctx->has_publisher); // Checked on parsing

            connection = SOPC_PubSubConfiguration_Get_PubConnection_At(config, pubicon);
            SOPC_ASSERT(NULL != connection);
            pubicon++;

            // Publisher connection
            if (SOPC_String_PublisherId == p_connection->publisher_id.type)
            {
                SOPC_PubSubConnection_Set_PublisherId_String(connection, &p_connection->publisher_id.data.string);
            }
            else
            {
                SOPC_PubSubConnection_Set_PublisherId_UInteger(connection, p_connection->publisher_id.data.uint);
            }
            allocSuccess = SOPC_PubSubConnection_Allocate_WriterGroup_Array(connection, p_connection->nb_messages);
            SOPC_PublishedDataSetSourceType type = SOPC_PublishedDataItemsDataType;
            if (allocSuccess)
            {
                SOPC_PubSubConnection_Set_AcyclicPublisher(connection, p_connection->is_acyclic);
                if (p_connection->is_acyclic)
                {
                    type = SOPC_PublishedDataSetCustomSourceDataType;
                }
            }

            for (uint16_t imsg = 0; imsg < p_connection->nb_messages && allocSuccess; imsg++)
            {
                struct sopc_xml_pubsub_message_t* msg = &p_connection->messageArr[imsg];

                // Create writer group
                SOPC_WriterGroup* writerGroup = SOPC_PubSubConnection_Get_WriterGroup_At(connection, imsg);
                SOPC_WriterGroup_Set_Id(writerGroup, msg->groupId);

                SOPC_WriterGroup_Set_PublishingInterval(writerGroup, msg->publishing_interval);
                SOPC_WriterGroup_Set_PublishingOffset(writerGroup, msg->publishing_offset);
                SOPC_WriterGroup_Set_Version(writerGroup, msg->groupVersion);
                SOPC_WriterGroup_Set_SecurityMode(writerGroup, msg->security_mode);
                SOPC_WriterGroup_Set_KeepAlive(writerGroup, msg->keepAliveTime);
                SOPC_WriterGroup_Set_Encoding(writerGroup, msg->encoding);

                const SOPC_WriterGroup_Options writerGroupOptions = {.useFixedSizeBuffer = msg->publisherFixedSize};
                SOPC_WriterGroup_Set_Options(writerGroup, &writerGroupOptions);

                // Associate dataSet with writer
                SOPC_ASSERT(msg->nb_datasets < 0x100);
                allocSuccess = SOPC_WriterGroup_Allocate_DataSetWriter_Array(writerGroup, (uint8_t) msg->nb_datasets);
                // msg->publisher_id ignored if present

                if (allocSuccess)
                {
                    SOPC_WriterGroup_Set_MqttTopic(writerGroup, msg->mqttTopic);
                }

                if (NULL != msg->sksArr && allocSuccess)
                {
                    allocSuccess = SOPC_WriterGroup_Allocate_SecurityKeyServices_Array(writerGroup, msg->nb_sks);
                    for (uint32_t isks = 0; isks < msg->nb_sks && allocSuccess; isks++)
                    {
                        SOPC_SerializedCertificate* cert = NULL;
                        struct sopc_xml_pubsub_sks_t* xmlsks = &msg->sksArr[isks];
                        SOPC_SecurityKeyServices* sks = SOPC_WriterGroup_Get_SecurityKeyServices_At(writerGroup, isks);
                        SOPC_ASSERT(sks != NULL);
                        allocSuccess = SOPC_SecurityKeyServices_Set_EndpointUrl(sks, xmlsks->endpointUrl);
                        SOPC_ReturnStatus status =
                            SOPC_KeyManager_SerializedCertificate_CreateFromFile(xmlsks->serverCertPath, &cert);
                        SOPC_SecurityKeyServices_Set_ServerCertificate(sks, cert);
                        allocSuccess = (allocSuccess && SOPC_STATUS_OK == status);
                    }
                }

                for (uint8_t ids = 0; ids < msg->nb_datasets && allocSuccess; ids++)
                {
                    struct sopc_xml_pubsub_dataset_t* ds = &msg->datasetArr[ids];

                    SOPC_PublishedDataSet* pubDataSet =
                        SOPC_PubSubConfiguration_Get_PublishedDataSet_At(config, pubDataSetIndex);
                    pubDataSetIndex++;
                    SOPC_PublishedDataSet_Init(pubDataSet, type, ds->nb_variables);

                    SOPC_DataSetWriter* dataSetWriter = SOPC_WriterGroup_Get_DataSetWriter_At(writerGroup, ids);
                    SOPC_ASSERT(dataSetWriter != NULL);
                    SOPC_DataSetWriter_Set_Id(dataSetWriter, ds->writer_id);
                    const SOPC_DataSetWriter_Options dsmOptions = {.noUseSeqNum = !ds->useDsmSeqNum,
                                                                   .noTimestamp = !ds->useDsmTimestamp};
                    SOPC_DataSetWriter_Set_Options(dataSetWriter, &dsmOptions);

                    SOPC_DataSetWriter_Set_DataSet(dataSetWriter, pubDataSet);

                    for (uint16_t ivar = 0; ivar < ds->nb_variables && allocSuccess; ivar++)
                    {
                        // Fill dataset
                        struct sopc_xml_pubsub_variable_t* var = &ds->variableArr[ivar];
                        SOPC_FieldMetaData* fieldMetaData =
                            SOPC_PublishedDataSet_Get_FieldMetaData_At(pubDataSet, ivar);
                        SOPC_ASSERT(fieldMetaData != NULL);
                        SOPC_PubSub_ArrayDimension arrDimension = {
                            .valueRank = var->valueRank, .arrayDimensions = var->arrayDimensions.arrayDimensions};
                        allocSuccess = SOPC_FieldMetaDeta_SetCopy_ArrayDimension(fieldMetaData, &arrDimension);
                        SOPC_FieldMetaData_Set_BuiltinType(fieldMetaData, var->dataType);
                        SOPC_PublishedVariable* publishedVar = SOPC_FieldMetaData_Get_PublishedVariable(fieldMetaData);
                        SOPC_ASSERT(publishedVar != NULL);
                        SOPC_PublishedVariable_Set_NodeId(publishedVar, var->nodeId);
                        var->nodeId = NULL; // Transfer ownership to publishedVariable

                        SOPC_PublishedVariable_Set_AttributeId(publishedVar, 13); // Value => AttributeId=13
                        // SOPC_PublishedVariable_Set_IndexRange() => no indexRange
                    }
                }
            }
        }
        else
        {
            // Subscriber connection
            connection = SOPC_PubSubConfiguration_Get_SubConnection_At(config, subicon);
            SOPC_ASSERT(NULL != connection);
            subicon++;

            allocSuccess = SOPC_PubSubConnection_Allocate_ReaderGroup_Array(connection, p_connection->nb_messages);
            for (uint16_t imsg = 0; imsg < p_connection->nb_messages && allocSuccess; imsg++)
            {
                struct sopc_xml_pubsub_message_t* msg = &p_connection->messageArr[imsg];
                // Create reader group
                SOPC_ReaderGroup* readerGroup = SOPC_PubSubConnection_Get_ReaderGroup_At(connection, imsg);
                SOPC_ASSERT(readerGroup != NULL);
                SOPC_ReaderGroup_Set_SecurityMode(readerGroup, msg->security_mode);
                SOPC_ReaderGroup_Set_GroupVersion(readerGroup, msg->groupVersion);
                SOPC_ReaderGroup_Set_GroupId(readerGroup, msg->groupId);
                SOPC_ReaderGroup_Set_MqttTopic(readerGroup, msg->mqttTopic);

                if (SOPC_String_PublisherId == msg->publisher_id.type)
                {
                    SOPC_ReaderGroup_Set_PublisherId_String(readerGroup, &msg->publisher_id.data.string);
                }
                else
                {
                    SOPC_ReaderGroup_Set_PublisherId_UInteger(readerGroup, msg->publisher_id.data.uint);
                }
                SOPC_ASSERT(msg->nb_datasets < 0x100);

                allocSuccess = SOPC_ReaderGroup_Allocate_SecurityKeyServices_Array(readerGroup, msg->nb_sks);
                for (uint32_t isks = 0; isks < msg->nb_sks && allocSuccess; isks++)
                {
                    SOPC_SerializedCertificate* cert = NULL;
                    struct sopc_xml_pubsub_sks_t* xmlsks = &msg->sksArr[isks];
                    SOPC_SecurityKeyServices* sks = SOPC_ReaderGroup_Get_SecurityKeyServices_At(readerGroup, isks);
                    SOPC_ASSERT(sks != NULL);
                    allocSuccess = SOPC_SecurityKeyServices_Set_EndpointUrl(sks, xmlsks->endpointUrl);
                    SOPC_ReturnStatus status =
                        SOPC_KeyManager_SerializedCertificate_CreateFromFile(xmlsks->serverCertPath, &cert);
                    SOPC_SecurityKeyServices_Set_ServerCertificate(sks, cert);
                    allocSuccess = (allocSuccess && SOPC_STATUS_OK == status);
                }

                if (allocSuccess)
                {
                    allocSuccess =
                        SOPC_ReaderGroup_Allocate_DataSetReader_Array(readerGroup, (uint8_t) msg->nb_datasets);
                }

                for (uint8_t ids = 0; ids < msg->nb_datasets && allocSuccess; ids++)
                {
                    struct sopc_xml_pubsub_dataset_t* ds = &msg->datasetArr[ids];

                    SOPC_DataSetReader* dataSetReader = SOPC_ReaderGroup_Get_DataSetReader_At(readerGroup, ids);
                    SOPC_ASSERT(dataSetReader != NULL);
                    SOPC_DataSetReader_Set_DataSetWriterId(dataSetReader, ds->writer_id);
                    SOPC_DataSetReader_Set_ReceiveTimeout(dataSetReader, 2 * msg->publishing_interval);

                    allocSuccess = SOPC_DataSetReader_Allocate_FieldMetaData_Array(
                        dataSetReader, SOPC_TargetVariablesDataType, ds->nb_variables);

                    for (uint16_t ivar = 0; ivar < ds->nb_variables && allocSuccess; ivar++)
                    {
                        struct sopc_xml_pubsub_variable_t* var = &ds->variableArr[ivar];

                        SOPC_FieldMetaData* fieldMetaData =
                            SOPC_DataSetReader_Get_FieldMetaData_At(dataSetReader, ivar);
                        SOPC_ASSERT(fieldMetaData != NULL);

                        /* FieldMetaData: type the field */
                        SOPC_FieldMetaData_Set_BuiltinType(fieldMetaData, var->dataType);
                        SOPC_PubSub_ArrayDimension arrDimension = {
                            .valueRank = var->valueRank, .arrayDimensions = var->arrayDimensions.arrayDimensions};
                        allocSuccess = SOPC_FieldMetaDeta_SetCopy_ArrayDimension(fieldMetaData, &arrDimension);
                        /* FieldTarget: link to the source/target data */
                        SOPC_FieldTarget* fieldTarget = SOPC_FieldMetaData_Get_TargetVariable(fieldMetaData);
                        SOPC_ASSERT(fieldTarget != NULL);
                        SOPC_FieldTarget_Set_NodeId(fieldTarget, var->nodeId);
                        var->nodeId = NULL; // Transfer ownership to fieldTarget
                        // SOPC_FieldTarget_Set_SourceIndexes() => no indexRange
                        // SOPC_FieldTarget_Set_TargetIndexes() => no indexRange
                        SOPC_FieldTarget_Set_AttributeId(fieldTarget, 13); // Value => AttributeId=13
                    }
                }
            }
        }
        if (allocSuccess)
        {
            allocSuccess = SOPC_PubSubConnection_Set_Address(connection, p_connection->address);
        }
        if (allocSuccess && NULL != p_connection->interfaceName)
        {
            allocSuccess = SOPC_PubSubConnection_Set_InterfaceName(connection, p_connection->interfaceName);
        }
        if (allocSuccess && NULL != p_connection->mqttUsername)
        {
            allocSuccess = SOPC_PubSubConnection_Set_MqttUsername(connection, p_connection->mqttUsername);
        }
        if (allocSuccess && NULL != p_connection->mqttPassword)
        {
            allocSuccess = SOPC_PubSubConnection_Set_MqttPassword(connection, p_connection->mqttPassword);
        }
    }
    if (!allocSuccess)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        SOPC_PubSubConfiguration_Delete(config);
        config = NULL;
    }
    return config;
}

static void clear_xml_pubsub_config(struct parse_context_t* ctx)
{
    for (uint16_t icon = 0; icon < ctx->nb_connections; icon++)
    {
        struct sopc_xml_pubsub_connection_t* p_connection = &ctx->connectionArr[icon];

        for (uint16_t imsg = 0; imsg < p_connection->nb_messages; imsg++)
        {
            struct sopc_xml_pubsub_message_t* msg = &p_connection->messageArr[imsg];

            for (uint16_t ids = 0; ids < msg->nb_datasets; ids++)
            {
                // Fill dataset
                struct sopc_xml_pubsub_dataset_t* ds = &msg->datasetArr[ids];
                for (uint16_t ivar = 0; ivar < ds->nb_variables; ivar++)
                {
                    // Fill dataset
                    struct sopc_xml_pubsub_variable_t* var = &ds->variableArr[ivar];
                    SOPC_LocalizedText_Clear(&var->displayName);
                    SOPC_NodeId_Clear(var->nodeId);
                    SOPC_Free(var->nodeId);
                    var->nodeId = NULL;
                    SOPC_Free(var->arrayDimensions.arrayDimensions);
                    var->arrayDimensions.arrayDimensions = NULL;
                }
                SOPC_Free(ds->variableArr);
                ds->variableArr = NULL;
            }

            SOPC_Free(msg->datasetArr);
            msg->datasetArr = NULL;
            SOPC_Free(msg->mqttTopic);
            msg->mqttTopic = NULL;
            SOPC_String_Clear(&msg->publisher_id.data.string);

            for (uint16_t isks = 0; isks < msg->nb_sks; isks++)
            {
                struct sopc_xml_pubsub_sks_t* sks = &msg->sksArr[isks];
                SOPC_Free(sks->endpointUrl);
                SOPC_Free(sks->serverCertPath);
            }
            SOPC_Free(msg->sksArr);
            msg->sksArr = NULL;
            msg->nb_sks = 0;
        }

        SOPC_Free(p_connection->address);
        p_connection->address = NULL;
        SOPC_Free(p_connection->interfaceName);
        p_connection->interfaceName = NULL;
        SOPC_String_Clear(&p_connection->publisher_id.data.string);
        SOPC_Free(p_connection->mqttUsername);
        p_connection->mqttUsername = NULL;
        SOPC_Free(p_connection->mqttPassword);
        p_connection->mqttPassword = NULL;
        SOPC_Free(p_connection->messageArr);
        p_connection->messageArr = NULL;
    }

    SOPC_Free(ctx->connectionArr);
    ctx->connectionArr = NULL;
}

SOPC_PubSubConfiguration* SOPC_PubSubConfig_ParseXML(FILE* fd)
{
    XML_Parser parser = XML_ParserCreateNS(NULL, '|');

    if (NULL == parser)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        XML_ParserFree(parser);
    }

    struct parse_context_t ctx;
    memset(&ctx, 0, sizeof(struct parse_context_t));
    XML_SetUserData(parser, &ctx);

    ctx.state = PARSE_START;
    ctx.parser = parser;
    ctx.has_publisher = false;
    ctx.nb_connections = 0;
    ctx.nb_pubconnections = 0;
    ctx.nb_datasets = 0;
    ctx.nb_messages = 0;
    ctx.connectionArr = NULL;
    ctx.currentMessage = NULL;

    XML_SetElementHandler(parser, start_element_handler, end_element_handler);
    // XML_SetCharacterDataHandler(parser, char_data_handler);

    bool res = parse(parser, fd);
    XML_ParserFree(parser);

    SOPC_PubSubConfiguration* config = NULL;
    if (res)
    {
        config = build_pubsub_config(&ctx);
    }
    clear_xml_pubsub_config(&ctx);

    return config;
}
