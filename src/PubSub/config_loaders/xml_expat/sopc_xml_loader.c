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

#include <assert.h>
#include <errno.h>
#include <float.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "expat.h"

#include "sopc_builtintypes.h"
#include "sopc_helper_string.h"
#include "sopc_mem_alloc.h"

#ifdef XML_CONFIG_LOADER_LOG
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

#define LOG_MEMORY_ALLOCATION_FAILURE LOG("Memory allocation failure")

#define TAG_PUBSUB "PubSub"
#define TAG_CONNECTION "connection"
#define TAG_MESSAGE "message"
#define TAG_VARIABLE "variable"
#define TAG_SKS_SERVER "skserver"

#define ATTR_PUBSUB_ID "publisherId"

#define ATTR_CONNECTION_ADDR "address"
#define ATTR_CONNECTION_MODE "mode"
#define ATTR_CONNECTION_MODE_VAL_PUB "publisher"
#define ATTR_CONNECTION_MODE_VAL_SUB "subscriber"

#define ATTR_MESSAGE_ID "id"
#define ATTR_MESSAGE_PUBLISHING_ITV "publishingInterval"
#define ATTR_MESSAGE_VERSION "version"
#define ATTR_MESSAGE_SECURITY "securityMode"
#define ATTR_MESSAGE_SECURITY_VAL_NONE "none"
#define ATTR_MESSAGE_SECURITY_VAL_SIGN "sign"
#define ATTR_MESSAGE_SECURITY_VAL_SIGNANDENCRYPT "signAndEncrypt"
#define ATTR_MESSAGE_SECURITY_SKS_ADDR "sksAddress"

#define ATTR_MESSAGE_PUBLISHER_ID "publisherId"

#define ATTR_VARIABLE_NODE_ID "nodeId"
#define ATTR_VARIABLE_DISPLAY_NAME "displayName"
#define ATTR_VARIABLE_DATA_TYPE "dataType"

#define ATTR_SKS_ENDPOINT_URL "endpointUrl"
#define ATTR_SKS_SERVER_CERT_PATH "serverCertPath"

typedef enum
{
    PARSE_START,      // Beginning of file
    PARSE_PUBSUB,     // In a PubSub
    PARSE_CONNECTION, // In a Connection
    PARSE_MESSAGE,    // In a connection message
    PARSE_VARIABLE,   // In a message variable
    PARSE_SKS,        // In a SKS Server
} parse_state_t;

struct sopc_xml_pubsub_variable_t
{
    SOPC_NodeId* nodeId;
    SOPC_LocalizedText displayName;
    SOPC_BuiltinId dataType;
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
    uint16_t id; // WriteGroupId
    double publishing_interval;
    uint64_t publisher_id;
    uint32_t version;
    SOPC_SecurityMode_Type security_mode;
    char* sks_address;
    uint16_t nb_variables;
    struct sopc_xml_pubsub_variable_t* variableArr;

    /* Array of to define Security Key Servers (SKS) that manage the security keys for the SecurityGroup
       assigned to the PubSubGroup. Null if the SecurityMode is None. */
    uint32_t nb_sks;
    struct sopc_xml_pubsub_sks_t* sksArr;
};

struct sopc_xml_pubsub_connection_t
{
    char* address;
    bool is_publisher;
    uint16_t nb_messages;
    struct sopc_xml_pubsub_message_t* messageArr;
};

struct parse_context_t
{
    XML_Parser parser;
    bool is_publisher;
    uint64_t publisher_id; // Same config format is used for Pub/Sub but some attributes could be mandatory for one only
    uint32_t nb_connections;
    uint32_t nb_pubconnections;
    uint32_t nb_messages;
    struct sopc_xml_pubsub_connection_t* connectionArr;
    parse_state_t state;
};

