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

 File Name            : session_mgr.c

 Date                 : 04/08/2022 14:53:19

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "session_mgr.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void session_mgr__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void session_mgr__local_client_close_session(
   const constants__t_session_i session_mgr__session,
   const constants_statuscodes_bs__t_StatusCode_i session_mgr__sc_reason) {
   session_request_handle_bs__client_remove_all_request_handles(session_mgr__session);
   session_core__client_close_session_sm(session_mgr__session,
      session_mgr__sc_reason);
}

void session_mgr__local_client_close_session_if_needed(
   const t_bool session_mgr__cond,
   const constants__t_session_i session_mgr__session,
   const constants_statuscodes_bs__t_StatusCode_i session_mgr__sc_reason) {
   if (session_mgr__cond == false) {
      session_mgr__local_client_close_session(session_mgr__session,
         session_mgr__sc_reason);
   }
}

void session_mgr__local_client_activate_sessions_on_SC_connection(
   const constants__t_channel_config_idx_i session_mgr__channel_config_idx) {
   {
      t_bool session_mgr__l_continue;
      constants__t_session_i session_mgr__l_session;
      t_bool session_mgr__l_dom;
      constants__t_channel_config_idx_i session_mgr__l_channel_config_idx;
      
      session_mgr_it__init_iter_session(&session_mgr__l_continue);
      if (session_mgr__l_continue == true) {
         while (session_mgr__l_continue == true) {
            session_mgr_it__continue_iter_session(&session_mgr__l_continue,
               &session_mgr__l_session);
            session_core__getall_orphaned(session_mgr__l_session,
               &session_mgr__l_dom,
               &session_mgr__l_channel_config_idx);
            if ((session_mgr__l_dom == true) &&
               (session_mgr__l_channel_config_idx == session_mgr__channel_config_idx)) {
               session_core__client_gen_activate_orphaned_session_internal_event(session_mgr__l_session,
                  session_mgr__channel_config_idx);
            }
            session_core__getall_to_create(session_mgr__l_session,
               &session_mgr__l_dom,
               &session_mgr__l_channel_config_idx);
            if ((session_mgr__l_dom == true) &&
               (session_mgr__l_channel_config_idx == session_mgr__channel_config_idx)) {
               session_core__reset_session_to_create(session_mgr__l_session);
               session_core__client_gen_create_session_internal_event(session_mgr__l_session,
                  session_mgr__channel_config_idx);
            }
         }
      }
   }
}

void session_mgr__local_client_close_sessions_on_SC_final_connection_failure(
   const constants__t_channel_config_idx_i session_mgr__channel_config_idx) {
   {
      t_bool session_mgr__l_continue;
      constants__t_session_i session_mgr__l_session;
      t_bool session_mgr__l_dom;
      constants__t_channel_config_idx_i session_mgr__l_channel_config_idx;
      
      session_mgr_it__init_iter_session(&session_mgr__l_continue);
      if (session_mgr__l_continue == true) {
         while (session_mgr__l_continue == true) {
            session_mgr_it__continue_iter_session(&session_mgr__l_continue,
               &session_mgr__l_session);
            session_core__getall_orphaned(session_mgr__l_session,
               &session_mgr__l_dom,
               &session_mgr__l_channel_config_idx);
            if ((session_mgr__l_dom == true) &&
               (session_mgr__l_channel_config_idx == session_mgr__channel_config_idx)) {
               session_mgr__local_client_close_session(session_mgr__l_session,
                  constants_statuscodes_bs__e_sc_bad_secure_channel_closed);
            }
            session_core__getall_to_create(session_mgr__l_session,
               &session_mgr__l_dom,
               &session_mgr__l_channel_config_idx);
            if ((session_mgr__l_dom == true) &&
               (session_mgr__l_channel_config_idx == session_mgr__channel_config_idx)) {
               session_mgr__local_client_close_session(session_mgr__l_session,
                  constants_statuscodes_bs__e_sc_bad_secure_channel_closed);
            }
         }
      }
   }
}

