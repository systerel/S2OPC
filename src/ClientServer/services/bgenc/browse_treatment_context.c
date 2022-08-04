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

 File Name            : browse_treatment_context.c

 Date                 : 04/08/2022 14:53:00

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "browse_treatment_context.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
constants__t_BrowseDirection_i browse_treatment_context__in_BrowseValue_BrowseDirection_i;
t_bool browse_treatment_context__in_BrowseValue_IncludeSubtypes_i;
constants__t_BrowseNodeClassMask_i browse_treatment_context__in_BrowseValue_NodeClassMask_i;
constants__t_NodeId_i browse_treatment_context__in_BrowseValue_NodeId_i;
constants__t_NodeId_i browse_treatment_context__in_BrowseValue_ReferenceTypeId_i;
constants__t_BrowseResultMask_i browse_treatment_context__in_BrowseValue_ResultMask_i;
constants__t_NodeId_i browse_treatment_context__in_BrowseView_i;
t_bool browse_treatment_context__in_ReleasePrevContinuationPoint_i;
t_entier4 browse_treatment_context__in_maxReferencesPerNode_i;
constants__t_session_i browse_treatment_context__in_session_i;
t_entier4 browse_treatment_context__in_startIndex_i;
t_bool browse_treatment_context__isBrowseValueContextDefined_i;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void browse_treatment_context__INITIALISATION(void) {
   browse_treatment_context__in_startIndex_i = 0;
   browse_treatment_context__in_session_i = constants__c_session_indet;
   browse_treatment_context__in_maxReferencesPerNode_i = 0;
   browse_treatment_context__in_BrowseView_i = constants__c_NodeId_indet;
   browse_treatment_context__in_BrowseValue_NodeId_i = constants__c_NodeId_indet;
   browse_treatment_context__in_BrowseValue_BrowseDirection_i = constants__e_bd_indet;
   browse_treatment_context__in_BrowseValue_ReferenceTypeId_i = constants__c_NodeId_indet;
   browse_treatment_context__in_BrowseValue_IncludeSubtypes_i = false;
   browse_treatment_context__in_BrowseValue_NodeClassMask_i = constants__c_BrowseNodeClassMask_indet;
   browse_treatment_context__in_BrowseValue_ResultMask_i = constants__c_BrowseResultMask_indet;
   browse_treatment_context__in_ReleasePrevContinuationPoint_i = false;
   browse_treatment_context__isBrowseValueContextDefined_i = false;
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void browse_treatment_context__local_clear_browse_value_context(void) {
   browse_treatment_context__in_startIndex_i = 0;
   browse_treatment_context__in_session_i = constants__c_session_indet;
   browse_treatment_context__in_maxReferencesPerNode_i = 0;
   node_id_pointer_bs__free_node_id_pointer(browse_treatment_context__in_BrowseView_i);
   browse_treatment_context__in_BrowseView_i = constants__c_NodeId_indet;
   node_id_pointer_bs__free_node_id_pointer(browse_treatment_context__in_BrowseValue_NodeId_i);
   browse_treatment_context__in_BrowseValue_NodeId_i = constants__c_NodeId_indet;
   browse_treatment_context__in_BrowseValue_BrowseDirection_i = constants__e_bd_indet;
   node_id_pointer_bs__free_node_id_pointer(browse_treatment_context__in_BrowseValue_ReferenceTypeId_i);
   browse_treatment_context__in_BrowseValue_ReferenceTypeId_i = constants__c_NodeId_indet;
   browse_treatment_context__in_BrowseValue_IncludeSubtypes_i = false;
   browse_treatment_context__in_BrowseValue_NodeClassMask_i = constants__c_BrowseNodeClassMask_indet;
   browse_treatment_context__in_BrowseValue_ResultMask_i = constants__c_BrowseResultMask_indet;
   browse_treatment_context__in_ReleasePrevContinuationPoint_i = false;
   browse_treatment_context__isBrowseValueContextDefined_i = false;
}

void browse_treatment_context__setall_browse_value_context(
   const t_entier4 browse_treatment_context__p_startIndex,
   const constants__t_session_i browse_treatment_context__p_session,
   const t_entier4 browse_treatment_context__p_maxTargetRef,
   const constants__t_NodeId_i browse_treatment_context__p_browseView,
   const constants__t_NodeId_i browse_treatment_context__p_nodeId,
   const constants__t_BrowseDirection_i browse_treatment_context__p_browseDirection,
   const constants__t_NodeId_i browse_treatment_context__p_referenceType,
   const t_bool browse_treatment_context__p_includeSubtypes,
   const constants__t_BrowseNodeClassMask_i browse_treatment_context__p_nodeClassMask,
   const constants__t_BrowseResultMask_i browse_treatment_context__p_resultMask,
   const t_bool browse_treatment_context__p_autoReleaseCP,
   constants_statuscodes_bs__t_StatusCode_i * const browse_treatment_context__p_service_StatusCode) {
   {
      t_bool browse_treatment_context__l_bresView;
      t_bool browse_treatment_context__l_bresSrc;
      t_bool browse_treatment_context__l_bresRef;
      
      node_id_pointer_bs__copy_node_id_pointer_content(browse_treatment_context__p_browseView,
         &browse_treatment_context__l_bresView,
         &browse_treatment_context__in_BrowseView_i);
      node_id_pointer_bs__copy_node_id_pointer_content(browse_treatment_context__p_nodeId,
         &browse_treatment_context__l_bresSrc,
         &browse_treatment_context__in_BrowseValue_NodeId_i);
      node_id_pointer_bs__copy_node_id_pointer_content(browse_treatment_context__p_referenceType,
         &browse_treatment_context__l_bresRef,
         &browse_treatment_context__in_BrowseValue_ReferenceTypeId_i);
      browse_treatment_context__in_startIndex_i = browse_treatment_context__p_startIndex;
      browse_treatment_context__in_session_i = browse_treatment_context__p_session;
      browse_treatment_context__in_maxReferencesPerNode_i = browse_treatment_context__p_maxTargetRef;
      browse_treatment_context__in_BrowseValue_BrowseDirection_i = browse_treatment_context__p_browseDirection;
      browse_treatment_context__in_BrowseValue_IncludeSubtypes_i = browse_treatment_context__p_includeSubtypes;
      browse_treatment_context__in_BrowseValue_NodeClassMask_i = browse_treatment_context__p_nodeClassMask;
      browse_treatment_context__in_BrowseValue_ResultMask_i = browse_treatment_context__p_resultMask;
      browse_treatment_context__in_ReleasePrevContinuationPoint_i = browse_treatment_context__p_autoReleaseCP;
      browse_treatment_context__isBrowseValueContextDefined_i = true;
      if (((browse_treatment_context__l_bresView == true) &&
         (browse_treatment_context__l_bresSrc == true)) &&
         (browse_treatment_context__l_bresRef == true)) {
         *browse_treatment_context__p_service_StatusCode = constants_statuscodes_bs__e_sc_ok;
      }
      else {
         *browse_treatment_context__p_service_StatusCode = constants_statuscodes_bs__e_sc_bad_out_of_memory;
         browse_treatment_context__local_clear_browse_value_context();
      }
   }
}

void browse_treatment_context__getall_browse_value_context(
   t_entier4 * const browse_treatment_context__p_startIndex,
   constants__t_session_i * const browse_treatment_context__p_session,
   t_entier4 * const browse_treatment_context__p_maxTargetRef,
   constants__t_NodeId_i * const browse_treatment_context__p_browseView,
   constants__t_NodeId_i * const browse_treatment_context__p_nodeId,
   constants__t_BrowseDirection_i * const browse_treatment_context__p_browseDirection,
   t_bool * const browse_treatment_context__p_refType_defined,
   constants__t_NodeId_i * const browse_treatment_context__p_referenceType,
   t_bool * const browse_treatment_context__p_includeSubtypes,
   constants__t_BrowseNodeClassMask_i * const browse_treatment_context__p_nodeClassMask,
   constants__t_BrowseResultMask_i * const browse_treatment_context__p_resultMask,
   t_bool * const browse_treatment_context__p_autoReleaseCP) {
   *browse_treatment_context__p_startIndex = browse_treatment_context__in_startIndex_i;
   *browse_treatment_context__p_session = browse_treatment_context__in_session_i;
   *browse_treatment_context__p_maxTargetRef = browse_treatment_context__in_maxReferencesPerNode_i;
   *browse_treatment_context__p_browseView = browse_treatment_context__in_BrowseView_i;
   *browse_treatment_context__p_nodeId = browse_treatment_context__in_BrowseValue_NodeId_i;
   *browse_treatment_context__p_browseDirection = browse_treatment_context__in_BrowseValue_BrowseDirection_i;
   *browse_treatment_context__p_refType_defined = (browse_treatment_context__in_BrowseValue_ReferenceTypeId_i != constants__c_NodeId_indet);
   *browse_treatment_context__p_referenceType = browse_treatment_context__in_BrowseValue_ReferenceTypeId_i;
   *browse_treatment_context__p_includeSubtypes = browse_treatment_context__in_BrowseValue_IncludeSubtypes_i;
   *browse_treatment_context__p_nodeClassMask = browse_treatment_context__in_BrowseValue_NodeClassMask_i;
   *browse_treatment_context__p_resultMask = browse_treatment_context__in_BrowseValue_ResultMask_i;
   *browse_treatment_context__p_autoReleaseCP = browse_treatment_context__in_ReleasePrevContinuationPoint_i;
}

void browse_treatment_context__is_NodeClass_in_NodeClassMask(
   const constants__t_NodeClass_i browse_treatment_context__p_nodeClass,
   t_bool * const browse_treatment_context__bres) {
   if (browse_treatment_context__p_nodeClass == constants__c_NodeClass_indet) {
      *browse_treatment_context__bres = true;
   }
   else {
      browse_treatment_context_bs__is_NodeClass_in_NodeClassMask_bs(browse_treatment_context__p_nodeClass,
         browse_treatment_context__in_BrowseValue_NodeClassMask_i,
         browse_treatment_context__bres);
   }
}

void browse_treatment_context__clear_browse_value_context(void) {
   browse_treatment_context__local_clear_browse_value_context();
}

