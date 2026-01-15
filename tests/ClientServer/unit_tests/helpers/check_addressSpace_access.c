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

#include <check.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include "check_helpers.h"
#include "embedded/sopc_addspace_loader.h"

#include "opcua_identifiers.h"
#include "opcua_statuscodes.h"

#include "sopc_address_space.h"
#include "sopc_address_space_access.h"
#include "sopc_address_space_access_internal.h"
#include "sopc_mem_alloc.h"
#include "sopc_toolkit_config.h"

#if 0 != S2OPC_NODE_MANAGEMENT
START_TEST(test_read_attribute)
{
    SOPC_AddressSpace* addressSpace = SOPC_Embedded_AddressSpace_LoadWithAlloc(true);
    ck_assert_ptr_nonnull(addressSpace);
    SOPC_AddressSpaceAccess* addSpaceAccess = SOPC_AddressSpaceAccess_Create(addressSpace, false);
    ck_assert_ptr_nonnull(addSpaceAccess);
    const char* nodeIdString = "i=85";
    SOPC_NodeId* nodeId = SOPC_NodeId_FromCString(nodeIdString);
    ck_assert_ptr_nonnull(nodeId);
    SOPC_Variant* outValue = NULL;
    SOPC_StatusCode status =
        SOPC_AddressSpaceAccess_ReadAttribute(addSpaceAccess, nodeId, SOPC_AttributeId_BrowseName, &outValue);
    ck_assert_uint_eq(SOPC_GoodGenericStatus, status);
    ck_assert_ptr_nonnull(outValue);
    SOPC_String* variantValue = &outValue->Value.Qname->Name;
    ck_assert_ptr_nonnull(variantValue);

    char* actualResult = SOPC_String_GetCString(variantValue);
    ck_assert_ptr_nonnull(actualResult);
    ck_assert_str_eq("Objects", actualResult);

    SOPC_Variant_Clear(outValue);
    SOPC_Free(outValue);
    SOPC_NodeId_Clear(nodeId);
    SOPC_Free(nodeId);
    SOPC_Free(actualResult);
    SOPC_AddressSpace_Delete(addressSpace);
    SOPC_AddressSpaceAccess_Delete(&addSpaceAccess);
}
END_TEST

START_TEST(test_read_value)
{
    SOPC_AddressSpace* addressSpace = SOPC_Embedded_AddressSpace_LoadWithAlloc(true);
    ck_assert_ptr_nonnull(addressSpace);
    SOPC_AddressSpaceAccess* addSpaceAccess = SOPC_AddressSpaceAccess_Create(addressSpace, false);
    ck_assert_ptr_nonnull(addSpaceAccess);
    const char* nodeIdString = "i=11511";
    SOPC_NodeId* nodeId = SOPC_NodeId_FromCString(nodeIdString);
    ck_assert_ptr_nonnull(nodeId);
    SOPC_DataValue* outValue = NULL;
    SOPC_StatusCode status = SOPC_AddressSpaceAccess_ReadValue(addSpaceAccess, nodeId, NULL, &outValue);
    ck_assert_uint_eq(SOPC_GoodGenericStatus, status);
    ck_assert_ptr_nonnull(outValue);
    uint32_t variantValue = outValue->Value.Value.Uint32;
    ck_assert_uint_eq(1, variantValue);

    SOPC_Free(outValue);
    SOPC_NodeId_Clear(nodeId);
    SOPC_Free(nodeId);
    SOPC_AddressSpace_Delete(addressSpace);
    SOPC_AddressSpaceAccess_Delete(&addSpaceAccess);
}
END_TEST

