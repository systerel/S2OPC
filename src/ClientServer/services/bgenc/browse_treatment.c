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

 File Name            : browse_treatment.c

 Date                 : 04/08/2022 14:53:02

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "browse_treatment.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void browse_treatment__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void browse_treatment__min_nb_result_refs(
   const t_entier4 browse_treatment__p_maxTargetRef,
   const t_entier4 browse_treatment__p_nb_target,
   t_entier4 * const browse_treatment__p_maxResultRefs) {
   {
      t_entier4 browse_treatment__l_maxTargetRef;
      
      if ((0 == browse_treatment__p_maxTargetRef) ||
         (browse_treatment__p_maxTargetRef >= constants__k_n_BrowseTarget_max)) {
         browse_treatment__l_maxTargetRef = constants__k_n_BrowseTarget_max;
      }
      else {
         browse_treatment__l_maxTargetRef = browse_treatment__p_maxTargetRef;
      }
      if (browse_treatment__p_nb_target < browse_treatment__l_maxTargetRef) {
         *browse_treatment__p_maxResultRefs = browse_treatment__p_nb_target;
      }
      else {
         *browse_treatment__p_maxResultRefs = browse_treatment__l_maxTargetRef;
      }
   }
}

void browse_treatment__local_is_valid_ReferenceTypeId(
   const t_bool browse_treatment__p_refType_defined,
   const constants__t_NodeId_i browse_treatment__p_referenceTypeId,
   t_bool * const browse_treatment__bres) {
   if (browse_treatment__p_refType_defined == true) {
      address_space_itf__is_valid_ReferenceTypeId(browse_treatment__p_referenceTypeId,
         browse_treatment__bres);
   }
   else {
      *browse_treatment__bres = true;
   }
}

void browse_treatment__apply_result_mask_filter(
   const constants__t_BrowseResultMask_i browse_treatment__p_resultMask,
   const constants__t_NodeId_i browse_treatment__p_RefType,
   const t_bool browse_treatment__p_IsForward,
   const constants__t_QualifiedName_i browse_treatment__p_BrowseName,
   const constants__t_LocalizedText_i browse_treatment__p_DisplayName,
   const constants__t_NodeClass_i browse_treatment__p_NodeClass,
   const constants__t_ExpandedNodeId_i browse_treatment__p_TypeDefinition,
   constants__t_NodeId_i * const browse_treatment__out_RefType,
   t_bool * const browse_treatment__out_IsForward,
   constants__t_QualifiedName_i * const browse_treatment__out_BrowseName,
   constants__t_LocalizedText_i * const browse_treatment__out_DisplayName,
   constants__t_NodeClass_i * const browse_treatment__out_NodeClass,
   constants__t_ExpandedNodeId_i * const browse_treatment__out_TypeDefinition) {
   {
      t_bool browse_treatment__is_in_mask;
      
      browse_treatment_result_bs__is_BrowseName_in_mask(browse_treatment__p_resultMask,
         &browse_treatment__is_in_mask);
      if (browse_treatment__is_in_mask == true) {
         *browse_treatment__out_BrowseName = browse_treatment__p_BrowseName;
      }
      else {
         *browse_treatment__out_BrowseName = constants__c_QualifiedName_indet;
      }
      browse_treatment_result_bs__is_DisplayName_in_mask(browse_treatment__p_resultMask,
         &browse_treatment__is_in_mask);
      if (browse_treatment__is_in_mask == true) {
         *browse_treatment__out_DisplayName = browse_treatment__p_DisplayName;
      }
      else {
         *browse_treatment__out_DisplayName = constants__c_LocalizedText_indet;
      }
      browse_treatment_result_bs__is_NodeClass_in_mask(browse_treatment__p_resultMask,
         &browse_treatment__is_in_mask);
      if (browse_treatment__is_in_mask == true) {
         *browse_treatment__out_NodeClass = browse_treatment__p_NodeClass;
      }
      else {
         *browse_treatment__out_NodeClass = constants__c_NodeClass_indet;
      }
      browse_treatment_result_bs__is_TypeDefinition_in_mask(browse_treatment__p_resultMask,
         &browse_treatment__is_in_mask);
      if (browse_treatment__is_in_mask == true) {
         *browse_treatment__out_TypeDefinition = browse_treatment__p_TypeDefinition;
      }
      else {
         *browse_treatment__out_TypeDefinition = constants__c_ExpandedNodeId_indet;
      }
      browse_treatment_result_bs__is_ReferenceType_in_mask(browse_treatment__p_resultMask,
         &browse_treatment__is_in_mask);
      if (browse_treatment__is_in_mask == true) {
         *browse_treatment__out_RefType = browse_treatment__p_RefType;
      }
      else {
         *browse_treatment__out_RefType = constants__c_NodeId_indet;
      }
      browse_treatment_result_bs__is_IsForward_in_mask(browse_treatment__p_resultMask,
         &browse_treatment__is_in_mask);
      if (browse_treatment__is_in_mask == true) {
         *browse_treatment__out_IsForward = browse_treatment__p_IsForward;
      }
      else {
         *browse_treatment__out_IsForward = false;
      }
   }
}

