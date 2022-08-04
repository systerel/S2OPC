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

 File Name            : session_core.c

 Date                 : 04/08/2022 14:53:18

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "session_core.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void session_core__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void session_core__server_internal_activate_req_and_resp(
   const constants__t_channel_i session_core__channel,
   const constants__t_session_i session_core__session,
   const constants__t_sessionState session_core__transitoryState,
   const constants__t_user_i session_core__p_user,
   const constants__t_msg_i session_core__activate_req_msg,
   const constants__t_msg_i session_core__activate_resp_msg,
   t_bool * const session_core__res_activated) {
   {
      constants__t_channel_config_idx_i session_core__l_channel_config_idx;
      constants__t_SecurityPolicy session_core__l_secpol;
      constants__t_Nonce_i session_core__l_nonce;
      constants__t_LocaleIds_i session_core__l_localeIds;
      
      channel_mgr__get_channel_info(session_core__channel,
         &session_core__l_channel_config_idx);
      channel_mgr__get_SecurityPolicy(session_core__channel,
         &session_core__l_secpol);
      if (session_core__l_secpol != constants__e_secpol_None) {
         session_core_1__server_activate_session_check_crypto(session_core__session,
            session_core__channel,
            session_core__l_channel_config_idx,
            session_core__activate_req_msg,
            session_core__res_activated);
      }
      else {
         *session_core__res_activated = true;
      }
      if (*session_core__res_activated == true) {
         session_core_1__server_set_fresh_nonce(session_core__session,
            session_core__l_channel_config_idx,
            session_core__res_activated,
            &session_core__l_nonce);
         if (*session_core__res_activated == true) {
            msg_session_bs__write_activate_session_resp_nonce(session_core__activate_resp_msg,
               session_core__l_nonce);
         }
      }
      if (*session_core__res_activated == true) {
         session_core_1__set_session_state(session_core__session,
            session_core__transitoryState,
            false);
         session_core_1__drop_user_server(session_core__session);
         session_core_1__set_session_user_server(session_core__session,
            session_core__p_user);
         session_core_1__set_session_channel(session_core__session,
            session_core__channel);
         message_in_bs__read_activate_req_msg_locales(session_core__activate_req_msg,
            &session_core__l_localeIds);
         session_core_1__set_server_session_preferred_locales_or_indet(session_core__session,
            session_core__l_localeIds);
         session_core_1__set_session_state(session_core__session,
            constants__e_session_userActivated,
            false);
      }
   }
}

void session_core__client_init_session_sm(
   constants__t_session_i * const session_core__nsession) {
   session_core_1__init_new_session(true,
      session_core__nsession);
}

void session_core__client_create_session_req_sm(
   const constants__t_session_i session_core__session,
   const constants__t_channel_i session_core__channel,
   const constants__t_msg_i session_core__create_req_msg,
   t_bool * const session_core__valid) {
   {
      constants__t_channel_config_idx_i session_core__l_channel_config_idx;
      t_bool session_core__l_nonce_needed;
      constants__t_Nonce_i session_core__l_nonce;
      constants__t_session_application_context_i session_core__l_app_context;
      
      session_core_1__create_session(session_core__session,
         session_core__channel,
         constants__e_session_creating,
         true);
      channel_mgr__get_channel_info(session_core__channel,
         &session_core__l_channel_config_idx);
      session_core_1__client_create_session_req_do_crypto(session_core__session,
         session_core__channel,
         session_core__l_channel_config_idx,
         session_core__valid,
         &session_core__l_nonce_needed);
      if (*session_core__valid == true) {
         msg_session_bs__write_create_session_req_msg_clientDescription(session_core__create_req_msg,
            session_core__l_channel_config_idx);
         msg_session_bs__write_create_session_req_msg_serverUri(session_core__create_req_msg,
            session_core__l_channel_config_idx);
         msg_session_bs__write_create_session_req_msg_endpointUrl(session_core__create_req_msg,
            session_core__l_channel_config_idx);
         session_core_1__get_session_app_context(session_core__session,
            &session_core__l_app_context);
         msg_session_bs__write_create_session_req_msg_sessionName(session_core__create_req_msg,
            session_core__l_app_context);
         if (session_core__l_nonce_needed == true) {
            session_core_1__get_NonceClient(session_core__session,
               &session_core__l_nonce);
            msg_session_bs__write_create_session_req_msg_crypto(session_core__create_req_msg,
               session_core__l_channel_config_idx,
               session_core__l_nonce,
               session_core__valid);
         }
         msg_session_bs__write_create_session_req_msg_sessionTimeout(session_core__create_req_msg);
         msg_session_bs__write_create_session_req_msg_maxResponseMessageSize(session_core__create_req_msg);
      }
   }
}

