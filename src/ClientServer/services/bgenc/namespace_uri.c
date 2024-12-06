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

 File Name            : namespace_uri.c

 Date                 : 17/10/2024 16:34:13

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "namespace_uri.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void namespace_uri__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void namespace_uri__l_ref_get_node(
   const constants__t_Reference_i namespace_uri__p_ref,
   constants__t_Node_i * const namespace_uri__p_node,
   constants__t_NodeId_i * const namespace_uri__p_nodeId) {
   {
      constants__t_ExpandedNodeId_i namespace_uri__l_ref_target;
      t_bool namespace_uri__l_local_server;
      constants__t_NodeId_i namespace_uri__l_ref_target_NodeId;
      t_bool namespace_uri__l_isvalid;
      constants__t_Node_i namespace_uri__l_ref_target_Node;
      
      *namespace_uri__p_node = constants__c_Node_indet;
      *namespace_uri__p_nodeId = constants__c_NodeId_indet;
      address_space_bs__get_Reference_TargetNode(namespace_uri__p_ref,
         &namespace_uri__l_ref_target);
      constants__getall_conv_ExpandedNodeId_NodeId(namespace_uri__l_ref_target,
         &namespace_uri__l_local_server,
         &namespace_uri__l_ref_target_NodeId);
      if (namespace_uri__l_local_server == true) {
         address_space_bs__readall_AddressSpace_Node(namespace_uri__l_ref_target_NodeId,
            &namespace_uri__l_isvalid,
            &namespace_uri__l_ref_target_Node);
         if (namespace_uri__l_isvalid == true) {
            *namespace_uri__p_node = namespace_uri__l_ref_target_Node;
            *namespace_uri__p_nodeId = namespace_uri__l_ref_target_NodeId;
         }
      }
   }
}

void namespace_uri__ref_maybe_get_NamespaceUri(
   const constants__t_Reference_i namespace_uri__p_ref,
   constants__t_Variant_i * const namespace_uri__p_maybe_val_NamespaceUri) {
   {
      constants__t_Node_i namespace_uri__l_ref_target_Node;
      constants__t_NodeId_i namespace_uri__l_ref_target_NodeId;
      constants__t_NodeClass_i namespace_uri__l_NodeClass;
      constants__t_QualifiedName_i namespace_uri__l_browseName;
      t_bool namespace_uri__l_browseName_comparison;
      constants_statuscodes_bs__t_StatusCode_i namespace_uri__l_sc;
      constants__t_Variant_i namespace_uri__l_val;
      constants__t_RawStatusCode namespace_uri__l_val_sc;
      constants__t_Timestamp namespace_uri__l_val_ts_src;
      
      *namespace_uri__p_maybe_val_NamespaceUri = constants__c_Variant_indet;
      namespace_uri__l_NodeClass = constants__c_NodeClass_indet;
      namespace_uri__l_ref_get_node(namespace_uri__p_ref,
         &namespace_uri__l_ref_target_Node,
         &namespace_uri__l_ref_target_NodeId);
      if (namespace_uri__l_ref_target_Node != constants__c_Node_indet) {
         address_space_bs__get_NodeClass(namespace_uri__l_ref_target_Node,
            &namespace_uri__l_NodeClass);
      }
      if (namespace_uri__l_NodeClass == constants__e_ncl_Variable) {
         address_space_bs__get_BrowseName(namespace_uri__l_ref_target_Node,
            &namespace_uri__l_browseName);
         constants__is_QualifiedNames_Equal(namespace_uri__l_browseName,
            constants__c_NamespaceUri_QualifiedName,
            &namespace_uri__l_browseName_comparison);
         if (namespace_uri__l_browseName_comparison == true) {
            address_space_bs__read_AddressSpace_Raw_Node_Value_value(namespace_uri__l_ref_target_Node,
               namespace_uri__l_ref_target_NodeId,
               constants__e_aid_Value,
               &namespace_uri__l_sc,
               &namespace_uri__l_val,
               &namespace_uri__l_val_sc,
               &namespace_uri__l_val_ts_src);
            *namespace_uri__p_maybe_val_NamespaceUri = namespace_uri__l_val;
         }
      }
   }
}

