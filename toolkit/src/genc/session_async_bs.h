/******************************************************************************

 File Name            : session_async_bs.h

 Date                 : 03/08/2017 13:22:26

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _session_async_bs_h
#define _session_async_bs_h

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
extern void session_async_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void session_async_bs__add_session_to_activate(
   const constants__t_session_i session_async_bs__session,
   const constants__t_user_i session_async_bs__user,
   t_bool * const session_async_bs__ret);
extern void session_async_bs__add_session_to_create(
   const constants__t_session_i session_async_bs__session,
   const constants__t_channel_config_idx_i session_async_bs__channel_config_idx,
   t_bool * const session_async_bs__ret);
extern void session_async_bs__is_session_to_activate(
   const constants__t_session_i session_async_bs__session,
   constants__t_user_i * const session_async_bs__user);
extern void session_async_bs__is_session_to_create(
   const constants__t_channel_config_idx_i session_async_bs__channel_config_idx,
   constants__t_session_i * const session_async_bs__session);

#endif
