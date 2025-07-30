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
 * Implements the base machine that reads a HistoryReadRequest.
 */

#include <stdint.h>
#include <stdio.h>

#include "address_space_impl.h"
#include "msg_history_read_request_bs.h"
#include "sopc_address_space_utils_internal.h"
#include "util_b2c.h"
#include "util_variant.h"

#include "sopc_logger.h"
#include "sopc_macros.h"

#define DEFAULT_BINARY "Default Binary"
static const SOPC_String SOPC_DEFAULT_BINARY = SOPC_STRING(DEFAULT_BINARY);
const SOPC_NodeId structureNodeId = SOPC_NODEID_NS0_NUMERIC(OpcUaId_Structure);

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void msg_history_read_request_bs__INITIALISATION(void)
{ /*Translated from B but an intialisation is not needed from this module.*/
}

/*--------------------
   OPERATIONS Clause
  --------------------*/

void msg_history_read_request_bs__get_msg_hist_read_req_TSToReturn(
    const constants__t_msg_i msg_history_read_request_bs__p_req_msg,
    constants__t_TimestampsToReturn_i* const msg_history_read_request_bs__p_tsToReturn)
{
    OpcUa_HistoryReadRequest* msg_hist_read_req = (OpcUa_HistoryReadRequest*) msg_history_read_request_bs__p_req_msg;
    *msg_history_read_request_bs__p_tsToReturn = util_TimestampsToReturn__C_to_B(msg_hist_read_req->TimestampsToReturn);
}

void msg_history_read_request_bs__get_msg_hist_read_req_nb_nodes_to_read(
    const constants__t_msg_i msg_history_read_request_bs__p_req_msg,
    t_entier4* const msg_history_read_request_bs__p_nb_nodes_to_read)
{
    OpcUa_HistoryReadRequest* msg_hist_read_req = (OpcUa_HistoryReadRequest*) msg_history_read_request_bs__p_req_msg;

    /* Return a NAT value */
    if (msg_hist_read_req->NoOfNodesToRead >= 0)
    {
        *msg_history_read_request_bs__p_nb_nodes_to_read = msg_hist_read_req->NoOfNodesToRead;
    }
    else
    {
        *msg_history_read_request_bs__p_nb_nodes_to_read = 0;
    }
}

void msg_history_read_request_bs__get_msg_hist_read_req_release_CP(
    const constants__t_msg_i msg_history_read_request_bs__p_req_msg,
    t_bool* const msg_history_read_request_bs__p_continuation_point)
{
    OpcUa_HistoryReadRequest* msg_hist_read_req = (OpcUa_HistoryReadRequest*) msg_history_read_request_bs__p_req_msg;
    *msg_history_read_request_bs__p_continuation_point = msg_hist_read_req->ReleaseContinuationPoints;
}

void msg_history_read_request_bs__getall_msg_hist_read_req_read_details(
    const constants__t_msg_i msg_history_read_request_bs__p_req_msg,
    constants_statuscodes_bs__t_StatusCode_i* const msg_history_read_request_bs__p_sc,
    constants__t_readRawModifiedDetails_i* const msg_history_read_request_bs__p_hist_read_details)
{
    OpcUa_HistoryReadRequest* msg_hist_read_req = (OpcUa_HistoryReadRequest*) msg_history_read_request_bs__p_req_msg;
    SOPC_ExtObjectBodyEncoding encoding = msg_hist_read_req->HistoryReadDetails.Encoding;
    if (encoding != SOPC_ExtObjBodyEncoding_Object)
    {
        *msg_history_read_request_bs__p_sc = constants_statuscodes_bs__e_sc_bad_history_operation_invalid;
        *msg_history_read_request_bs__p_hist_read_details = constants_bs__c_readRawModifiedDetails_indet;
    }
    else
    {
        SOPC_EncodeableType* type = msg_hist_read_req->HistoryReadDetails.Body.Object.ObjType;
        if (type != &OpcUa_ReadRawModifiedDetails_EncodeableType)
        {
            *msg_history_read_request_bs__p_sc = constants_statuscodes_bs__e_sc_bad_history_operation_unsupported;
            *msg_history_read_request_bs__p_hist_read_details = constants_bs__c_readRawModifiedDetails_indet;
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "Unsupported HistoryReadDetails type: only ReadRawModifiedDetails is supported, but "
                                   "received type: '%s'.",
                                   type->TypeName);
        }
        else
        {
            *msg_history_read_request_bs__p_sc = constants_statuscodes_bs__e_sc_ok;
            *msg_history_read_request_bs__p_hist_read_details =
                (constants__t_readRawModifiedDetails_i) msg_hist_read_req->HistoryReadDetails.Body.Object.Value;
        }
    }
}