START_TEST(test_write_value)
{
    SOPC_AddressSpace* addressSpace = SOPC_Embedded_AddressSpace_LoadWithAlloc(true);
    ck_assert_ptr_nonnull(addressSpace);
    SOPC_AddressSpaceAccess* addSpaceAccess = SOPC_AddressSpaceAccess_Create(addressSpace, false);
    ck_assert_ptr_nonnull(addSpaceAccess);
    const char* nodeIdString = "i=2735";
    SOPC_NodeId* nodeId = SOPC_NodeId_FromCString(nodeIdString);
    ck_assert_ptr_nonnull(nodeId);
    SOPC_Variant* writeValue = SOPC_Variant_Create();
    ck_assert_ptr_nonnull(writeValue);
    writeValue->Value.Uint16 = 10;
    writeValue->BuiltInTypeId = SOPC_Int16_Id;
    writeValue->ArrayType = SOPC_VariantArrayType_SingleValue;

    SOPC_DataValue* outValue = NULL;
    SOPC_StatusCode status = SOPC_AddressSpaceAccess_ReadValue(addSpaceAccess, nodeId, NULL, &outValue);
    ck_assert_uint_eq(SOPC_GoodGenericStatus, status);
    ck_assert_ptr_nonnull(outValue);
    uint16_t variantValue = outValue->Value.Value.Uint16;
    ck_assert_int_eq(0, variantValue);

    status = SOPC_AddressSpaceAccess_WriteValue(addSpaceAccess, nodeId, NULL, writeValue, NULL, NULL, NULL);
    ck_assert_uint_eq(SOPC_GoodGenericStatus, status);

    ck_assert_ptr_nonnull(outValue);
    SOPC_DataValue_Clear(outValue);
    SOPC_Free(outValue);

    status = SOPC_AddressSpaceAccess_ReadValue(addSpaceAccess, nodeId, NULL, &outValue);
    ck_assert_uint_eq(SOPC_GoodGenericStatus, status);
    ck_assert_ptr_nonnull(outValue);
    variantValue = outValue->Value.Value.Uint16;
    ck_assert_uint_eq(10, variantValue);

    SOPC_DataValue_Clear(outValue);
    SOPC_Free(outValue);
    SOPC_NodeId_Clear(nodeId);
    SOPC_Free(nodeId);
    SOPC_Variant_Clear(writeValue);
    SOPC_Free(writeValue);
    SOPC_AddressSpace_Delete(addressSpace);
    SOPC_AddressSpaceAccess_Delete(&addSpaceAccess);
}
END_TEST

