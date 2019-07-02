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

#include "loader.h"

#include <assert.h>
#include <errno.h>
#include <float.h>
#include <stdio.h>
#include <string.h>

#include <expat.h>
#include "opcua_identifiers.h"
#include "opcua_statuscodes.h"
#include "sopc_dict.h"
#include "sopc_encoder.h"
#include "sopc_hash.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"

#ifdef UANODESET_LOADER_LOG
#define LOG(str) fprintf(stderr, "UANODESET_LOADER: %s:%d: %s\n", __FILE__, __LINE__, (str))
#define LOG_XML_ERROR(str)                                                                        \
    fprintf(stderr, "UANODESET_LOADER: %s:%d: at line %lu, column %lu: %s\n", __FILE__, __LINE__, \
            XML_GetCurrentLineNumber(ctx->parser), XML_GetCurrentColumnNumber(ctx->parser), (str))

#define LOGF(format, ...) fprintf(stderr, "UANODESET_LOADER: %s:%d: " format "\n", __FILE__, __LINE__, __VA_ARGS__)
#define LOG_XML_ERRORF(format, ...)                                                                       \
    fprintf(stderr, "UANODESET_LOADER: %s:%d: at line %lu, column %lu: " format "\n", __FILE__, __LINE__, \
            XML_GetCurrentLineNumber(ctx->parser), XML_GetCurrentColumnNumber(ctx->parser), __VA_ARGS__)
#else
#define LOG(str)
#define LOG_XML_ERROR(str)
#define LOGF(format, ...)
#define LOG_XML_ERRORF(format, ...)
#endif

#define LOG_MEMORY_ALLOCATION_FAILURE LOG("Memory allocation failure")

typedef enum
{
    PARSE_START,             // Beginning of file
    PARSE_NODESET,           // In a UANodeSet
    PARSE_ALIASES,           // In an Aliases tag
    PARSE_ALIAS,             // ... in its Alias
    PARSE_NODE,              // In a UANode subtype tag
    PARSE_NODE_DISPLAYNAME,  // ... in its DisplayName
    PARSE_NODE_DESCRIPTION,  // ... in its Description
    PARSE_NODE_REFERENCES,   // ... in its References
    PARSE_NODE_REFERENCE,    // ... in its References/Reference
    PARSE_NODE_VALUE,        // In the Value tag of a UAVariable/UAVariableType tag
    PARSE_NODE_VALUE_SCALAR, // ... for a scalar type
    PARSE_NODE_VALUE_ARRAY,  // ... for an array type
} parse_state_t;

#define SKIP_TAG_LEN 256

struct parse_context_t
{
    XML_Parser parser;
    SOPC_AddressSpace* space;
    parse_state_t state;
    char skip_tag[SKIP_TAG_LEN]; // If set, start_tag events are ignored until this tag is closed

    // 0 terminated buffer for the char data handler (a single piece of char
    // data in the XML can be broken across many callbacks).
    char* char_data_buffer;

    // strlen of the text in char_data_buffer
    size_t char_data_len;

    // allocated size of char_data_buffer, at least char_data_len + 1 (for the
    // NULL terminator).
    size_t char_data_cap;

    SOPC_Dict* aliases;
    char* current_alias_alias;
    SOPC_BuiltinId current_value_type;
    SOPC_VariantArrayType current_array_type;
    SOPC_AddressSpace_Item item;
    // Temporary array to store the references.
    SOPC_Array* references;
    SOPC_Array* list_items;
};

#define NS_SEPARATOR "|"
#define UA_NODESET_NS "http://opcfoundation.org/UA/2011/03/UANodeSet.xsd"
#define UA_TYPES_NS "http://opcfoundation.org/UA/2008/02/Types.xsd"
#define NS(ns, tag) ns NS_SEPARATOR tag

static bool is_whitespace_char(char c)
{
    switch (c)
    {
    case ' ':
    case '\t':
    case '\n':
        return true;
    default:
        return false;
    }
}

static const char* strip_whitespace(char* s, size_t len)
{
    char* end = s + len - 1;
    for (; ((*s) != '\0') && is_whitespace_char(*s); ++s)
    {
    }
    for (; (end >= s) && is_whitespace_char(*end); --end)
    {
    }
    *(end + 1) = '\0';
    return s;
}

static bool ctx_char_data_append(struct parse_context_t* ctx, const char* data, size_t len)
{
    size_t required_cap = ctx->char_data_len + len + 1;

    if (required_cap > ctx->char_data_cap)
    {
        size_t cap = 2 * ctx->char_data_cap;

        while (cap < required_cap)
        {
            cap = 2 * cap;
        }

        char* dataBuff = SOPC_Realloc(ctx->char_data_buffer, ctx->char_data_cap * sizeof(char), cap * sizeof(char));

        if (dataBuff == NULL)
        {
            LOG_MEMORY_ALLOCATION_FAILURE;
            return false;
        }

        ctx->char_data_buffer = dataBuff;
        ctx->char_data_cap = cap;
    }

    memcpy(ctx->char_data_buffer + ctx->char_data_len, data, len);
    ctx->char_data_len += len;
    ctx->char_data_buffer[ctx->char_data_len] = '\0';

    return true;
}

static void ctx_char_data_reset(struct parse_context_t* ctx)
{
    ctx->char_data_buffer[0] = '\0';
    ctx->char_data_len = 0;
}

static const char* ctx_char_data_stripped(struct parse_context_t* ctx)
{
    return strip_whitespace(ctx->char_data_buffer, ctx->char_data_len);
}

static SOPC_ReturnStatus parse(XML_Parser parser, FILE* fd)
{
    char buf[65365];

    while (!feof(fd))
    {
        size_t r = fread(buf, sizeof(char), sizeof(buf) / sizeof(char), fd);

        if ((r == 0) && (ferror(fd) != 0))
        {
            LOGF("Error while reading input file: %s", strerror(errno));
            return SOPC_STATUS_NOK;
        }

        if (XML_Parse(parser, buf, (int) r, 0) != XML_STATUS_OK)
        {
            enum XML_Error parser_error = XML_GetErrorCode(parser);

            if (parser_error != XML_ERROR_NONE)
            {
                fprintf(stderr, "XML parsing failed at line %lu, column %lu. Error code is %d.\n",
                        XML_GetCurrentLineNumber(parser), XML_GetCurrentColumnNumber(parser), XML_GetErrorCode(parser));
            }

            // else, the error comes from one of the callbacks, that log an error
            // themselves.

            return SOPC_STATUS_NOK;
        }
    }

    // Tell the parser that we are at the end of the file
    if (XML_Parse(parser, "", 0, 1) != XML_STATUS_OK)
    {
        return SOPC_STATUS_NOK;
    }

    return SOPC_STATUS_OK;
}

