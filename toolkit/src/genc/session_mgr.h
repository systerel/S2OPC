/******************************************************************************

 File Name            : session_mgr.h

 Date                 : 31/07/2017 12:03:50

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
#include "session_core.h"

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
#define session_mgr__client_close_session session_core__client_close_session
#define session_mgr__client_init_session session_core__client_init_session
#define session_mgr__client_secure_channel_lost session_core__client_secure_channel_lost
#define session_mgr__continue_iter_orphaned_t_session session_core__continue_iter_orphaned_t_session
#define session_mgr__delete_session session_core__delete_session
#define session_mgr__get_session_from_token session_core__get_session_from_token
#define session_mgr__get_session_state_or_closed session_core__get_session_state_or_closed
#define session_mgr__get_session_user_or_indet session_core__get_session_user_or_indet
#define session_mgr__init_iter_orphaned_t_session session_core__init_iter_orphaned_t_session
#define session_mgr__is_valid_session session_core__is_valid_session
#define session_mgr__server_close_session session_core__server_close_session
#define session_mgr__server_secure_channel_lost session_core__server_secure_channel_lost

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void session_mgr__client_close_req(
   const constants__t_session_i session_mgr__session,
   const constants__t_request_handle_i session_mgr__req_handle,
   const constants__t_msg_i session_mgr__close_req_msg,
   constants__t_StatusCode_i * const session_mgr__ret,
   constants__t_channel_i * const session_mgr__channel,
   constants__t_session_token_i * const session_mgr__session_token);
extern void session_mgr__client_create_req(
   const constants__t_session_i session_mgr__session,
   const constants__t_channel_i session_mgr__channel,
   const constants__t_request_handle_i session_mgr__req_handle,
   const constants__t_msg_i session_mgr__create_req_msg,
   constants__t_StatusCode_i * const session_mgr__ret);
extern void session_mgr__client_receive_session_resp(
   const constants__t_channel_i session_mgr__channel,
   const constants__t_request_handle_i session_mgr__req_handle,
   const constants__t_msg_i session_mgr__resp_msg,
   const constants__t_msg_type session_mgr__resp_typ,
   constants__t_session_i * const session_mgr__session);
extern void session_mgr__client_sc_activate_req(
   const constants__t_session_i session_mgr__session,
   const constants__t_request_handle_i session_mgr__req_handle,
   const constants__t_channel_i session_mgr__channel,
   const constants__t_msg_i session_mgr__activate_req_msg,
   constants__t_StatusCode_i * const session_mgr__ret,
   constants__t_session_token_i * const session_mgr__session_token);
extern void session_mgr__client_user_activate_req(
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
   const constants__t_msg_i session_mgr__resp_msg,
   t_bool * const session_mgr__bres);
extern void session_mgr__server_receive_session_req(
   const constants__t_channel_i session_mgr__channel,
   const constants__t_request_handle_i session_mgr__req_handle,
   const constants__t_session_token_i session_mgr__session_token,
   const constants__t_msg_i session_mgr__req_msg,
   const constants__t_msg_type session_mgr__req_typ,
   const constants__t_msg_i session_mgr__resp_msg,
   constants__t_session_i * const session_mgr__session,
   constants__t_StatusCode_i * const session_mgr__service_ret);
extern void session_mgr__server_validate_session_service_req(
   const constants__t_channel_i session_mgr__channel,
   const constants__t_request_handle_i session_mgr__req_handle,
   const constants__t_session_token_i session_mgr__session_token,
   const constants__t_msg_i session_mgr__req_msg,
   t_bool * const session_mgr__bres,
   t_bool * const session_mgr__snd_err);
extern void session_mgr__server_validate_session_service_resp(
   const constants__t_channel_i session_mgr__channel,
   const constants__t_session_i session_mgr__session,
   const constants__t_request_handle_i session_mgr__req_handle,
   const constants__t_msg_i session_mgr__req_msg,
   const constants__t_msg_i session_mgr__resp_msg,
   t_bool * const session_mgr__bres,
   t_bool * const session_mgr__snd_err);

#endif