START_TEST(test_add_variable_node)
{
    SOPC_AddressSpace* addressSpace = SOPC_Embedded_AddressSpace_LoadWithAlloc(true);
    ck_assert_ptr_nonnull(addressSpace);
    SOPC_AddressSpaceAccess* addSpaceAccess = SOPC_AddressSpaceAccess_Create(addressSpace, false);
    ck_assert_ptr_nonnull(addSpaceAccess);

    SOPC_ExpandedNodeId parentNodeId;
    SOPC_ExpandedNodeId_Initialize(&parentNodeId);
    const char* nodeIdString = "i=2996";
    SOPC_NodeId* tmpNodeId = SOPC_NodeId_FromCString(nodeIdString);
    parentNodeId.NodeId = *tmpNodeId;

    nodeIdString = "i=1111";
    SOPC_NodeId* nodeId = SOPC_NodeId_FromCString(nodeIdString);
    ck_assert_ptr_nonnull(nodeId);
    SOPC_ExpandedNodeId typeDefId;
    SOPC_ExpandedNodeId_Initialize(&typeDefId);
    const SOPC_NodeId BaseDataVariableType = SOPC_NODEID_NS0_NUMERIC(OpcUaId_BaseDataVariableType);
    typeDefId.NodeId = BaseDataVariableType;

    SOPC_QualifiedName browseName;
    SOPC_QualifiedName_Initialize(&browseName);
    browseName.NamespaceIndex = 1;
    SOPC_ReturnStatus status = SOPC_String_AttachFromCstring(&browseName.Name, "ExampleNode");
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    OpcUa_VariableAttributes varAttributes;
    OpcUa_VariableAttributes_Initialize(&varAttributes);
    varAttributes.SpecifiedAttributes = OpcUa_NodeAttributesMask_DataType | OpcUa_NodeAttributesMask_AccessLevel |
                                        OpcUa_NodeAttributesMask_Value | OpcUa_NodeAttributesMask_DisplayName;
    varAttributes.AccessLevel = 1;
    SOPC_Variant_Initialize(&varAttributes.Value);
    varAttributes.Value.BuiltInTypeId = SOPC_Boolean_Id;
    varAttributes.Value.Value.Boolean = true;
    varAttributes.Value.ArrayType = SOPC_VariantArrayType_SingleValue;
    const SOPC_NodeId datatype_boolean = SOPC_NODEID_NS0_NUMERIC(OpcUaId_Boolean);
    varAttributes.DataType = datatype_boolean;

    SOPC_String name;
    SOPC_String_Initialize(&name);
    status = SOPC_String_AttachFromCstring(&name, "TestDisplayName");
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    SOPC_LocalizedText displayName;
    SOPC_LocalizedText_Initialize(&displayName);
    displayName.defaultText = name;
    varAttributes.DisplayName = displayName;

    const SOPC_NodeId HasComponentType = SOPC_NODEID_NS0_NUMERIC(OpcUaId_HasComponent);
    SOPC_StatusCode statusCode = SOPC_AddressSpaceAccess_AddVariableNode(
        addSpaceAccess, &parentNodeId, &HasComponentType, nodeId, &browseName, &varAttributes, &typeDefId, false);
    ck_assert_uint_eq(SOPC_GoodGenericStatus, statusCode);

    SOPC_DataValue* outValue = NULL;
    statusCode = SOPC_AddressSpaceAccess_ReadValue(addSpaceAccess, nodeId, NULL, &outValue);
    ck_assert_uint_eq(SOPC_GoodGenericStatus, statusCode);
    ck_assert_ptr_nonnull(outValue);
    SOPC_Boolean variantValue = outValue->Value.Value.Boolean;
    ck_assert(variantValue);
    SOPC_Variant* outValueAttribute = NULL;

    statusCode =
        SOPC_AddressSpaceAccess_ReadAttribute(addSpaceAccess, nodeId, SOPC_AttributeId_DisplayName, &outValueAttribute);
    ck_assert_uint_eq(SOPC_GoodGenericStatus, statusCode);
    ck_assert_ptr_nonnull(outValueAttribute);
    ck_assert_ptr_nonnull(outValueAttribute->Value.LocalizedText);
    SOPC_String* defaultText = &outValueAttribute->Value.LocalizedText->defaultText;
    ck_assert_ptr_nonnull(defaultText);

    char* actualResult = SOPC_String_GetCString(defaultText);
    ck_assert_ptr_nonnull(actualResult);
    ck_assert_str_eq("TestDisplayName", actualResult);

    SOPC_LocalizedText_Clear(&displayName);
    SOPC_Free(actualResult);
    SOPC_DataValue_Clear(outValue);
    SOPC_Free(outValue);
    SOPC_Variant_Clear(outValueAttribute);
    SOPC_Free(outValueAttribute);
    SOPC_Boolean_Clear(&variantValue);
    SOPC_NodeId_Clear(nodeId);
    SOPC_Free(nodeId);
    SOPC_NodeId_Clear(tmpNodeId);
    SOPC_Free(tmpNodeId);
    SOPC_AddressSpace_Delete(addressSpace);
    SOPC_AddressSpaceAccess_Delete(&addSpaceAccess);
}
END_TEST

