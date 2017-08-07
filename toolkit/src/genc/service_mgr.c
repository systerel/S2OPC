/******************************************************************************

 File Name            : service_mgr.c

 Date                 : 08/08/2017 10:57:23

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "service_mgr.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void service_mgr__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void service_mgr__get_response_type(
   const constants__t_msg_type_i service_mgr__req_msg_typ,
   constants__t_msg_type_i * const service_mgr__resp_msg_typ) {
   switch (service_mgr__req_msg_typ) {
   case constants__e_msg_public_service_req:
      *service_mgr__resp_msg_typ = constants__e_msg_public_service_resp;
      break;
   case constants__e_msg_session_create_req:
      *service_mgr__resp_msg_typ = constants__e_msg_session_create_resp;
      break;
   case constants__e_msg_session_activate_req:
      *service_mgr__resp_msg_typ = constants__e_msg_session_activate_resp;
      break;
   case constants__e_msg_session_close_req:
      *service_mgr__resp_msg_typ = constants__e_msg_session_close_resp;
      break;
   case constants__e_msg_session_read_req:
      *service_mgr__resp_msg_typ = constants__e_msg_session_read_resp;
      break;
   case constants__e_msg_session_write_req:
      *service_mgr__resp_msg_typ = constants__e_msg_session_write_resp;
      break;
   default:
      break;
   }
}

void service_mgr__treat_read_request(
   const constants__t_msg_i service_mgr__p_request_msg,
   const constants__t_msg_i service_mgr__p_response_msg) {
   {
      t_entier4 service_mgr__l_nb_ReadValue;
      t_bool service_mgr__l_is_valid;
      
      service_read__check_ReadRequest(service_mgr__p_request_msg,
         &service_mgr__l_is_valid,
         &service_mgr__l_nb_ReadValue);
      if (service_mgr__l_is_valid == true) {
         service_read__alloc_read_response(service_mgr__l_nb_ReadValue,
            service_mgr__p_response_msg,
            &service_mgr__l_is_valid);
         if (service_mgr__l_is_valid == true) {
            service_read__fill_read_response(service_mgr__p_request_msg,
               service_mgr__p_response_msg);
         }
      }
   }
}

void service_mgr__treat_write_request(
   const constants__t_msg_i service_mgr__write_msg,
   const constants__t_user_i service_mgr__userid,
   constants__t_StatusCode_i * const service_mgr__StatusCode_service) {
   {
      t_entier4 service_mgr__l_nb_req;
      
      service_write_decode_bs__decode_write_request(service_mgr__write_msg,
         service_mgr__StatusCode_service);
      if (*service_mgr__StatusCode_service == constants__e_sc_ok) {
         service_write_decode_bs__get_nb_WriteValue(&service_mgr__l_nb_req);
         address_space__alloc_write_request_responses(service_mgr__l_nb_req,
            service_mgr__StatusCode_service);
         if (*service_mgr__StatusCode_service == constants__e_sc_ok) {
            address_space__treat_write_request_WriteValues(service_mgr__userid,
               service_mgr__StatusCode_service);
         }
      }
      service_write_decode_bs__free_write_request();
   }
}

void service_mgr__local_create_session(
   const constants__t_session_i service_mgr__session,
   const constants__t_channel_i service_mgr__channel) {
   {
      constants__t_msg_header_i service_mgr__l_msg_header;
      t_bool service_mgr__l_valid_msg_header;
      constants__t_msg_i service_mgr__l_req_msg;
      constants__t_request_handle_i service_mgr__l_req_handle;
      t_bool service_mgr__l_valid_req_handle;
      t_bool service_mgr__l_valid_msg;
      t_bool service_mgr__l_valid_session;
      constants__t_byte_buffer_i service_mgr__l_buffer;
      t_bool service_mgr__l_valid_buffer;
      constants__t_StatusCode_i service_mgr__l_ret;
      
      message_out_bs__alloc_req_msg(constants__e_msg_session_create_req,
         &service_mgr__l_msg_header,
         &service_mgr__l_req_msg);
      message_out_bs__is_valid_msg_out(service_mgr__l_req_msg,
         &service_mgr__l_valid_msg);
      message_out_bs__is_valid_msg_out_header(service_mgr__l_msg_header,
         &service_mgr__l_valid_msg_header);
      if ((service_mgr__l_valid_msg_header == true) &&
         (service_mgr__l_valid_msg == true)) {
         request_handle_bs__client_fresh_req_handle(constants__e_msg_session_create_resp,
            &service_mgr__l_req_handle);
         request_handle_bs__is_valid_req_handle(service_mgr__l_req_handle,
            &service_mgr__l_valid_req_handle);
         if (service_mgr__l_valid_req_handle == true) {
            session_mgr__is_valid_session(service_mgr__session,
               &service_mgr__l_valid_session);
            if (service_mgr__l_valid_session == true) {
               session_mgr__client_create_req(service_mgr__session,
                  service_mgr__channel,
                  service_mgr__l_req_handle,
                  service_mgr__l_req_msg,
                  &service_mgr__l_ret);
               if (service_mgr__l_ret == constants__e_sc_ok) {
                  message_out_bs__write_msg_out_header_req_handle(service_mgr__l_msg_header,
                     service_mgr__l_req_handle);
                  message_out_bs__encode_msg(constants__e_msg_session_create_req,
                     service_mgr__l_msg_header,
                     service_mgr__l_req_msg,
                     &service_mgr__l_buffer);
                  message_out_bs__is_valid_buffer_out(service_mgr__l_buffer,
                     &service_mgr__l_valid_buffer);
                  if (service_mgr__l_valid_buffer == true) {
                     channel_mgr_bs__send_channel_msg_buffer(service_mgr__channel,
                        service_mgr__l_buffer,
                        &service_mgr__l_ret);
                  }
                  else {
                     service_mgr__l_ret = constants__e_sc_nok;
                  }
                  if (service_mgr__l_ret != constants__e_sc_ok) {
                     request_handle_bs__client_remove_req_handle(service_mgr__l_req_handle);
                     session_mgr__client_session_mgr_close_session(service_mgr__session);
                  }
               }
               else {
                  request_handle_bs__client_remove_req_handle(service_mgr__l_req_handle);
                  session_mgr__client_session_mgr_close_session(service_mgr__session);
               }
            }
            else {
               session_mgr__client_session_mgr_close_session(service_mgr__session);
            }
         }
         message_out_bs__dealloc_msg_header_out(service_mgr__l_msg_header);
         message_out_bs__dealloc_msg_out(service_mgr__l_req_msg);
      }
   }
}

void service_mgr__local_sc_activate_orphaned_sessions(
   const constants__t_channel_config_idx_i service_mgr__channel_config_idx,
   const constants__t_channel_i service_mgr__channel) {
   {
      t_bool service_mgr__l_valid_new_channel;
      constants__t_msg_header_i service_mgr__l_msg_header;
      t_bool service_mgr__l_valid_msg_header;
      constants__t_msg_i service_mgr__l_req_msg;
      t_bool service_mgr__l_valid_msg;
      t_bool service_mgr__l_continue;
      constants__t_session_i service_mgr__l_session;
      constants__t_StatusCode_i service_mgr__l_ret;
      constants__t_request_handle_i service_mgr__l_req_handle;
      t_bool service_mgr__l_valid_req_handle;
      constants__t_session_token_i service_mgr__l_session_token;
      constants__t_byte_buffer_i service_mgr__l_buffer;
      t_bool service_mgr__l_valid_buffer;
      
      channel_mgr_bs__is_connected_channel(service_mgr__channel,
         &service_mgr__l_valid_new_channel);
      if (service_mgr__l_valid_new_channel == true) {
         session_mgr__init_iter_orphaned_t_session(service_mgr__channel_config_idx,
            &service_mgr__l_continue);
         while (service_mgr__l_continue == true) {
            session_mgr__continue_iter_orphaned_t_session(&service_mgr__l_session,
               &service_mgr__l_continue);
            message_out_bs__alloc_req_msg(constants__e_msg_session_activate_req,
               &service_mgr__l_msg_header,
               &service_mgr__l_req_msg);
            message_out_bs__is_valid_msg_out(service_mgr__l_req_msg,
               &service_mgr__l_valid_msg);
            message_out_bs__is_valid_msg_out_header(service_mgr__l_msg_header,
               &service_mgr__l_valid_msg_header);
            if ((service_mgr__l_valid_msg == true) &&
               (service_mgr__l_valid_msg_header == true)) {
               request_handle_bs__client_fresh_req_handle(constants__e_msg_session_activate_resp,
                  &service_mgr__l_req_handle);
               request_handle_bs__is_valid_req_handle(service_mgr__l_req_handle,
                  &service_mgr__l_valid_req_handle);
               if (service_mgr__l_valid_req_handle == true) {
                  session_mgr__client_sc_activate_req(service_mgr__l_session,
                     service_mgr__l_req_handle,
                     service_mgr__channel,
                     service_mgr__l_req_msg,
                     &service_mgr__l_ret,
                     &service_mgr__l_session_token);
                  if (service_mgr__l_ret == constants__e_sc_ok) {
                     message_out_bs__write_msg_out_header_req_handle(service_mgr__l_msg_header,
                        service_mgr__l_req_handle);
                     message_out_bs__write_msg_out_header_session_token(service_mgr__l_msg_header,
                        service_mgr__l_session_token);
                     message_out_bs__encode_msg(constants__e_msg_session_activate_req,
                        service_mgr__l_msg_header,
                        service_mgr__l_req_msg,
                        &service_mgr__l_buffer);
                     message_out_bs__is_valid_buffer_out(service_mgr__l_buffer,
                        &service_mgr__l_valid_buffer);
                     if (service_mgr__l_valid_buffer == true) {
                        channel_mgr_bs__send_channel_msg_buffer(service_mgr__channel,
                           service_mgr__l_buffer,
                           &service_mgr__l_ret);
                     }
                     else {
                        service_mgr__l_ret = constants__e_sc_nok;
                     }
                     if (service_mgr__l_ret != constants__e_sc_ok) {
                        request_handle_bs__client_remove_req_handle(service_mgr__l_req_handle);
                        session_mgr__client_session_mgr_close_session(service_mgr__l_session);
                     }
                  }
               }
               message_out_bs__dealloc_msg_header_out(service_mgr__l_msg_header);
               message_out_bs__dealloc_msg_out(service_mgr__l_req_msg);
            }
         }
      }
   }
}

void service_mgr__local_activate_session(
   const constants__t_session_i service_mgr__session,
   const constants__t_user_i service_mgr__user,
   constants__t_StatusCode_i * const service_mgr__ret) {
   {
      constants__t_msg_header_i service_mgr__l_msg_header;
      t_bool service_mgr__l_valid_msg_header;
      constants__t_msg_i service_mgr__l_req_msg;
      t_bool service_mgr__l_valid_msg;
      constants__t_StatusCode_i service_mgr__l_ret;
      constants__t_channel_i service_mgr__l_channel;
      constants__t_request_handle_i service_mgr__l_req_handle;
      t_bool service_mgr__l_valid_req_handle;
      constants__t_session_token_i service_mgr__l_session_token;
      constants__t_byte_buffer_i service_mgr__l_buffer;
      t_bool service_mgr__l_valid_buffer;
      
      message_out_bs__alloc_req_msg(constants__e_msg_session_activate_req,
         &service_mgr__l_msg_header,
         &service_mgr__l_req_msg);
      message_out_bs__is_valid_msg_out(service_mgr__l_req_msg,
         &service_mgr__l_valid_msg);
      message_out_bs__is_valid_msg_out_header(service_mgr__l_msg_header,
         &service_mgr__l_valid_msg_header);
      if ((service_mgr__l_valid_msg == true) &&
         (service_mgr__l_valid_msg_header == true)) {
         request_handle_bs__client_fresh_req_handle(constants__e_msg_session_activate_resp,
            &service_mgr__l_req_handle);
         request_handle_bs__is_valid_req_handle(service_mgr__l_req_handle,
            &service_mgr__l_valid_req_handle);
         if (service_mgr__l_valid_req_handle == true) {
            session_mgr__client_user_activate_req(service_mgr__session,
               service_mgr__l_req_handle,
               service_mgr__user,
               service_mgr__l_req_msg,
               &service_mgr__l_ret,
               &service_mgr__l_channel,
               &service_mgr__l_session_token);
            if (service_mgr__l_ret == constants__e_sc_ok) {
               message_out_bs__write_msg_out_header_req_handle(service_mgr__l_msg_header,
                  service_mgr__l_req_handle);
               message_out_bs__write_msg_out_header_session_token(service_mgr__l_msg_header,
                  service_mgr__l_session_token);
               message_out_bs__encode_msg(constants__e_msg_session_activate_req,
                  service_mgr__l_msg_header,
                  service_mgr__l_req_msg,
                  &service_mgr__l_buffer);
               message_out_bs__is_valid_buffer_out(service_mgr__l_buffer,
                  &service_mgr__l_valid_buffer);
               if (service_mgr__l_valid_buffer == true) {
                  channel_mgr_bs__send_channel_msg_buffer(service_mgr__l_channel,
                     service_mgr__l_buffer,
                     &service_mgr__l_ret);
               }
               else {
                  service_mgr__l_ret = constants__e_sc_nok;
               }
               if (service_mgr__l_ret != constants__e_sc_ok) {
                  request_handle_bs__client_remove_req_handle(service_mgr__l_req_handle);
                  session_mgr__client_session_mgr_close_session(service_mgr__session);
               }
            }
         }
         else {
            service_mgr__l_ret = constants__e_sc_bad_out_of_memory;
         }
         message_out_bs__dealloc_msg_header_out(service_mgr__l_msg_header);
         message_out_bs__dealloc_msg_out(service_mgr__l_req_msg);
      }
      else {
         service_mgr__l_ret = constants__e_sc_bad_out_of_memory;
      }
      *service_mgr__ret = service_mgr__l_ret;
   }
}

void service_mgr__server_receive_session_treatment_req(
   const constants__t_channel_i service_mgr__channel,
   const constants__t_msg_type_i service_mgr__req_typ,
   const constants__t_byte_buffer_i service_mgr__msg_buffer) {
   {
      constants__t_msg_header_i service_mgr__l_req_msg_header;
      t_bool service_mgr__l_valid_req_header;
      t_bool service_mgr__l_checked_req_header;
      constants__t_msg_i service_mgr__l_req_msg;
      t_bool service_mgr__l_valid_req;
      constants__t_request_handle_i service_mgr__l_request_handle;
      t_bool service_mgr__l_valid_req_handle;
      constants__t_session_token_i service_mgr__l_session_token;
      constants__t_msg_type_i service_mgr__l_resp_msg_typ;
      constants__t_msg_i service_mgr__l_resp_msg;
      t_bool service_mgr__l_valid_msg;
      constants__t_msg_header_i service_mgr__l_resp_msg_header;
      t_bool service_mgr__l_valid_resp_header;
      constants__t_session_i service_mgr__l_session;
      constants__t_StatusCode_i service_mgr__l_ret;
      constants__t_byte_buffer_i service_mgr__l_buffer_out;
      t_bool service_mgr__l_valid_buffer;
      
      message_in_bs__decode_msg_header(service_mgr__msg_buffer,
         &service_mgr__l_req_msg_header);
      message_in_bs__is_valid_msg_in_header(service_mgr__l_req_msg_header,
         &service_mgr__l_valid_req_header);
      if (service_mgr__l_valid_req_header == true) {
         message_in_bs__read_msg_header_req_handle(service_mgr__l_req_msg_header,
            &service_mgr__l_request_handle);
         request_handle_bs__is_valid_req_handle(service_mgr__l_request_handle,
            &service_mgr__l_valid_req_handle);
         message_in_bs__read_msg_req_header_session_token(service_mgr__l_req_msg_header,
            &service_mgr__l_session_token);
         service_mgr__l_checked_req_header = true;
         if ((service_mgr__l_checked_req_header == true) &&
            (service_mgr__l_valid_req_handle == true)) {
            message_in_bs__decode_msg(service_mgr__msg_buffer,
               &service_mgr__l_req_msg);
            message_in_bs__is_valid_msg_in(service_mgr__l_req_msg,
               &service_mgr__l_valid_req);
            if (service_mgr__l_valid_req == true) {
               service_mgr__get_response_type(service_mgr__req_typ,
                  &service_mgr__l_resp_msg_typ);
               message_out_bs__alloc_resp_msg(service_mgr__l_resp_msg_typ,
                  service_mgr__l_req_msg,
                  &service_mgr__l_resp_msg_header,
                  &service_mgr__l_resp_msg);
               message_out_bs__is_valid_msg_out(service_mgr__l_resp_msg,
                  &service_mgr__l_valid_msg);
               message_out_bs__is_valid_msg_out_header(service_mgr__l_resp_msg_header,
                  &service_mgr__l_valid_resp_header);
               if ((service_mgr__l_valid_msg == true) &&
                  (service_mgr__l_valid_resp_header == true)) {
                  session_mgr__server_receive_session_req(service_mgr__channel,
                     service_mgr__l_session_token,
                     service_mgr__l_req_msg,
                     service_mgr__req_typ,
                     service_mgr__l_resp_msg,
                     &service_mgr__l_session,
                     &service_mgr__l_ret);
                  message_out_bs__write_msg_resp_header_service_status(service_mgr__l_resp_msg_header,
                     service_mgr__l_ret);
                  message_out_bs__write_msg_out_header_req_handle(service_mgr__l_resp_msg_header,
                     service_mgr__l_request_handle);
                  message_out_bs__encode_msg(service_mgr__l_resp_msg_typ,
                     service_mgr__l_resp_msg_header,
                     service_mgr__l_resp_msg,
                     &service_mgr__l_buffer_out);
                  message_out_bs__is_valid_buffer_out(service_mgr__l_buffer_out,
                     &service_mgr__l_valid_buffer);
                  if (service_mgr__l_valid_buffer == true) {
                     channel_mgr_bs__send_channel_msg_buffer(service_mgr__channel,
                        service_mgr__l_buffer_out,
                        &service_mgr__l_ret);
                  }
                  else {
                     service_mgr__l_ret = constants__e_sc_nok;
                  }
                  if (service_mgr__l_ret != constants__e_sc_ok) {
                     session_mgr__server_close_session(service_mgr__l_session);
                  }
                  message_out_bs__dealloc_msg_header_out(service_mgr__l_resp_msg_header);
                  message_out_bs__dealloc_msg_out(service_mgr__l_resp_msg);
               }
               message_in_bs__dealloc_msg_in(service_mgr__l_req_msg);
            }
         }
      }
      if (service_mgr__l_valid_req_header == true) {
         message_in_bs__dealloc_msg_in_header(service_mgr__l_req_msg_header);
      }
      message_in_bs__dealloc_msg_in_buffer(service_mgr__msg_buffer);
   }
}

void service_mgr__client_receive_session_treatment_resp(
   const constants__t_channel_i service_mgr__channel,
   const constants__t_msg_type_i service_mgr__resp_typ,
   const constants__t_byte_buffer_i service_mgr__msg_buffer) {
   {
      constants__t_msg_header_i service_mgr__l_resp_msg_header;
      t_bool service_mgr__l_valid_resp_header;
      constants__t_msg_i service_mgr__l_resp_msg;
      t_bool service_mgr__l_valid_resp;
      constants__t_request_handle_i service_mgr__l_request_handle;
      t_bool service_mgr__l_validated_req_handle;
      constants__t_session_i service_mgr__l_session;
      constants__t_sessionState service_mgr__l_session_state;
      constants__t_user_i service_mgr__l_session_user;
      constants__t_StatusCode_i service_mgr__l_ret;
      
      message_in_bs__decode_msg_header(service_mgr__msg_buffer,
         &service_mgr__l_resp_msg_header);
      message_in_bs__is_valid_msg_in_header(service_mgr__l_resp_msg_header,
         &service_mgr__l_valid_resp_header);
      if (service_mgr__l_valid_resp_header == true) {
         message_in_bs__read_msg_header_req_handle(service_mgr__l_resp_msg_header,
            &service_mgr__l_request_handle);
         request_handle_bs__client_validate_response_request_handle(service_mgr__l_request_handle,
            service_mgr__resp_typ,
            &service_mgr__l_validated_req_handle);
         if (service_mgr__l_validated_req_handle == true) {
            message_in_bs__decode_msg(service_mgr__msg_buffer,
               &service_mgr__l_resp_msg);
            message_in_bs__is_valid_msg_in(service_mgr__l_resp_msg,
               &service_mgr__l_valid_resp);
            if (service_mgr__l_valid_resp == true) {
               session_mgr__client_receive_session_resp(service_mgr__channel,
                  service_mgr__l_request_handle,
                  service_mgr__resp_typ,
                  service_mgr__l_resp_msg_header,
                  service_mgr__l_resp_msg,
                  &service_mgr__l_session);
               request_handle_bs__client_remove_req_handle(service_mgr__l_request_handle);
               session_mgr__get_session_state_or_closed(service_mgr__l_session,
                  &service_mgr__l_session_state);
               if (service_mgr__l_session_state == constants__e_session_created) {
                  session_async_bs__is_session_to_activate(service_mgr__l_session,
                     &service_mgr__l_session_user);
                  if (service_mgr__l_session_user != constants__c_user_indet) {
                     service_mgr__local_activate_session(service_mgr__l_session,
                        service_mgr__l_session_user,
                        &service_mgr__l_ret);
                  }
               }
               message_in_bs__dealloc_msg_in(service_mgr__l_resp_msg);
            }
            request_handle_bs__client_remove_req_handle(service_mgr__l_request_handle);
         }
      }
      if (service_mgr__l_valid_resp_header == true) {
         message_in_bs__dealloc_msg_in_header(service_mgr__l_resp_msg_header);
      }
      message_in_bs__dealloc_msg_in_buffer(service_mgr__msg_buffer);
   }
}

void service_mgr__server_receive_session_service_req(
   const constants__t_channel_i service_mgr__channel,
   const constants__t_msg_type_i service_mgr__req_typ,
   const constants__t_byte_buffer_i service_mgr__msg_buffer) {
   {
      constants__t_msg_header_i service_mgr__l_req_msg_header;
      t_bool service_mgr__l_valid_req_header;
      constants__t_request_handle_i service_mgr__l_request_handle;
      t_bool service_mgr__l_valid_req_handle;
      constants__t_session_token_i service_mgr__l_session_token;
      t_bool service_mgr__l_is_valid_req;
      constants__t_session_i service_mgr__l_session;
      t_bool service_mgr__l_snd_session_err;
      constants__t_msg_i service_mgr__l_req_msg;
      t_bool service_mgr__l_valid_req;
      constants__t_msg_type_i service_mgr__l_resp_msg_typ;
      constants__t_msg_i service_mgr__l_resp_msg;
      t_bool service_mgr__l_valid_msg;
      constants__t_msg_header_i service_mgr__l_resp_msg_header;
      t_bool service_mgr__l_valid_resp_header;
      constants__t_user_i service_mgr__l_session_user;
      constants__t_StatusCode_i service_mgr__l_ret;
      t_bool service_mgr__l_is_valid_resp;
      constants__t_byte_buffer_i service_mgr__l_buffer_out;
      t_bool service_mgr__l_valid_buffer;
      
      message_in_bs__decode_msg_header(service_mgr__msg_buffer,
         &service_mgr__l_req_msg_header);
      message_in_bs__is_valid_msg_in_header(service_mgr__l_req_msg_header,
         &service_mgr__l_valid_req_header);
      if (service_mgr__l_valid_req_header == true) {
         message_in_bs__read_msg_header_req_handle(service_mgr__l_req_msg_header,
            &service_mgr__l_request_handle);
         request_handle_bs__is_valid_req_handle(service_mgr__l_request_handle,
            &service_mgr__l_valid_req_handle);
         if (service_mgr__l_valid_req_handle == true) {
            message_in_bs__read_msg_req_header_session_token(service_mgr__l_req_msg_header,
               &service_mgr__l_session_token);
            session_mgr__server_validate_session_service_req(service_mgr__channel,
               service_mgr__l_request_handle,
               service_mgr__l_session_token,
               &service_mgr__l_is_valid_req,
               &service_mgr__l_session,
               &service_mgr__l_snd_session_err);
            if (service_mgr__l_is_valid_req == true) {
               message_in_bs__decode_msg(service_mgr__msg_buffer,
                  &service_mgr__l_req_msg);
               message_in_bs__is_valid_msg_in(service_mgr__l_req_msg,
                  &service_mgr__l_valid_req);
               if (service_mgr__l_valid_req == true) {
                  if (service_mgr__l_is_valid_req == true) {
                     service_mgr__get_response_type(service_mgr__req_typ,
                        &service_mgr__l_resp_msg_typ);
                     message_out_bs__alloc_resp_msg(service_mgr__l_resp_msg_typ,
                        service_mgr__l_req_msg,
                        &service_mgr__l_resp_msg_header,
                        &service_mgr__l_resp_msg);
                     message_out_bs__is_valid_msg_out(service_mgr__l_resp_msg,
                        &service_mgr__l_valid_msg);
                     message_out_bs__is_valid_msg_out_header(service_mgr__l_resp_msg_header,
                        &service_mgr__l_valid_resp_header);
                     if ((service_mgr__l_valid_msg == true) &&
                        (service_mgr__l_valid_resp_header == true)) {
                        switch (service_mgr__req_typ) {
                        case constants__e_msg_session_read_req:
                           service_mgr__treat_read_request(service_mgr__l_req_msg,
                              service_mgr__l_resp_msg);
                           message_out_bs__write_msg_resp_header_service_status(service_mgr__l_resp_msg_header,
                              constants__e_sc_ok);
                           break;
                        case constants__e_msg_session_write_req:
                           session_mgr__get_session_user_or_indet(service_mgr__l_session,
                              &service_mgr__l_session_user);
                           service_mgr__treat_write_request(service_mgr__l_req_msg,
                              service_mgr__l_session_user,
                              &service_mgr__l_ret);
                           message_out_bs__write_msg_resp_header_service_status(service_mgr__l_resp_msg_header,
                              service_mgr__l_ret);
                           address_space__write_WriteResponse_msg_out(service_mgr__l_resp_msg);
                           address_space__dealloc_write_request_responses();
                           break;
                        default:
                           break;
                        }
                        session_mgr__server_validate_session_service_resp(service_mgr__channel,
                           service_mgr__l_session,
                           service_mgr__l_request_handle,
                           &service_mgr__l_is_valid_resp,
                           &service_mgr__l_snd_session_err);
                        if (service_mgr__l_is_valid_resp == true) {
                           message_out_bs__write_msg_out_header_req_handle(service_mgr__l_resp_msg_header,
                              service_mgr__l_request_handle);
                           message_out_bs__encode_msg(service_mgr__l_resp_msg_typ,
                              service_mgr__l_resp_msg_header,
                              service_mgr__l_resp_msg,
                              &service_mgr__l_buffer_out);
                           message_out_bs__is_valid_buffer_out(service_mgr__l_buffer_out,
                              &service_mgr__l_valid_buffer);
                           if (service_mgr__l_valid_buffer == true) {
                              channel_mgr_bs__send_channel_msg_buffer(service_mgr__channel,
                                 service_mgr__l_buffer_out,
                                 &service_mgr__l_ret);
                           }
                           else {
                              service_mgr__l_ret = constants__e_sc_nok;
                           }
                        }
                        message_out_bs__dealloc_msg_header_out(service_mgr__l_resp_msg_header);
                        message_out_bs__dealloc_msg_out(service_mgr__l_resp_msg);
                     }
                     message_in_bs__dealloc_msg_in(service_mgr__l_req_msg);
                  }
               }
            }
         }
      }
      if (service_mgr__l_valid_req_header == true) {
         message_in_bs__dealloc_msg_in_header(service_mgr__l_req_msg_header);
      }
      message_in_bs__dealloc_msg_in_buffer(service_mgr__msg_buffer);
   }
}

void service_mgr__client_receive_session_service_resp(
   const constants__t_channel_i service_mgr__channel,
   const constants__t_msg_type_i service_mgr__resp_typ,
   const constants__t_byte_buffer_i service_mgr__msg_buffer) {
   {
      constants__t_msg_header_i service_mgr__l_resp_msg_header;
      t_bool service_mgr__l_valid_resp_header;
      constants__t_request_handle_i service_mgr__l_request_handle;
      t_bool service_mgr__l_validated_req_handle;
      t_bool service_mgr__l_is_valid_session_resp;
      constants__t_msg_i service_mgr__l_resp_msg;
      t_bool service_mgr__l_valid_resp_msg;
      constants__t_StatusCode_i service_mgr__l_status;
      
      message_in_bs__decode_msg_header(service_mgr__msg_buffer,
         &service_mgr__l_resp_msg_header);
      message_in_bs__is_valid_msg_in_header(service_mgr__l_resp_msg_header,
         &service_mgr__l_valid_resp_header);
      if (service_mgr__l_valid_resp_header == true) {
         message_in_bs__read_msg_header_req_handle(service_mgr__l_resp_msg_header,
            &service_mgr__l_request_handle);
         request_handle_bs__client_validate_response_request_handle(service_mgr__l_request_handle,
            service_mgr__resp_typ,
            &service_mgr__l_validated_req_handle);
         if (service_mgr__l_validated_req_handle == true) {
            session_mgr__client_validate_session_service_resp(service_mgr__channel,
               service_mgr__l_request_handle,
               &service_mgr__l_is_valid_session_resp);
            if (service_mgr__l_is_valid_session_resp == true) {
               message_in_bs__decode_msg(service_mgr__msg_buffer,
                  &service_mgr__l_resp_msg);
               message_in_bs__is_valid_msg_in(service_mgr__l_resp_msg,
                  &service_mgr__l_valid_resp_msg);
               if (service_mgr__l_valid_resp_msg == true) {
                  message_in_bs__read_msg_resp_header_service_status(service_mgr__l_resp_msg_header,
                     &service_mgr__l_status);
                  service_response_cli_cb_bs__cli_service_response(service_mgr__l_resp_msg,
                     service_mgr__l_status);
               }
            }
            request_handle_bs__client_remove_req_handle(service_mgr__l_request_handle);
         }
      }
      ;
      message_in_bs__dealloc_msg_in_buffer(service_mgr__msg_buffer);
   }
}

void service_mgr__server_receive_public_service_req(
   const constants__t_channel_i service_mgr__channel,
   const constants__t_msg_type_i service_mgr__req_typ,
   const constants__t_byte_buffer_i service_mgr__msg_buffer) {
   ;
}

void service_mgr__client_receive_public_service_resp(
   const constants__t_channel_i service_mgr__channel,
   const constants__t_msg_type_i service_mgr__resp_typ,
   const constants__t_byte_buffer_i service_mgr__msg_buffer) {
   ;
}

void service_mgr__client_channel_connected_event_service(
   const constants__t_channel_config_idx_i service_mgr__channel_config_idx,
   const constants__t_channel_i service_mgr__channel) {
   {
      constants__t_session_i service_mgr__l_session_to_create;
      
      service_mgr__local_sc_activate_orphaned_sessions(service_mgr__channel_config_idx,
         service_mgr__channel);
      session_async_bs__is_session_to_create(service_mgr__channel_config_idx,
         &service_mgr__l_session_to_create);
      if (service_mgr__l_session_to_create != constants__c_session_indet) {
         service_mgr__local_create_session(service_mgr__l_session_to_create,
            service_mgr__channel);
      }
   }
}

void service_mgr__client_activate_new_session_wihtout_channel(
   const constants__t_channel_config_idx_i service_mgr__channel_config_idx,
   const constants__t_user_i service_mgr__user,
   t_bool * const service_mgr__bres) {
   {
      constants__t_session_i service_mgr__l_session;
      t_bool service_mgr__l_valid_session;
      
      session_mgr__client_init_session(&service_mgr__l_session);
      session_mgr__is_valid_session(service_mgr__l_session,
         &service_mgr__l_valid_session);
      if (service_mgr__l_valid_session == true) {
         session_async_bs__add_session_to_create(service_mgr__l_session,
            service_mgr__channel_config_idx,
            service_mgr__bres);
         if (*service_mgr__bres == true) {
            session_async_bs__add_session_to_activate(service_mgr__l_session,
               service_mgr__user,
               service_mgr__bres);
         }
         if (*service_mgr__bres == false) {
            session_async_bs__is_session_to_create(service_mgr__channel_config_idx,
               &service_mgr__l_session);
            session_mgr__delete_session(service_mgr__l_session);
         }
      }
      else {
         *service_mgr__bres = false;
      }
   }
}

void service_mgr__client_activate_new_session_with_channel(
   const constants__t_channel_config_idx_i service_mgr__channel_config_idx,
   const constants__t_channel_i service_mgr__channel,
   const constants__t_user_i service_mgr__user,
   t_bool * const service_mgr__bres) {
   {
      constants__t_session_i service_mgr__l_session;
      t_bool service_mgr__l_valid_session;
      
      session_mgr__client_init_session(&service_mgr__l_session);
      session_mgr__is_valid_session(service_mgr__l_session,
         &service_mgr__l_valid_session);
      if (service_mgr__l_valid_session == true) {
         service_mgr__local_create_session(service_mgr__l_session,
            service_mgr__channel);
         session_async_bs__add_session_to_activate(service_mgr__l_session,
            service_mgr__user,
            service_mgr__bres);
         if (*service_mgr__bres == false) {
            session_async_bs__is_session_to_create(service_mgr__channel_config_idx,
               &service_mgr__l_session);
            session_mgr__delete_session(service_mgr__l_session);
         }
      }
      else {
         *service_mgr__bres = false;
      }
   }
}

void service_mgr__client_activate_session(
   const constants__t_session_i service_mgr__session,
   const constants__t_user_i service_mgr__user,
   constants__t_StatusCode_i * const service_mgr__ret) {
   service_mgr__local_activate_session(service_mgr__session,
      service_mgr__user,
      service_mgr__ret);
}

void service_mgr__client_close_session(
   const constants__t_session_i service_mgr__session,
   constants__t_StatusCode_i * const service_mgr__ret) {
   {
      constants__t_msg_header_i service_mgr__l_msg_header;
      t_bool service_mgr__l_valid_msg_header;
      constants__t_msg_i service_mgr__l_req_msg;
      t_bool service_mgr__l_valid_msg;
      constants__t_channel_i service_mgr__l_channel;
      constants__t_request_handle_i service_mgr__l_req_handle;
      t_bool service_mgr__l_valid_req_handle;
      constants__t_session_token_i service_mgr__l_session_token;
      constants__t_byte_buffer_i service_mgr__l_buffer;
      t_bool service_mgr__l_valid_buffer;
      
      message_out_bs__alloc_req_msg(constants__e_msg_session_close_req,
         &service_mgr__l_msg_header,
         &service_mgr__l_req_msg);
      message_out_bs__is_valid_msg_out(service_mgr__l_req_msg,
         &service_mgr__l_valid_msg);
      message_out_bs__is_valid_msg_out_header(service_mgr__l_msg_header,
         &service_mgr__l_valid_msg_header);
      if ((service_mgr__l_valid_msg == true) &&
         (service_mgr__l_valid_msg_header == true)) {
         request_handle_bs__client_fresh_req_handle(constants__e_msg_session_close_resp,
            &service_mgr__l_req_handle);
         request_handle_bs__is_valid_req_handle(service_mgr__l_req_handle,
            &service_mgr__l_valid_req_handle);
         if (service_mgr__l_valid_req_handle == true) {
            session_mgr__client_close_req(service_mgr__session,
               service_mgr__l_req_handle,
               service_mgr__l_req_msg,
               service_mgr__ret,
               &service_mgr__l_channel,
               &service_mgr__l_session_token);
            if (*service_mgr__ret == constants__e_sc_ok) {
               message_out_bs__write_msg_out_header_req_handle(service_mgr__l_msg_header,
                  service_mgr__l_req_handle);
               message_out_bs__write_msg_out_header_session_token(service_mgr__l_msg_header,
                  service_mgr__l_session_token);
               message_out_bs__encode_msg(constants__e_msg_session_close_req,
                  service_mgr__l_msg_header,
                  service_mgr__l_req_msg,
                  &service_mgr__l_buffer);
               message_out_bs__is_valid_buffer_out(service_mgr__l_buffer,
                  &service_mgr__l_valid_buffer);
               if (service_mgr__l_valid_buffer == true) {
                  channel_mgr_bs__send_channel_msg_buffer(service_mgr__l_channel,
                     service_mgr__l_buffer,
                     service_mgr__ret);
               }
               else {
                  *service_mgr__ret = constants__e_sc_nok;
               }
               if (*service_mgr__ret != constants__e_sc_ok) {
                  request_handle_bs__client_remove_req_handle(service_mgr__l_req_handle);
                  session_mgr__client_session_mgr_close_session(service_mgr__session);
               }
            }
         }
         else {
            *service_mgr__ret = constants__e_sc_bad_out_of_memory;
         }
         message_out_bs__dealloc_msg_header_out(service_mgr__l_msg_header);
         message_out_bs__dealloc_msg_out(service_mgr__l_req_msg);
      }
      else {
         *service_mgr__ret = constants__e_sc_bad_out_of_memory;
      }
   }
}

void service_mgr__client_send_service_request_msg(
   const constants__t_session_i service_mgr__session,
   const constants__t_msg_i service_mgr__req_msg,
   constants__t_StatusCode_i * const service_mgr__ret) {
   {
      constants__t_msg_header_i service_mgr__l_msg_header;
      t_bool service_mgr__l_valid_msg_header;
      constants__t_msg_type_i service_mgr__l_req_typ;
      constants__t_msg_type_i service_mgr__l_resp_typ;
      t_bool service_mgr__l_valid_msg;
      constants__t_channel_i service_mgr__l_channel;
      constants__t_request_handle_i service_mgr__l_req_handle;
      t_bool service_mgr__l_valid_req_handle;
      constants__t_session_token_i service_mgr__l_session_token;
      constants__t_byte_buffer_i service_mgr__l_buffer;
      t_bool service_mgr__l_valid_buffer;
      
      message_out_bs__is_valid_msg_out(service_mgr__req_msg,
         &service_mgr__l_valid_msg);
      if (service_mgr__l_valid_msg == true) {
         message_in_bs__get_msg_in_type(service_mgr__req_msg,
            &service_mgr__l_req_typ);
         switch (service_mgr__l_req_typ) {
         case constants__e_msg_session_read_req:
         case constants__e_msg_session_write_req:
            service_mgr__get_response_type(service_mgr__l_req_typ,
               &service_mgr__l_resp_typ);
            message_out_bs__alloc_app_req_msg_header(service_mgr__l_req_typ,
               service_mgr__req_msg,
               &service_mgr__l_msg_header);
            message_out_bs__is_valid_msg_out_header(service_mgr__l_msg_header,
               &service_mgr__l_valid_msg_header);
            request_handle_bs__client_fresh_req_handle(service_mgr__l_resp_typ,
               &service_mgr__l_req_handle);
            request_handle_bs__is_valid_req_handle(service_mgr__l_req_handle,
               &service_mgr__l_valid_req_handle);
            if ((service_mgr__l_valid_req_handle == true) &&
               (service_mgr__l_valid_msg_header == true)) {
               message_out_bs__bless_msg_out(service_mgr__l_req_typ,
                  service_mgr__l_msg_header,
                  service_mgr__req_msg);
               session_mgr__client_validate_session_service_req(service_mgr__session,
                  service_mgr__l_req_handle,
                  service_mgr__req_msg,
                  service_mgr__ret,
                  &service_mgr__l_channel,
                  &service_mgr__l_session_token);
               if (*service_mgr__ret == constants__e_sc_ok) {
                  message_out_bs__write_msg_out_header_req_handle(service_mgr__l_msg_header,
                     service_mgr__l_req_handle);
                  message_out_bs__write_msg_out_header_session_token(service_mgr__l_msg_header,
                     service_mgr__l_session_token);
                  message_out_bs__encode_msg(constants__e_msg_session_close_req,
                     service_mgr__l_msg_header,
                     service_mgr__req_msg,
                     &service_mgr__l_buffer);
                  message_out_bs__is_valid_buffer_out(service_mgr__l_buffer,
                     &service_mgr__l_valid_buffer);
                  if (service_mgr__l_valid_buffer == true) {
                     channel_mgr_bs__send_channel_msg_buffer(service_mgr__l_channel,
                        service_mgr__l_buffer,
                        service_mgr__ret);
                  }
                  else {
                     *service_mgr__ret = constants__e_sc_nok;
                  }
               }
            }
            else {
               *service_mgr__ret = constants__e_sc_bad_out_of_memory;
            }
            if (service_mgr__l_valid_msg_header == true) {
               message_out_bs__dealloc_msg_header_out(service_mgr__l_msg_header);
            }
            message_out_bs__dealloc_msg_out(service_mgr__req_msg);
            break;
         default:
            *service_mgr__ret = constants__e_sc_bad_invalid_argument;
            break;
         }
      }
      else {
         *service_mgr__ret = constants__e_sc_bad_invalid_argument;
      }
   }
}