void session_mgr__client_receive_session_resp(
   const constants__t_channel_i session_mgr__channel,
   const constants__t_client_request_handle_i session_mgr__req_handle,
   const constants__t_msg_type_i session_mgr__resp_typ,
   const constants__t_msg_header_i session_mgr__resp_header,
   const constants__t_msg_i session_mgr__resp_msg,
   constants__t_session_i * const session_mgr__session) {
   {
      constants__t_session_token_i session_mgr__l_session_token;
      constants__t_sessionState session_mgr__l_session_state;
      constants__t_channel_i session_mgr__l_session_channel;
      constants_statuscodes_bs__t_StatusCode_i session_mgr__l_resp_status;
      constants__t_user_token_i session_mgr__l_session_user_token;
      constants_statuscodes_bs__t_StatusCode_i session_mgr__l_status_reason;
      t_bool session_mgr__l_bret;
      
      session_mgr__l_bret = false;
      session_request_handle_bs__client_get_session_and_remove_request_handle(session_mgr__req_handle,
         session_mgr__session);
      session_core__get_session_state_or_closed(*session_mgr__session,
         &session_mgr__l_session_state);
      message_in_bs__read_msg_resp_header_service_status(session_mgr__resp_header,
         &session_mgr__l_resp_status);
      if ((session_mgr__l_session_state != constants__e_session_closed) &&
         (session_mgr__l_resp_status == constants_statuscodes_bs__e_sc_ok)) {
         switch (session_mgr__resp_typ) {
         case constants__e_msg_session_create_resp:
            if (session_mgr__l_session_state == constants__e_session_creating) {
               session_core__get_session_channel(*session_mgr__session,
                  &session_mgr__l_session_channel);
               if (session_mgr__l_session_channel == session_mgr__channel) {
                  message_in_bs__read_create_session_msg_session_token(session_mgr__resp_msg,
                     &session_mgr__l_session_token);
                  session_core__client_create_session_resp_sm(session_mgr__channel,
                     *session_mgr__session,
                     session_mgr__l_session_token,
                     session_mgr__resp_msg,
                     &session_mgr__l_bret);
                  session_mgr__l_status_reason = constants_statuscodes_bs__e_sc_bad_security_checks_failed;
                  session_core__get_session_state_or_closed(*session_mgr__session,
                     &session_mgr__l_session_state);
                  if (session_mgr__l_session_state == constants__e_session_created) {
                     session_core__get_session_user_client(*session_mgr__session,
                        &session_mgr__l_session_user_token);
                     session_core__client_gen_activate_user_session_internal_event(*session_mgr__session,
                        session_mgr__l_session_user_token);
                  }
                  else {
                     session_mgr__l_bret = false;
                  }
               }
               else {
                  session_mgr__l_status_reason = constants_statuscodes_bs__e_sc_bad_secure_channel_id_invalid;
                  ;
               }
            }
            else {
               session_mgr__l_status_reason = constants_statuscodes_bs__e_sc_bad_invalid_state;
               ;
            }
            break;
         case constants__e_msg_session_activate_resp:
            if ((session_mgr__l_session_state == constants__e_session_userActivating) ||
               (session_mgr__l_session_state == constants__e_session_scActivating)) {
               session_core__get_session_channel(*session_mgr__session,
                  &session_mgr__l_session_channel);
               if (session_mgr__l_session_channel == session_mgr__channel) {
                  session_core__client_activate_session_resp_sm(session_mgr__channel,
                     *session_mgr__session,
                     session_mgr__resp_msg,
                     &session_mgr__l_bret);
                  session_mgr__l_status_reason = constants_statuscodes_bs__e_sc_bad_security_checks_failed;
               }
               else {
                  session_mgr__l_status_reason = constants_statuscodes_bs__e_sc_bad_secure_channel_id_invalid;
                  ;
               }
            }
            else {
               session_mgr__l_status_reason = constants_statuscodes_bs__e_sc_bad_invalid_state;
               ;
            }
            break;
         case constants__e_msg_session_close_resp:
            if (session_mgr__l_session_state == constants__e_session_closing) {
               session_core__get_session_channel(*session_mgr__session,
                  &session_mgr__l_session_channel);
               if (session_mgr__l_session_channel == session_mgr__channel) {
                  session_request_handle_bs__client_remove_all_request_handles(*session_mgr__session);
                  session_core__client_close_session_resp_sm(session_mgr__channel,
                     *session_mgr__session,
                     session_mgr__resp_msg);
                  session_mgr__l_bret = true;
                  session_mgr__l_status_reason = constants_statuscodes_bs__e_sc_ok;
               }
               else {
                  session_mgr__l_status_reason = constants_statuscodes_bs__e_sc_bad_secure_channel_id_invalid;
                  ;
               }
            }
            else {
               session_mgr__l_status_reason = constants_statuscodes_bs__e_sc_bad_invalid_state;
               ;
            }
            break;
         case constants__e_msg_session_cancel_resp:
            session_mgr__l_status_reason = constants_statuscodes_bs__e_sc_bad_service_unsupported;
            ;
            break;
         default:
            session_mgr__l_status_reason = constants_statuscodes_bs__e_sc_bad_service_unsupported;
            ;
            break;
         }
         session_mgr__local_client_close_session_if_needed(session_mgr__l_bret,
            *session_mgr__session,
            session_mgr__l_status_reason);
      }
      else if (session_mgr__l_session_state != constants__e_session_closed) {
         session_mgr__local_client_close_session(*session_mgr__session,
            session_mgr__l_resp_status);
      }
      else {
         ;
      }
   }
}

