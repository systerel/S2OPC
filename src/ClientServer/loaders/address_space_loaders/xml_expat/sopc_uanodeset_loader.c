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

#include "sopc_uanodeset_loader.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "expat.h"

#include "opcua_identifiers.h"
#include "opcua_statuscodes.h"
#include "sopc_array.h"
#include "sopc_dict.h"
#include "sopc_encodeable.h"
#include "sopc_encoder.h"
#include "sopc_hash.h"
#include "sopc_helper_expat.h"
#include "sopc_helper_string.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_singly_linked_list.h"
#include "sopc_time.h"

typedef enum
{
    PARSE_START,                     // Beginning of file
    PARSE_NODESET,                   // In a UANodeSet
    PARSE_ALIASES,                   // In an Aliases tag
    PARSE_ALIAS,                     // ... in its Alias
    PARSE_NODE,                      // In a UANode subtype tag
    PARSE_NODE_DISPLAYNAME,          // ... in its DisplayName
    PARSE_NODE_DESCRIPTION,          // ... in its Description
    PARSE_NODE_REFERENCES,           // ... in its References
    PARSE_NODE_REFERENCE,            // ... in its References/Reference
    PARSE_NODE_VALUE,                // In the Value tag of a UAVariable/UAVariableType tag
    PARSE_NODE_VALUE_SCALAR,         // ... for a scalar type
    PARSE_NODE_VALUE_SCALAR_COMPLEX, //     ... which is a complex value (sub-tags to manage)
    PARSE_NODE_VALUE_ARRAY,          // ... for an array type
} parse_state_t;

typedef struct parse_complex_value_tag_t parse_complex_value_tag_t;

typedef parse_complex_value_tag_t* parse_complex_value_tag_array_t;

struct parse_complex_value_tag_t
{
    const char* name;
    const bool is_array;
    parse_complex_value_tag_array_t childs; // C-array of tags

    bool set;
    char* single_value;       // shall remain NULL if is_array == true
    SOPC_Array* array_values; // shall remain null if is_array == false
    const void* user_data;    // Only used to store EncodeableType pointer for Extension object for now
};

/* Context used to parse 1 complex value (sub-tags to manage) */
typedef struct parse_complex_value_context_t
{
    bool is_extension_object; // Extension object need a specific treatment since (typeId needed to know body content)
    const char* value_tag;
    parse_complex_value_tag_array_t tags;          // C-array of tags
    SOPC_SLinkedList* end_element_restore_context; // restore the C-array of tags on end_element
} parse_complex_value_context_t;

struct parse_context_t
{
    SOPC_HelperExpatCtx helper_ctx;

    SOPC_AddressSpace* space;
    parse_state_t state;

    SOPC_Dict* aliases;
    char* current_alias_alias;

    // Variable Value management
    SOPC_BuiltinId current_value_type;
    SOPC_VariantArrayType current_array_type;
    parse_complex_value_context_t complex_value_ctx;
    SOPC_Array* list_nodes;

    SOPC_AddressSpace_Node node;
    // Temporary array to store the references.
    SOPC_Array* references;
};

#define NS_SEPARATOR "|"
#define UA_NODESET_NS "http://opcfoundation.org/UA/2011/03/UANodeSet.xsd"
#define UA_TYPES_NS "http://opcfoundation.org/UA/2008/02/Types.xsd"
#define NS(ns, tag) ns NS_SEPARATOR tag
#define UA_EXTENSION_OBJECT_VALUE UA_TYPES_NS NS_SEPARATOR "ExtensionObject"
#define UA_LIST_EXTENSION_OBJECT_VALUE UA_TYPES_NS NS_SEPARATOR "ListOfExtensionObject"

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
                        XML_GetCurrentLineNumber(parser), XML_GetCurrentColumnNumber(parser),
                        (int) XML_GetErrorCode(parser));
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

