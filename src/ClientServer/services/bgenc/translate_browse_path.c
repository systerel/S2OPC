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

 File Name            : translate_browse_path.c

 Date                 : 24/07/2023 14:29:19

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "translate_browse_path.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void translate_browse_path__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void translate_browse_path__init_translate_browse_paths_request(
   const constants__t_msg_i translate_browse_path__req_msg,
   constants_statuscodes_bs__t_StatusCode_i * const translate_browse_path__StatusCode_service) {
   {
      t_entier4 translate_browse_path__l_nb_BrowsePaths;
      
      msg_translate_browse_path_bs__decode_translate_browse_paths_request(translate_browse_path__req_msg,
         translate_browse_path__StatusCode_service);
      if (*translate_browse_path__StatusCode_service == constants_statuscodes_bs__e_sc_ok) {
         msg_translate_browse_path_bs__read_nb_BrowsePaths(&translate_browse_path__l_nb_BrowsePaths);
         if (translate_browse_path__l_nb_BrowsePaths == 0) {
            *translate_browse_path__StatusCode_service = constants_statuscodes_bs__e_sc_bad_nothing_to_do;
         }
         else {
            msg_translate_browse_path_bs__alloc_browse_path_result(translate_browse_path__StatusCode_service);
            if (*translate_browse_path__StatusCode_service != constants_statuscodes_bs__e_sc_ok) {
               *translate_browse_path__StatusCode_service = constants_statuscodes_bs__e_sc_bad_too_many_ops;
            }
         }
      }
   }
}

void translate_browse_path__treat_one_translate_browse_path(
   const constants__t_BrowsePath_i translate_browse_path__browsePath) {
   {
      constants_statuscodes_bs__t_StatusCode_i translate_browse_path__l_statusCode_operation;
      constants_statuscodes_bs__t_StatusCode_i translate_browse_path__l_statusCode_operation_2;
      t_entier4 translate_browse_path__l_size_rel_path;
      constants__t_RelativePath_i translate_browse_path__l_relativePath;
      constants__t_NodeId_i translate_browse_path__l_source;
      
      msg_translate_browse_path_bs__read_BrowsePath_RelativePath(translate_browse_path__browsePath,
         &translate_browse_path__l_relativePath);
      msg_translate_browse_path_bs__read_BrowsePath_StartingNode(translate_browse_path__browsePath,
         &translate_browse_path__l_source);
      msg_translate_browse_path_bs__read_RelativePath_Nb_RelativePathElt(translate_browse_path__l_relativePath,
         &translate_browse_path__l_size_rel_path);
      translate_browse_path__check_startingNode(translate_browse_path__l_source,
         &translate_browse_path__l_statusCode_operation);
      if (translate_browse_path__l_size_rel_path == 0) {
         msg_translate_browse_path_bs__write_BrowsePath_Res_StatusCode(translate_browse_path__browsePath,
            constants_statuscodes_bs__e_sc_bad_nothing_to_do);
      }
      else if (translate_browse_path__l_statusCode_operation != constants_statuscodes_bs__e_sc_ok) {
         msg_translate_browse_path_bs__write_BrowsePath_Res_StatusCode(translate_browse_path__browsePath,
            translate_browse_path__l_statusCode_operation);
      }
      else {
         translate_browse_path__treat_one_translate_browse_path_1(translate_browse_path__l_source,
            translate_browse_path__l_relativePath,
            &translate_browse_path__l_statusCode_operation);
         if ((translate_browse_path__l_statusCode_operation == constants_statuscodes_bs__e_sc_ok) ||
            (translate_browse_path__l_statusCode_operation == constants_statuscodes_bs__e_sc_uncertain_reference_out_of_server)) {
            translate_browse_path__l_statusCode_operation_2 = translate_browse_path__l_statusCode_operation;
            translate_browse_path__copy_browsePathResult_to_msg(translate_browse_path__browsePath,
               translate_browse_path__l_statusCode_operation_2,
               &translate_browse_path__l_statusCode_operation);
         }
         msg_translate_browse_path_bs__write_BrowsePath_Res_StatusCode(translate_browse_path__browsePath,
            translate_browse_path__l_statusCode_operation);
         translate_browse_path_result__free_BrowsePathResult();
         translate_browse_path_result__free_BrowsePathRemaining();
      }
   }
}