START_TEST(test_add_object_node)
{
    SOPC_AddressSpace* addressSpace = SOPC_Embedded_AddressSpace_LoadWithAlloc(true);
    ck_assert_ptr_nonnull(addressSpace);
    SOPC_AddressSpaceAccess* addSpaceAccess = SOPC_AddressSpaceAccess_Create(addressSpace, false);
    ck_assert_ptr_nonnull(addSpaceAccess);

    SOPC_ExpandedNodeId parentNodeId;
    SOPC_ExpandedNodeId_Initialize(&parentNodeId);

    const char* nodeIdString = "i=2268";
    SOPC_NodeId* tmpNodeId = SOPC_NodeId_FromCString(nodeIdString);
    parentNodeId.NodeId = *tmpNodeId;

    nodeIdString = "i=7000";
    SOPC_NodeId* nodeId = SOPC_NodeId_FromCString(nodeIdString);
    ck_assert_ptr_nonnull(nodeId);

    SOPC_QualifiedName browseName;
    SOPC_QualifiedName_Initialize(&browseName);
    browseName.NamespaceIndex = 1;
    SOPC_String name;
    SOPC_String_Initialize(&name);
    SOPC_ReturnStatus status = SOPC_String_AttachFromCstring(&name, "ExampleObjectNode");
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    browseName.Name = name;

    SOPC_ExpandedNodeId typeDefId;
    SOPC_ExpandedNodeId_Initialize(&typeDefId);
    const SOPC_NodeId BaseObjectVariableType = SOPC_NODEID_NS0_NUMERIC(OpcUaId_BaseObjectType);
    typeDefId.NodeId = BaseObjectVariableType;

    const SOPC_NodeId OrganizesType = SOPC_NODEID_NS0_NUMERIC(OpcUaId_Organizes);

    SOPC_LocalizedText displayName;
    SOPC_LocalizedText_Initialize(&displayName);
    displayName.defaultText = name;

    OpcUa_ObjectAttributes objAttributes;
    OpcUa_ObjectAttributes_Initialize(&objAttributes);
    objAttributes.encodeableType = &OpcUa_ObjectNode_EncodeableType;
    objAttributes.DisplayName = displayName;

    SOPC_StatusCode statusCode = SOPC_AddressSpaceAccess_AddObjectNode(
        addSpaceAccess, &parentNodeId, &OrganizesType, nodeId, &browseName, &objAttributes, &typeDefId, false);
    ck_assert_uint_eq(SOPC_GoodGenericStatus, statusCode);

    SOPC_Variant* outValue = NULL;
    statusCode = SOPC_AddressSpaceAccess_ReadAttribute(addSpaceAccess, nodeId, SOPC_AttributeId_DisplayName, &outValue);
    ck_assert_uint_eq(SOPC_GoodGenericStatus, statusCode);
    ck_assert_ptr_nonnull(outValue);
    SOPC_String* defaultText = &outValue->Value.LocalizedText->defaultText;
    ck_assert_ptr_nonnull(defaultText);

    char* actualResult = SOPC_String_GetCString(defaultText);
    ck_assert_ptr_nonnull(actualResult);
    ck_assert_str_eq("ExampleObjectNode", actualResult);

    SOPC_Variant_Clear(outValue);
    SOPC_Free(outValue);
    SOPC_NodeId_Clear(nodeId);
    SOPC_Free(nodeId);
    SOPC_NodeId_Clear(tmpNodeId);
    SOPC_Free(tmpNodeId);
    SOPC_Free(actualResult);
    SOPC_AddressSpace_Delete(addressSpace);
    SOPC_AddressSpaceAccess_Delete(&addSpaceAccess);
}
END_TEST

START_TEST(test_translate_browse_path)
{
    SOPC_AddressSpace* addressSpace = SOPC_Embedded_AddressSpace_LoadWithAlloc(true);
    ck_assert_ptr_nonnull(addressSpace);
    SOPC_AddressSpaceAccess* addSpaceAccess = SOPC_AddressSpaceAccess_Create(addressSpace, false);
    ck_assert_ptr_nonnull(addSpaceAccess);

    const char* nodeIdString = "i=86";
    SOPC_NodeId* startNodeId = SOPC_NodeId_FromCString(nodeIdString);
    ck_assert_ptr_nonnull(startNodeId);
    const SOPC_NodeId* targetNodeId = NULL;

    SOPC_QualifiedName browseName = SOPC_QUALIFIED_NAME(0, "InterfaceTypes");

    OpcUa_RelativePath path;
    OpcUa_RelativePath_Initialize(&path);
    const SOPC_NodeId OrganizesType = SOPC_NODEID_NS0_NUMERIC(OpcUaId_Organizes);
    path.NoOfElements = 1;
    path.Elements = SOPC_Calloc((size_t) path.NoOfElements, sizeof(*path.Elements));
    ck_assert_ptr_nonnull(path.Elements);
    path.Elements[0].TargetName = browseName;
    path.Elements[0].IsInverse = false;
    path.Elements[0].ReferenceTypeId = OrganizesType;
    path.Elements[0].IncludeSubtypes = true;

    SOPC_StatusCode statusCode =
        SOPC_AddressSpaceAccess_TranslateBrowsePath(addSpaceAccess, startNodeId, &path, &targetNodeId);
    ck_assert_uint_eq(SOPC_GoodGenericStatus, statusCode);

    char* actualResult = SOPC_NodeId_ToCString(targetNodeId);
    ck_assert_ptr_nonnull(actualResult);
    ck_assert_str_eq("i=17708", actualResult);

    SOPC_NodeId_Clear(startNodeId);
    SOPC_Free(startNodeId);
    SOPC_Free(actualResult);
    SOPC_AddressSpace_Delete(addressSpace);
    SOPC_AddressSpaceAccess_Delete(&addSpaceAccess);
    OpcUa_RelativePath_Clear(&path);
}
END_TEST

