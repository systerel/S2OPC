/******************************************************************************

 File Name            : channel_mgr.h

 Date                 : 18/12/2017 17:24:01

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _channel_mgr_h
#define _channel_mgr_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*-----------------
   IMPORTS Clause
  -----------------*/
#include "channel_mgr_1.h"
#include "channel_mgr_bs.h"
#include "channel_mgr_it.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
extern t_bool channel_mgr__all_channel_closing;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void channel_mgr__INITIALISATION(void);

/*-------------------------------
   PROMOTES and EXTENDS Clauses
  -------------------------------*/
#define channel_mgr__channel_do_nothing channel_mgr_bs__channel_do_nothing
#define channel_mgr__get_SecurityPolicy channel_mgr_bs__get_SecurityPolicy
#define channel_mgr__get_channel_info channel_mgr_1__get_channel_info
#define channel_mgr__get_connected_channel channel_mgr_1__get_connected_channel
#define channel_mgr__is_client_channel channel_mgr_1__is_client_channel
#define channel_mgr__is_connected_channel channel_mgr_1__is_connected_channel
#define channel_mgr__is_disconnecting_channel channel_mgr_1__is_disconnecting_channel
#define channel_mgr__send_channel_msg_buffer channel_mgr_bs__send_channel_msg_buffer
#define channel_mgr__server_get_endpoint_config channel_mgr_1__server_get_endpoint_config

/*--------------------------
   LOCAL_OPERATIONS Clause
  --------------------------*/
extern void channel_mgr__l_check_all_channel_lost(void);
extern void channel_mgr__l_close_secure_channel(const constants__t_channel_i channel_mgr__p_channel);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void channel_mgr__channel_lost(const constants__t_channel_i channel_mgr__channel);
extern void channel_mgr__cli_open_secure_channel(const constants__t_channel_config_idx_i channel_mgr__config_idx,
                                                 t_bool* const channel_mgr__bres);
extern void channel_mgr__cli_set_connected_channel(const constants__t_channel_config_idx_i channel_mgr__config_idx,
                                                   const constants__t_channel_i channel_mgr__channel,
                                                   t_bool* const channel_mgr__bres);
extern void channel_mgr__cli_set_connection_timeout_channel(
    const constants__t_channel_config_idx_i channel_mgr__config_idx,
    t_bool* const channel_mgr__bres);
extern void channel_mgr__close_all_channel(t_bool* const channel_mgr__bres);
extern void channel_mgr__close_secure_channel(const constants__t_channel_i channel_mgr__channel);
extern void channel_mgr__srv_new_secure_channel(
    const constants__t_endpoint_config_idx_i channel_mgr__endpoint_config_idx,
    const constants__t_channel_config_idx_i channel_mgr__channel_config_idx,
    const constants__t_channel_i channel_mgr__channel,
    t_bool* const channel_mgr__bres);

#endif
