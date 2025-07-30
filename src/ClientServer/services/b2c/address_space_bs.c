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

/** \file
 *
 * Implements the structures behind the address space.
 */

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "address_space_bs.h"

#include "address_space_impl.h"
#include "app_cb_call_context_internal.h"
#include "b2c.h"
#include "opcua_identifiers.h"
#include "sopc_address_space_access_internal.h"
#include "sopc_address_space_utils_internal.h"
#include "sopc_assert.h"
#include "sopc_builtintypes.h"
#include "sopc_date_time.h"
#include "sopc_dict.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_numeric_range.h"
#include "sopc_service_call_context.h"
#include "sopc_toolkit_config_internal.h"
#include "sopc_user_manager.h"
#include "util_b2c.h"
#include "util_variant.h"

bool sopc_addressSpace_configured = false;
SOPC_AddressSpace* address_space_bs__nodes = NULL;

#define GENERATED_NODE_NAMESPACE_INDEX 1

#define InputArguments_BrowseName "InputArguments"

static bool is_inputArgument(const OpcUa_VariableNode* node);

void SOPC_AddressSpace_Check_Configured(void)
{
    if (sopc_addressSpace_configured)
    {
        SOPC_ASSERT(NULL != address_space_bs__nodes);
    }
}

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void address_space_bs__INITIALISATION(void)
{ /*Translated from B but an intialisation is not needed from this module.*/
}

/*--------------------
   OPERATIONS Clause
  --------------------*/

void address_space_bs__address_space_bs_UNINITIALISATION(void) {}

static void generate_notifs_after_address_space_access(SOPC_SLinkedList* operations)
{
    SOPC_ASSERT(NULL != operations);
    SOPC_AddressSpaceAccessOperation* operation =
        (SOPC_AddressSpaceAccessOperation*) SOPC_SLinkedList_PopHead(operations);
    // Note: operations were pushed (prepended) and we pop (head) them which leads to a FILO behavior.
    //       Since we push them as next events in the services event queue, it finally leads to a FIFO behavior.
    while (NULL != operation)
    {
        switch (operation->operation)
        {
        case SOPC_ADDSPACE_WRITE:
            SOPC_EventHandler_PostAsNext(SOPC_Services_GetEventHandler(), SE_TO_SE_SERVER_DATA_CHANGED, 0,
                                         operation->param1, operation->param2);
            break;
        case SOPC_ADDSPACE_CHANGE_NODE:
            SOPC_EventHandler_PostAsNext(SOPC_Services_GetEventHandler(), SE_TO_SE_SERVER_NODE_CHANGED, 0,
                                         operation->param1, operation->param2);
            break;
        default:
            SOPC_ASSERT(false);
            break;
        }
        SOPC_Free(operation);
        operation = (SOPC_AddressSpaceAccessOperation*) SOPC_SLinkedList_PopHead(operations);
    }
    SOPC_SLinkedList_Delete(operations);
}

void address_space_bs__deleteNode_AddressSpace(
    const constants__t_NodeId_i address_space_bs__p_nodeId,
    const t_bool address_space_bs__p_b_deleteTargetReferences,
    constants_statuscodes_bs__t_StatusCode_i* const address_space_bs__sc_deleteNode)
{
    SOPC_StatusCode retCode = OpcUa_BadInternalError;
    SOPC_AddressSpaceAccess* deleteNode_addSpaceAccess = SOPC_AddressSpaceAccess_Create(address_space_bs__nodes, true);
    if (NULL == deleteNode_addSpaceAccess)
    {
        retCode = OpcUa_BadOutOfMemory;
    }
    else
    {
        bool b_deleteChildNodes = false;
#if 0 != S2OPC_NODE_DELETE_CHILD_NODES
        b_deleteChildNodes = true;
#endif
        retCode = SOPC_AddressSpaceAccess_DeleteNode(deleteNode_addSpaceAccess, address_space_bs__p_nodeId,
                                                     address_space_bs__p_b_deleteTargetReferences, b_deleteChildNodes);
        if (SOPC_IsGoodOrUncertainStatus(retCode))
        {
            // Trigger notification on MI on the deleted nodes
            generate_notifs_after_address_space_access(
                SOPC_AddressSpaceAccess_GetOperations(deleteNode_addSpaceAccess));
        }
        SOPC_AddressSpaceAccess_Delete(&deleteNode_addSpaceAccess);
    }
    util_status_code__C_to_B(retCode, address_space_bs__sc_deleteNode);
}

void address_space_bs__exec_callMethod(const constants__t_endpoint_config_idx_i address_space_bs__p_endpoint_config_idx,
                                       const constants__t_CallMethodPointer_i address_space_bs__p_call_method_pointer,
                                       constants__t_RawStatusCode* const address_space_bs__p_rawStatusCode,
                                       t_entier4* const address_space_bs__p_nb_out,
                                       constants__t_ArgumentsPointer_i* const address_space_bs__p_out_arguments)
{
    *address_space_bs__p_nb_out = 0;
    *address_space_bs__p_out_arguments = NULL;
    const OpcUa_CallMethodRequest* methodToCall = address_space_bs__p_call_method_pointer;
    SOPC_ASSERT(NULL != methodToCall);

    /* Get the Method Call Manager from server configuration */
    SOPC_Endpoint_Config* endpoint_config =
        SOPC_ToolkitServer_GetEndpointConfig(address_space_bs__p_endpoint_config_idx);
    if (NULL == endpoint_config || NULL == endpoint_config->serverConfigPtr)
    {
        *address_space_bs__p_rawStatusCode = OpcUa_BadInternalError;
        return;
    }
    SOPC_MethodCallManager* mcm = endpoint_config->serverConfigPtr->mcm;
    if (NULL == mcm)
    {
        *address_space_bs__p_rawStatusCode = OpcUa_BadNotImplemented;
        return;
    }

    /* Get the C function corresponding to the method */
    const SOPC_NodeId* methodId = &methodToCall->MethodId;
    SOPC_MethodCallFunc* method_c = SOPC_MethodCallManager_GetMethod(mcm, methodId);
    if (NULL == method_c)
    {
        *address_space_bs__p_rawStatusCode = OpcUa_BadNotImplemented;
        return;
    }

    const SOPC_NodeId* objectId = &methodToCall->ObjectId;
    uint32_t nbInputArgs = (0 < methodToCall->NoOfInputArguments) ? (uint32_t) methodToCall->NoOfInputArguments
                                                                  : 0; /* convert to avoid compilator error */
    SOPC_Variant* inputArgs = methodToCall->InputArguments;
    uint32_t noOfOutput = 0;
    SOPC_Variant* outputArgs = NULL;
    SOPC_CallContext* cc = SOPC_CallContext_Copy(SOPC_CallContext_GetCurrent());
    cc->addressSpaceForMethodCall = SOPC_AddressSpaceAccess_Create(address_space_bs__nodes, true);
    if (NULL == cc->addressSpaceForMethodCall)
    {
        SOPC_CallContext_Free(cc);
        *address_space_bs__p_rawStatusCode = OpcUa_BadOutOfMemory;
        return;
    }
    *address_space_bs__p_rawStatusCode =
        method_c->pMethodFunc(cc, objectId, nbInputArgs, inputArgs, &noOfOutput, &outputArgs, method_c->pParam);
    generate_notifs_after_address_space_access(SOPC_AddressSpaceAccess_GetOperations(cc->addressSpaceForMethodCall));
    SOPC_AddressSpaceAccess_Delete(&cc->addressSpaceForMethodCall);
    SOPC_CallContext_Free(cc);
    if (0 != noOfOutput && NULL == outputArgs)
    {
        char* mNodeId = SOPC_NodeId_ToCString(methodId);
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "MethodCall %s unexpected failure: application variant array result is NULL which is "
                               "not expected when noOfOutputs (%" PRIu32 ") > 0",
                               mNodeId, noOfOutput);
        SOPC_Free(mNodeId);
        *address_space_bs__p_rawStatusCode = OpcUa_BadNotImplemented;
        return;
    }
    if (noOfOutput > INT32_MAX)
    {
        noOfOutput = INT32_MAX;
        // Note: normally used for input arguments but it is the better match and should not occur
        *address_space_bs__p_rawStatusCode = OpcUa_BadTooManyArguments;
    }
    if (SOPC_IsGoodOrUncertainStatus(*address_space_bs__p_rawStatusCode))
    {
        *address_space_bs__p_nb_out = (int32_t) noOfOutput;
        *address_space_bs__p_out_arguments = outputArgs;
    }
    else
    {
        int32_t nbElts = (int32_t) noOfOutput;
        SOPC_Clear_Array(&nbElts, (void**) &outputArgs, sizeof(SOPC_Variant), SOPC_Variant_ClearAux);
    }
}

