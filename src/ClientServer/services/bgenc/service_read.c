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

 Date                 : 04/08/2022 14:53:11

 C Translator Version : tradc Java V1.2 (06/02/2022)

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
void service_read__treat_read_request(
   const constants__t_user_i service_read__p_user,
   const constants__t_LocaleIds_i service_read__p_locales,
   const constants__t_msg_i service_read__p_request_msg,
   const constants__t_msg_i service_read__p_response_msg,
   constants_statuscodes_bs__t_StatusCode_i * const service_read__StatusCode_service) {
   {
      t_entier4 service_read__l_nb_ReadValue;
      t_bool service_read__l_is_valid;
      constants__t_TimestampsToReturn_i service_read__l_TimestampsToReturn;
      
      service_read_1__check_ReadRequest(service_read__p_request_msg,
         &service_read__l_is_valid,
         &service_read__l_nb_ReadValue,
         &service_read__l_TimestampsToReturn,
         service_read__StatusCode_service);
      if (service_read__l_is_valid == true) {
         service_read_1__alloc_read_response(service_read__l_nb_ReadValue,
            service_read__p_response_msg,
            &service_read__l_is_valid);
         if (service_read__l_is_valid == true) {
            service_read_1__fill_read_response(service_read__l_TimestampsToReturn,
               service_read__p_user,
               service_read__p_locales,
               service_read__p_request_msg,
               service_read__p_response_msg);
         }
         else {
            *service_read__StatusCode_service = constants_statuscodes_bs__e_sc_bad_out_of_memory;
         }
      }
   }
}