void session_mgr__server_receive_session_req(
   const constants__t_channel_i session_mgr__channel,
   const constants__t_session_token_i session_mgr__session_token,
   const constants__t_msg_i session_mgr__req_msg,
   const constants__t_msg_type_i session_mgr__req_typ,
   const constants__t_msg_i session_mgr__resp_msg,
   constants__t_session_i * const session_mgr__session,
   constants_statuscodes_bs__t_StatusCode_i * const session_mgr__service_ret) {
   {
      t_bool session_mgr__l_valid_session;
      constants__t_sessionState session_mgr__l_session_state;
      constants__t_channel_i session_mgr__l_session_channel;
      t_bool session_mgr__l_is_valid_user_token;
      constants__t_user_token_i session_mgr__l_user_token;
      constants__t_channel_config_idx_i session_mgr__l_channel_config_idx;
      constants__t_endpoint_config_idx_i session_mgr__l_endpoint_config_idx;
      t_bool session_mgr__l_has_user_token_policy_available;
      t_bool session_mgr__l_timer_creation_ok;
      constants__t_user_i session_mgr__l_user;
      
      *session_mgr__session = constants__c_session_indet;
      *session_mgr__service_ret = constants_statuscodes_bs__c_StatusCode_indet;
      switch (session_mgr__req_typ) {
      case constants__e_msg_session_create_req:
         channel_mgr__get_channel_info(session_mgr__channel,
            &session_mgr__l_channel_config_idx);
         channel_mgr__server_get_endpoint_config(session_mgr__channel,
            &session_mgr__l_endpoint_config_idx);
         session_core__has_user_token_policy_available(session_mgr__l_channel_config_idx,
            session_mgr__l_endpoint_config_idx,
            &session_mgr__l_has_user_token_policy_available);
         if (session_mgr__l_has_user_token_policy_available == true) {
            session_core__server_create_session_req_and_resp_sm(session_mgr__channel,
               session_mgr__req_msg,
               session_mgr__resp_msg,
               session_mgr__session,
               session_mgr__service_ret);
            if (*session_mgr__service_ret == constants_statuscodes_bs__e_sc_ok) {
               session_core__server_session_timeout_start_timer(*session_mgr__session,
                  session_mgr__resp_msg,
                  &session_mgr__l_timer_creation_ok);
               if (session_mgr__l_timer_creation_ok == false) {
                  *session_mgr__service_ret = constants_statuscodes_bs__e_sc_bad_resource_unavailable;
                  session_core__server_close_session_sm(*session_mgr__session,
                     constants_statuscodes_bs__e_sc_bad_resource_unavailable);
               }
            }
         }
         else {
            *session_mgr__service_ret = constants_statuscodes_bs__e_sc_bad_service_unsupported;
         }
         break;
      case constants__e_msg_session_activate_req:
         session_core__server_get_session_from_token(session_mgr__session_token,
            session_mgr__session);
         session_core__is_valid_session(*session_mgr__session,
            &session_mgr__l_valid_session);
         session_core__get_session_state_or_closed(*session_mgr__session,
            &session_mgr__l_session_state);
         if (session_mgr__l_valid_session == true) {
            if (((session_mgr__l_session_state == constants__e_session_created) ||
               (session_mgr__l_session_state == constants__e_session_userActivated)) ||
               (session_mgr__l_session_state == constants__e_session_scOrphaned)) {
               message_in_bs__read_activate_req_msg_identity_token(session_mgr__req_msg,
                  &session_mgr__l_is_valid_user_token,
                  &session_mgr__l_user_token);
               if (session_mgr__l_is_valid_user_token == true) {
                  session_core__allocate_authenticated_user(session_mgr__channel,
                     *session_mgr__session,
                     session_mgr__l_user_token,
                     session_mgr__service_ret,
                     &session_mgr__l_user);
                  if (*session_mgr__service_ret == constants_statuscodes_bs__e_sc_ok) {
                     session_core__server_activate_session_req_and_resp_sm(session_mgr__channel,
                        *session_mgr__session,
                        session_mgr__l_user,
                        session_mgr__req_msg,
                        session_mgr__resp_msg,
                        session_mgr__service_ret);
                     if (*session_mgr__service_ret != constants_statuscodes_bs__e_sc_ok) {
                        session_core__deallocate_user(session_mgr__l_user);
                     }
                  }
               }
               else {
                  *session_mgr__service_ret = constants_statuscodes_bs__e_sc_bad_identity_token_invalid;
               }
               if (*session_mgr__service_ret == constants_statuscodes_bs__e_sc_ok) {
                  session_core__server_session_timeout_msg_received(*session_mgr__session);
               }
            }
            else {
               session_core__server_close_session_sm(*session_mgr__session,
                  constants_statuscodes_bs__e_sc_bad_invalid_state);
               *session_mgr__service_ret = constants_statuscodes_bs__e_sc_bad_invalid_state;
            }
         }
         else {
            *session_mgr__service_ret = constants_statuscodes_bs__e_sc_bad_session_id_invalid;
         }
         break;
      case constants__e_msg_session_close_req:
         session_core__server_get_session_from_token(session_mgr__session_token,
            session_mgr__session);
         session_core__is_valid_session(*session_mgr__session,
            &session_mgr__l_valid_session);
         session_core__get_session_state_or_closed(*session_mgr__session,
            &session_mgr__l_session_state);
         if (session_mgr__l_valid_session == true) {
            if (((session_mgr__l_session_state == constants__e_session_created) ||
               (session_mgr__l_session_state == constants__e_session_userActivating)) ||
               (session_mgr__l_session_state == constants__e_session_userActivated)) {
               session_core__get_session_channel(*session_mgr__session,
                  &session_mgr__l_session_channel);
               if (session_mgr__l_session_channel == session_mgr__channel) {
                  session_core__server_close_session_req_and_resp_sm(session_mgr__channel,
                     *session_mgr__session,
                     session_mgr__req_msg,
                     session_mgr__resp_msg,
                     session_mgr__service_ret);
               }
               else {
                  session_core__server_close_session_sm(*session_mgr__session,
                     constants_statuscodes_bs__e_sc_bad_secure_channel_id_invalid);
                  *session_mgr__service_ret = constants_statuscodes_bs__e_sc_bad_secure_channel_id_invalid;
               }
            }
            else {
               session_core__server_close_session_sm(*session_mgr__session,
                  constants_statuscodes_bs__e_sc_bad_invalid_state);
               *session_mgr__service_ret = constants_statuscodes_bs__e_sc_bad_invalid_state;
            }
         }
         else {
            *session_mgr__service_ret = constants_statuscodes_bs__e_sc_bad_session_id_invalid;
         }
         break;
      default:
         *session_mgr__service_ret = constants_statuscodes_bs__e_sc_bad_service_unsupported;
         break;
      }
   }
}

