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

 Date                 : 14/06/2019 07:38:03

 C Translator Version : tradc Java V1.0 (14/03/2012)

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
      t_bool translate_browse_path__l_continue;
      constants__t_RelativePath_i translate_browse_path__l_relativePath;
      constants__t_RelativePathElt_i translate_browse_path__l_relativePathElt;
      t_entier4 translate_browse_path__l_index;
      t_bool translate_browse_path__l_local_server;
      constants__t_NodeId_i translate_browse_path__l_source;
      constants__t_ExpandedNodeId_i translate_browse_path__l_target;
      constants__t_ExpandedNodeId_i translate_browse_path__l_previous_target;
      constants_statuscodes_bs__t_StatusCode_i translate_browse_path__l_statusCode_alloc;
      
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
         translate_browse_path__l_target = constants__c_ExpandedNodeId_indet;
         translate_browse_path__l_previous_target = constants__c_ExpandedNodeId_indet;
         translate_browse_path__l_index = 0;
         translate_browse_path__l_statusCode_operation = constants_statuscodes_bs__e_sc_bad_nothing_to_do;
         while (translate_browse_path__l_continue == true) {
            translate_browse_path_element_it__continue_iter_relativePath(&translate_browse_path__l_continue,
               &translate_browse_path__l_relativePathElt,
               &translate_browse_path__l_index);
            translate_browse_path__treat_one_relative_path_element(translate_browse_path__l_source,
               translate_browse_path__l_relativePathElt,
               &translate_browse_path__l_statusCode_operation,
               &translate_browse_path__l_target);
            constants__free_ExpandedNodeId(translate_browse_path__l_previous_target);
            translate_browse_path__l_source = constants__c_NodeId_indet;
            translate_browse_path__l_previous_target = translate_browse_path__l_target;
            if (translate_browse_path__l_statusCode_operation == constants_statuscodes_bs__e_sc_ok) {
               constants__getall_conv_ExpandedNodeId_NodeId(translate_browse_path__l_target,
                  &translate_browse_path__l_local_server,
                  &translate_browse_path__l_source);
            }
            translate_browse_path__l_continue = ((translate_browse_path__l_continue == true) &&
               (translate_browse_path__l_statusCode_operation == constants_statuscodes_bs__e_sc_ok));
         }
         msg_translate_browse_path_bs__write_BrowsePath_Res_StatusCode(translate_browse_path__browsePath,
            translate_browse_path__l_statusCode_operation);
         if ((translate_browse_path__l_statusCode_operation == constants_statuscodes_bs__e_sc_ok) ||
            (translate_browse_path__l_statusCode_operation == constants_statuscodes_bs__e_sc_uncertain_reference_out_of_server)) {
            msg_translate_browse_path_bs__alloc_BrowsePath_Res_Target(translate_browse_path__browsePath,
               1,
               &translate_browse_path__l_statusCode_alloc);
            if (translate_browse_path__l_statusCode_alloc == constants_statuscodes_bs__e_sc_ok) {
               if (translate_browse_path__l_statusCode_operation == constants_statuscodes_bs__e_sc_ok) {
                  msg_translate_browse_path_bs__add_BrowsePath_Res_Target(translate_browse_path__browsePath,
                     translate_browse_path__l_target,
                     &translate_browse_path__l_statusCode_alloc);
               }
               else {
                  msg_translate_browse_path_bs__add_BrowsePath_Res_Target_withRemainingPath(translate_browse_path__browsePath,
                     translate_browse_path__l_target,
                     translate_browse_path__l_index,
                     &translate_browse_path__l_statusCode_alloc);
               }
            }
         }
         constants__free_ExpandedNodeId(translate_browse_path__l_target);
      }
   }
}

