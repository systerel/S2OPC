/******************************************************************************

 File Name            : channel_mgr_bs.h

 Date                 : 10/08/2017 17:27:13

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _channel_mgr_bs_h
#define _channel_mgr_bs_h

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
extern void channel_mgr_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void channel_mgr_bs__channel_lost(
   const constants__t_channel_i channel_mgr_bs__channel);
extern void channel_mgr_bs__cli_open_secure_channel(
   const constants__t_channel_config_idx_i channel_mgr_bs__config_idx,
   t_bool * const channel_mgr_bs__bres);
extern void channel_mgr_bs__cli_set_connected_channel(
   const constants__t_channel_config_idx_i channel_mgr_bs__config_idx,
   const constants__t_channel_i channel_mgr_bs__channel,
   t_bool * const channel_mgr_bs__bres);
extern void channel_mgr_bs__cli_set_connection_timeout_channel(
   const constants__t_channel_config_idx_i channel_mgr_bs__config_idx,
   t_bool * const channel_mgr_bs__bres);
extern void channel_mgr_bs__close_all_channel(
   t_bool * const channel_mgr_bs__bres);
extern void channel_mgr_bs__close_secure_channel(
   const constants__t_channel_i channel_mgr_bs__channel);
extern void channel_mgr_bs__get_channel_info(
   const constants__t_channel_i channel_mgr_bs__channel,
   constants__t_channel_config_idx_i * const channel_mgr_bs__config_idx);
extern void channel_mgr_bs__get_connected_channel(
   const constants__t_channel_config_idx_i channel_mgr_bs__config_idx,
   constants__t_channel_i * const channel_mgr_bs__channel);
extern void channel_mgr_bs__is_client_channel(
   const constants__t_channel_i channel_mgr_bs__channel,
   t_bool * const channel_mgr_bs__bres);
extern void channel_mgr_bs__is_connected_channel(
   const constants__t_channel_i channel_mgr_bs__channel,
   t_bool * const channel_mgr_bs__bres);
extern void channel_mgr_bs__is_disconnecting_channel(
   const constants__t_channel_config_idx_i channel_mgr_bs__config_idx,
   t_bool * const channel_mgr_bs__bres);
extern void channel_mgr_bs__send_channel_msg_buffer(
   const constants__t_channel_i channel_mgr_bs__channel,
   const constants__t_byte_buffer_i channel_mgr_bs__buffer);
extern void channel_mgr_bs__server_get_endpoint_config(
   const constants__t_channel_i channel_mgr_bs__channel,
   constants__t_endpoint_config_idx_i * const channel_mgr_bs__endpoint_config_idx);
extern void channel_mgr_bs__srv_new_secure_channel(
   const constants__t_endpoint_config_idx_i channel_mgr_bs__endpoint_config_idx,
   const constants__t_channel_config_idx_i channel_mgr_bs__channel_config_idx,
   const constants__t_channel_i channel_mgr_bs__channel,
   t_bool * const channel_mgr_bs__bres);

#endif
