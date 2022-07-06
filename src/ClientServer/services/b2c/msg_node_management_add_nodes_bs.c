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

#include "msg_node_management_add_nodes_bs.h"

#include "sopc_assert.h"
#include "sopc_mem_alloc.h"
#include "sopc_types.h"
#include "util_b2c.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void msg_node_management_add_nodes_bs__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void msg_node_management_add_nodes_bs__alloc_msg_add_nodes_resp_results(
    const constants__t_msg_i msg_node_management_add_nodes_bs__p_resp_msg,
    const t_entier4 msg_node_management_add_nodes_bs__p_nb_results,
    t_bool* const msg_node_management_add_nodes_bs__bres)
{
    *msg_node_management_add_nodes_bs__bres = false;
    OpcUa_AddNodesResponse* addNodesResp = (OpcUa_AddNodesResponse*) msg_node_management_add_nodes_bs__p_resp_msg;
    if (msg_node_management_add_nodes_bs__p_nb_results > 0)
    {
        if (SIZE_MAX / (uint32_t) msg_node_management_add_nodes_bs__p_nb_results >
            sizeof(OpcUa_MonitoredItemCreateResult))
        {
            addNodesResp->NoOfResults = msg_node_management_add_nodes_bs__p_nb_results;
            addNodesResp->Results =
                SOPC_Calloc((size_t) msg_node_management_add_nodes_bs__p_nb_results, sizeof(*addNodesResp->Results));
            if (NULL != addNodesResp->Results)
            {
                for (int32_t i = 0; i < addNodesResp->NoOfResults; i++)
                {
                    OpcUa_AddNodesResult_Initialize(&addNodesResp->Results[i]);
                }
                *msg_node_management_add_nodes_bs__bres = true;
            }
        }
    }
    else
    {
        addNodesResp->NoOfResults = 0;
        *msg_node_management_add_nodes_bs__bres = true;
    }
}

void msg_node_management_add_nodes_bs__get_msg_create_add_nodes_req_nb_add_nodes(
    const constants__t_msg_i msg_node_management_add_nodes_bs__p_req_msg,
    t_entier4* const msg_node_management_add_nodes_bs__p_nb_add_nodes)
{
    OpcUa_AddNodesRequest* addNodesReq = (OpcUa_AddNodesRequest*) msg_node_management_add_nodes_bs__p_req_msg;
    *msg_node_management_add_nodes_bs__p_nb_add_nodes = addNodesReq->NoOfNodesToAdd;
}

static bool check_node_class_type_definition_constraint(constants__t_NodeClass_i p_nodeClass,
                                                        const SOPC_ExpandedNodeId* typeDefinition)
{
    bool typeDefPresent = false;
    bool typeDefExpected = false;
    switch (p_nodeClass)
    {
    case constants__e_ncl_Object:
    case constants__e_ncl_Variable:
        typeDefExpected = true;
        break;
    default:
        typeDefExpected = false;
    }
    // TODO: replace by call to constants_bs__getall_conv_ExpandedNodeId_NodeId (make it an util in b2c ?)
    // Note: we should accept an URI defined here but it means we have to translate URI to index in NodeId
    if (0 == typeDefinition->ServerIndex && typeDefinition->NamespaceUri.Length <= 0 &&
        SOPC_NodeId_IsNull(&typeDefinition->NodeId))
    {
        typeDefPresent = false;
    }
    else
    {
        typeDefPresent = true;
    }
    return (typeDefExpected == typeDefPresent);
}