void translate_browse_path__treat_one_relative_path_element(
   const constants__t_NodeId_i translate_browse_path__source,
   const constants__t_RelativePathElt_i translate_browse_path__relativePathElt,
   constants_statuscodes_bs__t_StatusCode_i * const translate_browse_path__statusCode_operation,
   constants__t_ExpandedNodeId_i * const translate_browse_path__target) {
   {
      t_bool translate_browse_path__l_continue;
      constants__t_NodeId_i translate_browse_path__l_referenceTypeId;
      t_bool translate_browse_path__l_isInverse;
      t_bool translate_browse_path__l_includedSubtypes;
      constants__t_QualifiedName_i translate_browse_path__l_targetName;
      constants__t_BrowseDirection_i translate_browse_path__l_browseDirection;
      constants_statuscodes_bs__t_StatusCode_i translate_browse_path__l_browse_statusCode;
      constants__t_ContinuationPointId_i translate_browse_path__l_continuationPoint;
      t_entier4 translate_browse_path__l_nbReferences;
      t_entier4 translate_browse_path__l_browseResult_index;
      constants__t_NodeId_i translate_browse_path__l_res_refTypeId;
      t_bool translate_browse_path__l_res_isForward;
      constants__t_ExpandedNodeId_i translate_browse_path__l_res_ExpandedNodeId;
      constants__t_QualifiedName_i translate_browse_path__l_res_BrowseName;
      constants__t_LocalizedText_i translate_browse_path__l_res_DisplayName;
      constants__t_NodeClass_i translate_browse_path__l_res_NodeClass;
      constants__t_ExpandedNodeId_i translate_browse_path__l_res_TypeDefinition;
      t_bool translate_browse_path__l_name_empty;
      constants_statuscodes_bs__t_StatusCode_i translate_browse_path__l_translate_statusCode;
      t_bool translate_browse_path__l_found;
      constants__t_ExpandedNodeId_i translate_browse_path__l_target_found;
      t_bool translate_browse_path__l_local_server;
      constants__t_NodeId_i translate_browse_path__l_source;
      
      translate_browse_path__l_nbReferences = 0;
      *translate_browse_path__target = constants__c_ExpandedNodeId_indet;
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
         browse_treatment__set_browse_value_context(constants__c_session_indet,
            0,
            constants__c_NodeId_indet,
            translate_browse_path__source,
            translate_browse_path__l_browseDirection,
            translate_browse_path__l_referenceTypeId,
            translate_browse_path__l_includedSubtypes,
            constants__c_BrowseNodeClassMask_indet,
            constants__c_BrowseResultMask_all,
            &translate_browse_path__l_browse_statusCode);
         if (translate_browse_path__l_browse_statusCode == constants_statuscodes_bs__e_sc_ok) {
            browse_treatment__compute_browse_result(&translate_browse_path__l_browse_statusCode,
               &translate_browse_path__l_continuationPoint,
               &translate_browse_path__l_nbReferences);
            browse_treatment__clear_browse_value_context();
            translate_browse_path__get_translateStatus_from_browseStatus(translate_browse_path__l_browse_statusCode,
               &translate_browse_path__l_translate_statusCode);
         }
         else {
            translate_browse_path__l_translate_statusCode = translate_browse_path__l_browse_statusCode;
         }
         if (translate_browse_path__l_translate_statusCode != constants_statuscodes_bs__e_sc_ok) {
            *translate_browse_path__statusCode_operation = translate_browse_path__l_translate_statusCode;
         }
         else {
            *translate_browse_path__statusCode_operation = constants_statuscodes_bs__e_sc_bad_no_match;
            translate_browse_path__l_target_found = constants__c_ExpandedNodeId_indet;
            browse_treatment__init_iter_browseResult(translate_browse_path__l_nbReferences,
               &translate_browse_path__l_continue);
            while (translate_browse_path__l_continue == true) {
               browse_treatment__continue_iter_browseResult(&translate_browse_path__l_continue,
                  &translate_browse_path__l_browseResult_index);
               browse_treatment__getall_browse_result_reference_at(translate_browse_path__l_browseResult_index,
                  &translate_browse_path__l_res_refTypeId,
                  &translate_browse_path__l_res_isForward,
                  &translate_browse_path__l_res_ExpandedNodeId,
                  &translate_browse_path__l_res_BrowseName,
                  &translate_browse_path__l_res_DisplayName,
                  &translate_browse_path__l_res_NodeClass,
                  &translate_browse_path__l_res_TypeDefinition);
               constants__is_QualifiedNames_Equal(translate_browse_path__l_targetName,
                  translate_browse_path__l_res_BrowseName,
                  &translate_browse_path__l_found);
               if (translate_browse_path__l_found == true) {
                  if (translate_browse_path__l_target_found == constants__c_ExpandedNodeId_indet) {
                     translate_browse_path__l_target_found = translate_browse_path__l_res_ExpandedNodeId;
                     *translate_browse_path__statusCode_operation = constants_statuscodes_bs__e_sc_ok;
                  }
                  else {
                     translate_browse_path__l_target_found = constants__c_ExpandedNodeId_indet;
                     *translate_browse_path__statusCode_operation = constants_statuscodes_bs__e_sc_bad_query_too_complex;
                     translate_browse_path__l_continue = false;
                  }
               }
               else if (translate_browse_path__l_res_ExpandedNodeId != constants__c_ExpandedNodeId_indet) {
                  constants__getall_conv_ExpandedNodeId_NodeId(translate_browse_path__l_res_ExpandedNodeId,
                     &translate_browse_path__l_local_server,
                     &translate_browse_path__l_source);
                  if (translate_browse_path__l_local_server == false) {
                     translate_browse_path__l_target_found = translate_browse_path__l_res_ExpandedNodeId;
                     *translate_browse_path__statusCode_operation = constants_statuscodes_bs__e_sc_uncertain_reference_out_of_server;
                     translate_browse_path__l_continue = false;
                  }
               }
            }
            if (translate_browse_path__l_target_found != constants__c_ExpandedNodeId_indet) {
               constants__get_copy_ExpandedNodeId(translate_browse_path__l_target_found,
                  translate_browse_path__target);
            }
            browse_treatment__clear_browse_result();
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
         address_space__readall_AddressSpace_Node(translate_browse_path__nodeid,
            &translate_browse_path__l_isvalid,
            &translate_browse_path__l_src_node);
         if (translate_browse_path__l_isvalid == false) {
            *translate_browse_path__StatusCode = constants_statuscodes_bs__e_sc_bad_node_id_unknown;
         }
      }
   }
}

