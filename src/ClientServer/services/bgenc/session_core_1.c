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

 Date                 : 01/12/2025 10:31:54

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
   const constants__t_sessionState_i session_core_1__p_state) {
   {
      constants__t_sessionState_i session_core_1__l_prec_state;
      t_bool session_core_1__l_is_client;
      
      session_core_2__get_session_state(session_core_1__p_session,
         &session_core_1__l_prec_state);
      session_core_2__set_session_state_1(session_core_1__p_session,
         session_core_1__p_state);
      session_core_2__is_client_session(session_core_1__p_session,
         &session_core_1__l_is_client);
      session_core_bs__notify_set_session_state(session_core_1__p_session,
         session_core_1__l_prec_state,
         session_core_1__p_state,
         constants_statuscodes_bs__c_StatusCode_indet,
         session_core_1__l_is_client);
   }
}

void session_core_1__l_delete_session_roles(
   const constants__t_session_i session_core_1__p_session) {
   {
      constants__t_sessionRoles_i session_core_1__l_old_roles;
      
      session_core_2__reset_server_session_roles(session_core_1__p_session,
         &session_core_1__l_old_roles);
      constants__free_roles(session_core_1__l_old_roles);
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

void session_core_1__l_reset_server_client_create_session_info(
   const constants__t_session_i session_core_1__p_session) {
   {
      constants__t_ApplicationDescription_i session_core_1__l_cli_app_desc;
      constants__t_CertThumbprint_i session_core_1__l_cli_cert_tb;
      
      session_core_2__reset_server_client_create_session_info(session_core_1__p_session,
         &session_core_1__l_cli_app_desc,
         &session_core_1__l_cli_cert_tb);
      constants__free_ApplicationDescription(session_core_1__l_cli_app_desc);
      constants__free_CertThumbprint(session_core_1__l_cli_cert_tb);
   }
}

void session_core_1__init_new_session(
   const t_bool session_core_1__is_client,
   constants__t_session_i * const session_core_1__p_session) {
   {
      t_bool session_core_1__l_is_session;
      t_bool session_core_1__l_continue;
      constants__t_timeref_i session_core_1__l_current_time;
      
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
         time_reference_bs__get_current_TimeReference(&session_core_1__l_current_time);
         session_core_2__add_session(*session_core_1__p_session,
            session_core_1__is_client,
            session_core_1__l_current_time);
         session_core_bs__notify_set_session_state(*session_core_1__p_session,
            constants__e_session_closed,
            constants__e_session_init,
            constants_statuscodes_bs__c_StatusCode_indet,
            session_core_1__is_client);
      }
   }
}

void session_core_1__set_session_state(
   const constants__t_session_i session_core_1__session,
   const constants__t_sessionState_i session_core_1__state) {
   session_core_1__l_set_session_state(session_core_1__session,
      session_core_1__state);
}

void session_core_1__create_session(
   const constants__t_session_i session_core_1__session,
   const constants__t_channel_i session_core_1__channel,
   const constants__t_sessionState_i session_core_1__state) {
   {
      t_bool session_core_1__l_is_client;
      
      session_core_2__set_session_channel(session_core_1__session,
         session_core_1__channel);
      session_core_1__l_set_session_state(session_core_1__session,
         session_core_1__state);
      session_core_2__is_client_session(session_core_1__session,
         &session_core_1__l_is_client);
      if (session_core_1__l_is_client == false) {
         session_core_2__set_server_session_user_auth_attempts(session_core_1__session,
            0);
      }
   }
}

void session_core_1__check_server_session_user_auth_attempts(
   const constants__t_session_i session_core_1__p_session,
   const t_bool session_core_1__p_success,
   t_bool * const session_core_1__p_max_reached) {
   {
      t_entier4 session_core_1__l_attempts;
      
      if (session_core_1__p_success == true) {
         *session_core_1__p_max_reached = false;
      }
      else {
         session_core_2__get_server_session_user_auth_attempts(session_core_1__p_session,
            &session_core_1__l_attempts);
         session_core_1__l_attempts = session_core_1__l_attempts +
            1;
         session_core_2__set_server_session_user_auth_attempts(session_core_1__p_session,
            session_core_1__l_attempts);
         *session_core_1__p_max_reached = (session_core_1__l_attempts >= constants__k_n_UserAuthAttempts_max);
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

void session_core_1__set_session_roles(
   const constants__t_session_i session_core_1__p_session,
   const constants__t_sessionRoles_i session_core_1__p_roles) {
   session_core_1__l_delete_session_roles(session_core_1__p_session);
   session_core_2__set_session_roles_2(session_core_1__p_session,
      session_core_1__p_roles);
}

void session_core_1__set_session_state_closed(
   const constants__t_session_i session_core_1__session,
   const constants_statuscodes_bs__t_StatusCode_i session_core_1__sc_reason) {
   {
      constants__t_sessionState_i session_core_1__l_prec_state;
      t_bool session_core_1__l_is_client;
      
      session_core_2__get_session_state(session_core_1__session,
         &session_core_1__l_prec_state);
      session_core_2__is_client_session(session_core_1__session,
         &session_core_1__l_is_client);
      session_core_bs__notify_set_session_state(session_core_1__session,
         session_core_1__l_prec_state,
         constants__e_session_closed,
         session_core_1__sc_reason,
         session_core_1__l_is_client);
      session_core_2__reset_session_channel(session_core_1__session);
      session_core_1__l_delete_session_roles(session_core_1__session);
      session_core_bs__delete_session_token(session_core_1__session,
         session_core_1__l_is_client);
      session_core_2__reset_session_to_create(session_core_1__session);
      session_core_bs__delete_session_application_context(session_core_1__session);
      session_core_2__reset_session_orphaned(session_core_1__session);
      session_core_1__l_reset_server_session_preferred_locales(session_core_1__session);
      session_core_1__l_reset_server_client_create_session_info(session_core_1__session);
      session_core_bs__remove_NonceServer(session_core_1__session,
         session_core_1__l_is_client);
      session_core_2__remove_session(session_core_1__session);
      if (session_core_1__l_is_client == false) {
         session_core_bs__server_session_timeout_stop_timer(session_core_1__session);
         session_core_bs__drop_user_server(session_core_1__session);
         session_core_2__reset_server_session_user_auth_attempts(session_core_1__session);
      }
      else {
         session_core_bs__drop_NonceClient(session_core_1__session);
      }
   }
}

void session_core_1__set_server_client_create_session_info(
   const constants__t_session_i session_core_1__p_session,
   const constants__t_ApplicationDescription_i session_core_1__p_cliAppDesc,
   const constants__t_CertThumbprint_i session_core_1__p_cliCertTb) {
   if (session_core_1__p_cliAppDesc != constants__c_ApplicationDescription_indet) {
      session_core_2__set_server_session_client_app_desc(session_core_1__p_session,
         session_core_1__p_cliAppDesc);
   }
   if (session_core_1__p_cliCertTb != constants__c_CertThumbprint_indet) {
      session_core_2__set_server_session_client_cert_tb(session_core_1__p_session,
         session_core_1__p_cliCertTb);
   }
}

void session_core_1__find_session_to_close(
   t_bool * const session_core_1__p_has_session_to_close,
   constants__t_session_i * const session_core_1__p_session_to_close) {
   {
      t_bool session_core_1__l_continue;
      constants__t_session_i session_core_1__l_session;
      t_bool session_core_1__l_valid_session;
      constants__t_sessionState_i session_core_1__l_state;
      constants__t_timeref_i session_core_1__l_timeref;
      t_bool session_core_1__l_is_older_than;
      constants__t_session_i session_core_1__l_oldest_session;
      constants__t_timeref_i session_core_1__l_oldest_session_timeref;
      constants__t_timeref_i session_core_1__l_min_timeref_req;
      constants__t_timeref_i session_core_1__l_current_timeref;
      
      session_core_1__l_oldest_session = constants__c_session_indet;
      session_core_1__l_oldest_session_timeref = constants__c_timeref_indet;
      session_core_1_it__init_iter_session(&session_core_1__l_continue);
      while (session_core_1__l_continue == true) {
         session_core_1_it__continue_iter_session(&session_core_1__l_continue,
            &session_core_1__l_session);
         session_core_2__is_valid_session(session_core_1__l_session,
            &session_core_1__l_valid_session);
         if (session_core_1__l_valid_session == true) {
            session_core_2__get_session_state(session_core_1__l_session,
               &session_core_1__l_state);
            if (session_core_1__l_state == constants__e_session_created) {
               session_core_2__get_init_time(session_core_1__l_session,
                  &session_core_1__l_timeref);
               if (session_core_1__l_oldest_session_timeref == constants__c_timeref_indet) {
                  time_reference_bs__add_delay_TimeReference(session_core_1__l_timeref,
                     constants__c_session_activation_min_delay,
                     &session_core_1__l_min_timeref_req);
                  time_reference_bs__get_current_TimeReference(&session_core_1__l_current_timeref);
                  time_reference_bs__is_less_than_TimeReference(session_core_1__l_min_timeref_req,
                     session_core_1__l_current_timeref,
                     &session_core_1__l_is_older_than);
                  if (session_core_1__l_is_older_than == true) {
                     session_core_1__l_oldest_session_timeref = session_core_1__l_timeref;
                     session_core_1__l_oldest_session = session_core_1__l_session;
                  }
               }
               else {
                  time_reference_bs__is_less_than_TimeReference(session_core_1__l_timeref,
                     session_core_1__l_oldest_session_timeref,
                     &session_core_1__l_is_older_than);
                  if (session_core_1__l_is_older_than == true) {
                     session_core_1__l_oldest_session_timeref = session_core_1__l_timeref;
                     session_core_1__l_oldest_session = session_core_1__l_session;
                  }
               }
            }
         }
      }
      if (session_core_1__l_oldest_session == constants__c_session_indet) {
         *session_core_1__p_has_session_to_close = false;
         *session_core_1__p_session_to_close = constants__c_session_indet;
      }
      else {
         *session_core_1__p_has_session_to_close = true;
         *session_core_1__p_session_to_close = session_core_1__l_oldest_session;
      }
   }
}

void session_core_1__is_auto_close_session_active(
   t_bool * const session_core_1__p_auto_closed_active) {
   {
      t_entier4 session_core_1__l_t_session_card;
      t_entier4 session_core_1__l_s_session_card;
      t_entier4 session_core_1__l_avail_session_card;
      
      constants__get_card_t_session(&session_core_1__l_t_session_card);
      session_core_2__get_card_s_session(&session_core_1__l_s_session_card);
      session_core_1__l_avail_session_card = session_core_1__l_t_session_card -
         session_core_1__l_s_session_card;
      *session_core_1__p_auto_closed_active = (session_core_1__l_avail_session_card <= 1);
   }
}