void address_space_bs__addNode_AddressSpace_Variable(
    const t_bool address_space_bs__p_local,
    const constants__t_ExpandedNodeId_i address_space_bs__p_parentNid,
    const constants__t_NodeId_i address_space_bs__p_refTypeId,
    const constants__t_NodeId_i address_space_bs__p_newNodeId,
    const constants__t_QualifiedName_i address_space_bs__p_browseName,
    const constants__t_NodeClass_i address_space_bs__p_nodeClass,
    const constants__t_NodeAttributes_i address_space_bs__p_nodeAttributes,
    const constants__t_ExpandedNodeId_i address_space_bs__p_typeDefId,
    constants_statuscodes_bs__t_StatusCode_i* const address_space_bs__sc_addnode)
{
    SOPC_UNUSED_ARG(address_space_bs__p_nodeClass); // For B precondition
    SOPC_ASSERT(NULL != address_space_bs__p_nodeAttributes);
    SOPC_ASSERT(SOPC_ExtObjBodyEncoding_Object == address_space_bs__p_nodeAttributes->Encoding);
    SOPC_ASSERT(&OpcUa_NodeAttributes_EncodeableType == address_space_bs__p_nodeAttributes->Body.Object.ObjType ||
                &OpcUa_VariableAttributes_EncodeableType == address_space_bs__p_nodeAttributes->Body.Object.ObjType);
    SOPC_AddressSpaceAccess* addSpaceAccess = SOPC_AddressSpaceAccess_Create(address_space_bs__nodes, true);
    bool recursive = S2OPC_NODE_INTERNAL_ADD_CHILD_NODES || !address_space_bs__p_local;
    SOPC_StatusCode retCode = SOPC_AddressSpaceAccess_AddVariableNode(
        addSpaceAccess, address_space_bs__p_parentNid, address_space_bs__p_refTypeId, address_space_bs__p_newNodeId,
        address_space_bs__p_browseName,
        (const OpcUa_VariableAttributes*) address_space_bs__p_nodeAttributes->Body.Object.Value,
        address_space_bs__p_typeDefId, recursive);
    util_status_code__C_to_B(retCode, address_space_bs__sc_addnode);

    if (SOPC_IsGoodOrUncertainStatus(retCode))
    {
        // Trigger notification on MI on the added nodes
        generate_notifs_after_address_space_access(SOPC_AddressSpaceAccess_GetOperations(addSpaceAccess));
    }
    SOPC_AddressSpaceAccess_Delete(&addSpaceAccess);
}

void address_space_bs__addNode_AddressSpace_Object(
    const t_bool address_space_bs__p_local,
    const constants__t_ExpandedNodeId_i address_space_bs__p_parentNid,
    const constants__t_NodeId_i address_space_bs__p_refTypeId,
    const constants__t_NodeId_i address_space_bs__p_newNodeId,
    const constants__t_QualifiedName_i address_space_bs__p_browseName,
    const constants__t_NodeClass_i address_space_bs__p_nodeClass,
    const constants__t_NodeAttributes_i address_space_bs__p_nodeAttributes,
    const constants__t_ExpandedNodeId_i address_space_bs__p_typeDefId,
    constants_statuscodes_bs__t_StatusCode_i* const address_space_bs__sc_addnode)
{
    SOPC_UNUSED_ARG(address_space_bs__p_nodeClass); // For B precondition
    SOPC_ASSERT(NULL != address_space_bs__p_nodeAttributes);
    SOPC_ASSERT(SOPC_ExtObjBodyEncoding_Object == address_space_bs__p_nodeAttributes->Encoding);
    SOPC_ASSERT(&OpcUa_NodeAttributes_EncodeableType == address_space_bs__p_nodeAttributes->Body.Object.ObjType ||
                &OpcUa_ObjectAttributes_EncodeableType == address_space_bs__p_nodeAttributes->Body.Object.ObjType);

    SOPC_AddressSpaceAccess* addSpaceAccess = SOPC_AddressSpaceAccess_Create(address_space_bs__nodes, true);
    bool recursive = S2OPC_NODE_INTERNAL_ADD_CHILD_NODES || !address_space_bs__p_local;
    SOPC_StatusCode retCode = SOPC_AddressSpaceAccess_AddObjectNode(
        addSpaceAccess, address_space_bs__p_parentNid, address_space_bs__p_refTypeId, address_space_bs__p_newNodeId,
        address_space_bs__p_browseName,
        (const OpcUa_ObjectAttributes*) address_space_bs__p_nodeAttributes->Body.Object.Value,
        address_space_bs__p_typeDefId, recursive);
    util_status_code__C_to_B(retCode, address_space_bs__sc_addnode);

    if (SOPC_IsGoodOrUncertainStatus(retCode))
    {
        // Trigger notification on MI on the added nodes
        generate_notifs_after_address_space_access(SOPC_AddressSpaceAccess_GetOperations(addSpaceAccess));
    }
    SOPC_AddressSpaceAccess_Delete(&addSpaceAccess);
}

void address_space_bs__addNode_AddressSpace_Method(
    const constants__t_ExpandedNodeId_i address_space_bs__p_parentNid,
    const constants__t_NodeId_i address_space_bs__p_refTypeId,
    const constants__t_NodeId_i address_space_bs__p_newNodeId,
    const constants__t_QualifiedName_i address_space_bs__p_browseName,
    const constants__t_NodeClass_i address_space_bs__p_nodeClass,
    const constants__t_NodeAttributes_i address_space_bs__p_nodeAttributes,
    constants_statuscodes_bs__t_StatusCode_i* const address_space_bs__sc_addnode)
{
    SOPC_UNUSED_ARG(address_space_bs__p_nodeClass); // For B precondition
    SOPC_ASSERT(NULL != address_space_bs__p_nodeAttributes);
    SOPC_ASSERT(SOPC_ExtObjBodyEncoding_Object == address_space_bs__p_nodeAttributes->Encoding);
    SOPC_ASSERT(&OpcUa_NodeAttributes_EncodeableType == address_space_bs__p_nodeAttributes->Body.Object.ObjType ||
                &OpcUa_MethodAttributes_EncodeableType == address_space_bs__p_nodeAttributes->Body.Object.ObjType);
    SOPC_AddressSpaceAccess* addSpaceAccess = SOPC_AddressSpaceAccess_Create(address_space_bs__nodes, true);
    SOPC_StatusCode retCode = SOPC_AddressSpaceAccess_AddMethodNode(
        addSpaceAccess, address_space_bs__p_parentNid, address_space_bs__p_refTypeId, address_space_bs__p_newNodeId,
        address_space_bs__p_browseName,
        (const OpcUa_MethodAttributes*) address_space_bs__p_nodeAttributes->Body.Object.Value);
    util_status_code__C_to_B(retCode, address_space_bs__sc_addnode);

    if (SOPC_IsGoodStatus(retCode))
    {
        // Trigger notification on MI on the added nodes
        generate_notifs_after_address_space_access(SOPC_AddressSpaceAccess_GetOperations(addSpaceAccess));
    }
    SOPC_AddressSpaceAccess_Delete(&addSpaceAccess);
}

void address_space_bs__addNode_check_valid_node_attributes_type(
    const constants__t_NodeClass_i address_space_bs__p_nodeClass,
    const constants__t_NodeAttributes_i address_space_bs__p_nodeAttributes,
    t_bool* const address_space_bs__bres)
{
    // Check NodeAttributes is well decoded as an OPC UA object: verified in msg_node_management_add_nodes_bs
    SOPC_ASSERT(SOPC_ExtObjBodyEncoding_Object == address_space_bs__p_nodeAttributes->Encoding);

    // Check NodeAttributes type depending on NodeClass
    SOPC_EncodeableType* expectedNodeAttrsType = NULL;
    switch (address_space_bs__p_nodeClass)
    {
    case constants__e_ncl_Object:
        expectedNodeAttrsType = &OpcUa_ObjectAttributes_EncodeableType;
        break;
    case constants__e_ncl_Variable:
        expectedNodeAttrsType = &OpcUa_VariableAttributes_EncodeableType;
        break;
    case constants__e_ncl_Method:
        expectedNodeAttrsType = &OpcUa_MethodAttributes_EncodeableType;
        break;
    case constants__e_ncl_ObjectType:
        expectedNodeAttrsType = &OpcUa_ObjectTypeAttributes_EncodeableType;
        break;
    case constants__e_ncl_VariableType:
        expectedNodeAttrsType = &OpcUa_VariableTypeAttributes_EncodeableType;
        break;
    case constants__e_ncl_ReferenceType:
        expectedNodeAttrsType = &OpcUa_ReferenceTypeAttributes_EncodeableType;
        break;
    case constants__e_ncl_DataType:
        expectedNodeAttrsType = &OpcUa_DataTypeAttributes_EncodeableType;
        break;
    case constants__e_ncl_View:
        expectedNodeAttrsType = &OpcUa_ViewAttributes_EncodeableType;
        break;
    default:
        SOPC_ASSERT(false &&
                    "NodeClass must have been already checked by "
                    "msg_node_management_add_nodes_bs__getall_add_node_item_req_params");
    }

    SOPC_EncodeableType* actualNodeAttrsType = address_space_bs__p_nodeAttributes->Body.Object.ObjType;
    // EncodeableType might be either the generic attributs type (common to all nodes) or specialized for node class
    if (&OpcUa_NodeAttributes_EncodeableType == actualNodeAttrsType || expectedNodeAttrsType == actualNodeAttrsType)
    {
        *address_space_bs__bres = true;
    }
    else
    {
        *address_space_bs__bres = false;
    }
}

