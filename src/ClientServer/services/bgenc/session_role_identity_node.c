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

 File Name            : session_role_identity_node.c

 Date                 : 11/09/2024 10:08:08

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "session_role_identity_node.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void session_role_identity_node__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void session_role_identity_node__l_check_node_NodeClass_and_BrowseName(
   const constants__t_Node_i session_role_identity_node__p_node,
   t_bool * const session_role_identity_node__p_bres) {
   {
      constants__t_NodeClass_i session_role_identity_node__l_NodeClass;
      constants__t_QualifiedName_i session_role_identity_node__l_browseName;
      t_bool session_role_identity_node__l_browseName_comparison;
      
      *session_role_identity_node__p_bres = false;
      address_space_itf__get_NodeClass(session_role_identity_node__p_node,
         &session_role_identity_node__l_NodeClass);
      if (session_role_identity_node__l_NodeClass == constants__e_ncl_Variable) {
         address_space_itf__get_BrowseName(session_role_identity_node__p_node,
            &session_role_identity_node__l_browseName);
         constants__is_QualifiedNames_Equal(session_role_identity_node__l_browseName,
            constants__c_Identities_QualifiedName,
            &session_role_identity_node__l_browseName_comparison);
         if (session_role_identity_node__l_browseName_comparison == true) {
            *session_role_identity_node__p_bres = true;
         }
      }
   }
}

void session_role_identity_node__l_check_ref_isForward_and_RefTypeProperty(
   const constants__t_Reference_i session_role_identity_node__p_ref,
   t_bool * const session_role_identity_node__p_bres) {
   {
      t_bool session_role_identity_node__l_IsForward;
      constants__t_NodeId_i session_role_identity_node__l_RefType_NodeId;
      t_bool session_role_identity_node__l_nodeIdsEqual;
      
      *session_role_identity_node__p_bres = false;
      address_space_itf__get_Reference_IsForward(session_role_identity_node__p_ref,
         &session_role_identity_node__l_IsForward);
      if (session_role_identity_node__l_IsForward == true) {
         address_space_itf__get_Reference_ReferenceType(session_role_identity_node__p_ref,
            &session_role_identity_node__l_RefType_NodeId);
         address_space_itf__is_NodeId_equal(session_role_identity_node__l_RefType_NodeId,
            constants__c_HasPropertyType_NodeId,
            &session_role_identity_node__l_nodeIdsEqual);
         if (session_role_identity_node__l_nodeIdsEqual == true) {
            *session_role_identity_node__p_bres = true;
         }
      }
   }
}

void session_role_identity_node__l_ref_get_node(
   const constants__t_Reference_i session_role_identity_node__p_ref,
   constants__t_Node_i * const session_role_identity_node__p_node,
   constants__t_NodeId_i * const session_role_identity_node__p_nodeId) {
   {
      constants__t_ExpandedNodeId_i session_role_identity_node__l_ref_target;
      t_bool session_role_identity_node__l_local_server;
      constants__t_NodeId_i session_role_identity_node__l_ref_target_NodeId;
      t_bool session_role_identity_node__l_isvalid;
      constants__t_Node_i session_role_identity_node__l_ref_target_Node;
      
      *session_role_identity_node__p_node = constants__c_Node_indet;
      *session_role_identity_node__p_nodeId = constants__c_NodeId_indet;
      address_space_itf__get_Reference_TargetNode(session_role_identity_node__p_ref,
         &session_role_identity_node__l_ref_target);
      constants__getall_conv_ExpandedNodeId_NodeId(session_role_identity_node__l_ref_target,
         &session_role_identity_node__l_local_server,
         &session_role_identity_node__l_ref_target_NodeId);
      address_space_itf__readall_AddressSpace_Node(session_role_identity_node__l_ref_target_NodeId,
         &session_role_identity_node__l_isvalid,
         &session_role_identity_node__l_ref_target_Node);
      if (session_role_identity_node__l_isvalid == true) {
         *session_role_identity_node__p_node = session_role_identity_node__l_ref_target_Node;
         *session_role_identity_node__p_nodeId = session_role_identity_node__l_ref_target_NodeId;
      }
   }
}

void session_role_identity_node__ref_maybe_get_Identity(
   const constants__t_Reference_i session_role_identity_node__p_ref,
   constants__t_Node_i * const session_role_identity_node__p_maybe_node_Identity,
   constants__t_NodeId_i * const session_role_identity_node__p_maybe_nodeId_Identity) {
   {
      t_bool session_role_identity_node__l_bres;
      constants__t_Node_i session_role_identity_node__l_maybe_identity_node;
      constants__t_NodeId_i session_role_identity_node__l_maybe_identity_nodeId;
      
      *session_role_identity_node__p_maybe_node_Identity = constants__c_Node_indet;
      *session_role_identity_node__p_maybe_nodeId_Identity = constants__c_NodeId_indet;
      session_role_identity_node__l_check_ref_isForward_and_RefTypeProperty(session_role_identity_node__p_ref,
         &session_role_identity_node__l_bres);
      if (session_role_identity_node__l_bres == true) {
         session_role_identity_node__l_ref_get_node(session_role_identity_node__p_ref,
            &session_role_identity_node__l_maybe_identity_node,
            &session_role_identity_node__l_maybe_identity_nodeId);
         session_role_identity_node__l_check_node_NodeClass_and_BrowseName(session_role_identity_node__l_maybe_identity_node,
            &session_role_identity_node__l_bres);
         if (session_role_identity_node__l_bres == true) {
            *session_role_identity_node__p_maybe_node_Identity = session_role_identity_node__l_maybe_identity_node;
            *session_role_identity_node__p_maybe_nodeId_Identity = session_role_identity_node__l_maybe_identity_nodeId;
         }
      }
   }
}

