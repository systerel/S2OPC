/******************************************************************************

 File Name            : msg_read_response.c

 Date                 : 13/07/2017 16:54:06

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "msg_read_response.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
constants__t_StatusCode_i msg_read_response__StatusCode;
constants__t_msg_i msg_read_response__msg;
t_entier4 msg_read_response__nb_resps_to_go;
constants__t_StatusCode_i msg_read_response__tab_StatusCode[constants__t_ReadValue_i_max+1];
constants__t_Variant_i msg_read_response__tab_Value[constants__t_ReadValue_i_max+1];

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void msg_read_response__INITIALISATION(void) {
   {
      t_entier4 i;
      for (i = constants__t_ReadValue_i_max; 0 <= i; i = i - 1) {
         msg_read_response__tab_Value[i] = constants__c_Variant_indet;
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_ReadValue_i_max; 0 <= i; i = i - 1) {
         msg_read_response__tab_StatusCode[i] = constants__c_StatusCode_indet;
      }
   }
   msg_read_response__StatusCode = constants__c_StatusCode_indet;
   msg_read_response__nb_resps_to_go = 0;
   msg_read_response__msg = constants__c_msg_indet;
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void msg_read_response__init_iterwrite_DataValue(
   const t_entier4 msg_read_response__a_nb_resps,
   const constants__t_msg_i msg_read_response__resp_msg,
   t_bool * const msg_read_response__continue) {
   msg_read_response__StatusCode = constants__e_sc_ok;
   {
      t_entier4 i;
      for (i = constants__t_ReadValue_i_max; 0 <= i; i = i - 1) {
         msg_read_response__tab_Value[i] = constants__c_Variant_indet;
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_ReadValue_i_max; 0 <= i; i = i - 1) {
         msg_read_response__tab_StatusCode[i] = constants__c_StatusCode_indet;
      }
   }
   msg_read_response__nb_resps_to_go = msg_read_response__a_nb_resps;
   *msg_read_response__continue = (0 < msg_read_response__a_nb_resps);
   msg_read_response__msg = msg_read_response__resp_msg;
   msg_read_response_bs__write_read_response_init(msg_read_response__a_nb_resps,
      msg_read_response__resp_msg);
}

void msg_read_response__continue_iterwrite_DataValue(
   const constants__t_ReadValue_i msg_read_response__rvi,
   const constants__t_Variant_i msg_read_response__val,
   const constants__t_StatusCode_i msg_read_response__sc,
   t_bool * const msg_read_response__continue) {
   if (msg_read_response__val != constants__c_Variant_indet) {
      msg_read_response__tab_Value[msg_read_response__rvi] = msg_read_response__val;
   }
   msg_read_response__tab_StatusCode[msg_read_response__rvi] = msg_read_response__sc;
   if (0 < msg_read_response__nb_resps_to_go) {
      msg_read_response__nb_resps_to_go = msg_read_response__nb_resps_to_go -
         1;
   }
   *msg_read_response__continue = (0 < msg_read_response__nb_resps_to_go);
   msg_read_response_bs__write_read_response_iter(msg_read_response__msg,
      msg_read_response__rvi,
      msg_read_response__val,
      msg_read_response__sc);
}

void msg_read_response__write_response_error(
   const constants__t_StatusCode_i msg_read_response__sc,
   const constants__t_msg_i msg_read_response__resp_msg) {
   msg_read_response__StatusCode = msg_read_response__sc;
}

