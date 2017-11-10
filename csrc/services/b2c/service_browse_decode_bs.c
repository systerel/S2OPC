/*
 *  Copyright (C) 2017 Systerel and others.
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

/** \file
 *
 * Implements the getters for the write request.
 */


#include "service_browse_decode_bs.h"

#include "sopc_types.h"

#include "util_b2c.h"


/* Globals */
static OpcUa_BrowseRequest *request;


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
   constants__t_StatusCode_i * const service_browse_decode_bs__StatusCode_service)
{
    /* TODO: actually decode something */
    SOPC_EncodeableType* encType = *(SOPC_EncodeableType**) service_browse_decode_bs__req_payload;
    *service_browse_decode_bs__StatusCode_service = constants__e_sc_nok;

    if(encType == &OpcUa_BrowseRequest_EncodeableType){
        OpcUa_BrowseRequest *req = (OpcUa_BrowseRequest*) service_browse_decode_bs__req_payload;

        if(0 == req->NoOfNodesToBrowse)
            *service_browse_decode_bs__StatusCode_service = constants__e_sc_bad_nothing_to_do;
        else if(req->NoOfNodesToBrowse > constants__k_n_BrowseResponse_max)
            *service_browse_decode_bs__StatusCode_service = constants__e_sc_bad_too_many_ops;
        else {
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


void service_browse_decode_bs__get_nb_BrowseValue(
   t_entier4 * const service_browse_decode_bs__nb_req)
{
    if(NULL != request)
        *service_browse_decode_bs__nb_req = request->NoOfNodesToBrowse;
    else
        *service_browse_decode_bs__nb_req = 0;
}


void service_browse_decode_bs__get_nb_BrowseTargetMax(
   t_entier4 * const service_browse_decode_bs__p_nb_BrowseTargetMax)
{
    if(NULL == request)
        *service_browse_decode_bs__p_nb_BrowseTargetMax = 0;
    else {
        *service_browse_decode_bs__p_nb_BrowseTargetMax = request->RequestedMaxReferencesPerNode;
        if(0 == *service_browse_decode_bs__p_nb_BrowseTargetMax ||
           *service_browse_decode_bs__p_nb_BrowseTargetMax > constants__k_n_BrowseTarget_max)
            *service_browse_decode_bs__p_nb_BrowseTargetMax = constants__k_n_BrowseTarget_max;
    }
}


void service_browse_decode_bs__getall_BrowseValue(
   const constants__t_BrowseValue_i service_browse_decode_bs__p_bvi,
   t_bool * const service_browse_decode_bs__p_isvalid,
   constants__t_NodeId_i * const service_browse_decode_bs__p_NodeId,
   constants__t_BrowseDirection_i * const service_browse_decode_bs__p_dir,
   t_bool * const service_browse_decode_bs__p_isreftype,
   constants__t_NodeId_i * const service_browse_decode_bs__p_reftype,
   t_bool * const service_browse_decode_bs__p_inc_subtype)
{
    SOPC_NodeId *pNid = NULL;
    OpcUa_BrowseDescription *pBwseDesc = NULL;

    /* Default value for every output */
    *service_browse_decode_bs__p_isvalid = false;
    *service_browse_decode_bs__p_NodeId = constants__c_NodeId_indet;
    *service_browse_decode_bs__p_dir = constants__e_bd_indet;
    *service_browse_decode_bs__p_isreftype = false;
    *service_browse_decode_bs__p_reftype = constants__c_NodeId_indet;
    *service_browse_decode_bs__p_inc_subtype = false;

    if(NULL != request) /* && 0 < service_browse_decode_bs__p_bvi && service_browse_decode_bs__p_bvi <= request->NoOfNodesToBrowse) These are already verified by PRE */
    {
        pBwseDesc = &request->NodesToBrowse[service_browse_decode_bs__p_bvi-1];
        *service_browse_decode_bs__p_NodeId = (constants__t_NodeId_i *)&pBwseDesc->NodeId;
        if(false == util_BrowseDirection__C_to_B(pBwseDesc->BrowseDirection, service_browse_decode_bs__p_dir))
            return;

        /* TODO: Have a clearer definition of what a "not specified ReferenceType" is... */
        pNid = &pBwseDesc->ReferenceTypeId;
        if(! (pNid->IdentifierType == SOPC_IdentifierType_Numeric && pNid->Data.Numeric == 0)) {
            *service_browse_decode_bs__p_isreftype = true;
            *service_browse_decode_bs__p_reftype = (constants__t_NodeId_i)pNid;
            *service_browse_decode_bs__p_inc_subtype = pBwseDesc->IncludeSubtypes;
        }

        *service_browse_decode_bs__p_isvalid = true;
    }
}

