/******************************************************************************

 File Name            : session_core.c

 Date                 : 01/08/2017 11:32:38

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
void session_core__srv_internal_activate_req_and_resp(
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

void session_core__client_init_session(
   constants__t_session_i * const session_core__nsession) {
   session_core_1_bs__init_new_session(session_core__nsession);
}

void session_core__cli_create_req(
   const constants__t_session_i session_core__session,
   const constants__t_channel_i session_core__channel,
   const constants__t_request_handle_i session_core__req_handle,
   const constants__t_msg_i session_core__create_req_msg,
   constants__t_StatusCode_i * const session_core__ret) {
   {
      t_bool session_core__l_valid_handle;
      
      session_core_1_bs__create_session(session_core__session,
         session_core__channel,
         constants__e_session_creating);
      session_core_1_bs__cli_add_pending_request(session_core__session,
         session_core__req_handle,
         &session_core__l_valid_handle);
      if (session_core__l_valid_handle == true) {
         *session_core__ret = constants__e_sc_ok;
      }
      else {
         *session_core__ret = constants__e_sc_bad_invalid_argument;
      }
   }
}

void session_core__srv_create_req_and_resp(
   const constants__t_channel_i session_core__channel,
   const constants__t_request_handle_i session_core__req_handle,
   const constants__t_msg_i session_core__create_req_msg,
   const constants__t_msg_i session_core__create_resp_msg,
   constants__t_session_i * const session_core__nsession,
   constants__t_StatusCode_i * const session_core__service_ret) {
   {
      constants__t_session_i session_core__l_nsession;
      t_bool session_core__l_valid_session;
      constants__t_session_token_i session_core__l_nsession_token;
      t_bool session_core__l_valid_session_token;
      
      session_core_1_bs__init_new_session(&session_core__l_nsession);
      session_core_1_bs__is_valid_session(session_core__l_nsession,
         &session_core__l_valid_session);
      if (session_core__l_valid_session == true) {
         session_core_1_bs__create_session(session_core__l_nsession,
            session_core__channel,
            constants__e_session_created);
         session_core_1_bs__get_fresh_session_token(session_core__l_nsession,
            &session_core__l_nsession_token);
         session_core_1_bs__is_valid_session_token(session_core__l_nsession_token,
            &session_core__l_valid_session_token);
         if (session_core__l_valid_session_token == true) {
            message_out_bs__write_create_session_msg_session_token(session_core__create_resp_msg,
               session_core__l_nsession_token);
            *session_core__service_ret = constants__e_sc_ok;
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

void session_core__cli_create_resp(
   const constants__t_channel_i session_core__channel,
   const constants__t_session_i session_core__session,
   const constants__t_request_handle_i session_core__req_handle,
   const constants__t_session_token_i session_core__session_token,
   const constants__t_msg_i session_core__create_resp_msg,
   constants__t_StatusCode_i * const session_core__ret) {
   {
      t_bool session_core__l_valid_handle;
      t_bool session_core__l_fresh_session_token;
      t_bool session_core__l_has_session_token;
      constants__t_StatusCode_i session_core__l_status;
      
      session_core_1_bs__is_fresh_session_token(session_core__session_token,
         &session_core__l_fresh_session_token);
      session_core_1_bs__has_session_token(session_core__session,
         &session_core__l_has_session_token);
      if ((session_core__l_fresh_session_token == true) &&
         (session_core__l_has_session_token == false)) {
         session_core_1_bs__cli_remove_pending_request(session_core__session,
            session_core__req_handle,
            &session_core__l_valid_handle);
         message_in_bs__read_msg_resp_header_service_status(session_core__create_resp_msg,
            &session_core__l_status);
         if (session_core__l_valid_handle == true) {
            if (session_core__l_status == constants__e_sc_ok) {
               session_core_1_bs__set_session_state(session_core__session,
                  constants__e_session_created);
               session_core_1_bs__set_session_token(session_core__session,
                  session_core__session_token);
               *session_core__ret = constants__e_sc_ok;
            }
            else {
               *session_core__ret = constants__e_sc_nok;
            }
         }
         else {
            *session_core__ret = constants__e_sc_bad_invalid_argument;
         }
      }
      else {
         *session_core__ret = constants__e_sc_bad_invalid_argument;
      }
   }
}

void session_core__cli_user_activate_req(
   const constants__t_session_i session_core__session,
   const constants__t_request_handle_i session_core__req_handle,
   const constants__t_user_i session_core__user,
   const constants__t_msg_i session_core__activate_req_msg,
   constants__t_StatusCode_i * const session_core__ret,
   constants__t_channel_i * const session_core__channel,
   constants__t_session_token_i * const session_core__session_token) {
   {
      t_bool session_core__l_valid_handle;
      constants__t_session_token_i session_core__l_session_token;
      constants__t_channel_i session_core__l_channel;
      t_bool session_core__l_connected_channel;
      constants__t_StatusCode_i session_core__l_ret;
      
      session_core__l_channel = constants__c_channel_indet;
      session_core__l_session_token = constants__c_session_token_indet;
      session_core_1_bs__get_session_channel(session_core__session,
         &session_core__l_channel);
      channel_mgr_bs__is_connected_channel(session_core__l_channel,
         &session_core__l_connected_channel);
      session_core_1_bs__get_token_from_session(session_core__session,
         &session_core__l_session_token);
      if (session_core__l_connected_channel == true) {
         session_core_1_bs__cli_add_pending_request(session_core__session,
            session_core__req_handle,
            &session_core__l_valid_handle);
         if (session_core__l_valid_handle == true) {
            message_out_bs__write_activate_msg_user(session_core__activate_req_msg,
               session_core__user);
            session_core_1_bs__set_session_user(session_core__session,
               session_core__user);
            session_core_1_bs__set_session_state(session_core__session,
               constants__e_session_userActivating);
            session_core__l_ret = constants__e_sc_ok;
         }
         else {
            session_core__l_session_token = constants__c_session_token_indet;
            session_core__l_ret = constants__e_sc_bad_invalid_argument;
         }
      }
      else {
         session_core__l_session_token = constants__c_session_token_indet;
         session_core__l_ret = constants__e_sc_bad_unexpected_error;
      }
      *session_core__ret = session_core__l_ret;
      *session_core__session_token = session_core__l_session_token;
      *session_core__channel = session_core__l_channel;
   }
}

void session_core__cli_sc_activate_req(
   const constants__t_session_i session_core__session,
   const constants__t_request_handle_i session_core__req_handle,
   const constants__t_channel_i session_core__channel,
   const constants__t_msg_i session_core__activate_req_msg,
   constants__t_StatusCode_i * const session_core__ret,
   constants__t_session_token_i * const session_core__session_token) {
   {
      t_bool session_core__l_valid_handle;
      constants__t_session_token_i session_core__l_session_token;
      constants__t_user_i session_core__l_user;
      constants__t_StatusCode_i session_core__l_ret;
      
      session_core__l_session_token = constants__c_session_token_indet;
      session_core_1_bs__get_token_from_session(session_core__session,
         &session_core__l_session_token);
      session_core_1_bs__get_session_user(session_core__session,
         &session_core__l_user);
      session_core_1_bs__cli_add_pending_request(session_core__session,
         session_core__req_handle,
         &session_core__l_valid_handle);
      if (session_core__l_valid_handle == true) {
         message_out_bs__write_activate_msg_user(session_core__activate_req_msg,
            session_core__l_user);
         session_core_1_bs__set_session_channel(session_core__session,
            session_core__channel);
         session_core_1_bs__set_session_state(session_core__session,
            constants__e_session_scActivating);
         session_core__l_ret = constants__e_sc_ok;
      }
      else {
         session_core__l_session_token = constants__c_session_token_indet;
         session_core__l_ret = constants__e_sc_bad_invalid_argument;
      }
      *session_core__session_token = session_core__l_session_token;
      *session_core__ret = session_core__l_ret;
   }
}

void session_core__srv_activate_req_and_resp(
   const constants__t_channel_i session_core__channel,
   const constants__t_session_i session_core__session,
   const constants__t_request_handle_i session_core__req_handle,
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
            session_core__srv_internal_activate_req_and_resp(session_core__channel,
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
            session_core__srv_internal_activate_req_and_resp(session_core__channel,
               session_core__session,
               session_core__user,
               session_core__activate_resp_msg);
            session_core__l_ret = constants__e_sc_ok;
         }
         else if ((session_core__l_channel != session_core__channel) &&
            (session_core__l_user == session_core__user)) {
            session_core__srv_internal_activate_req_and_resp(session_core__channel,
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
            session_core__srv_internal_activate_req_and_resp(session_core__channel,
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

void session_core__cli_activate_resp(
   const constants__t_channel_i session_core__channel,
   const constants__t_session_i session_core__session,
   const constants__t_request_handle_i session_core__req_handle,
   const constants__t_msg_i session_core__activate_resp_msg,
   constants__t_StatusCode_i * const session_core__ret) {
   {
      t_bool session_core__l_valid_handle;
      constants__t_sessionState session_core__l_state;
      constants__t_StatusCode_i session_core__l_status;
      
      session_core_1_bs__get_session_state(session_core__session,
         &session_core__l_state);
      session_core_1_bs__cli_remove_pending_request(session_core__session,
         session_core__req_handle,
         &session_core__l_valid_handle);
      message_in_bs__read_msg_resp_header_service_status(session_core__activate_resp_msg,
         &session_core__l_status);
      if (session_core__l_valid_handle == true) {
         if (session_core__l_status == constants__e_sc_ok) {
            session_core_1_bs__set_session_state(session_core__session,
               constants__e_session_userActivated);
         }
         else {
            *session_core__ret = constants__e_sc_nok;
         }
      }
      else {
         *session_core__ret = constants__e_sc_bad_invalid_argument;
      }
   }
}

void session_core__client_secure_channel_lost(
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

void session_core__server_secure_channel_lost(
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

void session_core__cli_close_req(
   const constants__t_session_i session_core__session,
   const constants__t_request_handle_i session_core__req_handle,
   const constants__t_msg_i session_core__close_req_msg,
   constants__t_StatusCode_i * const session_core__ret,
   constants__t_channel_i * const session_core__channel,
   constants__t_session_token_i * const session_core__session_token) {
   {
      constants__t_channel_i session_core__l_channel;
      t_bool session_core__l_connected_channel;
      t_bool session_core__l_valid_handle;
      constants__t_session_token_i session_core__l_session_token;
      constants__t_StatusCode_i session_core__l_ret;
      
      session_core__l_channel = constants__c_channel_indet;
      session_core__l_session_token = constants__c_session_token_indet;
      session_core_1_bs__get_session_channel(session_core__session,
         &session_core__l_channel);
      channel_mgr_bs__is_connected_channel(session_core__l_channel,
         &session_core__l_connected_channel);
      session_core_1_bs__get_token_from_session(session_core__session,
         &session_core__l_session_token);
      if (session_core__l_connected_channel == true) {
         session_core_1_bs__cli_add_pending_request(session_core__session,
            session_core__req_handle,
            &session_core__l_valid_handle);
         if (session_core__l_valid_handle == true) {
            session_core_1_bs__set_session_state(session_core__session,
               constants__e_session_closing);
            session_core__l_ret = constants__e_sc_ok;
         }
         else {
            session_core__l_session_token = constants__c_session_token_indet;
            session_core__l_ret = constants__e_sc_bad_invalid_argument;
         }
      }
      else {
         session_core__l_ret = constants__e_sc_bad_unexpected_error;
      }
      *session_core__channel = session_core__l_channel;
      *session_core__session_token = session_core__l_session_token;
      *session_core__ret = session_core__l_ret;
   }
}

void session_core__srv_close_req_and_resp(
   const constants__t_channel_i session_core__channel,
   const constants__t_session_i session_core__session,
   const constants__t_request_handle_i session_core__req_handle,
   const constants__t_msg_i session_core__close_req_msg,
   const constants__t_msg_i session_core__close_resp_msg,
   constants__t_StatusCode_i * const session_core__ret) {
   session_core_1_bs__set_session_state_closed(session_core__session);
   *session_core__ret = constants__e_sc_ok;
}

void session_core__cli_close_resp(
   const constants__t_channel_i session_core__channel,
   const constants__t_session_i session_core__session,
   const constants__t_request_handle_i session_core__req_handle,
   const constants__t_msg_i session_core__close_resp_msg) {
   {
      t_bool session_core__l_valid_handle;
      constants__t_StatusCode_i session_core__l_status;
      
      session_core_1_bs__cli_remove_pending_request(session_core__session,
         session_core__req_handle,
         &session_core__l_valid_handle);
      message_in_bs__read_msg_resp_header_service_status(session_core__close_resp_msg,
         &session_core__l_status);
      session_core_1_bs__set_session_state_closed(session_core__session);
   }
}

void session_core__client_close_session(
   const constants__t_session_i session_core__session) {
   session_core_1_bs__set_session_state_closed(session_core__session);
}

void session_core__server_close_session(
   const constants__t_session_i session_core__session) {
   session_core_1_bs__set_session_state_closed(session_core__session);
}

void session_core__cli_new_session_service_req(
   const constants__t_session_i session_core__session,
   const constants__t_request_handle_i session_core__req_handle,
   constants__t_StatusCode_i * const session_core__ret,
   constants__t_channel_i * const session_core__channel,
   constants__t_session_token_i * const session_core__session_token) {
   {
      t_bool session_core__l_valid_handle;
      constants__t_session_token_i session_core__l_session_token;
      
      *session_core__session_token = constants__c_session_token_indet;
      *session_core__channel = constants__c_channel_indet;
      session_core_1_bs__cli_add_pending_request(session_core__session,
         session_core__req_handle,
         &session_core__l_valid_handle);
      session_core_1_bs__get_token_from_session(session_core__session,
         &session_core__l_session_token);
      if (session_core__l_valid_handle == true) {
         *session_core__session_token = session_core__l_session_token;
         session_core_1_bs__get_session_channel(session_core__session,
            session_core__channel);
         *session_core__ret = constants__e_sc_ok;
      }
      else {
         *session_core__ret = constants__e_sc_bad_invalid_argument;
      }
   }
}

void session_core__cli_record_session_service_resp(
   const constants__t_session_i session_core__session,
   const constants__t_msg_i session_core__msg,
   const constants__t_request_handle_i session_core__req_handle,
   t_bool * const session_core__bres) {
   session_core_1_bs__cli_remove_pending_request(session_core__session,
      session_core__req_handle,
      session_core__bres);
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