/* This is a_NodeId~ */
void address_space_bs__readall_AddressSpace_Node(const constants__t_NodeId_i address_space_bs__nid,
                                                 t_bool* const address_space_bs__nid_valid,
                                                 constants__t_Node_i* const address_space_bs__node)
{
    SOPC_NodeId* pnid_req;
    bool val_found = false;
    void* val;

    *address_space_bs__nid_valid = false;

    pnid_req = address_space_bs__nid;

    if (NULL == pnid_req)
        return;

    val = SOPC_AddressSpace_Get_Node(address_space_bs__nodes, pnid_req, &val_found);

    if (val_found)
    {
        *address_space_bs__nid_valid = true;
        *address_space_bs__node = val;
    }
}

static SOPC_Byte SOPC_Internal_ComputeAccessLevel_Value(const SOPC_AddressSpace_Node* node)
{
    SOPC_ASSERT(node->node_class == OpcUa_NodeClass_Variable);
    SOPC_Byte accessLevel = node->data.variable.AccessLevel;
    SOPC_Byte supportedFlags =
        (SOPC_AccessLevelMask_CurrentRead | SOPC_AccessLevelMask_CurrentWrite | SOPC_AccessLevelMask_HistoryRead);
    bool onlyValueWritable = SOPC_AddressSpace_AreReadOnlyNodes(address_space_bs__nodes);
    if (!onlyValueWritable)
    {
        // Status and Ts are only writable when nodes are not in "read only except value" mode
        supportedFlags |= (SOPC_AccessLevelMask_StatusWrite | SOPC_AccessLevelMask_TimestampWrite);
    }
    // Note: keep only supported access level flags in final value
    return (accessLevel & supportedFlags);
}

void address_space_bs__read_AddressSpace_AccessLevelEx_value(
    const constants__t_Node_i address_space_bs__p_node,
    constants_statuscodes_bs__t_StatusCode_i* const address_space_bs__sc,
    constants__t_Variant_i* const address_space_bs__variant)
{
    SOPC_Byte accessLevel = SOPC_Internal_ComputeAccessLevel_Value(address_space_bs__p_node);
    // Note: always returns 0 for extension bytes since we always support atomic
    // read/write operations
    // + write with index range
    *address_space_bs__variant = util_variant__new_Variant_from_uint32((uint32_t) accessLevel);
    if (NULL == *address_space_bs__variant)
    {
        *address_space_bs__sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
    }
    else
    {
        *address_space_bs__sc = constants_statuscodes_bs__e_sc_ok;
    }
}

void address_space_bs__read_AddressSpace_AccessLevel_value(
    const constants__t_Node_i address_space_bs__p_node,
    constants_statuscodes_bs__t_StatusCode_i* const address_space_bs__sc,
    constants__t_Variant_i* const address_space_bs__variant)
{
    SOPC_Byte accessLevel = SOPC_Internal_ComputeAccessLevel_Value(address_space_bs__p_node);
    *address_space_bs__variant = util_variant__new_Variant_from_Byte(accessLevel);
    if (NULL == *address_space_bs__variant)
    {
        *address_space_bs__sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
    }
    else
    {
        *address_space_bs__sc = constants_statuscodes_bs__e_sc_ok;
    }
}

void address_space_bs__read_AddressSpace_ArrayDimensions_value(
    const constants__t_Node_i address_space_bs__p_node,
    const t_bool address_space_bs__p_deepCopy,
    constants_statuscodes_bs__t_StatusCode_i* const address_space_bs__sc,
    constants__t_Variant_i* const address_space_bs__variant)
{
    SOPC_UNUSED_ARG(address_space_bs__p_deepCopy); // for now we create a new array with values to 0
    SOPC_ASSERT(address_space_bs__p_node->node_class == OpcUa_NodeClass_Variable ||
                address_space_bs__p_node->node_class == OpcUa_NodeClass_VariableType);
    *address_space_bs__sc = constants_statuscodes_bs__e_sc_ok;
    int32_t* valueRank = SOPC_AddressSpace_Get_ValueRank(address_space_bs__nodes, address_space_bs__p_node);
    SOPC_Variant* variant = SOPC_Variant_Create();
    if (variant == NULL)
    {
        *address_space_bs__sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
    }
    else
    {
        // See table 1 of part 5, we return unknown size or NULL value
        if (*valueRank > 0)
        {
            uint32_t* arrayDimensionsArray = SOPC_Calloc((size_t) *valueRank, sizeof(uint32_t));
            if (NULL != arrayDimensionsArray)
            {
                variant->BuiltInTypeId = SOPC_UInt32_Id;
                variant->ArrayType = SOPC_VariantArrayType_Array;
                variant->Value.Array.Length = *valueRank;
                variant->Value.Array.Content.Uint32Arr = arrayDimensionsArray; // All items already set to 0 by alloc
            }
            else
            {
                *address_space_bs__sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
                SOPC_Variant_Delete(variant);
                variant = NULL;
            }
        } // else NULL variant content (nothing to do in variant content)
    }
    *address_space_bs__variant = variant;
}

void address_space_bs__read_AddressSpace_BrowseName_value(
    const constants__t_Node_i address_space_bs__p_node,
    const t_bool address_space_bs__p_deepCopy,
    constants_statuscodes_bs__t_StatusCode_i* const address_space_bs__sc,
    constants__t_Variant_i* const address_space_bs__variant)
{
    *address_space_bs__sc = constants_statuscodes_bs__e_sc_ok;
    *address_space_bs__variant = util_variant__new_Variant_from_QualifiedName(
        SOPC_AddressSpace_Get_BrowseName(address_space_bs__nodes, address_space_bs__p_node),
        address_space_bs__p_deepCopy);
    if (NULL == *address_space_bs__variant)
    {
        *address_space_bs__sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
    }
}

void address_space_bs__read_AddressSpace_ContainsNoLoops_value(
    const constants__t_Node_i address_space_bs__p_node,
    constants_statuscodes_bs__t_StatusCode_i* const address_space_bs__sc,
    constants__t_Variant_i* const address_space_bs__variant)
{
    SOPC_ASSERT(address_space_bs__p_node->node_class == OpcUa_NodeClass_View);
    *address_space_bs__sc = constants_statuscodes_bs__e_sc_ok;
    // Note: always returns false since we do not check this property
    *address_space_bs__variant = util_variant__new_Variant_from_Bool(false);
    if (NULL == *address_space_bs__variant)
    {
        *address_space_bs__sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
    }
}

void address_space_bs__read_AddressSpace_DataTypeDefinition_value(
    const constants__t_Node_i address_space_bs__p_node,
    const t_bool address_space_bs__p_deepCopy,
    constants_statuscodes_bs__t_StatusCode_i* const address_space_bs__sc,
    constants__t_Variant_i* const address_space_bs__variant)
{
    SOPC_ASSERT(address_space_bs__p_node->node_class == OpcUa_NodeClass_DataType);
    *address_space_bs__sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
    SOPC_ExtensionObject* extObj =
        SOPC_AddressSpace_Get_DataTypeDefinition(address_space_bs__nodes, address_space_bs__p_node);
    SOPC_Variant* variant = NULL;

    if (SOPC_ExtObjBodyEncoding_Object == extObj->Encoding &&
        (&OpcUa_StructureDefinition_EncodeableType == extObj->Body.Object.ObjType ||
         &OpcUa_EnumDefinition_EncodeableType == extObj->Body.Object.ObjType))
    {
        variant = util_variant__new_Variant_from_ExtensionObject(extObj, address_space_bs__p_deepCopy);
        if (NULL != variant)
        {
            *address_space_bs__sc = constants_statuscodes_bs__e_sc_ok;
        }
    }
    else
    {
        *address_space_bs__sc = constants_statuscodes_bs__e_sc_bad_attribute_id_invalid;
    }
    *address_space_bs__variant = variant;
}

void address_space_bs__read_AddressSpace_DataType_value(
    const constants__t_Node_i address_space_bs__p_node,
    const t_bool address_space_bs__p_deepCopy,
    constants_statuscodes_bs__t_StatusCode_i* const address_space_bs__sc,
    constants__t_Variant_i* const address_space_bs__variant)
{
    SOPC_ASSERT(address_space_bs__p_node->node_class == OpcUa_NodeClass_Variable ||
                address_space_bs__p_node->node_class == OpcUa_NodeClass_VariableType);
    *address_space_bs__sc = constants_statuscodes_bs__e_sc_ok;
    *address_space_bs__variant = util_variant__new_Variant_from_NodeId(
        SOPC_AddressSpace_Get_DataType(address_space_bs__nodes, address_space_bs__p_node),
        address_space_bs__p_deepCopy);
    if (NULL == *address_space_bs__variant)
    {
        *address_space_bs__sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
    }
}

