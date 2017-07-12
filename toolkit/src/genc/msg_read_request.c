/******************************************************************************

 File Name            : msg_read_request.c

 Date                 : 13/07/2017 16:54:05

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "msg_read_request.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
t_bool msg_read_request__init_ok;
t_entier4 msg_read_request__nb_ReadValue;
constants__t_AttributeId_i msg_read_request__tab_req_AttributeId[constants__t_ReadValue_i_max+1];
constants__t_NodeId_i msg_read_request__tab_req_NodeId[constants__t_ReadValue_i_max+1];

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void msg_read_request__INITIALISATION(void) {
   msg_read_request__init_ok = false;
   msg_read_request__nb_ReadValue = 0;
   {
      t_entier4 i;
      for (i = constants__t_ReadValue_i_max; 0 <= i; i = i - 1) {
         msg_read_request__tab_req_NodeId[i] = constants__c_NodeId_indet;
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_ReadValue_i_max; 0 <= i; i = i - 1) {
         msg_read_request__tab_req_AttributeId[i] = constants__c_AttributeId_indet;
      }
   }
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void msg_read_request__read_ReadRequest(
   const constants__t_msg_i msg_read_request__msg,
   t_bool * const msg_read_request__rr) {
   {
      t_bool msg_read_request__l_continue;
      constants__t_ReadValue_i msg_read_request__rvi;
      t_entier4 msg_read_request__l_nb_RV;
      constants__t_NodeId_i msg_read_request__l_nid;
      constants__t_AttributeId_i msg_read_request__l_aid;
      
      msg_read_request_bs__read_req_nb_ReadValue(msg_read_request__msg,
         &msg_read_request__l_nb_RV);
      msg_read_request__init_ok = ((0 <= msg_read_request__l_nb_RV) &&
         (msg_read_request__l_nb_RV <= constants__k_n_read_resp_max));
      if (msg_read_request__init_ok == true) {
         msg_read_request__nb_ReadValue = msg_read_request__l_nb_RV;
         msg_read_request_it__init_iter_reqs(msg_read_request__l_nb_RV,
            &msg_read_request__l_continue);
         while (msg_read_request__l_continue == true) {
            msg_read_request_it__continue_iter_reqs(&msg_read_request__l_continue,
               &msg_read_request__rvi);
            msg_read_request_bs__read_req_ReadValue_NodeId(msg_read_request__msg,
               msg_read_request__rvi,
               &msg_read_request__l_nid);
            msg_read_request_bs__read_req_ReadValue_AttributeId(msg_read_request__msg,
               msg_read_request__rvi,
               &msg_read_request__l_aid);
            msg_read_request__tab_req_NodeId[msg_read_request__rvi] = msg_read_request__l_nid;
            msg_read_request__tab_req_AttributeId[msg_read_request__rvi] = msg_read_request__l_aid;
         }
      }
      else {
         msg_read_request__nb_ReadValue = 0;
      }
      *msg_read_request__rr = msg_read_request__init_ok;
   }
}

void msg_read_request__read_nb_ReadValue(
   t_entier4 * const msg_read_request__rr) {
   *msg_read_request__rr = msg_read_request__nb_ReadValue;
}

void msg_read_request__readall_ReadValue_Node_AttributeId(
   const constants__t_ReadValue_i msg_read_request__rvi,
   t_bool * const msg_read_request__isvalid,
   constants__t_Node_i * const msg_read_request__node,
   constants__t_AttributeId_i * const msg_read_request__aid) {
   {
      t_bool msg_read_request__l_nid_valid;
      
      address_space__readall_AddressSpace_Node(msg_read_request__tab_req_NodeId[msg_read_request__rvi],
         &msg_read_request__l_nid_valid,
         msg_read_request__node);
      *msg_read_request__aid = msg_read_request__tab_req_AttributeId[msg_read_request__rvi];
      *msg_read_request__isvalid = ((msg_read_request__l_nid_valid == true) &&
         (*msg_read_request__aid != constants__c_AttributeId_indet));
   }
}