void session_core__server_create_session_req_and_resp_sm(
   const constants__t_channel_i session_core__channel,
   const constants__t_msg_i session_core__create_req_msg,
   const constants__t_msg_i session_core__create_resp_msg,
   constants__t_session_i * const session_core__nsession,
   constants_statuscodes_bs__t_StatusCode_i * const session_core__service_ret) {
   {
      constants__t_session_i session_core__l_nsession;
      t_bool session_core__l_valid_session;
      constants__t_channel_config_idx_i session_core__l_channel_config_idx;
      constants__t_session_token_i session_core__l_nsession_token;
      t_bool session_core__l_valid_session_token;
      constants__t_endpoint_config_idx_i session_core__l_endpoint_config_idx;
      t_bool session_core__l_valid_server_cert;
      constants_statuscodes_bs__t_StatusCode_i session_core__l_crypto_status;
      constants__t_SignatureData_i session_core__l_signature;
      constants__t_Nonce_i session_core__l_nonce;
      constants__t_SecurityPolicy session_core__l_secpol;
      t_bool session_core__l_set_cert;
      t_bool session_core__l_bret;
      
      session_core__l_set_cert = false;
      session_core__l_bret = false;
      *session_core__service_ret = constants_statuscodes_bs__e_sc_bad_out_of_memory;
      session_core_1__init_new_session(false,
         &session_core__l_nsession);
      session_core_1__is_valid_session(session_core__l_nsession,
         &session_core__l_valid_session);
      if (session_core__l_valid_session == true) {
         session_core_1__create_session(session_core__l_nsession,
            session_core__channel,
            constants__e_session_created,
            false);
         channel_mgr__get_channel_info(session_core__channel,
            &session_core__l_channel_config_idx);
         msg_session_bs__create_session_req_check_client_certificate(session_core__create_req_msg,
            session_core__l_channel_config_idx,
            &session_core__l_valid_server_cert);
         session_core_1__server_get_fresh_session_token(session_core__l_channel_config_idx,
            session_core__l_nsession,
            &session_core__l_nsession_token);
         session_core_1__server_is_valid_session_token(session_core__l_nsession_token,
            &session_core__l_valid_session_token);
         if ((session_core__l_valid_session_token == true) &&
            (session_core__l_valid_server_cert == true)) {
            channel_mgr__server_get_endpoint_config(session_core__channel,
               &session_core__l_endpoint_config_idx);
            msg_session_bs__write_create_session_msg_session_token(session_core__create_resp_msg,
               session_core__l_nsession,
               session_core__l_nsession_token);
            msg_session_bs__write_create_session_msg_session_revised_timeout(session_core__create_req_msg,
               session_core__create_resp_msg);
            msg_session_bs__write_create_session_resp_msg_maxRequestMessageSize(session_core__create_resp_msg);
            msg_session_bs__write_create_session_msg_server_endpoints(session_core__create_req_msg,
               session_core__create_resp_msg,
               session_core__l_endpoint_config_idx,
               session_core__service_ret);
            if (*session_core__service_ret == constants_statuscodes_bs__e_sc_ok) {
               channel_mgr__get_SecurityPolicy(session_core__channel,
                  &session_core__l_secpol);
               if (session_core__l_secpol != constants__e_secpol_None) {
                  session_core__l_set_cert = true;
                  session_core_1__server_create_session_req_do_crypto(session_core__l_nsession,
                     session_core__create_req_msg,
                     session_core__l_endpoint_config_idx,
                     session_core__l_channel_config_idx,
                     &session_core__l_crypto_status,
                     &session_core__l_signature);
                  if (session_core__l_crypto_status == constants_statuscodes_bs__e_sc_ok) {
                     msg_session_bs__write_create_session_resp_signature(session_core__create_resp_msg,
                        session_core__l_signature,
                        &session_core__l_bret);
                     session_core_1__clear_Signature(session_core__l_nsession,
                        false,
                        session_core__l_signature);
                     if (session_core__l_bret == false) {
                        *session_core__service_ret = constants_statuscodes_bs__e_sc_bad_unexpected_error;
                     }
                  }
                  else {
                     *session_core__service_ret = session_core__l_crypto_status;
                  }
               }
               if (*session_core__service_ret == constants_statuscodes_bs__e_sc_ok) {
                  if (session_core__l_set_cert == false) {
                     channel_mgr__server_get_endpoint_config(session_core__channel,
                        &session_core__l_endpoint_config_idx);
                     session_core_1__server_may_need_user_token_encryption(session_core__l_endpoint_config_idx,
                        session_core__l_channel_config_idx,
                        &session_core__l_set_cert);
                  }
                  if (session_core__l_set_cert == true) {
                     msg_session_bs__write_create_session_resp_cert(session_core__create_resp_msg,
                        session_core__l_channel_config_idx,
                        &session_core__l_bret);
                  }
                  else {
                     session_core__l_bret = true;
                  }
                  if (session_core__l_bret == true) {
                     session_core_1__server_set_fresh_nonce(session_core__l_nsession,
                        session_core__l_channel_config_idx,
                        &session_core__l_bret,
                        &session_core__l_nonce);
                     if (session_core__l_bret == true) {
                        msg_session_bs__write_create_session_resp_nonce(session_core__create_resp_msg,
                           session_core__l_nonce);
                     }
                  }
                  if (session_core__l_bret == false) {
                     *session_core__service_ret = constants_statuscodes_bs__e_sc_bad_unexpected_error;
                  }
               }
            }
            if (*session_core__service_ret == constants_statuscodes_bs__e_sc_ok) {
               msg_session_bs__create_session_req_export_maxResponseMessageSize(session_core__l_channel_config_idx,
                  session_core__create_req_msg);
            }
         }
         else {
            if (session_core__l_valid_server_cert == true) {
               *session_core__service_ret = constants_statuscodes_bs__e_sc_bad_too_many_sessions;
            }
            else {
               *session_core__service_ret = constants_statuscodes_bs__e_sc_bad_security_checks_failed;
            }
            session_core_1__set_session_state_closed(session_core__l_nsession,
               constants_statuscodes_bs__e_sc_bad_session_id_invalid,
               false);
            session_core__l_nsession = constants__c_session_indet;
         }
      }
      else {
         *session_core__service_ret = constants_statuscodes_bs__e_sc_bad_too_many_sessions;
      }
      *session_core__nsession = session_core__l_nsession;
   }
}

