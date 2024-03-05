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

 File Name            : monitored_item_filter_treatment.c

 Date                 : 08/04/2024 12:58:41

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "monitored_item_filter_treatment.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void monitored_item_filter_treatment__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void monitored_item_filter_treatment__check_monitored_item_filter_valid_and_fill_result(
   const constants__t_endpoint_config_idx_i monitored_item_filter_treatment__p_endpoint_idx,
   const constants__t_NodeId_i monitored_item_filter_treatment__p_nid,
   const constants__t_AttributeId_i monitored_item_filter_treatment__p_aid,
   const constants__t_monitoringFilter_i monitored_item_filter_treatment__p_filter,
   const constants__t_Variant_i monitored_item_filter_treatment__p_value,
   constants_statuscodes_bs__t_StatusCode_i * const monitored_item_filter_treatment__statusCode,
   constants__t_monitoringFilterCtx_i * const monitored_item_filter_treatment__filterCtx,
   constants__t_filterResult_i * const monitored_item_filter_treatment__filterResult,
   t_bool * const monitored_item_filter_treatment__isEvent) {
   {
      t_bool monitored_item_filter_treatment__l_bres_presentNode;
      constants__t_Node_i monitored_item_filter_treatment__l_node;
      
      *monitored_item_filter_treatment__filterCtx = constants__c_monitoringFilterCtx_indet;
      *monitored_item_filter_treatment__filterResult = constants__c_filterResult_indet;
      *monitored_item_filter_treatment__isEvent = false;
      if (monitored_item_filter_treatment__p_filter != constants__c_monitoringFilter_indet) {
         address_space_itf__readall_AddressSpace_Node(monitored_item_filter_treatment__p_nid,
            &monitored_item_filter_treatment__l_bres_presentNode,
            &monitored_item_filter_treatment__l_node);
         if (monitored_item_filter_treatment__l_bres_presentNode == true) {
            if (monitored_item_filter_treatment__p_aid != constants__e_aid_EventNotifier) {
               monitored_item_data_filter_treatment_bs__check_monitored_item_data_filter_valid(monitored_item_filter_treatment__l_node,
                  monitored_item_filter_treatment__p_filter,
                  monitored_item_filter_treatment__statusCode,
                  monitored_item_filter_treatment__filterCtx);
            }
            else {
               monitored_item_event_filter_treatment__check_monitored_item_event_filter_valid(monitored_item_filter_treatment__p_endpoint_idx,
                  monitored_item_filter_treatment__p_aid,
                  monitored_item_filter_treatment__p_filter,
                  monitored_item_filter_treatment__p_value,
                  monitored_item_filter_treatment__statusCode,
                  monitored_item_filter_treatment__filterCtx,
                  monitored_item_filter_treatment__filterResult);
               *monitored_item_filter_treatment__isEvent = true;
            }
         }
         else {
            *monitored_item_filter_treatment__statusCode = constants_statuscodes_bs__e_sc_bad_filter_not_allowed;
         }
      }
      else if (monitored_item_filter_treatment__p_aid == constants__e_aid_EventNotifier) {
         *monitored_item_filter_treatment__statusCode = constants_statuscodes_bs__e_sc_bad_monitored_item_filter_invalid;
      }
      else {
         *monitored_item_filter_treatment__statusCode = constants_statuscodes_bs__e_sc_ok;
      }
   }
}

