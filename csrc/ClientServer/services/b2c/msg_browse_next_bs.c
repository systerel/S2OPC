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

#include "msg_browse_next_bs.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include "sopc_mem_alloc.h"
#include "util_b2c.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void msg_browse_next_bs__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void msg_browse_next_bs__alloc_browse_next_response(const constants__t_msg_i msg_browse_next_bs__p_resp_msg,
                                                    const t_entier4 msg_browse_next_bs__p_nb_bvi,
                                                    t_bool* const msg_browse_next_bs__p_isallocated)
{
    *msg_browse_next_bs__p_isallocated = false;
    OpcUa_BrowseNextResponse* resp = msg_browse_next_bs__p_resp_msg;
    assert((uint64_t) msg_browse_next_bs__p_nb_bvi < SIZE_MAX);
    resp->Results = SOPC_Malloc(sizeof(*resp->Results) * (size_t) msg_browse_next_bs__p_nb_bvi);
    if (NULL != resp->Results)
    {
        for (int32_t i = 0; i < msg_browse_next_bs__p_nb_bvi; i++)
        {
            OpcUa_BrowseResult_Initialize(&resp->Results[i]);
        }
        resp->NoOfResults = msg_browse_next_bs__p_nb_bvi;
        *msg_browse_next_bs__p_isallocated = true;
    }
}

void msg_browse_next_bs__get_browse_next_request_params(const constants__t_msg_i msg_browse_next_bs__p_req_msg,
                                                        t_bool* const msg_browse_next_bs__p_releaseContinuationPoints,
                                                        t_entier4* const msg_browse_next_bs__p_nb_ContinuationPoints)
{
    OpcUa_BrowseNextRequest* req = msg_browse_next_bs__p_req_msg;
    *msg_browse_next_bs__p_releaseContinuationPoints = util_SOPC_Boolean_to_B(req->ReleaseContinuationPoints);

    if (req->NoOfContinuationPoints > 0)
    {
        *msg_browse_next_bs__p_nb_ContinuationPoints = req->NoOfContinuationPoints;
    }
    else
    {
        *msg_browse_next_bs__p_nb_ContinuationPoints = 0;
    }
}

void msg_browse_next_bs__getall_ContinuationPoint(
    const constants__t_msg_i msg_browse_next_bs__p_req_msg,
    const constants__t_BrowseValue_i msg_browse_next_bs__p_cpi,
    constants__t_ContinuationPointId_i* const msg_browse_next_bs__p_ContinuationPointId)
{
    OpcUa_BrowseNextRequest* req = msg_browse_next_bs__p_req_msg;
    // Note: msg_browse_next_bs__p_cpi : 1..N to be translated into C array index by adding (-1)
    assert(msg_browse_next_bs__p_cpi > 0);
    assert(msg_browse_next_bs__p_cpi <= req->NoOfContinuationPoints);
    SOPC_ReturnStatus status = SOPC_ContinuationPointId_Decode(&req->ContinuationPoints[msg_browse_next_bs__p_cpi - 1],
                                                               msg_browse_next_bs__p_ContinuationPointId);
    if (SOPC_STATUS_OK != status)
    {
        *msg_browse_next_bs__p_ContinuationPointId = constants__c_ContinuationPointId_indet;
    }
}

void msg_browse_next_bs__set_ResponseBrowseNext_BrowseResult(
    const constants__t_msg_i msg_browse_next_bs__p_resp_msg,
    const constants__t_BrowseValue_i msg_browse_next_bs__p_bvi,
    const t_entier4 msg_browse_next_bs__p_nb_targets,
    const constants__t_BrowseResultReferences_i msg_browse_next_bs__p_browseResultReferences)
{
    OpcUa_BrowseNextResponse* resp = msg_browse_next_bs__p_resp_msg;
    // Note: msg_browse_next_bs__p_bvi : 1..N to be translated into C array index by adding (-1)
    assert(msg_browse_next_bs__p_bvi > 0);
    assert(msg_browse_next_bs__p_bvi <= resp->NoOfResults);
    OpcUa_BrowseResult* res = &resp->Results[msg_browse_next_bs__p_bvi - 1];
    res->NoOfReferences = msg_browse_next_bs__p_nb_targets;
    res->References = msg_browse_next_bs__p_browseResultReferences;
}

void msg_browse_next_bs__set_ResponseBrowseNext_BrowseStatus(
    const constants__t_msg_i msg_browse_next_bs__p_resp_msg,
    const constants__t_BrowseValue_i msg_browse_next_bs__p_bvi,
    const constants_statuscodes_bs__t_StatusCode_i msg_browse_next_bs__p_sc)
{
    OpcUa_BrowseNextResponse* resp = msg_browse_next_bs__p_resp_msg;
    // Note: msg_browse_next_bs__p_bvi : 1..N to be translated into C array index by adding (-1)
    assert(msg_browse_next_bs__p_bvi > 0);
    assert(msg_browse_next_bs__p_bvi <= resp->NoOfResults);
    OpcUa_BrowseResult* res = &resp->Results[msg_browse_next_bs__p_bvi - 1];
    util_status_code__B_to_C(msg_browse_next_bs__p_sc, &res->StatusCode);
}

void msg_browse_next_bs__set_ResponseBrowseNext_ContinuationPoint(
    const constants__t_msg_i msg_browse_next_bs__p_resp_msg,
    const constants__t_BrowseValue_i msg_browse_next_bs__p_bvi,
    const constants__t_ContinuationPointId_i msg_browse_next_bs__p_continuationPointId)
{
    if (msg_browse_next_bs__p_continuationPointId != constants__c_ContinuationPointId_indet)
    {
        OpcUa_BrowseNextResponse* resp = msg_browse_next_bs__p_resp_msg;
        // Note: msg_browse_next_bs__p_bvi : 1..N to be translated into C array index by adding (-1)
        assert(msg_browse_next_bs__p_bvi > 0);
        assert(msg_browse_next_bs__p_bvi <= resp->NoOfResults);
        OpcUa_BrowseResult* res = &resp->Results[msg_browse_next_bs__p_bvi - 1];
        SOPC_ReturnStatus status =
            SOPC_ContinuationPointId_Encode(msg_browse_next_bs__p_continuationPointId, &res->ContinuationPoint);
        assert(SOPC_STATUS_OK == status);
    }
}
