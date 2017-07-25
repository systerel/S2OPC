/******************************************************************************

 File Name            : io_dispatch_mgr.h

 Date                 : 25/07/2017 17:25:08

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _io_dispatch_mgr_h
#define _io_dispatch_mgr_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*-----------------
   IMPORTS Clause
  -----------------*/
#include "address_space.h"
#include "channel_mgr_bs.h"
#include "message_in_bs.h"
#include "message_out_bs.h"
#include "request_handle_bs.h"
#include "service_read.h"
#include "service_response_cli_cb_bs.h"
#include "service_write_decode_bs.h"
#include "session_async_bs.h"
#include "session_mgr.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void io_dispatch_mgr__INITIALISATION(void);

/*-------------------------------
   PROMOTES and EXTENDS Clauses
  -------------------------------*/
#define io_dispatch_mgr__get_session_state_or_closed session_mgr__get_session_state_or_closed

/*--------------------------
   LOCAL_OPERATIONS Clause
  --------------------------*/
extern void io_dispatch_mgr__get_response_type(
   const constants__t_msg_type io_dispatch_mgr__req_msg_typ,
   constants__t_msg_type * const io_dispatch_mgr__resp_msg_typ);
extern void io_dispatch_mgr__is_request_type(
   const constants__t_msg_type io_dispatch_mgr__msg_typ,
   t_bool * const io_dispatch_mgr__bres);
extern void io_dispatch_mgr__local_activate_session(
   const constants__t_session_i io_dispatch_mgr__session,
   const constants__t_user_i io_dispatch_mgr__user,
   constants__t_StatusCode_i * const io_dispatch_mgr__ret);
extern void io_dispatch_mgr__local_create_session(
   const constants__t_session_i io_dispatch_mgr__session,
   const constants__t_channel_i io_dispatch_mgr__channel);
extern void io_dispatch_mgr__local_sc_activate_orphaned_sessions(
   const constants__t_channel_config_idx_i io_dispatch_mgr__channel_config_idx,
   const constants__t_channel_i io_dispatch_mgr__channel);
extern void io_dispatch_mgr__treat_read_request(
   const constants__t_msg_i io_dispatch_mgr__p_request_msg,
   const constants__t_msg_i io_dispatch_mgr__p_response_msg);
extern void io_dispatch_mgr__treat_write_request(
   const constants__t_ByteString_i io_dispatch_mgr__req_payload,
   const constants__t_user_i io_dispatch_mgr__userid,
   constants__t_StatusCode_i * const io_dispatch_mgr__StatusCode_service);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void io_dispatch_mgr__activate_new_session(
   const constants__t_channel_config_idx_i io_dispatch_mgr__channel_config_idx,
   const constants__t_user_i io_dispatch_mgr__user,
   t_bool * const io_dispatch_mgr__bres);
extern void io_dispatch_mgr__activate_session(
   const constants__t_session_i io_dispatch_mgr__session,
   const constants__t_user_i io_dispatch_mgr__user,
   constants__t_StatusCode_i * const io_dispatch_mgr__ret);
extern void io_dispatch_mgr__cli_channel_connected_event(
   const constants__t_channel_config_idx_i io_dispatch_mgr__channel_config_idx,
   const constants__t_channel_i io_dispatch_mgr__channel);
extern void io_dispatch_mgr__cli_secure_channel_timeout(
   const constants__t_channel_config_idx_i io_dispatch_mgr__channel_config_idx);
extern void io_dispatch_mgr__close_all_active_connections(
   t_bool * const io_dispatch_mgr__bres);
extern void io_dispatch_mgr__close_session(
   const constants__t_session_i io_dispatch_mgr__session,
   constants__t_StatusCode_i * const io_dispatch_mgr__ret);
extern void io_dispatch_mgr__msgs_memory_changed(void);
extern void io_dispatch_mgr__receive_msg(
   const constants__t_channel_i io_dispatch_mgr__channel,
   const constants__t_msg_i io_dispatch_mgr__msg);
extern void io_dispatch_mgr__secure_channel_lost(
   const constants__t_channel_i io_dispatch_mgr__channel);
extern void io_dispatch_mgr__send_service_request_msg(
   const constants__t_session_i io_dispatch_mgr__session,
   const constants__t_msg_i io_dispatch_mgr__req_msg,
   constants__t_StatusCode_i * const io_dispatch_mgr__ret);
extern void io_dispatch_mgr__srv_channel_connected_event(
   const constants__t_endpoint_config_idx_i io_dispatch_mgr__endpoint_config_idx,
   const constants__t_channel_config_idx_i io_dispatch_mgr__channel_config_idx,
   const constants__t_channel_i io_dispatch_mgr__channel);

#endif