void session_mgr__client_validate_session_service_req(
   const constants__t_session_i session_mgr__session,
   const constants__t_client_request_handle_i session_mgr__req_handle,
   constants_statuscodes_bs__t_StatusCode_i * const session_mgr__ret,
   constants__t_channel_i * const session_mgr__channel,
   constants__t_session_token_i * const session_mgr__session_token) {
   {
      constants__t_sessionState session_mgr__l_session_state;
      constants_statuscodes_bs__t_StatusCode_i session_mgr__l_ret;
      
      *session_mgr__session_token = constants__c_session_token_indet;
      *session_mgr__channel = constants__c_channel_indet;
      session_core__get_session_state_or_closed(session_mgr__session,
         &session_mgr__l_session_state);
      if (session_mgr__l_session_state == constants__e_session_userActivated) {
         session_core__client_get_token_from_session(session_mgr__session,
            session_mgr__session_token);
         session_core__get_session_channel(session_mgr__session,
            session_mgr__channel);
         session_request_handle_bs__client_add_session_request_handle(session_mgr__session,
            session_mgr__req_handle);
         session_mgr__l_ret = constants_statuscodes_bs__e_sc_ok;
      }
      else {
         session_mgr__l_ret = constants_statuscodes_bs__e_sc_bad_invalid_argument;
      }
      *session_mgr__ret = session_mgr__l_ret;
   }
}

