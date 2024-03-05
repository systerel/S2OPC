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

 File Name            : monitored_item_event_filter_treatment.c

 Date                 : 10/04/2024 09:43:25

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "monitored_item_event_filter_treatment.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void monitored_item_event_filter_treatment__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void monitored_item_event_filter_treatment__check_monitored_item_event_filter_valid(
   const constants__t_endpoint_config_idx_i monitored_item_event_filter_treatment__p_endpoint_idx,
   const constants__t_AttributeId_i monitored_item_event_filter_treatment__p_aid,
   const constants__t_monitoringFilter_i monitored_item_event_filter_treatment__p_filter,
   const constants__t_Variant_i monitored_item_event_filter_treatment__p_value,
   constants_statuscodes_bs__t_StatusCode_i * const monitored_item_event_filter_treatment__statusCode,
   constants__t_monitoringFilterCtx_i * const monitored_item_event_filter_treatment__filterCtx,
   constants__t_filterResult_i * const monitored_item_event_filter_treatment__filterResult) {
   {
      constants_statuscodes_bs__t_StatusCode_i monitored_item_event_filter_treatment__l_StatusCode;
      constants__t_monitoringFilterCtx_i monitored_item_event_filter_treatment__l_filterCtx;
      constants__t_filterResult_i monitored_item_event_filter_treatment__l_filterResult;
      t_entier4 monitored_item_event_filter_treatment__l_nbSelectClauses;
      t_entier4 monitored_item_event_filter_treatment__l_nbWhereElts;
      constants_statuscodes_bs__t_StatusCode_i monitored_item_event_filter_treatment__l_WhereClauseSc;
      constants_statuscodes_bs__t_StatusCode_i monitored_item_event_filter_treatment__l_scSelectClauses;
      t_bool monitored_item_event_filter_treatment__l_oneInvalidSelectClause;
      
      *monitored_item_event_filter_treatment__filterCtx = constants__c_monitoringFilterCtx_indet;
      *monitored_item_event_filter_treatment__filterResult = constants__c_filterResult_indet;
      monitored_item_event_filter_treatment__l_filterCtx = constants__c_monitoringFilterCtx_indet;
      monitored_item_event_filter_treatment__l_filterResult = constants__c_filterResult_indet;
      monitored_item_event_filter_treatment__l_nbSelectClauses = 0;
      monitored_item_event_filter_treatment__l_nbWhereElts = 0;
      monitored_item_event_filter_treatment__l_scSelectClauses = constants_statuscodes_bs__e_sc_ok;
      monitored_item_event_filter_treatment__l_oneInvalidSelectClause = false;
      monitored_item_event_filter_treatment__l_WhereClauseSc = constants_statuscodes_bs__e_sc_ok;
      monitored_item_event_filter_treatment__l_StatusCode = constants_statuscodes_bs__e_sc_ok;
      monitored_item_event_filter_treatment_bs__check_events_supported(monitored_item_event_filter_treatment__p_endpoint_idx,
         &monitored_item_event_filter_treatment__l_StatusCode);
      if ((monitored_item_event_filter_treatment__l_StatusCode == constants_statuscodes_bs__e_sc_ok) &&
         (monitored_item_event_filter_treatment__p_value != constants__c_Variant_indet)) {
         monitored_item_event_filter_treatment_bs__check_is_event_notifier(monitored_item_event_filter_treatment__p_value,
            &monitored_item_event_filter_treatment__l_StatusCode);
      }
      if (monitored_item_event_filter_treatment__l_StatusCode == constants_statuscodes_bs__e_sc_ok) {
         monitored_item_event_filter_treatment_bs__init_and_check_is_event_filter(monitored_item_event_filter_treatment__p_filter,
            monitored_item_event_filter_treatment__p_aid,
            &monitored_item_event_filter_treatment__l_StatusCode);
      }
      if (monitored_item_event_filter_treatment__l_StatusCode == constants_statuscodes_bs__e_sc_ok) {
         monitored_item_event_filter_treatment_bs__init_event_filter_ctx_and_result(monitored_item_event_filter_treatment__p_filter,
            &monitored_item_event_filter_treatment__l_StatusCode,
            &monitored_item_event_filter_treatment__l_filterCtx,
            &monitored_item_event_filter_treatment__l_filterResult,
            &monitored_item_event_filter_treatment__l_nbSelectClauses,
            &monitored_item_event_filter_treatment__l_nbWhereElts);
      }
      if (monitored_item_event_filter_treatment__l_StatusCode == constants_statuscodes_bs__e_sc_ok) {
         monitored_item_event_filter_where_clause__check_where_clause_and_fill_ctx_and_result(monitored_item_event_filter_treatment__l_filterCtx,
            monitored_item_event_filter_treatment__l_filterResult,
            monitored_item_event_filter_treatment__l_nbWhereElts,
            &monitored_item_event_filter_treatment__l_WhereClauseSc);
      }
      if (monitored_item_event_filter_treatment__l_StatusCode == constants_statuscodes_bs__e_sc_ok) {
         monitored_item_event_filter_select_clauses__check_select_clauses_and_fill_ctx_and_result(monitored_item_event_filter_treatment__l_filterCtx,
            monitored_item_event_filter_treatment__l_filterResult,
            monitored_item_event_filter_treatment__l_nbSelectClauses,
            &monitored_item_event_filter_treatment__l_scSelectClauses,
            &monitored_item_event_filter_treatment__l_oneInvalidSelectClause);
      }
      if (monitored_item_event_filter_treatment__l_StatusCode != constants_statuscodes_bs__e_sc_ok) {
         monitored_item_event_filter_treatment_bs__delete_event_filter_context(monitored_item_event_filter_treatment__l_filterCtx);
         monitored_item_event_filter_treatment__l_filterCtx = constants__c_monitoringFilterCtx_indet;
         monitored_item_event_filter_treatment_bs__delete_event_filter_result(monitored_item_event_filter_treatment__l_filterResult);
         monitored_item_event_filter_treatment__l_filterResult = constants__c_filterResult_indet;
      }
      if ((monitored_item_event_filter_treatment__l_StatusCode == constants_statuscodes_bs__e_sc_ok) &&
         (monitored_item_event_filter_treatment__l_WhereClauseSc != constants_statuscodes_bs__e_sc_ok)) {
         monitored_item_event_filter_treatment__l_StatusCode = monitored_item_event_filter_treatment__l_WhereClauseSc;
         monitored_item_event_filter_treatment_bs__delete_event_filter_context(monitored_item_event_filter_treatment__l_filterCtx);
         monitored_item_event_filter_treatment__l_filterCtx = constants__c_monitoringFilterCtx_indet;
      }
      if ((monitored_item_event_filter_treatment__l_StatusCode == constants_statuscodes_bs__e_sc_ok) &&
         (monitored_item_event_filter_treatment__l_scSelectClauses != constants_statuscodes_bs__e_sc_ok)) {
         monitored_item_event_filter_treatment__l_StatusCode = monitored_item_event_filter_treatment__l_scSelectClauses;
         monitored_item_event_filter_treatment_bs__delete_event_filter_context(monitored_item_event_filter_treatment__l_filterCtx);
         monitored_item_event_filter_treatment__l_filterCtx = constants__c_monitoringFilterCtx_indet;
      }
      if ((monitored_item_event_filter_treatment__l_StatusCode == constants_statuscodes_bs__e_sc_ok) &&
         (monitored_item_event_filter_treatment__l_oneInvalidSelectClause == false)) {
         monitored_item_event_filter_treatment_bs__delete_event_filter_result(monitored_item_event_filter_treatment__l_filterResult);
         monitored_item_event_filter_treatment__l_filterResult = constants__c_filterResult_indet;
      }
      *monitored_item_event_filter_treatment__statusCode = monitored_item_event_filter_treatment__l_StatusCode;
      *monitored_item_event_filter_treatment__filterResult = monitored_item_event_filter_treatment__l_filterResult;
      if (monitored_item_event_filter_treatment__l_StatusCode == constants_statuscodes_bs__e_sc_ok) {
         *monitored_item_event_filter_treatment__filterCtx = monitored_item_event_filter_treatment__l_filterCtx;
      }
   }
}

