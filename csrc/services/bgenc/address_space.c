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

 File Name            : address_space.c

 Date                 : 24/01/2019 09:56:14

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "address_space.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
t_bool address_space__ResponseWrite_allocated;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void address_space__INITIALISATION(void) {
   address_space__ResponseWrite_allocated = false;
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void address_space__read_NodeClass_Attribute(
   const constants__t_user_i address_space__p_user,
   const constants__t_Node_i address_space__node,
   const constants__t_AttributeId_i address_space__aid,
   const constants__t_IndexRange_i address_space__index_range,
   constants__t_StatusCode_i * const address_space__sc,
   constants__t_NodeClass_i * const address_space__ncl,
   constants__t_Variant_i * const address_space__val) {
   address_space_bs__get_NodeClass(address_space__node,
      address_space__ncl);
   address_space_bs__read_AddressSpace_Attribute_value(address_space__p_user,
      address_space__node,
      *address_space__ncl,
      address_space__aid,
      address_space__index_range,
      address_space__sc,
      address_space__val);
}

void address_space__alloc_write_request_responses(
   const t_entier4 address_space__nb_req,
   t_bool * const address_space__bret) {
   if (address_space__nb_req <= constants__k_n_WriteResponse_max) {
      response_write_bs__alloc_write_request_responses_malloc(address_space__nb_req,
         &address_space__ResponseWrite_allocated);
   }
   else {
      address_space__ResponseWrite_allocated = false;
   }
   *address_space__bret = address_space__ResponseWrite_allocated;
}

void address_space__treat_write_request_WriteValues(
   const constants__t_user_i address_space__p_user,
   constants__t_StatusCode_i * const address_space__StatusCode_service) {
   {
      t_entier4 address_space__l_nb_req;
      t_bool address_space__l_continue;
      constants__t_AttributeId_i address_space__l_aid;
      constants__t_NodeId_i address_space__l_nid;
      constants__t_DataValue_i address_space__l_dataValue;
      constants__t_IndexRange_i address_space__l_index_range;
      constants__t_WriteValue_i address_space__l_wvi;
      constants__t_StatusCode_i address_space__l_status1;
      constants__t_StatusCode_i address_space__l_status2;
      constants__t_DataValue_i address_space__l_prev_dataValue;
      t_bool address_space__l_isvalid;
      t_bool address_space__l_local_treatment;
      constants__t_WriteValuePointer_i address_space__l_wv;
      t_bool address_space__l_bres_wv_copy;
      constants__t_WriteValuePointer_i address_space__l_wv_copy;
      
      *address_space__StatusCode_service = constants__e_sc_ok;
      service_write_decode_bs__get_nb_WriteValue(&address_space__l_nb_req);
      address_space_it__init_iter_write_request(address_space__l_nb_req,
         &address_space__l_continue);
      while (address_space__l_continue == true) {
         address_space_it__continue_iter_write_request(&address_space__l_continue,
            &address_space__l_wvi);
         service_write_decode_bs__getall_WriteValue(address_space__l_wvi,
            &address_space__l_isvalid,
            &address_space__l_status1,
            &address_space__l_nid,
            &address_space__l_aid,
            &address_space__l_dataValue,
            &address_space__l_index_range);
         address_space__treat_write_1(address_space__l_isvalid,
            address_space__l_status1,
            address_space__p_user,
            address_space__l_nid,
            address_space__l_aid,
            address_space__l_dataValue,
            address_space__l_index_range,
            &address_space__l_status2,
            &address_space__l_prev_dataValue);
         response_write_bs__set_ResponseWrite_StatusCode(address_space__l_wvi,
            address_space__l_status2);
         service_write_decode_bs__getall_WriteValuePointer(address_space__l_wvi,
            &address_space__l_wv);
         if (address_space__l_status2 == constants__e_sc_ok) {
            write_value_pointer_bs__copy_write_value_pointer_content(address_space__l_wv,
               &address_space__l_bres_wv_copy,
               &address_space__l_wv_copy);
            if (address_space__l_bres_wv_copy == true) {
               gen_subscription_event_bs__gen_data_changed_event(address_space__l_prev_dataValue,
                  address_space__l_wv_copy);
            }
            else {
               gen_subscription_event_bs__gen_data_changed_event_failed(address_space__l_prev_dataValue);
            }
         }
         else {
            address_space_bs__write_AddressSpace_free_dataValue(address_space__l_prev_dataValue);
         }
         service_mgr_1__is_local_service_treatment(&address_space__l_local_treatment);
         if (address_space__l_local_treatment == false) {
            write_value_pointer_bs__copy_write_value_pointer_content(address_space__l_wv,
               &address_space__l_bres_wv_copy,
               &address_space__l_wv_copy);
            if (address_space__l_bres_wv_copy == true) {
               service_response_cb_bs__srv_write_notification(address_space__l_wv_copy,
                  address_space__l_status2);
            }
            else {
               ;
            }
         }
      }
   }
}

void address_space__dealloc_write_request_responses(void) {
   address_space__ResponseWrite_allocated = false;
   response_write_bs__reset_ResponseWrite();
}

void address_space__treat_write_1(
   const t_bool address_space__isvalid,
   const constants__t_StatusCode_i address_space__status,
   const constants__t_user_i address_space__p_user,
   const constants__t_NodeId_i address_space__nid,
   const constants__t_AttributeId_i address_space__aid,
   const constants__t_DataValue_i address_space__dataValue,
   const constants__t_IndexRange_i address_space__index_range,
   constants__t_StatusCode_i * const address_space__serviceStatusCode,
   constants__t_DataValue_i * const address_space__prev_dataValue) {
   {
      t_bool address_space__l_isvalid;
      constants__t_Node_i address_space__l_node;
      constants__t_NodeClass_i address_space__l_ncl;
      constants__t_access_level address_space__l_access_lvl;
      t_bool address_space__l_access_write;
      t_bool address_space__l_authorized_write;
      t_bool address_space__l_local_treatment;
      
      *address_space__prev_dataValue = constants__c_DataValue_indet;
      if (address_space__isvalid == true) {
         address_space_bs__readall_AddressSpace_Node(address_space__nid,
            &address_space__l_isvalid,
            &address_space__l_node);
         if (address_space__l_isvalid == true) {
            address_space_bs__get_NodeClass(address_space__l_node,
               &address_space__l_ncl);
            if ((address_space__aid == constants__e_aid_Value) &&
               (address_space__l_ncl == constants__e_ncl_Variable)) {
               service_mgr_1__is_local_service_treatment(&address_space__l_local_treatment);
               if (address_space__l_local_treatment == true) {
                  address_space_bs__set_Value(address_space__p_user,
                     address_space__l_node,
                     address_space__dataValue,
                     address_space__index_range,
                     address_space__serviceStatusCode,
                     address_space__prev_dataValue);
               }
               else {
                  address_space_bs__get_AccessLevel(address_space__l_node,
                     &address_space__l_access_lvl);
                  constants__is_t_acces_level_currentWrite(address_space__l_access_lvl,
                     &address_space__l_access_write);
                  if (address_space__l_access_write == true) {
                     user_authorization_bs__get_user_authorization(constants__e_operation_type_write,
                        address_space__nid,
                        address_space__aid,
                        address_space__p_user,
                        &address_space__l_authorized_write);
                     if (address_space__l_authorized_write == true) {
                        address_space_bs__set_Value(address_space__p_user,
                           address_space__l_node,
                           address_space__dataValue,
                           address_space__index_range,
                           address_space__serviceStatusCode,
                           address_space__prev_dataValue);
                     }
                     else {
                        *address_space__serviceStatusCode = constants__e_sc_bad_user_access_denied;
                     }
                  }
                  else {
                     *address_space__serviceStatusCode = constants__e_sc_bad_not_writable;
                  }
               }
            }
            else {
               *address_space__serviceStatusCode = constants__e_sc_bad_not_writable;
            }
         }
         else {
            *address_space__serviceStatusCode = constants__e_sc_bad_node_id_unknown;
         }
      }
      else {
         *address_space__serviceStatusCode = address_space__status;
      }
   }
}

