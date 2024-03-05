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
    monitored_item_event_filter_select_clauses_bs

SEES
    constants,
    constants_statuscodes_bs,
    monitored_item_event_filter_treatment_bs

ABSTRACT_VARIABLES
    c_filterResult,
    a_selectClauseResultSet,

    c_eventFieldList,
    a_eventFieldListFilled

INVARIANT
    c_filterResult : t_filterResult_i &
    a_selectClauseResultSet <: NAT &

    c_eventFieldList : t_eventFieldList_i &
    a_eventFieldListFilled <: NAT

INITIALISATION
    c_filterResult,
    a_selectClauseResultSet,
    c_eventFieldList,
    a_eventFieldListFilled
    :(
      c_filterResult = c_filterResult_indet &
      a_selectClauseResultSet = {} &
      c_eventFieldList : t_eventFieldList_i &
      a_eventFieldListFilled = {}
     )

OPERATIONS

    init_select_clause_results (p_filterResult) =
    PRE
        p_filterResult : t_filterResult_i &
        p_filterResult : t_filterResult
    THEN
        c_filterResult := p_filterResult ||
        a_selectClauseResultSet := {}
    END
    ;

    set_select_clause_result (p_filterResult, p_selectClauseIdx, p_rawSc) =
    PRE
        p_filterResult : t_filterResult_i &
        p_filterResult : t_filterResult &
        c_filterResult = p_filterResult &
        p_selectClauseIdx : NAT &
        p_selectClauseIdx : 1..a_filter_NbSelectClauses &
        p_rawSc : t_RawStatusCode
    THEN
       a_selectClauseResultSet := a_selectClauseResultSet \/ {p_selectClauseIdx}
    END
    ;

    bres, p_eventFieldList <-- alloc_event_field_list (p_clientHandle, p_nbSelectClauses) =
    PRE
        p_clientHandle : t_client_handle_i &
        p_nbSelectClauses : NAT &
        p_nbSelectClauses = a_filterCtx_NbSelectClauses
    THEN
        bres, p_eventFieldList, c_eventFieldList, a_eventFieldListFilled :(
            bres : BOOL & p_eventFieldList : t_eventFieldList_i & c_eventFieldList : t_eventFieldList_i &
            p_eventFieldList = c_eventFieldList & a_eventFieldListFilled <: NAT & a_eventFieldListFilled = {} &
            (bres = TRUE => c_eventFieldList : t_eventFieldList) &
            (bres = FALSE => c_eventFieldList = c_eventFieldList_indet)
        )
    END
    ;

    set_event_field_list_elt (p_localeIds,
                              p_timestampToReturn, p_userAccessGranted,
                              p_selectClauseIdx, p_filterCtx,
                              p_eventFieldList, p_event) =
    PRE
        p_localeIds : t_LocaleIds_i &
        p_localeIds : t_LocaleIds &
        p_timestampToReturn : t_TimestampsToReturn_i &
        p_timestampToReturn : t_TimestampsToReturn &
        p_userAccessGranted : BOOL &
        p_selectClauseIdx : NAT &
        p_selectClauseIdx : 1..a_filterCtx_NbSelectClauses &
        p_filterCtx : t_monitoringFilterCtx_i &
        p_filterCtx : t_monitoringFilterCtx &
        c_filterCtx = p_filterCtx &
        p_eventFieldList : t_eventFieldList_i &
        p_eventFieldList : t_eventFieldList &
        c_eventFieldList = p_eventFieldList &
        p_event : t_Event_i &
        p_event : t_Event &
        c_event = p_event
    THEN
        a_eventFieldListFilled := a_eventFieldListFilled \/ {p_selectClauseIdx}
    END
    ;

    set_event_field_list_elt_null (p_selectClauseIdx, p_eventFieldList) =
    PRE
        p_selectClauseIdx : NAT &
        p_selectClauseIdx : 1..a_filterCtx_NbSelectClauses &
        p_eventFieldList : t_eventFieldList_i &
        p_eventFieldList : t_eventFieldList &
        c_eventFieldList = p_eventFieldList
    THEN
        a_eventFieldListFilled := a_eventFieldListFilled \/ {p_selectClauseIdx}
    END

END
