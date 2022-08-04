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

 File Name            : service_write.c

 Date                 : 04/08/2022 14:53:16

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "service_write.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void service_write__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void service_write__treat_write_request(
   const constants__t_user_i service_write__p_user,
   const constants__t_LocaleIds_i service_write__p_locales,
   const constants__t_msg_i service_write__write_msg,
   const constants__t_msg_i service_write__resp_msg,
   constants_statuscodes_bs__t_StatusCode_i * const service_write__StatusCode_service) {
   {
      t_entier4 service_write__l_nb_req;
      t_bool service_write__l_bret;
      
      service_write_decode_bs__decode_write_request(service_write__write_msg,
         service_write__StatusCode_service);
      if (*service_write__StatusCode_service == constants_statuscodes_bs__e_sc_ok) {
         service_write_decode_bs__get_nb_WriteValue(&service_write__l_nb_req);
         service_write_1__alloc_write_request_responses(service_write__l_nb_req,
            &service_write__l_bret);
         if (service_write__l_bret == true) {
            service_write_1__treat_write_request_WriteValues(service_write__p_user,
               service_write__p_locales,
               service_write__StatusCode_service);
         }
         else {
            *service_write__StatusCode_service = constants_statuscodes_bs__e_sc_bad_out_of_memory;
         }
      }
      service_write_decode_bs__free_write_request();
      service_write_1__write_WriteResponse_msg_out(service_write__resp_msg);
      service_write_1__dealloc_write_request_responses();
   }
}

