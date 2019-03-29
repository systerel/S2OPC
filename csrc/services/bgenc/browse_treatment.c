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

 Date                 : 05/04/2019 08:04:44

 C Translator Version : tradc Java V1.0 (14/03/2012)

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
void browse_treatment__set_browse_value_context(
   const constants__t_session_i browse_treatment__p_session,
   const t_entier4 browse_treatment__p_maxTargetRef,
   const constants__t_NodeId_i browse_treatment__p_browseView,
   const constants__t_NodeId_i browse_treatment__p_nodeId,
   const constants__t_BrowseDirection_i browse_treatment__p_browseDirection,
   const constants__t_NodeId_i browse_treatment__p_referenceType,
   const t_bool browse_treatment__p_includeSubtypes) {
   browse_treatment_context__setall_browse_value_context(0,
      browse_treatment__p_session,
      browse_treatment__p_maxTargetRef,
      browse_treatment__p_browseView,
      browse_treatment__p_nodeId,
      browse_treatment__p_browseDirection,
      browse_treatment__p_referenceType,
      browse_treatment__p_includeSubtypes);
}

void browse_treatment__set_browse_value_context_from_continuation_point(
   const constants__t_session_i browse_treatment__p_session,
   const constants__t_ContinuationPoint_i browse_treatment__p_continuationPoint,
   constants_statuscodes_bs__t_StatusCode_i * const browse_treatment__p_service_StatusCode) {
   *browse_treatment__p_service_StatusCode = constants_statuscodes_bs__e_sc_bad_continuation_point_invalid;
}

void browse_treatment__compute_browse_result(
   constants_statuscodes_bs__t_StatusCode_i * const browse_treatment__p_serviceStatusCode,
   constants__t_ContinuationPoint_i * const browse_treatment__p_continuationPoint,
   t_entier4 * const browse_treatment__p_nbReferences) {
   *browse_treatment__p_serviceStatusCode = constants_statuscodes_bs__e_sc_bad_out_of_memory;
   *browse_treatment__p_continuationPoint = constants__c_ContinuationPoint_indet;
   *browse_treatment__p_nbReferences = 0;
}

void browse_treatment__set_session_closed(
   const constants__t_session_i browse_treatment__p_session) {
   ;
}

void browse_treatment__getall_browse_result(
   const t_entier4 browse_treatment__p_refIndex,
   constants__t_NodeId_i * const browse_treatment__p_refTypeId,
   t_bool * const browse_treatment__p_isForward,
   constants__t_ExpandedNodeId_i * const browse_treatment__p_NodeId,
   constants__t_QualifiedName_i * const browse_treatment__p_BrowseName,
   constants__t_LocalizedText_i * const browse_treatment__p_DisplayName,
   constants__t_NodeClass_i * const browse_treatment__p_NodeClass,
   constants__t_ExpandedNodeId_i * const browse_treatment__p_TypeDefinition) {
   *browse_treatment__p_refTypeId = constants__c_NodeId_indet;
   *browse_treatment__p_isForward = true;
   *browse_treatment__p_NodeId = constants__c_ExpandedNodeId_indet;
   *browse_treatment__p_BrowseName = constants__c_QualifiedName_indet;
   *browse_treatment__p_DisplayName = constants__c_LocalizedText_indet;
   *browse_treatment__p_NodeClass = constants__c_NodeClass_indet;
   *browse_treatment__p_TypeDefinition = constants__c_ExpandedNodeId_indet;
}

void browse_treatment__getall_and_clear_browse_results(
   constants__t_BrowseResult_i * const browse_treatment__p_browseResult) {
   *browse_treatment__p_browseResult = constants__c_BrowseResult_indet;
}

void browse_treatment__clear_browse_results(void) {
   ;
}

