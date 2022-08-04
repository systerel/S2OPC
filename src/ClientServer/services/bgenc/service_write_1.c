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

 File Name            : service_write_1.c

 Date                 : 04/08/2022 14:53:15

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "service_write_1.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
t_bool service_write_1__ResponseWrite_allocated;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void service_write_1__INITIALISATION(void) {
   service_write_1__ResponseWrite_allocated = false;
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void service_write_1__alloc_write_request_responses(
   const t_entier4 service_write_1__nb_req,
   t_bool * const service_write_1__bret) {
   if (service_write_1__nb_req <= constants__k_n_WriteResponse_max) {
      response_write_bs__alloc_write_request_responses_malloc(service_write_1__nb_req,
         &service_write_1__ResponseWrite_allocated);
   }
   else {
      service_write_1__ResponseWrite_allocated = false;
   }
   *service_write_1__bret = service_write_1__ResponseWrite_allocated;
}

void service_write_1__treat_write_request_WriteValues(
   const constants__t_user_i service_write_1__p_user,
   const constants__t_LocaleIds_i service_write_1__p_locales,
   constants_statuscodes_bs__t_StatusCode_i * const service_write_1__StatusCode_service) {
   {
      t_entier4 service_write_1__l_nb_req;
      t_bool service_write_1__l_continue;
      constants__t_WriteValue_i service_write_1__l_wvi;
      constants_statuscodes_bs__t_StatusCode_i service_write_1__l_status;
      
      *service_write_1__StatusCode_service = constants_statuscodes_bs__e_sc_ok;
      service_write_decode_bs__get_nb_WriteValue(&service_write_1__l_nb_req);
      service_write_1_it__init_iter_write_request(service_write_1__l_nb_req,
         &service_write_1__l_continue);
      while (service_write_1__l_continue == true) {
         service_write_1_it__continue_iter_write_request(&service_write_1__l_continue,
            &service_write_1__l_wvi);
         address_space__treat_write_request_WriteValue(service_write_1__p_user,
            service_write_1__p_locales,
            service_write_1__l_wvi,
            &service_write_1__l_status);
         response_write_bs__set_ResponseWrite_StatusCode(service_write_1__l_wvi,
            service_write_1__l_status);
      }
   }
}

void service_write_1__dealloc_write_request_responses(void) {
   service_write_1__ResponseWrite_allocated = false;
   response_write_bs__reset_ResponseWrite();
}