void session_core__client_create_session_resp_sm(
   const constants__t_channel_i session_core__channel,
   const constants__t_session_i session_core__session,
   const constants__t_session_token_i session_core__session_token,
   const constants__t_msg_i session_core__create_resp_msg,
   t_bool * const session_core__bret) {
   {
      t_bool session_core__l_valid_user_secu_properties;
      t_bool session_core__l_endpoints_bres;
      t_bool session_core__l_valid_server_cert;
      constants__t_channel_config_idx_i session_core__l_channel_config_idx;
      constants__t_SecurityPolicy session_core__l_secpol;
      
      channel_mgr__get_SecurityPolicy(session_core__channel,
         &session_core__l_secpol);
      channel_mgr__get_channel_info(session_core__channel,
         &session_core__l_channel_config_idx);
      msg_session_bs__create_session_resp_check_server_certificate(session_core__create_resp_msg,
         session_core__l_channel_config_idx,
         &session_core__l_valid_server_cert);
      msg_session_bs__create_session_resp_check_server_endpoints(session_core__create_resp_msg,
         session_core__l_channel_config_idx,
         &session_core__l_endpoints_bres);
      session_core_1__client_create_session_set_user_token_secu_properties(session_core__session,
         session_core__l_channel_config_idx,
         session_core__create_resp_msg,
         &session_core__l_valid_user_secu_properties);
      *session_core__bret = (((session_core__l_valid_server_cert == true) &&
         (session_core__l_endpoints_bres == true)) &&
         (session_core__l_valid_user_secu_properties == true));
      if (*session_core__bret == true) {
         if (session_core__l_secpol != constants__e_secpol_None) {
            session_core_1__client_create_session_check_crypto(session_core__session,
               session_core__l_channel_config_idx,
               session_core__create_resp_msg,
               session_core__bret);
            if (*session_core__bret == true) {
               session_core_1__drop_NonceClient(session_core__session);
            }
         }
         else {
            session_core_1__client_set_NonceServer(session_core__session,
               session_core__create_resp_msg);
         }
      }
      if (*session_core__bret == true) {
         session_core_1__set_session_state(session_core__session,
            constants__e_session_created,
            true);
         session_core_1__client_set_session_token(session_core__session,
            session_core__session_token);
         msg_session_bs__create_session_resp_export_maxRequestMessageSize(session_core__l_channel_config_idx,
            session_core__create_resp_msg);
      }
   }
}