void session_mgr__client_validate_session_service_resp(
   const constants__t_channel_i session_mgr__channel,
   const constants__t_client_request_handle_i session_mgr__req_handle,
   t_bool * const session_mgr__bres,
   constants__t_session_i * const session_mgr__session) {
   {
      constants__t_session_i session_mgr__l_session;
      t_bool session_mgr__l_valid_session;
      constants__t_sessionState session_mgr__l_session_state;
      constants__t_channel_i session_mgr__l_session_channel;
      
      *session_mgr__session = constants__c_session_indet;
      session_request_handle_bs__client_get_session_and_remove_request_handle(session_mgr__req_handle,
         &session_mgr__l_session);
      session_core__is_valid_session(session_mgr__l_session,
         &session_mgr__l_valid_session);
      session_core__get_session_state_or_closed(session_mgr__l_session,
         &session_mgr__l_session_state);
      if (session_mgr__l_valid_session == true) {
         session_core__get_session_channel(session_mgr__l_session,
            &session_mgr__l_session_channel);
         if ((session_mgr__l_session_state == constants__e_session_userActivated) &&
            (session_mgr__l_session_channel == session_mgr__channel)) {
            session_core__is_session_valid_for_service(session_mgr__channel,
               session_mgr__l_session,
               session_mgr__bres);
         }
         else {
            *session_mgr__bres = false;
         }
         if (*session_mgr__bres == false) {
            if (session_mgr__l_session_state != constants__e_session_userActivated) {
               session_mgr__local_client_close_session(session_mgr__l_session,
                  constants_statuscodes_bs__e_sc_bad_invalid_state);
            }
            else {
               session_mgr__local_client_close_session(session_mgr__l_session,
                  constants_statuscodes_bs__e_sc_bad_secure_channel_id_invalid);
            }
         }
         else {
            *session_mgr__session = session_mgr__l_session;
         }
      }
      else {
         *session_mgr__bres = false;
      }
   }
}

void session_mgr__server_validate_session_service_req(
   const constants__t_channel_i session_mgr__channel,
   const constants__t_session_token_i session_mgr__session_token,
   t_bool * const session_mgr__is_valid_res,
   constants__t_session_i * const session_mgr__session,
   constants_statuscodes_bs__t_StatusCode_i * const session_mgr__status_code_err) {
   {
      constants__t_session_i session_mgr__l_session;
      t_bool session_mgr__l_valid_session;
      constants__t_sessionState session_mgr__l_session_state;
      constants__t_channel_i session_mgr__l_session_channel;
      constants__t_user_i session_mgr__l_user;
      
      session_core__server_get_session_from_token(session_mgr__session_token,
         &session_mgr__l_session);
      session_core__is_valid_session(session_mgr__l_session,
         &session_mgr__l_valid_session);
      session_core__get_session_state_or_closed(session_mgr__l_session,
         &session_mgr__l_session_state);
      *session_mgr__session = constants__c_session_indet;
      if (session_mgr__l_valid_session == true) {
         session_core__get_session_channel(session_mgr__l_session,
            &session_mgr__l_session_channel);
         if ((session_mgr__l_session_state == constants__e_session_userActivated) &&
            (session_mgr__l_session_channel == session_mgr__channel)) {
            session_core__is_session_valid_for_service(session_mgr__channel,
               session_mgr__l_session,
               session_mgr__is_valid_res);
            *session_mgr__status_code_err = constants_statuscodes_bs__e_sc_ok;
            *session_mgr__session = session_mgr__l_session;
            if (*session_mgr__is_valid_res == true) {
               session_core__get_session_user_server(session_mgr__l_session,
                  &session_mgr__l_user);
               app_cb_call_context_bs__set_app_call_context_session(session_mgr__l_user);
               session_core__server_session_timeout_msg_received(session_mgr__l_session);
            }
         }
         else {
            if (session_mgr__l_session_channel != session_mgr__channel) {
               *session_mgr__status_code_err = constants_statuscodes_bs__e_sc_bad_secure_channel_id_invalid;
            }
            else {
               if (session_mgr__l_session_state == constants__e_session_created) {
                  *session_mgr__status_code_err = constants_statuscodes_bs__e_sc_bad_session_not_activated;
               }
               else {
                  *session_mgr__status_code_err = constants_statuscodes_bs__e_sc_bad_invalid_state;
               }
               session_core__server_close_session_sm(session_mgr__l_session,
                  *session_mgr__status_code_err);
            }
            *session_mgr__is_valid_res = false;
         }
      }
      else {
         *session_mgr__is_valid_res = false;
         *session_mgr__status_code_err = constants_statuscodes_bs__e_sc_bad_session_id_invalid;
      }
   }
}

