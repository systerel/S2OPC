/******************************************************************************

 File Name            : msg_read_request_it.h

 Date                 : 31/05/2017 17:51:42

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _msg_read_request_it_h
#define _msg_read_request_it_h

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
extern t_entier4 msg_read_request_it__ind_reqs;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void msg_read_request_it__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void msg_read_request_it__continue_iter_reqs(
   t_bool * const msg_read_request_it__continue,
   constants__t_ReadValue_i * const msg_read_request_it__rvi);
extern void msg_read_request_it__init_iter_reqs(
   const t_entier4 msg_read_request_it__a_nb_reqs,
   t_bool * const msg_read_request_it__continue);

#endif