void session_core__client_user_activate_session_req_sm(
   const constants__t_session_i session_core__session,
   const constants__t_user_token_i session_core__p_user_token,
   const constants__t_msg_i session_core__activate_req_msg,
   constants_statuscodes_bs__t_StatusCode_i * const session_core__ret,
   constants__t_channel_i * const session_core__channel,
   constants__t_session_token_i * const session_core__session_token) {
   {
      t_bool session_core__l_is_connected_channel;
      constants__t_channel_config_idx_i session_core__l_channel_config_idx;
      constants__t_SecurityPolicy session_core__l_secpol;
      t_bool session_core__l_valid_crypto;
      constants__t_SignatureData_i session_core__l_signature;
      constants__t_Nonce_i session_core__l_server_nonce;
      constants__t_SecurityPolicy session_core__l_user_secu_policy;
      constants__t_byte_buffer_i session_core__l_user_server_cert;
      constants__t_user_token_i session_core__l_encrypted_user_token;
      t_bool session_core__l_valid_cert;
      t_bool session_core__l_valid_encrypt;
      t_bool session_core__l_bret;
      
      session_core_1__get_session_channel(session_core__session,
         session_core__channel);
      channel_mgr__is_connected_channel(*session_core__channel,
         &session_core__l_is_connected_channel);
      if (session_core__l_is_connected_channel == true) {
         session_core_1__client_get_token_from_session(session_core__session,
            session_core__session_token);
         channel_mgr__get_channel_info(*session_core__channel,
            &session_core__l_channel_config_idx);
         session_core_1__get_NonceServer(session_core__session,
            true,
            &session_core__l_server_nonce);
         session_core_1__get_session_user_secu_client(session_core__session,
            &session_core__l_user_secu_policy);
         session_core_1__get_session_user_server_certificate(session_core__session,
            &session_core__l_user_server_cert);
         if (session_core__l_user_secu_policy == constants__e_secpol_None) {
            session_core__l_valid_cert = true;
         }
         else {
            session_core_1__may_validate_server_certificate(session_core__session,
               session_core__l_channel_config_idx,
               session_core__l_user_server_cert,
               session_core__l_user_secu_policy,
               &session_core__l_valid_cert);
         }
         if (session_core__l_valid_cert == true) {
            user_authentication__may_encrypt_user_token(session_core__l_channel_config_idx,
               session_core__l_user_server_cert,
               session_core__l_server_nonce,
               session_core__l_user_secu_policy,
               session_core__p_user_token,
               &session_core__l_valid_encrypt,
               &session_core__l_encrypted_user_token);
         }
         else {
            session_core__l_encrypted_user_token = constants__c_user_token_indet;
            session_core__l_valid_encrypt = false;
         }
         if ((session_core__l_valid_cert == true) &&
            (session_core__l_valid_encrypt == true)) {
            msg_session_bs__write_activate_msg_user(session_core__activate_req_msg,
               session_core__l_encrypted_user_token);
            channel_mgr__get_SecurityPolicy(*session_core__channel,
               &session_core__l_secpol);
            if (session_core__l_secpol != constants__e_secpol_None) {
               session_core_1__client_activate_session_req_do_crypto(session_core__session,
                  session_core__l_channel_config_idx,
                  session_core__l_server_nonce,
                  &session_core__l_valid_crypto,
                  &session_core__l_signature);
               if (session_core__l_valid_crypto == true) {
                  msg_session_bs__write_activate_session_req_msg_crypto(session_core__activate_req_msg,
                     session_core__l_signature,
                     &session_core__l_bret);
                  session_core_1__clear_Signature(session_core__session,
                     true,
                     session_core__l_signature);
                  if (session_core__l_bret == true) {
                     msg_session_bs__write_activate_req_msg_locales(session_core__activate_req_msg,
                        session_core__l_channel_config_idx);
                     *session_core__ret = constants_statuscodes_bs__e_sc_ok;
                  }
                  else {
                     *session_core__ret = constants_statuscodes_bs__e_sc_bad_unexpected_error;
                  }
               }
               else {
                  *session_core__ret = constants_statuscodes_bs__e_sc_bad_security_checks_failed;
               }
            }
            else {
               *session_core__ret = constants_statuscodes_bs__e_sc_ok;
            }
         }
         else {
            *session_core__ret = constants_statuscodes_bs__e_sc_bad_security_checks_failed;
         }
      }
      else {
         *session_core__channel = constants__c_channel_indet;
         *session_core__session_token = constants__c_session_token_indet;
         *session_core__ret = constants_statuscodes_bs__e_sc_bad_unexpected_error;
      }
      if (*session_core__ret == constants_statuscodes_bs__e_sc_ok) {
         session_core_1__set_session_state(session_core__session,
            constants__e_session_userActivating,
            true);
      }
   }
}