void browse_treatment__fill_browse_result_ref(
   const constants__t_Reference_i browse_treatment__p_ref,
   const constants__t_NodeId_i browse_treatment__p_browseView,
   const constants__t_BrowseDirection_i browse_treatment__p_browseDirection,
   const t_bool browse_treatment__p_refType_defined,
   const constants__t_NodeId_i browse_treatment__p_referenceType,
   const t_bool browse_treatment__p_includeSubtypes,
   const constants__t_BrowseResultMask_i browse_treatment__p_resultMask,
   t_bool * const browse_treatment__p_continue,
   t_bool * const browse_treatment__p_alloc_failed) {
   {
      constants__t_NodeId_i browse_treatment__l_RefType;
      constants__t_ExpandedNodeId_i browse_treatment__l_TargetNode;
      t_bool browse_treatment__l_IsForward;
      t_bool browse_treatment__l_res;
      t_entier4 browse_treatment__l_bri;
      constants__t_QualifiedName_i browse_treatment__l_BrowseName;
      constants__t_LocalizedText_i browse_treatment__l_DisplayName;
      constants__t_NodeClass_i browse_treatment__l_NodeClass;
      constants__t_ExpandedNodeId_i browse_treatment__l_TypeDefinition;
      t_bool browse_treatment__l_NodeClassInMask;
      constants__t_NodeId_i browse_treatment__l_RefType2;
      t_bool browse_treatment__l_IsForward2;
      constants__t_QualifiedName_i browse_treatment__l_BrowseName2;
      constants__t_LocalizedText_i browse_treatment__l_DisplayName2;
      constants__t_NodeClass_i browse_treatment__l_NodeClass2;
      constants__t_ExpandedNodeId_i browse_treatment__l_TypeDefinition2;
      
      *browse_treatment__p_alloc_failed = false;
      *browse_treatment__p_continue = true;
      browse_treatment_result_bs__unused_browse_view(browse_treatment__p_browseView);
      address_space_itf__get_Reference_ReferenceType(browse_treatment__p_ref,
         &browse_treatment__l_RefType);
      address_space_itf__get_Reference_TargetNode(browse_treatment__p_ref,
         &browse_treatment__l_TargetNode);
      address_space_itf__get_Reference_IsForward(browse_treatment__p_ref,
         &browse_treatment__l_IsForward);
      constants__get_Is_Dir_Forward_Compatible(browse_treatment__p_browseDirection,
         browse_treatment__l_IsForward,
         &browse_treatment__l_res);
      if (browse_treatment__l_res == true) {
         browse_treatment_1__Is_RefTypes_Compatible(browse_treatment__p_refType_defined,
            browse_treatment__p_referenceType,
            browse_treatment__p_includeSubtypes,
            browse_treatment__l_RefType,
            &browse_treatment__l_res);
         if (browse_treatment__l_res == true) {
            browse_treatment_1__get_optional_fields_ReferenceDescription(browse_treatment__l_TargetNode,
               &browse_treatment__l_BrowseName,
               &browse_treatment__l_DisplayName,
               &browse_treatment__l_NodeClass,
               &browse_treatment__l_TypeDefinition);
            browse_treatment_context__is_NodeClass_in_NodeClassMask(browse_treatment__l_NodeClass,
               &browse_treatment__l_NodeClassInMask);
            if (browse_treatment__l_NodeClassInMask == true) {
               browse_treatment_result_it__continue_iter_browseResult(browse_treatment__p_continue,
                  &browse_treatment__l_bri);
               browse_treatment__apply_result_mask_filter(browse_treatment__p_resultMask,
                  browse_treatment__l_RefType,
                  browse_treatment__l_IsForward,
                  browse_treatment__l_BrowseName,
                  browse_treatment__l_DisplayName,
                  browse_treatment__l_NodeClass,
                  browse_treatment__l_TypeDefinition,
                  &browse_treatment__l_RefType2,
                  &browse_treatment__l_IsForward2,
                  &browse_treatment__l_BrowseName2,
                  &browse_treatment__l_DisplayName2,
                  &browse_treatment__l_NodeClass2,
                  &browse_treatment__l_TypeDefinition2);
               browse_treatment_result_bs__setall_browse_result_reference_at(browse_treatment__l_bri,
                  browse_treatment__l_RefType2,
                  browse_treatment__l_IsForward2,
                  browse_treatment__l_TargetNode,
                  browse_treatment__l_BrowseName2,
                  browse_treatment__l_DisplayName2,
                  browse_treatment__l_NodeClass2,
                  browse_treatment__l_TypeDefinition2,
                  browse_treatment__p_alloc_failed);
            }
         }
      }
      if (*browse_treatment__p_alloc_failed == true) {
         *browse_treatment__p_continue = false;
      }
   }
}