START_TEST(test_browse_node)
{
    SOPC_AddressSpace* addressSpace = SOPC_Embedded_AddressSpace_LoadWithAlloc(true);
    ck_assert_ptr_nonnull(addressSpace);
    SOPC_AddressSpaceAccess* addSpaceAccess = SOPC_AddressSpaceAccess_Create(addressSpace, false);
    ck_assert_ptr_nonnull(addSpaceAccess);

    const char* nodeIdString = "i=86";
    SOPC_NodeId* nodeId = SOPC_NodeId_FromCString(nodeIdString);
    ck_assert_ptr_nonnull(nodeId);
    const SOPC_NodeId OrganizesType = SOPC_NODEID_NS0_NUMERIC(OpcUaId_Organizes);
    OpcUa_ReferenceDescription* references = NULL;
    int32_t nbOfReferences = 0;

    SOPC_StatusCode statusCode = SOPC_AddressSpaceAccess_BrowseNode(
        addSpaceAccess, nodeId, OpcUa_BrowseDirection_Both, &OrganizesType, false, 0, 0, &references, &nbOfReferences);
    ck_assert_uint_eq(SOPC_GoodGenericStatus, statusCode);
    ck_assert_ptr_nonnull(&references);

    ck_assert_int_eq(nbOfReferences, 7);

    OpcUa_ReferenceDescription* ref = &references[0];
    char* firstRefNodeId = SOPC_NodeId_ToCString(&ref->NodeId.NodeId);
    ck_assert_str_eq(firstRefNodeId, "i=84");

    SOPC_Clear_Array(&nbOfReferences, (void**) &references, sizeof(*references), OpcUa_ReferenceDescription_Clear);
    SOPC_Free(references);
    SOPC_NodeId_Clear(nodeId);
    SOPC_Free(nodeId);
    SOPC_Free(firstRefNodeId);
    SOPC_AddressSpace_Delete(addressSpace);
    SOPC_AddressSpaceAccess_Delete(&addSpaceAccess);
}
END_TEST

