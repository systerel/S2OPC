/******************************************************************************

 File Name            : service_read_it.h

 Date                 : 25/07/2017 17:17:59

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _service_read_it_h
#define _service_read_it_h

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
extern t_entier4 service_read_it__rreqs_i;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void service_read_it__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void service_read_it__continue_iter_write_request(
   t_bool * const service_read_it__continue,
   constants__t_ReadValue_i * const service_read_it__rvi);
extern void service_read_it__init_iter_write_request(
   const t_entier4 service_read_it__nb_req,
   t_bool * const service_read_it__continue);

#endif
