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

MACHINE
    msg_browse_next_bs

SEES
    constants,
    constants_statuscodes_bs,
    message_in_bs,
    message_out_bs

DEFINITIONS
    d_variables_req ==
        RequestBrowseNext,
        RequestBrowseNext_ReleaseContinuationPoints,
        RequestBrowseNext_ContinuationPointId
    ;
    d_variables_resp ==
        ResponseBrowseNext,
        ResponseBrowseNext_BrowseStatus,
        ResponseBrowseNext_ContinuationPoint,
        /* The result array of complying references starting from browsed node for each browse request item */
        ResponseBrowseNext_ResultReferences,
        isBrowseNextResponseAllocated
    ;
    d_inv_req ==
        RequestBrowseNext                           : t_msg_i &
        RequestBrowseNext_ReleaseContinuationPoints : BOOL &
        RequestBrowseNext_ContinuationPointId       : t_BrowseValue +-> t_ContinuationPointId_i &
        dom(RequestBrowseNext_ContinuationPointId) = cast_t_BrowseValue[1..card(RequestBrowseNext_ContinuationPointId)]
    ;
    d_inv_resp ==
        ResponseBrowseNext                     : t_msg_i &
        ResponseBrowseNext_BrowseStatus        : t_BrowseValue +-> t_StatusCode        &
        ResponseBrowseNext_ContinuationPoint   : t_BrowseValue +-> t_ContinuationPointId_i &
        ResponseBrowseNext_ResultReferences    : t_BrowseValue +-> t_BrowseResultReferences &
        isBrowseNextResponseAllocated : BOOL
    ;
    d_init_req ==
        d_variables_req
        :(
            RequestBrowseNext_ReleaseContinuationPoints = FALSE &
            RequestBrowseNext_ContinuationPointId = {}
        )
    ;
    d_init_resp ==
        d_variables_resp
        :(
            ResponseBrowseNext_BrowseStatus        = {} &
            ResponseBrowseNext_ContinuationPoint   = {} &
            ResponseBrowseNext_ResultReferences    = {} &
            isBrowseNextResponseAllocated = FALSE
         )

ABSTRACT_VARIABLES
    d_variables_req,
    d_variables_resp

INVARIANT
    d_inv_req &
    d_inv_resp

INITIALISATION
    d_init_req ||
    d_init_resp

OPERATIONS

    p_releaseContinuationPoints, p_nb_ContinuationPoints <-- get_browse_next_request_params(p_req_msg) =
    PRE
        p_req_msg : t_msg_i &
        p_req_msg : t_msg &
        p_req_msg = c_msg_in &
        a_msg_in_type = e_msg_view_browse_next_req
    THEN
        p_releaseContinuationPoints,
        p_nb_ContinuationPoints,
        d_variables_req
        :(
            p_releaseContinuationPoints : BOOL &
            p_nb_ContinuationPoints : NAT &
            d_inv_req &
            RequestBrowseNext = p_req_msg &
            p_nb_ContinuationPoints = card(RequestBrowseNext_ContinuationPointId) &
            p_releaseContinuationPoints = RequestBrowseNext_ReleaseContinuationPoints
        )
    END
    ;

    p_ContinuationPointId <-- getall_ContinuationPoint(p_req_msg, p_cpi) =
    PRE
        p_req_msg : t_msg_i &
        p_req_msg : t_msg &
        p_req_msg = c_msg_in &
        a_msg_in_type = e_msg_view_browse_next_req &

        RequestBrowseNext = p_req_msg &

        p_cpi : t_BrowseValue_i &
        p_cpi : t_BrowseValue   &
        p_cpi : dom(RequestBrowseNext_ContinuationPointId)
    THEN
        p_ContinuationPointId := RequestBrowseNext_ContinuationPointId(p_cpi)
    END
    ;

    /* Browse Response */

    p_isallocated <-- alloc_browse_next_response(p_resp_msg, p_nb_bvi) =
    PRE
        p_resp_msg : t_msg_i &
        p_resp_msg : t_msg &
        p_resp_msg = c_msg_out &
        a_msg_out_type = e_msg_view_browse_resp &
        p_nb_bvi : NAT &
        p_nb_bvi = card(RequestBrowseNext_ContinuationPointId)
    THEN
        p_isallocated :: BOOL ||
        d_variables_resp
        :(d_inv_resp &
          isBrowseNextResponseAllocated = p_isallocated &
          ResponseBrowseNext = p_resp_msg &
          ResponseBrowseNext_BrowseStatus        = {} &
          ResponseBrowseNext_ContinuationPoint   = {} &
          ResponseBrowseNext_ResultReferences    = {}
         )
    END
    ;

    set_ResponseBrowseNext_BrowseStatus(p_resp_msg, p_bvi, p_sc) =
    PRE
        isBrowseNextResponseAllocated = TRUE &
        p_resp_msg : t_msg_i &
        p_resp_msg : t_msg &
        p_resp_msg = c_msg_out &
        a_msg_out_type = e_msg_view_browse_resp &
        ResponseBrowseNext = p_resp_msg &
        p_bvi  : t_BrowseValue_i &
        p_bvi  : t_BrowseValue   &
        p_bvi : dom(RequestBrowseNext_ContinuationPointId) &
        p_sc : t_StatusCode_i &
        p_sc : t_StatusCode
    THEN
        ResponseBrowseNext_BrowseStatus(p_bvi) := p_sc
    END
    ;

    set_ResponseBrowseNext_ContinuationPoint(p_resp_msg, p_bvi, p_continuationPointId) =
    PRE
        isBrowseNextResponseAllocated = TRUE &
        p_resp_msg : t_msg_i &
        p_resp_msg : t_msg &
        p_resp_msg = c_msg_out &
        a_msg_out_type = e_msg_view_browse_resp &
        ResponseBrowseNext = p_resp_msg &
        p_bvi : t_BrowseValue_i &
        p_bvi : t_BrowseValue   &
        p_bvi : dom(RequestBrowseNext_ContinuationPointId) &
        p_continuationPointId : t_ContinuationPointId_i   &
        p_continuationPointId : t_ContinuationPointId
    THEN
        ResponseBrowseNext_ContinuationPoint(p_bvi) := p_continuationPointId
    END
    ;

    set_ResponseBrowseNext_BrowseResult(p_resp_msg, p_bvi, p_nb_targets, p_browseResultReferences) =
    PRE
        isBrowseNextResponseAllocated = TRUE &
        p_resp_msg : t_msg_i &
        p_resp_msg : t_msg &
        p_resp_msg = c_msg_out &
        a_msg_out_type = e_msg_view_browse_resp &
        ResponseBrowseNext = p_resp_msg &
        p_bvi          : t_BrowseValue_i  &
        p_bvi          : t_BrowseValue    &
        p_bvi : dom(RequestBrowseNext_ContinuationPointId) &
        p_nb_targets   : NAT &
        p_nb_targets  <= k_n_BrowseTarget_max &
        p_browseResultReferences : t_BrowseResultReferences_i
    THEN
        ResponseBrowseNext_ResultReferences (p_bvi) := p_browseResultReferences
    END

END
