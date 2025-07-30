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

 File Name            : service_history_read.c

 Date                 : 22/10/2025 10:33:16

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "service_history_read.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void service_history_read__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void service_history_read__l_fill_response_hist_read_1(
   const constants__t_msg_i service_history_read__p_req_msg,
   const constants__t_msg_i service_history_read__p_resp_msg,
   const constants__t_Timestamp service_history_read__p_currentTs,
   const t_entier4 service_history_read__p_index,
   const constants__t_user_i service_history_read__p_user,
   const constants__t_sessionRoles_i service_history_read__p_roles,
   const constants__t_readRawModifiedDetails_i service_history_read__p_readRawDetails,
   const t_bool service_history_read__p_TsSrvRequired,
   const t_bool service_history_read__p_TsSrcRequired,
   const t_bool service_history_read__p_ContinuationPoint) {
   {
      constants__t_historyReadValueId_i service_history_read__l_singleValueId;
      constants__t_NodeId_i service_history_read__l_nodeId;
      t_bool service_history_read__l_nid_valid;
      constants__t_Node_i service_history_read__l_node;
      t_bool service_history_read__l_access_histRead;
      constants_statuscodes_bs__t_StatusCode_i service_history_read__l_sc;
      t_bool service_history_read__l_bres;
      constants__t_Nonce_i service_history_read__l_contPoint;
      t_entier4 service_history_read__l_nb_dataValues;
      constants__t_DataValue_array_i service_history_read__l_dataValues;
      
      service_history_read__l_nodeId = constants__c_NodeId_indet;
      service_history_read__l_contPoint = constants__c_Nonce_indet;
      service_history_read__l_nb_dataValues = 0;
      service_history_read__l_dataValues = constants__c_DataValue_array_indet;
      msg_history_read_request__getall_msg_hist_read_req_singleValueId(service_history_read__p_req_msg,
         service_history_read__p_index,
         &service_history_read__l_sc,
         &service_history_read__l_singleValueId,
         &service_history_read__l_nodeId);
      if (service_history_read__l_sc == constants_statuscodes_bs__e_sc_ok) {
         address_space_itf__readall_AddressSpace_Node(service_history_read__l_nodeId,
            &service_history_read__l_nid_valid,
            &service_history_read__l_node);
         if (service_history_read__l_nid_valid == true) {
            address_space_itf__has_access_level_histRead(service_history_read__l_node,
               &service_history_read__l_access_histRead);
            if (service_history_read__l_access_histRead == true) {
               address_space_itf__get_user_authorization(constants__e_operation_type_historyread,
                  service_history_read__l_nodeId,
                  constants__e_aid_Value,
                  service_history_read__p_user,
                  service_history_read__p_roles,
                  &service_history_read__l_bres);
               if (service_history_read__l_bres == true) {
                  history_read_treatment_bs__external_history_raw_read(service_history_read__p_readRawDetails,
                     service_history_read__p_TsSrcRequired,
                     service_history_read__p_ContinuationPoint,
                     service_history_read__l_singleValueId,
                     &service_history_read__l_sc,
                     &service_history_read__l_contPoint,
                     &service_history_read__l_nb_dataValues,
                     &service_history_read__l_dataValues);
                  if ((service_history_read__p_TsSrvRequired == true) &&
                     (service_history_read__l_sc == constants_statuscodes_bs__e_sc_ok)) {
                     history_read_treatment_bs__set_ts_srv_dataValues(service_history_read__l_nb_dataValues,
                        service_history_read__l_dataValues,
                        service_history_read__p_currentTs);
                  }
               }
               else {
                  service_history_read__l_sc = constants_statuscodes_bs__e_sc_bad_user_access_denied;
               }
            }
            else {
               service_history_read__l_sc = constants_statuscodes_bs__e_sc_bad_user_access_denied;
            }
         }
      }
      msg_history_read_response_bs__set_msg_hist_read_response(service_history_read__p_resp_msg,
         service_history_read__p_index,
         service_history_read__l_sc,
         service_history_read__l_contPoint,
         service_history_read__l_nb_dataValues,
         service_history_read__l_dataValues);
   }
}