// strdup is POSIX only...
static char* dup_c_string(const char* s)
{
    size_t len = strlen(s);
    char* res = SOPC_Calloc(1 + len, sizeof(char));

    if (res == NULL)
    {
        return NULL;
    }

    memcpy(res, s, len * sizeof(char));
    return res;
}

static bool parse_signed_value(const char* data, size_t len, uint8_t width, void* dest)
{
    char buf[21];

    if (len > (sizeof(buf) / sizeof(char) - 1))
    {
        return false;
    }

    memcpy(buf, data, len);
    buf[len] = '\0';

    char* endptr;
    int64_t val = strtol(buf, &endptr, 10);

    if (endptr != (buf + len))
    {
        return false;
    }

    if (width == 8 && val >= INT8_MIN && val <= INT8_MAX)
    {
        *((int8_t*) dest) = (int8_t) val;
        return true;
    }
    else if (width == 16 && val >= INT16_MIN && val <= INT16_MAX)
    {
        *((int16_t*) dest) = (int16_t) val;
        return true;
    }
    else if (width == 32 && val >= INT32_MIN && val <= INT32_MAX)
    {
        *((int32_t*) dest) = (int32_t) val;
        return true;
    }
    else if (width == 64)
    {
        *((int64_t*) dest) = (int64_t) val;
        return true;
    }
    else
    {
        // Invalid width and/or out of bounds value
        return false;
    }
}

