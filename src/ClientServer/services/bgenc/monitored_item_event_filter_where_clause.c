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

 File Name            : monitored_item_event_filter_where_clause.c

 Date                 : 10/04/2024 15:55:21

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "monitored_item_event_filter_where_clause.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void monitored_item_event_filter_where_clause__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void monitored_item_event_filter_where_clause__check_where_clause_and_fill_ctx_and_result(
   const constants__t_monitoringFilterCtx_i monitored_item_event_filter_where_clause__p_filterCtx,
   const constants__t_filterResult_i monitored_item_event_filter_where_clause__p_filterResult,
   const t_entier4 monitored_item_event_filter_where_clause__p_nbWhereElts,
   constants_statuscodes_bs__t_StatusCode_i * const monitored_item_event_filter_where_clause__scWhereClause) {
   {
      t_bool monitored_item_event_filter_where_clause__l_continue;
      constants_statuscodes_bs__t_StatusCode_i monitored_item_event_filter_where_clause__l_whereStatusCode;
      constants__t_RawStatusCode monitored_item_event_filter_where_clause__l_whereOperatorResult;
      constants__t_RawStatusCode monitored_item_event_filter_where_clause__l_whereOperandResult;
      t_entier4 monitored_item_event_filter_where_clause__l_whereEltIdx;
      
      *monitored_item_event_filter_where_clause__scWhereClause = constants_statuscodes_bs__e_sc_ok;
      monitored_item_event_filter_where_clause_bs__init_where_elt_results(monitored_item_event_filter_where_clause__p_filterResult);
      monitored_item_event_filter_where_it__init_iter_eventFilterWhereElt(monitored_item_event_filter_where_clause__p_nbWhereElts,
         &monitored_item_event_filter_where_clause__l_continue);
      if (monitored_item_event_filter_where_clause__l_continue == true) {
         while (monitored_item_event_filter_where_clause__l_continue == true) {
            monitored_item_event_filter_where_it__continue_iter_eventFilterWhereEltIdx(&monitored_item_event_filter_where_clause__l_continue,
               &monitored_item_event_filter_where_clause__l_whereEltIdx);
            monitored_item_event_filter_treatment_bs__check_where_elt_and_fill_ctx(monitored_item_event_filter_where_clause__l_whereEltIdx,
               monitored_item_event_filter_where_clause__p_filterCtx,
               &monitored_item_event_filter_where_clause__l_whereStatusCode,
               &monitored_item_event_filter_where_clause__l_whereOperatorResult,
               &monitored_item_event_filter_where_clause__l_whereOperandResult);
            monitored_item_event_filter_where_clause_bs__set_where_element_result(monitored_item_event_filter_where_clause__p_filterResult,
               monitored_item_event_filter_where_clause__l_whereEltIdx,
               monitored_item_event_filter_where_clause__l_whereOperatorResult,
               monitored_item_event_filter_where_clause__l_whereOperandResult);
            if ((monitored_item_event_filter_where_clause__l_whereStatusCode != constants_statuscodes_bs__e_sc_ok) &&
               (*monitored_item_event_filter_where_clause__scWhereClause == constants_statuscodes_bs__e_sc_ok)) {
               *monitored_item_event_filter_where_clause__scWhereClause = monitored_item_event_filter_where_clause__l_whereStatusCode;
            }
         }
      }
   }
}

void monitored_item_event_filter_where_clause__evaluate_where_clause(
   const constants__t_monitoringFilterCtx_i monitored_item_event_filter_where_clause__p_filterCtx,
   const t_entier4 monitored_item_event_filter_where_clause__p_nbWhereElts,
   const constants__t_Event_i monitored_item_event_filter_where_clause__p_event,
   t_bool * const monitored_item_event_filter_where_clause__notifTriggered) {
   {
      constants__t_NodeId_i monitored_item_event_filter_where_clause__l_eventTypeId;
      constants__t_NodeId_i monitored_item_event_filter_where_clause__l_whereOfTypeId;
      t_bool monitored_item_event_filter_where_clause__l_nodeIdsEqual;
      t_bool monitored_item_event_filter_where_clause__l_isEventTypeRetained;
      
      monitored_item_event_filter_where_clause__l_isEventTypeRetained = (monitored_item_event_filter_where_clause__p_nbWhereElts == 0);
      if (monitored_item_event_filter_where_clause__l_isEventTypeRetained == false) {
         monitored_item_event_filter_treatment_bs__get_event_type_id(monitored_item_event_filter_where_clause__p_event,
            &monitored_item_event_filter_where_clause__l_eventTypeId);
         monitored_item_event_filter_treatment_bs__get_where_elt_of_type_id(monitored_item_event_filter_where_clause__p_filterCtx,
            &monitored_item_event_filter_where_clause__l_whereOfTypeId);
         address_space_itf__is_NodeId_equal(monitored_item_event_filter_where_clause__l_eventTypeId,
            monitored_item_event_filter_where_clause__l_whereOfTypeId,
            &monitored_item_event_filter_where_clause__l_nodeIdsEqual);
         if (monitored_item_event_filter_where_clause__l_nodeIdsEqual == true) {
            monitored_item_event_filter_where_clause__l_isEventTypeRetained = true;
         }
         else {
            address_space_itf__is_transitive_subtype(monitored_item_event_filter_where_clause__l_eventTypeId,
               monitored_item_event_filter_where_clause__l_whereOfTypeId,
               &monitored_item_event_filter_where_clause__l_isEventTypeRetained);
         }
      }
      *monitored_item_event_filter_where_clause__notifTriggered = monitored_item_event_filter_where_clause__l_isEventTypeRetained;
   }
}

