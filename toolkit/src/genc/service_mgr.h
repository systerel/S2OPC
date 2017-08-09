/******************************************************************************

 File Name            : service_mgr.h

 Date                 : 09/08/2017 10:37:54

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _service_mgr_h
#define _service_mgr_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*-----------------
   IMPORTS Clause
  -----------------*/
#include "address_space.h"
#include "message_in_bs.h"
#include "message_out_bs.h"
#include "request_handle_bs.h"
#include "service_read.h"
#include "service_response_cli_cb_bs.h"
#include "service_write_decode_bs.h"
#include "session_mgr.h"

/*--------------
   SEES Clause
  --------------*/
#include "channel_mgr_bs.h"
#include "constants.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void service_mgr__INITIALISATION(void);

/*-------------------------------
   PROMOTES and EXTENDS Clauses
  -------------------------------*/
#define service_mgr__client_async_activate_new_session_with_channel session_mgr__client_async_activate_new_session_with_channel
#define service_mgr__client_async_activate_new_session_without_channel session_mgr__client_async_activate_new_session_without_channel
#define service_mgr__client_channel_connected_event_session session_mgr__client_channel_connected_event_session
#define service_mgr__client_secure_channel_lost session_mgr__client_secure_channel_lost
#define service_mgr__decode_msg_type message_in_bs__decode_msg_type
#define service_mgr__is_valid_msg_in_type message_in_bs__is_valid_msg_in_type
#define service_mgr__server_secure_channel_lost session_mgr__server_secure_channel_lost

/*--------------------------
   LOCAL_OPERATIONS Clause
  --------------------------*/
extern void service_mgr__get_response_type(
   const constants__t_msg_type_i service_mgr__req_msg_typ,
   constants__t_msg_type_i * const service_mgr__resp_msg_typ);
extern void service_mgr__treat_read_request(
   const constants__t_msg_i service_mgr__p_request_msg,
   const constants__t_msg_i service_mgr__p_response_msg);
extern void service_mgr__treat_write_request(
   const constants__t_msg_i service_mgr__write_msg,
   const constants__t_user_i service_mgr__userid,
   constants__t_StatusCode_i * const service_mgr__StatusCode_service);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void service_mgr__client_activate_orphaned_session(
   const constants__t_session_i service_mgr__session,
   const constants__t_channel_i service_mgr__channel);
extern void service_mgr__client_activate_session(
   const constants__t_session_i service_mgr__session,
   const constants__t_user_i service_mgr__user,
   constants__t_StatusCode_i * const service_mgr__ret);
extern void service_mgr__client_close_session(
   const constants__t_session_i service_mgr__session,
   constants__t_StatusCode_i * const service_mgr__ret);
extern void service_mgr__client_create_session(
   const constants__t_session_i service_mgr__session,
   const constants__t_channel_i service_mgr__channel);
extern void service_mgr__client_receive_public_service_resp(
   const constants__t_channel_i service_mgr__channel,
   const constants__t_msg_type_i service_mgr__resp_typ,
   const constants__t_byte_buffer_i service_mgr__msg_buffer);
extern void service_mgr__client_receive_session_service_resp(
   const constants__t_channel_i service_mgr__channel,
   const constants__t_msg_type_i service_mgr__resp_typ,
   const constants__t_byte_buffer_i service_mgr__msg_buffer);
extern void service_mgr__client_receive_session_treatment_resp(
   const constants__t_channel_i service_mgr__channel,
   const constants__t_msg_type_i service_mgr__resp_typ,
   const constants__t_byte_buffer_i service_mgr__msg_buffer);
extern void service_mgr__client_send_service_request_msg(
   const constants__t_session_i service_mgr__session,
   const constants__t_msg_i service_mgr__req_msg,
   constants__t_StatusCode_i * const service_mgr__ret);
extern void service_mgr__server_receive_public_service_req(
   const constants__t_channel_i service_mgr__channel,
   const constants__t_msg_type_i service_mgr__req_typ,
   const constants__t_byte_buffer_i service_mgr__msg_buffer);
extern void service_mgr__server_receive_session_service_req(
   const constants__t_channel_i service_mgr__channel,
   const constants__t_msg_type_i service_mgr__req_typ,
   const constants__t_byte_buffer_i service_mgr__msg_buffer);
extern void service_mgr__server_receive_session_treatment_req(
   const constants__t_channel_i service_mgr__channel,
   const constants__t_msg_type_i service_mgr__req_typ,
   const constants__t_byte_buffer_i service_mgr__msg_buffer);

#endif