void session_mgr__server_validate_session_service_resp(
   const constants__t_session_i session_mgr__session,
   t_bool * const session_mgr__is_valid_res,
   constants_statuscodes_bs__t_StatusCode_i * const session_mgr__status_code_err,
   constants__t_channel_i * const session_mgr__channel) {
   {
      t_bool session_mgr__l_valid_session;
      constants__t_sessionState session_mgr__l_session_state;
      constants__t_channel_i session_mgr__l_session_channel;
      
      *session_mgr__channel = constants__c_channel_indet;
      session_core__is_valid_session(session_mgr__session,
         &session_mgr__l_valid_session);
      session_core__get_session_state_or_closed(session_mgr__session,
         &session_mgr__l_session_state);
      if (session_mgr__l_valid_session == true) {
         session_core__get_session_channel(session_mgr__session,
            &session_mgr__l_session_channel);
         if (session_mgr__l_session_state == constants__e_session_userActivated) {
            session_core__is_session_valid_for_service(session_mgr__l_session_channel,
               session_mgr__session,
               session_mgr__is_valid_res);
            *session_mgr__status_code_err = constants_statuscodes_bs__e_sc_ok;
            *session_mgr__channel = session_mgr__l_session_channel;
         }
         else {
            *session_mgr__is_valid_res = false;
            if (session_mgr__l_session_channel == *session_mgr__channel) {
               *session_mgr__status_code_err = constants_statuscodes_bs__e_sc_bad_invalid_state;
            }
            else {
               *session_mgr__status_code_err = constants_statuscodes_bs__e_sc_bad_secure_channel_id_invalid;
            }
         }
      }
      else {
         *session_mgr__is_valid_res = false;
         *session_mgr__status_code_err = constants_statuscodes_bs__e_sc_bad_session_id_invalid;
      }
   }
}

void session_mgr__client_create_session_req(
   const constants__t_session_i session_mgr__session,
   const constants__t_channel_i session_mgr__channel,
   const constants__t_client_request_handle_i session_mgr__req_handle,
   const constants__t_msg_i session_mgr__create_req_msg,
   t_bool * const session_mgr__bret) {
   {
      t_bool session_mgr__l_valid_session;
      constants__t_sessionState session_mgr__l_session_state;
      t_bool session_mgr__l_valid;
      t_bool session_mgr__l_bret;
      
      session_mgr__l_bret = false;
      session_core__is_valid_session(session_mgr__session,
         &session_mgr__l_valid_session);
      session_core__get_session_state_or_closed(session_mgr__session,
         &session_mgr__l_session_state);
      if (session_mgr__l_valid_session == true) {
         if (session_mgr__l_session_state == constants__e_session_init) {
            session_core__client_create_session_req_sm(session_mgr__session,
               session_mgr__channel,
               session_mgr__create_req_msg,
               &session_mgr__l_valid);
            if (session_mgr__l_valid == true) {
               session_request_handle_bs__client_add_session_request_handle(session_mgr__session,
                  session_mgr__req_handle);
               session_mgr__l_bret = true;
            }
            else {
               ;
            }
         }
         else {
            ;
         }
      }
      else {
         ;
      }
      *session_mgr__bret = session_mgr__l_bret;
   }
}

void session_mgr__client_async_activate_new_session_without_channel(
   const constants__t_channel_config_idx_i session_mgr__channel_config_idx,
   const constants__t_user_token_i session_mgr__p_user_token,
   const constants__t_session_application_context_i session_mgr__app_context,
   t_bool * const session_mgr__bres) {
   {
      constants__t_session_i session_mgr__l_session;
      constants__t_sessionState session_mgr__l_session_state;
      
      session_core__client_init_session_sm(&session_mgr__l_session);
      session_core__get_session_state_or_closed(session_mgr__l_session,
         &session_mgr__l_session_state);
      if (session_mgr__l_session_state == constants__e_session_init) {
         *session_mgr__bres = true;
         session_core__set_session_to_create(session_mgr__l_session,
            session_mgr__channel_config_idx);
         session_core__set_session_app_context(session_mgr__l_session,
            session_mgr__app_context);
         session_core__set_session_user_client(session_mgr__l_session,
            session_mgr__p_user_token);
      }
      else {
         *session_mgr__bres = false;
      }
   }
}

void session_mgr__client_async_activate_new_session_with_channel(
   const constants__t_channel_config_idx_i session_mgr__channel_config_idx,
   const constants__t_channel_i session_mgr__channel,
   const constants__t_user_token_i session_mgr__p_user_token,
   const constants__t_session_application_context_i session_mgr__app_context,
   t_bool * const session_mgr__bres) {
   {
      constants__t_session_i session_mgr__l_session;
      constants__t_sessionState session_mgr__l_session_state;
      
      channel_mgr__channel_do_nothing(session_mgr__channel);
      session_core__client_init_session_sm(&session_mgr__l_session);
      session_core__get_session_state_or_closed(session_mgr__l_session,
         &session_mgr__l_session_state);
      if (session_mgr__l_session_state == constants__e_session_init) {
         *session_mgr__bres = true;
         session_core__client_gen_create_session_internal_event(session_mgr__l_session,
            session_mgr__channel_config_idx);
         session_core__set_session_user_client(session_mgr__l_session,
            session_mgr__p_user_token);
         session_core__set_session_app_context(session_mgr__l_session,
            session_mgr__app_context);
      }
      else {
         *session_mgr__bres = false;
      }
   }
}

