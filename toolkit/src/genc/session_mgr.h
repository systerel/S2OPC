/******************************************************************************

 File Name            : session_mgr.h

 Date                 : 29/09/2017 10:52:00

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _session_mgr_h
#define _session_mgr_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*-----------------
   IMPORTS Clause
  -----------------*/
#include "session_async_bs.h"
#include "session_core.h"
#include "session_request_handle_bs.h"

/*--------------
   SEES Clause
  --------------*/
#include "channel_mgr_bs.h"
#include "constants.h"
#include "message_in_bs.h"
#include "message_out_bs.h"
#include "request_handle_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void session_mgr__INITIALISATION(void);

/*-------------------------------
   PROMOTES and EXTENDS Clauses
  -------------------------------*/
#define session_mgr__client_secure_channel_lost_session_sm session_core__client_secure_channel_lost_session_sm
#define session_mgr__get_session_user_or_indet session_core__get_session_user_or_indet
#define session_mgr__server_close_session_sm session_core__server_close_session_sm
#define session_mgr__server_secure_channel_lost_session_sm session_core__server_secure_channel_lost_session_sm

/*--------------------------
   LOCAL_OPERATIONS Clause
  --------------------------*/
extern void session_mgr__local_sc_activate_orphaned_sessions(
   const constants__t_channel_config_idx_i session_mgr__channel_config_idx,
   const constants__t_channel_i session_mgr__channel);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void session_mgr__client_async_activate_new_session_with_channel(
   const constants__t_channel_config_idx_i session_mgr__channel_config_idx,
   const constants__t_channel_i session_mgr__channel,
   const constants__t_user_i session_mgr__user,
   t_bool * const session_mgr__bres);
extern void session_mgr__client_async_activate_new_session_without_channel(
   const constants__t_channel_config_idx_i session_mgr__channel_config_idx,
   const constants__t_user_i session_mgr__user,
   t_bool * const session_mgr__bres);
extern void session_mgr__client_channel_connected_event_session(
   const constants__t_channel_config_idx_i session_mgr__channel_config_idx,
   const constants__t_channel_i session_mgr__channel);
extern void session_mgr__client_close_session(
   const constants__t_session_i session_mgr__session);
extern void session_mgr__client_close_session_req(
   const constants__t_session_i session_mgr__session,
   const constants__t_request_handle_i session_mgr__req_handle,
   const constants__t_msg_i session_mgr__close_req_msg,
   constants__t_StatusCode_i * const session_mgr__ret,
   constants__t_channel_i * const session_mgr__channel,
   constants__t_session_token_i * const session_mgr__session_token);
extern void session_mgr__client_create_session_req(
   const constants__t_session_i session_mgr__session,
   const constants__t_channel_i session_mgr__channel,
   const constants__t_request_handle_i session_mgr__req_handle,
   const constants__t_msg_i session_mgr__create_req_msg,
   constants__t_StatusCode_i * const session_mgr__ret);
extern void session_mgr__client_receive_session_resp(
   const constants__t_channel_i session_mgr__channel,
   const constants__t_request_handle_i session_mgr__req_handle,
   const constants__t_msg_type_i session_mgr__resp_typ,
   const constants__t_msg_header_i session_mgr__resp_header,
   const constants__t_msg_i session_mgr__resp_msg,
   constants__t_session_i * const session_mgr__session);
extern void session_mgr__client_sc_activate_session_req(
   const constants__t_session_i session_mgr__session,
   const constants__t_request_handle_i session_mgr__req_handle,
   const constants__t_channel_i session_mgr__channel,
   const constants__t_msg_i session_mgr__activate_req_msg,
   constants__t_StatusCode_i * const session_mgr__ret,
   constants__t_session_token_i * const session_mgr__session_token);
extern void session_mgr__client_user_activate_session_req(
   const constants__t_session_i session_mgr__session,
   const constants__t_request_handle_i session_mgr__req_handle,
   const constants__t_user_i session_mgr__user,
   const constants__t_msg_i session_mgr__activate_req_msg,
   constants__t_StatusCode_i * const session_mgr__ret,
   constants__t_channel_i * const session_mgr__channel,
   constants__t_session_token_i * const session_mgr__session_token);
extern void session_mgr__client_validate_session_service_req(
   const constants__t_session_i session_mgr__session,
   const constants__t_request_handle_i session_mgr__req_handle,
   const constants__t_msg_i session_mgr__req_msg,
   constants__t_StatusCode_i * const session_mgr__ret,
   constants__t_channel_i * const session_mgr__channel,
   constants__t_session_token_i * const session_mgr__session_token);
extern void session_mgr__client_validate_session_service_resp(
   const constants__t_channel_i session_mgr__channel,
   const constants__t_request_handle_i session_mgr__req_handle,
   t_bool * const session_mgr__bres);
extern void session_mgr__server_receive_session_req(
   const constants__t_channel_i session_mgr__channel,
   const constants__t_session_token_i session_mgr__session_token,
   const constants__t_msg_i session_mgr__req_msg,
   const constants__t_msg_type_i session_mgr__req_typ,
   const constants__t_msg_i session_mgr__resp_msg,
   constants__t_session_i * const session_mgr__session,
   constants__t_StatusCode_i * const session_mgr__service_ret);
extern void session_mgr__server_validate_session_service_req(
   const constants__t_channel_i session_mgr__channel,
   const constants__t_request_handle_i session_mgr__req_handle,
   const constants__t_session_token_i session_mgr__session_token,
   t_bool * const session_mgr__is_valid_res,
   constants__t_session_i * const session_mgr__session,
   t_bool * const session_mgr__snd_err);
extern void session_mgr__server_validate_session_service_resp(
   const constants__t_channel_i session_mgr__channel,
   const constants__t_session_i session_mgr__session,
   const constants__t_request_handle_i session_mgr__req_handle,
   t_bool * const session_mgr__is_valid_res,
   t_bool * const session_mgr__snd_err);

#endif
