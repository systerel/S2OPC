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

 File Name            : address_space_itf.c

 Date                 : 01/06/2022 17:07:46

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "address_space_itf.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void address_space_itf__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void address_space_itf__treat_write_request(
   const constants__t_user_i address_space_itf__p_user,
   const constants__t_LocaleIds_i address_space_itf__p_locales,
   const constants__t_msg_i address_space_itf__write_msg,
   const constants__t_msg_i address_space_itf__resp_msg,
   constants_statuscodes_bs__t_StatusCode_i * const address_space_itf__StatusCode_service) {
   {
      t_entier4 address_space_itf__l_nb_req;
      t_bool address_space_itf__l_bret;
      
      service_write_decode_bs__decode_write_request(address_space_itf__write_msg,
         address_space_itf__StatusCode_service);
      if (*address_space_itf__StatusCode_service == constants_statuscodes_bs__e_sc_ok) {
         service_write_decode_bs__get_nb_WriteValue(&address_space_itf__l_nb_req);
         address_space__alloc_write_request_responses(address_space_itf__l_nb_req,
            &address_space_itf__l_bret);
         if (address_space_itf__l_bret == true) {
            address_space__treat_write_request_WriteValues(address_space_itf__p_user,
               address_space_itf__p_locales,
               address_space_itf__StatusCode_service);
         }
         else {
            *address_space_itf__StatusCode_service = constants_statuscodes_bs__e_sc_bad_out_of_memory;
         }
      }
      service_write_decode_bs__free_write_request();
      address_space__write_WriteResponse_msg_out(address_space_itf__resp_msg);
      address_space__dealloc_write_request_responses();
   }
}

