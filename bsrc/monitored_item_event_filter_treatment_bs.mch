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
    monitored_item_event_filter_treatment_bs

SEES
    constants,
    constants_statuscodes_bs

ABSTRACT_VARIABLES
    c_eventsSupport,

    s_isEventNotifier,

    c_filter,
    a_isEventFilter,

    a_filter_NbSelectClauses,
    a_filter_NbWhereClauseElements,

    a_eventSelectClauseValid,
    a_whereClauseElementValid,

    c_filterCtx,
    a_isEventFilterCtx,

    a_filterCtx_NbSelectClauses,
    a_filterCtx_NbWhereClauseElements,
    a_selectClauseTypeId,
    a_whereClauseElementOfTypeId,

    c_event,
    a_eventTypeId

INVARIANT
    c_eventsSupport : BOOL &

    s_isEventNotifier <: t_Variant_i &

    c_filter : t_monitoringFilter_i &
    a_isEventFilter : BOOL &

    a_filter_NbSelectClauses : NAT &
    a_filter_NbWhereClauseElements : NAT &

    a_eventSelectClauseValid : 1..a_filter_NbSelectClauses --> BOOL &
    a_whereClauseElementValid : BOOL &

    c_filterCtx : t_monitoringFilterCtx_i &
    a_isEventFilterCtx : BOOL &

    a_filterCtx_NbSelectClauses : NAT &
    a_filterCtx_NbWhereClauseElements : 0..1 &
    a_selectClauseTypeId : NAT --> t_NodeId &
    a_whereClauseElementOfTypeId : t_NodeId_i &

    c_event : t_Event_i &
    a_eventTypeId : t_NodeId_i

INITIALISATION
    c_eventsSupport,
    s_isEventNotifier,

    c_filter,
    a_isEventFilter,

    a_filter_NbSelectClauses,
    a_filter_NbWhereClauseElements,

    a_eventSelectClauseValid,
    a_whereClauseElementValid,

    c_filterCtx,
    a_isEventFilterCtx,

    a_filterCtx_NbSelectClauses,
    a_filterCtx_NbWhereClauseElements,
    a_selectClauseTypeId,
    a_whereClauseElementOfTypeId,

    c_event,
    a_eventTypeId

    :(c_eventsSupport : BOOL &
      s_isEventNotifier <: t_Variant_i &

      c_filter : t_monitoringFilter_i &
      c_filter = c_monitoringFilter_indet &
      a_isEventFilter : BOOL &
      a_isEventFilter = FALSE &

      a_filter_NbSelectClauses : NAT &
      a_filter_NbSelectClauses = 0 &
      a_filter_NbWhereClauseElements : NAT &
      a_filter_NbWhereClauseElements = 0 &

      a_eventSelectClauseValid = {} &
      a_whereClauseElementValid = FALSE &

      c_filterCtx : t_monitoringFilterCtx_i &
      c_filterCtx = c_monitoringFilterCtx_indet &
      a_isEventFilterCtx : BOOL &
      a_isEventFilterCtx = FALSE &

      a_filterCtx_NbSelectClauses = 0 &
      a_filterCtx_NbWhereClauseElements = 0 &
      a_whereClauseElementOfTypeId = c_NodeId_indet &
      a_selectClauseTypeId : NAT --> t_NodeId &
      a_selectClauseTypeId = {} &

      c_event : t_Event_i &
      c_event = c_Event_indet &

      a_eventTypeId : t_NodeId_i &
      a_eventTypeId = c_NodeId_indet

     )

