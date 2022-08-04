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

 File Name            : browse_treatment_1.c

 Date                 : 04/08/2022 14:53:00

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "browse_treatment_1.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void browse_treatment_1__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void browse_treatment_1__getall_SourceNode_NbRef(
   const constants__t_NodeId_i browse_treatment_1__p_src_nodeid,
   t_bool * const browse_treatment_1__p_isvalid,
   t_entier4 * const browse_treatment_1__p_nb_ref,
   constants__t_Node_i * const browse_treatment_1__p_src_node) {
   address_space_itf__readall_AddressSpace_Node(browse_treatment_1__p_src_nodeid,
      browse_treatment_1__p_isvalid,
      browse_treatment_1__p_src_node);
   if (*browse_treatment_1__p_isvalid == true) {
      address_space_itf__get_Node_RefIndexEnd(*browse_treatment_1__p_src_node,
         browse_treatment_1__p_nb_ref);
   }
   else {
      *browse_treatment_1__p_nb_ref = 0;
   }
}

void browse_treatment_1__Is_RefTypes_Compatible(
   const t_bool browse_treatment_1__p_is_ref_type1,
   const constants__t_NodeId_i browse_treatment_1__p_ref_type1,
   const t_bool browse_treatment_1__p_inc_subtypes,
   const constants__t_NodeId_i browse_treatment_1__p_ref_type2,
   t_bool * const browse_treatment_1__p_ref_types_compat) {
   {
      t_bool browse_treatment_1__l_node_ids_equal;
      
      if (browse_treatment_1__p_is_ref_type1 == true) {
         address_space_itf__is_NodeId_equal(browse_treatment_1__p_ref_type1,
            browse_treatment_1__p_ref_type2,
            &browse_treatment_1__l_node_ids_equal);
         if (browse_treatment_1__l_node_ids_equal == true) {
            *browse_treatment_1__p_ref_types_compat = true;
         }
         else if (browse_treatment_1__p_inc_subtypes == true) {
            address_space_itf__is_transitive_subtype(browse_treatment_1__p_ref_type2,
               browse_treatment_1__p_ref_type1,
               browse_treatment_1__p_ref_types_compat);
         }
         else {
            *browse_treatment_1__p_ref_types_compat = false;
         }
      }
      else {
         *browse_treatment_1__p_ref_types_compat = true;
      }
   }
}

void browse_treatment_1__get_optional_fields_ReferenceDescription(
   const constants__t_ExpandedNodeId_i browse_treatment_1__p_TargetNodeId,
   constants__t_QualifiedName_i * const browse_treatment_1__p_BrowseName,
   constants__t_LocalizedText_i * const browse_treatment_1__p_DisplayName,
   constants__t_NodeClass_i * const browse_treatment_1__p_NodeClass,
   constants__t_ExpandedNodeId_i * const browse_treatment_1__p_TypeDefinition) {
   {
      t_bool browse_treatment_1__l_local_server;
      constants__t_NodeId_i browse_treatment_1__l_NodeId;
      t_bool browse_treatment_1__l_isvalid;
      constants__t_Node_i browse_treatment_1__l_node;
      
      constants__getall_conv_ExpandedNodeId_NodeId(browse_treatment_1__p_TargetNodeId,
         &browse_treatment_1__l_local_server,
         &browse_treatment_1__l_NodeId);
      if (browse_treatment_1__l_local_server == true) {
         address_space_itf__readall_AddressSpace_Node(browse_treatment_1__l_NodeId,
            &browse_treatment_1__l_isvalid,
            &browse_treatment_1__l_node);
         if (browse_treatment_1__l_isvalid == true) {
            address_space_itf__get_BrowseName(browse_treatment_1__l_node,
               browse_treatment_1__p_BrowseName);
            address_space_itf__get_DisplayName(browse_treatment_1__l_node,
               browse_treatment_1__p_DisplayName);
            address_space_itf__get_NodeClass(browse_treatment_1__l_node,
               browse_treatment_1__p_NodeClass);
            address_space_itf__get_TypeDefinition(browse_treatment_1__l_node,
               browse_treatment_1__p_TypeDefinition);
         }
         else {
            *browse_treatment_1__p_BrowseName = constants__c_QualifiedName_indet;
            *browse_treatment_1__p_DisplayName = constants__c_LocalizedText_indet;
            *browse_treatment_1__p_NodeClass = constants__c_NodeClass_indet;
            *browse_treatment_1__p_TypeDefinition = constants__c_ExpandedNodeId_indet;
         }
      }
      else {
         *browse_treatment_1__p_BrowseName = constants__c_QualifiedName_indet;
         *browse_treatment_1__p_DisplayName = constants__c_LocalizedText_indet;
         *browse_treatment_1__p_NodeClass = constants__c_NodeClass_indet;
         *browse_treatment_1__p_TypeDefinition = constants__c_ExpandedNodeId_indet;
      }
   }
}

