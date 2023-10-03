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

 File Name            : translate_browse_path_result.c

 Date                 : 03/10/2023 14:12:15

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "translate_browse_path_result.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void translate_browse_path_result__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void translate_browse_path_result__treat_one_relative_path_element(
   const constants__t_RelativePathElt_i translate_browse_path_result__relativePathElt,
   const t_entier4 translate_browse_path_result__path_index,
   constants_statuscodes_bs__t_StatusCode_i * const translate_browse_path_result__statusCode_operation) {
   {
      constants__t_NodeId_i translate_browse_path_result__l_referenceTypeId;
      t_bool translate_browse_path_result__l_isInverse;
      t_bool translate_browse_path_result__l_includedSubtypes;
      constants__t_QualifiedName_i translate_browse_path_result__l_targetName;
      constants__t_BrowseDirection_i translate_browse_path_result__l_browseDirection;
      t_bool translate_browse_path_result__l_name_empty;
      
      msg_translate_browse_path_bs__read_RelativePathElt_ReferenceTypeId(translate_browse_path_result__relativePathElt,
         &translate_browse_path_result__l_referenceTypeId);
      if (translate_browse_path_result__l_referenceTypeId == constants__c_NodeId_indet) {
         translate_browse_path_result__l_includedSubtypes = false;
      }
      else {
         msg_translate_browse_path_bs__read_RelativePathElt_IncludedSubtypes(translate_browse_path_result__relativePathElt,
            &translate_browse_path_result__l_includedSubtypes);
      }
      msg_translate_browse_path_bs__read_RelativePathElt_IsInverse(translate_browse_path_result__relativePathElt,
         &translate_browse_path_result__l_isInverse);
      msg_translate_browse_path_bs__read_RelativePathElt_TargetName(translate_browse_path_result__relativePathElt,
         &translate_browse_path_result__l_targetName);
      translate_browse_path_result__get_browseDirection_from_isInverse(translate_browse_path_result__l_isInverse,
         &translate_browse_path_result__l_browseDirection);
      constants__is_QualifiedNames_Empty(translate_browse_path_result__l_targetName,
         &translate_browse_path_result__l_name_empty);
      if (translate_browse_path_result__l_name_empty == true) {
         *translate_browse_path_result__statusCode_operation = constants_statuscodes_bs__e_sc_bad_browse_name_invalid;
      }
      else {
         translate_browse_path_result__treat_one_relative_path_element_1(translate_browse_path_result__l_browseDirection,
            translate_browse_path_result__l_referenceTypeId,
            translate_browse_path_result__l_includedSubtypes,
            translate_browse_path_result__path_index,
            translate_browse_path_result__l_targetName,
            translate_browse_path_result__statusCode_operation);
      }
   }
}

void translate_browse_path_result__free_BrowsePathResult(void) {
   {
      t_entier4 translate_browse_path_result__l_size;
      t_bool translate_browse_path_result__l_continue;
      t_entier4 translate_browse_path_result__l_index;
      constants__t_ExpandedNodeId_i translate_browse_path_result__l_expandedNodeId;
      
      translate_browse_path_result_1__get_BrowsePathResultSize(&translate_browse_path_result__l_size);
      translate_browse_path_result_1_it__init_iter_browsePathIdx(translate_browse_path_result__l_size,
         &translate_browse_path_result__l_continue);
      translate_browse_path_result__l_index = 0;
      while (translate_browse_path_result__l_continue == true) {
         translate_browse_path_result_1_it__continue_iter_browsePathIdx(&translate_browse_path_result__l_continue,
            &translate_browse_path_result__l_index);
         translate_browse_path_result_1__get_BrowsePathResult(translate_browse_path_result__l_index,
            &translate_browse_path_result__l_expandedNodeId);
         constants__free_ExpandedNodeId(translate_browse_path_result__l_expandedNodeId);
      }
      translate_browse_path_result_1__init_BrowsePathResult();
   }
}

