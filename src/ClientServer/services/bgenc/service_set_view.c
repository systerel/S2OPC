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

 Date                 : 04/08/2022 14:53:14

 C Translator Version : tradc Java V1.2 (06/02/2022)

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
   const constants__t_BrowseValue_i service_set_view__p_bvi,
   const t_bool service_set_view__p_autoReleaseCP) {
   {
      constants__t_NodeId_i service_set_view__l_SrcNodeId;
      constants__t_BrowseDirection_i service_set_view__l_dir;
      constants__t_NodeId_i service_set_view__l_reftype;
      t_bool service_set_view__l_incsubtyp;
      constants_statuscodes_bs__t_StatusCode_i service_set_view__l_serviceStatusCode;
      constants__t_ContinuationPointId_i service_set_view__l_continuationPointId;
      t_entier4 service_set_view__l_nbTargets;
      constants__t_BrowseResultReferences_i service_set_view__l_browseResult;
      constants__t_BrowseNodeClassMask_i service_set_view__l_nodeClassMask;
      constants__t_BrowseResultMask_i service_set_view__l_resultMask;
      
      msg_browse_bs__getall_BrowseValue(service_set_view__p_req_msg,
         service_set_view__p_bvi,
         &service_set_view__l_SrcNodeId,
         &service_set_view__l_dir,
         &service_set_view__l_reftype,
         &service_set_view__l_incsubtyp,
         &service_set_view__l_nodeClassMask,
         &service_set_view__l_resultMask);
      if (service_set_view__l_dir != constants__e_bd_indet) {
         if (service_set_view__l_SrcNodeId != constants__c_NodeId_indet) {
            translate_browse_path__set_browse_value_context(service_set_view__p_session,
               service_set_view__p_nb_target_max,
               service_set_view__p_nid_view,
               service_set_view__l_SrcNodeId,
               service_set_view__l_dir,
               service_set_view__l_reftype,
               service_set_view__l_incsubtyp,
               service_set_view__l_nodeClassMask,
               service_set_view__l_resultMask,
               service_set_view__p_autoReleaseCP,
               &service_set_view__l_serviceStatusCode);
            if (service_set_view__l_serviceStatusCode == constants_statuscodes_bs__e_sc_ok) {
               translate_browse_path__compute_browse_result(&service_set_view__l_serviceStatusCode,
                  &service_set_view__l_continuationPointId,
                  &service_set_view__l_nbTargets);
               translate_browse_path__clear_browse_value_context();
               if ((service_set_view__l_serviceStatusCode == constants_statuscodes_bs__e_sc_ok) ||
                  (service_set_view__l_serviceStatusCode == constants_statuscodes_bs__e_sc_bad_no_continuation_points)) {
                  translate_browse_path__getall_and_move_browse_result(&service_set_view__l_nbTargets,
                     &service_set_view__l_browseResult);
                  msg_browse_bs__set_ResponseBrowse_BrowseResult(service_set_view__p_resp_msg,
                     service_set_view__p_bvi,
                     service_set_view__l_nbTargets,
                     service_set_view__l_browseResult);
                  msg_browse_bs__set_ResponseBrowse_ContinuationPoint(service_set_view__p_resp_msg,
                     service_set_view__p_bvi,
                     service_set_view__l_continuationPointId);
               }
            }
            msg_browse_bs__set_ResponseBrowse_BrowseStatus(service_set_view__p_resp_msg,
               service_set_view__p_bvi,
               service_set_view__l_serviceStatusCode);
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
      t_bool service_set_view__l_first_iteration;
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
               service_set_view__l_first_iteration = true;
               while (service_set_view__l_continue == true) {
                  service_browse_it__continue_iter_browse_request(&service_set_view__l_continue,
                     &service_set_view__l_bvi);
                  service_set_view__treat_browse_request_BrowseValue_1(service_set_view__p_session,
                     service_set_view__p_req_msg,
                     service_set_view__p_resp_msg,
                     service_set_view__p_nid_view,
                     service_set_view__p_nb_target_max,
                     service_set_view__l_bvi,
                     service_set_view__l_first_iteration);
                  service_set_view__l_first_iteration = false;
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

void service_set_view__treat_browse_next_request_BrowseContinuationPoint_1(
   const constants__t_session_i service_set_view__p_session,
   const constants__t_msg_i service_set_view__p_req_msg,
   const constants__t_msg_i service_set_view__p_resp_msg,
   const constants__t_BrowseValue_i service_set_view__p_cpi) {
   {
      constants__t_ContinuationPointId_i service_set_view__l_continuationPointId;
      constants_statuscodes_bs__t_StatusCode_i service_set_view__l_statusCode;
      t_entier4 service_set_view__l_nbTargets;
      constants__t_BrowseResultReferences_i service_set_view__l_browseResult;
      
      msg_browse_next_bs__getall_ContinuationPoint(service_set_view__p_req_msg,
         service_set_view__p_cpi,
         &service_set_view__l_continuationPointId);
      translate_browse_path__set_browse_value_context_from_continuation_point(service_set_view__p_session,
         service_set_view__l_continuationPointId,
         &service_set_view__l_statusCode);
      if (service_set_view__l_statusCode == constants_statuscodes_bs__e_sc_ok) {
         translate_browse_path__compute_browse_result(&service_set_view__l_statusCode,
            &service_set_view__l_continuationPointId,
            &service_set_view__l_nbTargets);
         translate_browse_path__clear_browse_value_context();
         if ((service_set_view__l_statusCode == constants_statuscodes_bs__e_sc_ok) ||
            (service_set_view__l_statusCode == constants_statuscodes_bs__e_sc_bad_no_continuation_points)) {
            translate_browse_path__getall_and_move_browse_result(&service_set_view__l_nbTargets,
               &service_set_view__l_browseResult);
            msg_browse_next_bs__set_ResponseBrowseNext_BrowseResult(service_set_view__p_resp_msg,
               service_set_view__p_cpi,
               service_set_view__l_nbTargets,
               service_set_view__l_browseResult);
            msg_browse_next_bs__set_ResponseBrowseNext_ContinuationPoint(service_set_view__p_resp_msg,
               service_set_view__p_cpi,
               service_set_view__l_continuationPointId);
         }
      }
      msg_browse_next_bs__set_ResponseBrowseNext_BrowseStatus(service_set_view__p_resp_msg,
         service_set_view__p_cpi,
         service_set_view__l_statusCode);
   }
}

void service_set_view__treat_browse_next_request_ReleaseContinuationPoint_1(
   const constants__t_session_i service_set_view__p_session,
   const constants__t_msg_i service_set_view__p_req_msg,
   const constants__t_msg_i service_set_view__p_resp_msg,
   const constants__t_BrowseValue_i service_set_view__p_cpi) {
   {
      constants__t_ContinuationPointId_i service_set_view__l_continuationPointId;
      t_bool service_set_view__l_bres;
      
      msg_browse_next_bs__getall_ContinuationPoint(service_set_view__p_req_msg,
         service_set_view__p_cpi,
         &service_set_view__l_continuationPointId);
      translate_browse_path__release_continuation_point(service_set_view__p_session,
         service_set_view__l_continuationPointId,
         &service_set_view__l_bres);
      if (service_set_view__l_bres == true) {
         msg_browse_next_bs__set_ResponseBrowseNext_BrowseStatus(service_set_view__p_resp_msg,
            service_set_view__p_cpi,
            constants_statuscodes_bs__e_sc_ok);
      }
      else {
         msg_browse_next_bs__set_ResponseBrowseNext_BrowseStatus(service_set_view__p_resp_msg,
            service_set_view__p_cpi,
            constants_statuscodes_bs__e_sc_bad_continuation_point_invalid);
      }
   }
}

void service_set_view__treat_browse_next_request_BrowseContinuationPoints(
   const constants__t_session_i service_set_view__p_session,
   const constants__t_msg_i service_set_view__p_req_msg,
   const constants__t_msg_i service_set_view__p_resp_msg,
   const t_bool service_set_view__p_releaseCP,
   const t_entier4 service_set_view__p_nbCP,
   constants_statuscodes_bs__t_StatusCode_i * const service_set_view__StatusCode_service) {
   {
      t_bool service_set_view__l_isallocated;
      t_bool service_set_view__l_continue;
      constants__t_BrowseValue_i service_set_view__l_cpi;
      
      if ((service_set_view__p_nbCP > 0) &&
         (service_set_view__p_nbCP <= constants__k_n_BrowseResponse_max)) {
         msg_browse_next_bs__alloc_browse_next_response(service_set_view__p_resp_msg,
            service_set_view__p_nbCP,
            &service_set_view__l_isallocated);
         if (service_set_view__l_isallocated == true) {
            *service_set_view__StatusCode_service = constants_statuscodes_bs__e_sc_ok;
            service_browse_it__init_iter_browse_request(service_set_view__p_nbCP,
               &service_set_view__l_continue);
            while (service_set_view__l_continue == true) {
               service_browse_it__continue_iter_browse_request(&service_set_view__l_continue,
                  &service_set_view__l_cpi);
               if (service_set_view__p_releaseCP == false) {
                  service_set_view__treat_browse_next_request_BrowseContinuationPoint_1(service_set_view__p_session,
                     service_set_view__p_req_msg,
                     service_set_view__p_resp_msg,
                     service_set_view__l_cpi);
               }
               else {
                  service_set_view__treat_browse_next_request_ReleaseContinuationPoint_1(service_set_view__p_session,
                     service_set_view__p_req_msg,
                     service_set_view__p_resp_msg,
                     service_set_view__l_cpi);
               }
            }
         }
         else {
            *service_set_view__StatusCode_service = constants_statuscodes_bs__e_sc_bad_out_of_memory;
         }
      }
      else {
         if (service_set_view__p_nbCP == 0) {
            *service_set_view__StatusCode_service = constants_statuscodes_bs__e_sc_bad_nothing_to_do;
         }
         else {
            *service_set_view__StatusCode_service = constants_statuscodes_bs__e_sc_bad_too_many_ops;
         }
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

void service_set_view__treat_browse_next_request(
   const constants__t_session_i service_set_view__p_session,
   const constants__t_msg_i service_set_view__p_req_msg,
   const constants__t_msg_i service_set_view__p_resp_msg,
   constants_statuscodes_bs__t_StatusCode_i * const service_set_view__StatusCode_service) {
   {
      t_bool service_set_view__l_releaseContinuationPoints;
      t_entier4 service_set_view__l_nb_ContinuationPoints;
      
      msg_browse_next_bs__get_browse_next_request_params(service_set_view__p_req_msg,
         &service_set_view__l_releaseContinuationPoints,
         &service_set_view__l_nb_ContinuationPoints);
      service_set_view__treat_browse_next_request_BrowseContinuationPoints(service_set_view__p_session,
         service_set_view__p_req_msg,
         service_set_view__p_resp_msg,
         service_set_view__l_releaseContinuationPoints,
         service_set_view__l_nb_ContinuationPoints,
         service_set_view__StatusCode_service);
   }
}

void service_set_view__service_set_view_set_session_closed(
   const constants__t_session_i service_set_view__p_session) {
   translate_browse_path__set_session_closed(service_set_view__p_session);
}

void service_set_view__service_set_view_service_node_management_used(void) {
   translate_browse_path__continuation_points_UNINITIALISATION();
}

void service_set_view__service_set_view_UNINITIALISATION(void) {
   translate_browse_path__continuation_points_UNINITIALISATION();
}

void service_set_view__treat_translate_browse_paths_request(
   const constants__t_msg_i service_set_view__p_req_msg,
   const constants__t_msg_i service_set_view__p_resp_msg,
   constants_statuscodes_bs__t_StatusCode_i * const service_set_view__StatusCode_service) {
   {
      t_bool service_set_view__l_continue;
      constants__t_BrowsePath_i service_set_view__l_browsePath;
      
      translate_browse_path__init_translate_browse_paths_request(service_set_view__p_req_msg,
         service_set_view__StatusCode_service);
      if (*service_set_view__StatusCode_service == constants_statuscodes_bs__e_sc_ok) {
         translate_browse_path_it__init_iter_browsePaths(&service_set_view__l_continue);
         while (service_set_view__l_continue == true) {
            translate_browse_path_it__continue_iter_browsePath(&service_set_view__l_continue,
               &service_set_view__l_browsePath);
            translate_browse_path__treat_one_translate_browse_path(service_set_view__l_browsePath);
         }
      }
      translate_browse_path__write_translate_browse_paths_response(service_set_view__p_resp_msg);
   }
}

