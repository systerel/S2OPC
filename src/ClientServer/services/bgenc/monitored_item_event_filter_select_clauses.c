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

 File Name            : monitored_item_event_filter_select_clauses.c

 Date                 : 03/05/2024 09:44:47

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "monitored_item_event_filter_select_clauses.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void monitored_item_event_filter_select_clauses__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void monitored_item_event_filter_select_clauses__check_select_clauses_and_fill_ctx_and_result(
   const constants__t_monitoringFilterCtx_i monitored_item_event_filter_select_clauses__p_filterCtx,
   const constants__t_filterResult_i monitored_item_event_filter_select_clauses__p_filterResult,
   const t_entier4 monitored_item_event_filter_select_clauses__p_nbSelectClauses,
   constants_statuscodes_bs__t_StatusCode_i * const monitored_item_event_filter_select_clauses__scSelectClauses,
   t_bool * const monitored_item_event_filter_select_clauses__hasInvalidClause) {
   {
      t_bool monitored_item_event_filter_select_clauses__l_continue;
      constants_statuscodes_bs__t_StatusCode_i monitored_item_event_filter_select_clauses__l_selectStatusCode;
      constants__t_RawStatusCode monitored_item_event_filter_select_clauses__l_clauseResult;
      t_entier4 monitored_item_event_filter_select_clauses__l_selectClauseIdx;
      t_bool monitored_item_event_filter_select_clauses__l_oneValidSelectClause;
      
      monitored_item_event_filter_select_clauses__l_oneValidSelectClause = false;
      *monitored_item_event_filter_select_clauses__hasInvalidClause = false;
      *monitored_item_event_filter_select_clauses__scSelectClauses = constants_statuscodes_bs__e_sc_bad_monitored_item_filter_invalid;
      monitored_item_event_filter_select_clauses_bs__init_select_clause_results(monitored_item_event_filter_select_clauses__p_filterResult);
      monitored_item_event_filter_select_it__init_iter_eventFilterSelectClause(monitored_item_event_filter_select_clauses__p_nbSelectClauses,
         &monitored_item_event_filter_select_clauses__l_continue);
      while (monitored_item_event_filter_select_clauses__l_continue == true) {
         monitored_item_event_filter_select_it__continue_iter_eventFilterSelectClauseIdx(&monitored_item_event_filter_select_clauses__l_continue,
            &monitored_item_event_filter_select_clauses__l_selectClauseIdx);
         monitored_item_event_filter_treatment_bs__check_select_clause_and_fill_ctx(monitored_item_event_filter_select_clauses__l_selectClauseIdx,
            monitored_item_event_filter_select_clauses__p_filterCtx,
            &monitored_item_event_filter_select_clauses__l_selectStatusCode,
            &monitored_item_event_filter_select_clauses__l_clauseResult);
         monitored_item_event_filter_select_clauses_bs__set_select_clause_result(monitored_item_event_filter_select_clauses__p_filterResult,
            monitored_item_event_filter_select_clauses__l_selectClauseIdx,
            monitored_item_event_filter_select_clauses__l_clauseResult);
         if (monitored_item_event_filter_select_clauses__l_selectStatusCode == constants_statuscodes_bs__e_sc_ok) {
            monitored_item_event_filter_select_clauses__l_oneValidSelectClause = true;
         }
         else {
            *monitored_item_event_filter_select_clauses__scSelectClauses = monitored_item_event_filter_select_clauses__l_selectStatusCode;
            *monitored_item_event_filter_select_clauses__hasInvalidClause = true;
         }
      }
      if (monitored_item_event_filter_select_clauses__l_oneValidSelectClause == true) {
         *monitored_item_event_filter_select_clauses__scSelectClauses = constants_statuscodes_bs__e_sc_ok;
      }
   }
}

