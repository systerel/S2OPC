/******************************************************************************

 File Name            : service_read.c

 Date                 : 28/07/2017 17:53:12

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "service_read.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void service_read__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void service_read__fill_read_response_1(
   const constants__t_msg_i service_read__p_resp_msg,
   const t_bool service_read__p_isvalid,
   const constants__t_NodeId_i service_read__p_nid,
   const constants__t_AttributeId_i service_read__p_aid,
   const constants__t_ReadValue_i service_read__p_rvi) {
   {
      t_bool service_read__l_is_valid;
      constants__t_Node_i service_read__l_node;
      constants__t_NodeClass_i service_read__l_ncl;
      constants__t_Variant_i service_read__l_value;
      constants__t_StatusCode_i service_read__l_sc;
      
      if (service_read__p_isvalid == true) {
         address_space__readall_AddressSpace_Node(service_read__p_nid,
            &service_read__l_is_valid,
            &service_read__l_node);
         if (service_read__l_is_valid == true) {
            address_space__read_NodeClass_Attribute(service_read__l_node,
               service_read__p_aid,
               &service_read__l_ncl,
               &service_read__l_value);
            if ((service_read__p_aid == constants__e_aid_Value) &&
               (service_read__l_ncl == constants__e_ncl_Variable)) {
               address_space__get_Value_StatusCode(service_read__l_node,
                  &service_read__l_sc);
            }
            else {
               service_read__l_sc = constants__e_sc_ok;
            }
            msg_read_response_bs__set_read_response(service_read__p_resp_msg,
               service_read__p_rvi,
               service_read__l_value,
               service_read__l_sc);
            address_space__read_AddressSpace_free_value(service_read__l_value);
         }
         else {
            msg_read_response_bs__set_read_response(service_read__p_resp_msg,
               service_read__p_rvi,
               constants__c_Variant_indet,
               constants__e_sc_nok);
         }
      }
      else {
         msg_read_response_bs__set_read_response(service_read__p_resp_msg,
            service_read__p_rvi,
            constants__c_Variant_indet,
            constants__e_sc_nok);
      }
   }
}

void service_read__fill_read_response(
   const constants__t_msg_i service_read__req_msg,
   const constants__t_msg_i service_read__resp_msg) {
   {
      t_entier4 service_read__l_nb_ReadValue;
      t_bool service_read__l_continue;
      t_bool service_read__l_isvalid;
      constants__t_ReadValue_i service_read__l_rvi;
      constants__t_NodeId_i service_read__l_nid;
      constants__t_AttributeId_i service_read__l_aid;
      
      msg_read_request__get_nb_ReadValue(&service_read__l_nb_ReadValue);
      service_read_it__init_iter_write_request(service_read__l_nb_ReadValue,
         &service_read__l_continue);
      while (service_read__l_continue == true) {
         service_read_it__continue_iter_write_request(&service_read__l_continue,
            &service_read__l_rvi);
         msg_read_request__getall_ReadValue_NodeId_AttributeId(service_read__req_msg,
            service_read__l_rvi,
            &service_read__l_isvalid,
            &service_read__l_nid,
            &service_read__l_aid);
         service_read__fill_read_response_1(service_read__resp_msg,
            service_read__l_isvalid,
            service_read__l_nid,
            service_read__l_aid,
            service_read__l_rvi);
      }
   }
}

