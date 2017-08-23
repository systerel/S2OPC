/******************************************************************************

 File Name            : service_mgr.c

 Date                 : 23/08/2017 17:27:30

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
   case constants__e_msg_get_endpoints_service_req:
      *service_mgr__resp_msg_typ = constants__e_msg_get_endpoints_service_resp;
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

void service_mgr__server_receive_session_treatment_req(
   const constants__t_channel_i service_mgr__channel,
   const constants__t_msg_type_i service_mgr__req_typ,
   const constants__t_byte_buffer_i service_mgr__msg_buffer,
   constants__t_byte_buffer_i * const service_mgr__buffer_out) {
   {
      constants__t_msg_header_i service_mgr__l_req_msg_header;
      t_bool service_mgr__l_valid_req_header;
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
      
      service_mgr__l_buffer_out = constants__c_byte_buffer_indet;
      message_in_bs__decode_msg_header(true,
         service_mgr__msg_buffer,
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
         if (service_mgr__l_valid_req_handle == true) {
            message_in_bs__decode_msg(service_mgr__req_typ,
               service_mgr__msg_buffer,
               &service_mgr__l_req_msg);
            message_in_bs__is_valid_msg_in(service_mgr__l_req_msg,
               &service_mgr__l_valid_req);
            if (service_mgr__l_valid_req == true) {
               service_mgr__get_response_type(service_mgr__req_typ,
                  &service_mgr__l_resp_msg_typ);
               message_out_bs__alloc_resp_msg(service_mgr__l_resp_msg_typ,
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
                  if (service_mgr__l_ret != constants__e_sc_ok) {
                     service_mgr__l_resp_msg_typ = constants__e_msg_service_fault_resp;
                  }
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
                  if (service_mgr__l_valid_buffer == false) {
                     session_mgr__server_close_session_sm(service_mgr__l_session);
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
      *service_mgr__buffer_out = service_mgr__l_buffer_out;
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
      
      message_in_bs__decode_msg_header(false,
         service_mgr__msg_buffer,
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
            message_in_bs__decode_msg(service_mgr__resp_typ,
               service_mgr__msg_buffer,
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
   const constants__t_byte_buffer_i service_mgr__msg_buffer,
   constants__t_byte_buffer_i * const service_mgr__buffer_out) {
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
      
      service_mgr__l_buffer_out = constants__c_byte_buffer_indet;
      message_in_bs__decode_msg_header(true,
         service_mgr__msg_buffer,
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
               message_in_bs__decode_msg(service_mgr__req_typ,
                  service_mgr__msg_buffer,
                  &service_mgr__l_req_msg);
               message_in_bs__is_valid_msg_in(service_mgr__l_req_msg,
                  &service_mgr__l_valid_req);
               if (service_mgr__l_valid_req == true) {
                  service_mgr__get_response_type(service_mgr__req_typ,
                     &service_mgr__l_resp_msg_typ);
                  message_out_bs__alloc_resp_msg(service_mgr__l_resp_msg_typ,
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
                        service_mgr__l_ret = constants__e_sc_ok;
                        break;
                     case constants__e_msg_session_write_req:
                        session_mgr__get_session_user_or_indet(service_mgr__l_session,
                           &service_mgr__l_session_user);
                        service_mgr__treat_write_request(service_mgr__l_req_msg,
                           service_mgr__l_session_user,
                           &service_mgr__l_ret);
                        address_space__write_WriteResponse_msg_out(service_mgr__l_resp_msg);
                        address_space__dealloc_write_request_responses();
                        break;
                     default:
                        service_mgr__l_ret = constants__e_sc_bad_unexpected_error;
                        break;
                     }
                     message_out_bs__write_msg_resp_header_service_status(service_mgr__l_resp_msg_header,
                        service_mgr__l_ret);
                     session_mgr__server_validate_session_service_resp(service_mgr__channel,
                        service_mgr__l_session,
                        service_mgr__l_request_handle,
                        &service_mgr__l_is_valid_resp,
                        &service_mgr__l_snd_session_err);
                     if (service_mgr__l_is_valid_resp == true) {
                        if (service_mgr__l_ret != constants__e_sc_ok) {
                           service_mgr__l_resp_msg_typ = constants__e_msg_service_fault_resp;
                        }
                        message_out_bs__write_msg_out_header_req_handle(service_mgr__l_resp_msg_header,
                           service_mgr__l_request_handle);
                        message_out_bs__encode_msg(service_mgr__l_resp_msg_typ,
                           service_mgr__l_resp_msg_header,
                           service_mgr__l_resp_msg,
                           &service_mgr__l_buffer_out);
                     }
                     message_out_bs__dealloc_msg_header_out(service_mgr__l_resp_msg_header);
                     message_out_bs__dealloc_msg_out(service_mgr__l_resp_msg);
                  }
                  message_in_bs__dealloc_msg_in(service_mgr__l_req_msg);
               }
            }
         }
      }
      if (service_mgr__l_valid_req_header == true) {
         message_in_bs__dealloc_msg_in_header(service_mgr__l_req_msg_header);
      }
      message_in_bs__dealloc_msg_in_buffer(service_mgr__msg_buffer);
      *service_mgr__buffer_out = service_mgr__l_buffer_out;
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
      
      message_in_bs__decode_msg_header(false,
         service_mgr__msg_buffer,
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
               message_in_bs__decode_msg(service_mgr__resp_typ,
                  service_mgr__msg_buffer,
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
         message_in_bs__dealloc_msg_in_header(service_mgr__l_resp_msg_header);
      }
      message_in_bs__dealloc_msg_in_buffer(service_mgr__msg_buffer);
   }
}

void service_mgr__server_receive_discovery_service_req(
   const constants__t_channel_i service_mgr__channel,
   const constants__t_msg_type_i service_mgr__req_typ,
   const constants__t_byte_buffer_i service_mgr__msg_buffer,
   constants__t_byte_buffer_i * const service_mgr__buffer_out) {
   {
      constants__t_msg_header_i service_mgr__l_req_msg_header;
      t_bool service_mgr__l_valid_req_header;
      constants__t_request_handle_i service_mgr__l_request_handle;
      t_bool service_mgr__l_valid_req_handle;
      constants__t_msg_i service_mgr__l_req_msg;
      t_bool service_mgr__l_valid_req;
      constants__t_msg_type_i service_mgr__l_resp_msg_typ;
      constants__t_msg_i service_mgr__l_resp_msg;
      t_bool service_mgr__l_valid_msg;
      constants__t_msg_header_i service_mgr__l_resp_msg_header;
      t_bool service_mgr__l_valid_resp_header;
      constants__t_StatusCode_i service_mgr__l_ret;
      constants__t_byte_buffer_i service_mgr__l_buffer_out;
      constants__t_endpoint_config_idx_i service_mgr__l_endpoint_config_idx;
      
      service_mgr__l_buffer_out = constants__c_byte_buffer_indet;
      message_in_bs__decode_msg_header(true,
         service_mgr__msg_buffer,
         &service_mgr__l_req_msg_header);
      message_in_bs__is_valid_msg_in_header(service_mgr__l_req_msg_header,
         &service_mgr__l_valid_req_header);
      if (service_mgr__l_valid_req_header == true) {
         message_in_bs__read_msg_header_req_handle(service_mgr__l_req_msg_header,
            &service_mgr__l_request_handle);
         request_handle_bs__is_valid_req_handle(service_mgr__l_request_handle,
            &service_mgr__l_valid_req_handle);
         if (service_mgr__l_valid_req_handle == true) {
            message_in_bs__decode_msg(service_mgr__req_typ,
               service_mgr__msg_buffer,
               &service_mgr__l_req_msg);
            message_in_bs__is_valid_msg_in(service_mgr__l_req_msg,
               &service_mgr__l_valid_req);
            if (service_mgr__l_valid_req == true) {
               service_mgr__get_response_type(service_mgr__req_typ,
                  &service_mgr__l_resp_msg_typ);
               message_out_bs__alloc_resp_msg(service_mgr__l_resp_msg_typ,
                  &service_mgr__l_resp_msg_header,
                  &service_mgr__l_resp_msg);
               message_out_bs__is_valid_msg_out(service_mgr__l_resp_msg,
                  &service_mgr__l_valid_msg);
               message_out_bs__is_valid_msg_out_header(service_mgr__l_resp_msg_header,
                  &service_mgr__l_valid_resp_header);
               if ((service_mgr__l_valid_msg == true) &&
                  (service_mgr__l_valid_resp_header == true)) {
                  switch (service_mgr__req_typ) {
                  case constants__e_msg_get_endpoints_service_req:
                     channel_mgr_bs__server_get_endpoint_config(service_mgr__channel,
                        &service_mgr__l_endpoint_config_idx);
                     service_get_endpoints_bs__treat_get_endpoints_request(service_mgr__l_req_msg,
                        service_mgr__l_resp_msg,
                        service_mgr__l_endpoint_config_idx,
                        &service_mgr__l_ret);
                     service_mgr__l_ret = constants__e_sc_ok;
                     break;
                  default:
                     service_mgr__l_ret = constants__e_sc_bad_unexpected_error;
                     break;
                  }
                  message_out_bs__write_msg_resp_header_service_status(service_mgr__l_resp_msg_header,
                     service_mgr__l_ret);
                  if (service_mgr__l_ret != constants__e_sc_ok) {
                     service_mgr__l_resp_msg_typ = constants__e_msg_service_fault_resp;
                  }
                  message_out_bs__write_msg_out_header_req_handle(service_mgr__l_resp_msg_header,
                     service_mgr__l_request_handle);
                  message_out_bs__encode_msg(service_mgr__l_resp_msg_typ,
                     service_mgr__l_resp_msg_header,
                     service_mgr__l_resp_msg,
                     &service_mgr__l_buffer_out);
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
      *service_mgr__buffer_out = service_mgr__l_buffer_out;
   }
}

void service_mgr__client_receive_discovery_service_resp(
   const constants__t_channel_i service_mgr__channel,
   const constants__t_msg_type_i service_mgr__resp_typ,
   const constants__t_byte_buffer_i service_mgr__msg_buffer) {
   {
      constants__t_msg_header_i service_mgr__l_resp_msg_header;
      t_bool service_mgr__l_valid_resp_header;
      constants__t_request_handle_i service_mgr__l_request_handle;
      t_bool service_mgr__l_validated_req_handle;
      constants__t_msg_i service_mgr__l_resp_msg;
      t_bool service_mgr__l_valid_resp_msg;
      constants__t_StatusCode_i service_mgr__l_status;
      
      message_in_bs__decode_msg_header(false,
         service_mgr__msg_buffer,
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
            message_in_bs__decode_msg(service_mgr__resp_typ,
               service_mgr__msg_buffer,
               &service_mgr__l_resp_msg);
            message_in_bs__is_valid_msg_in(service_mgr__l_resp_msg,
               &service_mgr__l_valid_resp_msg);
            if (service_mgr__l_valid_resp_msg == true) {
               message_in_bs__read_msg_resp_header_service_status(service_mgr__l_resp_msg_header,
                  &service_mgr__l_status);
               service_response_cli_cb_bs__cli_service_response(service_mgr__l_resp_msg,
                  service_mgr__l_status);
            }
            request_handle_bs__client_remove_req_handle(service_mgr__l_request_handle);
         }
         message_in_bs__dealloc_msg_in_header(service_mgr__l_resp_msg_header);
      }
      ;
      message_in_bs__dealloc_msg_in_buffer(service_mgr__msg_buffer);
   }
}

void service_mgr__client_service_create_session(
   const constants__t_session_i service_mgr__session,
   const constants__t_channel_i service_mgr__channel,
   constants__t_byte_buffer_i * const service_mgr__buffer_out) {
   {
      constants__t_msg_header_i service_mgr__l_msg_header;
      t_bool service_mgr__l_valid_msg_header;
      constants__t_msg_i service_mgr__l_req_msg;
      constants__t_request_handle_i service_mgr__l_req_handle;
      t_bool service_mgr__l_valid_req_handle;
      t_bool service_mgr__l_valid_msg;
      constants__t_byte_buffer_i service_mgr__l_buffer;
      t_bool service_mgr__l_valid_buffer;
      constants__t_StatusCode_i service_mgr__l_ret;
      
      service_mgr__l_buffer = constants__c_byte_buffer_indet;
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
            session_mgr__client_create_session_req(service_mgr__session,
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
               if (service_mgr__l_valid_buffer == false) {
                  request_handle_bs__client_remove_req_handle(service_mgr__l_req_handle);
                  session_mgr__client_close_session(service_mgr__session);
               }
            }
            else {
               request_handle_bs__client_remove_req_handle(service_mgr__l_req_handle);
               session_mgr__client_close_session(service_mgr__session);
            }
         }
         else {
            session_mgr__client_close_session(service_mgr__session);
         }
         message_out_bs__dealloc_msg_header_out(service_mgr__l_msg_header);
         message_out_bs__dealloc_msg_out(service_mgr__l_req_msg);
      }
      *service_mgr__buffer_out = service_mgr__l_buffer;
   }
}

void service_mgr__client_service_activate_orphaned_session(
   const constants__t_session_i service_mgr__session,
   const constants__t_channel_i service_mgr__channel,
   constants__t_byte_buffer_i * const service_mgr__buffer_out) {
   {
      constants__t_msg_header_i service_mgr__l_msg_header;
      constants__t_msg_i service_mgr__l_req_msg;
      t_bool service_mgr__l_valid_msg;
      t_bool service_mgr__l_valid_msg_header;
      constants__t_request_handle_i service_mgr__l_req_handle;
      t_bool service_mgr__l_valid_req_handle;
      constants__t_StatusCode_i service_mgr__l_ret;
      constants__t_session_token_i service_mgr__l_session_token;
      constants__t_byte_buffer_i service_mgr__l_buffer;
      t_bool service_mgr__l_valid_buffer;
      
      service_mgr__l_buffer = constants__c_byte_buffer_indet;
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
            session_mgr__client_sc_activate_session_req(service_mgr__session,
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
               if (service_mgr__l_valid_buffer == false) {
                  request_handle_bs__client_remove_req_handle(service_mgr__l_req_handle);
                  session_mgr__client_close_session(service_mgr__session);
               }
            }
         }
         message_out_bs__dealloc_msg_header_out(service_mgr__l_msg_header);
         message_out_bs__dealloc_msg_out(service_mgr__l_req_msg);
      }
      *service_mgr__buffer_out = service_mgr__l_buffer;
   }
}

void service_mgr__client_service_activate_session(
   const constants__t_session_i service_mgr__session,
   const constants__t_user_i service_mgr__user,
   constants__t_StatusCode_i * const service_mgr__ret,
   constants__t_channel_i * const service_mgr__channel,
   constants__t_byte_buffer_i * const service_mgr__buffer_out) {
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
      
      service_mgr__l_channel = constants__c_channel_indet;
      service_mgr__l_buffer = constants__c_byte_buffer_indet;
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
            session_mgr__client_user_activate_session_req(service_mgr__session,
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
               if (service_mgr__l_valid_buffer == false) {
                  request_handle_bs__client_remove_req_handle(service_mgr__l_req_handle);
                  service_mgr__l_ret = constants__e_sc_bad_encoding_error;
                  service_mgr__l_channel = constants__c_channel_indet;
                  session_mgr__client_close_session(service_mgr__session);
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
      *service_mgr__channel = service_mgr__l_channel;
      *service_mgr__buffer_out = service_mgr__l_buffer;
   }
}

void service_mgr__client_service_close_session(
   const constants__t_session_i service_mgr__session,
   constants__t_StatusCode_i * const service_mgr__ret,
   constants__t_channel_i * const service_mgr__channel,
   constants__t_byte_buffer_i * const service_mgr__buffer_out) {
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
      
      service_mgr__l_channel = constants__c_channel_indet;
      service_mgr__l_buffer = constants__c_byte_buffer_indet;
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
            session_mgr__client_close_session_req(service_mgr__session,
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
               if (service_mgr__l_valid_buffer == false) {
                  service_mgr__l_channel = constants__c_channel_indet;
                  *service_mgr__ret = constants__e_sc_bad_encoding_error;
                  request_handle_bs__client_remove_req_handle(service_mgr__l_req_handle);
                  session_mgr__client_close_session(service_mgr__session);
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
      *service_mgr__channel = service_mgr__l_channel;
      *service_mgr__buffer_out = service_mgr__l_buffer;
   }
}

void service_mgr__client_service_request(
   const constants__t_session_i service_mgr__session,
   const constants__t_msg_i service_mgr__req_msg,
   constants__t_StatusCode_i * const service_mgr__ret,
   constants__t_channel_i * const service_mgr__channel,
   constants__t_byte_buffer_i * const service_mgr__buffer_out) {
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
      
      service_mgr__l_channel = constants__c_channel_indet;
      service_mgr__l_buffer = constants__c_byte_buffer_indet;
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
                  if (service_mgr__l_valid_buffer == false) {
                     *service_mgr__ret = constants__e_sc_bad_encoding_error;
                     service_mgr__l_channel = constants__c_channel_indet;
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
      *service_mgr__channel = service_mgr__l_channel;
      *service_mgr__buffer_out = service_mgr__l_buffer;
   }
}