void msg_history_read_request_bs__getall_msg_hist_read_req_singleValueId(
    const constants__t_msg_i msg_history_read_request_bs__p_req_msg,
    const t_entier4 msg_history_read_request_bs__p_index,
    constants_statuscodes_bs__t_StatusCode_i* const msg_history_read_request_bs__p_sc,
    constants__t_historyReadValueId_i* const msg_history_read_request_bs__p_singleValueId,
    constants__t_NodeId_i* const msg_history_read_request_bs__p_nodeId)
{
    *msg_history_read_request_bs__p_nodeId = constants__c_NodeId_indet;
    OpcUa_HistoryReadRequest* msg_hist_read_req = (OpcUa_HistoryReadRequest*) msg_history_read_request_bs__p_req_msg;

    OpcUa_HistoryReadValueId* singleValueId = &msg_hist_read_req->NodesToRead[msg_history_read_request_bs__p_index - 1];

    /* Check the validity of the nodeId and get the node */
    SOPC_NodeId* nodeId = &singleValueId->NodeId;
    bool node_found = false;
    SOPC_AddressSpace_Node* node = SOPC_AddressSpace_Get_Node(address_space_bs__nodes, nodeId, &node_found);

    if (node_found)
    {
        /* The nodeId is in the address space, get the node and the variant */
        *msg_history_read_request_bs__p_nodeId = nodeId;
        if (node->node_class == OpcUa_NodeClass_Variable)
        {
            SOPC_QualifiedName dataEncoding =
                msg_hist_read_req->NodesToRead[msg_history_read_request_bs__p_index - 1].DataEncoding;
            if (dataEncoding.Name.Length <= 0 && 0 == dataEncoding.NamespaceIndex)
            {
                /* Data encoding field is empty */
                *msg_history_read_request_bs__p_sc = constants_statuscodes_bs__e_sc_ok;
            }
            else
            {
                /* Check if the DataType is a subtype of Structure */
                SOPC_NodeId* dataTypeId = &node->data.variable.DataType;
                bool res = SOPC_AddressSpaceUtil_RecursiveIsTransitiveSubtype(
                    address_space_bs__nodes, SOPC_RECURSION_LIMIT, dataTypeId, dataTypeId, &structureNodeId);
                if (res)
                {
                    /* The DataType is a structure : we only support binary encoding */
                    bool is_binary = SOPC_String_Equal(&SOPC_DEFAULT_BINARY, &dataEncoding.Name);
                    if (is_binary)
                    {
                        *msg_history_read_request_bs__p_sc = constants_statuscodes_bs__e_sc_ok;
                    }
                    else
                    {
                        *msg_history_read_request_bs__p_singleValueId = constants__c_historyReadValueId_indet;
                        *msg_history_read_request_bs__p_sc =
                            constants_statuscodes_bs__e_sc_bad_data_encoding_unsupported;
                    }
                }
                else
                {
                    /* An encoding was requested on a non-Structure type */
                    *msg_history_read_request_bs__p_sc = constants_statuscodes_bs__e_sc_bad_data_encoding_invalid;
                    *msg_history_read_request_bs__p_singleValueId = constants__c_historyReadValueId_indet;
                }
            }

            if (*msg_history_read_request_bs__p_sc == constants_statuscodes_bs__e_sc_ok)
            {
                /* Fill the singleValueId field */
                *msg_history_read_request_bs__p_singleValueId = singleValueId;

                /* Check index range */
                SOPC_String l_indexRange = singleValueId->IndexRange;
                SOPC_String* string_null = SOPC_String_Create();
                int32_t comparison = -1;
                SOPC_ReturnStatus status = SOPC_String_Compare(&l_indexRange, string_null, false, &comparison);
                SOPC_String_Delete(string_null);

                /* If there is an index range, value must be array */
                if (0 != comparison && status == SOPC_STATUS_OK)
                {
                    SOPC_Variant* l_variant = SOPC_AddressSpace_Get_Value(address_space_bs__nodes, node);
                    if (NULL != l_variant)
                    {
                        if (SOPC_VariantArrayType_Array == l_variant->ArrayType)
                        {
                            SOPC_NumericRange* range = NULL;
                            status = SOPC_NumericRange_Parse(SOPC_String_GetRawCString(&l_indexRange), &range);

                            if (status == SOPC_STATUS_OK)
                            {
                                /* The indexRange is well-defined */
                                *msg_history_read_request_bs__p_sc = constants_statuscodes_bs__e_sc_ok;
                            }
                            else
                            {
                                *msg_history_read_request_bs__p_sc =
                                    constants_statuscodes_bs__e_sc_bad_index_range_invalid;
                            }

                            SOPC_NumericRange_Delete(range);
                        }
                        else
                        {
                            /* The value of the index range is not an array */
                            *msg_history_read_request_bs__p_sc = constants_statuscodes_bs__e_sc_bad_index_range_no_data;
                        }
                    }
                }
                else
                {
                    *msg_history_read_request_bs__p_sc = constants_statuscodes_bs__e_sc_ok;
                }
            }
        }
        else
        {
            /* The node_class isn't OpcUa_NodeClass_Variable */
            *msg_history_read_request_bs__p_sc = constants_statuscodes_bs__e_sc_bad_history_operation_unsupported;
        }
    }
    else
    {
        /* The nodeId is not in the address space */
        *msg_history_read_request_bs__p_sc = constants_statuscodes_bs__e_sc_bad_node_id_unknown;
    }
}