void browse_treatment__fill_browse_result(
   const t_entier4 browse_treatment__p_startIndex,
   const t_entier4 browse_treatment__p_max_nb_results,
   const constants__t_NodeId_i browse_treatment__p_browseView,
   const constants__t_Node_i browse_treatment__p_src_node,
   const constants__t_BrowseDirection_i browse_treatment__p_browseDirection,
   const t_bool browse_treatment__p_refType_defined,
   const constants__t_NodeId_i browse_treatment__p_referenceType,
   const t_bool browse_treatment__p_includeSubtypes,
   const constants__t_BrowseResultMask_i browse_treatment__p_resultMask,
   constants_statuscodes_bs__t_StatusCode_i * const browse_treatment__p_serviceStatusCode,
   t_bool * const browse_treatment__p_toContinue,
   t_entier4 * const browse_treatment__p_nextIndex) {
   {
      t_bool browse_treatment__l_continue_bri;
      t_bool browse_treatment__l_continue_ref;
      constants__t_Reference_i browse_treatment__l_ref;
      t_bool browse_treatment__l_alloc_failed;
      
      *browse_treatment__p_serviceStatusCode = constants_statuscodes_bs__e_sc_ok;
      browse_treatment__l_ref = constants__c_Reference_indet;
      browse_treatment__l_alloc_failed = false;
      browse_treatment_result_it__init_iter_browseResult(browse_treatment__p_max_nb_results,
         &browse_treatment__l_continue_bri);
      browse_treatment_target_it__init_iter_reference(browse_treatment__p_src_node,
         browse_treatment__p_startIndex,
         &browse_treatment__l_continue_ref);
      *browse_treatment__p_nextIndex = browse_treatment__p_startIndex;
      while ((browse_treatment__l_continue_ref == true) &&
         (browse_treatment__l_continue_bri == true)) {
         browse_treatment_target_it__continue_iter_reference(&browse_treatment__l_continue_ref,
            &browse_treatment__l_ref,
            browse_treatment__p_nextIndex);
         browse_treatment__fill_browse_result_ref(browse_treatment__l_ref,
            browse_treatment__p_browseView,
            browse_treatment__p_browseDirection,
            browse_treatment__p_refType_defined,
            browse_treatment__p_referenceType,
            browse_treatment__p_includeSubtypes,
            browse_treatment__p_resultMask,
            &browse_treatment__l_continue_bri,
            &browse_treatment__l_alloc_failed);
      }
      *browse_treatment__p_toContinue = (((browse_treatment__l_continue_ref == true) &&
         (browse_treatment__l_continue_bri == false)) &&
         (browse_treatment__l_alloc_failed == false));
      if (browse_treatment__l_alloc_failed == true) {
         *browse_treatment__p_serviceStatusCode = constants_statuscodes_bs__e_sc_bad_out_of_memory;
      }
   }
}

void browse_treatment__set_browse_value_context(
   const constants__t_session_i browse_treatment__p_session,
   const t_entier4 browse_treatment__p_maxTargetRef,
   const constants__t_NodeId_i browse_treatment__p_browseView,
   const constants__t_NodeId_i browse_treatment__p_nodeId,
   const constants__t_BrowseDirection_i browse_treatment__p_browseDirection,
   const constants__t_NodeId_i browse_treatment__p_referenceType,
   const t_bool browse_treatment__p_includeSubtypes,
   const constants__t_BrowseNodeClassMask_i browse_treatment__p_nodeClassMask,
   const constants__t_BrowseResultMask_i browse_treatment__p_resultMask,
   const t_bool browse_treatment__p_autoReleaseCP,
   constants_statuscodes_bs__t_StatusCode_i * const browse_treatment__p_service_StatusCode) {
   browse_treatment_context__setall_browse_value_context(1,
      browse_treatment__p_session,
      browse_treatment__p_maxTargetRef,
      browse_treatment__p_browseView,
      browse_treatment__p_nodeId,
      browse_treatment__p_browseDirection,
      browse_treatment__p_referenceType,
      browse_treatment__p_includeSubtypes,
      browse_treatment__p_nodeClassMask,
      browse_treatment__p_resultMask,
      browse_treatment__p_autoReleaseCP,
      browse_treatment__p_service_StatusCode);
}

