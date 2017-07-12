/******************************************************************************

 File Name            : service_read.c

 Date                 : 13/07/2017 16:54:06

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
void service_read__treat_read_request(
   const constants__t_msg_i service_read__req_msg,
   const constants__t_msg_i service_read__resp_msg) {
   {
      t_entier4 service_read__ii;
      constants__t_ReadValue_i service_read__rvi;
      t_entier4 service_read__l_nb_ReadValue;
      t_bool service_read__l_isvalid;
      constants__t_Node_i service_read__l_node;
      constants__t_AttributeId_i service_read__l_aid;
      constants__t_Variant_i service_read__l_value;
      constants__t_NodeClass_i service_read__l_ncl;
      constants__t_StatusCode_i service_read__l_sc;
      t_bool service_read__l_continue;
      
      msg_read_request__read_ReadRequest(service_read__req_msg,
         &service_read__l_isvalid);
      if (service_read__l_isvalid == true) {
         msg_read_request__read_nb_ReadValue(&service_read__l_nb_ReadValue);
         msg_read_response__init_iterwrite_DataValue(service_read__l_nb_ReadValue,
            service_read__resp_msg,
            &service_read__l_continue);
         service_read__ii = 0;
         while (service_read__l_continue == true) {
            service_read__ii = service_read__ii +
               1;
            constants__read_cast_t_ReadValue(service_read__ii,
               &service_read__rvi);
            msg_read_request__readall_ReadValue_Node_AttributeId(service_read__rvi,
               &service_read__l_isvalid,
               &service_read__l_node,
               &service_read__l_aid);
            if (service_read__l_isvalid == true) {
               address_space__read_NodeClass_Attribute(service_read__l_node,
                  service_read__l_aid,
                  &service_read__l_ncl,
                  &service_read__l_value);
               if ((service_read__l_aid == constants__e_aid_Value) &&
                  (service_read__l_ncl == constants__e_ncl_Variable)) {
                  address_space__read_Value_StatusCode(service_read__l_node,
                     &service_read__l_sc);
               }
               else {
                  service_read__l_sc = constants__e_sc_ok;
               }
               msg_read_response__continue_iterwrite_DataValue(service_read__rvi,
                  service_read__l_value,
                  service_read__l_sc,
                  &service_read__l_continue);
               address_space__read_AddressSpace_free_value(service_read__l_value);
            }
            else {
               msg_read_response__continue_iterwrite_DataValue(service_read__rvi,
                  constants__c_Variant_indet,
                  constants__e_sc_nok,
                  &service_read__l_continue);
            }
         }
      }
      else {
         msg_read_response__write_response_error(constants__e_sc_nok,
            service_read__resp_msg);
      }
   }
}

