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

 Date                 : 04/08/2022 14:53:24

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
      t_bool translate_browse_path__l_continue;
      constants__t_RelativePath_i translate_browse_path__l_relativePath;
      constants__t_RelativePathElt_i translate_browse_path__l_relativePathElt;
      t_entier4 translate_browse_path__l_index;
      constants__t_NodeId_i translate_browse_path__l_source;
      constants__t_NodeId_i translate_browse_path__l_source_copy;
      
      msg_translate_browse_path_bs__read_BrowsePath_RelativePath(translate_browse_path__browsePath,
         &translate_browse_path__l_relativePath);
      msg_translate_browse_path_bs__read_BrowsePath_StartingNode(translate_browse_path__browsePath,
         &translate_browse_path__l_source);
      translate_browse_path_element_it__init_iter_relativePath(translate_browse_path__l_relativePath,
         &translate_browse_path__l_continue);
      translate_browse_path__check_startingNode(translate_browse_path__l_source,
         &translate_browse_path__l_statusCode_operation);
      if (translate_browse_path__l_continue == false) {
         msg_translate_browse_path_bs__write_BrowsePath_Res_StatusCode(translate_browse_path__browsePath,
            constants_statuscodes_bs__e_sc_bad_nothing_to_do);
      }
      else if (translate_browse_path__l_statusCode_operation != constants_statuscodes_bs__e_sc_ok) {
         msg_translate_browse_path_bs__write_BrowsePath_Res_StatusCode(translate_browse_path__browsePath,
            translate_browse_path__l_statusCode_operation);
      }
      else {
         node_id_pointer_bs__copy_node_id_pointer_content(translate_browse_path__l_source,
            &translate_browse_path__l_continue,
            &translate_browse_path__l_source_copy);
         translate_browse_path__l_index = 0;
         if (translate_browse_path__l_continue == true) {
            translate_browse_path_1__add_BrowsePathSource(translate_browse_path__l_source_copy);
         }
         else {
            translate_browse_path__l_statusCode_operation = constants_statuscodes_bs__e_sc_bad_out_of_memory;
         }
         while (translate_browse_path__l_continue == true) {
            translate_browse_path__free_BrowsePathResult();
            translate_browse_path_element_it__continue_iter_relativePath(&translate_browse_path__l_continue,
               &translate_browse_path__l_relativePathElt,
               &translate_browse_path__l_index);
            translate_browse_path__treat_one_relative_path_element(translate_browse_path__l_relativePathElt,
               translate_browse_path__l_index,
               &translate_browse_path__l_statusCode_operation);
            translate_browse_path__free_BrowsePathSource();
            translate_browse_path__l_continue = ((translate_browse_path__l_continue == true) &&
               ((translate_browse_path__l_statusCode_operation == constants_statuscodes_bs__e_sc_ok) ||
               (translate_browse_path__l_statusCode_operation == constants_statuscodes_bs__e_sc_uncertain_reference_out_of_server)));
            if (translate_browse_path__l_continue == true) {
               translate_browse_path__copy_browsePathResult_to_source(&translate_browse_path__l_statusCode_operation);
            }
         }
         if ((translate_browse_path__l_statusCode_operation == constants_statuscodes_bs__e_sc_ok) ||
            (translate_browse_path__l_statusCode_operation == constants_statuscodes_bs__e_sc_uncertain_reference_out_of_server)) {
            translate_browse_path__l_statusCode_operation_2 = translate_browse_path__l_statusCode_operation;
            translate_browse_path__copy_browsePathResult_to_msg(translate_browse_path__browsePath,
               translate_browse_path__l_statusCode_operation_2,
               &translate_browse_path__l_statusCode_operation);
         }
         msg_translate_browse_path_bs__write_BrowsePath_Res_StatusCode(translate_browse_path__browsePath,
            translate_browse_path__l_statusCode_operation);
         translate_browse_path__free_BrowsePathResult();
         translate_browse_path__free_BrowsePathRemaining();
      }
   }
}

