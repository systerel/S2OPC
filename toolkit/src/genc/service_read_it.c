/******************************************************************************

 File Name            : service_read_it.c

 Date                 : 09/08/2017 10:37:54

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "service_read_it.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
t_entier4 service_read_it__rreqs_i;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void service_read_it__INITIALISATION(void) {
   service_read_it__rreqs_i = 0;
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void service_read_it__init_iter_write_request(
   const t_entier4 service_read_it__nb_req,
   t_bool * const service_read_it__continue) {
   service_read_it__rreqs_i = service_read_it__nb_req;
   *service_read_it__continue = (0 < service_read_it__nb_req);
}

void service_read_it__continue_iter_write_request(
   t_bool * const service_read_it__continue,
   constants__t_ReadValue_i * const service_read_it__rvi) {
   constants__read_cast_t_ReadValue(service_read_it__rreqs_i,
      service_read_it__rvi);
   service_read_it__rreqs_i = service_read_it__rreqs_i -
      1;
   *service_read_it__continue = (0 < service_read_it__rreqs_i);
}