void translate_browse_path__check_startingNode(
   const constants__t_NodeId_i translate_browse_path__nodeid,
   constants_statuscodes_bs__t_StatusCode_i * const translate_browse_path__StatusCode) {
   {
      t_bool translate_browse_path__l_isvalid;
      constants__t_Node_i translate_browse_path__l_src_node;
      
      *translate_browse_path__StatusCode = constants_statuscodes_bs__e_sc_ok;
      if (translate_browse_path__nodeid == constants__c_NodeId_indet) {
         *translate_browse_path__StatusCode = constants_statuscodes_bs__e_sc_bad_node_id_invalid;
      }
      else {
         address_space_itf__readall_AddressSpace_Node(translate_browse_path__nodeid,
            &translate_browse_path__l_isvalid,
            &translate_browse_path__l_src_node);
         if (translate_browse_path__l_isvalid == false) {
            *translate_browse_path__StatusCode = constants_statuscodes_bs__e_sc_bad_node_id_unknown;
         }
      }
   }
}

void translate_browse_path__copy_browsePathResult_to_source(
   constants_statuscodes_bs__t_StatusCode_i * const translate_browse_path__statusCode_operation) {
   {
      t_entier4 translate_browse_path__l_size;
      t_bool translate_browse_path__l_continue;
      t_entier4 translate_browse_path__l_index;
      constants__t_ExpandedNodeId_i translate_browse_path__l_expandedNodeId;
      t_bool translate_browse_path__l_local_server;
      constants__t_NodeId_i translate_browse_path__l_nodeId;
      
      *translate_browse_path__statusCode_operation = constants_statuscodes_bs__e_sc_ok;
      translate_browse_path_result__get_BrowsePathResultSize(&translate_browse_path__l_size);
      translate_browse_path_source_it__init_iter_browsePathSourceIdx(translate_browse_path__l_size,
         &translate_browse_path__l_continue);
      while (translate_browse_path__l_continue == true) {
         translate_browse_path_source_it__continue_iter_browsePathSourceIdx(&translate_browse_path__l_continue,
            &translate_browse_path__l_index);
         translate_browse_path_result__get_BrowsePathResult(translate_browse_path__l_index,
            &translate_browse_path__l_expandedNodeId);
         if (translate_browse_path__l_expandedNodeId != constants__c_ExpandedNodeId_indet) {
            constants__getall_conv_ExpandedNodeId_NodeId(translate_browse_path__l_expandedNodeId,
               &translate_browse_path__l_local_server,
               &translate_browse_path__l_nodeId);
            translate_browse_path_source__update_one_browse_path_source(translate_browse_path__l_nodeId,
               translate_browse_path__statusCode_operation);
            if (*translate_browse_path__statusCode_operation != constants_statuscodes_bs__e_sc_ok) {
               translate_browse_path_source__free_BrowsePathSource();
               translate_browse_path__l_continue = false;
            }
         }
         else {
            translate_browse_path_source__free_BrowsePathSource();
            *translate_browse_path__statusCode_operation = constants_statuscodes_bs__e_sc_bad_unexpected_error;
            translate_browse_path__l_continue = false;
         }
      }
   }
}

