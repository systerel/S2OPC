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

 File Name            : session_core_3.c

 Date                 : 17/11/2026 15:32:45

 C Translator Version : tradc Java V2.0 (2026-09-14)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "session_core_3.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
constants__t_channel_i session_core_3__a_channel_i[constants__t_session_i_max+1];
t_entier4 session_core_3__a_channel_nb_sessions_i[constants__t_channel_i_max+1];
constants__t_channel_config_idx_i session_core_3__a_client_orphaned_i[constants__t_session_i_max+1];
t_bool session_core_3__a_client_session_i[constants__t_session_i_max+1];
constants__t_channel_config_idx_i session_core_3__a_client_to_create_i[constants__t_session_i_max+1];
constants__t_ApplicationDescription_i session_core_3__a_server_client_app_desc_i[constants__t_session_i_max+1];
constants__t_CertThumbprint_i session_core_3__a_server_client_cert_tb_i[constants__t_session_i_max+1];
constants__t_LocaleIds_i session_core_3__a_server_client_locales_i[constants__t_session_i_max+1];
constants__t_SessionName_i session_core_3__a_server_session_name_i[constants__t_session_i_max+1];
t_entier4 session_core_3__a_server_user_auth_attempts_i[constants__t_session_i_max+1];
constants__t_timeref_i session_core_3__a_session_init_time_i[constants__t_session_i_max+1];
constants__t_sessionRoles_i session_core_3__a_session_roles_i[constants__t_session_i_max+1];
constants__t_sessionState_i session_core_3__a_state_i[constants__t_session_i_max+1];
t_entier4 session_core_3__card_s_session_i;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void session_core_3__INITIALISATION(void) {
   {
      t_entier4 i;
      for (i = constants__t_session_i_max; 0 <= i; i = i - 1) {
         session_core_3__a_session_roles_i[i] = constants__c_sessionRoles_indet;
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_session_i_max; 0 <= i; i = i - 1) {
         session_core_3__a_client_session_i[i] = false;
      }
   }
   session_core_3__card_s_session_i = 0;
   {
      t_entier4 i;
      for (i = constants__t_session_i_max; 0 <= i; i = i - 1) {
         session_core_3__a_state_i[i] = constants__e_session_closed;
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_session_i_max; 0 <= i; i = i - 1) {
         session_core_3__a_session_init_time_i[i] = constants__c_timeref_indet;
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_session_i_max; 0 <= i; i = i - 1) {
         session_core_3__a_channel_i[i] = constants__c_channel_indet;
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_channel_i_max; 0 <= i; i = i - 1) {
         session_core_3__a_channel_nb_sessions_i[i] = 0;
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_session_i_max; 0 <= i; i = i - 1) {
         session_core_3__a_client_to_create_i[i] = constants__c_channel_config_idx_indet;
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_session_i_max; 0 <= i; i = i - 1) {
         session_core_3__a_client_orphaned_i[i] = constants__c_channel_config_idx_indet;
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_session_i_max; 0 <= i; i = i - 1) {
         session_core_3__a_server_user_auth_attempts_i[i] = 0;
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_session_i_max; 0 <= i; i = i - 1) {
         session_core_3__a_server_client_locales_i[i] = constants__c_LocaleIds_empty;
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_session_i_max; 0 <= i; i = i - 1) {
         session_core_3__a_server_client_app_desc_i[i] = constants__c_ApplicationDescription_indet;
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_session_i_max; 0 <= i; i = i - 1) {
         session_core_3__a_server_client_cert_tb_i[i] = constants__c_CertThumbprint_indet;
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_session_i_max; 0 <= i; i = i - 1) {
         session_core_3__a_server_session_name_i[i] = constants__c_SessionName_indet;
      }
   }
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void session_core_3__set_session_roles_2(
   const constants__t_session_i session_core_3__p_session,
   const constants__t_sessionRoles_i session_core_3__p_roles) {
   session_core_3__a_session_roles_i[session_core_3__p_session] = session_core_3__p_roles;
}

void session_core_3__set_client_session(
   const constants__t_session_i session_core_3__p_session,
   const t_bool session_core_3__p_is_client) {
   session_core_3__a_client_session_i[session_core_3__p_session] = session_core_3__p_is_client;
}

void session_core_3__increase_card_s_session(void) {
   session_core_3__card_s_session_i = session_core_3__card_s_session_i +
      1;
}

void session_core_3__decrease_card_s_session(void) {
   session_core_3__card_s_session_i = session_core_3__card_s_session_i -
      1;
}

void session_core_3__reset_session_to_create(
   const constants__t_session_i session_core_3__p_session) {
   session_core_3__a_client_to_create_i[session_core_3__p_session] = constants__c_channel_config_idx_indet;
}

void session_core_3__reset_session_orphaned(
   const constants__t_session_i session_core_3__p_session) {
   session_core_3__a_client_orphaned_i[session_core_3__p_session] = constants__c_channel_config_idx_indet;
}

void session_core_3__is_valid_session(
   const constants__t_session_i session_core_3__session,
   t_bool * const session_core_3__ret) {
   {
      constants__t_sessionState_i session_core_3__l_state;
      
      session_core_3__l_state = session_core_3__a_state_i[session_core_3__session];
      *session_core_3__ret = session_core_3__l_state != constants__e_session_closed;
   }
}

void session_core_3__is_client_session(
   const constants__t_session_i session_core_3__p_session,
   t_bool * const session_core_3__ret) {
   *session_core_3__ret = session_core_3__a_client_session_i[session_core_3__p_session];
}

void session_core_3__get_session_state(
   const constants__t_session_i session_core_3__session,
   constants__t_sessionState_i * const session_core_3__state) {
   *session_core_3__state = session_core_3__a_state_i[session_core_3__session];
}

void session_core_3__set_session_state_1(
   const constants__t_session_i session_core_3__p_session,
   const constants__t_sessionState_i session_core_3__p_state) {
   session_core_3__a_state_i[session_core_3__p_session] = session_core_3__p_state;
}

void session_core_3__reset_session_state(
   const constants__t_session_i session_core_3__p_session) {
   session_core_3__a_state_i[session_core_3__p_session] = constants__e_session_closed;
}

void session_core_3__getall_session_channel(
   const constants__t_session_i session_core_3__p_session,
   t_bool * const session_core_3__p_dom,
   constants__t_channel_i * const session_core_3__p_channel) {
   *session_core_3__p_channel = session_core_3__a_channel_i[session_core_3__p_session];
   constants__is_t_channel(*session_core_3__p_channel,
      session_core_3__p_dom);
}

void session_core_3__get_session_channel(
   const constants__t_session_i session_core_3__session,
   constants__t_channel_i * const session_core_3__channel) {
   *session_core_3__channel = session_core_3__a_channel_i[session_core_3__session];
}

void session_core_3__set_channel(
   const constants__t_session_i session_core_3__p_session,
   const constants__t_channel_i session_core_3__p_channel) {
   session_core_3__a_channel_i[session_core_3__p_session] = session_core_3__p_channel;
}

void session_core_3__reset_channel(
   const constants__t_session_i session_core_3__p_session) {
   session_core_3__a_channel_i[session_core_3__p_session] = constants__c_channel_indet;
}

void session_core_3__get_channel_nb_sessions(
   const constants__t_channel_i session_core_3__p_channel,
   t_entier4 * const session_core_3__p_nb_sessions) {
   *session_core_3__p_nb_sessions = session_core_3__a_channel_nb_sessions_i[session_core_3__p_channel];
}

void session_core_3__increase_channel_nb_sessions(
   const constants__t_channel_i session_core_3__p_channel) {
   session_core_3__a_channel_nb_sessions_i[session_core_3__p_channel] = session_core_3__a_channel_nb_sessions_i[session_core_3__p_channel] +
      1;
}

void session_core_3__decrease_channel_nb_sessions(
   const constants__t_channel_i session_core_3__p_channel) {
   session_core_3__a_channel_nb_sessions_i[session_core_3__p_channel] = session_core_3__a_channel_nb_sessions_i[session_core_3__p_channel] -
      1;
}

void session_core_3__getall_to_create(
   const constants__t_session_i session_core_3__p_session,
   t_bool * const session_core_3__p_dom,
   constants__t_channel_config_idx_i * const session_core_3__p_channel_config_idx) {
   *session_core_3__p_channel_config_idx = session_core_3__a_client_to_create_i[session_core_3__p_session];
   constants__is_t_channel_config_idx(*session_core_3__p_channel_config_idx,
      session_core_3__p_dom);
}

void session_core_3__getall_orphaned(
   const constants__t_session_i session_core_3__p_session,
   t_bool * const session_core_3__p_dom,
   constants__t_channel_config_idx_i * const session_core_3__p_channel_config_idx) {
   *session_core_3__p_channel_config_idx = session_core_3__a_client_orphaned_i[session_core_3__p_session];
   constants__is_t_channel_config_idx(*session_core_3__p_channel_config_idx,
      session_core_3__p_dom);
}

void session_core_3__set_session_to_create(
   const constants__t_session_i session_core_3__p_session,
   const constants__t_channel_config_idx_i session_core_3__p_channel_config_idx) {
   session_core_3__a_client_to_create_i[session_core_3__p_session] = session_core_3__p_channel_config_idx;
}

void session_core_3__set_client_orphaned(
   const constants__t_session_i session_core_3__p_session,
   const constants__t_channel_config_idx_i session_core_3__p_channel_config_idx) {
   session_core_3__a_client_orphaned_i[session_core_3__p_session] = session_core_3__p_channel_config_idx;
}

void session_core_3__set_server_session_preferred_locales(
   const constants__t_session_i session_core_3__p_session,
   const constants__t_LocaleIds_i session_core_3__p_localesIds) {
   session_core_3__a_server_client_locales_i[session_core_3__p_session] = session_core_3__p_localesIds;
}

void session_core_3__get_server_session_preferred_locales(
   const constants__t_session_i session_core_3__p_session,
   constants__t_LocaleIds_i * const session_core_3__p_localeIds) {
   *session_core_3__p_localeIds = session_core_3__a_server_client_locales_i[session_core_3__p_session];
}

void session_core_3__set_server_session_client_app_desc(
   const constants__t_session_i session_core_3__p_session,
   const constants__t_ApplicationDescription_i session_core_3__p_cliAppDesc) {
   session_core_3__a_server_client_app_desc_i[session_core_3__p_session] = session_core_3__p_cliAppDesc;
}

void session_core_3__get_server_session_client_app_desc(
   const constants__t_session_i session_core_3__p_session,
   constants__t_ApplicationDescription_i * const session_core_3__p_cliAppDesc) {
   *session_core_3__p_cliAppDesc = session_core_3__a_server_client_app_desc_i[session_core_3__p_session];
}

void session_core_3__set_server_session_client_cert_tb(
   const constants__t_session_i session_core_3__p_session,
   const constants__t_CertThumbprint_i session_core_3__p_cliCertTb) {
   session_core_3__a_server_client_cert_tb_i[session_core_3__p_session] = session_core_3__p_cliCertTb;
}

void session_core_3__get_server_session_client_cert_tb(
   const constants__t_session_i session_core_3__p_session,
   constants__t_CertThumbprint_i * const session_core_3__p_cliCertTb) {
   *session_core_3__p_cliCertTb = session_core_3__a_server_client_cert_tb_i[session_core_3__p_session];
}

void session_core_3__set_server_session_name(
   const constants__t_session_i session_core_3__p_session,
   const constants__t_SessionName_i session_core_3__p_sessionName) {
   session_core_3__a_server_session_name_i[session_core_3__p_session] = session_core_3__p_sessionName;
}

void session_core_3__reset_server_session_roles(
   const constants__t_session_i session_core_3__p_session,
   constants__t_sessionRoles_i * const session_core_3__p_roles) {
   {
      constants__t_sessionRoles_i session_core_3__l_old_roles;
      
      session_core_3__l_old_roles = session_core_3__a_session_roles_i[session_core_3__p_session];
      *session_core_3__p_roles = session_core_3__l_old_roles;
      session_core_3__a_session_roles_i[session_core_3__p_session] = constants__c_sessionRoles_indet;
   }
}

void session_core_3__reset_server_session_preferred_locales(
   const constants__t_session_i session_core_3__p_session,
   constants__t_LocaleIds_i * const session_core_3__p_localeIds) {
   {
      constants__t_LocaleIds_i session_core_3__l_old_localeIds;
      
      session_core_3__l_old_localeIds = session_core_3__a_server_client_locales_i[session_core_3__p_session];
      if (session_core_3__l_old_localeIds == constants__c_LocaleIds_empty) {
         *session_core_3__p_localeIds = constants__c_LocaleIds_indet;
      }
      else {
         *session_core_3__p_localeIds = session_core_3__l_old_localeIds;
      }
      session_core_3__a_server_client_locales_i[session_core_3__p_session] = constants__c_LocaleIds_empty;
   }
}

void session_core_3__reset_server_client_create_session_info(
   const constants__t_session_i session_core_3__p_session,
   constants__t_ApplicationDescription_i * const session_core_3__p_cliAppDesc,
   constants__t_CertThumbprint_i * const session_core_3__p_certTb,
   constants__t_SessionName_i * const session_core_3__p_sessionName) {
   *session_core_3__p_cliAppDesc = session_core_3__a_server_client_app_desc_i[session_core_3__p_session];
   session_core_3__a_server_client_app_desc_i[session_core_3__p_session] = constants__c_ApplicationDescription_indet;
   *session_core_3__p_certTb = session_core_3__a_server_client_cert_tb_i[session_core_3__p_session];
   session_core_3__a_server_client_cert_tb_i[session_core_3__p_session] = constants__c_CertThumbprint_indet;
   *session_core_3__p_sessionName = session_core_3__a_server_session_name_i[session_core_3__p_session];
   session_core_3__a_server_session_name_i[session_core_3__p_session] = constants__c_SessionName_indet;
}

void session_core_3__set_server_session_user_auth_attempts(
   const constants__t_session_i session_core_3__p_session,
   const t_entier4 session_core_3__p_attempts) {
   session_core_3__a_server_user_auth_attempts_i[session_core_3__p_session] = session_core_3__p_attempts;
}

void session_core_3__get_server_session_user_auth_attempts(
   const constants__t_session_i session_core_3__p_session,
   t_entier4 * const session_core_3__p_attempts) {
   *session_core_3__p_attempts = session_core_3__a_server_user_auth_attempts_i[session_core_3__p_session];
}

void session_core_3__reset_server_session_user_auth_attempts(
   const constants__t_session_i session_core_3__p_session) {
   session_core_3__a_server_user_auth_attempts_i[session_core_3__p_session] = 0;
}

void session_core_3__get_init_time(
   const constants__t_session_i session_core_3__p_session,
   constants__t_timeref_i * const session_core_3__p_timeref) {
   *session_core_3__p_timeref = session_core_3__a_session_init_time_i[session_core_3__p_session];
}

void session_core_3__reset_init_time(
   const constants__t_session_i session_core_3__p_session) {
   session_core_3__a_session_init_time_i[session_core_3__p_session] = constants__c_timeref_indet;
}

void session_core_3__set_init_time(
   const constants__t_session_i session_core_3__p_session,
   const constants__t_timeref_i session_core_3__p_init_time) {
   session_core_3__a_session_init_time_i[session_core_3__p_session] = session_core_3__p_init_time;
}

void session_core_3__get_card_s_session(
   t_entier4 * const session_core_3__p_nb_sessions) {
   *session_core_3__p_nb_sessions = session_core_3__card_s_session_i;
}

