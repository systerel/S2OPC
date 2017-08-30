/******************************************************************************

 File Name            : address_space_it.h

 Date                 : 30/08/2017 19:04:01

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _address_space_it_h
#define _address_space_it_h

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
extern t_entier4 address_space_it__wreqs_i;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void address_space_it__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void address_space_it__continue_iter_write_request(
   t_bool * const address_space_it__continue,
   constants__t_WriteValue_i * const address_space_it__wvi);
extern void address_space_it__init_iter_write_request(
   const t_entier4 address_space_it__nb_req,
   t_bool * const address_space_it__continue);

#endif