void translate_browse_path__copy_browsePathResult_to_msg(
   const constants__t_BrowsePath_i translate_browse_path__browsePath,
   const constants_statuscodes_bs__t_StatusCode_i translate_browse_path__in_statusCode_operation,
   constants_statuscodes_bs__t_StatusCode_i * const translate_browse_path__out_statusCode_operation) {
   {
      t_entier4 translate_browse_path__l_size;
      t_entier4 translate_browse_path__l_size_result;
      t_entier4 translate_browse_path__l_size_remaining;
      constants_statuscodes_bs__t_StatusCode_i translate_browse_path__l_statusCode_alloc;
      constants_statuscodes_bs__t_StatusCode_i translate_browse_path__l_statusCode_op;
      
      *translate_browse_path__out_statusCode_operation = translate_browse_path__in_statusCode_operation;
      translate_browse_path_result__get_BrowsePathResultSize(&translate_browse_path__l_size_result);
      translate_browse_path_result__get_BrowsePathRemainingSize(&translate_browse_path__l_size_remaining);
      translate_browse_path__l_size = translate_browse_path__l_size_result +
         translate_browse_path__l_size_remaining;
      if (translate_browse_path__l_size <= constants__k_n_BrowsePathResPerPath_max) {
         msg_translate_browse_path_bs__alloc_BrowsePath_Res_Target(translate_browse_path__browsePath,
            translate_browse_path__l_size,
            &translate_browse_path__l_statusCode_alloc);
         if (translate_browse_path__l_statusCode_alloc == constants_statuscodes_bs__e_sc_ok) {
            translate_browse_path__l_statusCode_op = *translate_browse_path__out_statusCode_operation;
            translate_browse_path__copy_browsePathResult_to_msg_1(translate_browse_path__browsePath,
               translate_browse_path__l_statusCode_op,
               translate_browse_path__l_size_result,
               translate_browse_path__out_statusCode_operation,
               &translate_browse_path__l_statusCode_alloc);
            if (translate_browse_path__l_statusCode_alloc == constants_statuscodes_bs__e_sc_ok) {
               translate_browse_path__l_statusCode_op = *translate_browse_path__out_statusCode_operation;
               translate_browse_path__copy_browsePathResult_to_msg_2(translate_browse_path__browsePath,
                  translate_browse_path__l_statusCode_op,
                  translate_browse_path__l_size_remaining,
                  translate_browse_path__out_statusCode_operation);
            }
         }
         else {
            *translate_browse_path__out_statusCode_operation = constants_statuscodes_bs__e_sc_bad_out_of_memory;
         }
      }
      else {
         *translate_browse_path__out_statusCode_operation = constants_statuscodes_bs__e_sc_bad_too_many_matches;
      }
   }
}

void translate_browse_path__copy_browsePathResult_to_msg_1(
   const constants__t_BrowsePath_i translate_browse_path__browsePath,
   const constants_statuscodes_bs__t_StatusCode_i translate_browse_path__in_statusCode_operation,
   const t_entier4 translate_browse_path__size_result,
   constants_statuscodes_bs__t_StatusCode_i * const translate_browse_path__out_statusCode_operation,
   constants_statuscodes_bs__t_StatusCode_i * const translate_browse_path__statusCode_alloc) {
   {
      t_bool translate_browse_path__l_continue;
      t_entier4 translate_browse_path__l_index;
      constants__t_ExpandedNodeId_i translate_browse_path__l_expandedNodeId;
      
      *translate_browse_path__statusCode_alloc = constants_statuscodes_bs__e_sc_ok;
      *translate_browse_path__out_statusCode_operation = translate_browse_path__in_statusCode_operation;
      translate_browse_path_source_it__init_iter_browsePathSourceIdx(translate_browse_path__size_result,
         &translate_browse_path__l_continue);
      translate_browse_path__l_index = 0;
      while (translate_browse_path__l_continue == true) {
         translate_browse_path_source_it__continue_iter_browsePathSourceIdx(&translate_browse_path__l_continue,
            &translate_browse_path__l_index);
         translate_browse_path_result__get_BrowsePathResult(translate_browse_path__l_index,
            &translate_browse_path__l_expandedNodeId);
         msg_translate_browse_path_bs__add_BrowsePath_Res_Target(translate_browse_path__browsePath,
            translate_browse_path__l_expandedNodeId,
            translate_browse_path__statusCode_alloc);
         if (*translate_browse_path__statusCode_alloc != constants_statuscodes_bs__e_sc_ok) {
            *translate_browse_path__out_statusCode_operation = constants_statuscodes_bs__e_sc_bad_out_of_memory;
            translate_browse_path__l_continue = false;
         }
      }
   }
}