void session_core__client_sc_activate_session_req_sm(
   const constants__t_session_i session_core__session,
   const constants__t_channel_i session_core__channel,
   const constants__t_msg_i session_core__activate_req_msg,
   constants__t_session_token_i * const session_core__session_token) {
   {
      constants__t_user_token_i session_core__l_user_token;
      constants__t_channel_config_idx_i session_core__l_channel_config_idx;
      
      session_core_1__client_get_token_from_session(session_core__session,
         session_core__session_token);
      session_core_1__get_session_user_client(session_core__session,
         &session_core__l_user_token);
      channel_mgr__get_channel_info(session_core__channel,
         &session_core__l_channel_config_idx);
      msg_session_bs__write_activate_req_msg_locales(session_core__activate_req_msg,
         session_core__l_channel_config_idx);
      msg_session_bs__write_activate_msg_user(session_core__activate_req_msg,
         session_core__l_user_token);
      session_core_1__set_session_channel(session_core__session,
         session_core__channel);
      session_core_1__set_session_state(session_core__session,
         constants__e_session_scActivating,
         true);
   }
}

void session_core__allocate_authenticated_user(
   const constants__t_channel_i session_core__p_channel,
   const constants__t_session_i session_core__p_session,
   const constants__t_user_token_i session_core__p_user_token,
   constants_statuscodes_bs__t_StatusCode_i * const session_core__p_sc_valid_user,
   constants__t_user_i * const session_core__p_user) {
   {
      constants__t_channel_config_idx_i session_core__l_channel_config_idx;
      constants__t_endpoint_config_idx_i session_core__l_endpoint_config_idx;
      constants__t_Nonce_i session_core__l_server_nonce;
      
      channel_mgr__get_channel_info(session_core__p_channel,
         &session_core__l_channel_config_idx);
      channel_mgr__server_get_endpoint_config(session_core__p_channel,
         &session_core__l_endpoint_config_idx);
      session_core_1__get_NonceServer(session_core__p_session,
         false,
         &session_core__l_server_nonce);
      user_authentication__allocate_valid_and_authenticated_user(session_core__p_user_token,
         session_core__l_server_nonce,
         session_core__l_channel_config_idx,
         session_core__l_endpoint_config_idx,
         session_core__p_sc_valid_user,
         session_core__p_user);
   }
}