void address_space_bs__read_AddressSpace_DisplayName_value(
    const constants__t_LocaleIds_i address_space_bs__p_locales,
    const constants__t_Node_i address_space_bs__p_node,
    const t_bool address_space_bs__p_deepCopy,
    constants_statuscodes_bs__t_StatusCode_i* const address_space_bs__sc,
    constants__t_Variant_i* const address_space_bs__variant)
{
    *address_space_bs__sc = constants_statuscodes_bs__e_sc_ok;
    *address_space_bs__variant = util_variant__new_Variant_from_LocalizedText(
        SOPC_AddressSpace_Get_DisplayName(address_space_bs__nodes, address_space_bs__p_node),
        address_space_bs__p_deepCopy);
    if (NULL == *address_space_bs__variant)
    {
        *address_space_bs__sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
    }
    else
    {
        *address_space_bs__variant = util_variant__set_PreferredLocalizedText_from_LocalizedText_Variant(
            address_space_bs__variant, address_space_bs__p_locales, NULL);
    }
}

#if S2OPC_EVENT_MANAGEMENT
static SOPC_Byte SOPC_Internal_ComputeEventNotifier_Value(const SOPC_AddressSpace_Node* node)
{
    SOPC_ASSERT(node->node_class == OpcUa_NodeClass_Object || node->node_class == OpcUa_NodeClass_View);
    SOPC_Byte eventNotifier =
        (node->node_class == OpcUa_NodeClass_Object ? node->data.object.EventNotifier : node->data.view.EventNotifier);
    SOPC_Byte supportedFlags = OpcUa_EventNotifierType_SubscribeToEvents;
    // Note: keep only supported event notifier flags in final value
    return (eventNotifier & supportedFlags);
}
#endif

void address_space_bs__read_AddressSpace_EventNotifier_value(
    const constants__t_Node_i address_space_bs__p_node,
    constants_statuscodes_bs__t_StatusCode_i* const address_space_bs__sc,
    constants__t_Variant_i* const address_space_bs__variant)
{
    SOPC_ASSERT(address_space_bs__p_node->node_class == OpcUa_NodeClass_View ||
                address_space_bs__p_node->node_class == OpcUa_NodeClass_Object);
    *address_space_bs__sc = constants_statuscodes_bs__e_sc_ok;
#if S2OPC_EVENT_MANAGEMENT
    SOPC_Byte eventNotifier = SOPC_Internal_ComputeEventNotifier_Value(address_space_bs__p_node);
    *address_space_bs__variant = util_variant__new_Variant_from_Byte(eventNotifier);
#else
    *address_space_bs__variant = util_variant__new_Variant_from_Byte(0);
#endif
    if (NULL == *address_space_bs__variant)
    {
        *address_space_bs__sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
    }
}

void address_space_bs__read_AddressSpace_Executable_value(
    const constants__t_Node_i address_space_bs__p_node,
    constants_statuscodes_bs__t_StatusCode_i* const address_space_bs__sc,
    constants__t_Variant_i* const address_space_bs__variant)
{
    SOPC_ASSERT(address_space_bs__p_node->node_class == OpcUa_NodeClass_Method);
    bool executable;
    *address_space_bs__sc = constants_statuscodes_bs__e_sc_ok;
    address_space_bs__get_Executable(address_space_bs__p_node, &executable);
    *address_space_bs__variant = util_variant__new_Variant_from_Bool(executable);
    if (NULL == *address_space_bs__variant)
    {
        *address_space_bs__sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
    }
}

void address_space_bs__read_AddressSpace_Historizing_value(
    const constants__t_Node_i address_space_bs__p_node,
    constants_statuscodes_bs__t_StatusCode_i* const address_space_bs__sc,
    constants__t_Variant_i* const address_space_bs__variant)
{
    SOPC_ASSERT(address_space_bs__p_node->node_class == OpcUa_NodeClass_Variable);
    *address_space_bs__sc = constants_statuscodes_bs__e_sc_ok;
    SOPC_Boolean historizing = SOPC_AddressSpace_Get_Historizing(address_space_bs__nodes, address_space_bs__p_node);
    *address_space_bs__variant = util_variant__new_Variant_from_Bool(historizing);
    if (NULL == *address_space_bs__variant)
    {
        *address_space_bs__sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
    }
}

void address_space_bs__read_AddressSpace_IsAbstract_value(
    const constants__t_Node_i address_space_bs__p_node,
    constants_statuscodes_bs__t_StatusCode_i* const address_space_bs__sc,
    constants__t_Variant_i* const address_space_bs__variant)
{
    SOPC_ASSERT(address_space_bs__p_node->node_class == OpcUa_NodeClass_VariableType ||
                address_space_bs__p_node->node_class == OpcUa_NodeClass_ObjectType ||
                address_space_bs__p_node->node_class == OpcUa_NodeClass_ReferenceType ||
                address_space_bs__p_node->node_class == OpcUa_NodeClass_DataType);
    *address_space_bs__sc = constants_statuscodes_bs__e_sc_ok;
    SOPC_Boolean* isAbstract = SOPC_AddressSpace_Get_IsAbstract(address_space_bs__nodes, address_space_bs__p_node);
    *address_space_bs__variant = util_variant__new_Variant_from_Bool(*isAbstract);
    if (NULL == *address_space_bs__variant)
    {
        *address_space_bs__sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
    }
}

void address_space_bs__read_AddressSpace_NodeClass_value(
    const constants__t_Node_i address_space_bs__p_node,
    constants_statuscodes_bs__t_StatusCode_i* const address_space_bs__sc,
    constants__t_Variant_i* const address_space_bs__variant)
{
    *address_space_bs__sc = constants_statuscodes_bs__e_sc_ok;
    *address_space_bs__variant = util_variant__new_Variant_from_NodeClass(address_space_bs__p_node->node_class);
    if (NULL == *address_space_bs__variant)
    {
        *address_space_bs__sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
    }
}

void address_space_bs__read_AddressSpace_NodeId_value(
    const constants__t_Node_i address_space_bs__p_node,
    const t_bool address_space_bs__p_deepCopy,
    constants_statuscodes_bs__t_StatusCode_i* const address_space_bs__sc,
    constants__t_Variant_i* const address_space_bs__variant)
{
    *address_space_bs__variant = util_variant__new_Variant_from_NodeId(
        SOPC_AddressSpace_Get_NodeId(address_space_bs__nodes, address_space_bs__p_node), address_space_bs__p_deepCopy);
    if (NULL == *address_space_bs__variant)
    {
        *address_space_bs__sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
    }
}

/* Raw means no index range or preferred locales filtering */
void address_space_bs__read_AddressSpace_Raw_Node_Value_value(
    const constants__t_Node_i address_space_bs__p_node,
    const constants__t_NodeId_i address_space_bs__p_nid,
    const constants__t_AttributeId_i address_space_bs__p_aid,
    constants_statuscodes_bs__t_StatusCode_i* const address_space_bs__sc,
    constants__t_Variant_i* const address_space_bs__variant,
    constants__t_RawStatusCode* const address_space_bs__val_sc,
    constants__t_Timestamp* const address_space_bs__val_ts_src)
{
    SOPC_ASSERT(address_space_bs__p_node->node_class == OpcUa_NodeClass_Variable ||
                address_space_bs__p_node->node_class == OpcUa_NodeClass_VariableType);
    // Arguments used for B preconditions only
    SOPC_UNUSED_ARG(address_space_bs__p_nid);
    SOPC_UNUSED_ARG(address_space_bs__p_aid);

    *address_space_bs__val_sc = OpcUa_BadInvalidState;
    *address_space_bs__val_ts_src = constants_bs__c_Timestamp_null;
    *address_space_bs__sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;

    *address_space_bs__variant = util_variant__new_Variant_from_Variant(
        SOPC_AddressSpace_Get_Value(address_space_bs__nodes, address_space_bs__p_node), false);
    if (NULL == *address_space_bs__variant)
    {
        return;
    }
    *address_space_bs__sc = constants_statuscodes_bs__e_sc_ok;
    if (address_space_bs__p_node->node_class == OpcUa_NodeClass_Variable)
    {
        *address_space_bs__val_sc = SOPC_AddressSpace_Get_StatusCode(address_space_bs__nodes, address_space_bs__p_node);
        *address_space_bs__val_ts_src =
            SOPC_AddressSpace_Get_SourceTs(address_space_bs__nodes, address_space_bs__p_node);
    }
    else
    {
        *address_space_bs__val_sc = SOPC_GoodGenericStatus;
    }
}

void address_space_bs__read_AddressSpace_RolePermissions(
    const constants__t_Node_i address_space_bs__p_node,
    constants_statuscodes_bs__t_StatusCode_i* const address_space_bs__sc,
    constants__t_RolePermissionTypes_i* const address_space_bs__rolePermissions)
{
    *address_space_bs__sc = constants_statuscodes_bs__e_sc_bad_not_found;
    *address_space_bs__rolePermissions = constants_bs__c_RolePermissionTypes_indet;
    int32_t* node_noOfRolePermissions =
        SOPC_AddressSpace_Get_NoOfRolePermissions(address_space_bs__nodes, address_space_bs__p_node);
    if (*node_noOfRolePermissions > 0)
    {
        *address_space_bs__sc = constants_statuscodes_bs__e_sc_ok;
        OpcUa_RolePermissionType** rolePermissions =
            SOPC_AddressSpace_Get_RolePermissions(address_space_bs__nodes, address_space_bs__p_node);
        // Variant contains some RolePermissions, it is checked by construction.
        *address_space_bs__rolePermissions =
            util_variant__new_Variant_from_RolePermissions(*rolePermissions, *node_noOfRolePermissions);
        if (NULL == *address_space_bs__rolePermissions)
        {
            *address_space_bs__sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
        }
    }
}

