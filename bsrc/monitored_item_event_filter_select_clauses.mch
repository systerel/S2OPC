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
    monitored_item_event_filter_select_clauses

SEES
    constants,
    constants_statuscodes_bs,
    monitored_item_event_filter_treatment_bs

ABSTRACT_VARIABLES
    a_selectClauseResultSet

INVARIANT
    a_selectClauseResultSet <: NAT

INITIALISATION
    a_selectClauseResultSet
    :(
      a_selectClauseResultSet = {}
     )

OPERATIONS

    scSelectClauses, hasInvalidClause <-- check_select_clauses_and_fill_ctx_and_result (p_filterCtx, p_filterResult, p_nbSelectClauses) =
    PRE
        c_filter : t_monitoringFilter_i &
        c_filter : t_monitoringFilter &
        p_filterCtx : t_monitoringFilterCtx_i &
        p_filterCtx : t_monitoringFilterCtx &
        p_filterResult : t_filterResult_i &
        p_filterResult : t_filterResult &
        p_nbSelectClauses : NAT &
        p_nbSelectClauses > 0 &
        p_nbSelectClauses = a_filter_NbSelectClauses
    THEN
        a_selectClauseResultSet,
        scSelectClauses, hasInvalidClause
        :(
            a_selectClauseResultSet = 1..p_nbSelectClauses &
            scSelectClauses : t_StatusCode_i &
            hasInvalidClause : BOOL &
            (scSelectClauses = e_sc_ok =>
                /* At least one select clause is valid */
                (TRUE : a_eventSelectClauseValid[1..p_nbSelectClauses]) &
                /* Flag for at least one invalid select clause */
                hasInvalidClause = bool(FALSE : a_eventSelectClauseValid[1..p_nbSelectClauses])
            ) &
            (scSelectClauses /= e_sc_ok =>
                scSelectClauses : {e_sc_bad_out_of_memory,
                                   e_sc_bad_monitored_item_filter_invalid,
                                   e_sc_bad_monitored_item_filter_unsupported}
            )
        )
    END
    ;

    notifTriggered, eventNotif <-- apply_select_clauses_and_build_event_field_list
                                             (p_clientHandle, p_localeIds,
                                              p_timestampToReturn, p_userAccessGranted,
                                              p_filterCtx, p_nbSelectClauses, p_event) =
    PRE
        p_clientHandle : t_client_handle_i &
        p_localeIds : t_LocaleIds_i &
        p_localeIds : t_LocaleIds &
        p_timestampToReturn : t_TimestampsToReturn_i &
        p_timestampToReturn : t_TimestampsToReturn &
        p_userAccessGranted : BOOL &
        p_filterCtx : t_monitoringFilterCtx_i &
        p_filterCtx : t_monitoringFilterCtx &
        c_filterCtx = p_filterCtx &
        p_nbSelectClauses : NAT &
        p_nbSelectClauses = a_filterCtx_NbSelectClauses &
        p_event : t_Event_i &
        p_event : t_Event &
        c_event = p_event &
        a_eventTypeId : t_NodeId
    THEN
        notifTriggered,
        eventNotif
        :(notifTriggered : BOOL &
          eventNotif : t_eventFieldList_i &
          (notifTriggered = TRUE => eventNotif : t_eventFieldList) &
          (notifTriggered = FALSE => eventNotif = c_eventFieldList_indet)
        )
    END

END