void session_core__server_activate_session_req_and_resp_sm(
   const constants__t_channel_i session_core__channel,
   const constants__t_session_i session_core__session,
   const constants__t_user_i session_core__user,
   const constants__t_msg_i session_core__activate_req_msg,
   const constants__t_msg_i session_core__activate_resp_msg,
   constants_statuscodes_bs__t_StatusCode_i * const session_core__ret) {
   {
      constants__t_channel_i session_core__l_channel;
      constants__t_sessionState session_core__l_state;
      constants__t_user_i session_core__l_user;
      t_bool session_core__l_is_same_user;
      t_bool session_core__l_valid;
      constants_statuscodes_bs__t_StatusCode_i session_core__l_ret;
      
      session_core_1__get_session_channel(session_core__session,
         &session_core__l_channel);
      session_core_1__get_session_state(session_core__session,
         &session_core__l_state);
      if (session_core__l_state == constants__e_session_created) {
         if (session_core__l_channel == session_core__channel) {
            session_core__server_internal_activate_req_and_resp(session_core__channel,
               session_core__session,
               constants__e_session_creating,
               session_core__user,
               session_core__activate_req_msg,
               session_core__activate_resp_msg,
               &session_core__l_valid);
            if (session_core__l_valid == true) {
               session_core__l_ret = constants_statuscodes_bs__e_sc_ok;
            }
            else {
               session_core__l_ret = constants_statuscodes_bs__e_sc_bad_application_signature_invalid;
            }
         }
         else {
            session_core__l_ret = constants_statuscodes_bs__e_sc_bad_invalid_argument;
         }
      }
      else if (session_core__l_state == constants__e_session_userActivated) {
         session_core_1__get_session_user_server(session_core__session,
            &session_core__l_user);
         if (session_core__l_channel == session_core__channel) {
            session_core__server_internal_activate_req_and_resp(session_core__channel,
               session_core__session,
               constants__e_session_userActivating,
               session_core__user,
               session_core__activate_req_msg,
               session_core__activate_resp_msg,
               &session_core__l_valid);
            if (session_core__l_valid == true) {
               session_core__l_ret = constants_statuscodes_bs__e_sc_ok;
            }
            else {
               session_core__l_ret = constants_statuscodes_bs__e_sc_bad_application_signature_invalid;
            }
         }
         else if (session_core__l_channel != session_core__channel) {
            session_core_1__is_same_user_server(session_core__l_user,
               session_core__user,
               &session_core__l_is_same_user);
            if (session_core__l_is_same_user == true) {
               session_core__server_internal_activate_req_and_resp(session_core__channel,
                  session_core__session,
                  constants__e_session_scActivating,
                  session_core__user,
                  session_core__activate_req_msg,
                  session_core__activate_resp_msg,
                  &session_core__l_valid);
               if (session_core__l_valid == true) {
                  session_core__l_ret = constants_statuscodes_bs__e_sc_ok;
               }
               else {
                  session_core__l_ret = constants_statuscodes_bs__e_sc_bad_application_signature_invalid;
               }
            }
            else {
               session_core__l_ret = constants_statuscodes_bs__e_sc_bad_invalid_state;
            }
         }
         else {
            session_core__l_ret = constants_statuscodes_bs__e_sc_bad_invalid_state;
         }
      }
      else if (session_core__l_state == constants__e_session_scOrphaned) {
         session_core_1__get_session_user_server(session_core__session,
            &session_core__l_user);
         session_core_1__is_same_user_server(session_core__l_user,
            session_core__user,
            &session_core__l_is_same_user);
         if ((session_core__l_channel != session_core__channel) &&
            (session_core__l_is_same_user == true)) {
            session_core__server_internal_activate_req_and_resp(session_core__channel,
               session_core__session,
               constants__e_session_scActivating,
               session_core__user,
               session_core__activate_req_msg,
               session_core__activate_resp_msg,
               &session_core__l_valid);
            if (session_core__l_valid == true) {
               session_core__l_ret = constants_statuscodes_bs__e_sc_ok;
            }
            else {
               session_core__l_ret = constants_statuscodes_bs__e_sc_bad_application_signature_invalid;
            }
         }
         else {
            session_core__l_ret = constants_statuscodes_bs__e_sc_bad_invalid_state;
         }
      }
      else {
         session_core__l_ret = constants_statuscodes_bs__e_sc_bad_invalid_state;
      }
      *session_core__ret = session_core__l_ret;
   }
}

void session_core__client_activate_session_resp_sm(
   const constants__t_channel_i session_core__channel,
   const constants__t_session_i session_core__session,
   const constants__t_msg_i session_core__activate_resp_msg,
   t_bool * const session_core__bret) {
   {
      constants__t_channel_config_idx_i session_core__l_channel_config_idx;
      t_bool session_core__l_valid_response;
      
      channel_mgr__get_channel_info(session_core__channel,
         &session_core__l_channel_config_idx);
      session_core_1__client_activate_session_resp_check(session_core__session,
         session_core__l_channel_config_idx,
         session_core__activate_resp_msg,
         &session_core__l_valid_response);
      if (session_core__l_valid_response == true) {
         session_core_1__set_session_state(session_core__session,
            constants__e_session_userActivated,
            true);
         *session_core__bret = true;
      }
      else {
         *session_core__bret = false;
      }
   }
}

void session_core__l_client_secure_channel_lost_session_sm(
   const t_bool session_core__p_dom,
   const constants__t_channel_i session_core__p_channel,
   const constants__t_channel_i session_core__p_lost_channel,
   const constants__t_session_i session_core__p_session,
   const constants__t_channel_config_idx_i session_core__p_channel_config_idx) {
   {
      constants__t_sessionState session_core__l_state;
      
      if ((session_core__p_dom == true) &&
         (session_core__p_channel == session_core__p_lost_channel)) {
         session_core_1__get_session_state(session_core__p_session,
            &session_core__l_state);
         if (session_core__l_state == constants__e_session_userActivated) {
            session_core_1__set_session_orphaned(session_core__p_session,
               session_core__p_channel_config_idx);
            session_core_1__set_session_state(session_core__p_session,
               constants__e_session_scOrphaned,
               true);
         }
         else {
            session_core_1__set_session_state_closed(session_core__p_session,
               constants_statuscodes_bs__e_sc_bad_secure_channel_closed,
               true);
         }
      }
   }
}

