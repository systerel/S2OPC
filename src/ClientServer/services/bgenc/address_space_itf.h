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

 File Name            : address_space_itf.h

 Date                 : 24/07/2024 17:29:49

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _address_space_itf_h
#define _address_space_itf_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*-----------------
   IMPORTS Clause
  -----------------*/
#include "service_add_nodes.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"
#include "constants_statuscodes_bs.h"
#include "message_in_bs.h"
#include "message_out_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void address_space_itf__INITIALISATION(void);

/*-------------------------------
   PROMOTES and EXTENDS Clauses
  -------------------------------*/
#define address_space_itf__address_space_bs_UNINITIALISATION service_add_nodes__address_space_bs_UNINITIALISATION
#define address_space_itf__check_nodeId_isValid service_add_nodes__check_nodeId_isValid
#define address_space_itf__check_object_has_method service_add_nodes__check_object_has_method
#define address_space_itf__get_BrowseName service_add_nodes__get_BrowseName
#define address_space_itf__get_DisplayName service_add_nodes__get_DisplayName
#define address_space_itf__get_EventNotifier service_add_nodes__get_EventNotifier
#define address_space_itf__get_InputArguments service_add_nodes__get_InputArguments
#define address_space_itf__get_NodeClass service_add_nodes__get_NodeClass
#define address_space_itf__get_Node_RefIndexEnd service_add_nodes__get_Node_RefIndexEnd
#define address_space_itf__get_RefIndex_Reference service_add_nodes__get_RefIndex_Reference
#define address_space_itf__get_Reference_IsForward service_add_nodes__get_Reference_IsForward
#define address_space_itf__get_Reference_ReferenceType service_add_nodes__get_Reference_ReferenceType
#define address_space_itf__get_Reference_TargetNode service_add_nodes__get_Reference_TargetNode
#define address_space_itf__get_TypeDefinition service_add_nodes__get_TypeDefinition
#define address_space_itf__get_Value_StatusCode service_add_nodes__get_Value_StatusCode
#define address_space_itf__get_conv_Variant_Type service_add_nodes__get_conv_Variant_Type
#define address_space_itf__get_conv_Variant_ValueRank service_add_nodes__get_conv_Variant_ValueRank
#define address_space_itf__get_user_authorization service_add_nodes__get_user_authorization
#define address_space_itf__has_access_level_executable service_add_nodes__has_access_level_executable
#define address_space_itf__is_AddressSpace_constant service_add_nodes__is_AddressSpace_constant
#define address_space_itf__is_IndexRangeDefined service_add_nodes__is_IndexRangeDefined
#define address_space_itf__is_NodeId_equal service_add_nodes__is_NodeId_equal
#define address_space_itf__is_local_service_treatment service_add_nodes__is_local_service_treatment
#define address_space_itf__is_mandatory_attribute service_add_nodes__is_mandatory_attribute
#define address_space_itf__is_transitive_subtype service_add_nodes__is_transitive_subtype
#define address_space_itf__is_valid_ReferenceTypeId service_add_nodes__is_valid_ReferenceTypeId
#define address_space_itf__read_AddressSpace_clear_value service_add_nodes__read_AddressSpace_clear_value
#define address_space_itf__read_AddressSpace_free_variant service_add_nodes__read_AddressSpace_free_variant
#define address_space_itf__read_Node_Attribute service_add_nodes__read_Node_Attribute
#define address_space_itf__read_variable_compat_type service_add_nodes__read_variable_compat_type
#define address_space_itf__readall_AddressSpace_Node service_add_nodes__readall_AddressSpace_Node
#define address_space_itf__set_local_service_treatment service_add_nodes__set_local_service_treatment
#define address_space_itf__treat_add_nodes_request service_add_nodes__treat_add_nodes_request
#define address_space_itf__treat_method_call_request service_add_nodes__treat_method_call_request
#define address_space_itf__treat_write_request service_add_nodes__treat_write_request

#endif
