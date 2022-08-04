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

 Date                 : 04/08/2022 14:53:18

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "session_core_2.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
constants__t_channel_i session_core_2__a_channel_i[constants__t_session_i_max+1];
constants__t_channel_config_idx_i session_core_2__a_client_orphaned_i[constants__t_session_i_max+1];
constants__t_channel_config_idx_i session_core_2__a_client_to_create_i[constants__t_session_i_max+1];
constants__t_session_i session_core_2__a_reverse_channel_i[constants__t_channel_i_max+1];
constants__t_LocaleIds_i session_core_2__a_server_client_locales_i[constants__t_session_i_max+1];
constants__t_sessionState session_core_2__a_state_i[constants__t_session_i_max+1];
t_bool session_core_2__s_session_i[constants__t_session_i_max+1];

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void session_core_2__INITIALISATION(void) {
   {
      t_entier4 i;
      for (i = constants__t_session_i_max; 0 <= i; i = i - 1) {
         session_core_2__s_session_i[i] = false;
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_session_i_max; 0 <= i; i = i - 1) {
         session_core_2__a_state_i[i] = constants__e_session_closed;
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_session_i_max; 0 <= i; i = i - 1) {
         session_core_2__a_channel_i[i] = constants__c_channel_indet;
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_channel_i_max; 0 <= i; i = i - 1) {
         session_core_2__a_reverse_channel_i[i] = constants__c_session_indet;
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_session_i_max; 0 <= i; i = i - 1) {
         session_core_2__a_client_to_create_i[i] = constants__c_channel_config_idx_indet;
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_session_i_max; 0 <= i; i = i - 1) {
         session_core_2__a_client_orphaned_i[i] = constants__c_channel_config_idx_indet;
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_session_i_max; 0 <= i; i = i - 1) {
         session_core_2__a_server_client_locales_i[i] = constants__c_LocaleIds_empty;
      }
   }
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void session_core_2__add_session(
   const constants__t_session_i session_core_2__p_session,
   const constants__t_sessionState session_core_2__p_state) {
   session_core_2__s_session_i[session_core_2__p_session] = true;
   session_core_2__a_state_i[session_core_2__p_session] = session_core_2__p_state;
}

void session_core_2__remove_session(
   const constants__t_session_i session_core_2__p_session) {
   session_core_2__s_session_i[session_core_2__p_session] = false;
}

void session_core_2__reset_session_channel(
   const constants__t_session_i session_core_2__p_session) {
   session_core_2__a_reverse_channel_i[session_core_2__a_channel_i[session_core_2__p_session]] = constants__c_session_indet;
   session_core_2__a_channel_i[session_core_2__p_session] = constants__c_channel_indet;
}

void session_core_2__reset_session_to_create(
   const constants__t_session_i session_core_2__p_session) {
   session_core_2__a_client_to_create_i[session_core_2__p_session] = constants__c_channel_config_idx_indet;
}

void session_core_2__reset_session_orphaned(
   const constants__t_session_i session_core_2__p_session) {
   session_core_2__a_client_orphaned_i[session_core_2__p_session] = constants__c_channel_config_idx_indet;
}

void session_core_2__is_valid_session(
   const constants__t_session_i session_core_2__session,
   t_bool * const session_core_2__ret) {
   *session_core_2__ret = session_core_2__s_session_i[session_core_2__session];
}

void session_core_2__get_session_state(
   const constants__t_session_i session_core_2__session,
   constants__t_sessionState * const session_core_2__state) {
   *session_core_2__state = session_core_2__a_state_i[session_core_2__session];
}

void session_core_2__set_session_state_1(
   const constants__t_session_i session_core_2__p_session,
   const constants__t_sessionState session_core_2__p_state) {
   session_core_2__a_state_i[session_core_2__p_session] = session_core_2__p_state;
}

void session_core_2__set_session_channel(
   const constants__t_session_i session_core_2__session,
   const constants__t_channel_i session_core_2__channel) {
   session_core_2__a_reverse_channel_i[session_core_2__a_channel_i[session_core_2__session]] = constants__c_session_indet;
   session_core_2__a_channel_i[session_core_2__session] = session_core_2__channel;
   session_core_2__a_reverse_channel_i[session_core_2__channel] = session_core_2__session;
}

void session_core_2__getall_session_channel(
   const constants__t_session_i session_core_2__p_session,
   t_bool * const session_core_2__p_dom,
   constants__t_channel_i * const session_core_2__p_channel) {
   *session_core_2__p_channel = session_core_2__a_channel_i[session_core_2__p_session];
   constants__is_t_channel(*session_core_2__p_channel,
      session_core_2__p_dom);
}

void session_core_2__get_session_channel(
   const constants__t_session_i session_core_2__session,
   constants__t_channel_i * const session_core_2__channel) {
   *session_core_2__channel = session_core_2__a_channel_i[session_core_2__session];
}

void session_core_2__get_channel_session(
   const constants__t_channel_i session_core_2__p_channel,
   constants__t_session_i * const session_core_2__p_session) {
   *session_core_2__p_session = session_core_2__a_reverse_channel_i[session_core_2__p_channel];
}

void session_core_2__getall_to_create(
   const constants__t_session_i session_core_2__p_session,
   t_bool * const session_core_2__p_dom,
   constants__t_channel_config_idx_i * const session_core_2__p_channel_config_idx) {
   *session_core_2__p_channel_config_idx = session_core_2__a_client_to_create_i[session_core_2__p_session];
   constants__is_t_channel_config_idx(*session_core_2__p_channel_config_idx,
      session_core_2__p_dom);
}

void session_core_2__getall_orphaned(
   const constants__t_session_i session_core_2__p_session,
   t_bool * const session_core_2__p_dom,
   constants__t_channel_config_idx_i * const session_core_2__p_channel_config_idx) {
   *session_core_2__p_channel_config_idx = session_core_2__a_client_orphaned_i[session_core_2__p_session];
   constants__is_t_channel_config_idx(*session_core_2__p_channel_config_idx,
      session_core_2__p_dom);
}

void session_core_2__set_session_to_create(
   const constants__t_session_i session_core_2__p_session,
   const constants__t_channel_config_idx_i session_core_2__p_channel_config_idx) {
   session_core_2__a_client_to_create_i[session_core_2__p_session] = session_core_2__p_channel_config_idx;
}

void session_core_2__set_session_orphaned_1(
   const constants__t_session_i session_core_2__p_session,
   const constants__t_channel_config_idx_i session_core_2__p_channel_config_idx) {
   session_core_2__a_client_orphaned_i[session_core_2__p_session] = session_core_2__p_channel_config_idx;
}

void session_core_2__set_server_session_preferred_locales(
   const constants__t_session_i session_core_2__p_session,
   const constants__t_LocaleIds_i session_core_2__p_localesIds) {
   session_core_2__a_server_client_locales_i[session_core_2__p_session] = session_core_2__p_localesIds;
}

void session_core_2__get_server_session_preferred_locales(
   const constants__t_session_i session_core_2__p_session,
   constants__t_LocaleIds_i * const session_core_2__p_localeIds) {
   *session_core_2__p_localeIds = session_core_2__a_server_client_locales_i[session_core_2__p_session];
}

void session_core_2__reset_server_session_preferred_locales(
   const constants__t_session_i session_core_2__p_session,
   constants__t_LocaleIds_i * const session_core_2__p_localeIds) {
   {
      constants__t_LocaleIds_i session_core_2__l_old_localeIds;
      
      session_core_2__l_old_localeIds = session_core_2__a_server_client_locales_i[session_core_2__p_session];
      if (session_core_2__l_old_localeIds == constants__c_LocaleIds_empty) {
         *session_core_2__p_localeIds = constants__c_LocaleIds_indet;
      }
      else {
         *session_core_2__p_localeIds = session_core_2__l_old_localeIds;
      }
      session_core_2__a_server_client_locales_i[session_core_2__p_session] = constants__c_LocaleIds_empty;
   }
}