void translate_browse_path__copy_browsePathResult_to_msg_2(
   const constants__t_BrowsePath_i translate_browse_path__browsePath,
   const constants_statuscodes_bs__t_StatusCode_i translate_browse_path__in_statusCode_operation,
   const t_entier4 translate_browse_path__nb_max_ref,
   constants_statuscodes_bs__t_StatusCode_i * const translate_browse_path__out_statusCode_operation) {
   {
      t_bool translate_browse_path__l_continue;
      t_entier4 translate_browse_path__l_index;
      constants__t_ExpandedNodeId_i translate_browse_path__l_expandedNodeId;
      t_entier4 translate_browse_path__l_remainingIndex;
      constants_statuscodes_bs__t_StatusCode_i translate_browse_path__l_statusCode_alloc;
      
      *translate_browse_path__out_statusCode_operation = translate_browse_path__in_statusCode_operation;
      translate_browse_path__l_statusCode_alloc = constants_statuscodes_bs__e_sc_ok;
      translate_browse_path_source_it__init_iter_browsePathSourceIdx(translate_browse_path__nb_max_ref,
         &translate_browse_path__l_continue);
      translate_browse_path__l_index = 0;
      while (translate_browse_path__l_continue == true) {
         translate_browse_path_source_it__continue_iter_browsePathSourceIdx(&translate_browse_path__l_continue,
            &translate_browse_path__l_index);
         translate_browse_path_result__get_BrowsePathRemaining(translate_browse_path__l_index,
            &translate_browse_path__l_expandedNodeId,
            &translate_browse_path__l_remainingIndex);
         msg_translate_browse_path_bs__add_BrowsePath_Res_Target_withRemainingPath(translate_browse_path__browsePath,
            translate_browse_path__l_expandedNodeId,
            translate_browse_path__l_remainingIndex,
            &translate_browse_path__l_statusCode_alloc);
         if (translate_browse_path__l_statusCode_alloc != constants_statuscodes_bs__e_sc_ok) {
            *translate_browse_path__out_statusCode_operation = constants_statuscodes_bs__e_sc_bad_out_of_memory;
            translate_browse_path__l_continue = false;
         }
      }
   }
}

void translate_browse_path__treat_one_translate_browse_path_1(
   const constants__t_NodeId_i translate_browse_path__source,
   const constants__t_RelativePath_i translate_browse_path__rel_path,
   constants_statuscodes_bs__t_StatusCode_i * const translate_browse_path__statusCode_operation) {
   {
      t_bool translate_browse_path__l_continue;
      t_bool translate_browse_path__l_continue_1;
      t_entier4 translate_browse_path__l_index;
      constants__t_RelativePathElt_i translate_browse_path__l_relativePathElt;
      
      translate_browse_path_source__update_one_browse_path_source(translate_browse_path__source,
         translate_browse_path__statusCode_operation);
      if (*translate_browse_path__statusCode_operation == constants_statuscodes_bs__e_sc_ok) {
         translate_browse_path_element_it__init_iter_relativePath(translate_browse_path__rel_path,
            &translate_browse_path__l_continue);
         translate_browse_path__l_index = 0;
         while (translate_browse_path__l_continue == true) {
            translate_browse_path_element_it__continue_iter_relativePath(&translate_browse_path__l_continue,
               &translate_browse_path__l_relativePathElt,
               &translate_browse_path__l_index);
            translate_browse_path__l_continue_1 = translate_browse_path__l_continue;
            translate_browse_path__treat_relative_path_sequence(translate_browse_path__l_relativePathElt,
               translate_browse_path__l_index,
               translate_browse_path__l_continue_1,
               translate_browse_path__statusCode_operation,
               &translate_browse_path__l_continue);
         }
      }
   }
}

void translate_browse_path__treat_relative_path_sequence(
   const constants__t_RelativePathElt_i translate_browse_path__rel_path_elt,
   const t_entier4 translate_browse_path__path_index,
   const t_bool translate_browse_path__continue,
   constants_statuscodes_bs__t_StatusCode_i * const translate_browse_path__statusCode_operation,
   t_bool * const translate_browse_path__p_continue) {
   translate_browse_path_result__free_BrowsePathResult();
   translate_browse_path_result__treat_one_relative_path_element(translate_browse_path__rel_path_elt,
      translate_browse_path__path_index,
      translate_browse_path__statusCode_operation);
   translate_browse_path_source__free_BrowsePathSource();
   *translate_browse_path__p_continue = ((translate_browse_path__continue == true) &&
      ((*translate_browse_path__statusCode_operation == constants_statuscodes_bs__e_sc_ok) ||
      (*translate_browse_path__statusCode_operation == constants_statuscodes_bs__e_sc_uncertain_reference_out_of_server)));
   if (*translate_browse_path__p_continue == true) {
      translate_browse_path__copy_browsePathResult_to_source(translate_browse_path__statusCode_operation);
   }
}

