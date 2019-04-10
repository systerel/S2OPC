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

 File Name            : service_set_view.c

 Date                 : 10/04/2019 17:16:12

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "service_set_view.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void service_set_view__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void service_set_view__treat_browse_request_BrowseValue_1(
   const constants__t_session_i service_set_view__p_session,
   const constants__t_msg_i service_set_view__p_req_msg,
   const constants__t_msg_i service_set_view__p_resp_msg,
   const constants__t_NodeId_i service_set_view__p_nid_view,
   const t_entier4 service_set_view__p_nb_target_max,
   const constants__t_BrowseValue_i service_set_view__p_bvi) {
   {
      constants__t_NodeId_i service_set_view__l_SrcNodeId;
      constants__t_BrowseDirection_i service_set_view__l_dir;
      constants__t_NodeId_i service_set_view__l_reftype;
      t_bool service_set_view__l_incsubtyp;
      constants_statuscodes_bs__t_StatusCode_i service_set_view__l_serviceStatusCode;
      constants__t_ContinuationPoint_i service_set_view__l_continuationPoint;
      t_entier4 service_set_view__l_nbTargets;
      constants__t_BrowseResultReferences_i service_set_view__l_browseResult;
      constants__t_BrowseNodeClassMask_i service_set_view__l_nodeClassMask;
      
      msg_browse_bs__getall_BrowseValue(service_set_view__p_req_msg,
         service_set_view__p_bvi,
         &service_set_view__l_SrcNodeId,
         &service_set_view__l_dir,
         &service_set_view__l_reftype,
         &service_set_view__l_incsubtyp,
         &service_set_view__l_nodeClassMask);
      if (service_set_view__l_dir != constants__e_bd_indet) {
         if (service_set_view__l_SrcNodeId != constants__c_NodeId_indet) {
            browse_treatment__set_browse_value_context(service_set_view__p_session,
               service_set_view__p_nb_target_max,
               service_set_view__p_nid_view,
               service_set_view__l_SrcNodeId,
               service_set_view__l_dir,
               service_set_view__l_reftype,
               service_set_view__l_incsubtyp,
               service_set_view__l_nodeClassMask);
            browse_treatment__compute_browse_result(&service_set_view__l_serviceStatusCode,
               &service_set_view__l_continuationPoint,
               &service_set_view__l_nbTargets);
            browse_treatment__clear_browse_value_context();
            msg_browse_bs__set_ResponseBrowse_BrowseStatus(service_set_view__p_resp_msg,
               service_set_view__p_bvi,
               service_set_view__l_serviceStatusCode);
            if ((service_set_view__l_serviceStatusCode == constants_statuscodes_bs__e_sc_ok) ||
               (service_set_view__l_serviceStatusCode == constants_statuscodes_bs__e_sc_bad_no_continuation_points)) {
               msg_browse_bs__set_ResponseBrowse_ContinuationPoint(service_set_view__p_resp_msg,
                  service_set_view__p_bvi,
                  service_set_view__l_continuationPoint);
               browse_treatment__getall_and_clear_browse_result(&service_set_view__l_nbTargets,
                  &service_set_view__l_browseResult);
               msg_browse_bs__set_ResponseBrowse_BrowseResult(service_set_view__p_resp_msg,
                  service_set_view__p_bvi,
                  service_set_view__l_nbTargets,
                  service_set_view__l_browseResult);
            }
         }
         else {
            msg_browse_bs__set_ResponseBrowse_BrowseStatus(service_set_view__p_resp_msg,
               service_set_view__p_bvi,
               constants_statuscodes_bs__e_sc_bad_node_id_invalid);
         }
      }
      else {
         msg_browse_bs__set_ResponseBrowse_BrowseStatus(service_set_view__p_resp_msg,
            service_set_view__p_bvi,
            constants_statuscodes_bs__e_sc_bad_browse_direction_invalid);
      }
   }
}

