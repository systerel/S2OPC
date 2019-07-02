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
#include <stddef.h>

#include "msg_browse_bs.h"
#include "sopc_mem_alloc.h"
#include "util_b2c.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void msg_browse_bs__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void msg_browse_bs__alloc_browse_response(const constants__t_msg_i msg_browse_bs__p_resp_msg,
                                          const t_entier4 msg_browse_bs__p_nb_bvi,
                                          t_bool* const msg_browse_bs__p_isallocated)
{
    *msg_browse_bs__p_isallocated = false;
    OpcUa_BrowseResponse* resp = msg_browse_bs__p_resp_msg;
    assert((uint64_t) msg_browse_bs__p_nb_bvi < SIZE_MAX);
    assert(msg_browse_bs__p_nb_bvi > 0);
    resp->Results = SOPC_Malloc(sizeof(*resp->Results) * (size_t) msg_browse_bs__p_nb_bvi);
    if (NULL != resp->Results)
    {
        for (int32_t i = 0; i < msg_browse_bs__p_nb_bvi; i++)
        {
            OpcUa_BrowseResult_Initialize(&resp->Results[i]);
        }
        resp->NoOfResults = msg_browse_bs__p_nb_bvi;
        *msg_browse_bs__p_isallocated = true;
    }
}

void msg_browse_bs__get_browse_request_params(const constants__t_msg_i msg_browse_bs__p_req_msg,
                                              constants__t_NodeId_i* const msg_browse_bs__p_nid_view,
                                              t_entier4* const msg_browse_bs__p_nb_BrowseTargetMax,
                                              t_entier4* const msg_browse_bs__p_nb_browse_value)
{
    OpcUa_BrowseRequest* req = msg_browse_bs__p_req_msg;
    if (SOPC_NodeId_IsNull(&req->View.ViewId))
    {
        *msg_browse_bs__p_nid_view = constants__c_NodeId_indet;
    }
    else
    {
        // Note: timestamp and version ignored
        *msg_browse_bs__p_nid_view = &req->View.ViewId;
    }
    if (req->RequestedMaxReferencesPerNode <= INT32_MAX)
    {
        *msg_browse_bs__p_nb_BrowseTargetMax = (int32_t) req->RequestedMaxReferencesPerNode;
    }
    else
    {
        *msg_browse_bs__p_nb_BrowseTargetMax = INT32_MAX;
    }
    *msg_browse_bs__p_nb_browse_value = req->NoOfNodesToBrowse;
}

void msg_browse_bs__getall_BrowseValue(const constants__t_msg_i msg_browse_bs__p_req_msg,
                                       const constants__t_BrowseValue_i msg_browse_bs__p_bvi,
                                       constants__t_NodeId_i* const msg_browse_bs__p_NodeId,
                                       constants__t_BrowseDirection_i* const msg_browse_bs__p_dir,
                                       constants__t_NodeId_i* const msg_browse_bs__p_reftype,
                                       t_bool* const msg_browse_bs__p_inc_subtype,
                                       constants__t_BrowseNodeClassMask_i* const msg_browse_bs__p_class_mask,
                                       constants__t_BrowseResultMask_i* const msg_browse_bs__p_result_mask)
{
    OpcUa_BrowseRequest* req = msg_browse_bs__p_req_msg;
    // Note: msg_browse_bs__p_bvi : 1..N to be translated into C array index by adding (-1)
    assert(msg_browse_bs__p_bvi > 0);
    assert(msg_browse_bs__p_bvi <= req->NoOfNodesToBrowse);
    OpcUa_BrowseDescription* bdesc = &req->NodesToBrowse[msg_browse_bs__p_bvi - 1];
    util_NodeId_borrowReference_or_indet__C_to_B(msg_browse_bs__p_NodeId, &bdesc->NodeId);
    *msg_browse_bs__p_dir = util_BrowseDirection__C_to_B(bdesc->BrowseDirection);
    util_NodeId_borrowReference_or_indet__C_to_B(msg_browse_bs__p_reftype, &bdesc->ReferenceTypeId);
    *msg_browse_bs__p_inc_subtype = bdesc->IncludeSubtypes;
    *msg_browse_bs__p_class_mask = bdesc->NodeClassMask;
    *msg_browse_bs__p_result_mask = bdesc->ResultMask;
}

void msg_browse_bs__set_ResponseBrowse_BrowseResult(
    const constants__t_msg_i msg_browse_bs__p_resp_msg,
    const constants__t_BrowseValue_i msg_browse_bs__p_bvi,
    const t_entier4 msg_browse_bs__p_nb_targets,
    const constants__t_BrowseResultReferences_i msg_browse_bs__p_browseResultReferences)
{
    OpcUa_BrowseResponse* resp = msg_browse_bs__p_resp_msg;
    // Note: msg_browse_bs__p_bvi : 1..N to be translated into C array index by adding (-1)
    assert(msg_browse_bs__p_bvi > 0);
    assert(msg_browse_bs__p_bvi <= resp->NoOfResults);
    OpcUa_BrowseResult* res = &resp->Results[msg_browse_bs__p_bvi - 1];
    res->NoOfReferences = msg_browse_bs__p_nb_targets;
    res->References = msg_browse_bs__p_browseResultReferences;
}

void msg_browse_bs__set_ResponseBrowse_BrowseStatus(const constants__t_msg_i msg_browse_bs__p_resp_msg,
                                                    const constants__t_BrowseValue_i msg_browse_bs__p_bvi,
                                                    const constants_statuscodes_bs__t_StatusCode_i msg_browse_bs__p_sc)
{
    OpcUa_BrowseResponse* resp = msg_browse_bs__p_resp_msg;
    // Note: msg_browse_bs__p_bvi : 1..N to be translated into C array index by adding (-1)
    assert(msg_browse_bs__p_bvi > 0);
    assert(msg_browse_bs__p_bvi <= resp->NoOfResults);
    OpcUa_BrowseResult* res = &resp->Results[msg_browse_bs__p_bvi - 1];
    util_status_code__B_to_C(msg_browse_bs__p_sc, &res->StatusCode);
}

void msg_browse_bs__set_ResponseBrowse_ContinuationPoint(
    const constants__t_msg_i msg_browse_bs__p_resp_msg,
    const constants__t_BrowseValue_i msg_browse_bs__p_bvi,
    const constants__t_ContinuationPointId_i msg_browse_bs__p_continuationPointId)
{
    if (msg_browse_bs__p_continuationPointId != constants__c_ContinuationPointId_indet)
    {
        OpcUa_BrowseResponse* resp = msg_browse_bs__p_resp_msg;
        // Note: msg_browse_bs__p_bvi : 1..N to be translated into C array index by adding (-1)
        assert(msg_browse_bs__p_bvi > 0);
        assert(msg_browse_bs__p_bvi <= resp->NoOfResults);
        OpcUa_BrowseResult* res = &resp->Results[msg_browse_bs__p_bvi - 1];
        SOPC_ReturnStatus status =
            SOPC_ContinuationPointId_Encode(msg_browse_bs__p_continuationPointId, &res->ContinuationPoint);
        assert(SOPC_STATUS_OK == status);
    }
}
