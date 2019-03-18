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

 File Name            : service_read.c

 Date                 : 29/01/2019 12:57:52

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "service_read.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void service_read__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void service_read__fill_read_response_1(
   const constants__t_user_i service_read__p_user,
   const constants__t_msg_i service_read__p_resp_msg,
   const constants_statuscodes_bs__t_StatusCode_i service_read__p_sc,
   const constants__t_NodeId_i service_read__p_nid,
   const constants__t_AttributeId_i service_read__p_aid,
   const constants__t_IndexRange_i service_read__p_index_range,
   const constants__t_ReadValue_i service_read__p_rvi) {
   {
      t_bool service_read__l_is_valid;
      constants__t_Node_i service_read__l_node;
      constants__t_NodeClass_i service_read__l_ncl;
      constants__t_Variant_i service_read__l_value;
      constants_statuscodes_bs__t_StatusCode_i service_read__l_sc;
      constants__t_RawStatusCode service_read__l_raw_sc;
      t_bool service_read__l_authorized;
      
      if (service_read__p_sc == constants_statuscodes_bs__e_sc_ok) {
         address_space__readall_AddressSpace_Node(service_read__p_nid,
            &service_read__l_is_valid,
            &service_read__l_node);
         if (service_read__l_is_valid == true) {
            address_space__get_user_authorization(constants__e_operation_type_read,
               service_read__p_nid,
               service_read__p_aid,
               service_read__p_user,
               &service_read__l_authorized);
            if (service_read__l_authorized == true) {
               address_space__read_NodeClass_Attribute(service_read__p_user,
                  service_read__l_node,
                  service_read__p_aid,
                  service_read__p_index_range,
                  &service_read__l_sc,
                  &service_read__l_ncl,
                  &service_read__l_value);
               if (service_read__l_sc == constants_statuscodes_bs__e_sc_ok) {
                  if ((service_read__p_aid == constants__e_aid_Value) &&
                     (service_read__l_ncl == constants__e_ncl_Variable)) {
                     address_space__get_Value_StatusCode(service_read__p_user,
                        service_read__l_node,
                        &service_read__l_raw_sc);
                  }
                  else {
                     constants_statuscodes_bs__getall_conv_StatusCode_To_RawStatusCode(constants_statuscodes_bs__e_sc_ok,
                        &service_read__l_raw_sc);
                  }
               }
               else {
                  constants_statuscodes_bs__getall_conv_StatusCode_To_RawStatusCode(service_read__l_sc,
                     &service_read__l_raw_sc);
               }
               msg_read_response_bs__set_read_response(service_read__p_resp_msg,
                  service_read__p_rvi,
                  service_read__l_value,
                  service_read__l_raw_sc,
                  service_read__p_aid);
               address_space__read_AddressSpace_free_variant(service_read__l_value);
            }
            else {
               constants_statuscodes_bs__getall_conv_StatusCode_To_RawStatusCode(constants_statuscodes_bs__e_sc_bad_user_access_denied,
                  &service_read__l_raw_sc);
               msg_read_response_bs__set_read_response(service_read__p_resp_msg,
                  service_read__p_rvi,
                  constants__c_Variant_indet,
                  service_read__l_raw_sc,
                  service_read__p_aid);
            }
         }
         else {
            constants_statuscodes_bs__getall_conv_StatusCode_To_RawStatusCode(constants_statuscodes_bs__e_sc_bad_node_id_unknown,
               &service_read__l_raw_sc);
            msg_read_response_bs__set_read_response(service_read__p_resp_msg,
               service_read__p_rvi,
               constants__c_Variant_indet,
               service_read__l_raw_sc,
               service_read__p_aid);
         }
      }
      else {
         constants_statuscodes_bs__getall_conv_StatusCode_To_RawStatusCode(service_read__p_sc,
            &service_read__l_raw_sc);
         msg_read_response_bs__set_read_response(service_read__p_resp_msg,
            service_read__p_rvi,
            constants__c_Variant_indet,
            service_read__l_raw_sc,
            service_read__p_aid);
      }
   }
}

void service_read__fill_read_response(
   const constants__t_user_i service_read__p_user,
   const constants__t_msg_i service_read__req_msg,
   const constants__t_msg_i service_read__resp_msg) {
   {
      t_entier4 service_read__l_nb_ReadValue;
      t_bool service_read__l_continue;
      constants_statuscodes_bs__t_StatusCode_i service_read__l_sc;
      constants__t_ReadValue_i service_read__l_rvi;
      constants__t_NodeId_i service_read__l_nid;
      constants__t_AttributeId_i service_read__l_aid;
      constants__t_IndexRange_i service_read__l_index_range;
      
      msg_read_request__get_nb_ReadValue(&service_read__l_nb_ReadValue);
      service_read_it__init_iter_write_request(service_read__l_nb_ReadValue,
         &service_read__l_continue);
      while (service_read__l_continue == true) {
         service_read_it__continue_iter_write_request(&service_read__l_continue,
            &service_read__l_rvi);
         msg_read_request__getall_ReadValue_NodeId_AttributeId(service_read__req_msg,
            service_read__l_rvi,
            &service_read__l_sc,
            &service_read__l_nid,
            &service_read__l_aid,
            &service_read__l_index_range);
         service_read__fill_read_response_1(service_read__p_user,
            service_read__resp_msg,
            service_read__l_sc,
            service_read__l_nid,
            service_read__l_aid,
            service_read__l_index_range,
            service_read__l_rvi);
      }
   }
}