void translate_browse_path_result__free_BrowsePathRemaining(void) {
   {
      t_entier4 translate_browse_path_result__l_size;
      t_bool translate_browse_path_result__l_continue;
      t_entier4 translate_browse_path_result__l_index;
      constants__t_ExpandedNodeId_i translate_browse_path_result__l_expandedNodeId;
      
      translate_browse_path_result_1__get_BrowsePathRemainingSize(&translate_browse_path_result__l_size);
      translate_browse_path_result_1_it__init_iter_browsePathIdx(translate_browse_path_result__l_size,
         &translate_browse_path_result__l_continue);
      translate_browse_path_result__l_index = 0;
      while (translate_browse_path_result__l_continue == true) {
         translate_browse_path_result_1_it__continue_iter_browsePathIdx(&translate_browse_path_result__l_continue,
            &translate_browse_path_result__l_index);
         translate_browse_path_result_1__get_BrowsePathResult(translate_browse_path_result__l_index,
            &translate_browse_path_result__l_expandedNodeId);
         constants__free_ExpandedNodeId(translate_browse_path_result__l_expandedNodeId);
      }
      translate_browse_path_result_1__init_BrowsePathRemaining();
   }
}

void translate_browse_path_result__treat_one_relative_path_element_1(
   const constants__t_BrowseDirection_i translate_browse_path_result__browseDirection,
   const constants__t_NodeId_i translate_browse_path_result__referenceTypeId,
   const t_bool translate_browse_path_result__includedSubtypes,
   const t_entier4 translate_browse_path_result__path_index,
   const constants__t_QualifiedName_i translate_browse_path_result__targetName,
   constants_statuscodes_bs__t_StatusCode_i * const translate_browse_path_result__statusCode_operation) {
   {
      t_entier4 translate_browse_path_result__l_size;
      t_entier4 translate_browse_path_result__l_index;
      t_bool translate_browse_path_result__l_continue_source;
      constants_statuscodes_bs__t_StatusCode_i translate_browse_path_result__l_translate_statusCode;
      
      *translate_browse_path_result__statusCode_operation = constants_statuscodes_bs__e_sc_bad_nothing_to_do;
      translate_browse_path_source__get_BrowsePathSourceSize(&translate_browse_path_result__l_size);
      translate_browse_path_result_1_it__init_iter_browsePathIdx(translate_browse_path_result__l_size,
         &translate_browse_path_result__l_continue_source);
      translate_browse_path_result__l_translate_statusCode = *translate_browse_path_result__statusCode_operation;
      translate_browse_path_result__l_index = 0;
      while (translate_browse_path_result__l_continue_source == true) {
         translate_browse_path_result_1_it__continue_iter_browsePathIdx(&translate_browse_path_result__l_continue_source,
            &translate_browse_path_result__l_index);
         translate_browse_path_result__treat_one_relative_path_element_2(translate_browse_path_result__l_translate_statusCode,
            translate_browse_path_result__l_index,
            translate_browse_path_result__browseDirection,
            translate_browse_path_result__referenceTypeId,
            translate_browse_path_result__includedSubtypes,
            translate_browse_path_result__path_index,
            translate_browse_path_result__targetName,
            translate_browse_path_result__statusCode_operation);
         translate_browse_path_result__l_translate_statusCode = *translate_browse_path_result__statusCode_operation;
      }
   }
}

void translate_browse_path_result__treat_one_relative_path_element_2(
   const constants_statuscodes_bs__t_StatusCode_i translate_browse_path_result__statusCode_init,
   const t_entier4 translate_browse_path_result__loop_index,
   const constants__t_BrowseDirection_i translate_browse_path_result__browseDirection,
   const constants__t_NodeId_i translate_browse_path_result__referenceTypeId,
   const t_bool translate_browse_path_result__includedSubtypes,
   const t_entier4 translate_browse_path_result__path_index,
   const constants__t_QualifiedName_i translate_browse_path_result__targetName,
   constants_statuscodes_bs__t_StatusCode_i * const translate_browse_path_result__statusCode_operation) {
   {
      constants__t_NodeId_i translate_browse_path_result__l_source;
      constants_statuscodes_bs__t_StatusCode_i translate_browse_path_result__l_translate_statusCode;
      t_entier4 translate_browse_path_result__l_nbReferences;
      
      *translate_browse_path_result__statusCode_operation = translate_browse_path_result__statusCode_init;
      translate_browse_path_source__get_BrowsePathSource(translate_browse_path_result__loop_index,
         &translate_browse_path_result__l_source);
      translate_browse_path_result__compute_browse_result_from_source(translate_browse_path_result__l_source,
         translate_browse_path_result__browseDirection,
         translate_browse_path_result__referenceTypeId,
         translate_browse_path_result__includedSubtypes,
         &translate_browse_path_result__l_translate_statusCode,
         &translate_browse_path_result__l_nbReferences);
      if (translate_browse_path_result__l_translate_statusCode != constants_statuscodes_bs__e_sc_ok) {
         *translate_browse_path_result__statusCode_operation = translate_browse_path_result__l_translate_statusCode;
      }
      else {
         translate_browse_path_result__treat_browse_result_one_source(translate_browse_path_result__path_index,
            translate_browse_path_result__targetName,
            translate_browse_path_result__l_nbReferences,
            &translate_browse_path_result__l_translate_statusCode);
         if (*translate_browse_path_result__statusCode_operation != constants_statuscodes_bs__e_sc_ok) {
            *translate_browse_path_result__statusCode_operation = translate_browse_path_result__l_translate_statusCode;
         }
         browse_treatment__clear_browse_result();
      }
   }
}