void monitored_item_event_filter_select_clauses__apply_select_clauses_and_build_event_field_list(
   const constants__t_client_handle_i monitored_item_event_filter_select_clauses__p_clientHandle,
   const constants__t_LocaleIds_i monitored_item_event_filter_select_clauses__p_localeIds,
   const constants__t_TimestampsToReturn_i monitored_item_event_filter_select_clauses__p_timestampToReturn,
   const t_bool monitored_item_event_filter_select_clauses__p_userAccessGranted,
   const constants__t_monitoringFilterCtx_i monitored_item_event_filter_select_clauses__p_filterCtx,
   const t_entier4 monitored_item_event_filter_select_clauses__p_nbSelectClauses,
   const constants__t_Event_i monitored_item_event_filter_select_clauses__p_event,
   t_bool * const monitored_item_event_filter_select_clauses__notifTriggered,
   constants__t_eventFieldList_i * const monitored_item_event_filter_select_clauses__eventNotif) {
   {
      t_bool monitored_item_event_filter_select_clauses__l_continue;
      t_entier4 monitored_item_event_filter_select_clauses__l_selectClauseIdx;
      constants__t_NodeId_i monitored_item_event_filter_select_clauses__l_selectClauseTypeId;
      constants__t_NodeId_i monitored_item_event_filter_select_clauses__l_eventTypeId;
      t_bool monitored_item_event_filter_select_clauses__l_nodeIdsEqual;
      t_bool monitored_item_event_filter_select_clauses__l_isEventTypeRetained;
      
      monitored_item_event_filter_select_clauses_bs__alloc_event_field_list(monitored_item_event_filter_select_clauses__p_clientHandle,
         monitored_item_event_filter_select_clauses__p_nbSelectClauses,
         monitored_item_event_filter_select_clauses__notifTriggered,
         monitored_item_event_filter_select_clauses__eventNotif);
      monitored_item_event_filter_select_clauses__l_isEventTypeRetained = false;
      monitored_item_event_filter_treatment_bs__get_event_type_id(monitored_item_event_filter_select_clauses__p_event,
         &monitored_item_event_filter_select_clauses__l_eventTypeId);
      monitored_item_event_filter_select_it__init_iter_eventFilterSelectClause(monitored_item_event_filter_select_clauses__p_nbSelectClauses,
         &monitored_item_event_filter_select_clauses__l_continue);
      while ((*monitored_item_event_filter_select_clauses__notifTriggered == true) &&
         (monitored_item_event_filter_select_clauses__l_continue == true)) {
         monitored_item_event_filter_select_it__continue_iter_eventFilterSelectClauseIdx(&monitored_item_event_filter_select_clauses__l_continue,
            &monitored_item_event_filter_select_clauses__l_selectClauseIdx);
         monitored_item_event_filter_treatment_bs__get_select_clause_type_id(monitored_item_event_filter_select_clauses__l_selectClauseIdx,
            monitored_item_event_filter_select_clauses__p_filterCtx,
            &monitored_item_event_filter_select_clauses__l_selectClauseTypeId);
         address_space_itf__is_NodeId_equal(monitored_item_event_filter_select_clauses__l_selectClauseTypeId,
            constants__c_BaseEventType_NodeId,
            &monitored_item_event_filter_select_clauses__l_nodeIdsEqual);
         if (monitored_item_event_filter_select_clauses__l_nodeIdsEqual == false) {
            address_space_itf__is_NodeId_equal(monitored_item_event_filter_select_clauses__l_eventTypeId,
               monitored_item_event_filter_select_clauses__l_selectClauseTypeId,
               &monitored_item_event_filter_select_clauses__l_nodeIdsEqual);
         }
         if (monitored_item_event_filter_select_clauses__l_nodeIdsEqual == true) {
            monitored_item_event_filter_select_clauses__l_isEventTypeRetained = true;
         }
         else {
            address_space_itf__is_transitive_subtype(monitored_item_event_filter_select_clauses__l_eventTypeId,
               monitored_item_event_filter_select_clauses__l_selectClauseTypeId,
               &monitored_item_event_filter_select_clauses__l_isEventTypeRetained);
         }
         if (monitored_item_event_filter_select_clauses__l_isEventTypeRetained == true) {
            monitored_item_event_filter_select_clauses_bs__set_event_field_list_elt(monitored_item_event_filter_select_clauses__p_localeIds,
               monitored_item_event_filter_select_clauses__p_timestampToReturn,
               monitored_item_event_filter_select_clauses__p_userAccessGranted,
               monitored_item_event_filter_select_clauses__l_selectClauseIdx,
               monitored_item_event_filter_select_clauses__p_filterCtx,
               *monitored_item_event_filter_select_clauses__eventNotif,
               monitored_item_event_filter_select_clauses__p_event);
         }
         else {
            monitored_item_event_filter_select_clauses_bs__set_event_field_list_elt_null(monitored_item_event_filter_select_clauses__l_selectClauseIdx,
               *monitored_item_event_filter_select_clauses__eventNotif);
         }
      }
   }
}

