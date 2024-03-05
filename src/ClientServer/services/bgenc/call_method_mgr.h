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

 File Name            : call_method_mgr.h

 Date                 : 22/03/2024 14:58:08

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _call_method_mgr_h
#define _call_method_mgr_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*-----------------
   IMPORTS Clause
  -----------------*/
#include "argument_pointer_bs.h"
#include "call_method_it.h"
#include "call_method_result_it.h"
#include "msg_call_method_bs.h"
#include "service_write.h"

/*--------------
   SEES Clause
  --------------*/
#include "channel_mgr.h"
#include "constants.h"
#include "constants_statuscodes_bs.h"
#include "message_in_bs.h"
#include "message_out_bs.h"
#include "node_id_pointer_bs.h"
#include "request_handle_bs.h"
#include "session_mgr.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void call_method_mgr__INITIALISATION(void);

/*-------------------------------
   PROMOTES and EXTENDS Clauses
  -------------------------------*/
#define call_method_mgr__addNode_AddressSpace service_write__addNode_AddressSpace
#define call_method_mgr__address_space_bs_UNINITIALISATION service_write__address_space_bs_UNINITIALISATION
#define call_method_mgr__check_nodeId_isValid service_write__check_nodeId_isValid
#define call_method_mgr__check_object_has_method service_write__check_object_has_method
#define call_method_mgr__get_BrowseName service_write__get_BrowseName
#define call_method_mgr__get_DisplayName service_write__get_DisplayName
#define call_method_mgr__get_EventNotifier service_write__get_EventNotifier
#define call_method_mgr__get_Executable service_write__get_Executable
#define call_method_mgr__get_InputArguments service_write__get_InputArguments
#define call_method_mgr__get_NodeClass service_write__get_NodeClass
#define call_method_mgr__get_Node_RefIndexEnd service_write__get_Node_RefIndexEnd
#define call_method_mgr__get_RefIndex_Reference service_write__get_RefIndex_Reference
#define call_method_mgr__get_Reference_IsForward service_write__get_Reference_IsForward
#define call_method_mgr__get_Reference_ReferenceType service_write__get_Reference_ReferenceType
#define call_method_mgr__get_Reference_TargetNode service_write__get_Reference_TargetNode
#define call_method_mgr__get_TypeDefinition service_write__get_TypeDefinition
#define call_method_mgr__get_Value_StatusCode service_write__get_Value_StatusCode
#define call_method_mgr__get_conv_Variant_Type service_write__get_conv_Variant_Type
#define call_method_mgr__get_conv_Variant_ValueRank service_write__get_conv_Variant_ValueRank
#define call_method_mgr__get_user_authorization service_write__get_user_authorization
#define call_method_mgr__is_AddressSpace_constant service_write__is_AddressSpace_constant
#define call_method_mgr__is_IndexRangeDefined service_write__is_IndexRangeDefined
#define call_method_mgr__is_NodeId_equal service_write__is_NodeId_equal
#define call_method_mgr__is_local_service_treatment service_write__is_local_service_treatment
#define call_method_mgr__is_mandatory_attribute service_write__is_mandatory_attribute
#define call_method_mgr__is_transitive_subtype service_write__is_transitive_subtype
#define call_method_mgr__is_valid_ReferenceTypeId service_write__is_valid_ReferenceTypeId
#define call_method_mgr__read_AddressSpace_clear_value service_write__read_AddressSpace_clear_value
#define call_method_mgr__read_AddressSpace_free_variant service_write__read_AddressSpace_free_variant
#define call_method_mgr__read_Node_Attribute service_write__read_Node_Attribute
#define call_method_mgr__read_variable_compat_type service_write__read_variable_compat_type
#define call_method_mgr__readall_AddressSpace_Node service_write__readall_AddressSpace_Node
#define call_method_mgr__set_local_service_treatment service_write__set_local_service_treatment
#define call_method_mgr__treat_write_request service_write__treat_write_request

/*--------------------------
   LOCAL_OPERATIONS Clause
  --------------------------*/
extern void call_method_mgr__check_method_call_arguments(
   const constants__t_msg_i call_method_mgr__p_req_msg,
   const constants__t_CallMethod_i call_method_mgr__p_callMethod,
   const constants__t_Node_i call_method_mgr__p_method_node,
   const constants__t_msg_i call_method_mgr__p_res_msg,
   constants_statuscodes_bs__t_StatusCode_i * const call_method_mgr__StatusCode);
extern void call_method_mgr__check_method_call_inputs(
   const constants__t_session_i call_method_mgr__p_session,
   const constants__t_msg_i call_method_mgr__p_req_msg,
   const constants__t_CallMethod_i call_method_mgr__p_callMethod,
   const constants__t_msg_i call_method_mgr__p_res_msg,
   constants_statuscodes_bs__t_StatusCode_i * const call_method_mgr__StatusCode);
extern void call_method_mgr__check_method_call_one_argument_type(
   const constants__t_Variant_i call_method_mgr__p_value,
   const constants__t_Argument_i call_method_mgr__p_arg,
   constants_statuscodes_bs__t_StatusCode_i * const call_method_mgr__StatusCode);
extern void call_method_mgr__treat_one_method_call(
   const constants__t_session_i call_method_mgr__p_session,
   const constants__t_msg_i call_method_mgr__p_req_msg,
   const constants__t_msg_i call_method_mgr__p_res_msg,
   const constants__t_CallMethod_i call_method_mgr__p_callMethod,
   const constants__t_endpoint_config_idx_i call_method_mgr__p_endpoint_config_idx,
   constants_statuscodes_bs__t_StatusCode_i * const call_method_mgr__StatusCode);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void call_method_mgr__treat_method_call_request(
   const constants__t_session_i call_method_mgr__p_session,
   const constants__t_msg_i call_method_mgr__p_req_msg,
   const constants__t_msg_i call_method_mgr__p_resp_msg,
   constants_statuscodes_bs__t_StatusCode_i * const call_method_mgr__StatusCode_service);

#endif
