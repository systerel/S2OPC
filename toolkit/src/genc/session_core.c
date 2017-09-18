/******************************************************************************

 File Name            : session_core.c

 Date                 : 15/09/2017 14:19:09

 C Translator Version : tradc Java V1.0 (14/03/2012)

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
   const constants__t_user_i session_core__user,
   const constants__t_msg_i session_core__activate_resp_msg) {
   session_core_1_bs__set_session_user(session_core__session,
      session_core__user);
   session_core_1_bs__set_session_channel(session_core__session,
      session_core__channel);
   session_core_1_bs__set_session_state(session_core__session,
      constants__e_session_userActivated);
}

void session_core__client_init_session_sm(
   constants__t_session_i * const session_core__nsession) {
   session_core_1_bs__init_new_session(session_core__nsession);
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
      
      session_core_1_bs__create_session(session_core__session,
         session_core__channel,
         constants__e_session_creating);
      channel_mgr_bs__get_channel_info(session_core__channel,
         &session_core__l_channel_config_idx);
      session_core_1_bs__client_create_session_req_do_crypto(session_core__session,
         session_core__channel,
         session_core__l_channel_config_idx,
         session_core__valid,
         &session_core__l_nonce_needed);
      if (*session_core__valid == true) {
         message_out_bs__write_create_session_req_msg_endpointUrl(session_core__create_req_msg,
            session_core__l_channel_config_idx);
         if (session_core__l_nonce_needed == true) {
            session_core_1_bs__get_NonceClient(session_core__session,
               &session_core__l_nonce);
            message_out_bs__write_create_session_req_msg_crypto(session_core__create_req_msg,
               session_core__l_channel_config_idx,
               session_core__l_nonce);
         }
      }
   }
}

void session_core__server_create_session_req_and_resp_sm(
   const constants__t_channel_i session_core__channel,
   const constants__t_msg_i session_core__create_req_msg,
   const constants__t_msg_i session_core__create_resp_msg,
   constants__t_session_i * const session_core__nsession,
   constants__t_StatusCode_i * const session_core__service_ret) {
   {
      constants__t_session_i session_core__l_nsession;
      t_bool session_core__l_valid_session;
      constants__t_session_token_i session_core__l_nsession_token;
      t_bool session_core__l_valid_session_token;
      constants__t_endpoint_config_idx_i session_core__l_endpoint_config_idx;
      constants__t_channel_config_idx_i session_core__l_config_idx;
      t_bool session_core__l_valid_crypto;
      constants__t_SignatureData_i session_core__l_signature;
      constants__t_Nonce_i session_core__l_nonce;
      constants__t_SecurityPolicy session_core__l_secpol;
      
      session_core_1_bs__init_new_session(&session_core__l_nsession);
      session_core_1_bs__is_valid_session(session_core__l_nsession,
         &session_core__l_valid_session);
      if (session_core__l_valid_session == true) {
         session_core_1_bs__create_session(session_core__l_nsession,
            session_core__channel,
            constants__e_session_created);
         session_core_1_bs__server_get_fresh_session_token(session_core__l_nsession,
            &session_core__l_nsession_token);
         session_core_1_bs__server_is_valid_session_token(session_core__l_nsession_token,
            &session_core__l_valid_session_token);
         if (session_core__l_valid_session_token == true) {
            channel_mgr_bs__server_get_endpoint_config(session_core__channel,
               &session_core__l_endpoint_config_idx);
            message_out_bs__write_create_session_msg_session_token(session_core__create_resp_msg,
               session_core__l_nsession_token);
            message_out_bs__write_create_session_msg_server_endpoints(session_core__create_req_msg,
               session_core__create_resp_msg,
               session_core__l_endpoint_config_idx,
               session_core__service_ret);
            if (*session_core__service_ret == constants__e_sc_ok) {
               channel_mgr_bs__get_SecurityPolicy(session_core__channel,
                  &session_core__l_secpol);
               if (session_core__l_secpol != constants__e_secpol_None) {
                  channel_mgr_bs__get_channel_info(session_core__channel,
                     &session_core__l_config_idx);
                  session_core_1_bs__server_create_session_req_do_crypto(session_core__l_nsession,
                     session_core__create_req_msg,
                     session_core__l_endpoint_config_idx,
                     session_core__l_config_idx,
                     &session_core__l_valid_crypto,
                     &session_core__l_signature);
                  if (session_core__l_valid_crypto == true) {
                     session_core_1_bs__get_NonceServer(session_core__l_nsession,
                        &session_core__l_nonce);
                     message_out_bs__write_create_session_msg_crypto(session_core__create_resp_msg,
                        session_core__l_nonce,
                        session_core__l_signature,
                        session_core__service_ret);
                  }
                  else {
                     *session_core__service_ret = constants__e_sc_bad_unexpected_error;
                  }
               }
            }
         }
         else {
            *session_core__service_ret = constants__e_sc_bad_out_of_memory;
            session_core_1_bs__delete_session(session_core__l_nsession);
            session_core__l_nsession = constants__c_session_indet;
         }
      }
      else {
         *session_core__service_ret = constants__e_sc_bad_out_of_memory;
      }
      *session_core__nsession = session_core__l_nsession;
   }
}

void session_core__client_create_session_resp_sm(
   const constants__t_channel_i session_core__channel,
   const constants__t_session_i session_core__session,
   const constants__t_session_token_i session_core__session_token,
   const constants__t_msg_i session_core__create_resp_msg,
   constants__t_StatusCode_i * const session_core__ret) {
   {
      constants__t_Nonce_i session_core__l_nonce;
      t_bool session_core__l_valid;
      constants__t_channel_config_idx_i session_core__l_channel_config_idx;
      constants__t_SecurityPolicy session_core__l_secpol;
      
      *session_core__ret = constants__e_sc_ok;
      channel_mgr_bs__get_SecurityPolicy(session_core__channel,
         &session_core__l_secpol);
      if (session_core__l_secpol != constants__e_secpol_None) {
         session_core_1_bs__get_NonceClient(session_core__session,
            &session_core__l_nonce);
         channel_mgr_bs__get_channel_info(session_core__channel,
            &session_core__l_channel_config_idx);
         session_core_1_bs__client_create_session_check_crypto(session_core__session,
            session_core__l_channel_config_idx,
            session_core__create_resp_msg,
            &session_core__l_valid);
         if (session_core__l_valid == true) {
            session_core_1_bs__set_session_state(session_core__session,
               constants__e_session_created);
            session_core_1_bs__client_set_session_token(session_core__session,
               session_core__session_token);
            session_core_1_bs__drop_NonceClient(session_core__session);
         }
         else {
            *session_core__ret = constants__e_sc_nok;
         }
      }
      else {
         session_core_1_bs__set_session_state(session_core__session,
            constants__e_session_created);
         session_core_1_bs__client_set_session_token(session_core__session,
            session_core__session_token);
      }
   }
}

void session_core__client_user_activate_session_req_sm(
   const constants__t_session_i session_core__session,
   const constants__t_user_i session_core__user,
   const constants__t_msg_i session_core__activate_req_msg,
   constants__t_StatusCode_i * const session_core__ret,
   constants__t_channel_i * const session_core__channel,
   constants__t_session_token_i * const session_core__session_token) {
   session_core_1_bs__get_session_channel(session_core__session,
      session_core__channel);
   session_core_1_bs__client_get_token_from_session(session_core__session,
      session_core__session_token);
   message_out_bs__write_activate_msg_user(session_core__activate_req_msg,
      session_core__user);
   session_core_1_bs__set_session_user(session_core__session,
      session_core__user);
   session_core_1_bs__set_session_state(session_core__session,
      constants__e_session_userActivating);
   *session_core__ret = constants__e_sc_ok;
}

void session_core__client_sc_activate_session_req_sm(
   const constants__t_session_i session_core__session,
   const constants__t_channel_i session_core__channel,
   const constants__t_msg_i session_core__activate_req_msg,
   constants__t_session_token_i * const session_core__session_token) {
   {
      constants__t_user_i session_core__l_user;
      
      session_core_1_bs__client_get_token_from_session(session_core__session,
         session_core__session_token);
      session_core_1_bs__get_session_user(session_core__session,
         &session_core__l_user);
      message_out_bs__write_activate_msg_user(session_core__activate_req_msg,
         session_core__l_user);
      session_core_1_bs__set_session_channel(session_core__session,
         session_core__channel);
      session_core_1_bs__set_session_state(session_core__session,
         constants__e_session_scActivating);
   }
}

void session_core__server_activate_session_req_and_resp_sm(
   const constants__t_channel_i session_core__channel,
   const constants__t_session_i session_core__session,
   const constants__t_user_i session_core__user,
   const constants__t_msg_i session_core__activate_req_msg,
   const constants__t_msg_i session_core__activate_resp_msg,
   constants__t_StatusCode_i * const session_core__ret) {
   {
      constants__t_channel_i session_core__l_channel;
      constants__t_user_i session_core__l_user;
      constants__t_sessionState session_core__l_state;
      constants__t_StatusCode_i session_core__l_ret;
      
      session_core_1_bs__get_session_channel(session_core__session,
         &session_core__l_channel);
      session_core_1_bs__get_session_user(session_core__session,
         &session_core__l_user);
      session_core_1_bs__get_session_state(session_core__session,
         &session_core__l_state);
      if (session_core__l_state == constants__e_session_created) {
         if (session_core__l_channel == session_core__channel) {
            session_core__server_internal_activate_req_and_resp(session_core__channel,
               session_core__session,
               session_core__user,
               session_core__activate_resp_msg);
            session_core__l_ret = constants__e_sc_ok;
         }
         else {
            session_core__l_ret = constants__e_sc_bad_invalid_argument;
         }
      }
      else if (session_core__l_state == constants__e_session_userActivated) {
         if ((session_core__l_channel == session_core__channel) &&
            (session_core__l_user != session_core__user)) {
            session_core__server_internal_activate_req_and_resp(session_core__channel,
               session_core__session,
               session_core__user,
               session_core__activate_resp_msg);
            session_core__l_ret = constants__e_sc_ok;
         }
         else if ((session_core__l_channel != session_core__channel) &&
            (session_core__l_user == session_core__user)) {
            session_core__server_internal_activate_req_and_resp(session_core__channel,
               session_core__session,
               session_core__user,
               session_core__activate_resp_msg);
            session_core__l_ret = constants__e_sc_ok;
         }
         else {
            session_core__l_ret = constants__e_sc_bad_invalid_argument;
         }
      }
      else if (session_core__l_state == constants__e_session_scOrphaned) {
         if ((session_core__l_channel != session_core__channel) &&
            (session_core__l_user == session_core__user)) {
            session_core__server_internal_activate_req_and_resp(session_core__channel,
               session_core__session,
               session_core__user,
               session_core__activate_resp_msg);
            session_core__l_ret = constants__e_sc_ok;
         }
         else {
            session_core__l_ret = constants__e_sc_bad_invalid_argument;
         }
      }
      else {
         session_core__l_ret = constants__e_sc_bad_invalid_state;
      }
      *session_core__ret = session_core__l_ret;
   }
}

void session_core__client_activate_session_resp_sm(
   const constants__t_channel_i session_core__channel,
   const constants__t_session_i session_core__session,
   const constants__t_msg_i session_core__activate_resp_msg,
   constants__t_StatusCode_i * const session_core__ret) {
   session_core_1_bs__set_session_state(session_core__session,
      constants__e_session_userActivated);
   *session_core__ret = constants__e_sc_ok;
}

void session_core__client_secure_channel_lost_session_sm(
   const constants__t_channel_i session_core__lost_channel,
   const constants__t_channel_config_idx_i session_core__channel_config_idx) {
   {
      t_bool session_core__l_continue;
      constants__t_session_i session_core__l_session;
      t_bool session_core__l_valid_session;
      constants__t_sessionState session_core__l_state;
      
      session_core_channel_lost_it_bs__init_iter_channel_lost_t_session(session_core__lost_channel,
         &session_core__l_continue);
      while (session_core__l_continue == true) {
         session_core_orphaned_it_bs__continue_iter_orphaned_t_session(&session_core__l_session,
            &session_core__l_continue);
         session_core_1_bs__is_valid_session(session_core__l_session,
            &session_core__l_valid_session);
         if (session_core__l_valid_session == true) {
            session_core_1_bs__get_session_state(session_core__l_session,
               &session_core__l_state);
            if ((session_core__l_state == constants__e_session_userActivated) &&
               (session_core__channel_config_idx != constants__c_channel_config_idx_indet)) {
               session_core_1_bs__set_session_orphaned(session_core__l_session,
                  session_core__lost_channel,
                  session_core__channel_config_idx);
               session_core_1_bs__set_session_state(session_core__l_session,
                  constants__e_session_scOrphaned);
            }
            else {
               session_core_1_bs__set_session_state_closed(session_core__l_session);
            }
         }
      }
   }
}

void session_core__server_secure_channel_lost_session_sm(
   const constants__t_channel_i session_core__channel) {
   {
      t_bool session_core__l_continue;
      constants__t_session_i session_core__l_session;
      t_bool session_core__l_valid_session;
      constants__t_sessionState session_core__l_state;
      
      session_core_channel_lost_it_bs__init_iter_channel_lost_t_session(session_core__channel,
         &session_core__l_continue);
      while (session_core__l_continue == true) {
         session_core_orphaned_it_bs__continue_iter_orphaned_t_session(&session_core__l_session,
            &session_core__l_continue);
         session_core_1_bs__is_valid_session(session_core__l_session,
            &session_core__l_valid_session);
         if (session_core__l_valid_session == true) {
            session_core_1_bs__get_session_state(session_core__l_session,
               &session_core__l_state);
            if (session_core__l_state == constants__e_session_userActivated) {
               session_core_1_bs__set_session_orphaned(session_core__l_session,
                  session_core__channel,
                  constants__c_channel_config_idx_indet);
               session_core_1_bs__set_session_state(session_core__l_session,
                  constants__e_session_scOrphaned);
            }
            else {
               session_core_1_bs__set_session_state_closed(session_core__l_session);
            }
         }
      }
   }
}

void session_core__client_close_session_req_sm(
   const constants__t_session_i session_core__session,
   const constants__t_msg_i session_core__close_req_msg,
   constants__t_StatusCode_i * const session_core__ret,
   constants__t_channel_i * const session_core__channel,
   constants__t_session_token_i * const session_core__session_token) {
   session_core_1_bs__get_session_channel(session_core__session,
      session_core__channel);
   session_core_1_bs__client_get_token_from_session(session_core__session,
      session_core__session_token);
   session_core_1_bs__set_session_state(session_core__session,
      constants__e_session_closing);
   *session_core__ret = constants__e_sc_ok;
}

void session_core__server_close_session_req_and_resp_sm(
   const constants__t_channel_i session_core__channel,
   const constants__t_session_i session_core__session,
   const constants__t_msg_i session_core__close_req_msg,
   const constants__t_msg_i session_core__close_resp_msg,
   constants__t_StatusCode_i * const session_core__ret) {
   session_core_1_bs__set_session_state_closed(session_core__session);
   *session_core__ret = constants__e_sc_ok;
}

void session_core__client_close_session_resp_sm(
   const constants__t_channel_i session_core__channel,
   const constants__t_session_i session_core__session,
   const constants__t_msg_i session_core__close_resp_msg) {
   session_core_1_bs__set_session_state_closed(session_core__session);
}

void session_core__client_close_session_sm(
   const constants__t_session_i session_core__session) {
   session_core_1_bs__set_session_state_closed(session_core__session);
}

void session_core__server_close_session_sm(
   const constants__t_session_i session_core__session) {
   session_core_1_bs__set_session_state_closed(session_core__session);
}

void session_core__is_session_valid_for_service(
   const constants__t_channel_i session_core__channel,
   const constants__t_session_i session_core__session,
   t_bool * const session_core__ret) {
   *session_core__ret = true;
}

void session_core__get_session_state_or_closed(
   const constants__t_session_i session_core__session,
   constants__t_sessionState * const session_core__state) {
   {
      t_bool session_core__l_valid_session;
      
      session_core_1_bs__is_valid_session(session_core__session,
         &session_core__l_valid_session);
      if (session_core__l_valid_session == true) {
         session_core_1_bs__get_session_state(session_core__session,
            session_core__state);
      }
      else {
         *session_core__state = constants__e_session_closed;
      }
   }
}

void session_core__get_session_user_or_indet(
   const constants__t_session_i session_core__session,
   constants__t_user_i * const session_core__user) {
   {
      t_bool session_core__l_valid_session;
      constants__t_sessionState session_core__l_session_state;
      
      *session_core__user = constants__c_user_indet;
      session_core_1_bs__is_valid_session(session_core__session,
         &session_core__l_valid_session);
      if (session_core__l_valid_session == true) {
         session_core_1_bs__get_session_state(session_core__session,
            &session_core__l_session_state);
         if ((((session_core__l_session_state != constants__e_session_created) &&
            (session_core__l_session_state != constants__e_session_creating)) &&
            (session_core__l_session_state != constants__e_session_closed)) &&
            (session_core__l_session_state != constants__e_session_init)) {
            session_core_1_bs__get_session_user(session_core__session,
               session_core__user);
         }
      }
   }
}

