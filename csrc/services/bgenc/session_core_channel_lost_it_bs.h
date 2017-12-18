/******************************************************************************

 File Name            : session_core_channel_lost_it_bs.h

 Date                 : 18/12/2017 17:24:10

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _session_core_channel_lost_it_bs_h
#define _session_core_channel_lost_it_bs_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"
#include "session_core_1.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void session_core_channel_lost_it_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void session_core_channel_lost_it_bs__continue_iter_channel_lost_t_session(
    constants__t_session_i* const session_core_channel_lost_it_bs__session,
    t_bool* const session_core_channel_lost_it_bs__continue);
extern void session_core_channel_lost_it_bs__init_iter_channel_lost_t_session(
    const constants__t_channel_i session_core_channel_lost_it_bs__lost_channel,
    t_bool* const session_core_channel_lost_it_bs__continue);

#endif
