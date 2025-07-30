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

 File Name            : msg_history_read_request.c

 Date                 : 27/08/2025 14:29:43

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "msg_history_read_request.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void msg_history_read_request__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void msg_history_read_request__check_history_read_request(
   const constants__t_msg_i msg_history_read_request__p_req_msg,
   constants_statuscodes_bs__t_StatusCode_i * const msg_history_read_request__p_StatusCode,
   constants__t_readRawModifiedDetails_i * const msg_history_read_request__p_readRawDetails,
   t_bool * const msg_history_read_request__p_TsSrcRequired,
   t_bool * const msg_history_read_request__p_TsSrvRequired,
   t_bool * const msg_history_read_request__p_ContinuationPoint,
   t_entier4 * const msg_history_read_request__p_nb_nodes_to_read) {
   {
      t_bool msg_history_read_request__l_read_ok;
      constants__t_TimestampsToReturn_i msg_history_read_request__l_TSToReturn;
      
      *msg_history_read_request__p_StatusCode = constants_statuscodes_bs__c_StatusCode_indet;
      *msg_history_read_request__p_readRawDetails = constants__c_readRawModifiedDetails_indet;
      *msg_history_read_request__p_TsSrcRequired = false;
      *msg_history_read_request__p_TsSrvRequired = false;
      *msg_history_read_request__p_ContinuationPoint = false;
      msg_history_read_request_bs__get_msg_hist_read_req_nb_nodes_to_read(msg_history_read_request__p_req_msg,
         msg_history_read_request__p_nb_nodes_to_read);
      msg_history_read_request__l_read_ok = ((0 < *msg_history_read_request__p_nb_nodes_to_read) &&
         (*msg_history_read_request__p_nb_nodes_to_read <= constants__k_n_histRead_max));
      if (msg_history_read_request__l_read_ok == true) {
         msg_history_read_request_bs__getall_msg_hist_read_req_read_details(msg_history_read_request__p_req_msg,
            msg_history_read_request__p_StatusCode,
            msg_history_read_request__p_readRawDetails);
         if (*msg_history_read_request__p_StatusCode == constants_statuscodes_bs__e_sc_ok) {
            msg_history_read_request_bs__get_msg_hist_read_req_TSToReturn(msg_history_read_request__p_req_msg,
               &msg_history_read_request__l_TSToReturn);
            if (((msg_history_read_request__l_TSToReturn == constants__e_ttr_source) ||
               (msg_history_read_request__l_TSToReturn == constants__e_ttr_server)) ||
               (msg_history_read_request__l_TSToReturn == constants__e_ttr_both)) {
               *msg_history_read_request__p_TsSrcRequired = (msg_history_read_request__l_TSToReturn != constants__e_ttr_server);
               *msg_history_read_request__p_TsSrvRequired = (msg_history_read_request__l_TSToReturn != constants__e_ttr_source);
               msg_history_read_request_bs__get_msg_hist_read_req_release_CP(msg_history_read_request__p_req_msg,
                  msg_history_read_request__p_ContinuationPoint);
               *msg_history_read_request__p_StatusCode = constants_statuscodes_bs__e_sc_ok;
            }
            else {
               *msg_history_read_request__p_StatusCode = constants_statuscodes_bs__e_sc_bad_timestamps_to_return_invalid;
            }
         }
      }
      else {
         if (*msg_history_read_request__p_nb_nodes_to_read <= 0) {
            *msg_history_read_request__p_StatusCode = constants_statuscodes_bs__e_sc_bad_nothing_to_do;
         }
         else if (*msg_history_read_request__p_nb_nodes_to_read > constants__k_n_histRead_max) {
            *msg_history_read_request__p_StatusCode = constants_statuscodes_bs__e_sc_bad_too_many_ops;
         }
         *msg_history_read_request__p_nb_nodes_to_read = 0;
      }
   }
}