void translate_browse_path__treat_one_relative_path_element(
   const constants__t_RelativePathElt_i translate_browse_path__relativePathElt,
   const t_entier4 translate_browse_path__index,
   constants_statuscodes_bs__t_StatusCode_i * const translate_browse_path__statusCode_operation) {
   {
      constants__t_NodeId_i translate_browse_path__l_referenceTypeId;
      t_bool translate_browse_path__l_isInverse;
      t_bool translate_browse_path__l_includedSubtypes;
      constants__t_QualifiedName_i translate_browse_path__l_targetName;
      constants__t_BrowseDirection_i translate_browse_path__l_browseDirection;
      t_entier4 translate_browse_path__l_nbReferences;
      t_bool translate_browse_path__l_name_empty;
      constants_statuscodes_bs__t_StatusCode_i translate_browse_path__l_translate_statusCode;
      t_bool translate_browse_path__l_continue_source;
      constants__t_NodeId_i translate_browse_path__l_source;
      t_entier4 translate_browse_path__l_size;
      t_entier4 translate_browse_path__l_index;
      
      translate_browse_path__l_nbReferences = 0;
      msg_translate_browse_path_bs__read_RelativePathElt_ReferenceTypeId(translate_browse_path__relativePathElt,
         &translate_browse_path__l_referenceTypeId);
      if (translate_browse_path__l_referenceTypeId == constants__c_NodeId_indet) {
         translate_browse_path__l_includedSubtypes = false;
      }
      else {
         msg_translate_browse_path_bs__read_RelativePathElt_IncludedSubtypes(translate_browse_path__relativePathElt,
            &translate_browse_path__l_includedSubtypes);
      }
      msg_translate_browse_path_bs__read_RelativePathElt_IsInverse(translate_browse_path__relativePathElt,
         &translate_browse_path__l_isInverse);
      msg_translate_browse_path_bs__read_RelativePathElt_TargetName(translate_browse_path__relativePathElt,
         &translate_browse_path__l_targetName);
      translate_browse_path__get_browseDirection_from_isInverse(translate_browse_path__l_isInverse,
         &translate_browse_path__l_browseDirection);
      constants__is_QualifiedNames_Empty(translate_browse_path__l_targetName,
         &translate_browse_path__l_name_empty);
      if (translate_browse_path__l_name_empty == true) {
         *translate_browse_path__statusCode_operation = constants_statuscodes_bs__e_sc_bad_browse_name_invalid;
      }
      else {
         *translate_browse_path__statusCode_operation = constants_statuscodes_bs__e_sc_bad_nothing_to_do;
         translate_browse_path_1__get_BrowsePathSourceSize(&translate_browse_path__l_size);
         translate_browse_path_source_it__init_iter_browsePathSourceIdx(translate_browse_path__l_size,
            &translate_browse_path__l_continue_source);
         while (translate_browse_path__l_continue_source == true) {
            translate_browse_path_source_it__continue_iter_browsePathSourceIdx(&translate_browse_path__l_continue_source,
               &translate_browse_path__l_index);
            translate_browse_path_1__get_BrowsePathSource(translate_browse_path__l_index,
               &translate_browse_path__l_source);
            translate_browse_path__compute_browse_result_from_source(translate_browse_path__l_source,
               translate_browse_path__l_browseDirection,
               translate_browse_path__l_referenceTypeId,
               translate_browse_path__l_includedSubtypes,
               &translate_browse_path__l_translate_statusCode,
               &translate_browse_path__l_nbReferences);
            if (translate_browse_path__l_translate_statusCode != constants_statuscodes_bs__e_sc_ok) {
               *translate_browse_path__statusCode_operation = translate_browse_path__l_translate_statusCode;
            }
            else {
               translate_browse_path__treat_browse_result_one_source(translate_browse_path__index,
                  translate_browse_path__l_targetName,
                  translate_browse_path__l_nbReferences,
                  &translate_browse_path__l_translate_statusCode);
               if (*translate_browse_path__statusCode_operation != constants_statuscodes_bs__e_sc_ok) {
                  *translate_browse_path__statusCode_operation = translate_browse_path__l_translate_statusCode;
               }
               browse_treatment__clear_browse_result();
            }
         }
      }
   }
}

