/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
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
 * Implements the getters for the write request.
 */

#include "service_browse_decode_bs.h"

#include "sopc_types.h"

#include "util_b2c.h"

/* Globals */
static OpcUa_BrowseRequest* request;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void service_browse_decode_bs__INITIALISATION(void)
{
    request = NULL;
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void service_browse_decode_bs__decode_browse_request(
    const constants__t_msg_i service_browse_decode_bs__req_payload,
    constants__t_StatusCode_i* const service_browse_decode_bs__StatusCode_service)
{
    /* TODO: actually decode something */
    SOPC_EncodeableType* encType = *(SOPC_EncodeableType**) service_browse_decode_bs__req_payload;
    *service_browse_decode_bs__StatusCode_service = constants__e_sc_bad_unexpected_error;

    if (encType == &OpcUa_BrowseRequest_EncodeableType)
    {
        OpcUa_BrowseRequest* req = (OpcUa_BrowseRequest*) service_browse_decode_bs__req_payload;

        if (0 == req->NoOfNodesToBrowse)
            *service_browse_decode_bs__StatusCode_service = constants__e_sc_bad_nothing_to_do;
        else if (req->NoOfNodesToBrowse > constants__k_n_BrowseResponse_max)
            *service_browse_decode_bs__StatusCode_service = constants__e_sc_bad_too_many_ops;
        else
        {
            /* TODO: req shall not be freed before request is null... */
            request = req;
            *service_browse_decode_bs__StatusCode_service = constants__e_sc_ok;
        }
    }
}

void service_browse_decode_bs__free_browse_request(void)
{
    /* TODO: don't free the request that you did not initialize */
    request = NULL;
}

void service_browse_decode_bs__get_nb_BrowseValue(t_entier4* const service_browse_decode_bs__nb_req)
{
    if (NULL != request)
        *service_browse_decode_bs__nb_req = request->NoOfNodesToBrowse;
    else
        *service_browse_decode_bs__nb_req = 0;
}

void service_browse_decode_bs__get_nb_BrowseTargetMax(t_entier4* const service_browse_decode_bs__p_nb_BrowseTargetMax)
{
    if (NULL == request)
        *service_browse_decode_bs__p_nb_BrowseTargetMax = 0;
    else
    {
        if (request->RequestedMaxReferencesPerNode < INT32_MAX)
        {
            *service_browse_decode_bs__p_nb_BrowseTargetMax = (int32_t) request->RequestedMaxReferencesPerNode;
        }
        else
        {
            *service_browse_decode_bs__p_nb_BrowseTargetMax = INT32_MAX;
        }
        if (0 == *service_browse_decode_bs__p_nb_BrowseTargetMax ||
            *service_browse_decode_bs__p_nb_BrowseTargetMax > constants__k_n_BrowseTarget_max)
            *service_browse_decode_bs__p_nb_BrowseTargetMax = constants__k_n_BrowseTarget_max;
    }
}

extern void service_browse_decode_bs__get_BrowseView(constants__t_NodeId_i* const service_browse_decode_bs__p_nid_view)
{
    SOPC_NodeId* pVid = NULL;

    *service_browse_decode_bs__p_nid_view = constants__c_NodeId_indet;

    if (request != NULL)
    {
        pVid = &(request->View.ViewId);
        if (pVid->IdentifierType != SOPC_IdentifierType_Numeric || pVid->Data.Numeric != 0)
        {
            *service_browse_decode_bs__p_nid_view = pVid;
        }
    }
}

void service_browse_decode_bs__getall_BrowseValue(const constants__t_BrowseValue_i service_browse_decode_bs__p_bvi,
                                                  t_bool* const service_browse_decode_bs__p_isvalid,
                                                  constants__t_NodeId_i* const service_browse_decode_bs__p_NodeId,
                                                  constants__t_BrowseDirection_i* const service_browse_decode_bs__p_dir,
                                                  t_bool* const service_browse_decode_bs__p_isreftype,
                                                  constants__t_NodeId_i* const service_browse_decode_bs__p_reftype,
                                                  t_bool* const service_browse_decode_bs__p_inc_subtype)
{
    SOPC_NodeId* pNid = NULL;
    OpcUa_BrowseDescription* pBwseDesc = NULL;

    /* Default value for every output */
    *service_browse_decode_bs__p_isvalid = false;
    *service_browse_decode_bs__p_NodeId = constants__c_NodeId_indet;
    *service_browse_decode_bs__p_dir = constants__e_bd_indet;
    *service_browse_decode_bs__p_isreftype = false;
    *service_browse_decode_bs__p_reftype = constants__c_NodeId_indet;
    *service_browse_decode_bs__p_inc_subtype = false;

    if (NULL != request && service_browse_decode_bs__p_bvi > 0)
    /* && 0 < service_browse_decode_bs__p_bvi && service_browse_decode_bs__p_bvi <=
request->NoOfNodesToBrowse) These are already verified by PRE */
    {
        pBwseDesc = &request->NodesToBrowse[service_browse_decode_bs__p_bvi - 1];
        *service_browse_decode_bs__p_NodeId = &pBwseDesc->NodeId;
        /* Invalid direction is tested by the B, so it's is not a reason to unset p_isvalid */
        util_BrowseDirection__C_to_B(pBwseDesc->BrowseDirection, service_browse_decode_bs__p_dir);

        /* TODO: Have a clearer definition of what a "not specified ReferenceType" is... */
        pNid = &pBwseDesc->ReferenceTypeId;
        if (!(pNid->IdentifierType == SOPC_IdentifierType_Numeric && pNid->Data.Numeric == 0))
        {
            *service_browse_decode_bs__p_isreftype = true;
            *service_browse_decode_bs__p_reftype = pNid;
            *service_browse_decode_bs__p_inc_subtype = pBwseDesc->IncludeSubtypes;
        }

        *service_browse_decode_bs__p_isvalid = true;
    }
}