static bool start_alias(struct parse_context_t* ctx, const XML_Char** attrs)
{
    assert(ctx->current_alias_alias == NULL);

    for (size_t i = 0; attrs[i]; i++)
    {
        const char* attr = attrs[i];

        if (strcmp(attr, "Alias") == 0)
        {
            const char* val = attrs[++i];
            ctx->current_alias_alias = SOPC_strdup(val);

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

/* Encode complex values as parse_complex_value_tag_t structures*/

// element to mark end of parse_complex_value_tag_t array (name == NULL)
#define END_COMPLEX_VALUE_TAG                      \
    {                                              \
        NULL, false, NULL, false, NULL, NULL, NULL \
    }

// Note use #define to could use it as distinct context storage during same complex value parsing (here several NodeIds)
#define COMPLEX_VALUE_NODE_ID_TAGS                                                  \
    {                                                                               \
        {"Identifier", false, NULL, false, NULL, NULL, NULL}, END_COMPLEX_VALUE_TAG \
    }

static parse_complex_value_tag_t complex_value_node_id_tags[] = COMPLEX_VALUE_NODE_ID_TAGS;

#define COMPLEX_VALUE_GUID_TAGS                                                 \
    {                                                                           \
        {"String", false, NULL, false, NULL, NULL, NULL}, END_COMPLEX_VALUE_TAG \
    }

static parse_complex_value_tag_t complex_value_guid_tags[] = COMPLEX_VALUE_GUID_TAGS;

#define COMPLEX_VALUE_LOCALIZED_TEXT_TAGS                                                                 \
    {                                                                                                     \
        {"Locale", false, NULL, false, NULL, NULL, NULL}, {"Text", false, NULL, false, NULL, NULL, NULL}, \
            END_COMPLEX_VALUE_TAG                                                                         \
    }

static parse_complex_value_tag_t complex_value_localized_text_tags[] = COMPLEX_VALUE_LOCALIZED_TEXT_TAGS;

static parse_complex_value_tag_t complex_value_empty_ext_obj_tags[] = {
    {"TypeId", false, (parse_complex_value_tag_t[]) COMPLEX_VALUE_NODE_ID_TAGS, false, NULL, NULL, NULL},
    {"Body", false, NULL, false, NULL, NULL, NULL},
    END_COMPLEX_VALUE_TAG};

/* ExtensionObject Body content complex values definition */
static parse_complex_value_tag_t complex_value_ext_obj_argument_tags[] = {

    {"Argument", false,
     (parse_complex_value_tag_t[]){
         {"Name", false, NULL, false, NULL, NULL, NULL},
         {"DataType", false, (parse_complex_value_tag_t[]) COMPLEX_VALUE_NODE_ID_TAGS, false, NULL, NULL, NULL},
         {"ValueRank", false, NULL, false, NULL, NULL, NULL},
         {"ArrayDimensions", false,
          (parse_complex_value_tag_t[]){{"UInt32", true, NULL, false, NULL, NULL, NULL}, END_COMPLEX_VALUE_TAG}, false,
          NULL, NULL, NULL},
         {"Description", false, (parse_complex_value_tag_t[]) COMPLEX_VALUE_LOCALIZED_TEXT_TAGS, false, NULL, NULL,
          NULL},
         END_COMPLEX_VALUE_TAG},
     false, NULL, NULL, NULL},
    END_COMPLEX_VALUE_TAG};

/*
 * Returns the extension object body tags expected for given TypeId nodeId as output parameter if available.
 * In case of success the EncodeableType pointer is returned and shall be used to create the extension object.
 * In case of failure, NULL is returned.
 */
static const void* ext_obj_body_from_its_type_id(uint32_t nid_in_NS0, parse_complex_value_tag_array_t* body_tags)
{
    assert(NULL != body_tags);
    const void* encType = NULL;

    switch (nid_in_NS0)
    {
    case 296: // Argument datatype nodeId
    case 297: // Argument XML encoding nodeId
        encType = &OpcUa_Argument_EncodeableType;
        *body_tags = complex_value_ext_obj_argument_tags;
        break;
    default:
        encType = NULL;
        break;
    }
    return encType;
}

static bool type_id_from_name(const char* name,
                              SOPC_BuiltinId* type_id,
                              bool* is_simple_type,
                              parse_complex_value_tag_array_t* tags)
{
    static const struct
    {
        const char* name;
        SOPC_BuiltinId id;
        bool is_simple_type; /* true => only text value to retrieve / false => sub-tags to manage */
        parse_complex_value_tag_array_t tags;
    } TYPE_IDS[] = {
        {"Boolean", SOPC_Boolean_Id, true, NULL},
        {"SByte", SOPC_SByte_Id, true, NULL},
        {"Byte", SOPC_Byte_Id, true, NULL},
        {"Int16", SOPC_Int16_Id, true, NULL},
        {"UInt16", SOPC_UInt16_Id, true, NULL},
        {"Int32", SOPC_Int32_Id, true, NULL},
        {"UInt32", SOPC_UInt32_Id, true, NULL},
        {"Int64", SOPC_Int64_Id, true, NULL},
        {"UInt64", SOPC_UInt64_Id, true, NULL},
        {"Float", SOPC_Float_Id, true, NULL},
        {"Double", SOPC_Double_Id, true, NULL},
        {"String", SOPC_String_Id, true, NULL},
        {"DateTime", SOPC_DateTime_Id, true, NULL},
        {"Guid", SOPC_Guid_Id, false, complex_value_guid_tags},
        {"ByteString", SOPC_ByteString_Id, true, NULL},
        {"XmlElement", SOPC_XmlElement_Id, true, NULL}, // Shall be XML to be interpreted as string
        {"NodeId", SOPC_NodeId_Id, false, complex_value_node_id_tags},
        {"ExpandedNodeId", SOPC_ExpandedNodeId_Id, false, NULL},
        {"StatusCode", SOPC_StatusCode_Id, true, NULL},
        {"QualifiedName", SOPC_QualifiedName_Id, false, NULL},
        {"LocalizedText", SOPC_LocalizedText_Id, false, complex_value_localized_text_tags},
        {"ExtensionObject", SOPC_ExtensionObject_Id, false, complex_value_empty_ext_obj_tags},
        {"Structure", SOPC_ExtensionObject_Id, false, complex_value_empty_ext_obj_tags},

        {NULL, SOPC_Null_Id, true, NULL},
    };

    for (size_t i = 0; TYPE_IDS[i].name != NULL; ++i)
    {
        if (strcmp(name, TYPE_IDS[i].name) == 0)
        {
            *type_id = TYPE_IDS[i].id;
            *is_simple_type = TYPE_IDS[i].is_simple_type;
            *tags = TYPE_IDS[i].tags;
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
    assert(ctx->node.node_class == 0);

    SOPC_AddressSpace_Node_Initialize(ctx->space, &ctx->node, element_type);
    // Note: value_status default value set on NodeId parsing

    for (size_t i = 0; attrs[i]; ++i)
    {
        const char* attr = attrs[i];

        if (strcmp("NodeId", attr) == 0)
        {
            const char* attr_val = attrs[++i];

            if (attr_val == NULL)
            {
                LOG_XML_ERROR(ctx->helper_ctx.parser, "Missing value for NodeId attribute");
                return false;
            }

            SOPC_NodeId* id = SOPC_NodeId_FromCString(attr_val, (int32_t) strlen(attr_val));

            if (id == NULL)
            {
                LOG_XML_ERRORF(ctx->helper_ctx.parser, "Invalid variable NodeId: %s", attr_val);
                return false;
            }

            // Set value_status default value:
            // Keep OPC UA default namespace nodes with a Good status,
            // necessary to pass UACTT otherwise keep Good status only if a value is defined
            ctx->node.value_status = id->Namespace == 0 ? SOPC_GoodGenericStatus : OpcUa_UncertainInitialValue;

            SOPC_NodeId* element_id = SOPC_AddressSpace_Get_NodeId(ctx->space, &ctx->node);
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

            SOPC_QualifiedName* element_browse_name = SOPC_AddressSpace_Get_BrowseName(ctx->space, &ctx->node);
            SOPC_QualifiedName_Initialize(element_browse_name);
            SOPC_ReturnStatus status = SOPC_QualifiedName_ParseCString(element_browse_name, attr_val);

            if (status != SOPC_STATUS_OK)
            {
                LOG_XML_ERRORF(ctx->helper_ctx.parser, "Invalid browse name: %s", attr_val);
                return false;
            }
        }
        else if (strcmp("DataType", attr) == 0)
        {
            if (OpcUa_NodeClass_Variable != element_type && OpcUa_NodeClass_VariableType != element_type)
            {
                LOG_XML_ERRORF(ctx->helper_ctx.parser,
                               "Unexpected DataType attribute (value '%s') on node of class = %s", attrs[++i],
                               tag_from_element_id(element_type));
                return false;
            }

            const char* attr_val = attrs[++i];

            if (attr_val == NULL)
            {
                LOG_XML_ERROR(ctx->helper_ctx.parser, "Missing value for DataType attribute");
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
                LOG_XML_ERRORF(ctx->helper_ctx.parser, "Invalid variable NodeId: %s", attr_val);
                return false;
            }

            SOPC_NodeId* dataType = SOPC_AddressSpace_Get_DataType(ctx->space, &ctx->node);
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
                LOG_XML_ERRORF(ctx->helper_ctx.parser,
                               "Unexpected ValueRank attribute (value '%s') on node of class = %s", attrs[++i],
                               tag_from_element_id(element_type));
                return false;
            }

            const char* attr_val = attrs[++i];

            if (attr_val == NULL)
            {
                LOG_XML_ERROR(ctx->helper_ctx.parser, "Missing value for ValueRank attribute");
                return false;
            }

            int32_t parsedValueRank;
            bool result = SOPC_strtoint(attr_val, (size_t) strlen(attr_val), 32, &parsedValueRank);

            if (!result)
            {
                LOG_XML_ERROR(ctx->helper_ctx.parser, "Incorrect value for ValueRank attribute");
                return false;
            }

            int32_t* valueRank = SOPC_AddressSpace_Get_ValueRank(ctx->space, &ctx->node);
            *valueRank = parsedValueRank;
        }
        else if (strcmp("AccessLevel", attr) == 0)
        {
            assert(OpcUa_NodeClass_Variable == element_type);
            if (OpcUa_NodeClass_Variable != element_type)
            {
                LOG_XML_ERRORF(ctx->helper_ctx.parser,
                               "Unexpected AccessLevel attribute (value '%s') on node of class = %s", attrs[++i],
                               tag_from_element_id(element_type));
                return false;
            }

            const char* attr_val = attrs[++i];

            if (!SOPC_strtouint(attr_val, strlen(attr_val), 8, &ctx->node.data.variable.AccessLevel))
            {
                LOG_XML_ERRORF(ctx->helper_ctx.parser, "Invalid AccessLevel on node value: '%s", attr_val);
                return false;
            }
        }
        else if (strcmp("Executable", attr) == 0)
        {
            assert(OpcUa_NodeClass_Method == element_type);
            if (OpcUa_NodeClass_Method != element_type)
            {
                LOG_XML_ERRORF(ctx->helper_ctx.parser,
                               "Unexpected Executable attribute (value '%s') on node of class = %s", attrs[++i],
                               tag_from_element_id(element_type));
                return false;
            }

            const char* attr_val = attrs[++i];

            ctx->node.data.method.Executable = (strcmp(attr_val, "true") == 0);
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
                LOG_XML_ERRORF(ctx->helper_ctx.parser, "Error while parsing ReferenceType '%s' into a NodeId\n.", val);
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
    assert(ctx->list_nodes == NULL);

    ctx->list_nodes = SOPC_Array_Create(sizeof(SOPC_Variant), 0, (SOPC_Array_Free_Func*) SOPC_Variant_ClearAux);

    if (ctx->list_nodes == NULL)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }

    return true;
}

static bool current_element_has_value(struct parse_context_t* ctx)
{
    switch (ctx->node.node_class)
    {
    case OpcUa_NodeClass_Variable:
    case OpcUa_NodeClass_VariableType:
        return true;
    default:
        return false;
    }
}

static void clear_complex_value_tag_array_content(parse_complex_value_tag_array_t complex_type_tags)
{
    int index = 0;
    parse_complex_value_tag_t* current = &complex_type_tags[index];
    while (NULL != current->name)
    {
        if (NULL != current->childs)
        {
            clear_complex_value_tag_array_content(current->childs);
        }
        if (current->is_array)
        {
            SOPC_Array_Delete(current->array_values);
            current->array_values = NULL;
        }
        else
        {
            SOPC_Free(current->single_value);
            current->single_value = NULL;
        }
        current->set = false;
        index++;
        current = &complex_type_tags[index];
    }
}

static void clear_complex_value_context(parse_complex_value_context_t* complex_value_ctx)
{
    clear_complex_value_tag_array_content(complex_value_ctx->tags);
    complex_value_ctx->tags = NULL;
    SOPC_SLinkedList_Delete(complex_value_ctx->end_element_restore_context);
    complex_value_ctx->end_element_restore_context = NULL;
    complex_value_ctx->is_extension_object = false;
    complex_value_ctx->value_tag = NULL;
}

static bool type_id_from_tag(const char* tag,
                             SOPC_BuiltinId* type_id,
                             SOPC_VariantArrayType* array_type,
                             bool* is_simple_type,
                             parse_complex_value_tag_array_t* complex_type_tags)
{
    // tag should have the correct namespace
    if (strncmp(tag, UA_TYPES_NS NS_SEPARATOR, strlen(UA_TYPES_NS NS_SEPARATOR)) != 0)
    {
        return false;
    }

    const char* name = tag + strlen(UA_TYPES_NS NS_SEPARATOR);

    if (strncmp(name, "ListOf", strlen("ListOf")) == 0)
    {
        *array_type = SOPC_VariantArrayType_Array;
        name = name + strlen("ListOf");
    }
    else
    {
        *array_type = SOPC_VariantArrayType_SingleValue;
    }

    return type_id_from_name(name, type_id, is_simple_type, complex_type_tags);
}

static bool complex_value_tag_from_tag_name_no_namespace(const char* tag_name,
                                                         parse_complex_value_tag_array_t inCurrentCtx,
                                                         parse_complex_value_tag_t** outTagCtx)
{
    assert(NULL != inCurrentCtx);
    assert(NULL != outTagCtx);

    *outTagCtx = NULL;
    int index = 0;
    parse_complex_value_tag_t* current = &inCurrentCtx[index];
    while (NULL != current->name)
    {
        if (strcmp(tag_name, current->name) == 0)
        {
            *outTagCtx = current;
            return true;
        }
        index++;
        current = &inCurrentCtx[index];
    }
    return false;
}

static bool complex_value_tag_from_tag(const char* tag,
                                       parse_complex_value_tag_array_t inCurrentCtx,
                                       parse_complex_value_tag_t** outTagCtx)
{
    // tag should have the correct namespace
    if (strncmp(tag, UA_TYPES_NS NS_SEPARATOR, strlen(UA_TYPES_NS NS_SEPARATOR)) != 0)
    {
        return false;
    }

    const char* name = tag + strlen(UA_TYPES_NS NS_SEPARATOR);

    return complex_value_tag_from_tag_name_no_namespace(name, inCurrentCtx, outTagCtx);
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

static bool init_value_complex_ctx(struct parse_context_t* ctx,
                                   const char* value_tag,
                                   parse_complex_value_tag_array_t complex_type_tags)
{
    /* Manage complex types */
    if (NULL == complex_type_tags)
    {
        LOG_XML_ERRORF(ctx->helper_ctx.parser, "Unsupported value type: %s", value_tag);
        SOPC_HelperExpat_PushSkipTag(&ctx->helper_ctx, value_tag);
        SOPC_Array_Delete(ctx->list_nodes);
        ctx->list_nodes = NULL;
        ctx->current_value_type = SOPC_Null_Id;
        ctx->current_array_type = SOPC_VariantArrayType_SingleValue;
        return false;
    }
    ctx->complex_value_ctx.value_tag = value_tag;
    ctx->complex_value_ctx.tags = complex_type_tags;
    ctx->complex_value_ctx.end_element_restore_context = SOPC_SLinkedList_Create(0);
    /* Append current value complex type context into context stack:
       when stack is empty it indicates end of the value tag */
    void* appended = SOPC_SLinkedList_Append(ctx->complex_value_ctx.end_element_restore_context, 0, complex_type_tags);
    if (ctx->complex_value_ctx.end_element_restore_context == NULL || appended == NULL)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        XML_StopParser(ctx->helper_ctx.parser, false);
        return false;
    }

    if (strncmp(UA_EXTENSION_OBJECT_VALUE, value_tag, strlen(UA_EXTENSION_OBJECT_VALUE)) == 0)
    {
        ctx->complex_value_ctx.is_extension_object = true;
    }
    else
    {
        ctx->complex_value_ctx.is_extension_object = false;
    }

    return true;
}

static bool start_in_extension_object(struct parse_context_t* ctx, parse_complex_value_tag_t* currentTagCtx)
{
    bool ok = false;

    if (0 == strncmp("Body", currentTagCtx->name, strlen("Body")))
    {
        // Only check if body children defined
        ok = NULL != currentTagCtx->childs;
        // Do not consider ExtensionObject exceptional case anymore to parse body content
        ctx->complex_value_ctx.is_extension_object = false;
        currentTagCtx->set = true;
    }
    else if (0 == strncmp("TypeId", currentTagCtx->name, strlen("TypedId")))
    {
        // Nothing to do on start
        ok = true;
    }
    else if (0 == strncmp("Identifier", currentTagCtx->name, strlen("Identifier")))
    {
        // Nothing to do on start
        ok = true;
    }
    else
    {
        assert(false);
    }

    if (!ok)
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser, "Unsupported Body defined first in ExtensionObject");
        // Clear all extension object (or list of) context
        clear_complex_value_context(&ctx->complex_value_ctx);
        // Clear value parsing context
        ctx->current_value_type = SOPC_Null_Id;
        SOPC_Array_Delete(ctx->list_nodes);
        ctx->list_nodes = NULL;
        // Skip until end of the Value
        if (SOPC_VariantArrayType_SingleValue == ctx->current_array_type)
        {
            SOPC_HelperExpat_PushSkipTag(&ctx->helper_ctx, UA_EXTENSION_OBJECT_VALUE);
        }
        else if (SOPC_VariantArrayType_Array == ctx->current_array_type)
        {
            SOPC_HelperExpat_PushSkipTag(&ctx->helper_ctx, UA_LIST_EXTENSION_OBJECT_VALUE);
        }
        else
        {
            assert(false);
        }
        ctx->current_array_type = SOPC_VariantArrayType_SingleValue;
        ctx->state = PARSE_NODE_VALUE;
    }

    return ok;
}

static void start_element_handler(void* user_data, const XML_Char* name, const XML_Char** attrs)
{
    struct parse_context_t* ctx = user_data;
    SOPC_HelperExpatCtx* helperCtx = &ctx->helper_ctx;
    uint32_t element_type = 0;
    SOPC_BuiltinId type_id = SOPC_Null_Id;
    SOPC_VariantArrayType array_type = SOPC_VariantArrayType_SingleValue;
    bool is_simple_type = false;
    parse_complex_value_tag_array_t complex_type_tags = NULL;
    parse_complex_value_tag_t* currentTagCtx = NULL;

    if (SOPC_HelperExpat_IsSkipTagActive(helperCtx))
    {
        return; // We're skipping until the end of a tag
    }

    switch (ctx->state)
    {
    case PARSE_START:
        if (strcmp(name, NS(UA_NODESET_NS, "UANodeSet")) != 0)
        {
            LOG_XML_ERRORF(helperCtx->parser, "Unexpected tag %s", name);
            XML_StopParser(helperCtx->parser, 0);
            return;
        }

        ctx->state = PARSE_NODESET;
        return;
    case PARSE_NODESET:
    {
        element_type = element_id_from_tag(name);

        if (element_type > 0)
        {
            if (!start_node(ctx, element_type, attrs))
            {
                XML_StopParser(helperCtx->parser, 0);
                return;
            }
        }
        else if (strcmp(NS(UA_NODESET_NS, "Aliases"), name) == 0)
        {
            ctx->state = PARSE_ALIASES;
        }
        else
        {
            SOPC_HelperExpat_PushSkipTag(&ctx->helper_ctx, name);
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
            SOPC_HelperExpat_PushSkipTag(&ctx->helper_ctx, name);
        }
        break;
    case PARSE_NODE_REFERENCES:
        if (strcmp(NS(UA_NODESET_NS, "Reference"), name) == 0)
        {
            if (!start_node_reference(ctx, attrs))
            {
                XML_StopParser(helperCtx->parser, 0);
                return;
            }
        }
        else
        {
            SOPC_HelperExpat_PushSkipTag(&ctx->helper_ctx, name);
        }
        break;
    case PARSE_ALIASES:
        if (strcmp(NS(UA_NODESET_NS, "Alias"), name) == 0)
        {
            if (!start_alias(ctx, attrs))
            {
                XML_StopParser(helperCtx->parser, 0);
                return;
            }
        }
        else
        {
            SOPC_HelperExpat_PushSkipTag(&ctx->helper_ctx, name);
        }
        break;
    case PARSE_NODE_VALUE:
    {
        assert(current_element_has_value(ctx));

        if (!type_id_from_tag(name, &type_id, &array_type, &is_simple_type, &complex_type_tags))
        {
            LOG_XML_ERRORF(helperCtx->parser, "Unsupported value type: %s", name);
            SOPC_HelperExpat_PushSkipTag(&ctx->helper_ctx, name);
            return;
        }

        assert(ctx->current_value_type == SOPC_Null_Id);

        ctx->current_value_type = type_id;
        ctx->current_array_type = array_type;

        if (array_type == SOPC_VariantArrayType_Array && !start_node_value_array(ctx))
        {
            XML_StopParser(helperCtx->parser, false);
            return;
        }

        if (array_type == SOPC_VariantArrayType_SingleValue)
        {
            if (is_simple_type)
            {
                ctx->state = PARSE_NODE_VALUE_SCALAR;
            }
            else
            {
                if (!init_value_complex_ctx(ctx, name, complex_type_tags))
                {
                    return;
                }
                ctx->state = PARSE_NODE_VALUE_SCALAR_COMPLEX;
            }
        }
        else
        {
            ctx->state = PARSE_NODE_VALUE_ARRAY;
        }

        break;
    }
    case PARSE_NODE_VALUE_SCALAR:
        LOG_XML_ERROR(helperCtx->parser, "Unexpected tag while parsing scalar value");
        XML_StopParser(helperCtx->parser, false);
        return;
    case PARSE_NODE_VALUE_ARRAY:
    {
        assert(current_element_has_value(ctx));
        assert(ctx->current_array_type == SOPC_VariantArrayType_Array);

        if (!type_id_from_tag(name, &type_id, &array_type, &is_simple_type, &complex_type_tags))
        {
            LOG_XML_ERRORF(helperCtx->parser, "Unsupported value type: %s", name);
            SOPC_HelperExpat_PushSkipTag(&ctx->helper_ctx, name);
            return;
        }

        if (type_id != ctx->current_value_type)
        {
            LOG_XML_ERRORF(helperCtx->parser, "Array value of type %s does not match array type", name);
            SOPC_HelperExpat_PushSkipTag(&ctx->helper_ctx, name);
            return;
        }

        if (array_type != SOPC_VariantArrayType_SingleValue)
        {
            LOG_XML_ERROR(helperCtx->parser, "Arrays cannot be nested");
            SOPC_HelperExpat_PushSkipTag(&ctx->helper_ctx, name);
            return;
        }

        if (is_simple_type)
        {
            ctx->state = PARSE_NODE_VALUE_SCALAR;
        }
        else
        {
            if (!init_value_complex_ctx(ctx, name, complex_type_tags))
            {
                return;
            }
            ctx->state = PARSE_NODE_VALUE_SCALAR_COMPLEX;
        }
        break;
    case PARSE_NODE_VALUE_SCALAR_COMPLEX:
        assert(current_element_has_value(ctx));
        assert(NULL != ctx->complex_value_ctx.tags);
        assert(NULL != ctx->complex_value_ctx.end_element_restore_context);

        if (!complex_value_tag_from_tag(name, ctx->complex_value_ctx.tags, &currentTagCtx))
        {
            LOG_XML_ERRORF(helperCtx->parser, "Unexpected tag in complex value: %s for BuiltInId type %d", name,
                           ctx->current_value_type);
            XML_StopParser(helperCtx->parser, 0);
            return;
        }

        /* Special treatment of ExtensionObject case: "TypeId" => fill body children*/
        if (ctx->complex_value_ctx.is_extension_object)
        {
            if (!start_in_extension_object(ctx, currentTagCtx))
            {
                return;
            }
        }

        // Enqueue the current context to be restored on end_element_handler
        SOPC_SLinkedList_Prepend(ctx->complex_value_ctx.end_element_restore_context, 0, ctx->complex_value_ctx.tags);

        if (NULL != currentTagCtx->childs)
        {
            // Replace current context by childs context until end_element_handler of this tag (using
            // end_element_restore_context)
            ctx->complex_value_ctx.tags = currentTagCtx->childs;
        }
        else
        {
            ctx->complex_value_ctx.tags = NULL;
        }

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
        LOG_XML_ERROR(ctx->helper_ctx.parser, "Missing Alias attribute on Alias.");
        return false;
    }

    char* target = SOPC_strdup(SOPC_HelperExpat_CharDataStripped(&ctx->helper_ctx));
    SOPC_HelperExpat_CharDataReset(&ctx->helper_ctx);

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
    const char* text = SOPC_HelperExpat_CharDataStripped(&ctx->helper_ctx);
    SOPC_NodeId* target_id = SOPC_NodeId_FromCString(text, (int32_t) strlen(text));

    if (target_id == NULL)
    {
        LOG_XML_ERRORF(ctx->helper_ctx.parser, "Cannot parse reference target '%s' into a NodeId.", text);
        return false;
    }

    SOPC_HelperExpat_CharDataReset(&ctx->helper_ctx);

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
        return SOPC_AddressSpace_Get_DisplayName(ctx->space, &ctx->node);
    case PARSE_NODE_DESCRIPTION:
        return SOPC_AddressSpace_Get_Description(ctx->space, &ctx->node);
    default:
        assert(false && "Unexpected state");
    }
    return NULL;
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
    return 0;
}

#define FOREACH_SIGNED_VALUE_TYPE(x) \
    x(SOPC_SByte_Id, Sbyte) x(SOPC_Int16_Id, Int16) x(SOPC_Int32_Id, Int32) x(SOPC_Int64_Id, Int64)

#define FOREACH_UNSIGNED_VALUE_TYPE(x) \
    x(SOPC_Byte_Id, Byte) x(SOPC_UInt16_Id, Uint16) x(SOPC_UInt32_Id, Uint32) x(SOPC_UInt64_Id, Uint64)

#define SET_INT_ELEMENT_VALUE_CASE(id, field, int_or_uint)                                \
    case id:                                                                              \
        if (SOPC_strto##int_or_uint(val, strlen(val), type_width(id), &var->Value.field)) \
        {                                                                                 \
            var->BuiltInTypeId = id;                                                      \
            return true;                                                                  \
        }                                                                                 \
        else                                                                              \
        {                                                                                 \
            LOG_XML_ERRORF(ctx->helper_ctx.parser, "Invalid integer value: '%s'", val);   \
            return false;                                                                 \
        }

#define SET_SIGNED_INT_ELEMENT_VALUE_CASE(id, field) SET_INT_ELEMENT_VALUE_CASE(id, field, int)
#define SET_UNSIGNED_INT_ELEMENT_VALUE_CASE(id, field) SET_INT_ELEMENT_VALUE_CASE(id, field, uint)

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

static bool set_variant_value_localized_text(SOPC_LocalizedText** plt, parse_complex_value_tag_array_t tagsContext)
{
    assert(plt != NULL);

    parse_complex_value_tag_t* currentTagCtx = NULL;
    bool locale_ok = complex_value_tag_from_tag_name_no_namespace("Locale", tagsContext, &currentTagCtx);
    assert(locale_ok);

    const char* locale = currentTagCtx->single_value;
    if (!currentTagCtx->set)
    {
        // Empty locale
        locale = "";
    }

    bool text_ok = complex_value_tag_from_tag_name_no_namespace("Text", tagsContext, &currentTagCtx);
    assert(text_ok);

    const char* text = currentTagCtx->single_value;
    if (!currentTagCtx->set)
    {
        // Empty text
        text = "";
    }

    SOPC_LocalizedText* lt = SOPC_Calloc(1, sizeof(SOPC_LocalizedText));

    if (lt == NULL)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        SOPC_Free(lt);
        return false;
    }

    SOPC_LocalizedText_Initialize(lt);

    if (SOPC_String_CopyFromCString(&lt->defaultLocale, locale) != SOPC_STATUS_OK)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        SOPC_LocalizedText_Clear(lt);
        SOPC_Free(lt);
        return false;
    }

    if (SOPC_String_CopyFromCString(&lt->defaultText, text) != SOPC_STATUS_OK)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        SOPC_LocalizedText_Clear(lt);
        SOPC_Free(lt);
        return false;
    }

    *plt = lt;
    return true;
}

static bool set_variant_value_nodeid(SOPC_NodeId** nodeId, parse_complex_value_tag_array_t tagsContext)
{
    assert(NULL != nodeId);

    parse_complex_value_tag_t* currentTagCtx = NULL;
    bool id_tag_ok = complex_value_tag_from_tag_name_no_namespace("Identifier", tagsContext, &currentTagCtx);
    assert(id_tag_ok);

    const char* id = currentTagCtx->single_value;
    if (!currentTagCtx->set)
    {
        // Null identifier
        id = "i=0";
    }

    size_t len = strlen(id);
    assert(len <= INT32_MAX);

    *nodeId = SOPC_NodeId_FromCString(id, (int32_t) len);

    if (*nodeId == NULL)
    {
        LOGF("Invalid NodeId: '%s'", id);
        return false;
    }

    return true;
}

static bool set_variant_value_guid(SOPC_Guid** guid, parse_complex_value_tag_array_t tagsContext)
{
    assert(NULL != guid);

    parse_complex_value_tag_t* currentTagCtx = NULL;
    bool id_tag_ok = complex_value_tag_from_tag_name_no_namespace("String", tagsContext, &currentTagCtx);
    assert(id_tag_ok);

    const char* stringGuid = currentTagCtx->single_value;
    if (!currentTagCtx->set)
    {
        // Null string
        stringGuid = "";
    }

    size_t len = strlen(stringGuid);
    assert(len <= INT32_MAX);

    *guid = SOPC_Malloc(sizeof(SOPC_Guid));
    if (NULL == *guid)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }

    SOPC_ReturnStatus status = SOPC_Guid_FromCString(*guid, stringGuid, len);
    if (SOPC_STATUS_OK != status)
    {
        LOGF("Invalid Guid: '%s'", stringGuid);
        return false;
    }

    return true;
}

static bool set_variant_value_extobj_argument(OpcUa_Argument* argument,
                                              parse_complex_value_tag_array_t bodyChildsTagContext)
{
    bool result = true;

    parse_complex_value_tag_t* argumentTagCtx = NULL;
    bool argument_tag_ok =
        complex_value_tag_from_tag_name_no_namespace("Argument", bodyChildsTagContext, &argumentTagCtx);
    assert(argument_tag_ok);

    parse_complex_value_tag_t* nameTagCtx = NULL;
    bool name_tag_ok = complex_value_tag_from_tag_name_no_namespace("Name", argumentTagCtx->childs, &nameTagCtx);
    assert(name_tag_ok);

    parse_complex_value_tag_t* dataTypeTagCtx = NULL;
    bool dataType_tag_ok =
        complex_value_tag_from_tag_name_no_namespace("DataType", argumentTagCtx->childs, &dataTypeTagCtx);
    assert(dataType_tag_ok);

    parse_complex_value_tag_t* dataTypeIdTagCtx = NULL;
    bool id_tag_ok =
        complex_value_tag_from_tag_name_no_namespace("Identifier", dataTypeTagCtx->childs, &dataTypeIdTagCtx);
    assert(id_tag_ok);

    parse_complex_value_tag_t* valueRankTagCtx = NULL;
    bool valueRank_tag_ok =
        complex_value_tag_from_tag_name_no_namespace("ValueRank", argumentTagCtx->childs, &valueRankTagCtx);
    assert(valueRank_tag_ok);

    parse_complex_value_tag_t* arrayDimensionsTagCtx = NULL;
    bool arrayDimensions_tag_ok =
        complex_value_tag_from_tag_name_no_namespace("ArrayDimensions", argumentTagCtx->childs, &arrayDimensionsTagCtx);
    assert(arrayDimensions_tag_ok);

    parse_complex_value_tag_t* arrayDimUInt32TagCtx = NULL;
    bool arrayDimUInt32_tag_ok =
        complex_value_tag_from_tag_name_no_namespace("UInt32", arrayDimensionsTagCtx->childs, &arrayDimUInt32TagCtx);
    assert(arrayDimUInt32_tag_ok);

    parse_complex_value_tag_t* descriptionTagCtx = NULL;
    bool description_tag_ok =
        complex_value_tag_from_tag_name_no_namespace("Description", argumentTagCtx->childs, &descriptionTagCtx);
    assert(description_tag_ok);

    if (nameTagCtx->set)
    {
        SOPC_ReturnStatus status = SOPC_String_CopyFromCString(&argument->Name, nameTagCtx->single_value);
        if (SOPC_STATUS_OK != status)
        {
            LOG_MEMORY_ALLOCATION_FAILURE;
            result = false;
        }
    }

    if (result && dataTypeIdTagCtx->set)
    {
        SOPC_NodeId* nodeId =
            SOPC_NodeId_FromCString(dataTypeIdTagCtx->single_value, (int32_t) strlen(dataTypeIdTagCtx->single_value));
        if (NULL == nodeId)
        {
            LOG_MEMORY_ALLOCATION_FAILURE;
            result = false;
        }
        else
        {
            argument->DataType = *nodeId;
        }
        SOPC_Free(nodeId);
    }

    if (result && valueRankTagCtx->set)
    {
        result = SOPC_strtoint(valueRankTagCtx->single_value, (size_t) strlen(valueRankTagCtx->single_value), 32,
                               &argument->ValueRank);
    }

    if (result && arrayDimUInt32TagCtx->set)
    {
        argument->NoOfArrayDimensions = (int32_t) SOPC_Array_Size(arrayDimUInt32TagCtx->array_values);
        argument->ArrayDimensions = SOPC_Array_Into_Raw(arrayDimUInt32TagCtx->array_values);
        arrayDimUInt32TagCtx->array_values = NULL;
    }

    if (result && descriptionTagCtx->set)
    {
        SOPC_LocalizedText* lt = NULL;
        result = set_variant_value_localized_text(&lt, descriptionTagCtx->childs);

        if (result)
        {
            argument->Description = *lt;
            SOPC_Free(lt);
        }
    }

    if (!result)
    {
        OpcUa_Argument_Clear(argument);
    }

    return result;
}

static bool set_variant_value_extensionobject(SOPC_ExtensionObject** extObj,
                                              parse_complex_value_tag_array_t tagsContext)
{
    assert(NULL != extObj);

    parse_complex_value_tag_t* typeIdTagCtx = NULL;
    bool typeid_tag_ok = complex_value_tag_from_tag_name_no_namespace("TypeId", tagsContext, &typeIdTagCtx);
    assert(typeid_tag_ok);
    if (!typeIdTagCtx->set)
    {
        return false;
    }
    assert(NULL != typeIdTagCtx->user_data);
    SOPC_EncodeableType* encType = typeIdTagCtx->user_data;

    parse_complex_value_tag_t* bodyTagCtx = NULL;
    bool body_tag_ok = complex_value_tag_from_tag_name_no_namespace("Body", tagsContext, &bodyTagCtx);
    assert(body_tag_ok);
    if (!bodyTagCtx->set)
    {
        return false;
    }
    assert(NULL != bodyTagCtx->childs);

    SOPC_ExtensionObject* newExtObj = SOPC_Malloc(sizeof(SOPC_ExtensionObject));
    if (NULL == newExtObj)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }

    SOPC_ExtensionObject_Initialize(newExtObj);
    void* object = NULL;
    SOPC_ReturnStatus status = SOPC_Encodeable_CreateExtension(newExtObj, encType, &object);

    if (SOPC_STATUS_OK != status)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        SOPC_Free(newExtObj);
        return false;
    }

    bool result = false;

    switch (encType->TypeId)
    {
    case OpcUaId_Argument:
        result = set_variant_value_extobj_argument((OpcUa_Argument*) object, bodyTagCtx->childs);
        break;
    default:
        assert(false);
    }

    if (!result)
    {
        SOPC_ExtensionObject_Clear(newExtObj);
        SOPC_Free(newExtObj);
        newExtObj = NULL;
    }

    *extObj = newExtObj;

    return result;
}

