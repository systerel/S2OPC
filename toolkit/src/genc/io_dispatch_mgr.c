/******************************************************************************

 File Name            : io_dispatch_mgr.c

 Date                 : 13/07/2017 16:54:05

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "io_dispatch_mgr.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void io_dispatch_mgr__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void io_dispatch_mgr__get_response_type(
   const constants__t_msg_type io_dispatch_mgr__req_msg_typ,
   constants__t_msg_type * const io_dispatch_mgr__resp_msg_typ) {
   switch (io_dispatch_mgr__req_msg_typ) {
   case constants__e_msg_tcpua_hello:
      *io_dispatch_mgr__resp_msg_typ = constants__e_msg_tcpua_ack;
      break;
   case constants__e_msg_sc_open_channel_req:
      *io_dispatch_mgr__resp_msg_typ = constants__e_msg_sc_open_channel_resp;
      break;
   case constants__e_msg_sc_close_channel_req:
      *io_dispatch_mgr__resp_msg_typ = constants__e_msg_sc_close_channel_resp;
      break;
   case constants__e_msg_public_service_req:
      *io_dispatch_mgr__resp_msg_typ = constants__e_msg_public_service_resp;
      break;
   case constants__e_msg_session_create_req:
      *io_dispatch_mgr__resp_msg_typ = constants__e_msg_session_create_resp;
      break;
   case constants__e_msg_session_activate_req:
      *io_dispatch_mgr__resp_msg_typ = constants__e_msg_session_activate_resp;
      break;
   case constants__e_msg_session_close_req:
      *io_dispatch_mgr__resp_msg_typ = constants__e_msg_session_close_resp;
      break;
   case constants__e_msg_session_read_req:
      *io_dispatch_mgr__resp_msg_typ = constants__e_msg_session_read_resp;
      break;
   default:
      break;
   }
}

void io_dispatch_mgr__is_request_type(
   const constants__t_msg_type io_dispatch_mgr__msg_typ,
   t_bool * const io_dispatch_mgr__bres) {
   switch (io_dispatch_mgr__msg_typ) {
   case constants__e_msg_tcpua_hello:
   case constants__e_msg_sc_open_channel_req:
   case constants__e_msg_sc_close_channel_req:
   case constants__e_msg_public_service_req:
   case constants__e_msg_session_create_req:
   case constants__e_msg_session_activate_req:
   case constants__e_msg_session_close_req:
   case constants__e_msg_session_read_req:
      *io_dispatch_mgr__bres = true;
      break;
   default:
      *io_dispatch_mgr__bres = false;
      break;
   }
}

void io_dispatch_mgr__treat_write_request(
   const constants__t_ByteString_i io_dispatch_mgr__req_payload,
   const constants__t_UserId_i io_dispatch_mgr__userid,
   constants__t_StatusCode_i * const io_dispatch_mgr__StatusCode_service) {
   {
      t_entier4 io_dispatch_mgr__l_nb_req;
      
      service_write_decode_bs__decode_write_request(io_dispatch_mgr__req_payload,
         io_dispatch_mgr__StatusCode_service);
      if (*io_dispatch_mgr__StatusCode_service == constants__e_sc_ok) {
         service_write_decode_bs__get_nb_WriteValue(&io_dispatch_mgr__l_nb_req);
         address_space__alloc_write_request_responses(io_dispatch_mgr__l_nb_req,
            io_dispatch_mgr__StatusCode_service);
         if (*io_dispatch_mgr__StatusCode_service == constants__e_sc_ok) {
            address_space__treat_write_request_WriteValues(io_dispatch_mgr__userid,
               io_dispatch_mgr__StatusCode_service);
         }
      }
      service_write_decode_bs__free_write_request();
   }
}

void io_dispatch_mgr__msgs_memory_changed(void) {
   message_in_bs__msg_in_memory_changed();
   message_out_bs__msg_out_memory_changed();
}

void io_dispatch_mgr__receive_msg(
   const constants__t_channel_i io_dispatch_mgr__channel,
   const constants__t_msg_i io_dispatch_mgr__msg) {
   {
      constants__t_request_handle_i io_dispatch_mgr__l_request_handle;
      constants__t_session_token_i io_dispatch_mgr__l_session_token;
      constants__t_session_i io_dispatch_mgr__l_session;
      t_bool io_dispatch_mgr__l_valid_msg;
      constants__t_msg_type io_dispatch_mgr__l_msg_type;
      t_bool io_dispatch_mgr__l_valid_channel;
      t_bool io_dispatch_mgr__l_is_client;
      t_bool io_dispatch_mgr__l_is_req;
      t_bool io_dispatch_mgr__l_is_valid_req;
      t_bool io_dispatch_mgr__l_snd_session_err;
      t_bool io_dispatch_mgr__l_is_valid_resp;
      constants__t_StatusCode_i io_dispatch_mgr__l_status;
      constants__t_msg_type io_dispatch_mgr__l_resp_msg_typ;
      constants__t_msg_i io_dispatch_mgr__l_resp_msg;
      t_bool io_dispatch_mgr__l_to_send;
      constants__t_StatusCode_i io_dispatch_mgr__l_ret;
      
      channel_mgr_bs__is_valid_channel(io_dispatch_mgr__channel,
         &io_dispatch_mgr__l_valid_channel);
      message_in_bs__is_valid_msg_in(io_dispatch_mgr__msg,
         &io_dispatch_mgr__l_valid_msg);
      if ((io_dispatch_mgr__l_valid_channel == true) &&
         (io_dispatch_mgr__l_valid_msg == true)) {
         message_in_bs__get_msg_in_type(io_dispatch_mgr__msg,
            &io_dispatch_mgr__l_msg_type);
         switch (io_dispatch_mgr__l_msg_type) {
         case constants__e_msg_tcpua_hello:
         case constants__e_msg_tcpua_ack:
         case constants__e_msg_sc_open_channel_req:
         case constants__e_msg_sc_open_channel_resp:
         case constants__e_msg_sc_close_channel_req:
         case constants__e_msg_sc_close_channel_resp:
         case constants__e_msg_public_service_req:
         case constants__e_msg_public_service_resp:
            if (io_dispatch_mgr__l_msg_type == constants__e_msg_tcpua_hello) {
               channel_mgr_bs__receive_hello_msg(io_dispatch_mgr__msg);
            }
            else {
               channel_mgr_bs__receive_channel_msg(io_dispatch_mgr__channel,
                  io_dispatch_mgr__msg);
            }
            break;
         default:
            channel_mgr_bs__is_client_channel(io_dispatch_mgr__channel,
               &io_dispatch_mgr__l_is_client);
            io_dispatch_mgr__is_request_type(io_dispatch_mgr__l_msg_type,
               &io_dispatch_mgr__l_is_req);
            if ((io_dispatch_mgr__l_is_client == true) &&
               (io_dispatch_mgr__l_is_req == false)) {
               switch (io_dispatch_mgr__l_msg_type) {
               case constants__e_msg_session_create_resp:
               case constants__e_msg_session_activate_resp:
               case constants__e_msg_session_close_resp:
                  message_in_bs__read_msg_header_req_handle(io_dispatch_mgr__msg,
                     &io_dispatch_mgr__l_request_handle);
                  session_mgr__receive_session_resp(io_dispatch_mgr__channel,
                     io_dispatch_mgr__l_request_handle,
                     io_dispatch_mgr__msg,
                     io_dispatch_mgr__l_msg_type);
                  request_handle_bs__remove_req_handle(io_dispatch_mgr__l_request_handle);
                  break;
               case constants__e_msg_session_read_resp:
                  message_in_bs__read_msg_header_req_handle(io_dispatch_mgr__msg,
                     &io_dispatch_mgr__l_request_handle);
                  session_mgr__cli_validate_session_service_resp(io_dispatch_mgr__channel,
                     io_dispatch_mgr__l_request_handle,
                     io_dispatch_mgr__msg,
                     &io_dispatch_mgr__l_is_valid_resp);
                  if (io_dispatch_mgr__l_is_valid_resp == true) {
                     message_in_bs__read_msg_resp_header_service_status(io_dispatch_mgr__msg,
                        &io_dispatch_mgr__l_status);
                     service_read_cli_cb_bs__cli_service_read_response(io_dispatch_mgr__msg,
                        io_dispatch_mgr__l_status);
                  }
                  break;
               default:
                  break;
               }
            }
            else if ((io_dispatch_mgr__l_is_client == false) &&
               (io_dispatch_mgr__l_is_req == true)) {
               switch (io_dispatch_mgr__l_msg_type) {
               case constants__e_msg_session_create_req:
               case constants__e_msg_session_activate_req:
               case constants__e_msg_session_close_req:
                  message_in_bs__read_msg_header_req_handle(io_dispatch_mgr__msg,
                     &io_dispatch_mgr__l_request_handle);
                  message_in_bs__read_msg_req_header_session_token(io_dispatch_mgr__msg,
                     &io_dispatch_mgr__l_session_token);
                  io_dispatch_mgr__get_response_type(io_dispatch_mgr__l_msg_type,
                     &io_dispatch_mgr__l_resp_msg_typ);
                  message_out_bs__alloc_msg(io_dispatch_mgr__l_resp_msg_typ,
                     &io_dispatch_mgr__l_resp_msg);
                  message_out_bs__is_valid_msg_out(io_dispatch_mgr__l_resp_msg,
                     &io_dispatch_mgr__l_valid_msg);
                  if (io_dispatch_mgr__l_valid_msg == true) {
                     session_mgr__receive_session_req(io_dispatch_mgr__channel,
                        io_dispatch_mgr__l_request_handle,
                        io_dispatch_mgr__l_session_token,
                        io_dispatch_mgr__msg,
                        io_dispatch_mgr__l_msg_type,
                        io_dispatch_mgr__l_resp_msg,
                        &io_dispatch_mgr__l_to_send,
                        &io_dispatch_mgr__l_session);
                     if (io_dispatch_mgr__l_to_send == true) {
                        message_out_bs__write_msg_out_header_req_handle(io_dispatch_mgr__l_resp_msg,
                           io_dispatch_mgr__l_request_handle);
                        channel_mgr_bs__send_channel_msg(io_dispatch_mgr__channel,
                           io_dispatch_mgr__l_resp_msg,
                           &io_dispatch_mgr__l_ret);
                        if (io_dispatch_mgr__l_ret != constants__e_sc_ok) {
                           session_mgr__cli_close_session(io_dispatch_mgr__l_session);
                        }
                     }
                     message_out_bs__dealloc_msg_out(io_dispatch_mgr__l_resp_msg);
                  }
                  break;
               case constants__e_msg_session_read_req:
                  message_in_bs__read_msg_header_req_handle(io_dispatch_mgr__msg,
                     &io_dispatch_mgr__l_request_handle);
                  message_in_bs__read_msg_req_header_session_token(io_dispatch_mgr__msg,
                     &io_dispatch_mgr__l_session_token);
                  session_mgr__srv_validate_session_service_req(io_dispatch_mgr__channel,
                     io_dispatch_mgr__l_request_handle,
                     io_dispatch_mgr__l_session_token,
                     io_dispatch_mgr__msg,
                     &io_dispatch_mgr__l_is_valid_req,
                     &io_dispatch_mgr__l_snd_session_err);
                  if (io_dispatch_mgr__l_is_valid_req == true) {
                     message_out_bs__alloc_msg(constants__e_msg_session_read_resp,
                        &io_dispatch_mgr__l_resp_msg);
                     message_out_bs__is_valid_msg_out(io_dispatch_mgr__l_resp_msg,
                        &io_dispatch_mgr__l_valid_msg);
                     session_mgr__get_session_from_token(io_dispatch_mgr__l_session_token,
                        &io_dispatch_mgr__l_session);
                     if (io_dispatch_mgr__l_valid_msg == true) {
                        service_read__treat_read_request(io_dispatch_mgr__msg,
                           io_dispatch_mgr__l_resp_msg);
                        session_mgr__srv_validate_session_service_resp(io_dispatch_mgr__channel,
                           io_dispatch_mgr__l_session,
                           io_dispatch_mgr__l_request_handle,
                           io_dispatch_mgr__msg,
                           io_dispatch_mgr__l_resp_msg,
                           &io_dispatch_mgr__l_is_valid_resp,
                           &io_dispatch_mgr__l_snd_session_err);
                        if (io_dispatch_mgr__l_is_valid_resp == true) {
                           message_out_bs__write_msg_out_header_req_handle(io_dispatch_mgr__l_resp_msg,
                              io_dispatch_mgr__l_request_handle);
                           channel_mgr_bs__send_channel_msg(io_dispatch_mgr__channel,
                              io_dispatch_mgr__l_resp_msg,
                              &io_dispatch_mgr__l_ret);
                        }
                        message_out_bs__dealloc_msg_out(io_dispatch_mgr__l_resp_msg);
                     }
                  }
                  if (io_dispatch_mgr__l_snd_session_err == true) {
                     message_out_bs__alloc_msg(constants__e_msg_session_read_resp,
                        &io_dispatch_mgr__l_resp_msg);
                     message_out_bs__is_valid_msg_out(io_dispatch_mgr__l_resp_msg,
                        &io_dispatch_mgr__l_valid_msg);
                     if (io_dispatch_mgr__l_valid_msg == true) {
                        message_out_bs__write_msg_out_header_req_handle(io_dispatch_mgr__l_resp_msg,
                           io_dispatch_mgr__l_request_handle);
                        message_out_bs__write_msg_resp_header_service_status(io_dispatch_mgr__l_resp_msg,
                           constants__e_sc_bad_session_closed);
                        channel_mgr_bs__send_channel_msg(io_dispatch_mgr__channel,
                           io_dispatch_mgr__l_resp_msg,
                           &io_dispatch_mgr__l_ret);
                        message_out_bs__dealloc_msg_out(io_dispatch_mgr__l_resp_msg);
                     }
                  }
                  break;
               default:
                  break;
               }
            }
            else {
               ;
            }
            break;
         }
      }
   }
}

void io_dispatch_mgr__create_session(
   const constants__t_endpoint_i io_dispatch_mgr__endpoint,
   constants__t_session_i * const io_dispatch_mgr__nsession) {
   {
      constants__t_channel_i io_dispatch_mgr__l_channel;
      t_bool io_dispatch_mgr__l_valid_channel;
      constants__t_msg_i io_dispatch_mgr__l_req_msg;
      constants__t_request_handle_i io_dispatch_mgr__l_req_handle;
      t_bool io_dispatch_mgr__l_valid_req_handle;
      t_bool io_dispatch_mgr__l_valid_msg;
      t_bool io_dispatch_mgr__l_valid_session;
      constants__t_StatusCode_i io_dispatch_mgr__l_ret;
      
      channel_mgr_bs__get_valid_channel(io_dispatch_mgr__endpoint,
         &io_dispatch_mgr__l_channel);
      channel_mgr_bs__is_valid_channel(io_dispatch_mgr__l_channel,
         &io_dispatch_mgr__l_valid_channel);
      if (io_dispatch_mgr__l_valid_channel == false) {
         channel_mgr_bs__open_secure_channel(io_dispatch_mgr__endpoint,
            &io_dispatch_mgr__l_channel);
      }
      channel_mgr_bs__is_valid_channel(io_dispatch_mgr__l_channel,
         &io_dispatch_mgr__l_valid_channel);
      if (io_dispatch_mgr__l_valid_channel == false) {
         *io_dispatch_mgr__nsession = constants__c_session_indet;
      }
      else {
         message_out_bs__alloc_msg(constants__e_msg_session_create_req,
            &io_dispatch_mgr__l_req_msg);
         message_out_bs__is_valid_msg_out(io_dispatch_mgr__l_req_msg,
            &io_dispatch_mgr__l_valid_msg);
         if (io_dispatch_mgr__l_valid_msg == true) {
            request_handle_bs__fresh_req_handle(&io_dispatch_mgr__l_req_handle);
            request_handle_bs__is_valid_req_handle(io_dispatch_mgr__l_req_handle,
               &io_dispatch_mgr__l_valid_req_handle);
            if (io_dispatch_mgr__l_valid_req_handle == true) {
               session_mgr__cli_create_req(io_dispatch_mgr__l_channel,
                  io_dispatch_mgr__l_req_handle,
                  io_dispatch_mgr__l_req_msg,
                  io_dispatch_mgr__nsession);
               session_mgr__is_valid_session(*io_dispatch_mgr__nsession,
                  &io_dispatch_mgr__l_valid_session);
               if (io_dispatch_mgr__l_valid_session == true) {
                  message_out_bs__write_msg_out_header_req_handle(io_dispatch_mgr__l_req_msg,
                     io_dispatch_mgr__l_req_handle);
                  channel_mgr_bs__send_channel_msg(io_dispatch_mgr__l_channel,
                     io_dispatch_mgr__l_req_msg,
                     &io_dispatch_mgr__l_ret);
                  if (io_dispatch_mgr__l_ret != constants__e_sc_ok) {
                     request_handle_bs__remove_req_handle(io_dispatch_mgr__l_req_handle);
                     session_mgr__cli_close_session(*io_dispatch_mgr__nsession);
                     *io_dispatch_mgr__nsession = constants__c_session_indet;
                  }
               }
               else {
                  request_handle_bs__remove_req_handle(io_dispatch_mgr__l_req_handle);
                  session_mgr__cli_close_session(*io_dispatch_mgr__nsession);
               }
            }
            message_out_bs__dealloc_msg_out(io_dispatch_mgr__l_req_msg);
         }
         else {
            *io_dispatch_mgr__nsession = constants__c_session_indet;
         }
      }
   }
}

void io_dispatch_mgr__activate_session(
   const constants__t_session_i io_dispatch_mgr__session,
   const constants__t_user_i io_dispatch_mgr__user,
   constants__t_StatusCode_i * const io_dispatch_mgr__ret) {
   {
      constants__t_msg_i io_dispatch_mgr__l_req_msg;
      t_bool io_dispatch_mgr__l_valid_msg;
      constants__t_StatusCode_i io_dispatch_mgr__l_ret;
      constants__t_channel_i io_dispatch_mgr__l_channel;
      constants__t_request_handle_i io_dispatch_mgr__l_req_handle;
      t_bool io_dispatch_mgr__l_valid_req_handle;
      constants__t_session_token_i io_dispatch_mgr__l_session_token;
      
      message_out_bs__alloc_msg(constants__e_msg_session_activate_req,
         &io_dispatch_mgr__l_req_msg);
      message_out_bs__is_valid_msg_out(io_dispatch_mgr__l_req_msg,
         &io_dispatch_mgr__l_valid_msg);
      if (io_dispatch_mgr__l_valid_msg == true) {
         request_handle_bs__fresh_req_handle(&io_dispatch_mgr__l_req_handle);
         request_handle_bs__is_valid_req_handle(io_dispatch_mgr__l_req_handle,
            &io_dispatch_mgr__l_valid_req_handle);
         if (io_dispatch_mgr__l_valid_req_handle == true) {
            request_handle_bs__fresh_req_handle(&io_dispatch_mgr__l_req_handle);
            request_handle_bs__is_valid_req_handle(io_dispatch_mgr__l_req_handle,
               &io_dispatch_mgr__l_valid_req_handle);
            if (io_dispatch_mgr__l_valid_req_handle == true) {
               session_mgr__cli_user_activate_req(io_dispatch_mgr__session,
                  io_dispatch_mgr__l_req_handle,
                  io_dispatch_mgr__user,
                  io_dispatch_mgr__l_req_msg,
                  &io_dispatch_mgr__l_ret,
                  &io_dispatch_mgr__l_channel,
                  &io_dispatch_mgr__l_session_token);
               if (io_dispatch_mgr__l_ret == constants__e_sc_ok) {
                  message_out_bs__write_msg_out_header_req_handle(io_dispatch_mgr__l_req_msg,
                     io_dispatch_mgr__l_req_handle);
                  message_out_bs__write_msg_out_header_session_token(io_dispatch_mgr__l_req_msg,
                     io_dispatch_mgr__l_session_token);
                  channel_mgr_bs__send_channel_msg(io_dispatch_mgr__l_channel,
                     io_dispatch_mgr__l_req_msg,
                     &io_dispatch_mgr__l_ret);
                  if (io_dispatch_mgr__l_ret != constants__e_sc_ok) {
                     request_handle_bs__remove_req_handle(io_dispatch_mgr__l_req_handle);
                     session_mgr__cli_close_session(io_dispatch_mgr__session);
                  }
               }
            }
            else {
               io_dispatch_mgr__l_ret = constants__e_sc_bad_out_of_memory;
            }
         }
         else {
            io_dispatch_mgr__l_ret = constants__e_sc_bad_out_of_memory;
         }
         message_out_bs__dealloc_msg_out(io_dispatch_mgr__l_req_msg);
      }
      else {
         io_dispatch_mgr__l_ret = constants__e_sc_bad_out_of_memory;
      }
      *io_dispatch_mgr__ret = io_dispatch_mgr__l_ret;
   }
}

void io_dispatch_mgr__close_session(
   const constants__t_session_i io_dispatch_mgr__session,
   constants__t_StatusCode_i * const io_dispatch_mgr__ret) {
   {
      constants__t_msg_i io_dispatch_mgr__l_req_msg;
      t_bool io_dispatch_mgr__l_valid_msg;
      constants__t_channel_i io_dispatch_mgr__l_channel;
      constants__t_request_handle_i io_dispatch_mgr__l_req_handle;
      t_bool io_dispatch_mgr__l_valid_req_handle;
      constants__t_session_token_i io_dispatch_mgr__l_session_token;
      
      message_out_bs__alloc_msg(constants__e_msg_session_close_req,
         &io_dispatch_mgr__l_req_msg);
      message_out_bs__is_valid_msg_out(io_dispatch_mgr__l_req_msg,
         &io_dispatch_mgr__l_valid_msg);
      if (io_dispatch_mgr__l_valid_msg == true) {
         request_handle_bs__fresh_req_handle(&io_dispatch_mgr__l_req_handle);
         request_handle_bs__is_valid_req_handle(io_dispatch_mgr__l_req_handle,
            &io_dispatch_mgr__l_valid_req_handle);
         if (io_dispatch_mgr__l_valid_req_handle == true) {
            session_mgr__cli_close_req(io_dispatch_mgr__session,
               io_dispatch_mgr__l_req_handle,
               io_dispatch_mgr__l_req_msg,
               io_dispatch_mgr__ret,
               &io_dispatch_mgr__l_channel,
               &io_dispatch_mgr__l_session_token);
            if (*io_dispatch_mgr__ret == constants__e_sc_ok) {
               message_out_bs__write_msg_out_header_req_handle(io_dispatch_mgr__l_req_msg,
                  io_dispatch_mgr__l_req_handle);
               message_out_bs__write_msg_out_header_session_token(io_dispatch_mgr__l_req_msg,
                  io_dispatch_mgr__l_session_token);
               channel_mgr_bs__send_channel_msg(io_dispatch_mgr__l_channel,
                  io_dispatch_mgr__l_req_msg,
                  io_dispatch_mgr__ret);
               if (*io_dispatch_mgr__ret != constants__e_sc_ok) {
                  request_handle_bs__remove_req_handle(io_dispatch_mgr__l_req_handle);
                  session_mgr__cli_close_session(io_dispatch_mgr__session);
               }
            }
         }
         else {
            *io_dispatch_mgr__ret = constants__e_sc_bad_out_of_memory;
         }
         message_out_bs__dealloc_msg_out(io_dispatch_mgr__l_req_msg);
      }
      else {
         *io_dispatch_mgr__ret = constants__e_sc_bad_out_of_memory;
      }
   }
}

void io_dispatch_mgr__secure_channel_lost(
   const constants__t_channel_i io_dispatch_mgr__channel) {
   {
      t_bool io_dispatch_mgr__l_valid_channel;
      t_bool io_dispatch_mgr__l_valid_new_channel;
      t_bool io_dispatch_mgr__l_is_client;
      constants__t_endpoint_i io_dispatch_mgr__l_endpoint;
      constants__t_channel_i io_dispatch_mgr__l_new_channel;
      constants__t_msg_i io_dispatch_mgr__l_req_msg;
      t_bool io_dispatch_mgr__l_valid_msg;
      t_bool io_dispatch_mgr__l_continue;
      constants__t_session_i io_dispatch_mgr__l_session;
      constants__t_StatusCode_i io_dispatch_mgr__l_ret;
      constants__t_request_handle_i io_dispatch_mgr__l_req_handle;
      t_bool io_dispatch_mgr__l_valid_req_handle;
      constants__t_session_token_i io_dispatch_mgr__l_session_token;
      
      channel_mgr_bs__is_valid_channel(io_dispatch_mgr__channel,
         &io_dispatch_mgr__l_valid_channel);
      if (io_dispatch_mgr__l_valid_channel == true) {
         channel_mgr_bs__is_client_channel(io_dispatch_mgr__channel,
            &io_dispatch_mgr__l_is_client);
         if (io_dispatch_mgr__l_is_client == true) {
            channel_mgr_bs__get_channel_info(io_dispatch_mgr__channel,
               &io_dispatch_mgr__l_endpoint);
            channel_mgr_bs__get_valid_channel(io_dispatch_mgr__l_endpoint,
               &io_dispatch_mgr__l_new_channel);
            if (io_dispatch_mgr__l_new_channel == constants__c_channel_indet) {
               channel_mgr_bs__open_secure_channel(io_dispatch_mgr__l_endpoint,
                  &io_dispatch_mgr__l_new_channel);
            }
            channel_mgr_bs__is_valid_channel(io_dispatch_mgr__l_new_channel,
               &io_dispatch_mgr__l_valid_new_channel);
            session_mgr__cli_secure_channel_lost(io_dispatch_mgr__channel);
            if (io_dispatch_mgr__l_valid_new_channel == true) {
               session_mgr__init_iter_orphaned_t_session(io_dispatch_mgr__channel,
                  &io_dispatch_mgr__l_continue);
               while (io_dispatch_mgr__l_continue == true) {
                  session_mgr__continue_iter_orphaned_t_session(&io_dispatch_mgr__l_session,
                     &io_dispatch_mgr__l_continue);
                  message_out_bs__alloc_msg(constants__e_msg_session_activate_req,
                     &io_dispatch_mgr__l_req_msg);
                  message_out_bs__is_valid_msg_out(io_dispatch_mgr__l_req_msg,
                     &io_dispatch_mgr__l_valid_msg);
                  if (io_dispatch_mgr__l_valid_msg == true) {
                     request_handle_bs__fresh_req_handle(&io_dispatch_mgr__l_req_handle);
                     request_handle_bs__is_valid_req_handle(io_dispatch_mgr__l_req_handle,
                        &io_dispatch_mgr__l_valid_req_handle);
                     if (io_dispatch_mgr__l_valid_req_handle == true) {
                        session_mgr__cli_sc_activate_req(io_dispatch_mgr__l_session,
                           io_dispatch_mgr__l_req_handle,
                           io_dispatch_mgr__l_new_channel,
                           io_dispatch_mgr__l_req_msg,
                           &io_dispatch_mgr__l_ret,
                           &io_dispatch_mgr__l_session_token);
                        if (io_dispatch_mgr__l_ret == constants__e_sc_ok) {
                           message_out_bs__write_msg_out_header_req_handle(io_dispatch_mgr__l_req_msg,
                              io_dispatch_mgr__l_req_handle);
                           message_out_bs__write_msg_out_header_session_token(io_dispatch_mgr__l_req_msg,
                              io_dispatch_mgr__l_session_token);
                           channel_mgr_bs__send_channel_msg(io_dispatch_mgr__l_new_channel,
                              io_dispatch_mgr__l_req_msg,
                              &io_dispatch_mgr__l_ret);
                           if (io_dispatch_mgr__l_ret != constants__e_sc_ok) {
                              request_handle_bs__remove_req_handle(io_dispatch_mgr__l_req_handle);
                              session_mgr__cli_close_session(io_dispatch_mgr__l_session);
                           }
                        }
                     }
                     message_out_bs__dealloc_msg_out(io_dispatch_mgr__l_req_msg);
                  }
               }
            }
         }
         else {
            session_mgr__srv_secure_channel_lost(io_dispatch_mgr__channel);
         }
         channel_mgr_bs__channel_lost(io_dispatch_mgr__channel);
      }
   }
}

void io_dispatch_mgr__send_service_request_msg(
   const constants__t_session_i io_dispatch_mgr__session,
   const constants__t_msg_i io_dispatch_mgr__req_msg,
   constants__t_StatusCode_i * const io_dispatch_mgr__ret) {
   {
      constants__t_msg_type io_dispatch_mgr__l_msg_typ;
      t_bool io_dispatch_mgr__l_valid_msg;
      constants__t_channel_i io_dispatch_mgr__l_channel;
      constants__t_request_handle_i io_dispatch_mgr__l_req_handle;
      t_bool io_dispatch_mgr__l_valid_req_handle;
      constants__t_session_token_i io_dispatch_mgr__l_session_token;
      
      message_in_bs__get_msg_in_type(io_dispatch_mgr__req_msg,
         &io_dispatch_mgr__l_msg_typ);
      switch (io_dispatch_mgr__l_msg_typ) {
      case constants__e_msg_session_read_req:
         message_out_bs__bless_msg_out(io_dispatch_mgr__req_msg,
            io_dispatch_mgr__l_msg_typ);
         message_out_bs__is_valid_msg_out(io_dispatch_mgr__req_msg,
            &io_dispatch_mgr__l_valid_msg);
         if (io_dispatch_mgr__l_valid_msg == true) {
            request_handle_bs__fresh_req_handle(&io_dispatch_mgr__l_req_handle);
            request_handle_bs__is_valid_req_handle(io_dispatch_mgr__l_req_handle,
               &io_dispatch_mgr__l_valid_req_handle);
            if (io_dispatch_mgr__l_valid_req_handle == true) {
               session_mgr__cli_validate_session_service_req(io_dispatch_mgr__session,
                  io_dispatch_mgr__l_req_handle,
                  io_dispatch_mgr__req_msg,
                  io_dispatch_mgr__ret,
                  &io_dispatch_mgr__l_channel,
                  &io_dispatch_mgr__l_session_token);
               if (*io_dispatch_mgr__ret == constants__e_sc_ok) {
                  message_out_bs__write_msg_out_header_req_handle(io_dispatch_mgr__req_msg,
                     io_dispatch_mgr__l_req_handle);
                  message_out_bs__write_msg_out_header_session_token(io_dispatch_mgr__req_msg,
                     io_dispatch_mgr__l_session_token);
                  channel_mgr_bs__send_channel_msg(io_dispatch_mgr__l_channel,
                     io_dispatch_mgr__req_msg,
                     io_dispatch_mgr__ret);
               }
            }
            else {
               *io_dispatch_mgr__ret = constants__e_sc_bad_out_of_memory;
            }
         }
         else {
            *io_dispatch_mgr__ret = constants__e_sc_bad_invalid_argument;
         }
         break;
      default:
         *io_dispatch_mgr__ret = constants__e_sc_bad_invalid_argument;
         break;
      }
   }
}

void io_dispatch_mgr__close_all_active_connections(void) {
   channel_mgr_bs__close_all_channel();
}