void msg_node_management_add_nodes_bs__getall_add_node_item_req_params(
    const constants__t_msg_i msg_node_management_add_nodes_bs__p_req_msg,
    const t_entier4 msg_node_management_add_nodes_bs__p_index,
    constants_statuscodes_bs__t_StatusCode_i* const msg_node_management_add_nodes_bs__p_sc,
    constants__t_ExpandedNodeId_i* const msg_node_management_add_nodes_bs__p_parentNid,
    constants__t_NodeId_i* const msg_node_management_add_nodes_bs__p_refTypeId,
    constants__t_ExpandedNodeId_i* const msg_node_management_add_nodes_bs__p_reqNodeId,
    constants__t_QualifiedName_i* const msg_node_management_add_nodes_bs__p_browseName,
    constants__t_NodeClass_i* const msg_node_management_add_nodes_bs__p_nodeClass,
    constants__t_NodeAttributes_i* const msg_node_management_add_nodes_bs__p_nodeAttributes,
    constants__t_ExpandedNodeId_i* const msg_node_management_add_nodes_bs__p_typeDefId)
{
    bool result = true;
    // index is ensured to be valid by B model and NodesToAdd array to be valid due to decoding message success
    OpcUa_AddNodesRequest* addNodesReq = (OpcUa_AddNodesRequest*) msg_node_management_add_nodes_bs__p_req_msg;
    OpcUa_AddNodesItem* addNodesItem = &addNodesReq->NodesToAdd[msg_node_management_add_nodes_bs__p_index - 1];
    const SOPC_ExpandedNodeId* typeDefinition = &addNodesItem->TypeDefinition;

    // TODO: replace by call to constants_bs__getall_conv_ExpandedNodeId_NodeId (make it an util in b2c ?)
    // Check local constraint on requested NodeId: "the serverIndex in the expanded NodeId shall be 0."
    if (0 != addNodesItem->RequestedNewNodeId.ServerIndex)
    {
        *msg_node_management_add_nodes_bs__p_sc = constants_statuscodes_bs__e_sc_bad_node_id_rejected;
        result = false;
    }

    // Check BrowseName is not empty
    if (result && addNodesItem->BrowseName.Name.Length <= 0)
    {
        *msg_node_management_add_nodes_bs__p_sc = constants_statuscodes_bs__e_sc_bad_browse_name_invalid;
        result = false;
    }

    // Check NodeClass validity
    if (result)
    {
        result = util_NodeClass__C_to_B(addNodesItem->NodeClass, msg_node_management_add_nodes_bs__p_nodeClass);
        if (!result)
        {
            *msg_node_management_add_nodes_bs__p_sc = constants_statuscodes_bs__e_sc_bad_node_class_invalid;
        }
    }

    // Check NodeAttributes is well decoded as an OPC UA object
    if (result && SOPC_ExtObjBodyEncoding_Object != addNodesItem->NodeAttributes.Encoding)
    {
        // We do not succeeded to decode NodesAttributes as an object => consider it invalid
        *msg_node_management_add_nodes_bs__p_sc = constants_statuscodes_bs__e_sc_bad_node_attributes_invalid;
        result = false;
    }

    // Check TypeDefinition validity depending on NodeClass
    if (result && !check_node_class_type_definition_constraint(*msg_node_management_add_nodes_bs__p_nodeClass,
                                                               &addNodesItem->TypeDefinition))
    {
        *msg_node_management_add_nodes_bs__p_sc = constants_statuscodes_bs__e_sc_bad_type_definition_invalid;
        result = false;
    }

    if (result)
    {
        // Local verification on parameters succeeded: set all output parameters with ok status
        *msg_node_management_add_nodes_bs__p_sc = constants_statuscodes_bs__e_sc_ok;
        *msg_node_management_add_nodes_bs__p_parentNid = &addNodesItem->ParentNodeId;
        *msg_node_management_add_nodes_bs__p_refTypeId = &addNodesItem->ReferenceTypeId;
        if (addNodesItem->RequestedNewNodeId.NamespaceUri.Length <= 0 &&
            SOPC_NodeId_IsNull(&addNodesItem->RequestedNewNodeId.NodeId))
        {
            *msg_node_management_add_nodes_bs__p_reqNodeId = constants_bs__c_ExpandedNodeId_indet;
        }
        else
        {
            *msg_node_management_add_nodes_bs__p_reqNodeId = &addNodesItem->RequestedNewNodeId;
        }
        *msg_node_management_add_nodes_bs__p_browseName = &addNodesItem->BrowseName;
        *msg_node_management_add_nodes_bs__p_nodeAttributes = &addNodesItem->NodeAttributes;
        if (typeDefinition)
        {
            *msg_node_management_add_nodes_bs__p_typeDefId = &addNodesItem->TypeDefinition;
        }
        else
        {
            *msg_node_management_add_nodes_bs__p_typeDefId = constants_bs__c_ExpandedNodeId_indet;
        }
    }
}

void msg_node_management_add_nodes_bs__setall_msg_add_nodes_item_resp_params(
    const constants__t_msg_i msg_node_management_add_nodes_bs__p_resp_msg,
    const t_entier4 msg_node_management_add_nodes_bs__p_index,
    const constants_statuscodes_bs__t_StatusCode_i msg_node_management_add_nodes_bs__p_sc,
    const constants__t_NodeId_i msg_node_management_add_nodes_bs__p_nid)
{
    OpcUa_AddNodesResponse* addNodesResp = (OpcUa_AddNodesResponse*) msg_node_management_add_nodes_bs__p_resp_msg;
    OpcUa_AddNodesResult* addNodesResult = &addNodesResp->Results[msg_node_management_add_nodes_bs__p_index - 1];
    if (constants__c_NodeId_indet != msg_node_management_add_nodes_bs__p_nid)
    {
        // Shallow copy
        addNodesResult->AddedNodeId = *msg_node_management_add_nodes_bs__p_nid;
        SOPC_NodeId_Initialize(msg_node_management_add_nodes_bs__p_nid);
    } // else keep initialized NodeId (<=> IsNull)
    constants_statuscodes_bs__getall_conv_StatusCode_To_RawStatusCode(msg_node_management_add_nodes_bs__p_sc,
                                                                      &addNodesResult->StatusCode);
}