void translate_browse_path__get_browseDirection_from_isInverse(
   const t_bool translate_browse_path__isInverse,
   constants__t_BrowseDirection_i * const translate_browse_path__browseDirection) {
   if (translate_browse_path__isInverse == true) {
      *translate_browse_path__browseDirection = constants__e_bd_inverse;
   }
   else {
      *translate_browse_path__browseDirection = constants__e_bd_forward;
   }
}

void translate_browse_path__get_translateStatus_from_browseStatus(
   const constants_statuscodes_bs__t_StatusCode_i translate_browse_path__browse_statusCode,
   constants_statuscodes_bs__t_StatusCode_i * const translate_browse_path__translate_statusCode) {
   switch (translate_browse_path__browse_statusCode) {
   case constants_statuscodes_bs__e_sc_bad_node_id_unknown:
      *translate_browse_path__translate_statusCode = constants_statuscodes_bs__e_sc_bad_node_id_unknown;
      break;
   case constants_statuscodes_bs__e_sc_bad_reference_type_id_invalid:
      *translate_browse_path__translate_statusCode = constants_statuscodes_bs__e_sc_bad_no_match;
      break;
   case constants_statuscodes_bs__e_sc_bad_out_of_memory:
   case constants_statuscodes_bs__e_sc_bad_no_continuation_points:
   case constants_statuscodes_bs__e_sc_bad_view_id_unknown:
      *translate_browse_path__translate_statusCode = constants_statuscodes_bs__e_sc_bad_query_too_complex;
      break;
   case constants_statuscodes_bs__e_sc_ok:
      *translate_browse_path__translate_statusCode = constants_statuscodes_bs__e_sc_ok;
      break;
   default:
      *translate_browse_path__translate_statusCode = constants_statuscodes_bs__c_StatusCode_indet;
      break;
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
      t_bool translate_browse_path__l_alloc;
      constants__t_NodeId_i translate_browse_path__l_source_copy;
      
      *translate_browse_path__statusCode_operation = constants_statuscodes_bs__e_sc_ok;
      translate_browse_path_1__get_BrowsePathResultSize(&translate_browse_path__l_size);
      translate_browse_path_source_it__init_iter_browsePathSourceIdx(translate_browse_path__l_size,
         &translate_browse_path__l_continue);
      translate_browse_path__l_alloc = false;
      while (translate_browse_path__l_continue == true) {
         translate_browse_path_source_it__continue_iter_browsePathSourceIdx(&translate_browse_path__l_continue,
            &translate_browse_path__l_index);
         translate_browse_path_1__get_BrowsePathResult(translate_browse_path__l_index,
            &translate_browse_path__l_expandedNodeId);
         constants__getall_conv_ExpandedNodeId_NodeId(translate_browse_path__l_expandedNodeId,
            &translate_browse_path__l_local_server,
            &translate_browse_path__l_nodeId);
         node_id_pointer_bs__copy_node_id_pointer_content(translate_browse_path__l_nodeId,
            &translate_browse_path__l_alloc,
            &translate_browse_path__l_source_copy);
         if (translate_browse_path__l_alloc == true) {
            translate_browse_path_1__add_BrowsePathSource(translate_browse_path__l_source_copy);
         }
         else {
            translate_browse_path__free_BrowsePathSource();
            *translate_browse_path__statusCode_operation = constants_statuscodes_bs__e_sc_bad_out_of_memory;
            translate_browse_path__l_continue = false;
         }
      }
   }
}

