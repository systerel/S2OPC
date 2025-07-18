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

 File Name            : session_core_2.c

 Date                 : 18/11/2026 09:48:10

 C Translator Version : tradc Java V2.0 (2026-09-14)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "session_core_2.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void session_core_2__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void session_core_2__add_session(
   const constants__t_session_i session_core_2__p_session,
   const t_bool session_core_2__p_is_client,
   const constants__t_timeref_i session_core_2__p_timeref) {
   session_core_3__increase_card_s_session();
   session_core_3__set_session_state_1(session_core_2__p_session,
      constants__e_session_init);
   session_core_3__set_client_session(session_core_2__p_session,
      session_core_2__p_is_client);
   session_core_3__set_init_time(session_core_2__p_session,
      session_core_2__p_timeref);
   session_core_3__set_server_session_preferred_locales(session_core_2__p_session,
      constants__c_LocaleIds_empty);
}

void session_core_2__remove_session(
   const constants__t_session_i session_core_2__p_session) {
   session_core_3__decrease_card_s_session();
   session_core_3__reset_session_state(session_core_2__p_session);
   session_core_3__set_client_session(session_core_2__p_session,
      false);
   session_core_3__reset_init_time(session_core_2__p_session);
}

void session_core_2__reset_session_channel(
   const constants__t_session_i session_core_2__p_session) {
   {
      t_bool session_core_2__l_dom;
      constants__t_channel_i session_core_2__l_channel;
      
      session_core_3__getall_session_channel(session_core_2__p_session,
         &session_core_2__l_dom,
         &session_core_2__l_channel);
      if (session_core_2__l_dom == true) {
         session_core_3__decrease_channel_nb_sessions(session_core_2__l_channel);
      }
      session_core_3__reset_channel(session_core_2__p_session);
   }
}

void session_core_2__set_session_channel(
   const constants__t_session_i session_core_2__session,
   const constants__t_channel_i session_core_2__p_channel) {
   {
      t_bool session_core_2__l_dom;
      constants__t_channel_i session_core_2__l_prev_channel;
      
      session_core_3__getall_session_channel(session_core_2__session,
         &session_core_2__l_dom,
         &session_core_2__l_prev_channel);
      if (session_core_2__l_dom == true) {
         if (session_core_2__l_prev_channel != session_core_2__p_channel) {
            session_core_3__decrease_channel_nb_sessions(session_core_2__l_prev_channel);
            session_core_3__increase_channel_nb_sessions(session_core_2__p_channel);
         }
         else {
            ;
         }
      }
      else {
         session_core_3__increase_channel_nb_sessions(session_core_2__p_channel);
      }
      session_core_3__set_channel(session_core_2__session,
         session_core_2__p_channel);
   }
}