static bool parse_unsigned_value(const char* data, size_t len, uint8_t width, void* dest)
{
    char buf[21];

    if (len > (sizeof(buf) / sizeof(char) - 1))
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

static bool parse_float_value(const char* data, size_t len, uint8_t width, void* dest)
{
    char buf[340];

    if (len > (sizeof(buf) / sizeof(char) - 1))
    {
        return false;
    }

    memcpy(buf, data, len);
    buf[len] = '\0';

    char* endptr;
    double val = strtod(buf, &endptr);

    if (endptr != (buf + len))
    {
        return false;
    }

    if (width == 32 && val >= -FLT_MAX && val <= FLT_MAX)
    {
        *((float*) dest) = (float) val;
        return true;
    }
    else if (width == 64 && val >= -DBL_MAX && val <= DBL_MAX)
    {
        *((double*) dest) = val;
        return true;
    }
    else
    {
        // Invalid width and/or out of bounds value
        return false;
    }
}

static bool start_alias(struct parse_context_t* ctx, const XML_Char** attrs)
{
    assert(ctx->current_alias_alias == NULL);

    for (size_t i = 0; attrs[i]; i++)
    {
        const char* attr = attrs[i];

        if (strcmp(attr, "Alias") == 0)
        {
            const char* val = attrs[++i];
            ctx->current_alias_alias = dup_c_string(val);

            if (ctx->current_alias_alias == NULL)
            {
                LOG_MEMORY_ALLOCATION_FAILURE;
                return false;
            }

            break; // We ignore other attributes so far
        }
    }

    ctx->state = PARSE_ALIAS;

    return true;
}

static bool type_id_from_name(const char* name, SOPC_BuiltinId* type_id, SOPC_VariantArrayType* array_type)
{
    static const struct
    {
        const char* name;
        SOPC_BuiltinId id;
        SOPC_VariantArrayType array_type;
    } TYPE_IDS[] = {
        {"Boolean", SOPC_Boolean_Id, SOPC_VariantArrayType_SingleValue},
        {"SByte", SOPC_SByte_Id, SOPC_VariantArrayType_SingleValue},
        {"Byte", SOPC_Byte_Id, SOPC_VariantArrayType_SingleValue},
        {"Int16", SOPC_Int16_Id, SOPC_VariantArrayType_SingleValue},
        {"UInt16", SOPC_UInt16_Id, SOPC_VariantArrayType_SingleValue},
        {"Int32", SOPC_Int32_Id, SOPC_VariantArrayType_SingleValue},
        {"UInt32", SOPC_UInt32_Id, SOPC_VariantArrayType_SingleValue},
        {"Int64", SOPC_Int64_Id, SOPC_VariantArrayType_SingleValue},
        {"UInt64", SOPC_UInt64_Id, SOPC_VariantArrayType_SingleValue},
        {"Float", SOPC_Float_Id, SOPC_VariantArrayType_SingleValue},
        {"Double", SOPC_Double_Id, SOPC_VariantArrayType_SingleValue},
        {"String", SOPC_String_Id, SOPC_VariantArrayType_SingleValue},
        {"DateTime", SOPC_DateTime_Id, SOPC_VariantArrayType_SingleValue},
        {"Guid", SOPC_Guid_Id, SOPC_VariantArrayType_SingleValue},
        {"ByteString", SOPC_ByteString_Id, SOPC_VariantArrayType_SingleValue},
        {"XmlElement", SOPC_XmlElement_Id, SOPC_VariantArrayType_SingleValue},
        {"NodeId", SOPC_NodeId_Id, SOPC_VariantArrayType_SingleValue},
        {"ExpandedNodeId", SOPC_ExpandedNodeId_Id, SOPC_VariantArrayType_SingleValue},
        {"StatusCode", SOPC_StatusCode_Id, SOPC_VariantArrayType_SingleValue},
        {"QualifiedName", SOPC_QualifiedName_Id, SOPC_VariantArrayType_SingleValue},
        {"LocalizedText", SOPC_LocalizedText_Id, SOPC_VariantArrayType_SingleValue},
        {"ExtenstionObject", SOPC_ExtensionObject_Id, SOPC_VariantArrayType_SingleValue},
        {"Structure", SOPC_ExtensionObject_Id, SOPC_VariantArrayType_SingleValue},
        {"ListOfBoolean", SOPC_Boolean_Id, SOPC_VariantArrayType_Array},
        {"ListOfSByte", SOPC_SByte_Id, SOPC_VariantArrayType_Array},
        {"ListOfByte", SOPC_Byte_Id, SOPC_VariantArrayType_Array},
        {"ListOfInt16", SOPC_Int16_Id, SOPC_VariantArrayType_Array},
        {"ListOfUInt16", SOPC_UInt16_Id, SOPC_VariantArrayType_Array},
        {"ListOfInt32", SOPC_Int32_Id, SOPC_VariantArrayType_Array},
        {"ListOfUInt32", SOPC_UInt32_Id, SOPC_VariantArrayType_Array},
        {"ListOfInt64", SOPC_Int64_Id, SOPC_VariantArrayType_Array},
        {"ListOfUInt64", SOPC_UInt64_Id, SOPC_VariantArrayType_Array},
        {"ListOfFloat", SOPC_Float_Id, SOPC_VariantArrayType_Array},
        {"ListOfDouble", SOPC_Double_Id, SOPC_VariantArrayType_Array},
        {"ListOfString", SOPC_String_Id, SOPC_VariantArrayType_Array},
        {"ListOfDateTime", SOPC_DateTime_Id, SOPC_VariantArrayType_Array},
        {"ListOfGuid", SOPC_Guid_Id, SOPC_VariantArrayType_Array},
        {"ListOfByteString", SOPC_ByteString_Id, SOPC_VariantArrayType_Array},
        {"ListOfXmlElement", SOPC_XmlElement_Id, SOPC_VariantArrayType_Array},

        {NULL, SOPC_Null_Id, SOPC_VariantArrayType_SingleValue},
    };

    for (size_t i = 0; TYPE_IDS[i].name != NULL; ++i)
    {
        if (strcmp(name, TYPE_IDS[i].name) == 0)
        {
            *type_id = TYPE_IDS[i].id;
            *array_type = TYPE_IDS[i].array_type;
            return true;
        }
    }

    return false;
}

static const struct
{
    const char* name;
    uint32_t id;
} ELEMENT_TYPES[] = {
    {"UADataType", OpcUa_NodeClass_DataType},
    {"UAMethod", OpcUa_NodeClass_Method},
    {"UAObject", OpcUa_NodeClass_Object},
    {"UAObjectType", OpcUa_NodeClass_ObjectType},
    {"UAReferenceType", OpcUa_NodeClass_ReferenceType},
    {"UAVariable", OpcUa_NodeClass_Variable},
    {"UAVariableType", OpcUa_NodeClass_VariableType},
    {"UAView", OpcUa_NodeClass_View},
    {NULL, 0},
};

static const char* tag_from_element_id(const uint32_t id)
{
    for (size_t i = 0; ELEMENT_TYPES[i].name != NULL; ++i)
    {
        if (id == ELEMENT_TYPES[i].id)
        {
            return ELEMENT_TYPES[i].name;
        }
    }

    return NULL;
}

static bool start_node(struct parse_context_t* ctx, uint32_t element_type, const XML_Char** attrs)
{
    assert(ctx->item.node_class == 0);

    SOPC_AddressSpace_Item_Initialize(&ctx->item, element_type);
    // Note: value_status default value set on NodeId parsing

    for (size_t i = 0; attrs[i]; ++i)
    {
        const char* attr = attrs[i];

        if (strcmp("NodeId", attr) == 0)
        {
            const char* attr_val = attrs[++i];

            if (attr_val == NULL)
            {
                LOG_XML_ERROR("Missing value for NodeId attribute");
                return false;
            }

            SOPC_NodeId* id = SOPC_NodeId_FromCString(attr_val, (int32_t) strlen(attr_val));

            if (id == NULL)
            {
                LOG_XML_ERRORF("Invalid variable NodeId: %s", attr_val);
                return false;
            }

            // Set value_status default value:
            // Keep OPC UA default namespace nodes with a Good status,
            // necessary to pass UACTT otherwise keep Good status only if a value is defined
            ctx->item.value_status = id->Namespace == 0 ? SOPC_GoodGenericStatus : OpcUa_UncertainInitialValue;

            SOPC_NodeId* element_id = SOPC_AddressSpace_Item_Get_NodeId(&ctx->item);
            SOPC_ReturnStatus status = SOPC_NodeId_Copy(element_id, id);
            SOPC_NodeId_Clear(id);
            SOPC_Free(id);

            if (status != SOPC_STATUS_OK)
            {
                LOG_MEMORY_ALLOCATION_FAILURE;
                return false;
            }
        }
        else if (strcmp("BrowseName", attr) == 0)
        {
            const char* attr_val = attrs[++i];

            SOPC_QualifiedName* element_browse_name = SOPC_AddressSpace_Item_Get_BrowseName(&ctx->item);
            SOPC_QualifiedName_Initialize(element_browse_name);
            SOPC_ReturnStatus status = SOPC_QualifiedName_ParseCString(element_browse_name, attr_val);

            if (status != SOPC_STATUS_OK)
            {
                LOG_XML_ERRORF("Invalid browse name: %s", attr_val);
                return false;
            }
        }
        else if (strcmp("DataType", attr) == 0)
        {
            if (OpcUa_NodeClass_Variable != element_type && OpcUa_NodeClass_VariableType != element_type)
            {
                LOG_XML_ERRORF("Unexpected DataType attribute (value '%s') on node of class = %s", attrs[++i],
                               tag_from_element_id(element_type));
                return false;
            }

            const char* attr_val = attrs[++i];

            if (attr_val == NULL)
            {
                LOG_XML_ERROR("Missing value for DataType attribute");
                return false;
            }

            bool is_aliased;
            const char* aliased = SOPC_Dict_Get(ctx->aliases, attr_val, &is_aliased);

            if (is_aliased)
            {
                attr_val = aliased;
            }

            // Attempt to parse a NodeId as a string
            SOPC_NodeId* id = SOPC_NodeId_FromCString(attr_val, (int32_t) strlen(attr_val));

            if (id == NULL)
            {
                LOG_XML_ERRORF("Invalid variable NodeId: %s", attr_val);
                return false;
            }

            SOPC_NodeId* dataType = SOPC_AddressSpace_Item_Get_DataType(&ctx->item);
            SOPC_ReturnStatus status = SOPC_NodeId_Copy(dataType, id);
            SOPC_NodeId_Clear(id);
            SOPC_Free(id);

            if (status != SOPC_STATUS_OK)
            {
                LOG_MEMORY_ALLOCATION_FAILURE;
                return false;
            }
        }
        else if (strcmp("ValueRank", attr) == 0)
        {
            if (OpcUa_NodeClass_Variable != element_type && OpcUa_NodeClass_VariableType != element_type)
            {
                LOG_XML_ERRORF("Unexpected ValueRank attribute (value '%s') on node of class = %s", attrs[++i],
                               tag_from_element_id(element_type));
                return false;
            }

            const char* attr_val = attrs[++i];

            if (attr_val == NULL)
            {
                LOG_XML_ERROR("Missing value for ValueRank attribute");
                return false;
            }

            int32_t parsedValueRank;
            bool result = parse_signed_value(attr_val, (size_t) strlen(attr_val), 32, &parsedValueRank);

            if (!result)
            {
                LOG_XML_ERROR("Incorrect value for ValueRank attribute");
                return false;
            }

            int32_t* valueRank = SOPC_AddressSpace_Item_Get_ValueRank(&ctx->item);
            *valueRank = parsedValueRank;
        }
        else if (strcmp("AccessLevel", attr) == 0)
        {
            assert(OpcUa_NodeClass_Variable == element_type);
            if (OpcUa_NodeClass_Variable != element_type)
            {
                LOG_XML_ERRORF("Unexpected AccessLevel attribute (value '%s') on node of class = %s", attrs[++i],
                               tag_from_element_id(element_type));
                return false;
            }

            const char* attr_val = attrs[++i];

            if (!parse_unsigned_value(attr_val, strlen(attr_val), 8, &ctx->item.data.variable.AccessLevel))
            {
                LOG_XML_ERRORF("Invalid AccessLevel on node value: '%s", attr_val);
                return false;
            }
        }
        else
        {
            ++i; // Skip value of unknown attribute
        }
    }

    ctx->state = PARSE_NODE;

    return true;
}

static bool start_node_reference(struct parse_context_t* ctx, const XML_Char** attrs)
{
    OpcUa_ReferenceNode ref;
    OpcUa_ReferenceNode_Initialize(&ref);

    for (size_t i = 0; attrs[i]; ++i)
    {
        const char* attr = attrs[i];

        if (strcmp("ReferenceType", attr) == 0)
        {
            const char* val = attrs[++i];

            bool is_aliased;
            const char* aliased = SOPC_Dict_Get(ctx->aliases, val, &is_aliased);

            if (is_aliased)
            {
                val = aliased;
            }

            SOPC_NodeId* nodeid = SOPC_NodeId_FromCString(val, (int32_t) strlen(val));

            if (nodeid == NULL)
            {
                LOG_XML_ERRORF("Error while parsing ReferenceType '%s' into a NodeId\n.", val);
                return false;
            }

            SOPC_ReturnStatus status = SOPC_NodeId_Copy(&ref.ReferenceTypeId, nodeid);
            SOPC_NodeId_Clear(nodeid);
            SOPC_Free(nodeid);

            if (status != SOPC_STATUS_OK)
            {
                LOG_MEMORY_ALLOCATION_FAILURE;
                return false;
            }
        }
        else if (strcmp("IsForward", attr) == 0)
        {
            const char* val = attrs[++i];
            ref.IsInverse = (strcmp(val, "true") != 0);
        }
    }

    if (ctx->references == NULL)
    {
        ctx->references = SOPC_Array_Create(sizeof(OpcUa_ReferenceNode), 1, OpcUa_ReferenceNode_Clear);

        if (ctx->references == NULL)
        {
            OpcUa_ReferenceNode_Clear(&ref);
            LOG_MEMORY_ALLOCATION_FAILURE;
            return false;
        }
    }

    // Should not fail since we reserved space for one element above
    bool append = SOPC_Array_Append(ctx->references, ref);
    assert(append);

    ctx->state = PARSE_NODE_REFERENCE;

    return true;
}

static bool start_node_value_array(struct parse_context_t* ctx)
{
    assert(ctx->current_array_type == SOPC_VariantArrayType_Array);
    assert(ctx->list_items == NULL);

    ctx->list_items = SOPC_Array_Create(sizeof(SOPC_Variant), 0, (SOPC_Array_Free_Func) SOPC_Variant_ClearAux);

    if (ctx->list_items == NULL)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }

    return true;
}

