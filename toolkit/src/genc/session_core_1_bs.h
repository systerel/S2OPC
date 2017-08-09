/******************************************************************************

 File Name            : session_core_1_bs.h

 Date                 : 09/08/2017 10:37:58

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _session_core_1_bs_h
#define _session_core_1_bs_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"
#include "request_handle_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void session_core_1_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void session_core_1_bs__cli_add_pending_request(
   const constants__t_session_i session_core_1_bs__session,
   const constants__t_request_handle_i session_core_1_bs__req_handle,
   t_bool * const session_core_1_bs__ret);
extern void session_core_1_bs__cli_remove_pending_request(
   const constants__t_session_i session_core_1_bs__session,
   const constants__t_request_handle_i session_core_1_bs__req_handle,
   t_bool * const session_core_1_bs__ret);
extern void session_core_1_bs__client_get_token_from_session(
   const constants__t_session_i session_core_1_bs__session,
   constants__t_session_token_i * const session_core_1_bs__session_token);
extern void session_core_1_bs__client_set_session_token(
   const constants__t_session_i session_core_1_bs__session,
   const constants__t_session_token_i session_core_1_bs__token);
extern void session_core_1_bs__create_session(
   const constants__t_session_i session_core_1_bs__session,
   const constants__t_channel_i session_core_1_bs__channel,
   const constants__t_sessionState session_core_1_bs__state);
extern void session_core_1_bs__create_session_failure(
   const constants__t_session_i session_core_1_bs__session);
extern void session_core_1_bs__delete_session(
   const constants__t_session_i session_core_1_bs__session);
extern void session_core_1_bs__delete_session_token(
   const constants__t_session_token_i session_core_1_bs__session_token);
extern void session_core_1_bs__get_session_channel(
   const constants__t_session_i session_core_1_bs__session,
   constants__t_channel_i * const session_core_1_bs__channel);
extern void session_core_1_bs__get_session_state(
   const constants__t_session_i session_core_1_bs__session,
   constants__t_sessionState * const session_core_1_bs__state);
extern void session_core_1_bs__get_session_user(
   const constants__t_session_i session_core_1_bs__session,
   constants__t_user_i * const session_core_1_bs__user);
extern void session_core_1_bs__init_new_session(
   constants__t_session_i * const session_core_1_bs__session);
extern void session_core_1_bs__is_valid_session(
   const constants__t_session_i session_core_1_bs__session,
   t_bool * const session_core_1_bs__ret);
extern void session_core_1_bs__is_valid_user(
   const constants__t_user_i session_core_1_bs__user,
   t_bool * const session_core_1_bs__ret);
extern void session_core_1_bs__server_get_fresh_session_token(
   const constants__t_session_i session_core_1_bs__session,
   constants__t_session_token_i * const session_core_1_bs__token);
extern void session_core_1_bs__server_get_session_from_token(
   const constants__t_session_token_i session_core_1_bs__session_token,
   constants__t_session_i * const session_core_1_bs__session);
extern void session_core_1_bs__server_is_valid_session_token(
   const constants__t_session_token_i session_core_1_bs__token,
   t_bool * const session_core_1_bs__ret);
extern void session_core_1_bs__set_session_channel(
   const constants__t_session_i session_core_1_bs__session,
   const constants__t_channel_i session_core_1_bs__channel);
extern void session_core_1_bs__set_session_orphaned(
   const constants__t_session_i session_core_1_bs__session,
   const constants__t_channel_i session_core_1_bs__lost_channel,
   const constants__t_channel_config_idx_i session_core_1_bs__channel_config_idx);
extern void session_core_1_bs__set_session_state(
   const constants__t_session_i session_core_1_bs__session,
   const constants__t_sessionState session_core_1_bs__state);
extern void session_core_1_bs__set_session_state_closed(
   const constants__t_session_i session_core_1_bs__session);
extern void session_core_1_bs__set_session_user(
   const constants__t_session_i session_core_1_bs__session,
   const constants__t_user_i session_core_1_bs__user);

#endif