void browse_treatment__set_browse_value_context_from_continuation_point(
   const constants__t_session_i browse_treatment__p_session,
   const constants__t_ContinuationPointId_i browse_treatment__p_continuationPointId,
   constants_statuscodes_bs__t_StatusCode_i * const browse_treatment__p_service_StatusCode) {
   {
      t_bool browse_treatment__l_res;
      t_entier4 browse_treatment__l_nextIndex;
      t_entier4 browse_treatment__l_maxTargetRef;
      constants__t_NodeId_i browse_treatment__l_browseView;
      constants__t_NodeId_i browse_treatment__l_nodeId;
      constants__t_BrowseDirection_i browse_treatment__l_browseDirection;
      constants__t_NodeId_i browse_treatment__l_referenceType;
      t_bool browse_treatment__l_includeSubtypes;
      constants__t_BrowseNodeClassMask_i browse_treatment__l_nodeClassMask;
      constants__t_BrowseResultMask_i browse_treatment__l_resultMask;
      t_bool browse_treatment__l_autoReleaseCP;
      
      *browse_treatment__p_service_StatusCode = constants_statuscodes_bs__e_sc_bad_continuation_point_invalid;
      browse_treatment__l_autoReleaseCP = false;
      browse_treatment_continuation_points__getall_continuation_point(browse_treatment__p_session,
         browse_treatment__p_continuationPointId,
         &browse_treatment__l_res,
         &browse_treatment__l_nextIndex,
         &browse_treatment__l_maxTargetRef,
         &browse_treatment__l_browseView,
         &browse_treatment__l_nodeId,
         &browse_treatment__l_browseDirection,
         &browse_treatment__l_referenceType,
         &browse_treatment__l_includeSubtypes,
         &browse_treatment__l_nodeClassMask,
         &browse_treatment__l_resultMask);
      if (browse_treatment__l_res == true) {
         browse_treatment_context__setall_browse_value_context(browse_treatment__l_nextIndex,
            browse_treatment__p_session,
            browse_treatment__l_maxTargetRef,
            browse_treatment__l_browseView,
            browse_treatment__l_nodeId,
            browse_treatment__l_browseDirection,
            browse_treatment__l_referenceType,
            browse_treatment__l_includeSubtypes,
            browse_treatment__l_nodeClassMask,
            browse_treatment__l_resultMask,
            browse_treatment__l_autoReleaseCP,
            browse_treatment__p_service_StatusCode);
         if (*browse_treatment__p_service_StatusCode == constants_statuscodes_bs__e_sc_ok) {
            browse_treatment_continuation_points__release_continuation_point(browse_treatment__p_session,
               browse_treatment__p_continuationPointId,
               &browse_treatment__l_res);
         }
      }
   }
}