static void skip_tag(struct parse_context_t* ctx, const char* name)
{
    assert(ctx->skip_tag[0] == 0);
    assert(strlen(name) < SKIP_TAG_LEN);
    strncpy(ctx->skip_tag, name, SKIP_TAG_LEN - 1);
}

static bool current_element_has_value(struct parse_context_t* ctx)
{
    switch (ctx->item.node_class)
    {
    case OpcUa_NodeClass_Variable:
    case OpcUa_NodeClass_VariableType:
        return true;
    default:
        return false;
    }
}

static bool type_id_from_tag(const char* tag, SOPC_BuiltinId* type_id, SOPC_VariantArrayType* array_type)
{
    // tag should have the correct namespace
    if (strncmp(tag, UA_TYPES_NS NS_SEPARATOR, strlen(UA_TYPES_NS NS_SEPARATOR)) != 0)
    {
        return false;
    }

    const char* name = tag + strlen(UA_TYPES_NS NS_SEPARATOR);

    return type_id_from_name(name, type_id, array_type);
}

static uint32_t element_id_from_tag(const char* tag)
{
    if (strncmp(tag, UA_NODESET_NS NS_SEPARATOR, strlen(UA_NODESET_NS NS_SEPARATOR)) != 0)
    {
        return 0;
    }

    const char* name = tag + strlen(UA_NODESET_NS NS_SEPARATOR);

    for (size_t i = 0; ELEMENT_TYPES[i].name != NULL; ++i)
    {
        if (strcmp(name, ELEMENT_TYPES[i].name) == 0)
        {
            return ELEMENT_TYPES[i].id;
        }
    }

    return 0;
}

