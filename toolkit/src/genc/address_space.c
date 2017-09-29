/******************************************************************************

 File Name            : address_space.c

 Date                 : 29/09/2017 10:51:56

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "address_space.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
t_bool address_space__ResponseWrite_allocated;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void address_space__INITIALISATION(void) {
   address_space__ResponseWrite_allocated = false;
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void address_space__read_NodeClass_Attribute(
   const constants__t_Node_i address_space__node,
   const constants__t_AttributeId_i address_space__aid,
   constants__t_StatusCode_i * const address_space__sc,
   constants__t_NodeClass_i * const address_space__ncl,
   constants__t_Variant_i * const address_space__val) {
   address_space_bs__get_NodeClass(address_space__node,
      address_space__ncl);
   address_space_bs__read_AddressSpace_Attribute_value(address_space__node,
      *address_space__ncl,
      address_space__aid,
      address_space__sc,
      address_space__val);
}

void address_space__alloc_write_request_responses(
   const t_entier4 address_space__nb_req,
   constants__t_StatusCode_i * const address_space__StatusCode_service) {
   *address_space__StatusCode_service = constants__e_sc_nok;
   if (address_space__nb_req <= constants__k_n_WriteResponse_max) {
      response_write_bs__alloc_write_request_responses_malloc(address_space__nb_req,
         &address_space__ResponseWrite_allocated);
      if (address_space__ResponseWrite_allocated == true) {
         *address_space__StatusCode_service = constants__e_sc_ok;
      }
   }
   else {
      address_space__ResponseWrite_allocated = false;
   }
}

void address_space__treat_write_request_WriteValues(
   const constants__t_user_i address_space__userid,
   constants__t_StatusCode_i * const address_space__StatusCode_service) {
   {
      t_entier4 address_space__l_nb_req;
      t_bool address_space__l_continue;
      constants__t_AttributeId_i address_space__l_aid;
      constants__t_NodeId_i address_space__l_nid;
      constants__t_Variant_i address_space__l_value;
      constants__t_WriteValue_i address_space__l_wvi;
      constants__t_StatusCode_i address_space__l_status;
      t_bool address_space__l_isvalid;
      
      *address_space__StatusCode_service = constants__e_sc_ok;
      service_write_decode_bs__get_nb_WriteValue(&address_space__l_nb_req);
      address_space_it__init_iter_write_request(address_space__l_nb_req,
         &address_space__l_continue);
      while (address_space__l_continue == true) {
         address_space_it__continue_iter_write_request(&address_space__l_continue,
            &address_space__l_wvi);
         service_write_decode_bs__getall_WriteValue(address_space__l_wvi,
            &address_space__l_isvalid,
            &address_space__l_nid,
            &address_space__l_aid,
            &address_space__l_value);
         address_space__treat_write_1(address_space__l_isvalid,
            address_space__l_nid,
            address_space__l_aid,
            address_space__l_value,
            address_space__userid,
            &address_space__l_status);
         response_write_bs__set_ResponseWrite_StatusCode(address_space__l_wvi,
            address_space__l_status);
      }
   }
}

void address_space__dealloc_write_request_responses(void) {
   address_space__ResponseWrite_allocated = false;
   response_write_bs__reset_ResponseWrite();
}

void address_space__treat_write_1(
   const t_bool address_space__isvalid,
   const constants__t_NodeId_i address_space__nid,
   const constants__t_AttributeId_i address_space__aid,
   const constants__t_Variant_i address_space__value,
   const constants__t_user_i address_space__uid,
   constants__t_StatusCode_i * const address_space__sc) {
   if (address_space__isvalid == true) {
      address_space__treat_write_2(address_space__nid,
         address_space__aid,
         address_space__value,
         address_space__sc);
   }
   else {
      *address_space__sc = constants__e_sc_nok;
   }
}

void address_space__treat_write_2(
   const constants__t_NodeId_i address_space__nid,
   const constants__t_AttributeId_i address_space__aid,
   const constants__t_Variant_i address_space__value,
   constants__t_StatusCode_i * const address_space__sc) {
   {
      t_bool address_space__l_isvalid;
      constants__t_Node_i address_space__l_node;
      constants__t_NodeClass_i address_space__l_ncl;
      
      *address_space__sc = constants__e_sc_nok;
      address_space_bs__readall_AddressSpace_Node(address_space__nid,
         &address_space__l_isvalid,
         &address_space__l_node);
      if (address_space__l_isvalid == true) {
         if (address_space__aid == constants__e_aid_Value) {
            address_space_bs__get_NodeClass(address_space__l_node,
               &address_space__l_ncl);
            if (address_space__l_ncl == constants__e_ncl_Variable) {
               address_space_bs__set_Value(address_space__l_node,
                  address_space__value);
               *address_space__sc = constants__e_sc_ok;
            }
         }
      }
   }
}