void session_mgr__client_user_activate_session_req(
   const constants__t_session_i session_mgr__session,
   const constants__t_client_request_handle_i session_mgr__req_handle,
   const constants__t_user_token_i session_mgr__p_user_token,
   const constants__t_msg_i session_mgr__activate_req_msg,
   constants_statuscodes_bs__t_StatusCode_i * const session_mgr__ret,
   constants__t_channel_i * const session_mgr__channel,
   constants__t_session_token_i * const session_mgr__session_token) {
   {
      t_bool session_mgr__l_valid_session;
      constants__t_sessionState session_mgr__l_session_state;
      constants_statuscodes_bs__t_StatusCode_i session_mgr__l_ret;
      
      session_core__is_valid_session(session_mgr__session,
         &session_mgr__l_valid_session);
      if (session_mgr__l_valid_session == true) {
         session_core__get_session_state_or_closed(session_mgr__session,
            &session_mgr__l_session_state);
         if ((session_mgr__l_session_state == constants__e_session_created) ||
            (session_mgr__l_session_state == constants__e_session_userActivated)) {
            session_core__client_user_activate_session_req_sm(session_mgr__session,
               session_mgr__p_user_token,
               session_mgr__activate_req_msg,
               &session_mgr__l_ret,
               session_mgr__channel,
               session_mgr__session_token);
            if (session_mgr__l_ret == constants_statuscodes_bs__e_sc_ok) {
               session_request_handle_bs__client_add_session_request_handle(session_mgr__session,
                  session_mgr__req_handle);
            }
         }
         else {
            session_mgr__l_ret = constants_statuscodes_bs__e_sc_bad_invalid_state;
            *session_mgr__channel = constants__c_channel_indet;
            *session_mgr__session_token = constants__c_session_token_indet;
         }
      }
      else {
         session_mgr__l_ret = constants_statuscodes_bs__e_sc_bad_invalid_argument;
         *session_mgr__channel = constants__c_channel_indet;
         *session_mgr__session_token = constants__c_session_token_indet;
      }
      *session_mgr__ret = session_mgr__l_ret;
   }
}

void session_mgr__client_sc_activate_session_req(
   const constants__t_session_i session_mgr__session,
   const constants__t_client_request_handle_i session_mgr__req_handle,
   const constants__t_channel_i session_mgr__channel,
   const constants__t_msg_i session_mgr__activate_req_msg,
   constants_statuscodes_bs__t_StatusCode_i * const session_mgr__ret,
   constants__t_session_token_i * const session_mgr__session_token) {
   {
      t_bool session_mgr__l_valid_session;
      constants__t_sessionState session_mgr__l_session_state;
      constants_statuscodes_bs__t_StatusCode_i session_mgr__l_ret;
      
      session_core__is_valid_session(session_mgr__session,
         &session_mgr__l_valid_session);
      if (session_mgr__l_valid_session == true) {
         session_core__get_session_state_or_closed(session_mgr__session,
            &session_mgr__l_session_state);
         if ((session_mgr__l_session_state == constants__e_session_scOrphaned) ||
            (session_mgr__l_session_state == constants__e_session_userActivated)) {
            session_core__client_sc_activate_session_req_sm(session_mgr__session,
               session_mgr__channel,
               session_mgr__activate_req_msg,
               session_mgr__session_token);
            session_request_handle_bs__client_add_session_request_handle(session_mgr__session,
               session_mgr__req_handle);
            session_mgr__l_ret = constants_statuscodes_bs__e_sc_ok;
         }
         else {
            session_mgr__l_ret = constants_statuscodes_bs__e_sc_bad_invalid_state;
            *session_mgr__session_token = constants__c_session_token_indet;
         }
      }
      else {
         session_mgr__l_ret = constants_statuscodes_bs__e_sc_bad_invalid_argument;
         *session_mgr__session_token = constants__c_session_token_indet;
      }
      *session_mgr__ret = session_mgr__l_ret;
   }
}

void session_mgr__client_channel_connected_event_session(
   const constants__t_channel_config_idx_i session_mgr__channel_config_idx,
   const constants__t_channel_i session_mgr__channel) {
   channel_mgr__channel_do_nothing(session_mgr__channel);
   session_mgr__local_client_activate_sessions_on_SC_connection(session_mgr__channel_config_idx);
}