static void start_element_handler(void* user_data, const XML_Char* name, const XML_Char** attrs)
{
    struct parse_context_t* ctx = user_data;

    if (ctx->skip_tag[0] != 0)
    {
        return; // We're skipping until the end of a tag
    }

    switch (ctx->state)
    {
    case PARSE_START:
        if (strcmp(name, NS(UA_NODESET_NS, "UANodeSet")) != 0)
        {
            LOG_XML_ERRORF("Unexpected tag %s", name);
            XML_StopParser(ctx->parser, 0);
            return;
        }

        ctx->state = PARSE_NODESET;
        return;
    case PARSE_NODESET:
    {
        uint32_t element_type = element_id_from_tag(name);

        if (element_type > 0)
        {
            if (!start_node(ctx, element_type, attrs))
            {
                XML_StopParser(ctx->parser, 0);
                return;
            }
        }
        else if (strcmp(NS(UA_NODESET_NS, "Aliases"), name) == 0)
        {
            ctx->state = PARSE_ALIASES;
        }
        else
        {
            skip_tag(ctx, name);
        }
        break;
    }
    case PARSE_NODE:
        if (strcmp(NS(UA_NODESET_NS, "DisplayName"), name) == 0)
        {
            ctx->state = PARSE_NODE_DISPLAYNAME;
        }
        else if (strcmp(NS(UA_NODESET_NS, "Description"), name) == 0)
        {
            ctx->state = PARSE_NODE_DESCRIPTION;
        }
        else if (strcmp(NS(UA_NODESET_NS, "References"), name) == 0)
        {
            ctx->state = PARSE_NODE_REFERENCES;
        }
        else if (current_element_has_value(ctx) && strcmp(NS(UA_NODESET_NS, "Value"), name) == 0)
        {
            ctx->state = PARSE_NODE_VALUE;
        }
        else
        {
            skip_tag(ctx, name);
        }
        break;
    case PARSE_NODE_REFERENCES:
        if (strcmp(NS(UA_NODESET_NS, "Reference"), name) == 0)
        {
            if (!start_node_reference(ctx, attrs))
            {
                XML_StopParser(ctx->parser, 0);
                return;
            }
        }
        else
        {
            skip_tag(ctx, name);
        }
        break;
    case PARSE_ALIASES:
        if (strcmp(NS(UA_NODESET_NS, "Alias"), name) == 0)
        {
            if (!start_alias(ctx, attrs))
            {
                XML_StopParser(ctx->parser, 0);
                return;
            }
        }
        else
        {
            skip_tag(ctx, name);
        }
        break;
    case PARSE_NODE_VALUE:
    {
        assert(current_element_has_value(ctx));

        SOPC_BuiltinId type_id;
        SOPC_VariantArrayType array_type;

        if (!type_id_from_tag(name, &type_id, &array_type))
        {
            LOG_XML_ERRORF("Unsupported value type: %s", name);
            skip_tag(ctx, name);
            return;
        }

        assert(ctx->current_value_type == SOPC_Null_Id);
        ctx->current_value_type = type_id;
        ctx->current_array_type = array_type;

        if (array_type == SOPC_VariantArrayType_Array && !start_node_value_array(ctx))
        {
            XML_StopParser(ctx->parser, false);
            return;
        }

        ctx->state =
            (array_type == SOPC_VariantArrayType_SingleValue) ? PARSE_NODE_VALUE_SCALAR : PARSE_NODE_VALUE_ARRAY;
        break;
    }
    case PARSE_NODE_VALUE_SCALAR:
        LOG_XML_ERROR("Unexpected tag while parsing scalar value");
        XML_StopParser(ctx->parser, false);
        return;
    case PARSE_NODE_VALUE_ARRAY:
    {
        assert(current_element_has_value(ctx));
        assert(ctx->current_array_type == SOPC_VariantArrayType_Array);

        SOPC_BuiltinId type_id;
        SOPC_VariantArrayType array_type;

        if (!type_id_from_tag(name, &type_id, &array_type))
        {
            LOG_XML_ERRORF("Unsupported value type: %s", name);
            skip_tag(ctx, name);
            return;
        }

        if (type_id != ctx->current_value_type)
        {
            LOG_XML_ERRORF("Array value of type %s does not match array type", name);
            skip_tag(ctx, name);
            return;
        }

        if (array_type != SOPC_VariantArrayType_SingleValue)
        {
            LOG_XML_ERROR("Arrays cannot be nested");
            skip_tag(ctx, name);
            return;
        }

        ctx->state = PARSE_NODE_VALUE_SCALAR;
        break;
    }
    default:
        return;
    }
}

static bool finalize_alias(struct parse_context_t* ctx)
{
    if (ctx->current_alias_alias == NULL)
    {
        LOG_XML_ERROR("Missing Alias attribute on Alias.");
        return false;
    }

    char* target = dup_c_string(ctx_char_data_stripped(ctx));
    ctx_char_data_reset(ctx);

    if (target == NULL || !SOPC_Dict_Insert(ctx->aliases, ctx->current_alias_alias, target))
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        SOPC_Free(target);
        return false;
    }

    ctx->current_alias_alias = NULL;

    return true;
}

static bool finalize_reference(struct parse_context_t* ctx)
{
    size_t n_refs = SOPC_Array_Size(ctx->references);
    assert(n_refs > 0);

    OpcUa_ReferenceNode* ref = SOPC_Array_Get_Ptr(ctx->references, n_refs - 1);
    const char* text = ctx_char_data_stripped(ctx);
    SOPC_NodeId* target_id = SOPC_NodeId_FromCString(text, (int32_t) strlen(text));

    if (target_id == NULL)
    {
        LOG_XML_ERRORF("Cannot parse reference target '%s' into a NodeId.", text);
        return false;
    }

    ctx_char_data_reset(ctx);

    SOPC_ReturnStatus status = SOPC_NodeId_Copy(&ref->TargetId.NodeId, target_id);
    SOPC_NodeId_Clear(target_id);
    SOPC_Free(target_id);

    return status == SOPC_STATUS_OK;
}

static SOPC_LocalizedText* element_localized_text_for_state(struct parse_context_t* ctx)
{
    switch (ctx->state)
    {
    case PARSE_NODE_DISPLAYNAME:
        return SOPC_AddressSpace_Item_Get_DisplayName(&ctx->item);
    case PARSE_NODE_DESCRIPTION:
        return SOPC_AddressSpace_Item_Get_Description(&ctx->item);
    default:
        assert(false && "Unexpected state");
    }
}

static uint8_t type_width(SOPC_BuiltinId ty)
{
    switch (ty)
    {
    case SOPC_SByte_Id:
    case SOPC_Byte_Id:
        return 8;
    case SOPC_Int16_Id:
    case SOPC_UInt16_Id:
        return 16;
    case SOPC_Int32_Id:
    case SOPC_UInt32_Id:
        return 32;
    case SOPC_Int64_Id:
    case SOPC_UInt64_Id:
        return 64;
    default:
        assert(false && "Non numeric type");
    }
}

#define FOREACH_SIGNED_VALUE_TYPE(x) \
    x(SOPC_SByte_Id, Sbyte) x(SOPC_Int16_Id, Int16) x(SOPC_Int32_Id, Int32) x(SOPC_Int64_Id, Int64)

#define FOREACH_UNSIGNED_VALUE_TYPE(x) \
    x(SOPC_Byte_Id, Byte) x(SOPC_UInt16_Id, Uint16) x(SOPC_UInt32_Id, Uint32) x(SOPC_UInt64_Id, Uint64)

