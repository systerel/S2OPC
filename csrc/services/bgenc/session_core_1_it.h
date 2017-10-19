/******************************************************************************

 File Name            : session_core_1_it.h

 Date                 : 19/10/2017 10:16:39

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _session_core_1_it_h
#define _session_core_1_it_h

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
extern t_entier4 session_core_1_it__session_i;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void session_core_1_it__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void session_core_1_it__continue_iter_session(
   t_bool * const session_core_1_it__p_continue,
   constants__t_session_i * const session_core_1_it__p_session);
extern void session_core_1_it__init_iter_session(
   t_bool * const session_core_1_it__p_continue);

#endif
