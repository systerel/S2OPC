/*
 * Licensed to Systerel under one or more contributor license
 * agreements. See the NOTICE file distributed with this work
 * for additional information regarding copyright ownership.
 * Systerel licenses this file to you under the Apache
 * License, Version 2.0 (the "License"); you may not use this
 * file except in compliance with the License. You may obtain
 * a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

/******************************************************************************

 File Name            : service_add_nodes_1.c

 Date                 : 30/07/2025 08:56:23

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "service_add_nodes_1.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void service_add_nodes_1__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void service_add_nodes_1__has_NamespaceIndex(
   const constants__t_NamespaceIdx service_add_nodes_1__p_idx,
   t_bool * const service_add_nodes_1__bres) {
   address_space_namespaces_indexes__may_initialize_max_namespace_idx();
   address_space_namespaces_indexes__has_NamespaceIndex_index(service_add_nodes_1__p_idx,
      service_add_nodes_1__bres);
}

void service_add_nodes_1__check_add_nodes_item_params_parent_nid(
   const constants__t_ExpandedNodeId_i service_add_nodes_1__p_parentNid,
   constants_statuscodes_bs__t_StatusCode_i * const service_add_nodes_1__sc_operation) {
   {
      t_bool service_add_nodes_1__l_local_server_exp_node_id;
      constants__t_NodeId_i service_add_nodes_1__l_node_id;
      t_bool service_add_nodes_1__l_node_exists;
      constants__t_Node_i service_add_nodes_1__l_node;
      
      constants__getall_conv_ExpandedNodeId_NodeId(service_add_nodes_1__p_parentNid,
         &service_add_nodes_1__l_local_server_exp_node_id,
         &service_add_nodes_1__l_node_id);
      if (service_add_nodes_1__l_local_server_exp_node_id == true) {
         call_method_mgr__readall_AddressSpace_Node(service_add_nodes_1__l_node_id,
            &service_add_nodes_1__l_node_exists,
            &service_add_nodes_1__l_node);
         if (service_add_nodes_1__l_node_exists == true) {
            *service_add_nodes_1__sc_operation = constants_statuscodes_bs__e_sc_ok;
         }
         else {
            *service_add_nodes_1__sc_operation = constants_statuscodes_bs__e_sc_bad_parent_node_id_invalid;
         }
      }
      else {
         *service_add_nodes_1__sc_operation = constants_statuscodes_bs__e_sc_bad_parent_node_id_invalid;
      }
   }
}

void service_add_nodes_1__check_add_nodes_item_params_ref_type(
   const constants__t_NodeId_i service_add_nodes_1__p_refTypeId,
   const constants_statuscodes_bs__t_StatusCode_i service_add_nodes_1__p_sc_operation,
   constants_statuscodes_bs__t_StatusCode_i * const service_add_nodes_1__sc_operation) {
   {
      t_bool service_add_nodes_1__l_bres;
      
      *service_add_nodes_1__sc_operation = service_add_nodes_1__p_sc_operation;
      if (service_add_nodes_1__p_sc_operation == constants_statuscodes_bs__e_sc_ok) {
         call_method_mgr__is_valid_ReferenceTypeId(service_add_nodes_1__p_refTypeId,
            &service_add_nodes_1__l_bres);
         if (service_add_nodes_1__l_bres == false) {
            *service_add_nodes_1__sc_operation = constants_statuscodes_bs__e_sc_bad_reference_type_id_invalid;
         }
         else {
            call_method_mgr__is_transitive_subtype(service_add_nodes_1__p_refTypeId,
               constants__c_HierarchicalReferences_Type_NodeId,
               &service_add_nodes_1__l_bres);
            if (service_add_nodes_1__l_bres == false) {
               *service_add_nodes_1__sc_operation = constants_statuscodes_bs__e_sc_bad_reference_type_id_invalid;
            }
         }
      }
   }
}

void service_add_nodes_1__check_add_nodes_item_params_type_def(
   const constants__t_NodeClass_i service_add_nodes_1__p_nodeClass,
   const constants__t_ExpandedNodeId_i service_add_nodes_1__p_typeDefId,
   const constants_statuscodes_bs__t_StatusCode_i service_add_nodes_1__p_sc_operation,
   constants_statuscodes_bs__t_StatusCode_i * const service_add_nodes_1__sc_operation) {
   {
      t_bool service_add_nodes_1__l_local_server_exp_node_id;
      constants__t_NodeId_i service_add_nodes_1__l_node_id;
      t_bool service_add_nodes_1__l_node_exists;
      constants__t_Node_i service_add_nodes_1__l_node;
      constants__t_NodeClass_i service_add_nodes_1__l_node_class;
      
      *service_add_nodes_1__sc_operation = service_add_nodes_1__p_sc_operation;
      if ((*service_add_nodes_1__sc_operation == constants_statuscodes_bs__e_sc_ok) &&
         (service_add_nodes_1__p_typeDefId != constants__c_ExpandedNodeId_indet)) {
         constants__getall_conv_ExpandedNodeId_NodeId(service_add_nodes_1__p_typeDefId,
            &service_add_nodes_1__l_local_server_exp_node_id,
            &service_add_nodes_1__l_node_id);
         if (service_add_nodes_1__l_local_server_exp_node_id == true) {
            call_method_mgr__readall_AddressSpace_Node(service_add_nodes_1__l_node_id,
               &service_add_nodes_1__l_node_exists,
               &service_add_nodes_1__l_node);
            if (service_add_nodes_1__l_node_exists == true) {
               call_method_mgr__get_NodeClass(service_add_nodes_1__l_node,
                  &service_add_nodes_1__l_node_class);
               if ((service_add_nodes_1__l_node_class == constants__e_ncl_VariableType) &&
                  (service_add_nodes_1__p_nodeClass == constants__e_ncl_Variable)) {
                  ;
               }
               else if ((service_add_nodes_1__l_node_class == constants__e_ncl_ObjectType) &&
                  (service_add_nodes_1__p_nodeClass == constants__e_ncl_Object)) {
                  ;
               }
               else {
                  *service_add_nodes_1__sc_operation = constants_statuscodes_bs__e_sc_bad_type_definition_invalid;
               }
            }
            else {
               *service_add_nodes_1__sc_operation = constants_statuscodes_bs__e_sc_bad_type_definition_invalid;
            }
         }
      }
   }
}

void service_add_nodes_1__check_add_nodes_item_params_req_node_id(
   const constants__t_ExpandedNodeId_i service_add_nodes_1__p_reqNodeId,
   const constants_statuscodes_bs__t_StatusCode_i service_add_nodes_1__p_sc_operation,
   constants_statuscodes_bs__t_StatusCode_i * const service_add_nodes_1__sc_operation,
   constants__t_NodeId_i * const service_add_nodes_1__new_nid) {
   {
      t_bool service_add_nodes_1__l_local_server_exp_node_id;
      constants__t_NodeId_i service_add_nodes_1__l_node_id;
      constants__t_NamespaceIdx service_add_nodes_1__l_ns_index;
      t_bool service_add_nodes_1__l_ns_index_valid;
      t_bool service_add_nodes_1__l_node_exists;
      constants__t_Node_i service_add_nodes_1__l_node;
      t_bool service_add_nodes_1__l_bres;
      
      service_add_nodes_1__l_bres = false;
      *service_add_nodes_1__new_nid = constants__c_NodeId_indet;
      *service_add_nodes_1__sc_operation = service_add_nodes_1__p_sc_operation;
      if (*service_add_nodes_1__sc_operation == constants_statuscodes_bs__e_sc_ok) {
         if (service_add_nodes_1__p_reqNodeId != constants__c_ExpandedNodeId_indet) {
            constants__getall_conv_ExpandedNodeId_NodeId(service_add_nodes_1__p_reqNodeId,
               &service_add_nodes_1__l_local_server_exp_node_id,
               &service_add_nodes_1__l_node_id);
            if (service_add_nodes_1__l_local_server_exp_node_id == true) {
               call_method_mgr__readall_AddressSpace_Node(service_add_nodes_1__l_node_id,
                  &service_add_nodes_1__l_node_exists,
                  &service_add_nodes_1__l_node);
               constants__get_NodeId_NamespaceIndex(service_add_nodes_1__l_node_id,
                  &service_add_nodes_1__l_ns_index);
               service_add_nodes_1__has_NamespaceIndex(service_add_nodes_1__l_ns_index,
                  &service_add_nodes_1__l_ns_index_valid);
               if ((service_add_nodes_1__l_node_exists == false) &&
                  (service_add_nodes_1__l_ns_index_valid == true)) {
                  node_id_pointer_bs__copy_node_id_pointer_content(service_add_nodes_1__l_node_id,
                     &service_add_nodes_1__l_bres,
                     service_add_nodes_1__new_nid);
                  if (service_add_nodes_1__l_bres == false) {
                     *service_add_nodes_1__sc_operation = constants_statuscodes_bs__e_sc_bad_out_of_memory;
                  }
               }
               else if (service_add_nodes_1__l_ns_index_valid == false) {
                  *service_add_nodes_1__sc_operation = constants_statuscodes_bs__e_sc_bad_node_id_rejected;
               }
               else {
                  *service_add_nodes_1__sc_operation = constants_statuscodes_bs__e_sc_bad_node_id_exists;
               }
            }
            else {
               *service_add_nodes_1__sc_operation = constants_statuscodes_bs__e_sc_bad_node_id_rejected;
            }
         }
         else {
            call_method_mgr__gen_fresh_NodeId(&service_add_nodes_1__l_bres,
               service_add_nodes_1__new_nid);
            if (service_add_nodes_1__l_bres == false) {
               *service_add_nodes_1__sc_operation = constants_statuscodes_bs__e_sc_bad_out_of_memory;
            }
         }
      }
   }
}

void service_add_nodes_1__check_add_nodes_item_params(
   const constants__t_ExpandedNodeId_i service_add_nodes_1__p_parentNid,
   const constants__t_NodeId_i service_add_nodes_1__p_refTypeId,
   const constants__t_ExpandedNodeId_i service_add_nodes_1__p_reqNodeId,
   const constants__t_NodeClass_i service_add_nodes_1__p_nodeClass,
   const constants__t_ExpandedNodeId_i service_add_nodes_1__p_typeDefId,
   constants_statuscodes_bs__t_StatusCode_i * const service_add_nodes_1__sc_operation,
   constants__t_NodeId_i * const service_add_nodes_1__new_nid) {
   {
      constants_statuscodes_bs__t_StatusCode_i service_add_nodes_1__l_sc_operation;
      
      service_add_nodes_1__check_add_nodes_item_params_parent_nid(service_add_nodes_1__p_parentNid,
         &service_add_nodes_1__l_sc_operation);
      service_add_nodes_1__check_add_nodes_item_params_ref_type(service_add_nodes_1__p_refTypeId,
         service_add_nodes_1__l_sc_operation,
         service_add_nodes_1__sc_operation);
      service_add_nodes_1__check_add_nodes_item_params_type_def(service_add_nodes_1__p_nodeClass,
         service_add_nodes_1__p_typeDefId,
         *service_add_nodes_1__sc_operation,
         &service_add_nodes_1__l_sc_operation);
      service_add_nodes_1__check_add_nodes_item_params_req_node_id(service_add_nodes_1__p_reqNodeId,
         service_add_nodes_1__l_sc_operation,
         service_add_nodes_1__sc_operation,
         service_add_nodes_1__new_nid);
   }
}

void service_add_nodes_1__treat_add_nodes_item(
   const constants__t_ExpandedNodeId_i service_add_nodes_1__p_parentExpNid,
   const constants__t_NodeId_i service_add_nodes_1__p_refTypeId,
   const constants__t_ExpandedNodeId_i service_add_nodes_1__p_reqExpNodeId,
   const constants__t_QualifiedName_i service_add_nodes_1__p_browseName,
   const constants__t_NodeClass_i service_add_nodes_1__p_nodeClass,
   const constants__t_NodeAttributes_i service_add_nodes_1__p_nodeAttributes,
   const constants__t_ExpandedNodeId_i service_add_nodes_1__p_typeDefId,
   constants_statuscodes_bs__t_StatusCode_i * const service_add_nodes_1__sc_operation,
   constants__t_NodeId_i * const service_add_nodes_1__new_nodeId) {
   {
      constants_statuscodes_bs__t_StatusCode_i service_add_nodes_1__l_sc;
      constants__t_NodeId_i service_add_nodes_1__l_new_nid;
      
      service_add_nodes_1__check_add_nodes_item_params(service_add_nodes_1__p_parentExpNid,
         service_add_nodes_1__p_refTypeId,
         service_add_nodes_1__p_reqExpNodeId,
         service_add_nodes_1__p_nodeClass,
         service_add_nodes_1__p_typeDefId,
         &service_add_nodes_1__l_sc,
         &service_add_nodes_1__l_new_nid);
      if (service_add_nodes_1__l_sc == constants_statuscodes_bs__e_sc_ok) {
         call_method_mgr__addNode_AddressSpace(service_add_nodes_1__p_parentExpNid,
            service_add_nodes_1__p_refTypeId,
            service_add_nodes_1__l_new_nid,
            service_add_nodes_1__p_browseName,
            service_add_nodes_1__p_nodeClass,
            service_add_nodes_1__p_nodeAttributes,
            service_add_nodes_1__p_typeDefId,
            &service_add_nodes_1__l_sc);
      }
      *service_add_nodes_1__sc_operation = service_add_nodes_1__l_sc;
      if (*service_add_nodes_1__sc_operation == constants_statuscodes_bs__e_sc_ok) {
         *service_add_nodes_1__new_nodeId = service_add_nodes_1__l_new_nid;
      }
      else {
         node_id_pointer_bs__free_node_id_pointer(service_add_nodes_1__l_new_nid);
         *service_add_nodes_1__new_nodeId = constants__c_NodeId_indet;
      }
   }
}