#define SET_INT_ELEMENT_VALUE_CASE(id, field, signed_or_unsigned)                                    \
    case id:                                                                                         \
        if (parse_##signed_or_unsigned##_value(val, strlen(val), type_width(id), &var->Value.field)) \
        {                                                                                            \
            var->BuiltInTypeId = id;                                                                 \
            return true;                                                                             \
        }                                                                                            \
        else                                                                                         \
        {                                                                                            \
            LOGF("Invalid integer value: '%s'", val);                                                \
            return false;                                                                            \
        }

#define SET_SIGNED_INT_ELEMENT_VALUE_CASE(id, field) SET_INT_ELEMENT_VALUE_CASE(id, field, signed)
#define SET_UNSIGNED_INT_ELEMENT_VALUE_CASE(id, field) SET_INT_ELEMENT_VALUE_CASE(id, field, unsigned)

#define SET_STR_ELEMENT_VALUE_CASE(field)                                      \
    if (SOPC_String_CopyFromCString(&var->Value.field, val) == SOPC_STATUS_OK) \
    {                                                                          \
        var->BuiltInTypeId = type_id;                                          \
        return true;                                                           \
    }                                                                          \
    else                                                                       \
    {                                                                          \
        LOG_MEMORY_ALLOCATION_FAILURE;                                         \
        return false;                                                          \
    }

/* Using https://en.wikibooks.org/wiki/Algorithm_Implementation/Miscellaneous/Base64#C_2
 * to decode base64 */
#define WHITESPACE 64
#define EQUALS 65
#define INVALID 66

static const unsigned char d[] = {
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, WHITESPACE, INVALID,
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,    INVALID,
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,    INVALID,
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, 62,      INVALID, INVALID, INVALID,    63,
    52,      53,      54,      55,      56,      57,      58,      59,      60,      61,      INVALID,    INVALID,
    INVALID, EQUALS,  INVALID, INVALID, INVALID, 0,       1,       2,       3,       4,       5,          6,
    7,       8,       9,       10,      11,      12,      13,      14,      15,      16,      17,         18,
    19,      20,      21,      22,      23,      24,      25,      INVALID, INVALID, INVALID, INVALID,    INVALID,
    INVALID, 26,      27,      28,      29,      30,      31,      32,      33,      34,      35,         36,
    37,      38,      39,      40,      41,      42,      43,      44,      45,      46,      47,         48,
    49,      50,      51,      INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,    INVALID,
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,    INVALID,
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,    INVALID,
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,    INVALID,
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,    INVALID,
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,    INVALID,
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,    INVALID,
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,    INVALID,
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,    INVALID,
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,    INVALID,
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,    INVALID,
    INVALID, INVALID, INVALID, INVALID};

/* This function decodes a base64 ByteString. Base64 ByteString shall be null terminated.
 * Otherwise, the result is undefined.*/
static bool base64decode(const char* input, unsigned char* out, size_t* outLen)
{
    if (NULL == input || NULL == out || NULL == outLen)
    {
        return false;
    }

    const char* end = input + strlen(input);
    char iter = 0;
    uint32_t buf = 0;
    size_t len = 0;
    bool return_status = true;

    while (return_status && input < end)
    {
        unsigned char c = d[(int) *input];
        input++;

        switch (c)
        {
        case WHITESPACE:
            break; /* skip whitespace */
        case INVALID:
            return_status = false; /* invalid input, return error */
            break;
        case EQUALS: /* pad character, end of data */
            input = end;
            break;
        default:
            assert(c < 64);
            buf = buf << 6 | c;
            iter++; // increment the number of iteration
            /* If the buffer is full, split it into bytes */
            if (iter == 4)
            {
                len += 3;
                if (len > *outLen)
                {
                    return_status = false; /* buffer overflow */
                }
                else
                {
                    *(out++) = (buf >> 16) & 255;
                    *(out++) = (buf >> 8) & 255;
                    *(out++) = buf & 255;
                    buf = 0;
                    iter = 0;
                }
            }
            break;
        }
    }

    if (return_status && iter == 3)
    {
        if ((len += 2) > *outLen)
        {
            return_status = false; /* buffer overflow */
        }
        else
        {
            *(out++) = (buf >> 10) & 255;
            *(out++) = (buf >> 2) & 255;
        }
    }
    else if (return_status && iter == 2)
    {
        if (++len > *outLen)
        {
            return_status = false; /* buffer overflow */
        }
        else
        {
            *(out++) = (buf >> 4) & 255;
        }
    }

    if (return_status)
    {
        *outLen = len; /* modify to reflect the actual output size */
    }

    return return_status;
}

/* This function computes the variant corresponding to a base64 ByteString.
 * Base64 ByteString shall be null terminated. Otherwise, the result is undefined. */
static bool set_variant_value_bstring(SOPC_Variant* var, const char* bstring_str)
{
    size_t length = strlen(bstring_str);
    SOPC_ReturnStatus status;
    bool return_code;

    /* By definition, ByteString base64 length is greater than its corresponding string length */
    unsigned char* str = SOPC_Calloc(1, length);

    return_code = base64decode(bstring_str, str, &length);
    assert(true == return_code);

    status = SOPC_String_CopyFromCString(&var->Value.Bstring, (char*) str);
    SOPC_Free(str);

    if (status == SOPC_STATUS_OK)
    {
        var->BuiltInTypeId = SOPC_ByteString_Id;
        return true;
    }
    else
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }

    return true;
}

static bool set_variant_value_guid(SOPC_Variant* var, const char* guid_str)
{
    SOPC_Guid* guid = SOPC_Calloc(1, sizeof(SOPC_Guid));

    if (guid == NULL)
    {
        return false;
    }

    SOPC_Guid_Initialize(guid);

    if (SOPC_Guid_FromCString(guid, guid_str, strlen(guid_str)) != SOPC_STATUS_OK)
    {
        LOGF("Invalid GUID: '%s'", guid_str);
        SOPC_Guid_Clear(guid);
        SOPC_Free(guid);
        return false;
    }

    var->BuiltInTypeId = SOPC_Guid_Id;
    var->Value.Guid = guid;
    return true;
}

static bool set_variant_value_qname(SOPC_Variant* var, const char* qname_str)
{
    SOPC_QualifiedName* qname = SOPC_Calloc(1, sizeof(SOPC_QualifiedName));

    if (qname == NULL)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        SOPC_Free(qname);
        return false;
    }

    SOPC_QualifiedName_Initialize(qname);
    SOPC_ReturnStatus status = SOPC_QualifiedName_ParseCString(qname, qname_str);

    if (status == SOPC_STATUS_OK)
    {
        var->BuiltInTypeId = SOPC_QualifiedName_Id;
        var->Value.Qname = qname;
        return true;
    }
    else
    {
        LOGF("Invalid qualified name: '%s'", qname_str);
        SOPC_QualifiedName_Clear(qname);
        SOPC_Free(qname);
        return false;
    }
}