void session_core__client_secure_channel_lost_session_sm(
   const constants__t_channel_i session_core__p_lost_channel,
   const constants__t_channel_config_idx_i session_core__p_channel_config_idx) {
   {
      t_bool session_core__l_continue;
      constants__t_session_i session_core__l_session;
      t_bool session_core__l_dom;
      constants__t_channel_i session_core__l_channel;
      
      session_core_it__init_iter_session(&session_core__l_continue);
      if (session_core__l_continue == true) {
         while (session_core__l_continue == true) {
            session_core_it__continue_iter_session(&session_core__l_continue,
               &session_core__l_session);
            session_core_1__getall_session_channel(session_core__l_session,
               &session_core__l_dom,
               &session_core__l_channel);
            session_core__l_client_secure_channel_lost_session_sm(session_core__l_dom,
               session_core__l_channel,
               session_core__p_lost_channel,
               session_core__l_session,
               session_core__p_channel_config_idx);
         }
      }
   }
}

void session_core__l_server_secure_channel_lost_session_sm(
   const t_bool session_core__p_dom,
   const constants__t_channel_i session_core__p_channel,
   const constants__t_channel_i session_core__p_lost_channel,
   const constants__t_session_i session_core__p_session) {
   {
      constants__t_sessionState session_core__l_state;
      
      if ((session_core__p_dom == true) &&
         (session_core__p_channel == session_core__p_lost_channel)) {
         session_core_1__get_session_state(session_core__p_session,
            &session_core__l_state);
         if (session_core__l_state == constants__e_session_userActivated) {
            session_core_1__set_session_orphaned(session_core__p_session,
               constants__c_channel_config_idx_indet);
            session_core_1__set_session_state(session_core__p_session,
               constants__e_session_scOrphaned,
               false);
         }
         else {
            session_core_1__set_session_state_closed(session_core__p_session,
               constants_statuscodes_bs__e_sc_bad_secure_channel_closed,
               false);
         }
      }
   }
}

void session_core__server_secure_channel_lost_session_sm(
   const constants__t_channel_i session_core__p_lost_channel) {
   {
      t_bool session_core__l_continue;
      constants__t_session_i session_core__l_session;
      t_bool session_core__l_dom;
      constants__t_channel_i session_core__l_channel;
      
      session_core_it__init_iter_session(&session_core__l_continue);
      if (session_core__l_continue == true) {
         while (session_core__l_continue == true) {
            session_core_it__continue_iter_session(&session_core__l_continue,
               &session_core__l_session);
            session_core_1__getall_session_channel(session_core__l_session,
               &session_core__l_dom,
               &session_core__l_channel);
            session_core__l_server_secure_channel_lost_session_sm(session_core__l_dom,
               session_core__l_channel,
               session_core__p_lost_channel,
               session_core__l_session);
         }
      }
   }
}

void session_core__client_close_session_req_sm(
   const constants__t_session_i session_core__session,
   const constants__t_msg_i session_core__close_req_msg,
   constants_statuscodes_bs__t_StatusCode_i * const session_core__ret,
   constants__t_channel_i * const session_core__channel,
   constants__t_session_token_i * const session_core__session_token) {
   {
      t_bool session_core__l_is_connected_channel;
      
      session_core_1__client_close_session_req_msg(session_core__close_req_msg);
      session_core_1__get_session_channel(session_core__session,
         session_core__channel);
      channel_mgr__is_connected_channel(*session_core__channel,
         &session_core__l_is_connected_channel);
      if (session_core__l_is_connected_channel == true) {
         session_core_1__client_get_token_from_session(session_core__session,
            session_core__session_token);
         session_core_1__set_session_state(session_core__session,
            constants__e_session_closing,
            true);
         *session_core__ret = constants_statuscodes_bs__e_sc_ok;
      }
      else {
         *session_core__channel = constants__c_channel_indet;
         *session_core__session_token = constants__c_session_token_indet;
         *session_core__ret = constants_statuscodes_bs__e_sc_bad_unexpected_error;
      }
   }
}

void session_core__server_close_session_req_and_resp_sm(
   const constants__t_channel_i session_core__channel,
   const constants__t_session_i session_core__session,
   const constants__t_msg_i session_core__close_req_msg,
   const constants__t_msg_i session_core__close_resp_msg,
   constants_statuscodes_bs__t_StatusCode_i * const session_core__ret) {
   channel_mgr__channel_do_nothing(session_core__channel);
   session_core_1__server_close_session_check_req(session_core__close_req_msg,
      session_core__close_resp_msg);
   session_core_1__set_session_state_closed(session_core__session,
      constants_statuscodes_bs__e_sc_ok,
      false);
   *session_core__ret = constants_statuscodes_bs__e_sc_ok;
}

