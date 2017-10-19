/******************************************************************************

 File Name            : session_core_1.h

 Date                 : 19/10/2017 10:16:39

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _session_core_1_h
#define _session_core_1_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*-----------------
   IMPORTS Clause
  -----------------*/
#include "session_core_1_it.h"
#include "session_core_2.h"
#include "session_core_bs.h"

/*--------------
   SEES Clause
  --------------*/
#include "channel_mgr.h"
#include "constants.h"
#include "message_in_bs.h"
#include "message_out_bs.h"
#include "request_handle_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void session_core_1__INITIALISATION(void);

/*-------------------------------
   PROMOTES and EXTENDS Clauses
  -------------------------------*/
#define session_core_1__client_activate_session_req_do_crypto session_core_bs__client_activate_session_req_do_crypto
#define session_core_1__client_activate_session_resp_check session_core_bs__client_activate_session_resp_check
#define session_core_1__client_close_session_req_msg session_core_bs__client_close_session_req_msg
#define session_core_1__client_close_session_resp_msg session_core_bs__client_close_session_resp_msg
#define session_core_1__client_create_session_check_crypto session_core_bs__client_create_session_check_crypto
#define session_core_1__client_create_session_req_do_crypto session_core_bs__client_create_session_req_do_crypto
#define session_core_1__client_get_token_from_session session_core_bs__client_get_token_from_session
#define session_core_1__client_set_session_token session_core_bs__client_set_session_token
#define session_core_1__drop_NonceClient session_core_bs__drop_NonceClient
#define session_core_1__get_NonceClient session_core_bs__get_NonceClient
#define session_core_1__get_NonceServer session_core_bs__get_NonceServer
#define session_core_1__get_session_channel session_core_2__get_session_channel
#define session_core_1__get_session_state session_core_2__get_session_state
#define session_core_1__get_session_user session_core_bs__get_session_user
#define session_core_1__is_valid_session session_core_2__is_valid_session
#define session_core_1__is_valid_user session_core_bs__is_valid_user
#define session_core_1__server_activate_session_check_crypto session_core_bs__server_activate_session_check_crypto
#define session_core_1__server_close_session_check_req session_core_bs__server_close_session_check_req
#define session_core_1__server_create_session_req_do_crypto session_core_bs__server_create_session_req_do_crypto
#define session_core_1__server_get_fresh_session_token session_core_bs__server_get_fresh_session_token
#define session_core_1__server_get_session_from_token session_core_bs__server_get_session_from_token
#define session_core_1__server_is_valid_session_token session_core_bs__server_is_valid_session_token
#define session_core_1__session_do_nothing session_core_bs__session_do_nothing
#define session_core_1__set_session_channel session_core_2__set_session_channel
#define session_core_1__set_session_user session_core_bs__set_session_user

/*--------------------------
   LOCAL_OPERATIONS Clause
  --------------------------*/
extern void session_core_1__l_set_session_state(
   const constants__t_session_i session_core_1__p_session,
   const constants__t_sessionState session_core_1__p_state);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void session_core_1__create_session(
   const constants__t_session_i session_core_1__session,
   const constants__t_channel_i session_core_1__channel,
   const constants__t_sessionState session_core_1__state);
extern void session_core_1__create_session_failure(
   const constants__t_session_i session_core_1__session);
extern void session_core_1__delete_session(
   const constants__t_session_i session_core_1__session);
extern void session_core_1__init_new_session(
   constants__t_session_i * const session_core_1__p_session);
extern void session_core_1__set_session_orphaned(
   const constants__t_session_i session_core_1__session,
   const constants__t_channel_config_idx_i session_core_1__channel_config_idx);
extern void session_core_1__set_session_state(
   const constants__t_session_i session_core_1__session,
   const constants__t_sessionState session_core_1__state);
extern void session_core_1__set_session_state_closed(
   const constants__t_session_i session_core_1__session,
   const t_bool session_core_1__is_client);

#endif
