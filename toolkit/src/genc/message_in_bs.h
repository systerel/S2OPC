/******************************************************************************

 File Name            : message_in_bs.h

 Date                 : 28/09/2017 17:52:26

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _message_in_bs_h
#define _message_in_bs_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void message_in_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void message_in_bs__dealloc_msg_in(
   const constants__t_msg_i message_in_bs__msg);
extern void message_in_bs__dealloc_msg_in_buffer(
   const constants__t_byte_buffer_i message_in_bs__msg_buffer);
extern void message_in_bs__dealloc_msg_in_header(
   const constants__t_msg_header_i message_in_bs__msg_header);
extern void message_in_bs__decode_msg(
   const constants__t_msg_type_i message_in_bs__msg_type,
   const constants__t_byte_buffer_i message_in_bs__msg_buffer,
   constants__t_msg_i * const message_in_bs__msg);
extern void message_in_bs__decode_msg_header(
   const t_bool message_in_bs__is_request,
   const constants__t_byte_buffer_i message_in_bs__msg_buffer,
   constants__t_msg_header_i * const message_in_bs__msg_header);
extern void message_in_bs__decode_msg_type(
   const constants__t_byte_buffer_i message_in_bs__msg_buffer,
   constants__t_msg_type_i * const message_in_bs__msg_typ);
extern void message_in_bs__get_msg_in_type(
   const constants__t_msg_i message_in_bs__req_msg,
   constants__t_msg_type_i * const message_in_bs__msgtype);
extern void message_in_bs__is_valid_msg_in(
   const constants__t_msg_i message_in_bs__msg,
   t_bool * const message_in_bs__bres);
extern void message_in_bs__is_valid_msg_in_header(
   const constants__t_msg_header_i message_in_bs__msg_header,
   t_bool * const message_in_bs__bres);
extern void message_in_bs__is_valid_msg_in_type(
   const constants__t_msg_type_i message_in_bs__msg_typ,
   t_bool * const message_in_bs__bres);
extern void message_in_bs__is_valid_request_context(
   const constants__t_request_context_i message_in_bs__req_context,
   t_bool * const message_in_bs__bres);
extern void message_in_bs__read_activate_msg_user(
   const constants__t_msg_i message_in_bs__msg,
   constants__t_user_i * const message_in_bs__user);
extern void message_in_bs__read_create_session_msg_session_token(
   const constants__t_msg_i message_in_bs__msg,
   constants__t_session_token_i * const message_in_bs__session_token);
extern void message_in_bs__read_msg_header_req_handle(
   const constants__t_msg_header_i message_in_bs__msg_header,
   constants__t_request_handle_i * const message_in_bs__handle);
extern void message_in_bs__read_msg_req_header_session_token(
   const constants__t_msg_header_i message_in_bs__msg_header,
   constants__t_session_token_i * const message_in_bs__session_token);
extern void message_in_bs__read_msg_resp_header_service_status(
   const constants__t_msg_header_i message_in_bs__msg_header,
   constants__t_StatusCode_i * const message_in_bs__status);

#endif
