/******************************************************************************

 File Name            : msg_read_request.c

 Date                 : 24/07/2017 18:24:09

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "msg_read_request.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
t_entier4 msg_read_request__nb_ReadValue;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void msg_read_request__INITIALISATION(void) {
   msg_read_request__nb_ReadValue = 0;
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void msg_read_request__check_ReadRequest(
   const constants__t_msg_i msg_read_request__p_msg,
   t_bool * const msg_read_request__p_read_ok,
   t_entier4 * const msg_read_request__p_nb_ReadValue) {
   msg_read_request_bs__read_req_nb_ReadValue(msg_read_request__p_msg,
      msg_read_request__p_nb_ReadValue);
   *msg_read_request__p_read_ok = ((0 <= *msg_read_request__p_nb_ReadValue) &&
      (*msg_read_request__p_nb_ReadValue <= constants__k_n_read_resp_max));
   if (*msg_read_request__p_read_ok == true) {
      msg_read_request__nb_ReadValue = *msg_read_request__p_nb_ReadValue;
   }
}

void msg_read_request__getall_ReadValue_NodeId_AttributeId(
   const constants__t_msg_i msg_read_request__p_msg,
   const constants__t_ReadValue_i msg_read_request__p_rvi,
   t_bool * const msg_read_request__p_isvalid,
   constants__t_NodeId_i * const msg_read_request__p_nid,
   constants__t_AttributeId_i * const msg_read_request__p_aid) {
   msg_read_request_bs__getall_req_ReadValue_NodeId(msg_read_request__p_msg,
      msg_read_request__p_rvi,
      msg_read_request__p_isvalid,
      msg_read_request__p_nid);
   if (*msg_read_request__p_isvalid == true) {
      msg_read_request_bs__getall_req_ReadValue_AttributeId(msg_read_request__p_msg,
         msg_read_request__p_rvi,
         msg_read_request__p_isvalid,
         msg_read_request__p_aid);
   }
   else {
      *msg_read_request__p_aid = constants__c_AttributeId_indet;
   }
}

void msg_read_request__get_nb_ReadValue(
   t_entier4 * const msg_read_request__p_nb_ReadValue) {
   *msg_read_request__p_nb_ReadValue = msg_read_request__nb_ReadValue;
}

