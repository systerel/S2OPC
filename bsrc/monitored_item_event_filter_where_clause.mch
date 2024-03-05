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
    monitored_item_event_filter_where_clause

SEES
    constants,
    constants_statuscodes_bs,
    monitored_item_event_filter_treatment_bs,
    address_space_itf

ABSTRACT_VARIABLES
    a_whereElementResultSet

INVARIANT
    a_whereElementResultSet <: NAT

INITIALISATION
    a_whereElementResultSet
    :(
      a_whereElementResultSet = {}
     )

OPERATIONS

    scWhereClause <-- check_where_clause_and_fill_ctx_and_result (p_filterCtx, p_filterResult, p_nbWhereElts) =
    PRE
        c_filter : t_monitoringFilter_i &
        c_filter : t_monitoringFilter &
        p_filterCtx : t_monitoringFilterCtx_i &
        p_filterCtx : t_monitoringFilterCtx &
        p_filterResult : t_filterResult_i &
        p_filterResult : t_filterResult &
        p_nbWhereElts : NAT &
        p_nbWhereElts = a_filter_NbWhereClauseElements
    THEN
        a_whereElementResultSet,
        scWhereClause
        :(
            a_whereElementResultSet = 1..p_nbWhereElts &
            scWhereClause : t_StatusCode_i &
            (p_nbWhereElts = 0 or
             p_nbWhereElts = 1 & a_whereClauseElementValid = TRUE =>
                /* No element or only one valid (1 valid OfType) */
                scWhereClause = e_sc_ok
            ) &
            (not(p_nbWhereElts = 0 or
                 p_nbWhereElts = 1 & a_whereClauseElementValid = TRUE) =>
                scWhereClause : {e_sc_bad_monitored_item_filter_invalid,
                                 e_sc_bad_monitored_item_filter_unsupported}
            )
        )
    END
    ;

    notifTriggered <-- evaluate_where_clause (p_filterCtx, p_nbWhereElts, p_event) =
    PRE
        p_filterCtx : t_monitoringFilterCtx_i &
        p_filterCtx : t_monitoringFilterCtx &
        c_filterCtx = p_filterCtx &
        p_nbWhereElts : 0..1 &
        p_nbWhereElts = a_filterCtx_NbWhereClauseElements &
        p_event : t_Event_i &
        p_event : t_Event &
        c_event = p_event &
        a_eventTypeId : t_NodeId &
        (a_filterCtx_NbWhereClauseElements = 1 => a_whereClauseElementOfTypeId : t_NodeId) &
        (a_filterCtx_NbWhereClauseElements = 0 => a_whereClauseElementOfTypeId = c_NodeId_indet)
    THEN
        notifTriggered
        :(
            notifTriggered : BOOL &
            notifTriggered =
            bool(
                /* No element or  at least one where element type is compatible with event type */
                (p_nbWhereElts = 0 or a_whereClauseElementOfTypeId = a_eventTypeId
                    or a_eventTypeId |-> a_whereClauseElementOfTypeId : Type_IsTransitiveSubtype)
                )
        )
    END

END