void translate_browse_path__free_BrowsePathResult(void) {
   {
      t_entier4 translate_browse_path__l_size;
      t_bool translate_browse_path__l_continue;
      t_entier4 translate_browse_path__l_index;
      constants__t_ExpandedNodeId_i translate_browse_path__l_expandedNodeId;
      
      translate_browse_path_1__get_BrowsePathResultSize(&translate_browse_path__l_size);
      translate_browse_path_source_it__init_iter_browsePathSourceIdx(translate_browse_path__l_size,
         &translate_browse_path__l_continue);
      while (translate_browse_path__l_continue == true) {
         translate_browse_path_source_it__continue_iter_browsePathSourceIdx(&translate_browse_path__l_continue,
            &translate_browse_path__l_index);
         translate_browse_path_1__get_BrowsePathResult(translate_browse_path__l_index,
            &translate_browse_path__l_expandedNodeId);
         constants__free_ExpandedNodeId(translate_browse_path__l_expandedNodeId);
      }
      translate_browse_path_1__init_BrowsePathResult();
   }
}

void translate_browse_path__free_BrowsePathRemaining(void) {
   {
      t_entier4 translate_browse_path__l_size;
      t_bool translate_browse_path__l_continue;
      t_entier4 translate_browse_path__l_index;
      constants__t_ExpandedNodeId_i translate_browse_path__l_expandedNodeId;
      
      translate_browse_path_1__get_BrowsePathRemainingSize(&translate_browse_path__l_size);
      translate_browse_path_source_it__init_iter_browsePathSourceIdx(translate_browse_path__l_size,
         &translate_browse_path__l_continue);
      while (translate_browse_path__l_continue == true) {
         translate_browse_path_source_it__continue_iter_browsePathSourceIdx(&translate_browse_path__l_continue,
            &translate_browse_path__l_index);
         translate_browse_path_1__get_BrowsePathResult(translate_browse_path__l_index,
            &translate_browse_path__l_expandedNodeId);
         constants__free_ExpandedNodeId(translate_browse_path__l_expandedNodeId);
      }
      translate_browse_path_1__init_BrowsePathRemaining();
   }
}

void translate_browse_path__free_BrowsePathSource(void) {
   {
      t_entier4 translate_browse_path__l_size;
      t_bool translate_browse_path__l_continue;
      t_entier4 translate_browse_path__l_index;
      constants__t_NodeId_i translate_browse_path__l_nodeId;
      
      translate_browse_path_1__get_BrowsePathSourceSize(&translate_browse_path__l_size);
      translate_browse_path_source_it__init_iter_browsePathSourceIdx(translate_browse_path__l_size,
         &translate_browse_path__l_continue);
      while (translate_browse_path__l_continue == true) {
         translate_browse_path_source_it__continue_iter_browsePathSourceIdx(&translate_browse_path__l_continue,
            &translate_browse_path__l_index);
         translate_browse_path_1__get_BrowsePathSource(translate_browse_path__l_index,
            &translate_browse_path__l_nodeId);
         node_id_pointer_bs__free_node_id_pointer(translate_browse_path__l_nodeId);
      }
      translate_browse_path_1__init_BrowsePathSource();
   }
}

void translate_browse_path__checkAndAdd_BrowsePathResult(
   const constants__t_ExpandedNodeId_i translate_browse_path__expandedNodeId,
   constants_statuscodes_bs__t_StatusCode_i * const translate_browse_path__statusCode_operation) {
   {
      t_bool translate_browse_path__l_result_isFull;
      t_bool translate_browse_path__l_alloc;
      constants__t_ExpandedNodeId_i translate_browse_path__l_target_copy;
      
      translate_browse_path_1__get_BrowsePathResult_IsFull(&translate_browse_path__l_result_isFull);
      if (translate_browse_path__l_result_isFull == false) {
         constants__get_copy_ExpandedNodeId(translate_browse_path__expandedNodeId,
            &translate_browse_path__l_alloc,
            &translate_browse_path__l_target_copy);
         if (translate_browse_path__l_alloc == true) {
            translate_browse_path_1__add_BrowsePathResult(translate_browse_path__l_target_copy);
            *translate_browse_path__statusCode_operation = constants_statuscodes_bs__e_sc_ok;
         }
         else {
            *translate_browse_path__statusCode_operation = constants_statuscodes_bs__e_sc_bad_out_of_memory;
         }
      }
      else {
         *translate_browse_path__statusCode_operation = constants_statuscodes_bs__e_sc_bad_query_too_complex;
      }
   }
}

