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

 File Name            : msg_read_request.c

 Date                 : 04/08/2022 14:53:07

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "msg_read_request.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
t_entier4 msg_read_request__nb_ReadValue;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void msg_read_request__INITIALISATION(void) {
   msg_read_request__nb_ReadValue = 0;
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void msg_read_request__check_ReadRequest(
   const constants__t_msg_i msg_read_request__p_msg,
   t_bool * const msg_read_request__p_read_ok,
   t_entier4 * const msg_read_request__p_nb_ReadValue,
   constants__t_TimestampsToReturn_i * const msg_read_request__p_tsToReturn,
   constants_statuscodes_bs__t_StatusCode_i * const msg_read_request__p_statusCode) {
   msg_read_request_bs__read_req_nb_ReadValue(msg_read_request__p_msg,
      msg_read_request__p_nb_ReadValue);
   *msg_read_request__p_read_ok = ((0 < *msg_read_request__p_nb_ReadValue) &&
      (*msg_read_request__p_nb_ReadValue <= constants__k_n_read_resp_max));
   *msg_read_request__p_tsToReturn = constants__c_TimestampsToReturn_indet;
   *msg_read_request__p_statusCode = constants_statuscodes_bs__c_StatusCode_indet;
   if (*msg_read_request__p_read_ok == true) {
      msg_read_request__nb_ReadValue = *msg_read_request__p_nb_ReadValue;
      msg_read_request_bs__read_req_TimestampsToReturn(msg_read_request__p_msg,
         msg_read_request__p_tsToReturn);
      if (*msg_read_request__p_tsToReturn == constants__c_TimestampsToReturn_indet) {
         *msg_read_request__p_read_ok = false;
         *msg_read_request__p_statusCode = constants_statuscodes_bs__e_sc_bad_timestamps_to_return_invalid;
      }
      else {
         msg_read_request_bs__read_req_MaxAge(msg_read_request__p_msg,
            msg_read_request__p_read_ok);
         if (*msg_read_request__p_read_ok == false) {
            *msg_read_request__p_statusCode = constants_statuscodes_bs__e_sc_bad_max_age_invalid;
         }
         else {
            *msg_read_request__p_statusCode = constants_statuscodes_bs__e_sc_ok;
         }
      }
   }
   else {
      if (*msg_read_request__p_nb_ReadValue <= 0) {
         *msg_read_request__p_statusCode = constants_statuscodes_bs__e_sc_bad_nothing_to_do;
      }
      else if (*msg_read_request__p_nb_ReadValue > constants__k_n_read_resp_max) {
         *msg_read_request__p_statusCode = constants_statuscodes_bs__e_sc_bad_too_many_ops;
      }
      msg_read_request__nb_ReadValue = 0;
   }
}

void msg_read_request__getall_ReadValue_NodeId_AttributeId(
   const constants__t_msg_i msg_read_request__p_msg,
   const constants__t_ReadValue_i msg_read_request__p_rvi,
   constants_statuscodes_bs__t_StatusCode_i * const msg_read_request__p_sc,
   constants__t_NodeId_i * const msg_read_request__p_nid,
   constants__t_AttributeId_i * const msg_read_request__p_aid,
   constants__t_IndexRange_i * const msg_read_request__p_index_range) {
   {
      constants__t_QualifiedName_i msg_read_request__l_data_encoding;
      t_bool msg_read_request__l_is_known_data;
      
      msg_read_request_bs__getall_req_ReadValue_NodeId(msg_read_request__p_msg,
         msg_read_request__p_rvi,
         msg_read_request__p_nid);
      msg_read_request_bs__getall_req_ReadValue_AttributeId(msg_read_request__p_msg,
         msg_read_request__p_rvi,
         msg_read_request__p_sc,
         msg_read_request__p_aid);
      msg_read_request_bs__getall_req_ReadValue_IndexRange(msg_read_request__p_msg,
         msg_read_request__p_rvi,
         msg_read_request__p_index_range);
      msg_read_request_bs__getall_req_ReadValue_DataEncoding(msg_read_request__p_msg,
         msg_read_request__p_rvi,
         &msg_read_request__l_is_known_data,
         &msg_read_request__l_data_encoding);
      if ((*msg_read_request__p_sc == constants_statuscodes_bs__e_sc_ok) &&
         (msg_read_request__l_data_encoding != constants__c_QualifiedName_indet)) {
         if ((*msg_read_request__p_aid != constants__e_aid_Value) ||
            (msg_read_request__l_is_known_data == false)) {
            *msg_read_request__p_sc = constants_statuscodes_bs__e_sc_bad_data_encoding_invalid;
            *msg_read_request__p_aid = constants__c_AttributeId_indet;
         }
      }
   }
}

void msg_read_request__get_nb_ReadValue(
   t_entier4 * const msg_read_request__p_nb_ReadValue) {
   *msg_read_request__p_nb_ReadValue = msg_read_request__nb_ReadValue;
}