static bool set_variant_value(struct parse_context_t* ctx, SOPC_Variant* var, const char* val)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_BuiltinId type_id = ctx->current_value_type;
    switch (type_id)
    {
    case SOPC_Boolean_Id:
        var->BuiltInTypeId = SOPC_Boolean_Id;
        var->Value.Boolean = (strcmp(val, "true") == 0);
        return true;
        /* case SOPC_SByte_Id: */
        /* case SOPC_Int*_Id: */
        FOREACH_SIGNED_VALUE_TYPE(SET_SIGNED_INT_ELEMENT_VALUE_CASE)
        /* case SOPC_Byte_Id: */
        /* case SOPC_UInt*_Id: */
        FOREACH_UNSIGNED_VALUE_TYPE(SET_UNSIGNED_INT_ELEMENT_VALUE_CASE)
    case SOPC_Float_Id:
        if (SOPC_strtodouble(val, strlen(val), 32, &var->Value.Floatv))
        {
            var->BuiltInTypeId = SOPC_Float_Id;
            return true;
        }
        else
        {
            LOG_XML_ERRORF(ctx->helper_ctx.parser, "Invalid float value: '%s'", val);
            return false;
        }
    case SOPC_Double_Id:
        if (SOPC_strtodouble(val, strlen(val), 64, &var->Value.Doublev))
        {
            var->BuiltInTypeId = SOPC_Double_Id;
            return true;
        }
        else
        {
            LOG_XML_ERRORF(ctx->helper_ctx.parser, "Invalid double value: '%s'", val);
            return false;
        }
    case SOPC_DateTime_Id:
        status = SOPC_Time_FromXsdDateTime(val, strlen(val), &var->Value.Date);
        if (SOPC_STATUS_OK == status)
        {
            var->BuiltInTypeId = SOPC_DateTime_Id;
            return true;
        }
        else
        {
            LOG_XML_ERRORF(ctx->helper_ctx.parser, "Invalid or unsupported DateTime value: '%s', status = '%d'", val,
                           status);
            return false;
        }
    case SOPC_String_Id:
        SET_STR_ELEMENT_VALUE_CASE(String)
    case SOPC_ByteString_Id:
        return set_variant_value_bstring(var, val);
    case SOPC_XmlElement_Id: // TODO: should be a not simple type
        SET_STR_ELEMENT_VALUE_CASE(XmlElt)
    default:
        LOG_XML_ERRORF(ctx->helper_ctx.parser, "Cannot parse current value type (Not supported yet): '%d'", type_id);
        return false;
    }

    return false;
}

