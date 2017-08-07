/******************************************************************************

 File Name            : address_space_it.c

 Date                 : 08/08/2017 10:57:22

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "address_space_it.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
t_entier4 address_space_it__wreqs_i;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void address_space_it__INITIALISATION(void) {
   address_space_it__wreqs_i = 0;
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void address_space_it__init_iter_write_request(
   const t_entier4 address_space_it__nb_req,
   t_bool * const address_space_it__continue) {
   address_space_it__wreqs_i = address_space_it__nb_req;
   *address_space_it__continue = (0 < address_space_it__nb_req);
}

void address_space_it__continue_iter_write_request(
   t_bool * const address_space_it__continue,
   constants__t_WriteValue_i * const address_space_it__wvi) {
   constants__get_cast_t_WriteValue(address_space_it__wreqs_i,
      address_space_it__wvi);
   address_space_it__wreqs_i = address_space_it__wreqs_i -
      1;
   *address_space_it__continue = (0 < address_space_it__wreqs_i);
}