void address_space_bs__read_AddressSpace_Symmetric_value(
    const constants__t_Node_i address_space_bs__p_node,
    constants_statuscodes_bs__t_StatusCode_i* const address_space_bs__sc,
    constants__t_Variant_i* const address_space_bs__variant)
{
    SOPC_ASSERT(address_space_bs__p_node->node_class == OpcUa_NodeClass_ReferenceType);
    *address_space_bs__sc = constants_statuscodes_bs__e_sc_ok;
    *address_space_bs__variant =
        util_variant__new_Variant_from_Bool(address_space_bs__p_node->data.reference_type.Symmetric);
    if (NULL == *address_space_bs__variant)
    {
        *address_space_bs__sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
    }
}

void address_space_bs__read_AddressSpace_UserAccessLevel_value(
    const constants__t_Node_i address_space_bs__p_node,
    const t_bool address_space_bs__p_is_user_read_auth,
    const t_bool address_space_bs__p_is_user_write_auth,
    constants_statuscodes_bs__t_StatusCode_i* const address_space_bs__sc,
    constants__t_Variant_i* const address_space_bs__variant)
{
    SOPC_ASSERT(address_space_bs__p_node->node_class == OpcUa_NodeClass_Variable);
    /* UserAccess Level can be only more restrictive than access level  */
    SOPC_Byte accessLevel = SOPC_Internal_ComputeAccessLevel_Value(address_space_bs__p_node);
    SOPC_Byte userAccessLevel = 0;
    if (address_space_bs__p_is_user_read_auth)
    {
        // Keep supported read flags
        userAccessLevel = accessLevel & SOPC_AccessLevelMask_CurrentRead;
    }
    if (address_space_bs__p_is_user_write_auth)
    {
        // Keep supported write flags
        uint8_t supportedWriteFlags =
            SOPC_AccessLevelMask_CurrentWrite | SOPC_AccessLevelMask_StatusWrite | SOPC_AccessLevelMask_TimestampWrite;
        userAccessLevel |= (accessLevel & supportedWriteFlags);
    }

    *address_space_bs__sc = constants_statuscodes_bs__e_sc_ok;
    *address_space_bs__variant = util_variant__new_Variant_from_Byte(userAccessLevel);
    if (NULL == *address_space_bs__variant)
    {
        *address_space_bs__sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
    }
}

void address_space_bs__read_AddressSpace_UserExecutable_value(
    const constants__t_Node_i address_space_bs__p_node,
    const t_bool address_space_bs__p_is_user_executable_auth,
    constants_statuscodes_bs__t_StatusCode_i* const address_space_bs__sc,
    constants__t_Variant_i* const address_space_bs__variant)
{
    SOPC_ASSERT(address_space_bs__p_node->node_class == OpcUa_NodeClass_Method);
    *address_space_bs__sc = constants_statuscodes_bs__e_sc_ok;
    /* UserExecutable Level can be only more restrictive than executable attribute */
    *address_space_bs__variant = util_variant__new_Variant_from_Bool(address_space_bs__p_node->data.method.Executable &&
                                                                     address_space_bs__p_is_user_executable_auth);
    if (NULL == *address_space_bs__variant)
    {
        *address_space_bs__sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
    }
}

void address_space_bs__read_AddressSpace_ValueRank_value(
    const constants__t_Node_i address_space_bs__p_node,
    constants_statuscodes_bs__t_StatusCode_i* const address_space_bs__sc,
    constants__t_Variant_i* const address_space_bs__variant)
{
    SOPC_ASSERT(address_space_bs__p_node->node_class == OpcUa_NodeClass_Variable ||
                address_space_bs__p_node->node_class == OpcUa_NodeClass_VariableType);
    *address_space_bs__sc = constants_statuscodes_bs__e_sc_ok;
    *address_space_bs__variant = util_variant__new_Variant_from_int32(
        *SOPC_AddressSpace_Get_ValueRank(address_space_bs__nodes, address_space_bs__p_node));
    if (NULL == *address_space_bs__variant)
    {
        *address_space_bs__sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
    }
}

void address_space_bs__read_AddressSpace_Value_value(
    const constants__t_LocaleIds_i address_space_bs__p_locales,
    const constants__t_Node_i address_space_bs__p_node,
    const constants__t_IndexRange_i address_space_bs__index_range,
    const t_bool address_space_bs__p_deepCopy,
    constants_statuscodes_bs__t_StatusCode_i* const address_space_bs__sc,
    constants__t_Variant_i* const address_space_bs__variant,
    constants__t_RawStatusCode* const address_space_bs__val_sc,
    constants__t_Timestamp* const address_space_bs__val_ts_src)
{
    SOPC_ASSERT(address_space_bs__p_node->node_class == OpcUa_NodeClass_Variable ||
                address_space_bs__p_node->node_class == OpcUa_NodeClass_VariableType);
    *address_space_bs__val_sc = OpcUa_BadInvalidState;
    *address_space_bs__val_ts_src = constants_bs__c_Timestamp_null;

    SOPC_Variant* value = util_variant__new_Variant_from_Variant(
        SOPC_AddressSpace_Get_Value(address_space_bs__nodes, address_space_bs__p_node), address_space_bs__p_deepCopy);

    if (value == NULL)
    {
        *address_space_bs__sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
        *address_space_bs__variant = NULL;
        return;
    }
    else
    {
        if (SOPC_LocalizedText_Id == value->BuiltInTypeId)
        {
            // Get preferred localized text(s) (single value, array or matrix)
            value = util_variant__set_PreferredLocalizedText_from_LocalizedText_Variant(
                &value, address_space_bs__p_locales, NULL);
        }
    }

    if (address_space_bs__index_range == NULL || address_space_bs__index_range->Length <= 0)
    {
        *address_space_bs__sc = constants_statuscodes_bs__e_sc_ok;
        *address_space_bs__variant = value;
    }
    else
    {
        *address_space_bs__variant = SOPC_Variant_Create();

        if (NULL == *address_space_bs__variant)
        {
            *address_space_bs__sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
        }
        else
        {
            *address_space_bs__sc =
                util_read_value_string_indexed(*address_space_bs__variant, value, address_space_bs__index_range);

            if (constants_statuscodes_bs__e_sc_ok != *address_space_bs__sc)
            {
                SOPC_Variant_Delete(*address_space_bs__variant);
                *address_space_bs__variant = NULL;
            }
        }

        SOPC_Variant_Delete(value);
    }

    if (constants_statuscodes_bs__e_sc_ok == *address_space_bs__sc)
    {
        if (address_space_bs__p_node->node_class == OpcUa_NodeClass_Variable)
        {
            *address_space_bs__val_sc =
                SOPC_AddressSpace_Get_StatusCode(address_space_bs__nodes, address_space_bs__p_node);
            *address_space_bs__val_ts_src =
                SOPC_AddressSpace_Get_SourceTs(address_space_bs__nodes, address_space_bs__p_node);
        }
        else
        {
            *address_space_bs__val_sc = SOPC_GoodGenericStatus;
        }
    }
}

static SOPC_ReturnStatus modify_localized_text(char** supportedLocales,
                                               SOPC_Variant* node_value,
                                               const SOPC_Variant* new_value,
                                               SOPC_Variant* previous_value)
{
    SOPC_ASSERT(SOPC_LocalizedText_Id == new_value->BuiltInTypeId);
    SOPC_ASSERT(SOPC_LocalizedText_Id == node_value->BuiltInTypeId);
    SOPC_ASSERT(node_value->ArrayType == new_value->ArrayType);
    previous_value->ArrayType = node_value->ArrayType;
    previous_value->BuiltInTypeId = node_value->BuiltInTypeId;
    /* Important note: we shall use Move because the initial variant value might be part of initial address space
     * (DoNotClear flag set to true if constant declaration). Since we will modify variant content it will lead to
     * an hybrid content (constant and not constant) if we do not make a copy before modification.
     * 1. Move current value to previous value variant
     * 2. Copy previous value content to node value
     * 3. Modify node value with new content
     */

    // 1. Move current value to previous value
    SOPC_Variant_Move(previous_value, node_value);
    SOPC_Variant_Clear(node_value); // Reset DoNotClear flag
    // 2. Make a copy of the variant to modify it and ensure clear (DoNotClear flag ensured to be false)
    SOPC_ReturnStatus status = SOPC_Variant_Copy(node_value, previous_value);
    if (SOPC_STATUS_OK != status)
    {
        // Restore node value
        SOPC_Variant_Move(node_value, previous_value);
        return status;
    }

    // 3. Modify node value with new content
    status = util_variant__update_applying_supported_locales(node_value, new_value, supportedLocales);

    if (SOPC_STATUS_OK != status)
    {
        SOPC_Variant_Clear(node_value);
        SOPC_Variant_Move(node_value, previous_value);
        SOPC_Variant_Clear(previous_value);
    }
    return status;
}