void translate_browse_path_result__treat_browse_result_one_source(
   const t_entier4 translate_browse_path_result__path_index,
   const constants__t_QualifiedName_i translate_browse_path_result__targetName,
   const t_entier4 translate_browse_path_result__nbReferences,
   constants_statuscodes_bs__t_StatusCode_i * const translate_browse_path_result__statusCode_operation) {
   {
      t_bool translate_browse_path_result__l_continue;
      t_entier4 translate_browse_path_result__l_browseResult_index;
      constants_statuscodes_bs__t_StatusCode_i translate_browse_path_result__l_translate_statusCode;
      
      *translate_browse_path_result__statusCode_operation = constants_statuscodes_bs__e_sc_bad_no_match;
      translate_browse_path_result_it__init_iter_translate_browseResult(translate_browse_path_result__nbReferences,
         &translate_browse_path_result__l_continue);
      while (translate_browse_path_result__l_continue == true) {
         translate_browse_path_result_it__continue_iter_translate_browseResult(&translate_browse_path_result__l_continue,
            &translate_browse_path_result__l_browseResult_index);
         translate_browse_path_result__treat_browse_result_one_source_1(translate_browse_path_result__path_index,
            translate_browse_path_result__targetName,
            translate_browse_path_result__l_browseResult_index,
            *translate_browse_path_result__statusCode_operation,
            &translate_browse_path_result__l_translate_statusCode);
         *translate_browse_path_result__statusCode_operation = translate_browse_path_result__l_translate_statusCode;
         translate_browse_path_result__l_continue = ((translate_browse_path_result__l_continue == true) &&
            (((*translate_browse_path_result__statusCode_operation == constants_statuscodes_bs__e_sc_ok) ||
            (*translate_browse_path_result__statusCode_operation == constants_statuscodes_bs__e_sc_uncertain_reference_out_of_server)) ||
            (*translate_browse_path_result__statusCode_operation == constants_statuscodes_bs__e_sc_bad_no_match)));
      }
   }
}

