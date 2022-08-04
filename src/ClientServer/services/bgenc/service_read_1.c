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

 File Name            : service_read_1.c

 Date                 : 04/08/2022 14:53:11

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "service_read_1.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void service_read_1__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void service_read_1__fill_read_response_1(
   const constants__t_TimestampsToReturn_i service_read_1__p_TimestampsToReturn,
   const constants__t_user_i service_read_1__p_user,
   const constants__t_LocaleIds_i service_read_1__p_locales,
   const constants__t_msg_i service_read_1__p_resp_msg,
   const constants_statuscodes_bs__t_StatusCode_i service_read_1__p_sc,
   const constants__t_NodeId_i service_read_1__p_nid,
   const constants__t_AttributeId_i service_read_1__p_aid,
   const constants__t_IndexRange_i service_read_1__p_index_range,
   const constants__t_ReadValue_i service_read_1__p_rvi) {
   {
      t_bool service_read_1__l_is_valid;
      constants__t_Node_i service_read_1__l_node;
      constants__t_Variant_i service_read_1__l_value;
      constants_statuscodes_bs__t_StatusCode_i service_read_1__l_sc;
      constants__t_RawStatusCode service_read_1__l_raw_sc;
      constants__t_Timestamp service_read_1__l_ts_src;
      constants__t_Timestamp service_read_1__l_ts_srv;
      
      if (service_read_1__p_sc == constants_statuscodes_bs__e_sc_ok) {
         address_space_itf__readall_AddressSpace_Node(service_read_1__p_nid,
            &service_read_1__l_is_valid,
            &service_read_1__l_node);
         if (service_read_1__l_is_valid == true) {
            address_space_itf__read_Node_Attribute(service_read_1__p_user,
               service_read_1__p_locales,
               service_read_1__l_node,
               service_read_1__p_nid,
               service_read_1__p_aid,
               service_read_1__p_index_range,
               &service_read_1__l_sc,
               &service_read_1__l_value,
               &service_read_1__l_raw_sc,
               &service_read_1__l_ts_src,
               &service_read_1__l_ts_srv);
            if (service_read_1__l_sc == constants_statuscodes_bs__e_sc_ok) {
               switch (service_read_1__p_TimestampsToReturn) {
               case constants__e_ttr_source:
                  service_read_1__l_ts_srv = constants__c_Timestamp_null;
                  break;
               case constants__e_ttr_server:
                  service_read_1__l_ts_src = constants__c_Timestamp_null;
                  break;
               case constants__e_ttr_neither:
                  service_read_1__l_ts_src = constants__c_Timestamp_null;
                  service_read_1__l_ts_srv = constants__c_Timestamp_null;
                  break;
               default:
                  break;
               }
               msg_read_response_bs__set_read_response(service_read_1__p_resp_msg,
                  service_read_1__p_rvi,
                  service_read_1__l_value,
                  service_read_1__l_raw_sc,
                  service_read_1__l_ts_src,
                  service_read_1__l_ts_srv);
               address_space_itf__read_AddressSpace_free_variant(service_read_1__l_value);
            }
            else {
               constants_statuscodes_bs__getall_conv_StatusCode_To_RawStatusCode(service_read_1__l_sc,
                  &service_read_1__l_raw_sc);
               msg_read_response_bs__set_read_response(service_read_1__p_resp_msg,
                  service_read_1__p_rvi,
                  constants__c_Variant_indet,
                  service_read_1__l_raw_sc,
                  constants__c_Timestamp_null,
                  constants__c_Timestamp_null);
            }
         }
         else {
            constants_statuscodes_bs__getall_conv_StatusCode_To_RawStatusCode(constants_statuscodes_bs__e_sc_bad_node_id_unknown,
               &service_read_1__l_raw_sc);
            msg_read_response_bs__set_read_response(service_read_1__p_resp_msg,
               service_read_1__p_rvi,
               constants__c_Variant_indet,
               service_read_1__l_raw_sc,
               constants__c_Timestamp_null,
               constants__c_Timestamp_null);
         }
      }
      else {
         constants_statuscodes_bs__getall_conv_StatusCode_To_RawStatusCode(service_read_1__p_sc,
            &service_read_1__l_raw_sc);
         msg_read_response_bs__set_read_response(service_read_1__p_resp_msg,
            service_read_1__p_rvi,
            constants__c_Variant_indet,
            service_read_1__l_raw_sc,
            constants__c_Timestamp_null,
            constants__c_Timestamp_null);
      }
   }
}

void service_read_1__fill_read_response(
   const constants__t_TimestampsToReturn_i service_read_1__p_TimestampsToReturn,
   const constants__t_user_i service_read_1__p_user,
   const constants__t_LocaleIds_i service_read_1__p_locales,
   const constants__t_msg_i service_read_1__req_msg,
   const constants__t_msg_i service_read_1__resp_msg) {
   {
      t_entier4 service_read_1__l_nb_ReadValue;
      t_bool service_read_1__l_continue;
      constants_statuscodes_bs__t_StatusCode_i service_read_1__l_sc;
      constants__t_ReadValue_i service_read_1__l_rvi;
      constants__t_NodeId_i service_read_1__l_nid;
      constants__t_AttributeId_i service_read_1__l_aid;
      constants__t_IndexRange_i service_read_1__l_index_range;
      
      msg_read_request__get_nb_ReadValue(&service_read_1__l_nb_ReadValue);
      service_read_it__init_iter_read_request(service_read_1__l_nb_ReadValue,
         &service_read_1__l_continue);
      while (service_read_1__l_continue == true) {
         service_read_it__continue_iter_read_request(&service_read_1__l_continue,
            &service_read_1__l_rvi);
         msg_read_request__getall_ReadValue_NodeId_AttributeId(service_read_1__req_msg,
            service_read_1__l_rvi,
            &service_read_1__l_sc,
            &service_read_1__l_nid,
            &service_read_1__l_aid,
            &service_read_1__l_index_range);
         service_read_1__fill_read_response_1(service_read_1__p_TimestampsToReturn,
            service_read_1__p_user,
            service_read_1__p_locales,
            service_read_1__resp_msg,
            service_read_1__l_sc,
            service_read_1__l_nid,
            service_read_1__l_aid,
            service_read_1__l_index_range,
            service_read_1__l_rvi);
      }
   }
}