static bool is_localized_text_and_modifiable(SOPC_Variant* node_value, const SOPC_Variant* new_value)
{
    bool modifyLocalizedText = false;
    if (SOPC_LocalizedText_Id != node_value->BuiltInTypeId || SOPC_LocalizedText_Id != new_value->BuiltInTypeId)
    {
        // it is not a localized text or it is possible to have different types (new value can be NULL)
        return false;
    }
    if (node_value->ArrayType != new_value->ArrayType)
    {
        // we should overwrite and not modify since it is not same array type
        return false;
    }

    if (SOPC_VariantArrayType_SingleValue == node_value->ArrayType)
    {
        modifyLocalizedText = true;
    }
    else if (SOPC_VariantArrayType_Array == node_value->ArrayType)
    {
        modifyLocalizedText = node_value->Value.Array.Length == new_value->Value.Array.Length;
    }
    else if (SOPC_VariantArrayType_Matrix == node_value->ArrayType)
    {
        if (node_value->Value.Matrix.Dimensions == new_value->Value.Matrix.Dimensions)
        {
            modifyLocalizedText = true;
            for (int32_t i = 0; i < node_value->Value.Matrix.Dimensions; i++)
            {
                if (node_value->Value.Matrix.ArrayDimensions[i] != new_value->Value.Matrix.ArrayDimensions[i])
                {
                    modifyLocalizedText = false;
                }
            }
        }
    }
    else
    {
        return constants_statuscodes_bs__e_sc_bad_write_not_supported;
    }

    return modifyLocalizedText;
}

static constants_statuscodes_bs__t_StatusCode_i set_value_full(char** supportedLocales,
                                                               SOPC_Variant* node_value,
                                                               const SOPC_Variant* new_value,
                                                               SOPC_Variant* previous_value)
{
    bool modifyLocalizedText = is_localized_text_and_modifiable(node_value, new_value);

    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    if (modifyLocalizedText)
    {
        status = modify_localized_text(supportedLocales, node_value, new_value, previous_value);
    }
    else
    {
        // Overwrite current value
        SOPC_Variant_Move(previous_value, node_value);
        SOPC_Variant_Clear(node_value);
        SOPC_Variant_Initialize(node_value);
        status = SOPC_Variant_Copy(node_value, new_value);
    }

    switch (status)
    {
    case SOPC_STATUS_OK:
        return constants_statuscodes_bs__e_sc_ok;
    case SOPC_STATUS_NOT_SUPPORTED:
        return constants_statuscodes_bs__e_sc_bad_locale_not_supported;
    default:
        return constants_statuscodes_bs__e_sc_bad_internal_error;
    }
}

static constants_statuscodes_bs__t_StatusCode_i set_value_indexed_helper(SOPC_Variant* node_value,
                                                                         const SOPC_Variant* new_value,
                                                                         const SOPC_NumericRange* range,
                                                                         SOPC_Variant* previous_value)
{
    bool has_range = false;
    SOPC_ReturnStatus status = SOPC_Variant_HasRange(node_value, range, true, &has_range);

    if (status != SOPC_STATUS_OK)
    {
        return constants_statuscodes_bs__e_sc_bad_index_range_invalid;
    }

    if (!has_range)
    {
        return constants_statuscodes_bs__e_sc_bad_index_range_no_data;
    }

    status = SOPC_Variant_Copy(previous_value, node_value);

    if (status != SOPC_STATUS_OK)
    {
        return constants_statuscodes_bs__e_sc_bad_out_of_memory;
    }

    status = SOPC_Variant_SetRange(node_value, new_value, range);

    if (status != SOPC_STATUS_OK)
    {
        return util_return_status__C_to_status_code_B(status);
    }

    return constants_statuscodes_bs__e_sc_ok;
}

static constants_statuscodes_bs__t_StatusCode_i set_value_indexed(SOPC_Variant* node_value,
                                                                  const SOPC_Variant* new_value,
                                                                  const SOPC_String* range_str,
                                                                  SOPC_Variant* previous_value)
{
    SOPC_NumericRange* range = NULL;
    SOPC_ReturnStatus status = SOPC_NumericRange_Parse(SOPC_String_GetRawCString(range_str), &range);

    if (status != SOPC_STATUS_OK)
    {
        return (status == SOPC_STATUS_NOK) ? constants_statuscodes_bs__e_sc_bad_index_range_invalid
                                           : constants_statuscodes_bs__e_sc_bad_out_of_memory;
    }

    constants_statuscodes_bs__t_StatusCode_i ret =
        set_value_indexed_helper(node_value, new_value, range, previous_value);
    SOPC_NumericRange_Delete(range);

    return ret;
}

static SOPC_Variant* convertVariantType_ByteArrayByteString(SOPC_Variant* toConvert)
{
    SOPC_Variant* result = SOPC_Variant_Create();

    if (NULL == result)
    {
        return NULL;
    }

    if (SOPC_Byte_Id == toConvert->BuiltInTypeId && SOPC_VariantArrayType_Array == toConvert->ArrayType)
    {
        // Byte[] => ByteString
        result->ArrayType = SOPC_VariantArrayType_SingleValue;
        result->BuiltInTypeId = SOPC_ByteString_Id;
        result->DoNotClear = true; // We do not actually copy the content

        if (toConvert->Value.Array.Length > 0)
        {
            result->Value.Bstring.Length = toConvert->Value.Array.Length;
            result->Value.Bstring.Data = toConvert->Value.Array.Content.ByteArr;
        } // Otherwise NULL ByteString since array of length <= 0
    }
    else if (SOPC_ByteString_Id == toConvert->BuiltInTypeId &&
             SOPC_VariantArrayType_SingleValue == toConvert->ArrayType)
    {
        // ByteString => Byte[]
        result->ArrayType = SOPC_VariantArrayType_Array;
        result->BuiltInTypeId = SOPC_Byte_Id;
        result->DoNotClear = true; // We do not actually copy the content

        if (toConvert->Value.Bstring.Length > 0)
        {
            result->Value.Array.Length = toConvert->Value.Bstring.Length;
            result->Value.Array.Content.ByteArr = toConvert->Value.Bstring.Data;
        } // Otherwise empty Byte[] since ByteString of length <= 0
    }
    else
    {
        // It shall be a Byte[] or a ByteString
        SOPC_ASSERT(false);
    }

    return result;
}

void address_space_bs__set_Value(const constants__t_user_i address_space_bs__p_user,
                                 const constants__t_LocaleIds_i address_space_bs__p_locales,
                                 const constants__t_Node_i address_space_bs__node,
                                 const constants__t_Variant_i address_space_bs__variant,
                                 const t_bool address_space_bs__toConvert,
                                 const constants__t_IndexRange_i address_space_bs__index_range,
                                 constants_statuscodes_bs__t_StatusCode_i* const address_space_bs__serviceStatusCode,
                                 constants__t_DataValue_i* const address_space_bs__prev_dataValue)
{
    SOPC_UNUSED_ARG(
        address_space_bs__p_user); /* Keep for B precondition: user is already authorized for this operation */
    SOPC_AddressSpace_Node* node = address_space_bs__node;
    SOPC_Variant* pvar = SOPC_AddressSpace_Get_Value(address_space_bs__nodes, node);
    SOPC_Variant* convertedValue = NULL;
    const SOPC_Variant* newValue = address_space_bs__variant;

    *address_space_bs__prev_dataValue = SOPC_Malloc(sizeof(SOPC_DataValue));
    SOPC_DataValue_Initialize(*address_space_bs__prev_dataValue);

    if (address_space_bs__toConvert)
    {
        convertedValue = convertVariantType_ByteArrayByteString(address_space_bs__variant);
        newValue = convertedValue;
    }

    if (NULL == *address_space_bs__prev_dataValue || NULL == newValue)
    {
        *address_space_bs__serviceStatusCode = constants_statuscodes_bs__e_sc_bad_out_of_memory;
        return;
    }

    if (address_space_bs__index_range->Length <= 0)
    {
        *address_space_bs__serviceStatusCode =
            set_value_full(address_space_bs__p_locales, pvar, newValue, &(*address_space_bs__prev_dataValue)->Value);
    }
    else
    {
        // Note: set_value_indexed on single value will always fail except on ByteString/String
        // Note2: set_value_indexed does not support partial update of localized text (whole overwrite)
        *address_space_bs__serviceStatusCode = set_value_indexed(pvar, newValue, address_space_bs__index_range,
                                                                 &(*address_space_bs__prev_dataValue)->Value);
    }

    if (*address_space_bs__serviceStatusCode == constants_statuscodes_bs__e_sc_ok)
    {
        (*address_space_bs__prev_dataValue)->Status = SOPC_AddressSpace_Get_StatusCode(address_space_bs__nodes, node);
        SOPC_Value_Timestamp ts = SOPC_AddressSpace_Get_SourceTs(address_space_bs__nodes, node);
        (*address_space_bs__prev_dataValue)->SourceTimestamp = ts.timestamp;
        (*address_space_bs__prev_dataValue)->SourcePicoSeconds = ts.picoSeconds;
    }
    else
    {
        SOPC_DataValue_Clear(*address_space_bs__prev_dataValue);
        SOPC_Free(*address_space_bs__prev_dataValue);
        *address_space_bs__prev_dataValue = NULL;
    }

    if (address_space_bs__toConvert)
    {
        SOPC_Variant_Delete(convertedValue);
    }
}