void monitored_item_event_filter_treatment__server_subscription_get_notification_on_event(
   const constants__t_client_handle_i monitored_item_event_filter_treatment__p_clientHandle,
   const constants__t_LocaleIds_i monitored_item_event_filter_treatment__p_localeIds,
   const constants__t_TimestampsToReturn_i monitored_item_event_filter_treatment__p_timestampToReturn,
   const t_bool monitored_item_event_filter_treatment__p_userAccessGranted,
   const constants__t_monitoringFilterCtx_i monitored_item_event_filter_treatment__p_filterCtx,
   const constants__t_Event_i monitored_item_event_filter_treatment__p_event,
   t_bool * const monitored_item_event_filter_treatment__notifTriggered,
   constants__t_eventFieldList_i * const monitored_item_event_filter_treatment__eventNotif) {
   {
      t_bool monitored_item_event_filter_treatment__l_bres;
      t_entier4 monitored_item_event_filter_treatment__l_nbSelectClauses;
      t_entier4 monitored_item_event_filter_treatment__l_nbWhereClauseElements;
      
      *monitored_item_event_filter_treatment__notifTriggered = false;
      *monitored_item_event_filter_treatment__eventNotif = constants__c_eventFieldList_indet;
      monitored_item_event_filter_treatment_bs__init_and_check_filter_ctx(monitored_item_event_filter_treatment__p_filterCtx,
         monitored_item_event_filter_treatment__p_event,
         &monitored_item_event_filter_treatment__l_bres,
         &monitored_item_event_filter_treatment__l_nbSelectClauses,
         &monitored_item_event_filter_treatment__l_nbWhereClauseElements);
      if (monitored_item_event_filter_treatment__l_bres == true) {
         monitored_item_event_filter_where_clause__evaluate_where_clause(monitored_item_event_filter_treatment__p_filterCtx,
            monitored_item_event_filter_treatment__l_nbWhereClauseElements,
            monitored_item_event_filter_treatment__p_event,
            &monitored_item_event_filter_treatment__l_bres);
      }
      if (monitored_item_event_filter_treatment__l_bres == true) {
         monitored_item_event_filter_select_clauses__apply_select_clauses_and_build_event_field_list(monitored_item_event_filter_treatment__p_clientHandle,
            monitored_item_event_filter_treatment__p_localeIds,
            monitored_item_event_filter_treatment__p_timestampToReturn,
            monitored_item_event_filter_treatment__p_userAccessGranted,
            monitored_item_event_filter_treatment__p_filterCtx,
            monitored_item_event_filter_treatment__l_nbSelectClauses,
            monitored_item_event_filter_treatment__p_event,
            monitored_item_event_filter_treatment__notifTriggered,
            monitored_item_event_filter_treatment__eventNotif);
      }
   }
}