void translate_browse_path_result__treat_browse_result_one_source_1(
   const t_entier4 translate_browse_path_result__path_index,
   const constants__t_QualifiedName_i translate_browse_path_result__targetName,
   const t_entier4 translate_browse_path_result__browseResult_index,
   const constants_statuscodes_bs__t_StatusCode_i translate_browse_path_result__in_statusCode,
   constants_statuscodes_bs__t_StatusCode_i * const translate_browse_path_result__statusCode_operation) {
   {
      constants__t_NodeId_i translate_browse_path_result__l_res_refTypeId;
      t_bool translate_browse_path_result__l_res_isForward;
      constants__t_ExpandedNodeId_i translate_browse_path_result__l_res_ExpandedNodeId;
      constants__t_QualifiedName_i translate_browse_path_result__l_res_BrowseName;
      constants__t_LocalizedText_i translate_browse_path_result__l_res_DisplayName;
      constants__t_NodeClass_i translate_browse_path_result__l_res_NodeClass;
      constants__t_ExpandedNodeId_i translate_browse_path_result__l_res_TypeDefinition;
      t_bool translate_browse_path_result__l_found;
      constants_statuscodes_bs__t_StatusCode_i translate_browse_path_result__l_translate_statusCode;
      t_bool translate_browse_path_result__l_local_server;
      constants__t_NodeId_i translate_browse_path_result__l_source_tmp;
      
      *translate_browse_path_result__statusCode_operation = translate_browse_path_result__in_statusCode;
      browse_treatment__getall_browse_result_reference_at(translate_browse_path_result__browseResult_index,
         &translate_browse_path_result__l_res_refTypeId,
         &translate_browse_path_result__l_res_isForward,
         &translate_browse_path_result__l_res_ExpandedNodeId,
         &translate_browse_path_result__l_res_BrowseName,
         &translate_browse_path_result__l_res_DisplayName,
         &translate_browse_path_result__l_res_NodeClass,
         &translate_browse_path_result__l_res_TypeDefinition);
      constants__is_QualifiedNames_Equal(translate_browse_path_result__targetName,
         translate_browse_path_result__l_res_BrowseName,
         &translate_browse_path_result__l_found);
      if (translate_browse_path_result__l_found == true) {
         translate_browse_path_result__checkAndAdd_BrowsePathResult(translate_browse_path_result__l_res_ExpandedNodeId,
            &translate_browse_path_result__l_translate_statusCode);
         if ((translate_browse_path_result__l_translate_statusCode == constants_statuscodes_bs__e_sc_ok) &&
            (*translate_browse_path_result__statusCode_operation == constants_statuscodes_bs__e_sc_uncertain_reference_out_of_server)) {
            ;
         }
         else {
            *translate_browse_path_result__statusCode_operation = translate_browse_path_result__l_translate_statusCode;
         }
      }
      else if (translate_browse_path_result__l_res_ExpandedNodeId != constants__c_ExpandedNodeId_indet) {
         constants__getall_conv_ExpandedNodeId_NodeId(translate_browse_path_result__l_res_ExpandedNodeId,
            &translate_browse_path_result__l_local_server,
            &translate_browse_path_result__l_source_tmp);
         if (translate_browse_path_result__l_local_server == false) {
            translate_browse_path_result__checkAndAdd_BrowsePathRemaining(translate_browse_path_result__l_res_ExpandedNodeId,
               translate_browse_path_result__path_index,
               translate_browse_path_result__statusCode_operation);
         }
      }
   }
}

void translate_browse_path_result__compute_browse_result_from_source(
   const constants__t_NodeId_i translate_browse_path_result__source,
   const constants__t_BrowseDirection_i translate_browse_path_result__browseDirection,
   const constants__t_NodeId_i translate_browse_path_result__referenceTypeId,
   const t_bool translate_browse_path_result__includedSubtypes,
   constants_statuscodes_bs__t_StatusCode_i * const translate_browse_path_result__statusCode_operation,
   t_entier4 * const translate_browse_path_result__nbReferences) {
   {
      constants_statuscodes_bs__t_StatusCode_i translate_browse_path_result__l_browse_statusCode;
      constants__t_ContinuationPointId_i translate_browse_path_result__l_continuationPoint;
      
      *translate_browse_path_result__nbReferences = 0;
      browse_treatment__set_browse_value_context(constants__c_session_indet,
         0,
         constants__c_NodeId_indet,
         translate_browse_path_result__source,
         translate_browse_path_result__browseDirection,
         translate_browse_path_result__referenceTypeId,
         translate_browse_path_result__includedSubtypes,
         constants__c_BrowseNodeClassMask_indet,
         constants__c_BrowseResultMask_all,
         false,
         translate_browse_path_result__statusCode_operation);
      if (*translate_browse_path_result__statusCode_operation == constants_statuscodes_bs__e_sc_ok) {
         browse_treatment__compute_browse_result(&translate_browse_path_result__l_browse_statusCode,
            &translate_browse_path_result__l_continuationPoint,
            translate_browse_path_result__nbReferences);
         browse_treatment__clear_browse_value_context();
         translate_browse_path_result__get_translateStatus_from_browseStatus(translate_browse_path_result__l_browse_statusCode,
            translate_browse_path_result__statusCode_operation);
      }
   }
}

