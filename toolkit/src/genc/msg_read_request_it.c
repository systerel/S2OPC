/******************************************************************************

 File Name            : msg_read_request_it.c

 Date                 : 19/07/2017 17:51:26

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "msg_read_request_it.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
t_entier4 msg_read_request_it__ind_reqs;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void msg_read_request_it__INITIALISATION(void) {
   msg_read_request_it__ind_reqs = 0;
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void msg_read_request_it__init_iter_reqs(
   const t_entier4 msg_read_request_it__a_nb_reqs,
   t_bool * const msg_read_request_it__continue) {
   msg_read_request_it__ind_reqs = msg_read_request_it__a_nb_reqs;
   *msg_read_request_it__continue = (0 < msg_read_request_it__ind_reqs);
}

void msg_read_request_it__continue_iter_reqs(
   t_bool * const msg_read_request_it__continue,
   constants__t_ReadValue_i * const msg_read_request_it__rvi) {
   constants__read_cast_t_ReadValue(msg_read_request_it__ind_reqs,
      msg_read_request_it__rvi);
   msg_read_request_it__ind_reqs = msg_read_request_it__ind_reqs -
      1;
   *msg_read_request_it__continue = (0 < msg_read_request_it__ind_reqs);
}