void service_history_read__l_fill_response_hist_read(
   const constants__t_msg_i service_history_read__p_req_msg,
   const constants__t_msg_i service_history_read__p_resp_msg,
   const constants__t_user_i service_history_read__p_user,
   const constants__t_sessionRoles_i service_history_read__p_roles,
   const t_entier4 service_history_read__p_nb_nodes,
   const constants__t_readRawModifiedDetails_i service_history_read__p_readRawDetails,
   const t_bool service_history_read__p_TsSrcRequired,
   const t_bool service_history_read__p_TsSrvRequired,
   const t_bool service_history_read__p_ContinuationPoint) {
   {
      t_bool service_history_read__l_continue;
      t_entier4 service_history_read__l_index;
      constants__t_Timestamp service_history_read__l_currentTs;
      
      service_history_read__l_index = 0;
      service_history_read__l_currentTs = constants__c_Timestamp_null;
      if (service_history_read__p_TsSrvRequired == true) {
         constants__get_CurrentTimestamp(&service_history_read__l_currentTs);
      }
      history_read_it__init_iter_hist_read_request(service_history_read__p_nb_nodes,
         &service_history_read__l_continue);
      while (service_history_read__l_continue == true) {
         history_read_it__continue_iter_hist_read_request(&service_history_read__l_continue,
            &service_history_read__l_index);
         service_history_read__l_fill_response_hist_read_1(service_history_read__p_req_msg,
            service_history_read__p_resp_msg,
            service_history_read__l_currentTs,
            service_history_read__l_index,
            service_history_read__p_user,
            service_history_read__p_roles,
            service_history_read__p_readRawDetails,
            service_history_read__p_TsSrvRequired,
            service_history_read__p_TsSrcRequired,
            service_history_read__p_ContinuationPoint);
      }
   }
}

void service_history_read__treat_history_read_request(
   const constants__t_msg_i service_history_read__p_req_msg,
   const constants__t_msg_i service_history_read__p_resp_msg,
   const constants__t_user_i service_history_read__p_user,
   constants_statuscodes_bs__t_StatusCode_i * const service_history_read__p_StatusCode) {
   {
      constants__t_sessionRoles_i service_history_read__l_roles;
      constants__t_readRawModifiedDetails_i service_history_read__l_readRawDetails;
      t_bool service_history_read__l_TsSrcRequired;
      t_bool service_history_read__l_TsSrvRequired;
      t_bool service_history_read__l_ContinuationPoint;
      t_entier4 service_history_read__l_nb_nodes_to_read;
      t_bool service_history_read__l_bres;
      
      *service_history_read__p_StatusCode = constants_statuscodes_bs__c_StatusCode_indet;
      address_space_itf__get_user_roles(service_history_read__p_user,
         &service_history_read__l_roles);
      msg_history_read_request__check_history_read_request(service_history_read__p_req_msg,
         service_history_read__p_StatusCode,
         &service_history_read__l_readRawDetails,
         &service_history_read__l_TsSrcRequired,
         &service_history_read__l_TsSrvRequired,
         &service_history_read__l_ContinuationPoint,
         &service_history_read__l_nb_nodes_to_read);
      if (*service_history_read__p_StatusCode == constants_statuscodes_bs__e_sc_ok) {
         msg_history_read_response_bs__alloc_msg_hist_read_resp_results(service_history_read__l_nb_nodes_to_read,
            service_history_read__p_resp_msg,
            &service_history_read__l_bres);
         if (service_history_read__l_bres == false) {
            *service_history_read__p_StatusCode = constants_statuscodes_bs__e_sc_bad_out_of_memory;
         }
         else {
            service_history_read__l_fill_response_hist_read(service_history_read__p_req_msg,
               service_history_read__p_resp_msg,
               service_history_read__p_user,
               service_history_read__l_roles,
               service_history_read__l_nb_nodes_to_read,
               service_history_read__l_readRawDetails,
               service_history_read__l_TsSrcRequired,
               service_history_read__l_TsSrvRequired,
               service_history_read__l_ContinuationPoint);
         }
      }
   }
}

