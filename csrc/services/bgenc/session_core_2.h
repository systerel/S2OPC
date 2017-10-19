/******************************************************************************

 File Name            : session_core_2.h

 Date                 : 19/10/2017 10:16:39

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _session_core_2_h
#define _session_core_2_h

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

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
extern constants__t_channel_i session_core_2__a_channel_i[constants__t_session_i_max+1];
extern constants__t_channel_config_idx_i session_core_2__a_orphaned_i[constants__t_session_i_max+1];
extern constants__t_sessionState session_core_2__a_state_i[constants__t_session_i_max+1];
extern t_bool session_core_2__s_session_i[constants__t_session_i_max+1];

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void session_core_2__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void session_core_2__add_session(
   const constants__t_session_i session_core_2__p_session);
extern void session_core_2__get_session_channel(
   const constants__t_session_i session_core_2__session,
   constants__t_channel_i * const session_core_2__channel);
extern void session_core_2__get_session_state(
   const constants__t_session_i session_core_2__session,
   constants__t_sessionState * const session_core_2__state);
extern void session_core_2__is_valid_session(
   const constants__t_session_i session_core_2__session,
   t_bool * const session_core_2__ret);
extern void session_core_2__remove_session(
   const constants__t_session_i session_core_2__p_session);
extern void session_core_2__reset_session_channel(
   const constants__t_session_i session_core_2__p_session);
extern void session_core_2__reset_session_orphaned(
   const constants__t_session_i session_core_2__p_session);
extern void session_core_2__set_session_channel(
   const constants__t_session_i session_core_2__session,
   const constants__t_channel_i session_core_2__channel);
extern void session_core_2__set_session_orphaned_1(
   const constants__t_session_i session_core_2__p_session,
   const constants__t_channel_config_idx_i session_core_2__p_channel_config_idx);
extern void session_core_2__set_session_state_1(
   const constants__t_session_i session_core_2__p_session,
   const constants__t_sessionState session_core_2__p_state);

#endif