OPERATIONS

    /* Create/Modify MI filter check operations */

    scEventsSupported <-- check_events_supported (p_endpoint_idx) =
    PRE
        p_endpoint_idx : t_endpoint_config_idx_i &
        p_endpoint_idx : t_endpoint_config_idx
    THEN
        scEventsSupported
        :(scEventsSupported : t_StatusCode_i &
          (c_eventsSupport = TRUE => scEventsSupported = e_sc_ok) &
          (not (c_eventsSupport = TRUE) => scEventsSupported = e_sc_bad_filter_not_allowed)
        )
    END
    ;

    scIsEventNotifier <-- check_is_event_notifier(p_value) =
    PRE
        p_value : t_Variant_i &
        p_value /= c_Variant_indet
    THEN
        IF p_value : s_isEventNotifier & p_value : t_Variant THEN
            scIsEventNotifier := e_sc_ok
        ELSE
            scIsEventNotifier := e_sc_bad_filter_not_allowed
        END
    END
    ;

    scIsEventFilter <-- init_and_check_is_event_filter(p_filter, p_aid) =
    PRE
        c_eventsSupport = TRUE &
        p_filter : t_monitoringFilter_i &
        p_filter : t_monitoringFilter &
        p_aid = e_aid_EventNotifier
    THEN
        scIsEventFilter,
        c_filter,
        a_isEventFilter
        :(scIsEventFilter : t_StatusCode_i &
          c_filter : t_monitoringFilter_i &
          a_isEventFilter : BOOL &
          (scIsEventFilter = e_sc_ok =>
              c_filter = p_filter &
              a_isEventFilter = TRUE
          ) &
          (not(scIsEventFilter = e_sc_ok) =>
              c_filter = c_monitoringFilter_indet &
              a_isEventFilter = FALSE &
              scIsEventFilter = e_sc_bad_monitored_item_filter_invalid
          )
        )
    END
    ;

    p_sc, p_filterCtx, p_filterResult, p_nbSelectClauses, p_nbWhereClausesElements <-- init_event_filter_ctx_and_result (p_filter) =
    PRE
        p_filter : t_monitoringFilter_i &
        p_filter : t_monitoringFilter &
        p_filter = c_filter
    THEN
        a_filter_NbSelectClauses, a_filter_NbWhereClauseElements, a_eventSelectClauseValid, a_whereClauseElementValid,
        p_sc, p_filterCtx, p_filterResult, p_nbSelectClauses, p_nbWhereClausesElements
        :(
            a_filter_NbSelectClauses : NAT &
            a_filter_NbWhereClauseElements : NAT &
            a_eventSelectClauseValid : 1..a_filter_NbSelectClauses --> BOOL &
            a_whereClauseElementValid : BOOL &
            p_sc : t_StatusCode_i &
            p_sc : {e_sc_ok, e_sc_bad_monitored_item_filter_invalid, e_sc_bad_out_of_memory} &
            p_filterCtx : t_monitoringFilterCtx_i &
            p_filterResult : t_filterResult_i &
            p_nbSelectClauses : NAT &
            p_nbSelectClauses = a_filter_NbSelectClauses &
            p_nbWhereClausesElements : NAT &
            p_nbWhereClausesElements = a_filter_NbWhereClauseElements &
            (p_sc = e_sc_ok =>
                p_filterCtx : t_monitoringFilterCtx &
                p_filterResult : t_filterResult &
                a_filter_NbSelectClauses > 0
            ) &
            (not (p_sc = e_sc_ok) =>
                p_filterCtx = c_monitoringFilterCtx_indet &
                p_filterResult = c_filterResult_indet &
                a_filter_NbSelectClauses = 0 &
                a_filter_NbWhereClauseElements = 0
            )
        )
    END
    ;

    delete_event_filter_context (p_filterCtx) =
    PRE
        p_filterCtx : t_monitoringFilterCtx_i
    THEN
        skip
    END;

    delete_event_filter_result (p_filterResult) =
    PRE
        p_filterResult : t_filterResult_i
    THEN
        skip
    END;

    statusCode, operatorRawSc, operandRawSc <-- check_where_elt_and_fill_ctx (p_whereEltIdx, p_filterCtx) =
    PRE
        c_filter : t_monitoringFilter &
        p_whereEltIdx : NAT &
        p_whereEltIdx : 1..a_filter_NbWhereClauseElements &
        p_filterCtx : t_monitoringFilterCtx_i &
        p_filterCtx : t_monitoringFilterCtx
    THEN
        statusCode, operatorRawSc, operandRawSc
        :(statusCode : t_StatusCode_i &
          operatorRawSc : t_RawStatusCode &
          operandRawSc : t_RawStatusCode &
            /* Only OfType operator is accepted and in this case only 1 operator is present */
            (p_whereEltIdx <= 1 & a_whereClauseElementValid = TRUE =>
                operatorRawSc = c_RawStatusCode_Good &
                operandRawSc = c_RawStatusCode_Good &
                statusCode = e_sc_ok) &
            /* OfType operator invalid or unsupported expression with several operators was present */
            (not (p_whereEltIdx <= 1 & a_whereClauseElementValid = TRUE) =>
                statusCode : {e_sc_bad_monitored_item_filter_invalid,
                              e_sc_bad_monitored_item_filter_unsupported})
        )
    END
    ;

    selectStatusCode, clauseRawSc <-- check_select_clause_and_fill_ctx (p_selectClauseIdx, p_filterCtx) =
    PRE
        c_filter : t_monitoringFilter &
        p_selectClauseIdx : NAT &
        p_selectClauseIdx : 1..a_filter_NbSelectClauses &
        p_filterCtx : t_monitoringFilterCtx_i &
        p_filterCtx : t_monitoringFilterCtx
    THEN
        selectStatusCode, clauseRawSc
        :(selectStatusCode : t_StatusCode_i &
          clauseRawSc : t_RawStatusCode &
            (a_eventSelectClauseValid(p_selectClauseIdx) = TRUE =>
                clauseRawSc = c_RawStatusCode_Good &
                selectStatusCode = e_sc_ok) &
            (a_eventSelectClauseValid(p_selectClauseIdx) = FALSE =>
                clauseRawSc : conv_StatusCode_To_RawStatusCode[{
                                    e_sc_bad_attribute_id_invalid,
                                    e_sc_bad_browse_name_invalid,
                                    e_sc_bad_type_mismatch,
                                    e_sc_bad_index_range_invalid,
                                    e_sc_bad_node_id_unknown,
                                    e_sc_bad_out_of_memory}] &
                selectStatusCode : {e_sc_bad_out_of_memory,
                                    e_sc_bad_monitored_item_filter_invalid,
                                    e_sc_bad_monitored_item_filter_unsupported})
        )
    END
    ;

    /* Event triggered filter check operations */
    nodeId <-- get_event_type_id (p_event) =
    PRE
        p_event : t_Event_i &
        p_event : t_Event &
        p_event = c_event
    THEN
        nodeId := a_eventTypeId
    END
    ;

    bres, nbSelectClauses, nbWhereClauseElements <-- init_and_check_filter_ctx (p_filterCtx, p_event) =
    PRE
        p_filterCtx : t_monitoringFilterCtx_i &
        p_event : t_Event_i &
        p_event : t_Event
    THEN
        c_filterCtx, a_isEventFilterCtx, a_filterCtx_NbSelectClauses,
        a_filterCtx_NbWhereClauseElements, a_selectClauseTypeId, a_whereClauseElementOfTypeId,
        c_event, a_eventTypeId,
        bres, nbSelectClauses, nbWhereClauseElements
        :(c_filterCtx : t_monitoringFilterCtx_i & a_isEventFilterCtx : BOOL &
          a_filterCtx_NbSelectClauses : NAT & a_filterCtx_NbWhereClauseElements : 0..1 &
          a_selectClauseTypeId : NAT --> t_NodeId &
          a_selectClauseTypeId : 1..a_filterCtx_NbSelectClauses --> t_NodeId &
          a_whereClauseElementOfTypeId : t_NodeId_i &
          c_event : t_Event_i & a_eventTypeId : t_NodeId_i &
          bres : BOOL & nbSelectClauses : NAT & nbWhereClauseElements : NAT &

          (bres = TRUE => p_filterCtx : t_monitoringFilterCtx & a_isEventFilterCtx = TRUE &
                          a_filterCtx_NbSelectClauses > 0 & c_filterCtx = p_filterCtx &
                          c_event = p_event & a_eventTypeId : t_NodeId &
                          nbSelectClauses = a_filterCtx_NbSelectClauses &
                          nbWhereClauseElements = a_filterCtx_NbWhereClauseElements &
                          (a_filterCtx_NbWhereClauseElements = 1 => a_whereClauseElementOfTypeId : t_NodeId) &
                          (a_filterCtx_NbWhereClauseElements = 0 => a_whereClauseElementOfTypeId = c_NodeId_indet)
          ) &
          (bres = FALSE => c_filterCtx = c_monitoringFilterCtx_indet & a_isEventFilterCtx = FALSE &
                           a_filterCtx_NbSelectClauses = 0 & a_filterCtx_NbWhereClauseElements = 0 &
                           c_event = c_Event_indet & a_eventTypeId = c_NodeId_indet &
                           c_filterCtx = p_filterCtx)
        )
    END
    ;

    nodeId <-- get_where_elt_of_type_id (p_filterCtx) =
    PRE
        a_whereClauseElementOfTypeId : t_NodeId &
        p_filterCtx : t_monitoringFilterCtx_i &
        p_filterCtx : t_monitoringFilterCtx &
        p_filterCtx = c_filterCtx
    THEN
        nodeId :(nodeId : t_NodeId_i & nodeId = a_whereClauseElementOfTypeId)
    END
    ;

    nodeId <-- get_select_clause_type_id (p_selectClauseIdx, p_filterCtx) =
    PRE
        p_selectClauseIdx : NAT &
        p_selectClauseIdx : 1..a_filterCtx_NbSelectClauses &
        p_filterCtx : t_monitoringFilterCtx_i &
        p_filterCtx : t_monitoringFilterCtx &
        p_filterCtx = c_filterCtx
    THEN
        nodeId :(nodeId : t_NodeId_i & nodeId = a_selectClauseTypeId(p_selectClauseIdx))
    END

END