static bool set_variant_value_complex(SOPC_Variant* var,
                                      SOPC_BuiltinId type_id,
                                      parse_complex_value_tag_array_t tagsContext)
{
    bool ok = false;
    switch (type_id)
    {
    case SOPC_Boolean_Id:
    case SOPC_Float_Id:
    case SOPC_Double_Id:
    case SOPC_String_Id:
    case SOPC_ByteString_Id:
    case SOPC_XmlElement_Id: // TODO: not a simple type but not a complex one neither
        assert(false && "Unexpected simple type");
        break;
    case SOPC_QualifiedName_Id:
        assert(false && "QualifiedName not managed yet");
        break;
    case SOPC_Guid_Id:
        ok = set_variant_value_guid(&var->Value.Guid, tagsContext);
        break;
    case SOPC_LocalizedText_Id:
        ok = set_variant_value_localized_text(&var->Value.LocalizedText, tagsContext);
        break;
    case SOPC_NodeId_Id:
        ok = set_variant_value_nodeid(&var->Value.NodeId, tagsContext);
        break;
    case SOPC_ExtensionObject_Id:
        ok = set_variant_value_extensionobject(&var->Value.ExtObject, tagsContext);
        break;
    default:
        assert(false && "Cannot parse current value type.");
    }

    if (ok)
    {
        var->BuiltInTypeId = type_id;
    }

    return ok;
}

