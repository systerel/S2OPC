/******************************************************************************

 File Name            : session_core_bs.h

 Date                 : 19/10/2017 10:16:44

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _session_core_bs_h
#define _session_core_bs_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

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
extern void session_core_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void session_core_bs__client_activate_session_req_do_crypto(
   const constants__t_session_i session_core_bs__session,
   const constants__t_channel_config_idx_i session_core_bs__channel_config_idx,
   const constants__t_Nonce_i session_core_bs__server_nonce,
   t_bool * const session_core_bs__valid,
   constants__t_SignatureData_i * const session_core_bs__signature);
extern void session_core_bs__client_activate_session_resp_check(
   const constants__t_session_i session_core_bs__p_session,
   const constants__t_channel_config_idx_i session_core_bs__p_channel_config_idx,
   const constants__t_msg_i session_core_bs__p_resp_msg,
   t_bool * const session_core_bs__valid);
extern void session_core_bs__client_close_session_req_msg(
   const constants__t_msg_i session_core_bs__req_msg);
extern void session_core_bs__client_close_session_resp_msg(
   const constants__t_msg_i session_core_bs__resp_msg);
extern void session_core_bs__client_create_session_check_crypto(
   const constants__t_session_i session_core_bs__p_session,
   const constants__t_channel_config_idx_i session_core_bs__p_channel_config_idx,
   const constants__t_msg_i session_core_bs__p_resp_msg,
   t_bool * const session_core_bs__valid);
extern void session_core_bs__client_create_session_req_do_crypto(
   const constants__t_session_i session_core_bs__p_session,
   const constants__t_channel_i session_core_bs__p_channel,
   const constants__t_channel_config_idx_i session_core_bs__p_channel_config_idx,
   t_bool * const session_core_bs__valid,
   t_bool * const session_core_bs__nonce_needed);
extern void session_core_bs__client_get_token_from_session(
   const constants__t_session_i session_core_bs__session,
   constants__t_session_token_i * const session_core_bs__session_token);
extern void session_core_bs__client_set_session_token(
   const constants__t_session_i session_core_bs__session,
   const constants__t_session_token_i session_core_bs__token);
extern void session_core_bs__delete_session_token(
   const constants__t_session_i session_core_bs__p_session);
extern void session_core_bs__drop_NonceClient(
   const constants__t_session_i session_core_bs__p_session);
extern void session_core_bs__get_NonceClient(
   const constants__t_session_i session_core_bs__p_session,
   constants__t_Nonce_i * const session_core_bs__nonce);
extern void session_core_bs__get_NonceServer(
   const constants__t_session_i session_core_bs__p_session,
   constants__t_Nonce_i * const session_core_bs__nonce);
extern void session_core_bs__get_session_user(
   const constants__t_session_i session_core_bs__session,
   constants__t_user_i * const session_core_bs__user);
extern void session_core_bs__is_valid_user(
   const constants__t_user_i session_core_bs__user,
   t_bool * const session_core_bs__ret);
extern void session_core_bs__notify_set_session_state(
   const constants__t_session_i session_core_bs__session,
   const constants__t_sessionState session_core_bs__prec_state,
   const constants__t_sessionState session_core_bs__state);
extern void session_core_bs__prepare_close_session(
   const constants__t_session_i session_core_bs__session,
   const constants__t_sessionState session_core_bs__state,
   const t_bool session_core_bs__is_client);
extern void session_core_bs__server_activate_session_check_crypto(
   const constants__t_session_i session_core_bs__session,
   const constants__t_channel_i session_core_bs__channel,
   const constants__t_channel_config_idx_i session_core_bs__channel_config_idx,
   const constants__t_msg_i session_core_bs__activate_req_msg,
   t_bool * const session_core_bs__valid);
extern void session_core_bs__server_close_session_check_req(
   const constants__t_msg_i session_core_bs__req_msg,
   const constants__t_msg_i session_core_bs__resp_msg);
extern void session_core_bs__server_create_session_req_do_crypto(
   const constants__t_session_i session_core_bs__p_session,
   const constants__t_msg_i session_core_bs__p_req_msg,
   const constants__t_endpoint_config_idx_i session_core_bs__p_endpoint_config_idx,
   const constants__t_channel_config_idx_i session_core_bs__p_channel_config_idx,
   t_bool * const session_core_bs__valid,
   constants__t_SignatureData_i * const session_core_bs__signature);
extern void session_core_bs__server_get_fresh_session_token(
   const constants__t_session_i session_core_bs__session,
   constants__t_session_token_i * const session_core_bs__token);
extern void session_core_bs__server_get_session_from_token(
   const constants__t_session_token_i session_core_bs__session_token,
   constants__t_session_i * const session_core_bs__session);
extern void session_core_bs__server_is_valid_session_token(
   const constants__t_session_token_i session_core_bs__token,
   t_bool * const session_core_bs__ret);
extern void session_core_bs__session_do_nothing(
   const constants__t_session_i session_core_bs__session);
extern void session_core_bs__set_session_user(
   const constants__t_session_i session_core_bs__session,
   const constants__t_user_i session_core_bs__user);

#endif
