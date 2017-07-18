/******************************************************************************

 File Name            : message_in_bs.h

 Date                 : 25/07/2017 17:18:01

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
extern void message_in_bs__get_msg_in_type(
   const constants__t_msg_i message_in_bs__msg,
   constants__t_msg_type * const message_in_bs__msgtype);
extern void message_in_bs__get_msg_payload(
   const constants__t_msg_i message_in_bs__msg,
   constants__t_ByteString_i * const message_in_bs__payload);
extern void message_in_bs__is_valid_msg_in(
   const constants__t_msg_i message_in_bs__msg,
   t_bool * const message_in_bs__bres);
extern void message_in_bs__msg_in_memory_changed(void);
extern void message_in_bs__read_activate_msg_user(
   const constants__t_msg_i message_in_bs__msg,
   constants__t_user_i * const message_in_bs__user);
extern void message_in_bs__read_create_session_msg_session_token(
   const constants__t_msg_i message_in_bs__msg,
   constants__t_session_token_i * const message_in_bs__session_token);
extern void message_in_bs__read_msg_header_req_handle(
   const constants__t_msg_i message_in_bs__msg,
   constants__t_request_handle_i * const message_in_bs__handle);
extern void message_in_bs__read_msg_req_header_session_token(
   const constants__t_msg_i message_in_bs__msg,
   constants__t_session_token_i * const message_in_bs__session_token);
extern void message_in_bs__read_msg_resp_header_service_status(
   const constants__t_msg_i message_in_bs__msg,
   constants__t_StatusCode_i * const message_in_bs__status);

#endif