void translate_browse_path__checkAndAdd_BrowsePathRemaining(
   const constants__t_ExpandedNodeId_i translate_browse_path__expandedNodeId,
   const t_entier4 translate_browse_path__index,
   constants_statuscodes_bs__t_StatusCode_i * const translate_browse_path__statusCode_operation) {
   {
      t_bool translate_browse_path__l_isFull;
      t_bool translate_browse_path__l_alloc;
      constants__t_ExpandedNodeId_i translate_browse_path__l_copy;
      
      translate_browse_path_1__get_BrowsePathRemaining_IsFull(&translate_browse_path__l_isFull);
      if (translate_browse_path__l_isFull == false) {
         constants__get_copy_ExpandedNodeId(translate_browse_path__expandedNodeId,
            &translate_browse_path__l_alloc,
            &translate_browse_path__l_copy);
         if (translate_browse_path__l_alloc == true) {
            translate_browse_path_1__add_BrowsePathResultRemaining(translate_browse_path__l_copy,
               translate_browse_path__index);
            *translate_browse_path__statusCode_operation = constants_statuscodes_bs__e_sc_uncertain_reference_out_of_server;
         }
         else {
            *translate_browse_path__statusCode_operation = constants_statuscodes_bs__e_sc_bad_out_of_memory;
         }
      }
      else {
         *translate_browse_path__statusCode_operation = constants_statuscodes_bs__e_sc_bad_query_too_complex;
      }
   }
}