void service_set_view__treat_browse_request_BrowseValues(
   const constants__t_session_i service_set_view__p_session,
   const constants__t_msg_i service_set_view__p_req_msg,
   const constants__t_msg_i service_set_view__p_resp_msg,
   const constants__t_NodeId_i service_set_view__p_nid_view,
   const t_entier4 service_set_view__p_nb_target_max,
   const t_entier4 service_set_view__p_nb_browse_value,
   constants_statuscodes_bs__t_StatusCode_i * const service_set_view__StatusCode_service) {
   {
      t_bool service_set_view__l_continue;
      constants__t_BrowseValue_i service_set_view__l_bvi;
      t_bool service_set_view__l_isallocated;
      
      if (service_set_view__p_nid_view == constants__c_NodeId_indet) {
         if ((service_set_view__p_nb_browse_value > 0) &&
            (service_set_view__p_nb_browse_value <= constants__k_n_BrowseResponse_max)) {
            msg_browse_bs__alloc_browse_response(service_set_view__p_resp_msg,
               service_set_view__p_nb_browse_value,
               &service_set_view__l_isallocated);
            if (service_set_view__l_isallocated == true) {
               *service_set_view__StatusCode_service = constants_statuscodes_bs__e_sc_ok;
               service_browse_it__init_iter_browse_request(service_set_view__p_nb_browse_value,
                  &service_set_view__l_continue);
               while (service_set_view__l_continue == true) {
                  service_browse_it__continue_iter_browse_request(&service_set_view__l_continue,
                     &service_set_view__l_bvi);
                  service_set_view__treat_browse_request_BrowseValue_1(service_set_view__p_session,
                     service_set_view__p_req_msg,
                     service_set_view__p_resp_msg,
                     service_set_view__p_nid_view,
                     service_set_view__p_nb_target_max,
                     service_set_view__l_bvi);
               }
            }
            else {
               *service_set_view__StatusCode_service = constants_statuscodes_bs__e_sc_bad_out_of_memory;
            }
         }
         else {
            if (service_set_view__p_nb_browse_value == 0) {
               *service_set_view__StatusCode_service = constants_statuscodes_bs__e_sc_bad_nothing_to_do;
            }
            else {
               *service_set_view__StatusCode_service = constants_statuscodes_bs__e_sc_bad_too_many_ops;
            }
         }
      }
      else {
         *service_set_view__StatusCode_service = constants_statuscodes_bs__e_sc_bad_view_id_unknown;
      }
   }
}

void service_set_view__treat_browse_request(
   const constants__t_session_i service_set_view__p_session,
   const constants__t_msg_i service_set_view__p_req_msg,
   const constants__t_msg_i service_set_view__p_resp_msg,
   constants_statuscodes_bs__t_StatusCode_i * const service_set_view__StatusCode_service) {
   {
      constants__t_NodeId_i service_set_view__l_nid_view;
      t_entier4 service_set_view__l_nb_BrowseTargetMax;
      t_entier4 service_set_view__l_nb_BrowseValues;
      
      msg_browse_bs__get_browse_request_params(service_set_view__p_req_msg,
         &service_set_view__l_nid_view,
         &service_set_view__l_nb_BrowseTargetMax,
         &service_set_view__l_nb_BrowseValues);
      service_set_view__treat_browse_request_BrowseValues(service_set_view__p_session,
         service_set_view__p_req_msg,
         service_set_view__p_resp_msg,
         service_set_view__l_nid_view,
         service_set_view__l_nb_BrowseTargetMax,
         service_set_view__l_nb_BrowseValues,
         service_set_view__StatusCode_service);
   }
}

void service_set_view__service_set_view_set_session_closed(
   const constants__t_session_i service_set_view__p_session) {
   browse_treatment__set_session_closed(service_set_view__p_session);
}

void service_set_view__service_set_view_service_node_management_used(void) {
   browse_treatment__continuation_points_UNINITIALISATION();
}

void service_set_view__service_set_view_UNINITIALISATION(void) {
   browse_treatment__continuation_points_UNINITIALISATION();
}