static bool set_variant_value_localized_text(SOPC_Variant* var, const char* text)
{
    SOPC_LocalizedText* lt = SOPC_Calloc(1, sizeof(SOPC_LocalizedText));

    if (lt == NULL)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        SOPC_Free(lt);
        return false;
    }

    SOPC_LocalizedText_Initialize(lt);

    if (SOPC_String_CopyFromCString(&lt->Text, text) != SOPC_STATUS_OK)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        SOPC_LocalizedText_Clear(lt);
        SOPC_Free(lt);
        return false;
    }
    else
    {
        var->BuiltInTypeId = SOPC_LocalizedText_Id;
        var->Value.LocalizedText = lt;
        return true;
    }
}

static bool set_variant_value_nodeid(SOPC_Variant* var, const char* id)
{
    size_t len = strlen(id);
    assert(len <= INT32_MAX);

    var->Value.NodeId = SOPC_NodeId_FromCString(id, (int32_t) len);

    if (var->Value.NodeId == NULL)
    {
        LOGF("Invalid NodeId: '%s'", id);
        return false;
    }

    var->BuiltInTypeId = SOPC_NodeId_Id;
    return true;
}

static bool set_variant_value(SOPC_Variant* var, SOPC_BuiltinId type_id, const char* val)
{
    switch (type_id)
    {
    case SOPC_Boolean_Id:
        var->BuiltInTypeId = SOPC_Boolean_Id;
        var->Value.Boolean = (strcmp(val, "true") == 0);
        return true;
        FOREACH_SIGNED_VALUE_TYPE(SET_SIGNED_INT_ELEMENT_VALUE_CASE)
        FOREACH_UNSIGNED_VALUE_TYPE(SET_UNSIGNED_INT_ELEMENT_VALUE_CASE)
    case SOPC_Float_Id:
        if (parse_float_value(val, strlen(val), 32, &var->Value.Floatv))
        {
            var->BuiltInTypeId = SOPC_Float_Id;
            return true;
        }
        else
        {
            LOGF("Invalid float value: '%s'", val);
            return false;
        }
    case SOPC_Double_Id:
        if (parse_float_value(val, strlen(val), 64, &var->Value.Doublev))
        {
            var->BuiltInTypeId = SOPC_Double_Id;
            return true;
        }
        else
        {
            LOGF("Invalid double value: '%s'", val);
            return false;
        }
    case SOPC_String_Id:
        SET_STR_ELEMENT_VALUE_CASE(String)
    case SOPC_ByteString_Id:
        return set_variant_value_bstring(var, val);
    case SOPC_XmlElement_Id:
        SET_STR_ELEMENT_VALUE_CASE(XmlElt)
    case SOPC_Guid_Id:
        return set_variant_value_guid(var, val);
    case SOPC_NodeId_Id:
        return set_variant_value_nodeid(var, val);
    case SOPC_QualifiedName_Id:
        return set_variant_value_qname(var, val);
    case SOPC_LocalizedText_Id:
        return set_variant_value_localized_text(var, val);
    default:
        assert(false && "Cannot parse current value type.");
    }
}

static bool set_element_value_scalar(struct parse_context_t* ctx)
{
    assert(ctx->current_array_type == SOPC_VariantArrayType_SingleValue);

    SOPC_Variant* var = SOPC_AddressSpace_Item_Get_Value(&ctx->item);
    bool ok = set_variant_value(var, ctx->current_value_type, ctx_char_data_stripped(ctx));
    ctx_char_data_reset(ctx);

    if (ok)
    {
        ctx->item.value_status = SOPC_GoodGenericStatus;
    }

    return ok;
}

static bool append_element_value(struct parse_context_t* ctx)
{
    assert(ctx->current_array_type == SOPC_VariantArrayType_Array);

    SOPC_Variant* var = SOPC_Variant_Create();

    if (NULL == var)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }

    bool ok = set_variant_value(var, ctx->current_value_type, ctx_char_data_stripped(ctx));
    ctx_char_data_reset(ctx);

    if (!ok)
    {
        SOPC_Variant_Delete(var);
        return false;
    }

    bool appended = SOPC_Array_Append(ctx->list_items, *var);

    if (!appended)
    {
        SOPC_Variant_Delete(var);
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }
    // The structure content was copied in array but structure itself shall be freed
    SOPC_Free(var);

    return true;
}

static bool SOPC_Array_Of_Variant_Into_Variant_Array(SOPC_Array* var_arr, SOPC_BuiltinId builtInId, SOPC_Variant* var)
{
    size_t length = SOPC_Array_Size(var_arr);

    assert(length <= INT32_MAX);
    bool res = SOPC_Variant_Initialize_Array(var, builtInId, (int32_t) length);

    if (!res)
    {
        return false;
    }

    /* Check variant array is */
    for (size_t i = 0; i < length && res; i++)
    {
        SOPC_Variant* lvar = SOPC_Array_Get_Ptr(var_arr, i);
        assert(builtInId == lvar->BuiltInTypeId);
        const void* value = SOPC_Variant_Get_SingleValue(lvar, builtInId);
        res = SOPC_Variant_CopyInto_ArrayValueAt(var, builtInId, (int32_t) i, value);
    }

    if (!res)
    {
        SOPC_Variant_Clear(var);
    }

    return res;
}

static bool set_element_value_array(struct parse_context_t* ctx)
{
    assert(ctx->current_array_type == SOPC_VariantArrayType_Array);
    assert(ctx->list_items != NULL);

    SOPC_Variant* var = SOPC_AddressSpace_Item_Get_Value(&ctx->item);

    bool res = SOPC_Array_Of_Variant_Into_Variant_Array(ctx->list_items, ctx->current_value_type, var);

    SOPC_Array_Delete(ctx->list_items);
    ctx->list_items = NULL;
    ctx->item.value_status = SOPC_GoodGenericStatus;

    return res;
}

static bool finalize_node(struct parse_context_t* ctx)
{
    if (ctx->references != NULL)
    {
        size_t n_references = SOPC_Array_Size(ctx->references);
        assert(n_references <= INT32_MAX);

        *SOPC_AddressSpace_Item_Get_NoOfReferences(&ctx->item) = (int32_t) n_references;
        *SOPC_AddressSpace_Item_Get_References(&ctx->item) = SOPC_Array_Into_Raw(ctx->references);
        ctx->references = NULL;
    }

    SOPC_AddressSpace_Item* item = SOPC_Calloc(1, sizeof(SOPC_AddressSpace_Item));

    if (item == NULL)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }

    memcpy(item, &ctx->item, sizeof(SOPC_AddressSpace_Item));

    if (SOPC_AddressSpace_Append(ctx->space, item) == SOPC_STATUS_OK)
    {
        return true;
    }
    else
    {
        SOPC_Free(item);
        return false;
    }
}