void translate_browse_path__copy_browsePathResult_to_msg(
   const constants__t_BrowsePath_i translate_browse_path__browsePath,
   const constants_statuscodes_bs__t_StatusCode_i translate_browse_path__in_statusCode_operation,
   constants_statuscodes_bs__t_StatusCode_i * const translate_browse_path__out_statusCode_operation) {
   {
      t_entier4 translate_browse_path__l_size;
      t_bool translate_browse_path__l_continue;
      t_entier4 translate_browse_path__l_index;
      t_entier4 translate_browse_path__l_size_result;
      t_entier4 translate_browse_path__l_size_remaining;
      constants_statuscodes_bs__t_StatusCode_i translate_browse_path__l_statusCode_alloc;
      constants__t_ExpandedNodeId_i translate_browse_path__l_expandedNodeId;
      t_entier4 translate_browse_path__l_remainingIndex;
      
      *translate_browse_path__out_statusCode_operation = translate_browse_path__in_statusCode_operation;
      translate_browse_path_1__get_BrowsePathResultSize(&translate_browse_path__l_size_result);
      translate_browse_path_1__get_BrowsePathRemainingSize(&translate_browse_path__l_size_remaining);
      translate_browse_path__l_size = translate_browse_path__l_size_result +
         translate_browse_path__l_size_remaining;
      if (translate_browse_path__l_size <= constants__k_n_BrowsePathResPerPath_max) {
         msg_translate_browse_path_bs__alloc_BrowsePath_Res_Target(translate_browse_path__browsePath,
            translate_browse_path__l_size,
            &translate_browse_path__l_statusCode_alloc);
         if (translate_browse_path__l_statusCode_alloc == constants_statuscodes_bs__e_sc_ok) {
            translate_browse_path_source_it__init_iter_browsePathSourceIdx(translate_browse_path__l_size_result,
               &translate_browse_path__l_continue);
            translate_browse_path__l_index = 0;
            while (translate_browse_path__l_continue == true) {
               translate_browse_path_source_it__continue_iter_browsePathSourceIdx(&translate_browse_path__l_continue,
                  &translate_browse_path__l_index);
               translate_browse_path_1__get_BrowsePathResult(translate_browse_path__l_index,
                  &translate_browse_path__l_expandedNodeId);
               msg_translate_browse_path_bs__add_BrowsePath_Res_Target(translate_browse_path__browsePath,
                  translate_browse_path__l_expandedNodeId,
                  &translate_browse_path__l_statusCode_alloc);
               if (translate_browse_path__l_statusCode_alloc != constants_statuscodes_bs__e_sc_ok) {
                  *translate_browse_path__out_statusCode_operation = constants_statuscodes_bs__e_sc_bad_out_of_memory;
                  translate_browse_path__l_continue = false;
               }
            }
            translate_browse_path_source_it__init_iter_browsePathSourceIdx(translate_browse_path__l_size_remaining,
               &translate_browse_path__l_continue);
            translate_browse_path__l_continue = ((translate_browse_path__l_statusCode_alloc == constants_statuscodes_bs__e_sc_ok) &&
               (translate_browse_path__l_continue == true));
            translate_browse_path__l_index = 0;
            while (translate_browse_path__l_continue == true) {
               translate_browse_path_source_it__continue_iter_browsePathSourceIdx(&translate_browse_path__l_continue,
                  &translate_browse_path__l_index);
               translate_browse_path_1__get_BrowsePathRemaining(translate_browse_path__l_index,
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
         else {
            *translate_browse_path__out_statusCode_operation = constants_statuscodes_bs__e_sc_bad_out_of_memory;
         }
      }
      else {
         *translate_browse_path__out_statusCode_operation = constants_statuscodes_bs__e_sc_bad_too_many_matches;
      }
   }
}

void translate_browse_path__compute_browse_result_from_source(
   const constants__t_NodeId_i translate_browse_path__source,
   const constants__t_BrowseDirection_i translate_browse_path__browseDirection,
   const constants__t_NodeId_i translate_browse_path__referenceTypeId,
   const t_bool translate_browse_path__includedSubtypes,
   constants_statuscodes_bs__t_StatusCode_i * const translate_browse_path__statusCode_operation,
   t_entier4 * const translate_browse_path__nbReferences) {
   {
      constants_statuscodes_bs__t_StatusCode_i translate_browse_path__l_browse_statusCode;
      constants__t_ContinuationPointId_i translate_browse_path__l_continuationPoint;
      
      *translate_browse_path__nbReferences = 0;
      browse_treatment__set_browse_value_context(constants__c_session_indet,
         0,
         constants__c_NodeId_indet,
         translate_browse_path__source,
         translate_browse_path__browseDirection,
         translate_browse_path__referenceTypeId,
         translate_browse_path__includedSubtypes,
         constants__c_BrowseNodeClassMask_indet,
         constants__c_BrowseResultMask_all,
         false,
         translate_browse_path__statusCode_operation);
      if (*translate_browse_path__statusCode_operation == constants_statuscodes_bs__e_sc_ok) {
         browse_treatment__compute_browse_result(&translate_browse_path__l_browse_statusCode,
            &translate_browse_path__l_continuationPoint,
            translate_browse_path__nbReferences);
         browse_treatment__clear_browse_value_context();
         translate_browse_path__get_translateStatus_from_browseStatus(translate_browse_path__l_browse_statusCode,
            translate_browse_path__statusCode_operation);
      }
   }
}

void translate_browse_path__treat_browse_result_one_source(
   const t_entier4 translate_browse_path__index,
   const constants__t_QualifiedName_i translate_browse_path__targetName,
   const t_entier4 translate_browse_path__nbReferences,
   constants_statuscodes_bs__t_StatusCode_i * const translate_browse_path__statusCode_operation) {
   {
      t_bool translate_browse_path__l_continue;
      t_entier4 translate_browse_path__l_browseResult_index;
      constants__t_NodeId_i translate_browse_path__l_res_refTypeId;
      t_bool translate_browse_path__l_res_isForward;
      constants__t_ExpandedNodeId_i translate_browse_path__l_res_ExpandedNodeId;
      constants__t_QualifiedName_i translate_browse_path__l_res_BrowseName;
      constants__t_LocalizedText_i translate_browse_path__l_res_DisplayName;
      constants__t_NodeClass_i translate_browse_path__l_res_NodeClass;
      constants__t_ExpandedNodeId_i translate_browse_path__l_res_TypeDefinition;
      t_bool translate_browse_path__l_found;
      constants_statuscodes_bs__t_StatusCode_i translate_browse_path__l_translate_statusCode;
      t_bool translate_browse_path__l_local_server;
      constants__t_NodeId_i translate_browse_path__l_source_tmp;
      
      *translate_browse_path__statusCode_operation = constants_statuscodes_bs__e_sc_bad_no_match;
      translate_browse_path_result_it__init_iter_translate_browseResult(translate_browse_path__nbReferences,
         &translate_browse_path__l_continue);
      while (translate_browse_path__l_continue == true) {
         translate_browse_path_result_it__continue_iter_translate_browseResult(&translate_browse_path__l_continue,
            &translate_browse_path__l_browseResult_index);
         browse_treatment__getall_browse_result_reference_at(translate_browse_path__l_browseResult_index,
            &translate_browse_path__l_res_refTypeId,
            &translate_browse_path__l_res_isForward,
            &translate_browse_path__l_res_ExpandedNodeId,
            &translate_browse_path__l_res_BrowseName,
            &translate_browse_path__l_res_DisplayName,
            &translate_browse_path__l_res_NodeClass,
            &translate_browse_path__l_res_TypeDefinition);
         constants__is_QualifiedNames_Equal(translate_browse_path__targetName,
            translate_browse_path__l_res_BrowseName,
            &translate_browse_path__l_found);
         if (translate_browse_path__l_found == true) {
            translate_browse_path__checkAndAdd_BrowsePathResult(translate_browse_path__l_res_ExpandedNodeId,
               &translate_browse_path__l_translate_statusCode);
            if ((translate_browse_path__l_translate_statusCode == constants_statuscodes_bs__e_sc_ok) &&
               (*translate_browse_path__statusCode_operation == constants_statuscodes_bs__e_sc_uncertain_reference_out_of_server)) {
               ;
            }
            else {
               *translate_browse_path__statusCode_operation = translate_browse_path__l_translate_statusCode;
            }
         }
         else if (translate_browse_path__l_res_ExpandedNodeId != constants__c_ExpandedNodeId_indet) {
            constants__getall_conv_ExpandedNodeId_NodeId(translate_browse_path__l_res_ExpandedNodeId,
               &translate_browse_path__l_local_server,
               &translate_browse_path__l_source_tmp);
            if (translate_browse_path__l_local_server == false) {
               translate_browse_path__checkAndAdd_BrowsePathRemaining(translate_browse_path__l_res_ExpandedNodeId,
                  translate_browse_path__index,
                  translate_browse_path__statusCode_operation);
            }
         }
         translate_browse_path__l_continue = ((translate_browse_path__l_continue == true) &&
            (((*translate_browse_path__statusCode_operation == constants_statuscodes_bs__e_sc_ok) ||
            (*translate_browse_path__statusCode_operation == constants_statuscodes_bs__e_sc_uncertain_reference_out_of_server)) ||
            (*translate_browse_path__statusCode_operation == constants_statuscodes_bs__e_sc_bad_no_match)));
      }
   }
}

