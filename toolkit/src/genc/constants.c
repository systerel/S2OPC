/******************************************************************************

 File Name            : constants.c

 Date                 : 15/09/2017 14:19:07

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "constants.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void constants__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void constants__read_cast_t_ReadValue(
   const t_entier4 constants__ii,
   constants__t_ReadValue_i * const constants__rvi) {
   *constants__rvi = constants__ii;
}

void constants__get_cast_t_WriteValue(
   const t_entier4 constants__ii,
   constants__t_WriteValue_i * const constants__wvi) {
   *constants__wvi = constants__ii;
}

void constants__get_cast_t_BrowseValue(
   const t_entier4 constants__p_ind,
   constants__t_BrowseValue_i * const constants__p_bvi) {
   *constants__p_bvi = constants__p_ind;
}

void constants__get_cast_t_BrowseResult(
   const t_entier4 constants__p_ind,
   constants__t_BrowseResult_i * const constants__p_bri) {
   *constants__p_bri = constants__p_ind;
}

void constants__get_Is_Dir_Forward_Compatible(
   const constants__t_BrowseDirection_i constants__p_dir,
   const t_bool constants__p_IsForward,
   t_bool * const constants__p_dir_compat) {
   switch (constants__p_dir) {
   case constants__e_bd_forward:
      *constants__p_dir_compat = constants__p_IsForward;
      break;
   case constants__e_bd_inverse:
      *constants__p_dir_compat = (constants__p_IsForward == false);
      break;
   default:
      *constants__p_dir_compat = true;
      break;
   }
}

