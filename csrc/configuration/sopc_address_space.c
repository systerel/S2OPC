/*
 *  Copyright (C) 2018 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "opcua_identifiers.h"
#include "opcua_statuscodes.h"
#include "sopc_address_space.h"
#include "sopc_dict.h"
#include "sopc_types.h"
#include "sopc_user_app_itf.h"

#define ELEMENT_ATTRIBUTE_INITIALIZE_CASE(val, field, extra) \
    case OpcUa_NodeClass_##val:                              \
        OpcUa_##val##Node_Initialize(&item->data.field);     \
        break;

#define ELEMENT_ATTRIBUTE_CLEAR_CASE(val, field, extra) \
    case OpcUa_NodeClass_##val:                         \
        OpcUa_##val##Node_Clear(&item->data.field);     \
        break;

void SOPC_AddressSpace_Description_Item_Initialize(SOPC_AddressSpace_Description_Item* item, OpcUa_NodeClass node_class)
{
    switch (node_class)
    {
        FOR_EACH_ELEMENT_TYPE(ELEMENT_ATTRIBUTE_INITIALIZE_CASE, NULL)
    default:
        assert(false && "Unknown element type");
    }

    item->node_class = node_class;
}

#define ELEMENT_ATTRIBUTE_GETTER_CASE(val, field, extra) \
    case OpcUa_NodeClass_##val:                          \
        return &item->data.field.extra;

#define ELEMENT_ATTRIBUTE_GETTER(ret_type, name)                                                      \
    ret_type* SOPC_AddressSpace_Description_Item_Get_##name(SOPC_AddressSpace_Description_Item* item) \
    {                                                                                                 \
        assert(item->node_class > 0);                                                                 \
        switch (item->node_class)                                                                     \
        {                                                                                             \
            FOR_EACH_ELEMENT_TYPE(ELEMENT_ATTRIBUTE_GETTER_CASE, name)                                \
        default:                                                                                      \
            assert(false && "Unknown element type");                                                  \
        }                                                                                             \
    }

ELEMENT_ATTRIBUTE_GETTER(SOPC_NodeId, NodeId)
ELEMENT_ATTRIBUTE_GETTER(SOPC_QualifiedName, BrowseName)
ELEMENT_ATTRIBUTE_GETTER(SOPC_LocalizedText, DisplayName)
ELEMENT_ATTRIBUTE_GETTER(SOPC_LocalizedText, Description)
ELEMENT_ATTRIBUTE_GETTER(int32_t, NoOfReferences)
ELEMENT_ATTRIBUTE_GETTER(OpcUa_ReferenceNode*, References)

SOPC_Variant* SOPC_AddressSpace_Description_Item_Get_Value(SOPC_AddressSpace_Description_Item* item)
{
    switch (item->node_class)
    {
    case OpcUa_NodeClass_Variable:
        return &item->data.variable.Value;
    case OpcUa_NodeClass_VariableType:
        return &item->data.variable.Value;
    default:
        assert(false && "Current element has no value.");
    }
}

SOPC_Byte* SOPC_AddressSpace_Description_Item_Get_AccessLevel(SOPC_AddressSpace_Description_Item* item)
{
    switch (item->node_class)
    {
    case OpcUa_NodeClass_Variable:
        return &item->data.variable.AccessLevel;
    case OpcUa_NodeClass_VariableType:
        return &item->data.variable.AccessLevel;
    default:
        assert(false && "Current element has no value.");
    }
}

void SOPC_AddressSpace_Description_Item_Clear(SOPC_AddressSpace_Description_Item* item)
{
    assert(item->node_class > 0);

    switch (item->node_class)
    {
        FOR_EACH_ELEMENT_TYPE(ELEMENT_ATTRIBUTE_CLEAR_CASE, NULL)
    default:
        assert(false && "Unknown element type");
    }
}

static void free_description_item(void* data)
{
    SOPC_AddressSpace_Description_Item** item = data;
    SOPC_AddressSpace_Description_Item_Clear(*item);
    free(*item);
}

SOPC_AddressSpace_Description* SOPC_AddressSpace_Description_Create()
{
    return SOPC_Array_Create(sizeof(SOPC_AddressSpace_Description_Item*), 0, free_description_item);
}

SOPC_ReturnStatus SOPC_AddressSpace_Description_Append(SOPC_AddressSpace_Description* desc,
                                                       SOPC_AddressSpace_Description_Item* item)
{
    return SOPC_Array_Append(desc, item) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
}

static SOPC_AddressSpace_Description_Item* desc_get_item(const SOPC_AddressSpace_Description* desc, size_t i)
{
    SOPC_AddressSpace_Description_Item* item = SOPC_Array_Get(desc, SOPC_AddressSpace_Description_Item*, i);
    return item;
}

static void generate_node_counts(const SOPC_AddressSpace_Description* desc, SOPC_AddressSpace* space)
{
    size_t n_items = SOPC_Array_Size(desc);

    for (size_t i = 0; i < n_items; ++i)
    {
        SOPC_AddressSpace_Description_Item* item = desc_get_item(desc, i);

        switch (item->node_class)
        {
        case OpcUa_NodeClass_DataType:
            space->nbDataTypes++;
            break;
        case OpcUa_NodeClass_Method:
            space->nbMethods++;
            break;
        case OpcUa_NodeClass_Object:
            space->nbObjects++;
            break;
        case OpcUa_NodeClass_ObjectType:
            space->nbObjectTypes++;
            break;
        case OpcUa_NodeClass_ReferenceType:
            space->nbReferenceTypes++;
            break;
        case OpcUa_NodeClass_Variable:
            space->nbVariables++;
            break;
        case OpcUa_NodeClass_VariableType:
            space->nbVariableTypes++;
            break;
        case OpcUa_NodeClass_View:
            space->nbViews++;
            break;
        default:
            assert(false && "Unknown element type");
        }

        SOPC_LocalizedText* desc = SOPC_AddressSpace_Description_Item_Get_Description(item);

        if (desc->Text.Length > 0)
        {
            space->nbDescriptionsTotal++;
        }

        SOPC_LocalizedText* display_name = SOPC_AddressSpace_Description_Item_Get_DisplayName(item);

        if (display_name->Text.Length > 0)
        {
            space->nbDisplayNamesTotal++;
        }

        int32_t* n_refs = SOPC_AddressSpace_Description_Item_Get_NoOfReferences(item);
        assert(*n_refs >= 0);
        space->nbReferencesTotal += (size_t) *n_refs;
    }

    // Add the padding for the 0th element in attribute arrays
    space->nbDescriptionsTotal++;
    space->nbDisplayNamesTotal++;
    space->nbReferencesTotal++;

    space->nbNodesTotal = (uint32_t) n_items;
}

static bool generate_localized_text_attribute(SOPC_LocalizedText* dest_values,
                                              int* attr_begin_idx,
                                              int* attr_end_idx,
                                              size_t node_idx,
                                              size_t* attr_idx,
                                              SOPC_LocalizedText* value)
{
    assert(*attr_idx <= INT32_MAX);
    attr_begin_idx[node_idx] = (int32_t) *attr_idx;

    if (value->Text.Length > 0)
    {
        SOPC_LocalizedText* dest = dest_values + *attr_idx;

        if (SOPC_LocalizedText_Copy(dest, value) != SOPC_STATUS_OK)
        {
            return false;
        }

        (*attr_idx)++;
    }

    assert(*attr_idx <= INT32_MAX);
    attr_end_idx[node_idx] = (int32_t) *attr_idx;

    return true;
}

static bool generate_reference_attribute(SOPC_Dict* nodes_by_id,
                                         SOPC_NodeId** dest_types,
                                         SOPC_ExpandedNodeId** dest_targets,
                                         bool* dest_is_forward,
                                         int* attr_begin_idx,
                                         int* attr_end_idx,
                                         size_t node_idx,
                                         size_t* attr_idx,
                                         OpcUa_ReferenceNode* refs,
                                         size_t n_refs)
{
    assert(*attr_idx <= INT32_MAX);
    attr_begin_idx[node_idx] = (int32_t) *attr_idx;

    for (size_t i = 0; i < n_refs; ++i)
    {
        OpcUa_ReferenceNode* ref = &refs[i];

        bool found;
        SOPC_NodeId* type_id = SOPC_Dict_GetKey(nodes_by_id, &ref->ReferenceTypeId, &found);
        assert(found);
        dest_types[*attr_idx] = type_id;

        SOPC_NodeId* target_id = SOPC_Dict_GetKey(nodes_by_id, &ref->TargetId.NodeId, &found);
        assert(found);

        SOPC_ExpandedNodeId* target_id_copy = calloc(1, sizeof(SOPC_ExpandedNodeId));

        if ((target_id_copy == NULL) || (SOPC_NodeId_Copy(&target_id_copy->NodeId, target_id) != SOPC_STATUS_OK))
        {
            SOPC_ExpandedNodeId_Clear(target_id_copy);
            free(target_id_copy);
            return false;
        }

        dest_targets[*attr_idx] = target_id_copy;
        dest_is_forward[*attr_idx] = !ref->IsInverse;

        (*attr_idx)++;
    }

    assert(*attr_idx <= INT32_MAX);
    attr_end_idx[node_idx] = (int32_t) *attr_idx;

    return true;
}

static bool allocate_arrays(SOPC_AddressSpace* space)
{
    size_t n_items = space->nbNodesTotal;
    SOPC_NodeId** node_ids = calloc(1 + n_items, sizeof(SOPC_NodeId*));
    OpcUa_NodeClass* node_classes = calloc(1 + n_items, sizeof(OpcUa_NodeClass));
    SOPC_QualifiedName* browse_names = calloc(1 + n_items, sizeof(SOPC_QualifiedName));
    SOPC_Variant* values = calloc(1 + space->nbVariables + space->nbVariableTypes, sizeof(SOPC_Variant));
    SOPC_StatusCode* statuses = calloc(1 + space->nbVariables + space->nbVariableTypes, sizeof(SOPC_StatusCode));
    SOPC_Byte* access_levels = calloc(1 + space->nbVariables + space->nbVariableTypes, sizeof(SOPC_Byte));
    int* description_begin = calloc(1 + n_items, sizeof(int));
    int* description_end = calloc(1 + n_items, sizeof(int));
    SOPC_LocalizedText* descriptions = calloc(space->nbDescriptionsTotal, sizeof(SOPC_LocalizedText));
    int* display_name_begin = calloc(1 + n_items, sizeof(int));
    int* display_name_end = calloc(1 + n_items, sizeof(int));
    SOPC_LocalizedText* display_names = calloc(space->nbDisplayNamesTotal, sizeof(SOPC_LocalizedText));
    int* reference_begin = calloc(1 + n_items, sizeof(int));
    int* reference_end = calloc(1 + n_items, sizeof(int));
    SOPC_NodeId** reference_types = calloc(space->nbReferencesTotal, sizeof(SOPC_NodeId*));
    SOPC_ExpandedNodeId** reference_targets = calloc(space->nbReferencesTotal, sizeof(SOPC_ExpandedNodeId*));
    bool* reference_is_forward = calloc(space->nbReferencesTotal, sizeof(bool));

    if ((node_ids == NULL) || (browse_names == NULL) || (node_classes == NULL) || (values == NULL) ||
        (statuses == NULL) || (access_levels == NULL) || (description_begin == NULL) || (description_end == NULL) ||
        (descriptions == NULL) || (display_name_begin == NULL) || (display_name_end == NULL) ||
        (display_names == NULL) || (reference_begin == NULL) || (reference_end == NULL) || (reference_types == NULL) ||
        (reference_targets == NULL) || (reference_is_forward == NULL))
    {
        free(node_ids);
        free(node_classes);
        free(browse_names);
        free(values);
        free(statuses);
        free(access_levels);
        free(description_begin);
        free(description_end);
        free(descriptions);
        free(display_name_begin);
        free(display_name_end);
        free(display_names);
        free(reference_begin);
        free(reference_end);
        free(reference_types);
        free(reference_targets);
        free(reference_is_forward);
        return false;
    }

    space->nodeIdArray = node_ids;
    space->nodeClassArray = node_classes;
    space->browseNameArray = browse_names;
    space->valueArray = values;
    space->valueStatusArray = statuses;
    space->accessLevelArray = access_levels;
    space->descriptionIdxArray_begin = description_begin;
    space->descriptionIdxArray_end = description_end;
    space->descriptionArray = descriptions;
    space->displayNameIdxArray_begin = display_name_begin;
    space->displayNameIdxArray_end = display_name_end;
    space->displayNameArray = display_names;
    space->referenceIdxArray_begin = reference_begin;
    space->referenceIdxArray_end = reference_end;
    space->referenceTypeArray = reference_types;
    space->referenceTargetArray = reference_targets;
    space->referenceIsForwardArray = reference_is_forward;

    return true;
}

static bool generate_attributes(SOPC_Array* sorted_items, SOPC_AddressSpace* space, SOPC_Dict* nodes_by_id)
{
    size_t value_idx = 1;
    size_t description_idx = 1;
    size_t display_name_idx = 1;
    size_t reference_idx = 1;

    for (size_t i = 0; i < space->nbNodesTotal; ++i)
    {
        SOPC_AddressSpace_Description_Item* item = desc_get_item(sorted_items, i);

        // Get the interned value of the NodeId (the one we own)
        bool found;
        SOPC_NodeId* item_id = SOPC_AddressSpace_Description_Item_Get_NodeId(item);
        SOPC_NodeId* id = SOPC_Dict_GetKey(nodes_by_id, item_id, &found);
        assert(found);

        // NodeId
        space->nodeIdArray[i + 1] = id;

        // NodeClass
        space->nodeClassArray[i + 1] = item->node_class;

        // BrowseName
        SOPC_QualifiedName* browse_name = SOPC_AddressSpace_Description_Item_Get_BrowseName(item);

        if (SOPC_QualifiedName_Copy(&space->browseNameArray[i + 1], browse_name) != SOPC_STATUS_OK)
        {
            return false;
        }

        // Description
        SOPC_LocalizedText* description = SOPC_AddressSpace_Description_Item_Get_Description(item);

        if (!generate_localized_text_attribute(space->descriptionArray, space->descriptionIdxArray_begin,
                                               space->descriptionIdxArray_end, i + 1, &description_idx, description))
        {
            return false;
        }

        // DisplayName
        SOPC_LocalizedText* display_name = SOPC_AddressSpace_Description_Item_Get_DisplayName(item);

        if (!generate_localized_text_attribute(space->displayNameArray, space->displayNameIdxArray_begin,
                                               space->displayNameIdxArray_end, i + 1, &display_name_idx, display_name))
        {
            return false;
        }

        // References
        OpcUa_ReferenceNode** refs = SOPC_AddressSpace_Description_Item_Get_References(item);
        int32_t* n_refs = SOPC_AddressSpace_Description_Item_Get_NoOfReferences(item);

        if (!generate_reference_attribute(nodes_by_id, space->referenceTypeArray, space->referenceTargetArray,
                                          space->referenceIsForwardArray, space->referenceIdxArray_begin,
                                          space->referenceIdxArray_end, i + 1, &reference_idx, *refs, (size_t) *n_refs))
        {
            return false;
        }

        // Value
        if (item->node_class == OpcUa_NodeClass_Variable || item->node_class == OpcUa_NodeClass_VariableType)
        {
            SOPC_Variant* value = SOPC_AddressSpace_Description_Item_Get_Value(item);

            if (SOPC_Variant_Copy(&space->valueArray[value_idx], value) != SOPC_STATUS_OK)
            {
                return false;
            }

            space->valueStatusArray[value_idx] =
                ((value->BuiltInTypeId != SOPC_Null_Id) ? 0x00000000 : OpcUa_BadDataUnavailable);
            space->accessLevelArray[value_idx] = *SOPC_AddressSpace_Description_Item_Get_AccessLevel(item);

            value_idx++;
        }
    }

    return true;
}

// From the address space generation XSL stylesheet
static int node_class_ordinal(OpcUa_NodeClass klass)
{
    switch (klass)
    {
    case OpcUa_NodeClass_View:
        return 1;
    case OpcUa_NodeClass_Object:
        return 2;
    case OpcUa_NodeClass_Variable:
        return 3;
    case OpcUa_NodeClass_VariableType:
        return 4;
    case OpcUa_NodeClass_ObjectType:
        return 5;
    case OpcUa_NodeClass_ReferenceType:
        return 6;
    case OpcUa_NodeClass_DataType:
        return 7;
    case OpcUa_NodeClass_Method:
        return 8;
    default:
        assert(false && "Unknown node class");
    }
}

static int item_compare_func(const void* a, const void* b)
{
    const SOPC_AddressSpace_Description_Item* const* i1 = a;
    const SOPC_AddressSpace_Description_Item* const* i2 = b;

    return node_class_ordinal((*i1)->node_class) - node_class_ordinal((*i2)->node_class);
}

static uint64_t nodeid_hash(const void* id)
{
    uint64_t hash;
    SOPC_NodeId_Hash((const SOPC_NodeId*) id, &hash);
    return hash;
}

static bool nodeid_equal(const void* a, const void* b)
{
    int32_t cmp;
    SOPC_NodeId_Compare((const SOPC_NodeId*) a, (const SOPC_NodeId*) b, &cmp);

    return cmp == 0;
}

static void free_nodeid(void* data)
{
    if (data != NULL)
    {
        SOPC_NodeId_Clear((SOPC_NodeId*) data);
        free(data);
    }
}

SOPC_Dict* map_node_ids(const SOPC_AddressSpace_Description* desc)
{
    SOPC_Dict* dict = SOPC_Dict_Create(NULL, nodeid_hash, nodeid_equal, free_nodeid, NULL);

    if (dict == NULL)
    {
        return NULL;
    }

    size_t desc_size = SOPC_Array_Size(desc);

    for (size_t i = 0; i < desc_size; ++i)
    {
        SOPC_AddressSpace_Description_Item* item = desc_get_item(desc, i);
        SOPC_NodeId* id = SOPC_AddressSpace_Description_Item_Get_NodeId(item);

        // Small verification to check NodeId uniqueness
        bool found;
        SOPC_Dict_Get(dict, id, &found);
        assert(!found);

        // We keep a copy of the NodeId that we own
        SOPC_NodeId* id_copy = calloc(1, sizeof(SOPC_NodeId));

        if ((id_copy == NULL) || (SOPC_NodeId_Copy(id_copy, id) != SOPC_STATUS_OK) ||
            !SOPC_Dict_Insert(dict, id_copy, item))
        {
            SOPC_Dict_Delete(dict);
            return NULL;
        }
    }

    return dict;
}

static bool resolve_external_ids_helper(SOPC_NodeId* id,
                                        SOPC_Dict* nodes_by_id,
                                        SOPC_Array* external_ids,
                                        SOPC_Dict* known_external_ids)
{
    bool found;

    SOPC_Dict_Get(nodes_by_id, id, &found);

    if (found)
    {
        return true;
    }

    SOPC_Dict_Get(known_external_ids, id, &found);

    if (found)
    {
        return true;
    }

    return SOPC_Dict_Insert(known_external_ids, id, NULL) && SOPC_Array_Append(external_ids, id);
}

static bool resolve_external_ids(const SOPC_AddressSpace_Description* desc,
                                 SOPC_Dict* nodes_by_id,
                                 SOPC_Array* external_ids)
{
    SOPC_Dict* known_external_ids = SOPC_Dict_Create(NULL, nodeid_hash, nodeid_equal, NULL, NULL);

    if (known_external_ids == NULL)
    {
        return false;
    }

    size_t n_items = SOPC_Array_Size(desc);

    for (size_t i = 0; i < n_items; ++i)
    {
        SOPC_AddressSpace_Description_Item* item = desc_get_item(desc, i);
        int32_t* n_refs = SOPC_AddressSpace_Description_Item_Get_NoOfReferences(item);
        OpcUa_ReferenceNode** refs = SOPC_AddressSpace_Description_Item_Get_References(item);

        for (int32_t i = 0; i < *n_refs; ++i)
        {
            OpcUa_ReferenceNode* ref = &(*refs)[i];
            SOPC_NodeId* type_id = &ref->ReferenceTypeId;
            SOPC_NodeId* target_id = &ref->TargetId.NodeId;

            if (!resolve_external_ids_helper(type_id, nodes_by_id, external_ids, known_external_ids) ||
                !resolve_external_ids_helper(target_id, nodes_by_id, external_ids, known_external_ids))
            {
                SOPC_Dict_Delete(known_external_ids);
                return false;
            }
        }
    }

    SOPC_Dict_Delete(known_external_ids);

    return true;
}

static bool add_external_ids(SOPC_Array* external_ids, SOPC_AddressSpace* space, SOPC_Dict* nodes_by_id)
{
    size_t n_external_ids = SOPC_Array_Size(external_ids);
    space->externalNodeIds = calloc(n_external_ids, sizeof(SOPC_NodeId*));

    if (space->externalNodeIds == NULL)
    {
        return false;
    }

    space->nbExternalNodeIds = n_external_ids;

    for (size_t i = 0; i < n_external_ids; ++i)
    {
        SOPC_NodeId* id = SOPC_Array_Get(external_ids, SOPC_NodeId*, i);
        SOPC_NodeId* id_copy = calloc(1, sizeof(SOPC_NodeId));

        if ((id_copy == NULL) || (SOPC_NodeId_Copy(id_copy, id) != SOPC_STATUS_OK) ||
            (!SOPC_Dict_Insert(nodes_by_id, id_copy, NULL)))
        {
            return false;
        }

        space->externalNodeIds[i] = id_copy;
    }

    return true;
}

SOPC_ReturnStatus SOPC_AddressSpace_Generate(const SOPC_AddressSpace_Description* desc, SOPC_AddressSpace* space)
{
    if (SOPC_Array_Size(desc) > UINT32_MAX)
    {
        // SOPC_AddressSpace uses uint32 for node counts
        return SOPC_STATUS_NOK;
    }

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    // This map owns the NodeIds but not the nodes
    SOPC_Dict* nodes_by_id = map_node_ids(desc);

    // Does not own the NodeIds
    SOPC_Array* external_ids = SOPC_Array_Create(sizeof(SOPC_NodeId*), 0, NULL);

    if ((nodes_by_id == NULL) || (external_ids == NULL) || !resolve_external_ids(desc, nodes_by_id, external_ids))
    {
        SOPC_Dict_Delete(nodes_by_id);
        SOPC_Array_Delete(external_ids);
        return SOPC_STATUS_NOK;
    }

    // Sort items
    SOPC_Array* sorted_items = SOPC_Array_Copy(desc);

    if (sorted_items == NULL)
    {
        SOPC_Dict_Delete(nodes_by_id);
        SOPC_Array_Delete(external_ids);
        return SOPC_STATUS_NOK;
    }

    // The data will be freed by the original array
    SOPC_Array_Set_Free_Func(sorted_items, NULL);
    SOPC_Array_Sort(sorted_items, item_compare_func);

    // Generate arrays
    memset(space, 0, sizeof(SOPC_AddressSpace));

    generate_node_counts(desc, space);

    if (!add_external_ids(external_ids, space, nodes_by_id) || !allocate_arrays(space) ||
        !generate_attributes(sorted_items, space, nodes_by_id))
    {
        SOPC_AddressSpace_Clear(space);
        status = SOPC_STATUS_NOK;
    }

    SOPC_Array_Delete(sorted_items);
    SOPC_Array_Delete(external_ids);

    // The internal NodeIds now belong to the address space, we don't want to
    // free them.
    SOPC_Dict_SetKeyFreeFunc(nodes_by_id, NULL);
    SOPC_Dict_Delete(nodes_by_id);

    return status;
}

void SOPC_AddressSpace_Description_Delete(SOPC_AddressSpace_Description* desc)
{
    SOPC_Array_Delete(desc);
}

void SOPC_AddressSpace_Clear(SOPC_AddressSpace* space)
{
    if (space == NULL)
    {
        return;
    }

    if (space->browseNameArray != NULL)
    {
        for (size_t i = 1; i <= space->nbNodesTotal; ++i)
        {
            SOPC_QualifiedName_Clear(&space->browseNameArray[i]);
        }

        free(space->browseNameArray);
    }

    free(space->descriptionIdxArray_begin);
    free(space->descriptionIdxArray_end);

    if (space->descriptionArray != NULL)
    {
        for (size_t i = 1; i < space->nbDescriptionsTotal; ++i)
        {
            SOPC_LocalizedText_Clear(&space->descriptionArray[i]);
        }

        free(space->descriptionArray);
    }

    free(space->displayNameIdxArray_begin);
    free(space->displayNameIdxArray_end);

    if (space->displayNameArray != NULL)
    {
        for (size_t i = 1; i < space->nbDisplayNamesTotal; ++i)
        {
            SOPC_LocalizedText_Clear(&space->displayNameArray[i]);
        }

        free(space->displayNameArray);
    }

    free(space->nodeClassArray);

    if (space->nodeIdArray != NULL)
    {
        for (size_t i = 1; i <= space->nbNodesTotal; ++i)
        {
            SOPC_NodeId_Clear(space->nodeIdArray[i]);
            free(space->nodeIdArray[i]);
        }

        free(space->nodeIdArray);
    }

    free(space->referenceIdxArray_begin);
    free(space->referenceIdxArray_end);

    if (space->referenceTargetArray != NULL)
    {
        for (size_t i = 1; i < space->nbReferencesTotal; ++i)
        {
            SOPC_ExpandedNodeId_Clear(space->referenceTargetArray[i]);
            free(space->referenceTargetArray[i]);
        }

        free(space->referenceTargetArray);
    }

    free(space->referenceTypeArray); // NodeIds were freed with the NodeId list
    free(space->referenceIsForwardArray);

    if (space->valueArray != NULL)
    {
        size_t nb_values = 1 + space->nbVariables + space->nbVariableTypes;

        for (size_t i = 1; i < nb_values; ++i)
        {
            SOPC_Variant_Clear(&space->valueArray[i]);
        }

        free(space->valueArray);
    }

    free(space->valueStatusArray);
    free(space->accessLevelArray);

    if (space->externalNodeIds != NULL)
    {
        for (size_t i = 0; i < space->nbExternalNodeIds; ++i)
        {
            SOPC_NodeId_Clear(space->externalNodeIds[i]);
            free(space->externalNodeIds[i]);
        }

        free(space->externalNodeIds);
    }
}
