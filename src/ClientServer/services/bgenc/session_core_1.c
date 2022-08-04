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

 File Name            : session_core_1.c

 Date                 : 04/08/2022 14:53:17

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "session_core_1.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void session_core_1__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void session_core_1__l_set_session_state(
   const constants__t_session_i session_core_1__p_session,
   const constants__t_sessionState session_core_1__p_state,
   const t_bool session_core_1__is_client) {
   {
      constants__t_sessionState session_core_1__l_prec_state;
      
      session_core_2__get_session_state(session_core_1__p_session,
         &session_core_1__l_prec_state);
      session_core_2__set_session_state_1(session_core_1__p_session,
         session_core_1__p_state);
      session_core_bs__notify_set_session_state(session_core_1__p_session,
         session_core_1__l_prec_state,
         session_core_1__p_state,
         constants_statuscodes_bs__c_StatusCode_indet,
         session_core_1__is_client);
   }
}

void session_core_1__l_reset_server_session_preferred_locales(
   const constants__t_session_i session_core_1__p_session) {
   {
      constants__t_LocaleIds_i session_core_1__l_old_localeIds;
      
      session_core_2__reset_server_session_preferred_locales(session_core_1__p_session,
         &session_core_1__l_old_localeIds);
      if (session_core_1__l_old_localeIds != constants__c_LocaleIds_indet) {
         constants__free_LocaleIds(session_core_1__l_old_localeIds);
      }
   }
}

void session_core_1__init_new_session(
   const t_bool session_core_1__is_client,
   constants__t_session_i * const session_core_1__p_session) {
   {
      t_bool session_core_1__l_is_session;
      t_bool session_core_1__l_continue;
      
      *session_core_1__p_session = constants__c_session_indet;
      session_core_1__l_is_session = true;
      session_core_1_it__init_iter_session(&session_core_1__l_continue);
      if (session_core_1__l_continue == true) {
         while ((session_core_1__l_continue == true) &&
            (session_core_1__l_is_session == true)) {
            session_core_1_it__continue_iter_session(&session_core_1__l_continue,
               session_core_1__p_session);
            session_core_2__is_valid_session(*session_core_1__p_session,
               &session_core_1__l_is_session);
         }
      }
      if (session_core_1__l_is_session == true) {
         *session_core_1__p_session = constants__c_session_indet;
      }
      else {
         session_core_2__add_session(*session_core_1__p_session,
            constants__e_session_init);
         session_core_bs__notify_set_session_state(*session_core_1__p_session,
            constants__e_session_closed,
            constants__e_session_init,
            constants_statuscodes_bs__c_StatusCode_indet,
            session_core_1__is_client);
      }
   }
}

void session_core_1__create_session(
   const constants__t_session_i session_core_1__session,
   const constants__t_channel_i session_core_1__channel,
   const constants__t_sessionState session_core_1__state,
   const t_bool session_core_1__is_client) {
   session_core_2__set_session_channel(session_core_1__session,
      session_core_1__channel);
   session_core_1__l_set_session_state(session_core_1__session,
      session_core_1__state,
      session_core_1__is_client);
}

void session_core_1__set_session_state(
   const constants__t_session_i session_core_1__session,
   const constants__t_sessionState session_core_1__state,
   const t_bool session_core_1__is_client) {
   session_core_1__l_set_session_state(session_core_1__session,
      session_core_1__state,
      session_core_1__is_client);
}

void session_core_1__set_session_state_closed(
   const constants__t_session_i session_core_1__session,
   const constants_statuscodes_bs__t_StatusCode_i session_core_1__sc_reason,
   const t_bool session_core_1__is_client) {
   {
      constants__t_sessionState session_core_1__l_prec_state;
      
      session_core_2__get_session_state(session_core_1__session,
         &session_core_1__l_prec_state);
      session_core_bs__notify_set_session_state(session_core_1__session,
         session_core_1__l_prec_state,
         constants__e_session_closed,
         session_core_1__sc_reason,
         session_core_1__is_client);
      session_core_2__reset_session_channel(session_core_1__session);
      session_core_bs__delete_session_token(session_core_1__session,
         session_core_1__is_client);
      session_core_2__reset_session_to_create(session_core_1__session);
      session_core_bs__delete_session_application_context(session_core_1__session);
      session_core_2__reset_session_orphaned(session_core_1__session);
      session_core_1__l_reset_server_session_preferred_locales(session_core_1__session);
      session_core_bs__remove_NonceServer(session_core_1__session,
         session_core_1__is_client);
      session_core_2__remove_session(session_core_1__session);
      if (session_core_1__is_client == false) {
         session_core_bs__server_session_timeout_stop_timer(session_core_1__session);
         session_core_bs__drop_user_server(session_core_1__session);
      }
      else {
         session_core_bs__drop_NonceClient(session_core_1__session);
      }
   }
}

void session_core_1__set_session_orphaned(
   const constants__t_session_i session_core_1__session,
   const constants__t_channel_config_idx_i session_core_1__channel_config_idx) {
   {
      t_bool session_core_1__l_bool;
      
      session_core_2__reset_session_channel(session_core_1__session);
      constants__is_t_channel_config_idx(session_core_1__channel_config_idx,
         &session_core_1__l_bool);
      if (session_core_1__l_bool == true) {
         session_core_2__set_session_orphaned_1(session_core_1__session,
            session_core_1__channel_config_idx);
      }
   }
}

void session_core_1__set_server_session_preferred_locales_or_indet(
   const constants__t_session_i session_core_1__p_session,
   const constants__t_LocaleIds_i session_core_1__p_localesIds) {
   {
      constants__t_LocaleIds_i session_core_1__l_old_localeIds;
      
      if (session_core_1__p_localesIds != constants__c_LocaleIds_indet) {
         session_core_2__reset_server_session_preferred_locales(session_core_1__p_session,
            &session_core_1__l_old_localeIds);
         if (session_core_1__l_old_localeIds != constants__c_LocaleIds_indet) {
            constants__free_LocaleIds(session_core_1__l_old_localeIds);
         }
         session_core_2__set_server_session_preferred_locales(session_core_1__p_session,
            session_core_1__p_localesIds);
      }
   }
}

