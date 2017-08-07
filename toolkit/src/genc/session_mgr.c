/******************************************************************************

 File Name            : session_mgr.c

 Date                 : 07/08/2017 16:37:07

 C Translator Version : tradc Java V1.0 (14/03/2012)

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
void session_mgr__client_receive_session_resp(
   const constants__t_channel_i session_mgr__channel,
   const constants__t_request_handle_i session_mgr__req_handle,
   const constants__t_msg_type_i session_mgr__resp_typ,
   const constants__t_msg_header_i session_mgr__resp_header,
   const constants__t_msg_i session_mgr__resp_msg,
   constants__t_session_i * const session_mgr__session) {
   {
      constants__t_session_i session_mgr__l_session;
      constants__t_session_token_i session_mgr__l_session_token;
      constants__t_sessionState session_mgr__l_session_state;
      constants__t_channel_i session_mgr__l_session_channel;
      constants__t_StatusCode_i session_mgr__l_resp_status;
      constants__t_StatusCode_i session_mgr__l_ret;
      
      session_core__get_session_from_req_handle(session_mgr__req_handle,
         &session_mgr__l_session);
      session_core__get_session_state_or_closed(session_mgr__l_session,
         &session_mgr__l_session_state);
      message_in_bs__read_msg_resp_header_service_status(session_mgr__resp_header,
         &session_mgr__l_resp_status);
      if (session_mgr__l_resp_status == constants__e_sc_ok) {
         switch (session_mgr__resp_typ) {
         case constants__e_msg_session_create_resp:
            if (session_mgr__l_session_state == constants__e_session_creating) {
               session_core__get_session_channel(session_mgr__l_session,
                  &session_mgr__l_session_channel);
               if (session_mgr__l_session_channel == session_mgr__channel) {
                  message_in_bs__read_create_session_msg_session_token(session_mgr__resp_msg,
                     &session_mgr__l_session_token);
                  session_core__cli_create_resp(session_mgr__channel,
                     session_mgr__l_session,
                     session_mgr__req_handle,
                     session_mgr__l_session_token,
                     session_mgr__resp_msg,
                     &session_mgr__l_ret);
               }
               else {
                  session_mgr__l_ret = constants__e_sc_bad_secure_channel_id_invalid;
               }
            }
            else {
               session_mgr__l_ret = constants__e_sc_bad_invalid_state;
            }
            break;
         case constants__e_msg_session_activate_resp:
            if ((session_mgr__l_session_state == constants__e_session_userActivating) ||
               (session_mgr__l_session_state == constants__e_session_scActivating)) {
               session_core__get_session_channel(session_mgr__l_session,
                  &session_mgr__l_session_channel);
               if (session_mgr__l_session_channel == session_mgr__channel) {
                  session_core__cli_activate_resp(session_mgr__channel,
                     session_mgr__l_session,
                     session_mgr__req_handle,
                     session_mgr__resp_msg,
                     &session_mgr__l_ret);
               }
               else {
                  session_mgr__l_ret = constants__e_sc_bad_secure_channel_id_invalid;
               }
            }
            else {
               session_mgr__l_ret = constants__e_sc_bad_invalid_state;
            }
            break;
         case constants__e_msg_session_close_resp:
            if (session_mgr__l_session_state == constants__e_session_closing) {
               session_core__get_session_channel(session_mgr__l_session,
                  &session_mgr__l_session_channel);
               if (session_mgr__l_session_channel == session_mgr__channel) {
                  session_core__cli_close_resp(session_mgr__channel,
                     session_mgr__l_session,
                     session_mgr__req_handle,
                     session_mgr__resp_msg);
                  session_mgr__l_ret = constants__e_sc_ok;
               }
               else {
                  session_mgr__l_ret = constants__e_sc_bad_secure_channel_id_invalid;
               }
            }
            else {
               session_mgr__l_ret = constants__e_sc_bad_invalid_state;
            }
            break;
         default:
            session_mgr__l_ret = constants__e_sc_bad_unexpected_error;
            break;
         }
      }
      else {
         session_mgr__l_ret = session_mgr__l_resp_status;
      }
      if (session_mgr__l_ret != constants__e_sc_ok) {
         session_core__cli_close_session(*session_mgr__session);
      }
      *session_mgr__session = session_mgr__l_session;
   }
}

void session_mgr__server_receive_session_req(
   const constants__t_channel_i session_mgr__channel,
   const constants__t_session_token_i session_mgr__session_token,
   const constants__t_msg_i session_mgr__req_msg,
   const constants__t_msg_type_i session_mgr__req_typ,
   const constants__t_msg_i session_mgr__resp_msg,
   constants__t_session_i * const session_mgr__session,
   constants__t_StatusCode_i * const session_mgr__service_ret) {
   {
      t_bool session_mgr__l_valid_session;
      constants__t_sessionState session_mgr__l_session_state;
      constants__t_channel_i session_mgr__l_session_channel;
      constants__t_user_i session_mgr__l_user;
      t_bool session_mgr__l_valid_user;
      
      switch (session_mgr__req_typ) {
      case constants__e_msg_session_create_req:
         session_core__srv_create_req_and_resp(session_mgr__channel,
            session_mgr__req_msg,
            session_mgr__resp_msg,
            session_mgr__session,
            session_mgr__service_ret);
         break;
      case constants__e_msg_session_activate_req:
         session_core__get_session_from_token(session_mgr__session_token,
            session_mgr__session);
         session_core__is_valid_session(*session_mgr__session,
            &session_mgr__l_valid_session);
         session_core__get_session_state_or_closed(*session_mgr__session,
            &session_mgr__l_session_state);
         if (session_mgr__l_valid_session == true) {
            if ((((session_mgr__l_session_state == constants__e_session_created) ||
               (session_mgr__l_session_state == constants__e_session_userActivated)) ||
               (session_mgr__l_session_state == constants__e_session_scOrphaned)) ||
               (session_mgr__l_session_state == constants__e_session_userActivated)) {
               message_in_bs__read_activate_msg_user(session_mgr__req_msg,
                  &session_mgr__l_user);
               session_core__is_valid_user(session_mgr__l_user,
                  &session_mgr__l_valid_user);
               if (session_mgr__l_valid_user == true) {
                  session_core__srv_activate_req_and_resp(session_mgr__channel,
                     *session_mgr__session,
                     session_mgr__l_user,
                     session_mgr__req_msg,
                     session_mgr__resp_msg,
                     session_mgr__service_ret);
               }
               else {
                  *session_mgr__service_ret = constants__e_sc_bad_identity_token_invalid;
               }
               if (*session_mgr__service_ret != constants__e_sc_ok) {
                  session_core__server_close_session(*session_mgr__session);
               }
            }
            else {
               session_core__server_close_session(*session_mgr__session);
               *session_mgr__service_ret = constants__e_sc_bad_invalid_state;
            }
         }
         else {
            *session_mgr__service_ret = constants__e_sc_bad_session_id_invalid;
         }
         break;
      case constants__e_msg_session_close_req:
         session_core__get_session_from_token(session_mgr__session_token,
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
                  session_core__srv_close_req_and_resp(session_mgr__channel,
                     *session_mgr__session,
                     session_mgr__req_msg,
                     session_mgr__resp_msg,
                     session_mgr__service_ret);
               }
               else {
                  session_core__server_close_session(*session_mgr__session);
                  *session_mgr__service_ret = constants__e_sc_bad_secure_channel_id_invalid;
               }
            }
            else {
               session_core__server_close_session(*session_mgr__session);
               *session_mgr__service_ret = constants__e_sc_bad_invalid_state;
            }
         }
         else {
            *session_mgr__service_ret = constants__e_sc_bad_session_id_invalid;
         }
         break;
      default:
         break;
      }
   }
}

void session_mgr__client_validate_session_service_req(
   const constants__t_session_i session_mgr__session,
   const constants__t_request_handle_i session_mgr__req_handle,
   const constants__t_msg_i session_mgr__req_msg,
   constants__t_StatusCode_i * const session_mgr__ret,
   constants__t_channel_i * const session_mgr__channel,
   constants__t_session_token_i * const session_mgr__session_token) {
   {
      constants__t_sessionState session_mgr__l_session_state;
      constants__t_StatusCode_i session_mgr__ret;
      
      *session_mgr__session_token = constants__c_session_token_indet;
      *session_mgr__channel = constants__c_channel_indet;
      session_core__get_session_state_or_closed(session_mgr__session,
         &session_mgr__l_session_state);
      if (session_mgr__l_session_state == constants__e_session_userActivated) {
         session_core__cli_new_session_service_req(session_mgr__session,
            session_mgr__req_handle,
            &session_mgr__ret,
            session_mgr__channel,
            session_mgr__session_token);
      }
      else {
         session_mgr__ret = constants__e_sc_bad_invalid_argument;
      }
   }
}

void session_mgr__client_validate_session_service_resp(
   const constants__t_channel_i session_mgr__channel,
   const constants__t_request_handle_i session_mgr__req_handle,
   t_bool * const session_mgr__bres) {
   {
      constants__t_session_i session_mgr__l_session;
      t_bool session_mgr__l_valid_session;
      constants__t_sessionState session_mgr__l_session_state;
      constants__t_channel_i session_mgr__l_session_channel;
      
      session_core__get_session_from_req_handle(session_mgr__req_handle,
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
            session_core__cli_close_session(session_mgr__l_session);
         }
      }
      else {
         *session_mgr__bres = false;
      }
      if (*session_mgr__bres == true) {
         session_core__cli_record_session_service_resp(session_mgr__l_session,
            session_mgr__req_handle,
            session_mgr__bres);
      }
   }
}

void session_mgr__server_validate_session_service_req(
   const constants__t_channel_i session_mgr__channel,
   const constants__t_request_handle_i session_mgr__req_handle,
   const constants__t_session_token_i session_mgr__session_token,
   t_bool * const session_mgr__is_valid_res,
   constants__t_session_i * const session_mgr__session,
   t_bool * const session_mgr__snd_err) {
   {
      constants__t_session_i session_mgr__l_session;
      t_bool session_mgr__l_valid_session;
      constants__t_sessionState session_mgr__l_session_state;
      constants__t_channel_i session_mgr__l_session_channel;
      
      session_core__get_session_from_token(session_mgr__session_token,
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
            *session_mgr__snd_err = false;
            *session_mgr__session = session_mgr__l_session;
         }
         else {
            session_core__server_close_session(session_mgr__l_session);
            *session_mgr__is_valid_res = false;
            *session_mgr__snd_err = true;
         }
      }
      else {
         *session_mgr__is_valid_res = false;
         *session_mgr__snd_err = true;
      }
   }
}

void session_mgr__server_validate_session_service_resp(
   const constants__t_channel_i session_mgr__channel,
   const constants__t_session_i session_mgr__session,
   const constants__t_request_handle_i session_mgr__req_handle,
   t_bool * const session_mgr__is_valid_res,
   t_bool * const session_mgr__snd_err) {
   {
      t_bool session_mgr__l_valid_session;
      constants__t_sessionState session_mgr__l_session_state;
      constants__t_channel_i session_mgr__l_session_channel;
      
      session_core__is_valid_session(session_mgr__session,
         &session_mgr__l_valid_session);
      session_core__get_session_state_or_closed(session_mgr__session,
         &session_mgr__l_session_state);
      if (session_mgr__l_valid_session == true) {
         session_core__get_session_channel(session_mgr__session,
            &session_mgr__l_session_channel);
         if ((session_mgr__l_session_state == constants__e_session_userActivated) &&
            (session_mgr__l_session_channel == session_mgr__channel)) {
            session_core__is_session_valid_for_service(session_mgr__channel,
               session_mgr__session,
               session_mgr__is_valid_res);
            *session_mgr__snd_err = false;
         }
         else {
            *session_mgr__is_valid_res = false;
            *session_mgr__snd_err = true;
         }
      }
      else {
         *session_mgr__is_valid_res = false;
         *session_mgr__snd_err = true;
      }
   }
}

void session_mgr__client_create_req(
   const constants__t_session_i session_mgr__session,
   const constants__t_channel_i session_mgr__channel,
   const constants__t_request_handle_i session_mgr__req_handle,
   const constants__t_msg_i session_mgr__create_req_msg,
   constants__t_StatusCode_i * const session_mgr__ret) {
   {
      t_bool session_mgr__l_valid_session;
      constants__t_sessionState session_mgr__l_session_state;
      
      session_core__is_valid_session(session_mgr__session,
         &session_mgr__l_valid_session);
      session_core__get_session_state_or_closed(session_mgr__session,
         &session_mgr__l_session_state);
      if (session_mgr__l_valid_session == true) {
         if (session_mgr__l_session_state == constants__e_session_init) {
            session_core__cli_create_req(session_mgr__session,
               session_mgr__channel,
               session_mgr__req_handle,
               session_mgr__create_req_msg,
               session_mgr__ret);
         }
         else {
            *session_mgr__ret = constants__e_sc_bad_invalid_state;
         }
      }
      else {
         *session_mgr__ret = constants__e_sc_bad_invalid_argument;
      }
   }
}

void session_mgr__client_user_activate_req(
   const constants__t_session_i session_mgr__session,
   const constants__t_request_handle_i session_mgr__req_handle,
   const constants__t_user_i session_mgr__user,
   const constants__t_msg_i session_mgr__activate_req_msg,
   constants__t_StatusCode_i * const session_mgr__ret,
   constants__t_channel_i * const session_mgr__channel,
   constants__t_session_token_i * const session_mgr__session_token) {
   {
      t_bool session_mgr__l_valid_session;
      constants__t_sessionState session_mgr__l_session_state;
      
      session_core__is_valid_session(session_mgr__session,
         &session_mgr__l_valid_session);
      if (session_mgr__l_valid_session == true) {
         session_core__get_session_state_or_closed(session_mgr__session,
            &session_mgr__l_session_state);
         if ((session_mgr__l_session_state == constants__e_session_created) ||
            (session_mgr__l_session_state == constants__e_session_userActivated)) {
            session_core__cli_user_activate_req(session_mgr__session,
               session_mgr__req_handle,
               session_mgr__user,
               session_mgr__activate_req_msg,
               session_mgr__ret,
               session_mgr__channel,
               session_mgr__session_token);
         }
         else {
            *session_mgr__ret = constants__e_sc_bad_invalid_state;
            *session_mgr__channel = constants__c_channel_indet;
            *session_mgr__session_token = constants__c_session_token_indet;
         }
      }
      else {
         *session_mgr__ret = constants__e_sc_bad_invalid_argument;
         *session_mgr__channel = constants__c_channel_indet;
         *session_mgr__session_token = constants__c_session_token_indet;
      }
   }
}

void session_mgr__client_sc_activate_req(
   const constants__t_session_i session_mgr__session,
   const constants__t_request_handle_i session_mgr__req_handle,
   const constants__t_channel_i session_mgr__channel,
   const constants__t_msg_i session_mgr__activate_req_msg,
   constants__t_StatusCode_i * const session_mgr__ret,
   constants__t_session_token_i * const session_mgr__session_token) {
   {
      t_bool session_mgr__l_valid_session;
      constants__t_sessionState session_mgr__l_session_state;
      
      session_core__is_valid_session(session_mgr__session,
         &session_mgr__l_valid_session);
      if (session_mgr__l_valid_session == true) {
         session_core__get_session_state_or_closed(session_mgr__session,
            &session_mgr__l_session_state);
         if ((session_mgr__l_session_state == constants__e_session_scOrphaned) ||
            (session_mgr__l_session_state == constants__e_session_userActivated)) {
            session_core__cli_sc_activate_req(session_mgr__session,
               session_mgr__req_handle,
               session_mgr__channel,
               session_mgr__activate_req_msg,
               session_mgr__ret,
               session_mgr__session_token);
         }
         else {
            *session_mgr__ret = constants__e_sc_bad_invalid_state;
            *session_mgr__session_token = constants__c_session_token_indet;
         }
      }
      else {
         *session_mgr__ret = constants__e_sc_bad_invalid_argument;
         *session_mgr__session_token = constants__c_session_token_indet;
      }
   }
}

void session_mgr__client_close_req(
   const constants__t_session_i session_mgr__session,
   const constants__t_request_handle_i session_mgr__req_handle,
   const constants__t_msg_i session_mgr__close_req_msg,
   constants__t_StatusCode_i * const session_mgr__ret,
   constants__t_channel_i * const session_mgr__channel,
   constants__t_session_token_i * const session_mgr__session_token) {
   {
      t_bool session_mgr__l_valid_session;
      constants__t_sessionState session_mgr__l_session_state;
      
      session_core__is_valid_session(session_mgr__session,
         &session_mgr__l_valid_session);
      if (session_mgr__l_valid_session == true) {
         session_core__get_session_state_or_closed(session_mgr__session,
            &session_mgr__l_session_state);
         if (((session_mgr__l_session_state == constants__e_session_created) ||
            (session_mgr__l_session_state == constants__e_session_userActivating)) ||
            (session_mgr__l_session_state == constants__e_session_userActivated)) {
            session_core__cli_close_req(session_mgr__session,
               session_mgr__req_handle,
               session_mgr__close_req_msg,
               session_mgr__ret,
               session_mgr__channel,
               session_mgr__session_token);
         }
         else {
            *session_mgr__ret = constants__e_sc_bad_invalid_state;
            *session_mgr__channel = constants__c_channel_indet;
            *session_mgr__session_token = constants__c_session_token_indet;
         }
      }
      else {
         *session_mgr__ret = constants__e_sc_bad_invalid_argument;
         *session_mgr__channel = constants__c_channel_indet;
         *session_mgr__session_token = constants__c_session_token_indet;
      }
   }
}