static void end_element_handler(void* user_data, const XML_Char* name)
{
    struct parse_context_t* ctx = user_data;

    if ((ctx->skip_tag[0] != 0) && (strcmp(ctx->skip_tag, name) == 0))
    {
        ctx->skip_tag[0] = 0;
        return;
    }

    switch (ctx->state)
    {
    case PARSE_ALIASES:
        ctx->state = PARSE_NODESET;
        break;
    case PARSE_ALIAS:
    {
        bool ok = finalize_alias(ctx);

        SOPC_Free(ctx->current_alias_alias);
        ctx->current_alias_alias = NULL;

        if (!ok)
        {
            XML_StopParser(ctx->parser, false);
            return;
        }

        ctx->state = PARSE_ALIASES;
        break;
    }
    case PARSE_NODE_DISPLAYNAME:
    case PARSE_NODE_DESCRIPTION:
    {
        SOPC_LocalizedText* lt = element_localized_text_for_state(ctx);
        SOPC_String_Clear(&lt->Text);
        const char* stripped = ctx_char_data_stripped(ctx);
        SOPC_ReturnStatus status =
            (strlen(stripped) == 0) ? SOPC_STATUS_OK : SOPC_String_CopyFromCString(&lt->Text, stripped);
        ctx_char_data_reset(ctx);

        if (status != SOPC_STATUS_OK)
        {
            LOG_MEMORY_ALLOCATION_FAILURE;
            XML_StopParser(ctx->parser, false);
            return;
        }

        ctx->state = PARSE_NODE;
        break;
    }
    case PARSE_NODE_REFERENCES:
        ctx->state = PARSE_NODE;
        break;
    case PARSE_NODE_VALUE_SCALAR:
        assert(ctx->current_value_type != SOPC_Null_Id);

        if (ctx->current_array_type == SOPC_VariantArrayType_SingleValue)
        {
            if (!set_element_value_scalar(ctx))
            {
                XML_StopParser(ctx->parser, false);
                return;
            }

            ctx->current_value_type = SOPC_Null_Id;
            ctx->state = PARSE_NODE_VALUE;
        }
        else if (ctx->current_array_type == SOPC_VariantArrayType_Array)
        {
            if (!append_element_value(ctx))
            {
                XML_StopParser(ctx->parser, false);
                return;
            }

            ctx->state = PARSE_NODE_VALUE_ARRAY;
        }
        else
        {
            assert(false);
        }

        break;
    case PARSE_NODE_VALUE_ARRAY:
        assert(ctx->current_array_type == SOPC_VariantArrayType_Array);

        if (!set_element_value_array(ctx))
        {
            XML_StopParser(ctx->parser, false);
            return;
        }
        ctx->current_array_type = SOPC_VariantArrayType_SingleValue;
        ctx->current_value_type = SOPC_Null_Id;
        ctx->state = PARSE_NODE_VALUE;
        break;
    case PARSE_NODE_VALUE:
        ctx->state = PARSE_NODE;
        break;
    case PARSE_NODE:
    {
        bool ok = finalize_node(ctx);

        if (!ok)
        {
            SOPC_AddressSpace_Item_Clear(&ctx->item);
            XML_StopParser(ctx->parser, false);
            return;
        }

        ctx->item.node_class = 0;
        ctx->state = PARSE_NODESET;
        break;
    }
    case PARSE_NODE_REFERENCE:
        if (!finalize_reference(ctx))
        {
            XML_StopParser(ctx->parser, false);
            return;
        }

        ctx->state = PARSE_NODE_REFERENCES;
        break;
    case PARSE_NODESET:
        break;
    case PARSE_START:
        assert(false && "Got end_element callback when in PARSE_START state.");
        break;
    }
}

static void char_data_handler(void* user_data, const XML_Char* s, int len)
{
    assert(len >= 0);

    struct parse_context_t* ctx = user_data;

    switch (ctx->state)
    {
    case PARSE_NODE_DISPLAYNAME:
    case PARSE_NODE_DESCRIPTION:
    case PARSE_ALIAS:
    case PARSE_NODE_REFERENCE:
    case PARSE_NODE_VALUE_SCALAR:
        if (!ctx_char_data_append(ctx, s, (size_t) len))
        {
            XML_StopParser(ctx->parser, false);
            return;
        }

        break;
    default:
        return;
    }
}

static uint64_t str_hash(const void* data)
{
    return SOPC_DJBHash((const uint8_t*) data, strlen((const char*) data));
}

static bool str_equal(const void* a, const void* b)
{
    return strcmp((const char*) a, (const char*) b) == 0;
}

SOPC_AddressSpace* SOPC_UANodeSet_Parse(FILE* fd)
{
    static const size_t char_data_cap_initial = 4096;
    SOPC_Dict* aliases = SOPC_Dict_Create(NULL, str_hash, str_equal, SOPC_Free, SOPC_Free);
    XML_Parser parser = XML_ParserCreateNS(NULL, NS_SEPARATOR[0]);
    char* char_data_buffer = SOPC_Calloc(char_data_cap_initial, sizeof(char));
    SOPC_AddressSpace* space = SOPC_AddressSpace_Create(true);

    if ((aliases == NULL) || (parser == NULL) || (char_data_buffer == NULL) || (space == NULL))
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        SOPC_Dict_Delete(aliases);
        XML_ParserFree(parser);
        SOPC_Free(char_data_buffer);
        SOPC_AddressSpace_Delete(space);
        return NULL;
    }

    struct parse_context_t ctx;
    memset(&ctx, 0, sizeof(struct parse_context_t));
    XML_SetUserData(parser, &ctx);

    ctx.aliases = aliases;
    ctx.state = PARSE_START;
    ctx.parser = parser;
    ctx.space = space;
    ctx.char_data_buffer = char_data_buffer;
    ctx.char_data_cap = char_data_cap_initial;

    XML_SetElementHandler(parser, start_element_handler, end_element_handler);
    XML_SetCharacterDataHandler(parser, char_data_handler);

    SOPC_ReturnStatus res = parse(parser, fd);
    XML_ParserFree(parser);
    SOPC_Dict_Delete(aliases);
    SOPC_Free(ctx.current_alias_alias);
    SOPC_Free(ctx.char_data_buffer);
    SOPC_Array_Delete(ctx.references);
    SOPC_Array_Delete(ctx.list_items);

    if (res == SOPC_STATUS_OK)
    {
        return space;
    }
    else
    {
        SOPC_AddressSpace_Item_Clear(&ctx.item);
        SOPC_AddressSpace_Delete(space);
        return NULL;
    }
}