void session_mgr__client_close_session_req(
   const constants__t_session_i session_mgr__session,
   const constants__t_client_request_handle_i session_mgr__req_handle,
   const constants__t_msg_i session_mgr__close_req_msg,
   constants_statuscodes_bs__t_StatusCode_i * const session_mgr__ret,
   constants__t_channel_i * const session_mgr__channel,
   constants__t_session_token_i * const session_mgr__session_token) {
   {
      t_bool session_mgr__l_valid_session;
      constants__t_sessionState session_mgr__l_session_state;
      constants_statuscodes_bs__t_StatusCode_i session_mgr__l_ret;
      
      session_core__is_valid_session(session_mgr__session,
         &session_mgr__l_valid_session);
      if (session_mgr__l_valid_session == true) {
         session_core__get_session_state_or_closed(session_mgr__session,
            &session_mgr__l_session_state);
         if (((session_mgr__l_session_state == constants__e_session_created) ||
            (session_mgr__l_session_state == constants__e_session_userActivating)) ||
            (session_mgr__l_session_state == constants__e_session_userActivated)) {
            session_core__client_close_session_req_sm(session_mgr__session,
               session_mgr__close_req_msg,
               &session_mgr__l_ret,
               session_mgr__channel,
               session_mgr__session_token);
            if (session_mgr__l_ret != constants_statuscodes_bs__e_sc_ok) {
               session_mgr__local_client_close_session(session_mgr__session,
                  session_mgr__l_ret);
               session_mgr__l_ret = constants_statuscodes_bs__e_sc_bad_unexpected_error;
            }
            if (session_mgr__l_ret == constants_statuscodes_bs__e_sc_ok) {
               session_request_handle_bs__client_add_session_request_handle(session_mgr__session,
                  session_mgr__req_handle);
            }
         }
         else {
            session_mgr__l_ret = constants_statuscodes_bs__e_sc_bad_invalid_state;
            session_mgr__local_client_close_session(session_mgr__session,
               session_mgr__l_ret);
            *session_mgr__channel = constants__c_channel_indet;
            *session_mgr__session_token = constants__c_session_token_indet;
         }
      }
      else {
         session_mgr__l_ret = constants_statuscodes_bs__e_sc_bad_invalid_argument;
         *session_mgr__channel = constants__c_channel_indet;
         *session_mgr__session_token = constants__c_session_token_indet;
      }
      *session_mgr__ret = session_mgr__l_ret;
   }
}

void session_mgr__client_close_sessions_on_final_connection_failure(
   const constants__t_channel_config_idx_i session_mgr__channel_config_idx) {
   session_mgr__local_client_close_sessions_on_SC_final_connection_failure(session_mgr__channel_config_idx);
}

void session_mgr__client_close_session(
   const constants__t_session_i session_mgr__session,
   const constants_statuscodes_bs__t_StatusCode_i session_mgr__sc_reason) {
   {
      t_bool session_mgr__l_valid_session;
      
      session_core__is_valid_session(session_mgr__session,
         &session_mgr__l_valid_session);
      if (session_mgr__l_valid_session == true) {
         session_mgr__local_client_close_session(session_mgr__session,
            session_mgr__sc_reason);
      }
   }
}

void session_mgr__server_evaluate_session_timeout(
   const constants__t_session_i session_mgr__session) {
   {
      t_bool session_mgr__l_valid_session;
      t_bool session_mgr__l_session_expired;
      
      session_core__is_valid_session(session_mgr__session,
         &session_mgr__l_valid_session);
      if (session_mgr__l_valid_session == true) {
         session_core__server_session_timeout_evaluation(session_mgr__session,
            &session_mgr__l_session_expired);
         if (session_mgr__l_session_expired == true) {
            session_core__server_close_session_sm(session_mgr__session,
               constants_statuscodes_bs__e_sc_bad_timeout);
         }
      }
   }
}

void session_mgr__server_close_session(
   const constants__t_session_i session_mgr__session,
   const constants_statuscodes_bs__t_StatusCode_i session_mgr__sc_reason) {
   {
      t_bool session_mgr__l_valid_session;
      
      session_core__is_valid_session(session_mgr__session,
         &session_mgr__l_valid_session);
      if (session_mgr__l_valid_session == true) {
         session_core__server_close_session_sm(session_mgr__session,
            session_mgr__sc_reason);
      }
   }
}

void session_mgr__session_get_endpoint_config(
   const constants__t_session_i session_mgr__p_session,
   constants__t_endpoint_config_idx_i * const session_mgr__endpoint_config_idx) {
   {
      constants__t_channel_i session_mgr__l_channel;
      t_bool session_mgr__l_continue;
      
      *session_mgr__endpoint_config_idx = constants__c_endpoint_config_idx_indet;
      session_core__getall_valid_session_channel(session_mgr__p_session,
         &session_mgr__l_continue,
         &session_mgr__l_channel);
      if (session_mgr__l_continue == true) {
         channel_mgr__server_get_endpoint_config(session_mgr__l_channel,
            session_mgr__endpoint_config_idx);
      }
   }
}

