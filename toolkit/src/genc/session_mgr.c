/******************************************************************************

 File Name            : session_mgr.c

 Date                 : 18/07/2017 17:12:44

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
void session_mgr__receive_session_resp(
   const constants__t_channel_i session_mgr__channel,
   const constants__t_request_handle_i session_mgr__req_handle,
   const constants__t_msg_i session_mgr__resp_msg,
   const constants__t_msg_type session_mgr__resp_typ) {
   {
      constants__t_session_i session_mgr__l_session;
      constants__t_session_token_i session_mgr__l_session_token;
      
      session_core__get_session_from_req_handle(session_mgr__req_handle,
         &session_mgr__l_session);
      switch (session_mgr__resp_typ) {
      case constants__e_msg_session_create_resp:
         message_in_bs__read_create_session_msg_session_token(session_mgr__resp_msg,
            &session_mgr__l_session_token);
         session_core__cli_create_resp(session_mgr__channel,
            session_mgr__l_session,
            session_mgr__req_handle,
            session_mgr__l_session_token,
            session_mgr__resp_msg);
         break;
      case constants__e_msg_session_activate_resp:
         session_core__cli_activate_resp(session_mgr__channel,
            session_mgr__l_session,
            session_mgr__req_handle,
            session_mgr__resp_msg);
         break;
      case constants__e_msg_session_close_resp:
         session_core__cli_close_resp(session_mgr__channel,
            session_mgr__l_session,
            session_mgr__req_handle,
            session_mgr__resp_msg);
         break;
      default:
         break;
      }
   }
}

void session_mgr__receive_session_req(
   const constants__t_channel_i session_mgr__channel,
   const constants__t_request_handle_i session_mgr__req_handle,
   const constants__t_session_token_i session_mgr__session_token,
   const constants__t_msg_i session_mgr__req_msg,
   const constants__t_msg_type session_mgr__req_typ,
   const constants__t_msg_i session_mgr__resp_msg,
   t_bool * const session_mgr__b_send_resp,
   constants__t_session_i * const session_mgr__session) {
   {
      t_bool session_mgr__l_valid_session;
      constants__t_user_i session_mgr__l_user;
      constants__t_StatusCode_i session_mgr__l_ret;
      
      switch (session_mgr__req_typ) {
      case constants__e_msg_session_create_req:
         session_core__srv_create_req_and_resp(session_mgr__channel,
            session_mgr__req_handle,
            session_mgr__req_msg,
            session_mgr__resp_msg,
            session_mgr__session);
         session_core__is_valid_session(*session_mgr__session,
            &session_mgr__l_valid_session);
         if (session_mgr__l_valid_session == true) {
            session_mgr__l_ret = constants__e_sc_ok;
         }
         else {
            session_mgr__l_ret = constants__e_sc_bad_unexpected_error;
         }
         break;
      case constants__e_msg_session_activate_req:
         session_core__get_session_from_token(session_mgr__session_token,
            session_mgr__session);
         message_in_bs__read_activate_msg_user(session_mgr__req_msg,
            &session_mgr__l_user);
         session_core__srv_activate_req_and_resp(session_mgr__channel,
            *session_mgr__session,
            session_mgr__req_handle,
            session_mgr__l_user,
            session_mgr__req_msg,
            session_mgr__resp_msg,
            &session_mgr__l_ret);
         break;
      case constants__e_msg_session_close_req:
         session_core__get_session_from_token(session_mgr__session_token,
            session_mgr__session);
         session_core__srv_close_req_and_resp(session_mgr__channel,
            *session_mgr__session,
            session_mgr__req_handle,
            session_mgr__req_msg,
            session_mgr__resp_msg,
            &session_mgr__l_ret);
         break;
      default:
         session_mgr__l_ret = constants__e_sc_bad_unexpected_error;
         break;
      }
      if (session_mgr__l_ret == constants__e_sc_ok) {
         *session_mgr__b_send_resp = true;
      }
      else {
         *session_mgr__b_send_resp = false;
      }
   }
}

void session_mgr__cli_validate_session_service_req(
   const constants__t_session_i session_mgr__session,
   const constants__t_request_handle_i session_mgr__req_handle,
   const constants__t_msg_i session_mgr__req_msg,
   constants__t_StatusCode_i * const session_mgr__ret,
   constants__t_channel_i * const session_mgr__channel,
   constants__t_session_token_i * const session_mgr__session_token) {
   {
      t_bool session_mgr__l_valid_session;
      constants__t_channel_i session_mgr__l_channel;
      t_bool session_mgr__l_valid_channel;
      constants__t_session_token_i session_mgr__l_session_token;
      constants__t_StatusCode_i session_mgr__l_ret;
      t_bool session_mgr__l_bres;
      
      session_mgr__l_session_token = constants__c_session_token_indet;
      session_mgr__l_channel = constants__c_channel_indet;
      session_core__is_valid_session(session_mgr__session,
         &session_mgr__l_valid_session);
      if (session_mgr__l_valid_session == true) {
         session_core__get_session_channel(session_mgr__session,
            &session_mgr__l_channel);
         channel_mgr_bs__is_valid_channel(session_mgr__l_channel,
            &session_mgr__l_valid_channel);
         if (session_mgr__l_valid_channel == true) {
            session_core__cli_is_session_valid_for_service(session_mgr__l_channel,
               session_mgr__session,
               &session_mgr__l_bres);
            if (session_mgr__l_bres == true) {
               session_core__cli_new_session_service_req(session_mgr__session,
                  session_mgr__req_handle,
                  &session_mgr__l_ret,
                  &session_mgr__l_session_token);
            }
            else {
               session_mgr__l_channel = constants__c_channel_indet;
               session_mgr__l_ret = constants__e_sc_bad_invalid_state;
            }
         }
         else {
            session_mgr__l_ret = constants__e_sc_bad_invalid_argument;
         }
      }
      else {
         session_mgr__l_ret = constants__e_sc_bad_invalid_argument;
      }
      *session_mgr__ret = session_mgr__l_ret;
      *session_mgr__channel = session_mgr__l_channel;
      *session_mgr__session_token = session_mgr__l_session_token;
   }
}

void session_mgr__cli_validate_session_service_resp(
   const constants__t_channel_i session_mgr__channel,
   const constants__t_request_handle_i session_mgr__req_handle,
   const constants__t_msg_i session_mgr__resp_msg,
   t_bool * const session_mgr__bres) {
   {
      constants__t_session_i session_mgr__l_session;
      
      session_core__get_session_from_req_handle(session_mgr__req_handle,
         &session_mgr__l_session);
      session_core__cli_is_session_valid_for_service(session_mgr__channel,
         session_mgr__l_session,
         session_mgr__bres);
      if (*session_mgr__bres == true) {
         session_core__cli_record_session_service_resp(session_mgr__l_session,
            session_mgr__resp_msg,
            session_mgr__req_handle,
            session_mgr__bres);
      }
   }
}

void session_mgr__srv_validate_session_service_req(
   const constants__t_channel_i session_mgr__channel,
   const constants__t_request_handle_i session_mgr__req_handle,
   const constants__t_session_token_i session_mgr__session_token,
   const constants__t_msg_i session_mgr__req_msg,
   t_bool * const session_mgr__bres,
   t_bool * const session_mgr__snd_err) {
   {
      constants__t_session_i session_mgr__l_session;
      
      session_core__get_session_from_token(session_mgr__session_token,
         &session_mgr__l_session);
      session_core__srv_is_session_valid_for_service(session_mgr__channel,
         session_mgr__l_session,
         session_mgr__bres,
         session_mgr__snd_err);
   }
}

void session_mgr__srv_validate_session_service_resp(
   const constants__t_channel_i session_mgr__channel,
   const constants__t_session_i session_mgr__session,
   const constants__t_request_handle_i session_mgr__req_handle,
   const constants__t_msg_i session_mgr__req_msg,
   const constants__t_msg_i session_mgr__resp_msg,
   t_bool * const session_mgr__bres,
   t_bool * const session_mgr__snd_err) {
   session_core__srv_is_session_valid_for_service(session_mgr__channel,
      session_mgr__session,
      session_mgr__bres,
      session_mgr__snd_err);
}