static bool set_element_value_scalar(struct parse_context_t* ctx)
{
    assert(ctx->current_array_type == SOPC_VariantArrayType_SingleValue);

    SOPC_Variant* var = SOPC_AddressSpace_Get_Value(ctx->space, &ctx->node);
    bool ok = false;
    if (PARSE_NODE_VALUE_SCALAR == ctx->state)
    {
        ok = set_variant_value(ctx, var, SOPC_HelperExpat_CharDataStripped(&ctx->helper_ctx));
        SOPC_HelperExpat_CharDataReset(&ctx->helper_ctx);
    }
    else if (PARSE_NODE_VALUE_SCALAR_COMPLEX == ctx->state)
    {
        ok = set_variant_value_complex(var, ctx->current_value_type, ctx->complex_value_ctx.tags);
    }

    if (ok)
    {
        ctx->node.value_status = SOPC_GoodGenericStatus;
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

    bool ok = false;
    if (PARSE_NODE_VALUE_SCALAR == ctx->state)
    {
        ok = set_variant_value(ctx, var, SOPC_HelperExpat_CharDataStripped(&ctx->helper_ctx));
        SOPC_HelperExpat_CharDataReset(&ctx->helper_ctx);
    }
    else if (PARSE_NODE_VALUE_SCALAR_COMPLEX == ctx->state)
    {
        ok = set_variant_value_complex(var, ctx->current_value_type, ctx->complex_value_ctx.tags);
    }

    if (!ok)
    {
        SOPC_Variant_Delete(var);
        return false;
    }

    bool appended = SOPC_Array_Append(ctx->list_nodes, *var);

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

static bool SOPC_Variant_MoveExtensionObjectInto_ArrayValueAt(const SOPC_Variant* var,
                                                              int32_t index,
                                                              SOPC_ExtensionObject* extObj)
{
    assert(SOPC_VariantArrayType_Array == var->ArrayType);
    assert(SOPC_ExtensionObject_Id == var->BuiltInTypeId);
    assert(var->Value.Array.Length > index);

    return SOPC_STATUS_OK == SOPC_ExtensionObject_Move(&var->Value.Array.Content.ExtObjectArr[index], extObj);
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
        if (SOPC_ExtensionObject_Id != builtInId)
        {
            res = SOPC_Variant_CopyInto_ArrayValueAt(var, builtInId, (int32_t) i, value);
        }
        else
        {
            // Note: specific case for ExtensionObject: if a copy is made the object is encoded as ByteString.
            //       We use Move to keep object format to have same format with static and dynamic parsing
            res = SOPC_Variant_MoveExtensionObjectInto_ArrayValueAt(var, (int32_t) i, lvar->Value.ExtObject);
        }
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
    assert(ctx->list_nodes != NULL);

    SOPC_Variant* var = SOPC_AddressSpace_Get_Value(ctx->space, &ctx->node);

    bool res = SOPC_Array_Of_Variant_Into_Variant_Array(ctx->list_nodes, ctx->current_value_type, var);

    SOPC_Array_Delete(ctx->list_nodes);
    ctx->list_nodes = NULL;
    ctx->node.value_status = SOPC_GoodGenericStatus;

    return res;
}

static bool finalize_node(struct parse_context_t* ctx)
{
    if (ctx->references != NULL)
    {
        size_t n_references = SOPC_Array_Size(ctx->references);
        assert(n_references <= INT32_MAX);

        *SOPC_AddressSpace_Get_NoOfReferences(ctx->space, &ctx->node) = (int32_t) n_references;
        *SOPC_AddressSpace_Get_References(ctx->space, &ctx->node) = SOPC_Array_Into_Raw(ctx->references);
        ctx->references = NULL;
    }

    SOPC_AddressSpace_Node* node = SOPC_Calloc(1, sizeof(SOPC_AddressSpace_Node));

    if (node == NULL)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }

    memcpy(node, &ctx->node, sizeof(SOPC_AddressSpace_Node));

    if (SOPC_AddressSpace_Append(ctx->space, node) == SOPC_STATUS_OK)
    {
        return true;
    }
    else
    {
        SOPC_Free(node);
        return false;
    }
}

static void end_of_single_value_parsing(struct parse_context_t* ctx)
{
    if (ctx->current_array_type == SOPC_VariantArrayType_SingleValue)
    {
        if (!set_element_value_scalar(ctx))
        {
            XML_StopParser(ctx->helper_ctx.parser, false);
            return;
        }

        ctx->current_value_type = SOPC_Null_Id;
        ctx->state = PARSE_NODE_VALUE;
    }
    else if (ctx->current_array_type == SOPC_VariantArrayType_Array)
    {
        if (!append_element_value(ctx))
        {
            XML_StopParser(ctx->helper_ctx.parser, false);
            return;
        }

        ctx->state = PARSE_NODE_VALUE_ARRAY;
    }
    else
    {
        assert(false);
    }
}

static bool end_in_extension_object(struct parse_context_t* ctx, parse_complex_value_tag_t* currentTagCtx)
{
    bool ok = false;
    char* typeIdChar = NULL;

    if (0 == strncmp("TypeId", currentTagCtx->name, strlen("TypedId")))
    {
        // We have to associate the body children tags with body if type is known

        // Retrieve identifier of nodeId
        parse_complex_value_tag_t* identifierTagCtx = NULL;
        bool id_tag_ok =
            complex_value_tag_from_tag_name_no_namespace("Identifier", currentTagCtx->childs, &identifierTagCtx);
        assert(id_tag_ok);

        SOPC_NodeId* nodeId =
            SOPC_NodeId_FromCString(identifierTagCtx->single_value, (int32_t) strlen(identifierTagCtx->single_value));
        typeIdChar = identifierTagCtx->single_value;
        // Check nodeId is in NS0
        if (NULL != nodeId && SOPC_IdentifierType_Numeric == nodeId->IdentifierType && 0 == nodeId->Namespace)
        {
            // Retrieve context of Body tag
            parse_complex_value_tag_t* bodyTagCtx = NULL;
            bool body_tag_ok =
                complex_value_tag_from_tag_name_no_namespace("Body", ctx->complex_value_ctx.tags, &bodyTagCtx);
            assert(body_tag_ok);
            // Fill the Body tag children context and retrieve encodeableType
            const void* encType = ext_obj_body_from_its_type_id(nodeId->Data.Numeric, &bodyTagCtx->childs);
            if (NULL != encType)
            {
                currentTagCtx->user_data = encType;
                currentTagCtx->set = true;
                ok = true;
            }
        }
        SOPC_NodeId_Clear(nodeId);
        SOPC_Free(nodeId);
    }
    else if (0 == strncmp("Identifier", currentTagCtx->name, strlen("Identifier")))
    {
        // Skip until end of TypeId
        ok = true;
    }
    else // end of Body shall not trigger this function (is_extension_object set to false on Body start)
    {
        assert(false);
    }

    if (!ok)
    {
        // Note: if the current tag ctx is the body, it means body is first in XML
        LOG_XML_ERRORF(ctx->helper_ctx.parser, "Unsupported extension object typeId '%s'", typeIdChar);
        // Clear all extension object (or list of) context
        clear_complex_value_context(&ctx->complex_value_ctx);
        ctx->current_value_type = SOPC_Null_Id;
        SOPC_Array_Delete(ctx->list_nodes);
        ctx->list_nodes = NULL;
        // Skip until end of the Value
        if (SOPC_VariantArrayType_SingleValue == ctx->current_array_type)
        {
            SOPC_HelperExpat_PushSkipTag(&ctx->helper_ctx, UA_EXTENSION_OBJECT_VALUE);
        }
        else if (SOPC_VariantArrayType_Array == ctx->current_array_type)
        {
            SOPC_HelperExpat_PushSkipTag(&ctx->helper_ctx, UA_LIST_EXTENSION_OBJECT_VALUE);
        }
        else
        {
            assert(false);
        }
        ctx->current_array_type = SOPC_VariantArrayType_SingleValue;
        ctx->state = PARSE_NODE_VALUE;
    }

    return ok;
}

static void end_element_handler(void* user_data, const XML_Char* name)
{
    struct parse_context_t* ctx = user_data;
    bool appended = false;
    SOPC_LocalizedText* lt;
    const char* stripped = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    bool ok = false;
    parse_complex_value_tag_t* currentTagCtx = NULL;

    if (SOPC_HelperExpat_PopSkipTag(&ctx->helper_ctx, name))
    {
        return;
    }

    switch (ctx->state)
    {
    case PARSE_ALIASES:
        ctx->state = PARSE_NODESET;
        break;
    case PARSE_ALIAS:
    {
        ok = finalize_alias(ctx);

        SOPC_Free(ctx->current_alias_alias);
        ctx->current_alias_alias = NULL;

        if (!ok)
        {
            XML_StopParser(ctx->helper_ctx.parser, false);
            return;
        }

        ctx->state = PARSE_ALIASES;
        break;
    }
    case PARSE_NODE_DISPLAYNAME:
    case PARSE_NODE_DESCRIPTION:
    {
        lt = element_localized_text_for_state(ctx);
        SOPC_String_Clear(&lt->defaultText);
        stripped = SOPC_HelperExpat_CharDataStripped(&ctx->helper_ctx);
        status = (strlen(stripped) == 0) ? SOPC_STATUS_OK : SOPC_String_CopyFromCString(&lt->defaultText, stripped);
        SOPC_HelperExpat_CharDataReset(&ctx->helper_ctx);

        if (status != SOPC_STATUS_OK)
        {
            LOG_MEMORY_ALLOCATION_FAILURE;
            XML_StopParser(ctx->helper_ctx.parser, false);
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

        end_of_single_value_parsing(ctx);

        break;
    case PARSE_NODE_VALUE_SCALAR_COMPLEX:
        assert(current_element_has_value(ctx));
        assert(NULL != ctx->complex_value_ctx.end_element_restore_context);
        // Restore context of current tag
        ctx->complex_value_ctx.tags = SOPC_SLinkedList_PopHead(ctx->complex_value_ctx.end_element_restore_context);
        assert(NULL != ctx->complex_value_ctx.tags);

        // Check if it is the complex value closing node
        if (0 == SOPC_SLinkedList_GetLength(ctx->complex_value_ctx.end_element_restore_context))
        {
            assert(strncmp(name, ctx->complex_value_ctx.value_tag, strlen(name)) == 0);

            end_of_single_value_parsing(ctx);
            clear_complex_value_context(&ctx->complex_value_ctx);
            return;
        }
        // else: it is still internal treatment of complex value

        /* Retrieve current tag name in expected children of parent tag*/
        if (!complex_value_tag_from_tag(name, ctx->complex_value_ctx.tags, &currentTagCtx))
        {
            LOG_XML_ERRORF(ctx->helper_ctx.parser, "Unexpected end tag in complex value: %s for BuiltInId type %d",
                           name, ctx->current_value_type);
            XML_StopParser(ctx->helper_ctx.parser, 0);
            return;
        }

        // We retrieve the tag content only if no children defined
        if (NULL == currentTagCtx->childs)
        {
            if (!currentTagCtx->is_array)
            {
                // Retrieve text value
                currentTagCtx->single_value = SOPC_strdup(SOPC_HelperExpat_CharDataStripped(&ctx->helper_ctx));
            }
            else
            {
                if (NULL == currentTagCtx->array_values)
                {
                    currentTagCtx->array_values = SOPC_Array_Create(sizeof(char*), 1, SOPC_Free);
                }
                char* str_value = SOPC_strdup(SOPC_HelperExpat_CharDataStripped(&ctx->helper_ctx));
                appended = SOPC_Array_Append_Values(currentTagCtx->array_values, str_value, 1);
                if (!appended)
                {
                    SOPC_Free(str_value);
                    LOG_MEMORY_ALLOCATION_FAILURE;
                    XML_StopParser(ctx->helper_ctx.parser, false);
                    return;
                }
            }

            currentTagCtx->set = true;
        }

        /* Special treatment of ExtensionObject case: "TypeId" => fill body children*/
        if (ctx->complex_value_ctx.is_extension_object)
        {
            if (!end_in_extension_object(ctx, currentTagCtx))
            {
                return;
            }
        }

        SOPC_HelperExpat_CharDataReset(&ctx->helper_ctx);

        break;
    case PARSE_NODE_VALUE_ARRAY:
        assert(ctx->current_array_type == SOPC_VariantArrayType_Array);

        if (!set_element_value_array(ctx))
        {
            XML_StopParser(ctx->helper_ctx.parser, false);
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
        ok = finalize_node(ctx);

        if (!ok)
        {
            SOPC_AddressSpace_Node_Clear(ctx->space, &ctx->node);
            XML_StopParser(ctx->helper_ctx.parser, false);
            return;
        }

        ctx->node.node_class = 0;
        ctx->state = PARSE_NODESET;
        break;
    }
    case PARSE_NODE_REFERENCE:
        if (!finalize_reference(ctx))
        {
            XML_StopParser(ctx->helper_ctx.parser, false);
            return;
        }

        ctx->state = PARSE_NODE_REFERENCES;
        break;
    case PARSE_NODESET:
        break;
    case PARSE_START:
        assert(false && "Got end_element callback when in PARSE_START state.");
        break;
    default:
        assert(false && "Unknown state.");
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
    case PARSE_NODE_VALUE_SCALAR_COMPLEX:
        if (!SOPC_HelperExpat_CharDataAppend(&ctx->helper_ctx, s, (size_t) len))
        {
            XML_StopParser(ctx->helper_ctx.parser, false);
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
    ctx.helper_ctx.parser = parser;
    ctx.space = space;
    ctx.helper_ctx.char_data_buffer = char_data_buffer;
    ctx.helper_ctx.char_data_cap = char_data_cap_initial;

    XML_SetElementHandler(parser, start_element_handler, end_element_handler);
    XML_SetCharacterDataHandler(parser, char_data_handler);

    SOPC_ReturnStatus res = parse(parser, fd);
    XML_ParserFree(parser);
    SOPC_Dict_Delete(aliases);
    SOPC_Free(ctx.current_alias_alias);
    SOPC_Free(ctx.helper_ctx.char_data_buffer);
    SOPC_Array_Delete(ctx.references);
    SOPC_Array_Delete(ctx.list_nodes);

    if (res == SOPC_STATUS_OK)
    {
        return space;
    }
    else
    {
        SOPC_AddressSpace_Node_Clear(ctx.space, &ctx.node);
        SOPC_AddressSpace_Delete(space);
        return NULL;
    }
}
