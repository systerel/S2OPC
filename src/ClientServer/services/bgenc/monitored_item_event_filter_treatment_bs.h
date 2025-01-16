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

/******************************************************************************

 File Name            : monitored_item_event_filter_treatment_bs.h

 Date                 : 30/01/2025 17:22:05

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _monitored_item_event_filter_treatment_bs_h
#define _monitored_item_event_filter_treatment_bs_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"
#include "constants_statuscodes_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void monitored_item_event_filter_treatment_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void monitored_item_event_filter_treatment_bs__check_events_supported(
   const constants__t_endpoint_config_idx_i monitored_item_event_filter_treatment_bs__p_endpoint_idx,
   constants_statuscodes_bs__t_StatusCode_i * const monitored_item_event_filter_treatment_bs__scEventsSupported);
extern void monitored_item_event_filter_treatment_bs__check_is_event_notifier(
   const constants__t_Variant_i monitored_item_event_filter_treatment_bs__p_value,
   constants_statuscodes_bs__t_StatusCode_i * const monitored_item_event_filter_treatment_bs__scIsEventNotifier);
extern void monitored_item_event_filter_treatment_bs__check_select_clause_and_fill_ctx(
   const t_entier4 monitored_item_event_filter_treatment_bs__p_selectClauseIdx,
   const constants__t_monitoringFilterCtx_i monitored_item_event_filter_treatment_bs__p_filterCtx,
   constants_statuscodes_bs__t_StatusCode_i * const monitored_item_event_filter_treatment_bs__selectStatusCode,
   constants__t_RawStatusCode * const monitored_item_event_filter_treatment_bs__clauseRawSc);
extern void monitored_item_event_filter_treatment_bs__check_where_elt_and_fill_ctx(
   const t_entier4 monitored_item_event_filter_treatment_bs__p_whereEltIdx,
   const constants__t_monitoringFilterCtx_i monitored_item_event_filter_treatment_bs__p_filterCtx,
   constants_statuscodes_bs__t_StatusCode_i * const monitored_item_event_filter_treatment_bs__statusCode,
   constants__t_RawStatusCode * const monitored_item_event_filter_treatment_bs__operatorRawSc,
   constants__t_RawStatusCode * const monitored_item_event_filter_treatment_bs__operandRawSc);
extern void monitored_item_event_filter_treatment_bs__delete_event_filter_context(
   const constants__t_monitoringFilterCtx_i monitored_item_event_filter_treatment_bs__p_filterCtx);
extern void monitored_item_event_filter_treatment_bs__delete_event_filter_result(
   const constants__t_filterResult_i monitored_item_event_filter_treatment_bs__p_filterResult);
extern void monitored_item_event_filter_treatment_bs__event_check_filter_ctx(
   const constants__t_monitoringFilterCtx_i monitored_item_event_filter_treatment_bs__p_filterCtx,
   const constants__t_Event_i monitored_item_event_filter_treatment_bs__p_event,
   t_bool * const monitored_item_event_filter_treatment_bs__bres,
   t_entier4 * const monitored_item_event_filter_treatment_bs__nbSelectClauses,
   t_entier4 * const monitored_item_event_filter_treatment_bs__nbWhereClauseElements);
extern void monitored_item_event_filter_treatment_bs__get_event_source_node(
   const constants__t_Event_i monitored_item_event_filter_treatment_bs__p_event,
   constants__t_NodeId_i * const monitored_item_event_filter_treatment_bs__p_nodeId);
extern void monitored_item_event_filter_treatment_bs__get_event_type_id(
   const constants__t_Event_i monitored_item_event_filter_treatment_bs__p_event,
   constants__t_NodeId_i * const monitored_item_event_filter_treatment_bs__nodeId);
extern void monitored_item_event_filter_treatment_bs__get_select_clause_type_id(
   const t_entier4 monitored_item_event_filter_treatment_bs__p_selectClauseIdx,
   const constants__t_monitoringFilterCtx_i monitored_item_event_filter_treatment_bs__p_filterCtx,
   constants__t_NodeId_i * const monitored_item_event_filter_treatment_bs__nodeId);
extern void monitored_item_event_filter_treatment_bs__get_where_elt_of_type_id(
   const constants__t_monitoringFilterCtx_i monitored_item_event_filter_treatment_bs__p_filterCtx,
   constants__t_NodeId_i * const monitored_item_event_filter_treatment_bs__nodeId);
extern void monitored_item_event_filter_treatment_bs__init_and_check_is_event_filter(
   const constants__t_monitoringFilter_i monitored_item_event_filter_treatment_bs__p_filter,
   const constants__t_AttributeId_i monitored_item_event_filter_treatment_bs__p_aid,
   constants_statuscodes_bs__t_StatusCode_i * const monitored_item_event_filter_treatment_bs__scIsEventFilter);
extern void monitored_item_event_filter_treatment_bs__init_event(
   const constants__t_Event_i monitored_item_event_filter_treatment_bs__p_event);
extern void monitored_item_event_filter_treatment_bs__init_event_filter_ctx_and_result(
   const constants__t_monitoringFilter_i monitored_item_event_filter_treatment_bs__p_filter,
   constants_statuscodes_bs__t_StatusCode_i * const monitored_item_event_filter_treatment_bs__p_sc,
   constants__t_monitoringFilterCtx_i * const monitored_item_event_filter_treatment_bs__p_filterCtx,
   constants__t_filterResult_i * const monitored_item_event_filter_treatment_bs__p_filterResult,
   t_entier4 * const monitored_item_event_filter_treatment_bs__p_nbSelectClauses,
   t_entier4 * const monitored_item_event_filter_treatment_bs__p_nbWhereClausesElements);

#endif