START_TEST(test_delete_child_nodes)
{
    SOPC_AddressSpace* addressSpace = SOPC_Embedded_AddressSpace_LoadWithAlloc(true);
    ck_assert_ptr_nonnull(addressSpace);
    SOPC_AddressSpaceAccess* addSpaceAccess = SOPC_AddressSpaceAccess_Create(addressSpace, false);
    ck_assert_ptr_nonnull(addSpaceAccess);

    const char* nodeToDelete = "ns=1;s=AnalogItemArrayInt32";
    const char* childOfNodeToDelete = "ns=1;s=AnalogItemArrayInt32EURange";
    SOPC_NodeId* nodeId = SOPC_NodeId_FromCString(nodeToDelete);
    ck_assert_ptr_nonnull(nodeId);

    SOPC_StatusCode statusCode = SOPC_AddressSpaceAccess_DeleteNode(addSpaceAccess, nodeId, true, false);
    ck_assert_uint_eq(OpcUa_UncertainReferenceNotDeleted, statusCode);

    // Try to browse the node -> invalid
    const SOPC_NodeId referencesType = SOPC_NODEID_NS0_NUMERIC(OpcUaId_References);
    OpcUa_ReferenceDescription* references = NULL;
    int32_t nbOfReferences = 0;
    statusCode = SOPC_AddressSpaceAccess_BrowseNode(addSpaceAccess, nodeId, OpcUa_BrowseDirection_Both, &referencesType,
                                                    true, 0, 0, &references, &nbOfReferences);
    ck_assert_uint_eq(OpcUa_BadNodeIdUnknown, statusCode);
    ck_assert(NULL == references);
    ck_assert_int_eq(nbOfReferences, 0);

    // Try to browse the child node -> ok
    SOPC_NodeId* childNodeId = SOPC_NodeId_FromCString(childOfNodeToDelete);
    statusCode = SOPC_AddressSpaceAccess_BrowseNode(addSpaceAccess, childNodeId, OpcUa_BrowseDirection_Both,
                                                    &referencesType, true, 0, 0, &references, &nbOfReferences);
    ck_assert_uint_eq(SOPC_GoodGenericStatus, statusCode);
    ck_assert_ptr_nonnull(references);
    ck_assert_int_eq(nbOfReferences, 1); // We used TargetDeleteReference so we deleted the reference to its parent, it
                                         // remains only the reference to its type.
    SOPC_Clear_Array(&nbOfReferences, (void**) &references, sizeof(*references), OpcUa_ReferenceDescription_Clear);
    SOPC_Free(references);

    SOPC_NodeId_Clear(nodeId);
    SOPC_Free(nodeId);
    SOPC_NodeId_Clear(childNodeId);
    SOPC_Free(childNodeId);

    SOPC_AddressSpace_Delete(addressSpace);
    SOPC_AddressSpaceAccess_Delete(&addSpaceAccess);
}
END_TEST

START_TEST(test_add_method_node)
{
    SOPC_AddressSpace* addressSpace = SOPC_Embedded_AddressSpace_LoadWithAlloc(true);
    ck_assert_ptr_nonnull(addressSpace);
    SOPC_AddressSpaceAccess* addSpaceAccess = SOPC_AddressSpaceAccess_Create(addressSpace, false);
    ck_assert_ptr_nonnull(addSpaceAccess);

    SOPC_ExpandedNodeId parentNodeId;
    SOPC_ExpandedNodeId_Initialize(&parentNodeId);
    SOPC_NodeId* tmpNodeId = SOPC_NodeId_FromCString("i=2268");
    ck_assert_ptr_nonnull(tmpNodeId);
    parentNodeId.NodeId = *tmpNodeId;

    const SOPC_NodeId refToParentTypeId = SOPC_NODEID_NS0_NUMERIC(OpcUaId_HasComponent);

    SOPC_NodeId* newNodeId = SOPC_NodeId_FromCString("i=12039");
    ck_assert_ptr_nonnull(newNodeId);

    SOPC_QualifiedName browseName;
    SOPC_QualifiedName_Initialize(&browseName);
    browseName.NamespaceIndex = 1;
    SOPC_ReturnStatus status = SOPC_String_AttachFromCstring(&browseName.Name, "ExampleNewNode");
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    OpcUa_MethodAttributes metAttributes;
    OpcUa_MethodAttributes_Initialize(&metAttributes);
    metAttributes.SpecifiedAttributes = OpcUa_NodeAttributesMask_DisplayName | OpcUa_NodeAttributesMask_Description |
                                        OpcUa_AttributeWriteMask_Executable;

    SOPC_StatusCode statusCode = SOPC_AddressSpaceAccess_AddMethodNode(
        addSpaceAccess, &parentNodeId, &refToParentTypeId, newNodeId, &browseName, &metAttributes);
    ck_assert_uint_eq(SOPC_GoodGenericStatus, statusCode);

    OpcUa_MethodAttributes_Clear(&metAttributes);
    SOPC_NodeId_Clear(newNodeId);
    SOPC_Free(newNodeId);
    SOPC_NodeId_Clear(tmpNodeId);
    SOPC_Free(tmpNodeId);
    SOPC_AddressSpace_Delete(addressSpace);
    SOPC_AddressSpaceAccess_Delete(&addSpaceAccess);
}
END_TEST