void address_space_bs__set_Value_SourceTimestamp(const constants__t_user_i address_space_bs__p_user,
                                                 const constants__t_Node_i address_space_bs__p_node,
                                                 const constants__t_Timestamp address_space_bs__p_ts)
{
    SOPC_UNUSED_ARG(
        address_space_bs__p_user); /* Keep for B precondition: user is already authorized for this operation */
    SOPC_AddressSpace_Node* node = address_space_bs__p_node;
    SOPC_ASSERT(node->node_class == OpcUa_NodeClass_Variable);
    bool result = true;
    if (address_space_bs__p_ts.timestamp == 0 && address_space_bs__p_ts.picoSeconds == 0)
    {
        // Update source timestamp with current date if no date provided
        SOPC_Value_Timestamp ts;
        ts.timestamp = SOPC_Time_GetCurrentTimeUTC();
        ts.picoSeconds = 0;
        result = SOPC_AddressSpace_Set_SourceTs(address_space_bs__nodes, node, ts);
    }
    else
    {
        result = SOPC_AddressSpace_Set_SourceTs(address_space_bs__nodes, node, address_space_bs__p_ts);
    }

    if (!result)
    {
        static bool warned = false;
        if (!warned && (address_space_bs__p_ts.timestamp != 0 || address_space_bs__p_ts.picoSeconds != 0))
        {
            char* nodeId = SOPC_NodeId_ToCString(SOPC_AddressSpace_Get_NodeId(address_space_bs__nodes, node));
            SOPC_Logger_TraceWarning(
                SOPC_LOG_MODULE_CLIENTSERVER,
                "SourceTimestamp write on NodeId=%s failed due to constant metadata in address space. "
                "It should be forbidden by AccessLevel.",
                nodeId);
            SOPC_Free(nodeId);
            warned = true;
        }
    }
}

void address_space_bs__set_Value_StatusCode(const constants__t_user_i address_space_bs__p_user,
                                            const constants__t_Node_i address_space_bs__p_node,
                                            const constants__t_RawStatusCode address_space_bs__p_sc)
{
    SOPC_UNUSED_ARG(
        address_space_bs__p_user); /* Keep for B precondition: user is already authorized for this operation */
    SOPC_AddressSpace_Node* node = address_space_bs__p_node;
    SOPC_ASSERT(node->node_class == OpcUa_NodeClass_Variable);
    bool result = SOPC_AddressSpace_Set_StatusCode(address_space_bs__nodes, node, address_space_bs__p_sc);

    if (!result)
    {
        static bool warned = false;
        if (!warned && !SOPC_IsGoodStatus(address_space_bs__p_sc))
        {
            char* nodeId = SOPC_NodeId_ToCString(SOPC_AddressSpace_Get_NodeId(address_space_bs__nodes, node));
            SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                     "StatusCode write on NodeId=%s failed due to constant metadata in address space."
                                     "It should be forbidden by AccessLevel.",
                                     nodeId);
            SOPC_Free(nodeId);
            warned = true;
        }
    }
}

void address_space_bs__get_Value_StatusCode(const constants__t_user_i address_space_bs__p_user,
                                            const constants__t_Node_i address_space_bs__node,
                                            constants__t_RawStatusCode* const address_space_bs__sc)
{
    SOPC_UNUSED_ARG(address_space_bs__p_user); /* User is already authorized for this operation */
    *address_space_bs__sc = SOPC_AddressSpace_Get_StatusCode(address_space_bs__nodes, address_space_bs__node);
}

void address_space_bs__is_AddressSpace_constant(t_bool* const address_space_bs__bres)
{
    *address_space_bs__bres = (!SOPC_AddressSpace_AreNodesReleasable(address_space_bs__nodes) ||
                               SOPC_AddressSpace_AreReadOnlyNodes(address_space_bs__nodes));
}

void address_space_bs__is_IndexRangeDefined(const constants__t_IndexRange_i address_space_bs__p_index_range,
                                            t_bool* const address_space_bs__bres)
{
    *address_space_bs__bres = false;
    if (NULL != address_space_bs__p_index_range)
    {
        *address_space_bs__bres = (address_space_bs__p_index_range->Length > 0);
    }
}

void address_space_bs__is_NodeId_equal(const constants__t_NodeId_i address_space_bs__nid1,
                                       const constants__t_NodeId_i address_space_bs__nid2,
                                       t_bool* const address_space_bs__bres)
{
    *address_space_bs__bres = SOPC_NodeId_Equal(address_space_bs__nid1, address_space_bs__nid2);
}

void address_space_bs__read_AddressSpace_clear_value(const constants__t_Variant_i address_space_bs__val)
{
    SOPC_Variant_Clear(address_space_bs__val);
}

void address_space_bs__read_AddressSpace_free_variant(const constants__t_Variant_i address_space_bs__val)
{
    SOPC_Variant_Delete(address_space_bs__val);
}

void address_space_bs__write_AddressSpace_free_dataValue(const constants__t_DataValue_i address_space_bs__data)
{
    SOPC_DataValue_Clear(address_space_bs__data);
    SOPC_Free(address_space_bs__data);
}

void address_space_bs__gen_fresh_NodeId(t_bool* const address_space_bs__bres,
                                        constants__t_NodeId_i* const address_space_bs__newNid)
{
    SOPC_NodeId* newNid = SOPC_AddressSpace_GetFreshNodeId(address_space_bs__nodes, GENERATED_NODE_NAMESPACE_INDEX);
    if (NULL != newNid)
    {
        *address_space_bs__newNid = newNid;
        *address_space_bs__bres = true;
    }
    else
    {
        *address_space_bs__newNid = constants__c_NodeId_indet;
        *address_space_bs__bres = false;
    }
}

void address_space_bs__get_AccessLevel(const constants__t_Node_i address_space_bs__p_node,
                                       constants__t_access_level* const address_space_bs__p_access_level)
{
    SOPC_AddressSpace_Node* node = address_space_bs__p_node;
    *address_space_bs__p_access_level = SOPC_AddressSpace_Get_AccessLevel(address_space_bs__nodes, node);
}

void address_space_bs__get_BrowseName(const constants__t_Node_i address_space_bs__p_node,
                                      constants__t_QualifiedName_i* const address_space_bs__p_browse_name)
{
    SOPC_AddressSpace_Node* node = address_space_bs__p_node;
    *address_space_bs__p_browse_name = SOPC_AddressSpace_Get_BrowseName(address_space_bs__nodes, node);
}

void address_space_bs__get_DisplayName(const constants__t_Node_i address_space_bs__p_node,
                                       constants__t_LocalizedText_i* const address_space_bs__p_display_name)
{
    SOPC_AddressSpace_Node* node = address_space_bs__p_node;
    *address_space_bs__p_display_name = SOPC_AddressSpace_Get_DisplayName(address_space_bs__nodes, node);
}

void address_space_bs__get_EventNotifier(const constants__t_Node_i address_space_bs__p_node,
                                         constants__t_Byte* const address_space_bs__p_byte)
{
    SOPC_UNUSED_ARG(address_space_bs__p_node);
    *address_space_bs__p_byte = 0;

#if S2OPC_EVENT_MANAGEMENT
    *address_space_bs__p_byte = SOPC_Internal_ComputeEventNotifier_Value(address_space_bs__p_node);
#endif
}

void address_space_bs__get_Executable(const constants__t_Node_i address_space_bs__p_node,
                                      t_bool* const address_space_bs__p_bool)
{
    SOPC_AddressSpace_Node* node = address_space_bs__p_node;
    *address_space_bs__p_bool = SOPC_AddressSpace_Get_Executable(address_space_bs__nodes, node);
}

void address_space_bs__get_NodeClass(const constants__t_Node_i address_space_bs__p_node,
                                     constants__t_NodeClass_i* const address_space_bs__p_node_class)
{
    SOPC_ASSERT(NULL != address_space_bs__p_node);
    SOPC_AddressSpace_Node* node = address_space_bs__p_node;

    bool res = util_NodeClass__C_to_B(node->node_class, address_space_bs__p_node_class);
    if (false == res)
    {
        *address_space_bs__p_node_class = constants__c_NodeClass_indet;
    }
}

void address_space_bs__get_DataType(const constants__t_Node_i address_space_bs__p_node,
                                    constants__t_NodeId_i* const address_space_bs__p_data_type)
{
    SOPC_ASSERT(NULL != address_space_bs__p_node);
    SOPC_AddressSpace_Node* node = address_space_bs__p_node;
    *address_space_bs__p_data_type = SOPC_AddressSpace_Get_DataType(address_space_bs__nodes, node);
}

void address_space_bs__get_ValueRank(const constants__t_Node_i address_space_bs__p_node,
                                     t_entier4* const address_space_bs__p_value_rank)
{
    SOPC_ASSERT(NULL != address_space_bs__p_node);
    SOPC_AddressSpace_Node* node = address_space_bs__p_node;
    *address_space_bs__p_value_rank = *SOPC_AddressSpace_Get_ValueRank(address_space_bs__nodes, node);
}