void browse_treatment__compute_browse_result(
   constants_statuscodes_bs__t_StatusCode_i * const browse_treatment__p_serviceStatusCode,
   constants__t_ContinuationPointId_i * const browse_treatment__p_continuationPointId,
   t_entier4 * const browse_treatment__p_nbReferences) {
   {
      t_entier4 browse_treatment__l_startIndex;
      constants__t_session_i browse_treatment__l_session;
      t_entier4 browse_treatment__l_maxTargetRef;
      constants__t_NodeId_i browse_treatment__l_browseView;
      constants__t_NodeId_i browse_treatment__l_nodeId;
      constants__t_BrowseDirection_i browse_treatment__l_browseDirection;
      t_bool browse_treatment__l_refType_defined;
      constants__t_NodeId_i browse_treatment__l_referenceType;
      t_bool browse_treatment__l_includeSubtypes;
      constants__t_BrowseNodeClassMask_i browse_treatment__l_nodeClassMask;
      constants__t_BrowseResultMask_i browse_treatment__l_resultMask;
      t_bool browse_treatment__l_autoReleaseCP;
      t_bool browse_treatment__l_is_ref_type_id_valid;
      t_bool browse_treatment__l_is_src_node_valid;
      t_entier4 browse_treatment__l_nb_target;
      constants__t_Node_i browse_treatment__l_src_node;
      t_bool browse_treatment__l_alloc_bres;
      t_entier4 browse_treatment__l_max_nb_results;
      t_bool browse_treatment__l_toContinue;
      t_entier4 browse_treatment__l_nextIndex;
      t_bool browse_treatment__l_has_continuation_point;
      constants__t_ContinuationPointId_i browse_treatment__l_prev_cp_id;
      t_bool browse_treatment__l_cp_bres;
      
      *browse_treatment__p_continuationPointId = constants__c_ContinuationPointId_indet;
      *browse_treatment__p_nbReferences = 0;
      browse_treatment_context__getall_browse_value_context(&browse_treatment__l_startIndex,
         &browse_treatment__l_session,
         &browse_treatment__l_maxTargetRef,
         &browse_treatment__l_browseView,
         &browse_treatment__l_nodeId,
         &browse_treatment__l_browseDirection,
         &browse_treatment__l_refType_defined,
         &browse_treatment__l_referenceType,
         &browse_treatment__l_includeSubtypes,
         &browse_treatment__l_nodeClassMask,
         &browse_treatment__l_resultMask,
         &browse_treatment__l_autoReleaseCP);
      browse_treatment__local_is_valid_ReferenceTypeId(browse_treatment__l_refType_defined,
         browse_treatment__l_referenceType,
         &browse_treatment__l_is_ref_type_id_valid);
      browse_treatment_1__getall_SourceNode_NbRef(browse_treatment__l_nodeId,
         &browse_treatment__l_is_src_node_valid,
         &browse_treatment__l_nb_target,
         &browse_treatment__l_src_node);
      if ((browse_treatment__l_is_ref_type_id_valid == true) &&
         (browse_treatment__l_is_src_node_valid == true)) {
         browse_treatment__min_nb_result_refs(browse_treatment__l_maxTargetRef,
            browse_treatment__l_nb_target,
            &browse_treatment__l_max_nb_results);
         browse_treatment_result_bs__alloc_browse_result(browse_treatment__l_max_nb_results,
            &browse_treatment__l_alloc_bres);
         if (browse_treatment__l_alloc_bres == true) {
            browse_treatment__fill_browse_result(browse_treatment__l_startIndex,
               browse_treatment__l_max_nb_results,
               browse_treatment__l_browseView,
               browse_treatment__l_src_node,
               browse_treatment__l_browseDirection,
               browse_treatment__l_refType_defined,
               browse_treatment__l_referenceType,
               browse_treatment__l_includeSubtypes,
               browse_treatment__l_resultMask,
               browse_treatment__p_serviceStatusCode,
               &browse_treatment__l_toContinue,
               &browse_treatment__l_nextIndex);
            if (*browse_treatment__p_serviceStatusCode == constants_statuscodes_bs__e_sc_ok) {
               browse_treatment_result_bs__get_browse_result_nb_references(browse_treatment__p_nbReferences);
               if (browse_treatment__l_toContinue == true) {
                  browse_treatment_continuation_points__has_continuation_point(browse_treatment__l_session,
                     &browse_treatment__l_has_continuation_point,
                     &browse_treatment__l_prev_cp_id);
                  if ((browse_treatment__l_autoReleaseCP == true) &&
                     (browse_treatment__l_has_continuation_point == true)) {
                     browse_treatment_continuation_points__release_continuation_point(browse_treatment__l_session,
                        browse_treatment__l_prev_cp_id,
                        &browse_treatment__l_cp_bres);
                  }
                  browse_treatment_continuation_points__create_continuation_point(browse_treatment__l_session,
                     browse_treatment__l_nextIndex,
                     browse_treatment__l_maxTargetRef,
                     browse_treatment__l_browseView,
                     browse_treatment__l_nodeId,
                     browse_treatment__l_browseDirection,
                     browse_treatment__l_referenceType,
                     browse_treatment__l_includeSubtypes,
                     browse_treatment__l_nodeClassMask,
                     browse_treatment__l_resultMask,
                     &browse_treatment__l_cp_bres,
                     browse_treatment__p_continuationPointId);
                  if (browse_treatment__l_cp_bres == false) {
                     *browse_treatment__p_serviceStatusCode = constants_statuscodes_bs__e_sc_bad_no_continuation_points;
                  }
               }
            }
         }
         else {
            *browse_treatment__p_serviceStatusCode = constants_statuscodes_bs__e_sc_bad_out_of_memory;
         }
      }
      else {
         if (browse_treatment__l_is_ref_type_id_valid == false) {
            *browse_treatment__p_serviceStatusCode = constants_statuscodes_bs__e_sc_bad_reference_type_id_invalid;
         }
         else {
            *browse_treatment__p_serviceStatusCode = constants_statuscodes_bs__e_sc_bad_node_id_unknown;
         }
      }
   }
}

