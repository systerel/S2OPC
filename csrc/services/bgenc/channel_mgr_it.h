/******************************************************************************

 File Name            : channel_mgr_it.h

 Date                 : 19/10/2017 10:16:36

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _channel_mgr_it_h
#define _channel_mgr_it_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
extern t_entier4 channel_mgr_it__channel_i;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void channel_mgr_it__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void channel_mgr_it__continue_iter_channel(
   t_bool * const channel_mgr_it__p_continue,
   constants__t_channel_i * const channel_mgr_it__p_channel);
extern void channel_mgr_it__init_iter_channel(
   t_bool * const channel_mgr_it__p_continue);

#endif