static bool is_inputArgument(const OpcUa_VariableNode* node)
{
    if (NULL == node || &OpcUa_VariableNode_EncodeableType != node->encodeableType)
    {
        return false;
    }

    /* Type should be Argument */
    if (!(SOPC_IdentifierType_Numeric == node->DataType.IdentifierType &&
          OpcUaId_Argument == node->DataType.Data.Numeric))
    {
        return false;
    }

    return (strcmp(SOPC_String_GetRawCString(&node->BrowseName.Name), InputArguments_BrowseName) == 0);
}

void address_space_bs__get_TypeDefinition(const constants__t_Node_i address_space_bs__p_node,
                                          constants__t_ExpandedNodeId_i* const address_space_bs__p_type_def)
{
    *address_space_bs__p_type_def =
        SOPC_AddressSpaceUtil_GetTypeDefinition(address_space_bs__nodes, address_space_bs__p_node);
}

void address_space_bs__get_Reference_ReferenceType(const constants__t_Reference_i address_space_bs__p_ref,
                                                   constants__t_NodeId_i* const address_space_bs__p_RefType)
{
    OpcUa_ReferenceNode* ref = address_space_bs__p_ref;
    *address_space_bs__p_RefType = &ref->ReferenceTypeId;
}

void address_space_bs__get_Reference_TargetNode(const constants__t_Reference_i address_space_bs__p_ref,
                                                constants__t_ExpandedNodeId_i* const address_space_bs__p_TargetNode)
{
    OpcUa_ReferenceNode* ref = address_space_bs__p_ref;
    *address_space_bs__p_TargetNode = &ref->TargetId;
}

void address_space_bs__get_Reference_IsForward(const constants__t_Reference_i address_space_bs__p_ref,
                                               t_bool* const address_space_bs__p_IsForward)
{
    OpcUa_ReferenceNode* ref = address_space_bs__p_ref;
    *address_space_bs__p_IsForward = !ref->IsInverse;
}

void address_space_bs__get_Node_RefIndexEnd(const constants__t_Node_i address_space_bs__p_node,
                                            t_entier4* const address_space_bs__p_ref_index)
{
    SOPC_ASSERT(NULL != address_space_bs__p_node);
    SOPC_AddressSpace_Node* node = address_space_bs__p_node;
    int32_t* n_refs = SOPC_AddressSpace_Get_NoOfReferences(address_space_bs__nodes, node);
    *address_space_bs__p_ref_index = *n_refs;
}

void address_space_bs__get_RefIndex_Reference(const constants__t_Node_i address_space_bs__p_node,
                                              const t_entier4 address_space_bs__p_ref_index,
                                              constants__t_Reference_i* const address_space_bs__p_ref)
{
    SOPC_ASSERT(NULL != address_space_bs__p_node);
    SOPC_ASSERT(address_space_bs__p_ref_index > 0);
    SOPC_AddressSpace_Node* node = address_space_bs__p_node;
    OpcUa_ReferenceNode** refs = SOPC_AddressSpace_Get_References(address_space_bs__nodes, node);
    int32_t* n_refs = SOPC_AddressSpace_Get_NoOfReferences(address_space_bs__nodes, node);
    SOPC_ASSERT(address_space_bs__p_ref_index <= *n_refs);

    *address_space_bs__p_ref = &(*refs)[address_space_bs__p_ref_index - 1];
}

void address_space_bs__get_InputArguments(const constants__t_Node_i address_space_bs__p_node,
                                          constants__t_Variant_i* const address_space_bs__p_input_arg)
{
    SOPC_ASSERT(NULL != address_space_bs__p_node);
    SOPC_ASSERT(NULL != address_space_bs__p_input_arg);

    constants__t_Variant_i result = NULL;
    bool found;
    SOPC_AddressSpace_Node* targetNode;

    int32_t* n_refs = SOPC_AddressSpace_Get_NoOfReferences(address_space_bs__nodes, address_space_bs__p_node);
    OpcUa_ReferenceNode** refs = SOPC_AddressSpace_Get_References(address_space_bs__nodes, address_space_bs__p_node);
    for (int32_t i = 0; i < *n_refs && NULL == result; ++i)
    { /* stop when input argument is found */
        OpcUa_ReferenceNode* ref = &(*refs)[i];
        if (SOPC_AddressSpaceUtil_IsProperty(ref))
        {
            if (ref->TargetId.ServerIndex == 0 && ref->TargetId.NamespaceUri.Length <= 0)
            { // Shall be on same server and shall use only NodeId
                targetNode = SOPC_AddressSpace_Get_Node(address_space_bs__nodes, &ref->TargetId.NodeId, &found);
                if (found && NULL != targetNode && OpcUa_NodeClass_Variable == targetNode->node_class)
                {
                    if (is_inputArgument(&targetNode->data.variable))
                    {
                        result = SOPC_AddressSpace_Get_Value(address_space_bs__nodes, targetNode);
                    }
                }
            }
        }
    }
    *address_space_bs__p_input_arg = result;
}

static const SOPC_NodeId* getExtObjEncodingId(SOPC_ExtensionObject* extObj)
{
    SOPC_ASSERT(NULL != extObj);
    if (extObj->TypeId.NamespaceUri.Length <= 0)
    {
        return &extObj->TypeId.NodeId;
    }
    return NULL;
}

static const SOPC_NodeId* getVariantEncodingId(SOPC_Variant* varExtObj)
{
    SOPC_ASSERT(NULL != varExtObj && SOPC_ExtensionObject_Id == varExtObj->BuiltInTypeId);
    if (SOPC_VariantArrayType_SingleValue == varExtObj->ArrayType)
    {
        return getExtObjEncodingId(varExtObj->Value.ExtObject);
    }
    else if (SOPC_VariantArrayType_Array == varExtObj->ArrayType ||
             SOPC_VariantArrayType_Matrix == varExtObj->ArrayType)
    {
        // For array/matrix we need to check each element type

        const SOPC_NodeId* nextArrayEltTypeId = NULL;
        const SOPC_NodeId* arrayEltTypeId = NULL;

        int32_t extObjArrayLength = SOPC_Variant_GetArrayOrMatrixLength(varExtObj);
        SOPC_ExtensionObject* extObjArray = SOPC_VARIANT_GET_ARRAY_VALUES_PTR(varExtObj, ExtObject);
        SOPC_ASSERT(NULL != extObjArray || extObjArrayLength <= 0);

        for (int32_t i = 0; i < extObjArrayLength; i++)
        {
            nextArrayEltTypeId = getExtObjEncodingId(&extObjArray[i]);
            if (i > 0 && !SOPC_NodeId_Equal(arrayEltTypeId, nextArrayEltTypeId))
            {
                // Note: it would be necessary to find the first common ancestor of both types: not supported
                return NULL;
            }
            // All previous element types are identical
            arrayEltTypeId = nextArrayEltTypeId;
        }
        // => Null type id if array empty
        return arrayEltTypeId;
    }
    return NULL;
}

void address_space_bs__get_conv_Variant_Type(const constants__t_Variant_i address_space_bs__p_variant,
                                             constants__t_NodeId_i* const address_space_bs__p_type)
{
    SOPC_ASSERT(NULL != address_space_bs__p_variant);
    SOPC_ASSERT(NULL != address_space_bs__p_type);
    SOPC_NodeId* result = NULL;
    result = SOPC_Variant_Get_DataType(address_space_bs__p_variant);

    // In case resolution was not possible due to unknown encoder, try to retrieve type from address space
    if (NULL != result && /* NULL is possible due to ExtensionObject with None encoding mask case */
        SOPC_ExtensionObject_Id == address_space_bs__p_variant->BuiltInTypeId &&
        OPCUA_NAMESPACE_INDEX == result->Namespace && SOPC_IdentifierType_Numeric == result->IdentifierType &&
        OpcUaId_Structure == result->Data.Numeric)
    {
        const SOPC_NodeId* encodingNodeId = getVariantEncodingId(address_space_bs__p_variant);
        if (NULL != encodingNodeId)
        {
            const SOPC_NodeId* resolvedDataTypeId =
                SOPC_AddressSpaceUtil_GetEncodingDataType(address_space_bs__nodes, encodingNodeId);
            if (NULL != resolvedDataTypeId)
            {
                SOPC_NodeId_Clear(result);
                SOPC_ReturnStatus status = SOPC_NodeId_Copy(result, resolvedDataTypeId);
                if (SOPC_STATUS_OK != status)
                {
                    SOPC_Free(result);
                    result = NULL;
                }
            }
        }
    }
    *address_space_bs__p_type = result;
}

void address_space_bs__get_conv_Variant_ValueRank(const constants__t_Variant_i address_space_bs__p_variant,
                                                  t_entier4* const address_space_bs__p_valueRank)
{
    SOPC_ASSERT(NULL != address_space_bs__p_variant);
    SOPC_ASSERT(NULL != address_space_bs__p_valueRank);
    *address_space_bs__p_valueRank = SOPC_Variant_Get_ValueRank(address_space_bs__p_variant);
}
