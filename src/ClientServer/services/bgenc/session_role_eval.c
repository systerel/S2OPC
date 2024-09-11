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

 File Name            : session_role_eval.c

 Date                 : 11/09/2024 10:08:07

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "session_role_eval.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void session_role_eval__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void session_role_eval__l_check_ref_isForward_and_RefTypeComponent(
   const constants__t_Reference_i session_role_eval__p_ref,
   t_bool * const session_role_eval__p_bres) {
   {
      t_bool session_role_eval__l_IsForward;
      constants__t_NodeId_i session_role_eval__l_RefType_NodeId;
      t_bool session_role_eval__l_nodeIdsEqual;
      
      *session_role_eval__p_bres = false;
      address_space_itf__get_Reference_IsForward(session_role_eval__p_ref,
         &session_role_eval__l_IsForward);
      if (session_role_eval__l_IsForward == true) {
         address_space_itf__get_Reference_ReferenceType(session_role_eval__p_ref,
            &session_role_eval__l_RefType_NodeId);
         address_space_itf__is_NodeId_equal(session_role_eval__l_RefType_NodeId,
            constants__c_HasComponentType_NodeId,
            &session_role_eval__l_nodeIdsEqual);
         if (session_role_eval__l_nodeIdsEqual == true) {
            *session_role_eval__p_bres = true;
         }
      }
   }
}

void session_role_eval__l_check_node_NodeClass_and_TypeDef(
   const constants__t_Node_i session_role_eval__p_node,
   t_bool * const session_role_eval__p_bres) {
   {
      constants__t_NodeClass_i session_role_eval__l_NodeClass;
      constants__t_ExpandedNodeId_i session_role_eval__l_typeDefinition;
      t_bool session_role_eval__l_local_server;
      constants__t_NodeId_i session_role_eval__l_NodeId;
      t_bool session_role_eval__l_nodeIdsEqual;
      
      *session_role_eval__p_bres = false;
      address_space_itf__get_NodeClass(session_role_eval__p_node,
         &session_role_eval__l_NodeClass);
      if (session_role_eval__l_NodeClass == constants__e_ncl_Object) {
         address_space_itf__get_TypeDefinition(session_role_eval__p_node,
            &session_role_eval__l_typeDefinition);
         constants__getall_conv_ExpandedNodeId_NodeId(session_role_eval__l_typeDefinition,
            &session_role_eval__l_local_server,
            &session_role_eval__l_NodeId);
         address_space_itf__is_NodeId_equal(session_role_eval__l_NodeId,
            constants__c_RoleType_NodeId,
            &session_role_eval__l_nodeIdsEqual);
         if (session_role_eval__l_nodeIdsEqual == true) {
            *session_role_eval__p_bres = true;
         }
      }
   }
}