void translate_browse_path_result__checkAndAdd_BrowsePathResult(
   const constants__t_ExpandedNodeId_i translate_browse_path_result__expandedNodeId,
   constants_statuscodes_bs__t_StatusCode_i * const translate_browse_path_result__statusCode_operation) {
   {
      t_bool translate_browse_path_result__l_result_isFull;
      t_bool translate_browse_path_result__l_alloc;
      constants__t_ExpandedNodeId_i translate_browse_path_result__l_target_copy;
      
      translate_browse_path_result_1__get_BrowsePathResult_IsFull(&translate_browse_path_result__l_result_isFull);
      if (translate_browse_path_result__l_result_isFull == false) {
         constants__get_copy_ExpandedNodeId(translate_browse_path_result__expandedNodeId,
            &translate_browse_path_result__l_alloc,
            &translate_browse_path_result__l_target_copy);
         if (translate_browse_path_result__l_alloc == true) {
            translate_browse_path_result_1__add_BrowsePathResult(translate_browse_path_result__l_target_copy);
            *translate_browse_path_result__statusCode_operation = constants_statuscodes_bs__e_sc_ok;
         }
         else {
            *translate_browse_path_result__statusCode_operation = constants_statuscodes_bs__e_sc_bad_out_of_memory;
         }
      }
      else {
         *translate_browse_path_result__statusCode_operation = constants_statuscodes_bs__e_sc_bad_query_too_complex;
      }
   }
}

void translate_browse_path_result__checkAndAdd_BrowsePathRemaining(
   const constants__t_ExpandedNodeId_i translate_browse_path_result__expandedNodeId,
   const t_entier4 translate_browse_path_result__path_index,
   constants_statuscodes_bs__t_StatusCode_i * const translate_browse_path_result__statusCode_operation) {
   {
      t_bool translate_browse_path_result__l_isFull;
      t_bool translate_browse_path_result__l_alloc;
      constants__t_ExpandedNodeId_i translate_browse_path_result__l_copy;
      
      translate_browse_path_result_1__get_BrowsePathRemaining_IsFull(&translate_browse_path_result__l_isFull);
      if (translate_browse_path_result__l_isFull == false) {
         constants__get_copy_ExpandedNodeId(translate_browse_path_result__expandedNodeId,
            &translate_browse_path_result__l_alloc,
            &translate_browse_path_result__l_copy);
         if (translate_browse_path_result__l_alloc == true) {
            translate_browse_path_result_1__add_BrowsePathResultRemaining(translate_browse_path_result__l_copy,
               translate_browse_path_result__path_index);
            *translate_browse_path_result__statusCode_operation = constants_statuscodes_bs__e_sc_uncertain_reference_out_of_server;
         }
         else {
            *translate_browse_path_result__statusCode_operation = constants_statuscodes_bs__e_sc_bad_out_of_memory;
         }
      }
      else {
         *translate_browse_path_result__statusCode_operation = constants_statuscodes_bs__e_sc_bad_query_too_complex;
      }
   }
}

void translate_browse_path_result__get_translateStatus_from_browseStatus(
   const constants_statuscodes_bs__t_StatusCode_i translate_browse_path_result__browse_statusCode,
   constants_statuscodes_bs__t_StatusCode_i * const translate_browse_path_result__translate_statusCode) {
   switch (translate_browse_path_result__browse_statusCode) {
   case constants_statuscodes_bs__e_sc_bad_node_id_unknown:
      *translate_browse_path_result__translate_statusCode = constants_statuscodes_bs__e_sc_bad_node_id_unknown;
      break;
   case constants_statuscodes_bs__e_sc_bad_reference_type_id_invalid:
      *translate_browse_path_result__translate_statusCode = constants_statuscodes_bs__e_sc_bad_no_match;
      break;
   case constants_statuscodes_bs__e_sc_bad_out_of_memory:
   case constants_statuscodes_bs__e_sc_bad_no_continuation_points:
   case constants_statuscodes_bs__e_sc_bad_view_id_unknown:
      *translate_browse_path_result__translate_statusCode = constants_statuscodes_bs__e_sc_bad_query_too_complex;
      break;
   case constants_statuscodes_bs__e_sc_ok:
      *translate_browse_path_result__translate_statusCode = constants_statuscodes_bs__e_sc_ok;
      break;
   default:
      *translate_browse_path_result__translate_statusCode = constants_statuscodes_bs__c_StatusCode_indet;
      break;
   }
}

void translate_browse_path_result__get_browseDirection_from_isInverse(
   const t_bool translate_browse_path_result__isInverse,
   constants__t_BrowseDirection_i * const translate_browse_path_result__browseDirection) {
   if (translate_browse_path_result__isInverse == true) {
      *translate_browse_path_result__browseDirection = constants__e_bd_inverse;
   }
   else {
      *translate_browse_path_result__browseDirection = constants__e_bd_forward;
   }
}

