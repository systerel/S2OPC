/******************************************************************************

 File Name            : session_core_1_it.c

 Date                 : 19/10/2017 10:16:39

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "session_core_1_it.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
t_entier4 session_core_1_it__session_i;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void session_core_1_it__INITIALISATION(void) {
   session_core_1_it__session_i = 0;
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void session_core_1_it__init_iter_session(
   t_bool * const session_core_1_it__p_continue) {
   constants__get_card_t_session(&session_core_1_it__session_i);
   *session_core_1_it__p_continue = (1 <= session_core_1_it__session_i);
}

void session_core_1_it__continue_iter_session(
   t_bool * const session_core_1_it__p_continue,
   constants__t_session_i * const session_core_1_it__p_session) {
   constants__get_cast_t_session(session_core_1_it__session_i,
      session_core_1_it__p_session);
   session_core_1_it__session_i = session_core_1_it__session_i -
      1;
   *session_core_1_it__p_continue = (1 <= session_core_1_it__session_i);
}

