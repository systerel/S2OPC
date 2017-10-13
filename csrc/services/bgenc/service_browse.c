/******************************************************************************

 File Name            : service_browse.c

 Date                 : 13/10/2017 09:44:50

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "service_browse.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void service_browse__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void service_browse__get_SourceNode_NbRef(
   const constants__t_NodeId_i service_browse__p_src_nodeid,
   t_bool * const service_browse__p_isvalid,
   t_entier4 * const service_browse__p_nb_ref,
   constants__t_Node_i * const service_browse__p_src_node) {
   {
      t_entier4 service_browse__l_ref_index_begin;
      t_entier4 service_browse__l_ref_index_end;
      
      address_space__readall_AddressSpace_Node(service_browse__p_src_nodeid,
         service_browse__p_isvalid,
         service_browse__p_src_node);
      if (*service_browse__p_isvalid == true) {
         address_space__get_Node_RefIndexBegin(*service_browse__p_src_node,
            &service_browse__l_ref_index_begin);
         address_space__get_Node_RefIndexEnd(*service_browse__p_src_node,
            &service_browse__l_ref_index_end);
         *service_browse__p_nb_ref = (service_browse__l_ref_index_end -
            service_browse__l_ref_index_begin) +
            1;
         if (*service_browse__p_nb_ref < 0) {
            *service_browse__p_nb_ref = 0;
         }
      }
      else {
         *service_browse__p_nb_ref = 0;
      }
   }
}

void service_browse__alloc_browse_response(
   const t_entier4 service_browse__p_nb_bvi,
   t_bool * const service_browse__p_isallocated) {
   msg_browse_response_bs__malloc_browse_response(service_browse__p_nb_bvi,
      service_browse__p_isallocated);
}

void service_browse__alloc_browse_result(
   const constants__t_BrowseValue_i service_browse__p_bvi,
   const t_entier4 service_browse__p_nb_target_max,
   const t_entier4 service_browse__p_nb_target,
   t_bool * const service_browse__p_isallocated,
   t_entier4 * const service_browse__p_nb_bri) {
   if ((0 < service_browse__p_nb_target_max) &&
      (service_browse__p_nb_target_max < service_browse__p_nb_target)) {
      *service_browse__p_nb_bri = service_browse__p_nb_target_max;
   }
   else {
      *service_browse__p_nb_bri = service_browse__p_nb_target;
   }
   msg_browse_response_bs__malloc_browse_result(service_browse__p_bvi,
      *service_browse__p_nb_bri,
      service_browse__p_isallocated);
}

void service_browse__Is_RefTypes_Compatible(
   const t_bool service_browse__p_is_ref_type1,
   const constants__t_NodeId_i service_browse__p_ref_type1,
   const t_bool service_browse__p_inc_subtypes,
   const constants__t_NodeId_i service_browse__p_ref_type2,
   t_bool * const service_browse__p_ref_types_compat) {
   if (service_browse__p_is_ref_type1 == true) {
      if (service_browse__p_ref_type1 == service_browse__p_ref_type2) {
         *service_browse__p_ref_types_compat = true;
      }
      else if (service_browse__p_inc_subtypes == true) {
         constants__get_Is_SubType(service_browse__p_ref_type2,
            service_browse__p_ref_type1,
            service_browse__p_ref_types_compat);
      }
      else {
         *service_browse__p_ref_types_compat = false;
      }
   }
   else {
      *service_browse__p_ref_types_compat = true;
   }
}

void service_browse__copy_target_node_browse_result(
   const constants__t_BrowseValue_i service_browse__p_bvi,
   const constants__t_BrowseResult_i service_browse__p_bri,
   const constants__t_NodeId_i service_browse__p_RefType,
   const constants__t_ExpandedNodeId_i service_browse__p_NodeId,
   const t_bool service_browse__p_IsForward,
   t_bool * const service_browse__p_res) {
   {
      t_bool service_browse__l_isvalid;
      constants__t_NodeId_i service_browse__l_NodeId;
      constants__t_Node_i service_browse__l_node;
      constants__t_QualifiedName_i service_browse__l_BrowseName;
      constants__t_LocalizedText_i service_browse__l_DisplayName;
      constants__t_NodeClass_i service_browse__l_NodeClass;
      constants__t_ExpandedNodeId_i service_browse__l_TypeDefinition;
      
      *service_browse__p_res = true;
      msg_browse_response_bs__set_ResponseBrowse_Res_ReferenceTypeId(service_browse__p_bvi,
         service_browse__p_bri,
         service_browse__p_RefType);
      msg_browse_response_bs__set_ResponseBrowse_Res_NodeId(service_browse__p_bvi,
         service_browse__p_bri,
         service_browse__p_NodeId);
      msg_browse_response_bs__set_ResponseBrowse_Res_Forwards(service_browse__p_bvi,
         service_browse__p_bri,
         service_browse__p_IsForward);
      constants__getall_conv_ExpandedNodeId_NodeId(service_browse__p_NodeId,
         &service_browse__l_isvalid,
         &service_browse__l_NodeId);
      if (service_browse__l_isvalid == true) {
         address_space__readall_AddressSpace_Node(service_browse__l_NodeId,
            &service_browse__l_isvalid,
            &service_browse__l_node);
         if (service_browse__l_isvalid == true) {
            address_space__get_BrowseName(service_browse__l_node,
               &service_browse__l_BrowseName);
            msg_browse_response_bs__set_ResponseBrowse_Res_BrowseName(service_browse__p_bvi,
               service_browse__p_bri,
               service_browse__l_BrowseName);
            address_space__get_DisplayName(service_browse__l_node,
               &service_browse__l_DisplayName);
            msg_browse_response_bs__set_ResponseBrowse_Res_DisplayName(service_browse__p_bvi,
               service_browse__p_bri,
               service_browse__l_DisplayName);
            address_space__get_NodeClass(service_browse__l_node,
               &service_browse__l_NodeClass);
            msg_browse_response_bs__set_ResponseBrowse_Res_NodeClass(service_browse__p_bvi,
               service_browse__p_bri,
               service_browse__l_NodeClass);
            address_space__get_TypeDefinition(service_browse__l_node,
               &service_browse__l_TypeDefinition);
            msg_browse_response_bs__set_ResponseBrowse_Res_TypeDefinition(service_browse__p_bvi,
               service_browse__p_bri,
               service_browse__l_TypeDefinition);
         }
         else {
            msg_browse_response_bs__reset_ResponseBrowse_Res_BrowseName(service_browse__p_bvi,
               service_browse__p_bri);
            msg_browse_response_bs__reset_ResponseBrowse_Res_DisplayName(service_browse__p_bvi,
               service_browse__p_bri);
            msg_browse_response_bs__reset_ResponseBrowse_Res_NodeClass(service_browse__p_bvi,
               service_browse__p_bri);
            msg_browse_response_bs__reset_ResponseBrowse_Res_TypeDefinition(service_browse__p_bvi,
               service_browse__p_bri);
         }
      }
      else {
         msg_browse_response_bs__reset_ResponseBrowse_Res_BrowseName(service_browse__p_bvi,
            service_browse__p_bri);
         msg_browse_response_bs__reset_ResponseBrowse_Res_DisplayName(service_browse__p_bvi,
            service_browse__p_bri);
         msg_browse_response_bs__reset_ResponseBrowse_Res_NodeClass(service_browse__p_bvi,
            service_browse__p_bri);
         msg_browse_response_bs__reset_ResponseBrowse_Res_TypeDefinition(service_browse__p_bvi,
            service_browse__p_bri);
      }
   }
}

void service_browse__fill_continuation_point(
   const constants__t_BrowseValue_i service_browse__p_bvi,
   const t_bool service_browse__p_continue_ref,
   const constants__t_Reference_i service_browse__p_ref) {
   msg_browse_response_bs__set_ResponseBrowse_BrowseStatus(service_browse__p_bvi,
      constants__e_sc_ok);
   if (service_browse__p_continue_ref == true) {
      msg_browse_response_bs__set_ResponseBrowse_ContinuationPoint(service_browse__p_bvi,
         service_browse__p_ref);
   }
   else {
      msg_browse_response_bs__reset_ResponseBrowse_ContinuationPoint(service_browse__p_bvi);
   }
}