void session_core__client_close_session_resp_sm(
   const constants__t_channel_i session_core__channel,
   const constants__t_session_i session_core__session,
   const constants__t_msg_i session_core__close_resp_msg) {
   channel_mgr__channel_do_nothing(session_core__channel);
   session_core_1__client_close_session_resp_msg(session_core__close_resp_msg);
   session_core_1__set_session_state_closed(session_core__session,
      constants_statuscodes_bs__e_sc_ok,
      true);
}

void session_core__client_close_session_sm(
   const constants__t_session_i session_core__session,
   const constants_statuscodes_bs__t_StatusCode_i session_core__sc_reason) {
   session_core_1__set_session_state_closed(session_core__session,
      session_core__sc_reason,
      true);
}

void session_core__server_close_session_sm(
   const constants__t_session_i session_core__session,
   const constants_statuscodes_bs__t_StatusCode_i session_core__sc_reason) {
   session_core_1__set_session_state_closed(session_core__session,
      session_core__sc_reason,
      false);
}

void session_core__is_session_valid_for_service(
   const constants__t_channel_i session_core__channel,
   const constants__t_session_i session_core__session,
   t_bool * const session_core__ret) {
   channel_mgr__channel_do_nothing(session_core__channel);
   session_core_1__session_do_nothing(session_core__session);
   *session_core__ret = true;
}

void session_core__get_session_state_or_closed(
   const constants__t_session_i session_core__session,
   constants__t_sessionState * const session_core__state) {
   {
      t_bool session_core__l_valid_session;
      
      session_core_1__is_valid_session(session_core__session,
         &session_core__l_valid_session);
      if (session_core__l_valid_session == true) {
         session_core_1__get_session_state(session_core__session,
            session_core__state);
      }
      else {
         *session_core__state = constants__e_session_closed;
      }
   }
}

void session_core__find_channel_to_close(
   t_bool * const session_core__p_has_channel_to_close,
   constants__t_channel_i * const session_core__p_channel_to_close) {
   {
      t_bool session_core__l_continue;
      constants__t_channel_i session_core__l_channel;
      t_bool session_core__l_connected;
      constants__t_session_i session_core__l_session;
      constants__t_timeref_i session_core__l_timeref;
      t_bool session_core__l_is_older_than;
      constants__t_channel_i session_core__l_oldest_channel;
      constants__t_timeref_i session_core__l_oldest_channel_timeref;
      
      session_core__l_oldest_channel = constants__c_channel_indet;
      session_core__l_oldest_channel_timeref = constants__c_timeref_indet;
      session_channel_it__init_iter_channel(&session_core__l_continue);
      while (session_core__l_continue == true) {
         session_channel_it__continue_iter_channel(&session_core__l_continue,
            &session_core__l_channel);
         channel_mgr__is_connected_channel(session_core__l_channel,
            &session_core__l_connected);
         session_core_1__get_channel_session(session_core__l_channel,
            &session_core__l_session);
         if ((session_core__l_connected == true) &&
            (session_core__l_session == constants__c_session_indet)) {
            channel_mgr__get_connection_time(session_core__l_channel,
               &session_core__l_timeref);
            if (session_core__l_oldest_channel_timeref == constants__c_timeref_indet) {
               session_core__l_oldest_channel_timeref = session_core__l_timeref;
               session_core__l_oldest_channel = session_core__l_channel;
            }
            else {
               time_reference_bs__is_less_than_TimeReference(session_core__l_timeref,
                  session_core__l_oldest_channel_timeref,
                  &session_core__l_is_older_than);
               if (session_core__l_is_older_than == true) {
                  session_core__l_oldest_channel_timeref = session_core__l_timeref;
                  session_core__l_oldest_channel = session_core__l_channel;
               }
            }
         }
      }
      if (session_core__l_oldest_channel_timeref == constants__c_timeref_indet) {
         *session_core__p_has_channel_to_close = false;
         *session_core__p_channel_to_close = constants__c_channel_indet;
      }
      else {
         *session_core__p_has_channel_to_close = true;
         *session_core__p_channel_to_close = session_core__l_oldest_channel;
      }
   }
}

void session_core__getall_valid_session_channel(
   const constants__t_session_i session_core__session,
   t_bool * const session_core__bres,
   constants__t_channel_i * const session_core__channel) {
   session_core_1__is_valid_session(session_core__session,
      session_core__bres);
   if (*session_core__bres == true) {
      session_core_1__get_session_channel(session_core__session,
         session_core__channel);
   }
   else {
      session_core_1__getall_session_channel(session_core__session,
         session_core__bres,
         session_core__channel);
   }
}

