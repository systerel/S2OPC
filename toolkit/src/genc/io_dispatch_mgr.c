/******************************************************************************

 File Name            : io_dispatch_mgr.c

 Date                 : 01/08/2017 11:32:37

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
   case constants__e_msg_session_write_req:
      *io_dispatch_mgr__resp_msg_typ = constants__e_msg_session_write_resp;
      break;
   default:
      break;
   }
}

void io_dispatch_mgr__is_request_type(
   const constants__t_msg_type io_dispatch_mgr__msg_typ,
   t_bool * const io_dispatch_mgr__bres) {
   switch (io_dispatch_mgr__msg_typ) {
   case constants__e_msg_public_service_req:
   case constants__e_msg_session_create_req:
   case constants__e_msg_session_activate_req:
   case constants__e_msg_session_close_req:
   case constants__e_msg_session_read_req:
   case constants__e_msg_session_write_req:
      *io_dispatch_mgr__bres = true;
      break;
   default:
      *io_dispatch_mgr__bres = false;
      break;
   }
}

void io_dispatch_mgr__treat_read_request(
   const constants__t_msg_i io_dispatch_mgr__p_request_msg,
   const constants__t_msg_i io_dispatch_mgr__p_response_msg) {
   {
      t_entier4 io_dispatch_mgr__l_nb_ReadValue;
      t_bool io_dispatch_mgr__l_is_valid;
      
      service_read__check_ReadRequest(io_dispatch_mgr__p_request_msg,
         &io_dispatch_mgr__l_is_valid,
         &io_dispatch_mgr__l_nb_ReadValue);
      if (io_dispatch_mgr__l_is_valid == true) {
         service_read__alloc_read_response(io_dispatch_mgr__l_nb_ReadValue,
            io_dispatch_mgr__p_response_msg,
            &io_dispatch_mgr__l_is_valid);
         if (io_dispatch_mgr__l_is_valid == true) {
            service_read__fill_read_response(io_dispatch_mgr__p_request_msg,
               io_dispatch_mgr__p_response_msg);
         }
      }
   }
}

void io_dispatch_mgr__treat_write_request(
   const constants__t_ByteString_i io_dispatch_mgr__req_payload,
   const constants__t_user_i io_dispatch_mgr__userid,
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

void io_dispatch_mgr__local_create_session(
   const constants__t_session_i io_dispatch_mgr__session,
   const constants__t_channel_i io_dispatch_mgr__channel) {
   {
      constants__t_msg_i io_dispatch_mgr__l_req_msg;
      constants__t_request_handle_i io_dispatch_mgr__l_req_handle;
      t_bool io_dispatch_mgr__l_valid_req_handle;
      t_bool io_dispatch_mgr__l_valid_msg;
      t_bool io_dispatch_mgr__l_valid_session;
      constants__t_StatusCode_i io_dispatch_mgr__l_ret;
      
      message_out_bs__alloc_req_msg(constants__e_msg_session_create_req,
         &io_dispatch_mgr__l_req_msg);
      message_out_bs__is_valid_msg_out(io_dispatch_mgr__l_req_msg,
         &io_dispatch_mgr__l_valid_msg);
      if (io_dispatch_mgr__l_valid_msg == true) {
         request_handle_bs__fresh_req_handle(&io_dispatch_mgr__l_req_handle);
         request_handle_bs__is_valid_req_handle(io_dispatch_mgr__l_req_handle,
            &io_dispatch_mgr__l_valid_req_handle);
         if (io_dispatch_mgr__l_valid_req_handle == true) {
            session_mgr__is_valid_session(io_dispatch_mgr__session,
               &io_dispatch_mgr__l_valid_session);
            if (io_dispatch_mgr__l_valid_session == true) {
               session_mgr__client_create_req(io_dispatch_mgr__session,
                  io_dispatch_mgr__channel,
                  io_dispatch_mgr__l_req_handle,
                  io_dispatch_mgr__l_req_msg,
                  &io_dispatch_mgr__l_ret);
               if (io_dispatch_mgr__l_ret == constants__e_sc_ok) {
                  message_out_bs__write_msg_out_header_req_handle(io_dispatch_mgr__l_req_msg,
                     io_dispatch_mgr__l_req_handle);
                  channel_mgr_bs__send_channel_msg(io_dispatch_mgr__channel,
                     io_dispatch_mgr__l_req_msg,
                     &io_dispatch_mgr__l_ret);
                  if (io_dispatch_mgr__l_ret != constants__e_sc_ok) {
                     request_handle_bs__remove_req_handle(io_dispatch_mgr__l_req_handle);
                     session_mgr__client_close_session(io_dispatch_mgr__session);
                  }
               }
               else {
                  request_handle_bs__remove_req_handle(io_dispatch_mgr__l_req_handle);
                  session_mgr__client_close_session(io_dispatch_mgr__session);
               }
            }
            else {
               session_mgr__client_close_session(io_dispatch_mgr__session);
            }
         }
         message_out_bs__dealloc_msg_out(io_dispatch_mgr__l_req_msg);
      }
   }
}

void io_dispatch_mgr__local_sc_activate_orphaned_sessions(
   const constants__t_channel_config_idx_i io_dispatch_mgr__channel_config_idx,
   const constants__t_channel_i io_dispatch_mgr__channel) {
   {
      t_bool io_dispatch_mgr__l_valid_new_channel;
      constants__t_msg_i io_dispatch_mgr__l_req_msg;
      t_bool io_dispatch_mgr__l_valid_msg;
      t_bool io_dispatch_mgr__l_continue;
      constants__t_session_i io_dispatch_mgr__l_session;
      constants__t_StatusCode_i io_dispatch_mgr__l_ret;
      constants__t_request_handle_i io_dispatch_mgr__l_req_handle;
      t_bool io_dispatch_mgr__l_valid_req_handle;
      constants__t_session_token_i io_dispatch_mgr__l_session_token;
      
      channel_mgr_bs__is_connected_channel(io_dispatch_mgr__channel,
         &io_dispatch_mgr__l_valid_new_channel);
      if (io_dispatch_mgr__l_valid_new_channel == true) {
         session_mgr__init_iter_orphaned_t_session(io_dispatch_mgr__channel_config_idx,
            &io_dispatch_mgr__l_continue);
         while (io_dispatch_mgr__l_continue == true) {
            session_mgr__continue_iter_orphaned_t_session(&io_dispatch_mgr__l_session,
               &io_dispatch_mgr__l_continue);
            message_out_bs__alloc_req_msg(constants__e_msg_session_activate_req,
               &io_dispatch_mgr__l_req_msg);
            message_out_bs__is_valid_msg_out(io_dispatch_mgr__l_req_msg,
               &io_dispatch_mgr__l_valid_msg);
            if (io_dispatch_mgr__l_valid_msg == true) {
               request_handle_bs__fresh_req_handle(&io_dispatch_mgr__l_req_handle);
               request_handle_bs__is_valid_req_handle(io_dispatch_mgr__l_req_handle,
                  &io_dispatch_mgr__l_valid_req_handle);
               if (io_dispatch_mgr__l_valid_req_handle == true) {
                  session_mgr__client_sc_activate_req(io_dispatch_mgr__l_session,
                     io_dispatch_mgr__l_req_handle,
                     io_dispatch_mgr__channel,
                     io_dispatch_mgr__l_req_msg,
                     &io_dispatch_mgr__l_ret,
                     &io_dispatch_mgr__l_session_token);
                  if (io_dispatch_mgr__l_ret == constants__e_sc_ok) {
                     message_out_bs__write_msg_out_header_req_handle(io_dispatch_mgr__l_req_msg,
                        io_dispatch_mgr__l_req_handle);
                     message_out_bs__write_msg_out_header_session_token(io_dispatch_mgr__l_req_msg,
                        io_dispatch_mgr__l_session_token);
                     channel_mgr_bs__send_channel_msg(io_dispatch_mgr__channel,
                        io_dispatch_mgr__l_req_msg,
                        &io_dispatch_mgr__l_ret);
                     if (io_dispatch_mgr__l_ret != constants__e_sc_ok) {
                        request_handle_bs__remove_req_handle(io_dispatch_mgr__l_req_handle);
                        session_mgr__client_close_session(io_dispatch_mgr__l_session);
                     }
                  }
               }
               message_out_bs__dealloc_msg_out(io_dispatch_mgr__l_req_msg);
            }
         }
      }
   }
}

void io_dispatch_mgr__local_activate_session(
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
      
      message_out_bs__alloc_req_msg(constants__e_msg_session_activate_req,
         &io_dispatch_mgr__l_req_msg);
      message_out_bs__is_valid_msg_out(io_dispatch_mgr__l_req_msg,
         &io_dispatch_mgr__l_valid_msg);
      if (io_dispatch_mgr__l_valid_msg == true) {
         request_handle_bs__fresh_req_handle(&io_dispatch_mgr__l_req_handle);
         request_handle_bs__is_valid_req_handle(io_dispatch_mgr__l_req_handle,
            &io_dispatch_mgr__l_valid_req_handle);
         if (io_dispatch_mgr__l_valid_req_handle == true) {
            session_mgr__client_user_activate_req(io_dispatch_mgr__session,
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
                  session_mgr__client_close_session(io_dispatch_mgr__session);
               }
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
      constants__t_sessionState io_dispatch_mgr__l_session_state;
      constants__t_user_i io_dispatch_mgr__l_session_user;
      t_bool io_dispatch_mgr__l_valid_msg;
      constants__t_msg_type io_dispatch_mgr__l_msg_type;
      constants__t_ByteString_i io_dispatch_mgr__l_payload;
      t_bool io_dispatch_mgr__l_connected_channel;
      t_bool io_dispatch_mgr__l_is_client;
      t_bool io_dispatch_mgr__l_is_req;
      t_bool io_dispatch_mgr__l_is_valid_req;
      t_bool io_dispatch_mgr__l_snd_session_err;
      t_bool io_dispatch_mgr__l_is_valid_resp;
      constants__t_StatusCode_i io_dispatch_mgr__l_status;
      constants__t_msg_type io_dispatch_mgr__l_resp_msg_typ;
      constants__t_msg_i io_dispatch_mgr__l_resp_msg;
      constants__t_StatusCode_i io_dispatch_mgr__l_ret;
      
      channel_mgr_bs__is_connected_channel(io_dispatch_mgr__channel,
         &io_dispatch_mgr__l_connected_channel);
      message_in_bs__is_valid_msg_in(io_dispatch_mgr__msg,
         &io_dispatch_mgr__l_valid_msg);
      if ((io_dispatch_mgr__l_connected_channel == true) &&
         (io_dispatch_mgr__l_valid_msg == true)) {
         message_in_bs__get_msg_in_type(io_dispatch_mgr__msg,
            &io_dispatch_mgr__l_msg_type);
         switch (io_dispatch_mgr__l_msg_type) {
         case constants__e_msg_public_service_req:
         case constants__e_msg_public_service_resp:
            ;
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
                  session_mgr__client_receive_session_resp(io_dispatch_mgr__channel,
                     io_dispatch_mgr__l_request_handle,
                     io_dispatch_mgr__msg,
                     io_dispatch_mgr__l_msg_type,
                     &io_dispatch_mgr__l_session);
                  request_handle_bs__remove_req_handle(io_dispatch_mgr__l_request_handle);
                  session_mgr__get_session_state_or_closed(io_dispatch_mgr__l_session,
                     &io_dispatch_mgr__l_session_state);
                  if (io_dispatch_mgr__l_session_state == constants__e_session_created) {
                     session_async_bs__is_session_to_activate(io_dispatch_mgr__l_session,
                        &io_dispatch_mgr__l_session_user);
                     if (io_dispatch_mgr__l_session_user != constants__c_user_indet) {
                        io_dispatch_mgr__local_activate_session(io_dispatch_mgr__l_session,
                           io_dispatch_mgr__l_session_user,
                           &io_dispatch_mgr__l_ret);
                     }
                  }
                  message_in_bs__dealloc_msg_in(io_dispatch_mgr__msg);
                  break;
               case constants__e_msg_session_read_resp:
               case constants__e_msg_session_write_resp:
                  message_in_bs__read_msg_header_req_handle(io_dispatch_mgr__msg,
                     &io_dispatch_mgr__l_request_handle);
                  session_mgr__client_validate_session_service_resp(io_dispatch_mgr__channel,
                     io_dispatch_mgr__l_request_handle,
                     io_dispatch_mgr__msg,
                     &io_dispatch_mgr__l_is_valid_resp);
                  if (io_dispatch_mgr__l_is_valid_resp == true) {
                     message_in_bs__read_msg_resp_header_service_status(io_dispatch_mgr__msg,
                        &io_dispatch_mgr__l_status);
                     service_response_cli_cb_bs__cli_service_response(io_dispatch_mgr__msg,
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
                  message_out_bs__alloc_resp_msg(io_dispatch_mgr__l_resp_msg_typ,
                     io_dispatch_mgr__msg,
                     &io_dispatch_mgr__l_resp_msg);
                  message_out_bs__is_valid_msg_out(io_dispatch_mgr__l_resp_msg,
                     &io_dispatch_mgr__l_valid_msg);
                  if (io_dispatch_mgr__l_valid_msg == true) {
                     session_mgr__server_receive_session_req(io_dispatch_mgr__channel,
                        io_dispatch_mgr__l_request_handle,
                        io_dispatch_mgr__l_session_token,
                        io_dispatch_mgr__msg,
                        io_dispatch_mgr__l_msg_type,
                        io_dispatch_mgr__l_resp_msg,
                        &io_dispatch_mgr__l_session,
                        &io_dispatch_mgr__l_ret);
                     message_out_bs__write_msg_resp_header_service_status(io_dispatch_mgr__l_resp_msg,
                        io_dispatch_mgr__l_ret);
                     message_out_bs__write_msg_out_header_req_handle(io_dispatch_mgr__l_resp_msg,
                        io_dispatch_mgr__l_request_handle);
                     channel_mgr_bs__send_channel_msg(io_dispatch_mgr__channel,
                        io_dispatch_mgr__l_resp_msg,
                        &io_dispatch_mgr__l_ret);
                     if (io_dispatch_mgr__l_ret != constants__e_sc_ok) {
                        session_mgr__server_close_session(io_dispatch_mgr__l_session);
                     }
                     message_out_bs__dealloc_msg_out(io_dispatch_mgr__l_resp_msg);
                  }
                  break;
               case constants__e_msg_session_read_req:
               case constants__e_msg_session_write_req:
                  message_in_bs__read_msg_header_req_handle(io_dispatch_mgr__msg,
                     &io_dispatch_mgr__l_request_handle);
                  message_in_bs__read_msg_req_header_session_token(io_dispatch_mgr__msg,
                     &io_dispatch_mgr__l_session_token);
                  session_mgr__server_validate_session_service_req(io_dispatch_mgr__channel,
                     io_dispatch_mgr__l_request_handle,
                     io_dispatch_mgr__l_session_token,
                     io_dispatch_mgr__msg,
                     &io_dispatch_mgr__l_is_valid_req,
                     &io_dispatch_mgr__l_session,
                     &io_dispatch_mgr__l_snd_session_err);
                  if (io_dispatch_mgr__l_is_valid_req == true) {
                     io_dispatch_mgr__get_response_type(io_dispatch_mgr__l_msg_type,
                        &io_dispatch_mgr__l_resp_msg_typ);
                     message_out_bs__alloc_resp_msg(io_dispatch_mgr__l_resp_msg_typ,
                        io_dispatch_mgr__msg,
                        &io_dispatch_mgr__l_resp_msg);
                     message_out_bs__is_valid_msg_out(io_dispatch_mgr__l_resp_msg,
                        &io_dispatch_mgr__l_valid_msg);
                     if (io_dispatch_mgr__l_valid_msg == true) {
                        switch (io_dispatch_mgr__l_msg_type) {
                        case constants__e_msg_session_read_req:
                           io_dispatch_mgr__treat_read_request(io_dispatch_mgr__msg,
                              io_dispatch_mgr__l_resp_msg);
                           message_out_bs__write_msg_resp_header_service_status(io_dispatch_mgr__l_resp_msg,
                              constants__e_sc_ok);
                           break;
                        case constants__e_msg_session_write_req:
                           session_mgr__get_session_user_or_indet(io_dispatch_mgr__l_session,
                              &io_dispatch_mgr__l_session_user);
                           message_in_bs__get_msg_payload(io_dispatch_mgr__msg,
                              &io_dispatch_mgr__l_payload);
                           io_dispatch_mgr__treat_write_request(io_dispatch_mgr__l_payload,
                              io_dispatch_mgr__l_session_user,
                              &io_dispatch_mgr__l_ret);
                           message_out_bs__write_msg_resp_header_service_status(io_dispatch_mgr__l_resp_msg,
                              io_dispatch_mgr__l_ret);
                           address_space__write_WriteResponse_msg_out(io_dispatch_mgr__l_resp_msg);
                           address_space__dealloc_write_request_responses();
                           break;
                        default:
                           break;
                        }
                        session_mgr__server_validate_session_service_resp(io_dispatch_mgr__channel,
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
                     io_dispatch_mgr__get_response_type(io_dispatch_mgr__l_msg_type,
                        &io_dispatch_mgr__l_resp_msg_typ);
                     message_out_bs__alloc_resp_msg(io_dispatch_mgr__l_resp_msg_typ,
                        io_dispatch_mgr__msg,
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
               message_in_bs__dealloc_msg_in(io_dispatch_mgr__msg);
            }
            else {
               ;
            }
            break;
         }
      }
   }
}

void io_dispatch_mgr__cli_channel_connected_event(
   const constants__t_channel_config_idx_i io_dispatch_mgr__channel_config_idx,
   const constants__t_channel_i io_dispatch_mgr__channel) {
   {
      constants__t_session_i io_dispatch_mgr__l_session_to_create;
      t_bool io_dispatch_mgr__l_bres;
      
      channel_mgr_bs__cli_set_connected_channel(io_dispatch_mgr__channel_config_idx,
         io_dispatch_mgr__channel,
         &io_dispatch_mgr__l_bres);
      if (io_dispatch_mgr__l_bres == true) {
         io_dispatch_mgr__local_sc_activate_orphaned_sessions(io_dispatch_mgr__channel_config_idx,
            io_dispatch_mgr__channel);
         session_async_bs__is_session_to_create(io_dispatch_mgr__channel_config_idx,
            &io_dispatch_mgr__l_session_to_create);
         if (io_dispatch_mgr__l_session_to_create != constants__c_session_indet) {
            io_dispatch_mgr__local_create_session(io_dispatch_mgr__l_session_to_create,
               io_dispatch_mgr__channel);
         }
      }
   }
}

void io_dispatch_mgr__cli_secure_channel_timeout(
   const constants__t_channel_config_idx_i io_dispatch_mgr__channel_config_idx) {
   {
      t_bool io_dispatch_mgr__l_bres;
      
      channel_mgr_bs__cli_set_connection_timeout_channel(io_dispatch_mgr__channel_config_idx,
         &io_dispatch_mgr__l_bres);
   }
}

void io_dispatch_mgr__srv_channel_connected_event(
   const constants__t_endpoint_config_idx_i io_dispatch_mgr__endpoint_config_idx,
   const constants__t_channel_config_idx_i io_dispatch_mgr__channel_config_idx,
   const constants__t_channel_i io_dispatch_mgr__channel) {
   {
      t_bool io_dispatch_mgr__l_bres;
      
      channel_mgr_bs__srv_new_secure_channel(io_dispatch_mgr__endpoint_config_idx,
         io_dispatch_mgr__channel_config_idx,
         io_dispatch_mgr__channel,
         &io_dispatch_mgr__l_bres);
   }
}

void io_dispatch_mgr__activate_new_session(
   const constants__t_channel_config_idx_i io_dispatch_mgr__channel_config_idx,
   const constants__t_user_i io_dispatch_mgr__user,
   t_bool * const io_dispatch_mgr__bres) {
   {
      constants__t_channel_i io_dispatch_mgr__l_channel;
      t_bool io_dispatch_mgr__l_channel_connected;
      constants__t_session_i io_dispatch_mgr__l_session;
      t_bool io_dispatch_mgr__l_valid_session;
      t_bool io_dispatch_mgr__l_connected_channel;
      t_bool io_dispatch_mgr__l_bret;
      
      *io_dispatch_mgr__bres = false;
      channel_mgr_bs__get_connected_channel(io_dispatch_mgr__channel_config_idx,
         &io_dispatch_mgr__l_channel);
      channel_mgr_bs__is_connected_channel(io_dispatch_mgr__l_channel,
         &io_dispatch_mgr__l_connected_channel);
      if (io_dispatch_mgr__l_connected_channel == false) {
         channel_mgr_bs__cli_open_secure_channel(io_dispatch_mgr__channel_config_idx,
            &io_dispatch_mgr__l_bret);
         io_dispatch_mgr__l_channel_connected = false;
      }
      else {
         io_dispatch_mgr__l_bret = true;
         io_dispatch_mgr__l_channel_connected = true;
      }
      if (io_dispatch_mgr__l_bret == true) {
         session_mgr__client_init_session(&io_dispatch_mgr__l_session);
         session_mgr__is_valid_session(io_dispatch_mgr__l_session,
            &io_dispatch_mgr__l_valid_session);
         if (io_dispatch_mgr__l_valid_session == true) {
            if (io_dispatch_mgr__l_channel_connected == false) {
               session_async_bs__add_session_to_create(io_dispatch_mgr__l_session,
                  io_dispatch_mgr__channel_config_idx,
                  &io_dispatch_mgr__l_bret);
            }
            else {
               io_dispatch_mgr__local_create_session(io_dispatch_mgr__l_session,
                  io_dispatch_mgr__l_channel);
               io_dispatch_mgr__l_bret = true;
            }
            if (io_dispatch_mgr__l_bret == true) {
               session_async_bs__add_session_to_activate(io_dispatch_mgr__l_session,
                  io_dispatch_mgr__user,
                  &io_dispatch_mgr__l_bret);
            }
            if (io_dispatch_mgr__l_bret == false) {
               session_async_bs__is_session_to_create(io_dispatch_mgr__channel_config_idx,
                  &io_dispatch_mgr__l_session);
               session_mgr__delete_session(io_dispatch_mgr__l_session);
            }
         }
      }
      *io_dispatch_mgr__bres = io_dispatch_mgr__l_bret;
   }
}

void io_dispatch_mgr__activate_session(
   const constants__t_session_i io_dispatch_mgr__session,
   const constants__t_user_i io_dispatch_mgr__user,
   constants__t_StatusCode_i * const io_dispatch_mgr__ret) {
   io_dispatch_mgr__local_activate_session(io_dispatch_mgr__session,
      io_dispatch_mgr__user,
      io_dispatch_mgr__ret);
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
      
      message_out_bs__alloc_req_msg(constants__e_msg_session_close_req,
         &io_dispatch_mgr__l_req_msg);
      message_out_bs__is_valid_msg_out(io_dispatch_mgr__l_req_msg,
         &io_dispatch_mgr__l_valid_msg);
      if (io_dispatch_mgr__l_valid_msg == true) {
         request_handle_bs__fresh_req_handle(&io_dispatch_mgr__l_req_handle);
         request_handle_bs__is_valid_req_handle(io_dispatch_mgr__l_req_handle,
            &io_dispatch_mgr__l_valid_req_handle);
         if (io_dispatch_mgr__l_valid_req_handle == true) {
            session_mgr__client_close_req(io_dispatch_mgr__session,
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
                  session_mgr__client_close_session(io_dispatch_mgr__session);
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
      t_bool io_dispatch_mgr__l_connected_channel;
      t_bool io_dispatch_mgr__l_disconnecting_channel;
      t_bool io_dispatch_mgr__l_valid_new_channel;
      t_bool io_dispatch_mgr__l_is_client;
      constants__t_channel_config_idx_i io_dispatch_mgr__l_channel_config_idx;
      constants__t_channel_i io_dispatch_mgr__l_new_channel;
      t_bool io_dispatch_mgr__l_bres;
      
      channel_mgr_bs__is_connected_channel(io_dispatch_mgr__channel,
         &io_dispatch_mgr__l_connected_channel);
      if (io_dispatch_mgr__l_connected_channel == true) {
         channel_mgr_bs__is_client_channel(io_dispatch_mgr__channel,
            &io_dispatch_mgr__l_is_client);
         if (io_dispatch_mgr__l_is_client == true) {
            channel_mgr_bs__get_channel_info(io_dispatch_mgr__channel,
               &io_dispatch_mgr__l_channel_config_idx);
            channel_mgr_bs__is_disconnecting_channel(io_dispatch_mgr__l_channel_config_idx,
               &io_dispatch_mgr__l_disconnecting_channel);
            session_mgr__client_secure_channel_lost(io_dispatch_mgr__channel,
               io_dispatch_mgr__l_channel_config_idx);
            if (io_dispatch_mgr__l_disconnecting_channel == false) {
               channel_mgr_bs__get_connected_channel(io_dispatch_mgr__l_channel_config_idx,
                  &io_dispatch_mgr__l_new_channel);
               if (io_dispatch_mgr__l_new_channel == constants__c_channel_indet) {
                  channel_mgr_bs__cli_open_secure_channel(io_dispatch_mgr__l_channel_config_idx,
                     &io_dispatch_mgr__l_bres);
               }
               channel_mgr_bs__is_connected_channel(io_dispatch_mgr__l_new_channel,
                  &io_dispatch_mgr__l_valid_new_channel);
               if (io_dispatch_mgr__l_valid_new_channel == true) {
                  io_dispatch_mgr__local_sc_activate_orphaned_sessions(io_dispatch_mgr__l_channel_config_idx,
                     io_dispatch_mgr__l_new_channel);
               }
            }
         }
         else {
            session_mgr__server_secure_channel_lost(io_dispatch_mgr__channel);
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
      case constants__e_msg_session_write_req:
         message_out_bs__bless_msg_out(io_dispatch_mgr__req_msg,
            io_dispatch_mgr__l_msg_typ);
         message_out_bs__is_valid_msg_out(io_dispatch_mgr__req_msg,
            &io_dispatch_mgr__l_valid_msg);
         if (io_dispatch_mgr__l_valid_msg == true) {
            request_handle_bs__fresh_req_handle(&io_dispatch_mgr__l_req_handle);
            request_handle_bs__is_valid_req_handle(io_dispatch_mgr__l_req_handle,
               &io_dispatch_mgr__l_valid_req_handle);
            if (io_dispatch_mgr__l_valid_req_handle == true) {
               session_mgr__client_validate_session_service_req(io_dispatch_mgr__session,
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

void io_dispatch_mgr__close_all_active_connections(
   t_bool * const io_dispatch_mgr__bres) {
   channel_mgr_bs__close_all_channel(io_dispatch_mgr__bres);
}