START_TEST(test_get_fresh_node_id) {
    SOPC_AddressSpace* addressSpace = SOPC_Embedded_AddressSpace_LoadWithAlloc(true);
    ck_assert_ptr_nonnull(addressSpace);

    uint16_t nbOfNS = 0;
    SOPC_Variant* nsArrayValue = NULL;
    bool foundNode = false;
    static const SOPC_NodeId SOPC_NSARRAY_NID = SOPC_NODEID_NS0_NUMERIC(OpcUaId_Server_NamespaceArray);

    // The function SOPC_AddressSpace_GetFreshNodeId in sopc_address_space_access.c (used by SOPC_AddressSpaceAccess_GetFreshNodeId) needs to be given a SOPC_AddressSpace with the attribute MaxNsNumId initialized.
    // The SOPC_AddressSpace_MaxNsNumId_Initialize function requires the number of namespaces as a parameter. 
    // To obtain this number, the length of the array obtained from the node "NamespaceArray" is used.
    SOPC_AddressSpace_Node* node = SOPC_AddressSpace_Get_Node(addressSpace, &SOPC_NSARRAY_NID, &foundNode);
    ck_assert(foundNode);
    ck_assert_ptr_nonnull(node);
    nsArrayValue = SOPC_AddressSpace_Get_Value(addressSpace, node);
    ck_assert_ptr_nonnull(nsArrayValue);
    ck_assert_uint_eq(SOPC_VariantArrayType_Array, nsArrayValue->ArrayType);
    nbOfNS = (uint16_t) nsArrayValue->Value.Array.Length;

    SOPC_ReturnStatus returnStatus = SOPC_AddressSpace_MaxNsNumId_Initialize(addressSpace, nbOfNS);
    ck_assert_int_eq(SOPC_STATUS_OK, returnStatus);
    
    SOPC_AddressSpaceAccess* addSpaceAccess = SOPC_AddressSpaceAccess_Create(addressSpace, false);
    ck_assert_ptr_nonnull(addSpaceAccess);

    uint16_t nsIndex = 0;
    SOPC_NodeId freshNodeId;

    SOPC_StatusCode statusCode = SOPC_AddressSpaceAccess_GetFreshNodeId(addSpaceAccess, nsIndex, &freshNodeId);
    ck_assert_uint_eq(SOPC_GoodGenericStatus, statusCode);
    uint32_t newId = freshNodeId.Data.Numeric;

    // The function returns the id number 24245. 
    // As of now, in the file s2opc_base_nodeset_origin.xml, 24244 is indeed the biggest nodeId. 
    // Which means that any number above would be a new and unique id.
    const uint32_t biggestNodeId = 24244;
    ck_assert_uint_gt(newId, biggestNodeId);

    SOPC_AddressSpace_Node_Clear(addressSpace, node);
    SOPC_AddressSpace_Delete(addressSpace);
    SOPC_AddressSpaceAccess_Delete(&addSpaceAccess);
}
END_TEST
#endif

Suite* tests_make_suite_address_space_access(void)
{
    Suite* s;
    TCase* tc_nominal_use;

    s = suite_create("AddressSpace_Access");

    tc_nominal_use = tcase_create("Nominal");
#if 0 != S2OPC_NODE_MANAGEMENT
    tcase_add_test(tc_nominal_use, test_read_attribute);
    tcase_add_test(tc_nominal_use, test_read_value);
    tcase_add_test(tc_nominal_use, test_write_value);
    tcase_add_test(tc_nominal_use, test_add_variable_node);
    tcase_add_test(tc_nominal_use, test_add_object_node);
    tcase_add_test(tc_nominal_use, test_translate_browse_path);
    tcase_add_test(tc_nominal_use, test_browse_node);
    tcase_add_test(tc_nominal_use, test_delete_child_nodes);
    tcase_add_test(tc_nominal_use, test_add_method_node);
    tcase_add_test(tc_nominal_use, test_get_fresh_node_id);
#endif
    suite_add_tcase(s, tc_nominal_use);

    return s;
}