static bool parse(XML_Parser parser, FILE* fd)
{
    char buf[65365];

    while (!feof(fd))
    {
        size_t r = fread(buf, sizeof(char), sizeof(buf) / sizeof(char), fd);

        if ((r == 0) && (ferror(fd) != 0))
        {
            LOGF("Error while reading input file: %s", strerror(errno));
            return false;
        }

        if (XML_Parse(parser, buf, (int) r, 0) != XML_STATUS_OK)
        {
            enum XML_Error parser_error = XML_GetErrorCode(parser);

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

static bool start_pubsub(struct parse_context_t* ctx, const XML_Char** attrs)
{
    for (size_t i = 0; attrs[i]; ++i)
    {
        const char* attr = attrs[i];

        if (strcmp(ATTR_PUBSUB_ID, attr) == 0)
        {
            const char* val = attrs[++i];

            if (NULL == val || !parse_unsigned_value(val, strlen(val), 64, &ctx->publisher_id) ||
                0 == ctx->publisher_id)
            {
                LOG_XML_ERRORF("Invalid PublisherId value in PubSub tag: '%s", val);
                return false;
            }
            ctx->is_publisher = true;
            return true;
        }
    }
    return true;
}

static bool start_connection(struct parse_context_t* ctx, const XML_Char** attrs)
{
    bool addr = false;
    bool mode = false;

    struct sopc_xml_pubsub_connection_t* connection = &ctx->connectionArr[ctx->nb_connections - 1];
    memset(connection, 0, sizeof *connection);

    for (size_t i = 0; attrs[i]; ++i)
    {
        // Current attribute name
        const char* attr = attrs[i];

        if (strcmp(ATTR_CONNECTION_ADDR, attr) == 0)
        {
            // Retrieve current attribute value
            const char* attr_val = attrs[++i];

            if (attr_val == NULL)
            {
                LOG_XML_ERROR("Missing value for connection address attribute");
                return false;
            }

            connection->address = SOPC_Malloc(strlen(attr_val) + 1);
            if (NULL == connection->address)
            {
                LOG_MEMORY_ALLOCATION_FAILURE;
                return false;
            }
            connection->address = strcpy(connection->address, attr_val);
            addr = true;
        }
        else if (strcmp(ATTR_CONNECTION_MODE, attr) == 0)
        {
            const char* attr_val = attrs[++i];

            if (strcmp(ATTR_CONNECTION_MODE_VAL_PUB, attr_val) == 0)
            {
                if (ctx->is_publisher)
                {
                    connection->is_publisher = true;
                    ctx->nb_pubconnections++;
                }
                else
                {
                    LOG_XML_ERROR("Connection mode is 'publisher' whereas no publisher id defined by root node");
                    return false;
                }
            }
            else if (strcmp(ATTR_CONNECTION_MODE_VAL_SUB, attr_val) != 0)
            {
                LOG_XML_ERRORF("Invalid connection mode: %s", attr_val);
                return false;
            }

            mode = true;
        }
        else
        {
            ++i; // Skip value of unknown attribute
        }
    }

    if (!addr)
    {
        LOG_XML_ERROR("Connection address is missing");
        return false;
    }
    else if (!mode)
    {
        LOG_XML_ERROR("Connection mode is missing");
        return false;
    }

    ctx->state = PARSE_CONNECTION;
    return true;
}

static bool start_message(struct parse_context_t* ctx, struct sopc_xml_pubsub_message_t* msg, const XML_Char** attrs)
{
    bool id = false;
    bool pubItv = false;
    bool version = false;
    bool pubId = false;

    memset(msg, 0, sizeof *msg);
    // Security is disabled if it is not configured
    msg->security_mode = SOPC_SecurityMode_None;

    for (size_t i = 0; attrs[i]; ++i)
    {
        // Current attribute name
        const char* attr = attrs[i];

        if (strcmp(ATTR_MESSAGE_ID, attr) == 0)
        {
            // Retrieve current attribute value
            const char* attr_val = attrs[++i];

            if (NULL == attr_val || !parse_unsigned_value(attr_val, strlen(attr_val), 16, &msg->id) || 0 == msg->id)
            {
                LOG_XML_ERRORF("Invalid message id value: '%s", attr_val);
                return false;
            }

            id = true;
        }
        else if (strcmp(ATTR_MESSAGE_PUBLISHING_ITV, attr) == 0)
        {
            const char* attr_val = attrs[++i];

            if (NULL == attr_val || !SOPC_strtodouble(attr_val, strlen(attr_val), 64, &msg->publishing_interval) ||
                msg->publishing_interval <= 0.)
            {
                LOG_XML_ERRORF("Invalid message publishingInterval value: '%s", attr_val);
                return false;
            }

            pubItv = true;
        }
        else if (strcmp(ATTR_MESSAGE_VERSION, attr) == 0)
        {
            const char* attr_val = attrs[++i];

            if (NULL == attr_val || !parse_unsigned_value(attr_val, strlen(attr_val), 32, &msg->version) ||
                0 == msg->version)
            {
                LOG_XML_ERRORF("Invalid message version value: '%s", attr_val);
                return false;
            }

            version = true;
        }
        else if (strcmp(ATTR_MESSAGE_SECURITY, attr) == 0)
        {
            const char* attr_val = attrs[++i];

            if (strcmp(ATTR_MESSAGE_SECURITY_VAL_NONE, attr_val) == 0)
            {
                msg->security_mode = SOPC_SecurityMode_None;
            }
            else if (strcmp(ATTR_MESSAGE_SECURITY_VAL_SIGN, attr_val) == 0)
            {
                msg->security_mode = SOPC_SecurityMode_Sign;
            }
            else if (strcmp(ATTR_MESSAGE_SECURITY_VAL_SIGNANDENCRYPT, attr_val) == 0)
            {
                msg->security_mode = SOPC_SecurityMode_SignAndEncrypt;
            }
            else
            {
                LOG_XML_ERRORF("Invalid message security mode value: '%s", attr_val);
                return false;
            }
        }
        else if (strcmp(ATTR_MESSAGE_SECURITY_SKS_ADDR, attr) == 0)
        {
            const char* attr_val = attrs[++i];

            if (attr_val == NULL)
            {
                LOG_XML_ERROR("Missing value for message SKS address attribute");
                return false;
            }

            msg->sks_address = SOPC_Malloc(strlen(attr_val) + 1);
            if (NULL == msg->sks_address)
            {
                LOG_MEMORY_ALLOCATION_FAILURE;
                return false;
            }
            msg->sks_address = strcpy(msg->sks_address, attr_val);
        }
        else if (strcmp(ATTR_MESSAGE_PUBLISHER_ID, attr) == 0)
        {
            if (ctx->connectionArr[ctx->nb_connections - 1].is_publisher)
            {
                // The publisherId is the id of the publisher defined in root node in this case (self id)
                LOG_XML_ERROR("Message tag shall not contain a publisherId if the connection is in publisher mode");
                return false;
            }

            const char* attr_val = attrs[++i];

            if (NULL == attr_val || !parse_unsigned_value(attr_val, strlen(attr_val), 64, &msg->publisher_id) ||
                0 == msg->publisher_id)
            {
                LOG_XML_ERRORF("Invalid message publishingId value: '%s", attr_val);
                return false;
            }

            pubId = true;
        }
        else
        {
            ++i; // Skip value of unknown attribute
        }
    }

    if (!id)
    {
        LOG_XML_ERROR("Message id is missing");
        return false;
    }
    else if (!pubItv)
    {
        LOG_XML_ERROR("Message publishing interval is missing");
        return false;
    }
    else if (!version)
    {
        LOG_XML_ERROR("Message version is missing");
        return false;
    }
    else if (!pubId && !ctx->connectionArr[ctx->nb_connections - 1].is_publisher)
    {
        // A subscriber connection shall provide the message publisherId source
        LOG_XML_ERROR("Message publisherId is missing in a connection in subscriber mode");
        return false;
    }
    else if (pubId && ctx->connectionArr[ctx->nb_connections - 1].is_publisher)
    {
        // A publisher connection shall NOT provide a message publisherId source (itself here)
        LOG_XML_ERROR("Message publisherId shall not be provided in a connection in publisher mode");
        return false;
    }

    ctx->state = PARSE_MESSAGE;
    return true;
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

static bool start_variable(struct parse_context_t* ctx, struct sopc_xml_pubsub_variable_t* var, const XML_Char** attrs)
{
    bool nodeId = false;
    bool dispName = false;
    bool dataType = false;

    memset(var, 0, sizeof *var);

    for (size_t i = 0; attrs[i]; ++i)
    {
        // Current attribute name
        const char* attr = attrs[i];

        if (strcmp(ATTR_VARIABLE_NODE_ID, attr) == 0)
        {
            const char* attr_val = attrs[++i];

            if (NULL == attr_val)
            {
                LOG_XML_ERROR("Missing value for variable nodeId attribute");
                return false;
            }

            assert(strlen(attr_val) <= INT32_MAX);
            var->nodeId = SOPC_NodeId_FromCString(attr_val, (int32_t) strlen(attr_val));
            if (NULL == var->nodeId)
            {
                LOG_XML_ERROR("NodeId parsing failed or memory failure");
                return false;
            }
            nodeId = true;
        }
        else if (strcmp(ATTR_VARIABLE_DISPLAY_NAME, attr) == 0)
        {
            const char* attr_val = attrs[++i];

            if (NULL == attr_val)
            {
                LOG_XML_ERROR("Missing value for variable displayName attribute");
                return false;
            }

            SOPC_LocalizedText_Initialize(&var->displayName);
            SOPC_ReturnStatus status = SOPC_String_CopyFromCString(&var->displayName.defaultText, attr_val);
            if (SOPC_STATUS_OK != status)
            {
                LOG_MEMORY_ALLOCATION_FAILURE;
                return false;
            }
            dispName = true;
        }
        else if (strcmp(ATTR_VARIABLE_DATA_TYPE, attr) == 0)
        {
            const char* attr_val = attrs[++i];

            if (NULL == attr_val)
            {
                LOG_XML_ERROR("Missing value for variable dataType attribute");
                return false;
            }

            dataType = builtintype_id_from_tag(attr_val, &var->dataType);
            if (!dataType)
            {
                LOG_XML_ERRORF("DataType attribute value not recognized as BuiltInType: '%s", attr_val);
                return false;
            }
        }
        else
        {
            ++i; // Skip value of unknown attribute
        }
    }

    if (!nodeId)
    {
        LOG_XML_ERROR("Variable nodeId is missing");
        return false;
    }
    else if (!dispName)
    {
        LOG_XML_ERROR("Variable displayName is missing");
        return false;
    }
    else if (!dataType)
    {
        LOG_XML_ERROR("Variable dataType is missing");
        return false;
    }

    ctx->state = PARSE_VARIABLE;
    return true;
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

            assert(strlen(attr_val) <= INT32_MAX);
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

            assert(strlen(attr_val) <= INT32_MAX);
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
    bool isVariable = false;
    bool isSkserver = false;

    switch (ctx->state)
    {
    case PARSE_START:
        if (strcmp(name, TAG_PUBSUB) != 0)
        {
            LOG_XML_ERRORF("Unexpected tag %s", name);
            XML_StopParser(ctx->parser, 0);
            return;
        }
        if (!start_pubsub(ctx, attrs))
        {
            XML_StopParser(ctx->parser, 0);
            return;
        }
        ctx->state = PARSE_PUBSUB;
        break;
    case PARSE_PUBSUB:
        if (strcmp(name, TAG_CONNECTION) != 0)
        {
            LOG_XML_ERRORF("Unexpected tag %s", name);
            XML_StopParser(ctx->parser, 0);
            return;
        }
        if (NULL == ctx->connectionArr)
        {
            assert(ctx->nb_connections == 0);
            ctx->connectionArr = SOPC_Malloc(sizeof *ctx->connectionArr);
            ctx->nb_connections = 1;
            assert(NULL != ctx->connectionArr);
        }
        else
        {
            assert(ctx->nb_connections > 0);
            ctx->nb_connections++;
            ctx->connectionArr =
                SOPC_Realloc(ctx->connectionArr, (size_t)(ctx->nb_connections - 1) * sizeof *ctx->connectionArr,
                             ctx->nb_connections * sizeof *ctx->connectionArr);
            assert(NULL != ctx->connectionArr);
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
            LOG_XML_ERRORF("Unexpected tag %s", name);
            XML_StopParser(ctx->parser, 0);
            return;
        }
        connection = &ctx->connectionArr[ctx->nb_connections - 1];
        if (NULL == connection->messageArr)
        {
            assert(connection->nb_messages == 0);
            connection->messageArr = SOPC_Malloc(sizeof *connection->messageArr);
            connection->nb_messages = 1;
            assert(NULL != connection->messageArr);
        }
        else
        {
            assert(connection->nb_messages > 0);
            connection->nb_messages++;
            connection->messageArr = SOPC_Realloc(
                connection->messageArr, (size_t)(connection->nb_messages - 1) * sizeof *connection->messageArr,
                connection->nb_messages * sizeof *connection->messageArr);
            assert(NULL != connection->messageArr);
        }
        if (!start_message(ctx, &connection->messageArr[connection->nb_messages - 1], attrs))
        {
            XML_StopParser(ctx->parser, 0);
            return;
        }
        break;
    case PARSE_MESSAGE:
        isVariable = (strcmp(name, TAG_VARIABLE) == 0);
        isSkserver = (strcmp(name, TAG_SKS_SERVER) == 0);
        if (!(isVariable || isSkserver))
        {
            LOG_XML_ERRORF("Unexpected tag %s", name);
            XML_StopParser(ctx->parser, 0);
            return;
        }
        connection = &ctx->connectionArr[ctx->nb_connections - 1];
        assert(NULL != connection);
        msg = &connection->messageArr[connection->nb_messages - 1];

        if (isVariable)
        {
            if (NULL == msg->variableArr)
            {
                assert(msg->nb_variables == 0);
                msg->variableArr = SOPC_Malloc(sizeof *msg->variableArr);
                msg->nb_variables = 1;
                assert(NULL != msg->variableArr);
            }
            else
            {
                assert(msg->nb_variables > 0);
                msg->nb_variables++;
                msg->variableArr =
                    SOPC_Realloc(msg->variableArr, (size_t)(msg->nb_variables - 1) * sizeof *msg->variableArr,
                                 msg->nb_variables * sizeof *msg->variableArr);
                assert(NULL != msg->variableArr);
            }
            if (!start_variable(ctx, &msg->variableArr[msg->nb_variables - 1], attrs))
            {
                XML_StopParser(ctx->parser, 0);
                return;
            }
        }
        else
        {
            assert(isSkserver);
            if (NULL == msg->sksArr)
            {
                assert(msg->nb_sks == 0);
                msg->sksArr = SOPC_Malloc(sizeof *msg->sksArr);
                msg->nb_sks = 1;
                assert(NULL != msg->sksArr);
            }
            else
            {
                assert(msg->nb_sks > 0);
                msg->nb_sks++;
                msg->sksArr = SOPC_Realloc(msg->sksArr, (size_t)(msg->nb_sks - 1) * sizeof *msg->sksArr,
                                           msg->nb_sks * sizeof *msg->sksArr);
                assert(NULL != msg->sksArr);
            }
            if (!start_sks(ctx, &msg->sksArr[msg->nb_sks - 1], attrs))
            {
                XML_StopParser(ctx->parser, 0);
                return;
            }
        }
        break;
    default:
        return;
    }
}

static void end_element_handler(void* user_data, const XML_Char* name)
{
    (void) name;
    struct parse_context_t* ctx = user_data;
    struct sopc_xml_pubsub_connection_t* connection = NULL;

    switch (ctx->state)
    {
    case PARSE_CONNECTION:
        ctx->state = PARSE_PUBSUB;
        connection = &ctx->connectionArr[ctx->nb_connections - 1];
        ctx->nb_messages += connection->nb_messages;
        break;
    case PARSE_MESSAGE:
        ctx->state = PARSE_CONNECTION;
        break;
    case PARSE_VARIABLE:
        ctx->state = PARSE_MESSAGE;
        break;
    case PARSE_SKS:
        ctx->state = PARSE_MESSAGE;
        break;
    case PARSE_PUBSUB:
        break;
    case PARSE_START:
        assert(false && "Got end_element callback when in PARSE_START state.");
        break;
    default:
        assert(false && "Unknown state.");
        break;
    }
}

static SOPC_PubSubConfiguration* build_pubsub_config(struct parse_context_t* ctx)
{
    SOPC_PubSubConfiguration* config = SOPC_PubSubConfiguration_Create();
    uint16_t nb_publishedDataSet = 0;

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

    assert(ctx->nb_messages <= UINT16_MAX);

    if (ctx->is_publisher && allocSuccess)
    {
        allocSuccess = SOPC_PubSubConfiguration_Allocate_PublishedDataSet_Array(config, (uint16_t) ctx->nb_messages);
    }

    for (uint16_t icon = 0, pubicon = 0, subicon = 0; icon < ctx->nb_connections && allocSuccess; icon++)
    {
        SOPC_PubSubConnection* connection;
        struct sopc_xml_pubsub_connection_t* p_connection = &ctx->connectionArr[icon];

        if (p_connection->is_publisher)
        {
            assert(ctx->is_publisher); // Checked on parsing

            connection = SOPC_PubSubConfiguration_Get_PubConnection_At(config, pubicon);
            assert(NULL != connection);
            pubicon++;

            // Publisher connection
            SOPC_PubSubConnection_Set_PublisherId_UInteger(connection, ctx->publisher_id);
            allocSuccess = SOPC_PubSubConnection_Allocate_WriterGroup_Array(connection, p_connection->nb_messages);

            for (uint16_t imsg = 0; imsg < p_connection->nb_messages && allocSuccess; imsg++)
            {
                struct sopc_xml_pubsub_message_t* msg = &p_connection->messageArr[imsg];

                // Create writer group
                SOPC_WriterGroup* writerGroup = SOPC_PubSubConnection_Get_WriterGroup_At(connection, imsg);
                SOPC_WriterGroup_Set_Id(writerGroup, msg->id);
                SOPC_WriterGroup_Set_PublishingInterval(writerGroup, msg->publishing_interval);
                SOPC_WriterGroup_Set_Version(writerGroup, msg->version);
                SOPC_WriterGroup_Set_SecurityMode(writerGroup, msg->security_mode);

                allocSuccess = SOPC_WriterGroup_Allocate_SecurityKeyServices_Array(writerGroup, msg->nb_sks);
                for (uint32_t isks = 0; isks < msg->nb_sks && allocSuccess; isks++)
                {
                    SOPC_SerializedCertificate* cert = NULL;
                    struct sopc_xml_pubsub_sks_t* xmlsks = &msg->sksArr[isks];
                    SOPC_SecurityKeyServices* sks = SOPC_WriterGroup_Get_SecurityKeyServices_At(writerGroup, isks);
                    assert(sks != NULL);
                    allocSuccess = SOPC_SecurityKeyServices_Set_EndpointUrl(sks, xmlsks->endpointUrl);
                    SOPC_ReturnStatus status =
                        SOPC_KeyManager_SerializedCertificate_CreateFromFile(xmlsks->serverCertPath, &cert);
                    SOPC_SecurityKeyServices_Set_ServerCertificate(sks, cert);
                    allocSuccess = (allocSuccess && SOPC_STATUS_OK == status);
                }

                // msg->publisher_id ignored if present

                SOPC_PublishedDataSet* pubDataSet;
                if (allocSuccess)
                {
                    pubDataSet = SOPC_PubSubConfiguration_Get_PublishedDataSet_At(config, nb_publishedDataSet);
                    nb_publishedDataSet++;
                    SOPC_PublishedDataSet_Init(pubDataSet, SOPC_PublishedDataItemsDataType, msg->nb_variables);

                    // Associate dataSet with writer
                    allocSuccess = SOPC_WriterGroup_Allocate_DataSetWriter_Array(writerGroup, 1);
                }
                if (allocSuccess)
                {
                    SOPC_DataSetWriter* dataSetWriter = SOPC_WriterGroup_Get_DataSetWriter_At(writerGroup, 0);
                    assert(dataSetWriter != NULL);
                    SOPC_DataSetWriter_Set_Id(dataSetWriter, msg->id); // Same as WriterGroup
                    SOPC_DataSetWriter_Set_DataSet(dataSetWriter, pubDataSet);
                }

                for (uint16_t ivar = 0; ivar < msg->nb_variables && allocSuccess; ivar++)
                {
                    // Fill dataset
                    struct sopc_xml_pubsub_variable_t* var = &msg->variableArr[ivar];
                    SOPC_FieldMetaData* fieldMetaData = SOPC_PublishedDataSet_Get_FieldMetaData_At(pubDataSet, ivar);
                    assert(fieldMetaData != NULL);
                    SOPC_FieldMetaData_Set_ValueRank(fieldMetaData, -1); // Scalar value only for now
                    SOPC_FieldMetaData_Set_BuiltinType(fieldMetaData, var->dataType);

                    SOPC_PublishedVariable* publishedVar = SOPC_FieldMetaData_Get_PublishedVariable(fieldMetaData);
                    assert(publishedVar != NULL);
                    SOPC_PublishedVariable_Set_NodeId(publishedVar, var->nodeId);
                    var->nodeId = NULL; // Transfer ownership to publishedVariable

                    SOPC_PublishedVariable_Set_AttributeId(publishedVar, 13); // Value => AttributeId=13
                    // SOPC_PublishedVariable_Set_IndexRange() => no indexRange
                }
            }
        }
        else
        {
            // Subscriber connection
            connection = SOPC_PubSubConfiguration_Get_SubConnection_At(config, subicon);
            assert(NULL != connection);
            subicon++;

            allocSuccess = SOPC_PubSubConnection_Allocate_ReaderGroup_Array(connection, p_connection->nb_messages);
            for (uint16_t imsg = 0; imsg < p_connection->nb_messages && allocSuccess; imsg++)
            {
                struct sopc_xml_pubsub_message_t* msg = &p_connection->messageArr[imsg];
                // Create writer group
                SOPC_ReaderGroup* readerGroup = SOPC_PubSubConnection_Get_ReaderGroup_At(connection, imsg);
                assert(readerGroup != NULL);
                SOPC_ReaderGroup_Set_SecurityMode(readerGroup, msg->security_mode);

                allocSuccess = SOPC_ReaderGroup_Allocate_SecurityKeyServices_Array(readerGroup, msg->nb_sks);
                for (uint32_t isks = 0; isks < msg->nb_sks && allocSuccess; isks++)
                {
                    SOPC_SerializedCertificate* cert = NULL;
                    struct sopc_xml_pubsub_sks_t* xmlsks = &msg->sksArr[isks];
                    SOPC_SecurityKeyServices* sks = SOPC_ReaderGroup_Get_SecurityKeyServices_At(readerGroup, isks);
                    assert(sks != NULL);
                    allocSuccess = SOPC_SecurityKeyServices_Set_EndpointUrl(sks, xmlsks->endpointUrl);
                    SOPC_ReturnStatus status =
                        SOPC_KeyManager_SerializedCertificate_CreateFromFile(xmlsks->serverCertPath, &cert);
                    SOPC_SecurityKeyServices_Set_ServerCertificate(sks, cert);
                    allocSuccess = (allocSuccess && SOPC_STATUS_OK == status);
                }

                if (allocSuccess)
                {
                    allocSuccess = SOPC_ReaderGroup_Allocate_DataSetReader_Array(readerGroup, 1);
                }
                if (allocSuccess)
                {
                    SOPC_DataSetReader* dataSetReader = SOPC_ReaderGroup_Get_DataSetReader_At(readerGroup, 0);
                    assert(dataSetReader != NULL);
                    SOPC_DataSetReader_Set_WriterGroupVersion(dataSetReader, msg->version);
                    SOPC_DataSetReader_Set_WriterGroupId(dataSetReader, msg->id);
                    SOPC_DataSetReader_Set_DataSetWriterId(dataSetReader, msg->id); // Same as WriterGroup
                    SOPC_DataSetReader_Set_ReceiveTimeout(dataSetReader, 2 * msg->publishing_interval);

                    SOPC_DataSetReader_Set_PublisherId_UInteger(dataSetReader, msg->publisher_id);

                    allocSuccess = SOPC_DataSetReader_Allocate_FieldMetaData_Array(
                        dataSetReader, SOPC_TargetVariablesDataType, msg->nb_variables);

                    for (uint16_t ivar = 0; ivar < msg->nb_variables && allocSuccess; ivar++)
                    {
                        struct sopc_xml_pubsub_variable_t* var = &msg->variableArr[ivar];

                        SOPC_FieldMetaData* fieldMetaData =
                            SOPC_DataSetReader_Get_FieldMetaData_At(dataSetReader, ivar);
                        assert(fieldMetaData != NULL);

                        /* FieldMetaData: type the field */
                        SOPC_FieldMetaData_Set_ValueRank(fieldMetaData, -1);
                        SOPC_FieldMetaData_Set_BuiltinType(fieldMetaData, var->dataType);

                        /* FieldTarget: link to the source/target data */
                        SOPC_FieldTarget* fieldTarget = SOPC_FieldMetaData_Get_TargetVariable(fieldMetaData);
                        assert(fieldTarget != NULL);
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

            for (uint16_t ivar = 0; ivar < msg->nb_variables; ivar++)
            {
                // Fill dataset
                struct sopc_xml_pubsub_variable_t* var = &msg->variableArr[ivar];
                SOPC_LocalizedText_Clear(&var->displayName);
                SOPC_NodeId_Clear(var->nodeId);
                SOPC_Free(var->nodeId);
                var->nodeId = NULL;
            }
            SOPC_Free(msg->variableArr);
            msg->variableArr = NULL;
            msg->nb_variables = 0;

            if (NULL != msg->sks_address)
            {
                SOPC_Free(msg->sks_address);
            }

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
    ctx.is_publisher = false;
    ctx.publisher_id = 0;
    ctx.nb_connections = 0;
    ctx.nb_pubconnections = 0;
    ctx.connectionArr = NULL;

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
