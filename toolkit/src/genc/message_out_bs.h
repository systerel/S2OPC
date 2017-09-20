/******************************************************************************

 File Name            : message_out_bs.h

 Date                 : 20/09/2017 11:36:54

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _message_out_bs_h
#define _message_out_bs_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"
#include "message_in_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void message_out_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void message_out_bs__alloc_app_req_msg_header(
   const constants__t_msg_type_i message_out_bs__msg_type,
   constants__t_msg_header_i * const message_out_bs__nmsg_header);
extern void message_out_bs__alloc_req_msg(
   const constants__t_msg_type_i message_out_bs__msg_type,
   constants__t_msg_header_i * const message_out_bs__nmsg_header,
   constants__t_msg_i * const message_out_bs__nmsg);
extern void message_out_bs__alloc_resp_msg(
   const constants__t_msg_type_i message_out_bs__msg_type,
   constants__t_msg_header_i * const message_out_bs__nmsg_header,
   constants__t_msg_i * const message_out_bs__nmsg);
extern void message_out_bs__bless_msg_out(
   const constants__t_msg_type_i message_out_bs__msg_type,
   const constants__t_msg_header_i message_out_bs__msg_header,
   const constants__t_msg_i message_out_bs__msg);
extern void message_out_bs__dealloc_msg_header_out(
   const constants__t_msg_header_i message_out_bs__msg_header);
extern void message_out_bs__dealloc_msg_out(
   const constants__t_msg_i message_out_bs__msg);
extern void message_out_bs__encode_msg(
   const constants__t_msg_type_i message_out_bs__msg_type,
   const constants__t_msg_header_i message_out_bs__msg_header,
   const constants__t_msg_i message_out_bs__msg,
   constants__t_byte_buffer_i * const message_out_bs__buffer);
extern void message_out_bs__get_msg_out_type(
   const constants__t_msg_i message_out_bs__msg,
   constants__t_msg_type_i * const message_out_bs__msgtype);
extern void message_out_bs__is_valid_buffer_out(
   const constants__t_byte_buffer_i message_out_bs__buffer,
   t_bool * const message_out_bs__bres);
extern void message_out_bs__is_valid_msg_out(
   const constants__t_msg_i message_out_bs__msg,
   t_bool * const message_out_bs__bres);
extern void message_out_bs__is_valid_msg_out_header(
   const constants__t_msg_header_i message_out_bs__msg_header,
   t_bool * const message_out_bs__bres);
extern void message_out_bs__write_activate_msg_user(
   const constants__t_msg_i message_out_bs__msg,
   const constants__t_user_i message_out_bs__user);
extern void message_out_bs__write_create_session_msg_crypto(
   const constants__t_msg_i message_out_bs__p_msg,
   const constants__t_Nonce_i message_out_bs__p_nonce,
   const constants__t_SignatureData_i message_out_bs__p_signature,
   constants__t_StatusCode_i * const message_out_bs__sc);
extern void message_out_bs__write_create_session_msg_server_endpoints(
   const constants__t_msg_i message_out_bs__req_msg,
   const constants__t_msg_i message_out_bs__resp_msg,
   const constants__t_endpoint_config_idx_i message_out_bs__endpoint_config_idx,
   constants__t_StatusCode_i * const message_out_bs__ret);
extern void message_out_bs__write_create_session_msg_session_token(
   const constants__t_msg_i message_out_bs__msg,
   const constants__t_session_token_i message_out_bs__session_token);
extern void message_out_bs__write_create_session_req_msg_crypto(
   const constants__t_msg_i message_out_bs__p_req_msg,
   const constants__t_channel_config_idx_i message_out_bs__p_channel_config_idx,
   const constants__t_Nonce_i message_out_bs__p_nonce);
extern void message_out_bs__write_create_session_req_msg_endpointUrl(
   const constants__t_msg_i message_out_bs__msg,
   const constants__t_channel_config_idx_i message_out_bs__channel_config_idx);
extern void message_out_bs__write_msg_out_header_req_handle(
   const constants__t_msg_header_i message_out_bs__msg_header,
   const constants__t_request_handle_i message_out_bs__req_handle);
extern void message_out_bs__write_msg_out_header_session_token(
   const constants__t_msg_header_i message_out_bs__msg_header,
   const constants__t_session_token_i message_out_bs__session_token);
extern void message_out_bs__write_msg_resp_header_service_status(
   const constants__t_msg_header_i message_out_bs__msg_header,
   const constants__t_StatusCode_i message_out_bs__status_code);

#endif