void session_role_eval__role_eval_user(
   const constants__t_user_i session_role_eval__p_user,
   const constants__t_LocaleIds_i session_role_eval__p_locales,
   const constants__t_Node_i session_role_eval__p_role_node,
   t_bool * const session_role_eval__p_bres) {
   {
      constants__t_Node_i session_role_eval__l_identities_node;
      constants__t_NodeId_i session_role_eval__l_identities_nodeId;
      t_bool session_role_eval__l_continue;
      constants__t_Reference_i session_role_eval__l_ref;
      constants__t_Node_i session_role_eval__l_maybe_identity_node;
      constants__t_NodeId_i session_role_eval__l_maybe_identity_nodeId;
      constants_statuscodes_bs__t_StatusCode_i session_role_eval__l_sc;
      constants__t_Variant_i session_role_eval__l_val;
      constants__t_RawStatusCode session_role_eval__l_val_sc;
      constants__t_Timestamp session_role_eval__l_val_ts_src;
      constants__t_Timestamp session_role_eval__l_val_ts_srv;
      constants__t_IndexRange_i session_role_eval__l_index_range;
      t_entier4 session_role_eval__l_nb_identities;
      t_bool session_role_eval__l_bres;
      t_entier4 session_role_eval__l_identityIdx;
      constants__t_Identity_i session_role_eval__l_identity;
      
      *session_role_eval__p_bres = false;
      session_role_eval__l_identities_node = constants__c_Node_indet;
      session_role_eval__l_identities_nodeId = constants__c_NodeId_indet;
      role_references_it__init_iter_role_references(session_role_eval__p_role_node,
         &session_role_eval__l_continue);
      while (session_role_eval__l_continue == true) {
         role_references_it__continue_iter_role_references(&session_role_eval__l_continue,
            &session_role_eval__l_ref);
         session_role_identity_node__ref_maybe_get_Identity(session_role_eval__l_ref,
            &session_role_eval__l_maybe_identity_node,
            &session_role_eval__l_maybe_identity_nodeId);
         if (session_role_eval__l_maybe_identity_node != constants__c_Node_indet) {
            session_role_eval__l_identities_node = session_role_eval__l_maybe_identity_node;
            session_role_eval__l_identities_nodeId = session_role_eval__l_maybe_identity_nodeId;
         }
      }
      if ((session_role_eval__l_identities_node != constants__c_Node_indet) &&
         (session_role_eval__l_identities_nodeId != constants__c_NodeId_indet)) {
         session_role_eval__l_index_range = constants__c_IndexRange_indet;
         address_space_itf__read_Node_Attribute(session_role_eval__p_user,
            session_role_eval__p_locales,
            session_role_eval__l_identities_node,
            session_role_eval__l_identities_nodeId,
            constants__e_aid_Value,
            session_role_eval__l_index_range,
            &session_role_eval__l_sc,
            &session_role_eval__l_val,
            &session_role_eval__l_val_sc,
            &session_role_eval__l_val_ts_src,
            &session_role_eval__l_val_ts_srv);
         session_role_identities_bs__read_variant_nb_identities(session_role_eval__l_val,
            session_role_eval__l_identities_node,
            &session_role_eval__l_nb_identities);
         session_role_identities_it__init_iter_identities(session_role_eval__l_nb_identities,
            &session_role_eval__l_continue);
         while (session_role_eval__l_continue == true) {
            session_role_eval__l_bres = false;
            session_role_identities_it__continue_iter_identities(&session_role_eval__l_continue,
               &session_role_eval__l_identityIdx);
            session_role_identities_bs__read_variant_identity(session_role_eval__l_val,
               session_role_eval__l_identityIdx,
               &session_role_eval__l_identity);
            session_role_identity_eval__user_and_identity_match(session_role_eval__p_user,
               session_role_eval__l_identity,
               &session_role_eval__l_bres);
            if (session_role_eval__l_bres == true) {
               *session_role_eval__p_bres = true;
            }
         }
         address_space_itf__read_AddressSpace_free_variant(session_role_eval__l_val);
      }
   }
}

void session_role_eval__is_ref_role(
   const constants__t_Reference_i session_role_eval__p_ref,
   t_bool * const session_role_eval__p_bres,
   constants__t_Node_i * const session_role_eval__p_maybe_role_node,
   constants__t_NodeId_i * const session_role_eval__p_maybe_role_nodeId) {
   {
      t_bool session_role_eval__l_bValidRoleRef;
      constants__t_ExpandedNodeId_i session_role_eval__l_roleSet_Reference_ExpandedNodeId;
      t_bool session_role_eval__l_local_server;
      constants__t_NodeId_i session_role_eval__l_roleSet_Reference_NodeId;
      t_bool session_role_eval__l_isvalid;
      constants__t_Node_i session_role_eval__l_roleSet_Reference_Node;
      t_bool session_role_eval__l_bValidRoleNode;
      
      *session_role_eval__p_bres = false;
      *session_role_eval__p_maybe_role_node = constants__c_Node_indet;
      *session_role_eval__p_maybe_role_nodeId = constants__c_NodeId_indet;
      session_role_eval__l_check_ref_isForward_and_RefTypeComponent(session_role_eval__p_ref,
         &session_role_eval__l_bValidRoleRef);
      if (session_role_eval__l_bValidRoleRef == true) {
         address_space_itf__get_Reference_TargetNode(session_role_eval__p_ref,
            &session_role_eval__l_roleSet_Reference_ExpandedNodeId);
         constants__getall_conv_ExpandedNodeId_NodeId(session_role_eval__l_roleSet_Reference_ExpandedNodeId,
            &session_role_eval__l_local_server,
            &session_role_eval__l_roleSet_Reference_NodeId);
         address_space_itf__readall_AddressSpace_Node(session_role_eval__l_roleSet_Reference_NodeId,
            &session_role_eval__l_isvalid,
            &session_role_eval__l_roleSet_Reference_Node);
         session_role_eval__l_check_node_NodeClass_and_TypeDef(session_role_eval__l_roleSet_Reference_Node,
            &session_role_eval__l_bValidRoleNode);
         if (session_role_eval__l_bValidRoleNode == true) {
            *session_role_eval__p_bres = true;
            *session_role_eval__p_maybe_role_node = session_role_eval__l_roleSet_Reference_Node;
            *session_role_eval__p_maybe_role_nodeId = session_role_eval__l_roleSet_Reference_NodeId;
         }
      }
   }
}

