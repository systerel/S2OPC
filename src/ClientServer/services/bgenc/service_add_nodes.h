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

 File Name            : service_add_nodes.h

 Date                 : 05/08/2022 09:03:06

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _service_add_nodes_h
#define _service_add_nodes_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*-----------------
   IMPORTS Clause
  -----------------*/
#include "msg_node_management_add_nodes.h"
#include "node_management_add_nodes_items_it.h"
#include "service_add_nodes_1.h"

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
extern void service_add_nodes__INITIALISATION(void);

/*-------------------------------
   PROMOTES and EXTENDS Clauses
  -------------------------------*/
#define service_add_nodes__check_nodeId_isValid service_add_nodes_1__check_nodeId_isValid
#define service_add_nodes__check_object_has_method service_add_nodes_1__check_object_has_method
#define service_add_nodes__get_BrowseName service_add_nodes_1__get_BrowseName
#define service_add_nodes__get_DisplayName service_add_nodes_1__get_DisplayName
#define service_add_nodes__get_Executable service_add_nodes_1__get_Executable
#define service_add_nodes__get_InputArguments service_add_nodes_1__get_InputArguments
#define service_add_nodes__get_NodeClass service_add_nodes_1__get_NodeClass
#define service_add_nodes__get_Node_RefIndexEnd service_add_nodes_1__get_Node_RefIndexEnd
#define service_add_nodes__get_RefIndex_Reference service_add_nodes_1__get_RefIndex_Reference
#define service_add_nodes__get_Reference_IsForward service_add_nodes_1__get_Reference_IsForward
#define service_add_nodes__get_Reference_ReferenceType service_add_nodes_1__get_Reference_ReferenceType
#define service_add_nodes__get_Reference_TargetNode service_add_nodes_1__get_Reference_TargetNode
#define service_add_nodes__get_TypeDefinition service_add_nodes_1__get_TypeDefinition
#define service_add_nodes__get_Value_StatusCode service_add_nodes_1__get_Value_StatusCode
#define service_add_nodes__get_conv_Variant_Type service_add_nodes_1__get_conv_Variant_Type
#define service_add_nodes__get_conv_Variant_ValueRank service_add_nodes_1__get_conv_Variant_ValueRank
#define service_add_nodes__get_user_authorization service_add_nodes_1__get_user_authorization
#define service_add_nodes__is_AddressSpace_constant service_add_nodes_1__is_AddressSpace_constant
#define service_add_nodes__is_IndexRangeDefined service_add_nodes_1__is_IndexRangeDefined
#define service_add_nodes__is_NodeId_equal service_add_nodes_1__is_NodeId_equal
#define service_add_nodes__is_local_service_treatment service_add_nodes_1__is_local_service_treatment
#define service_add_nodes__is_mandatory_attribute service_add_nodes_1__is_mandatory_attribute
#define service_add_nodes__is_transitive_subtype service_add_nodes_1__is_transitive_subtype
#define service_add_nodes__is_valid_ReferenceTypeId service_add_nodes_1__is_valid_ReferenceTypeId
#define service_add_nodes__read_AddressSpace_clear_value service_add_nodes_1__read_AddressSpace_clear_value
#define service_add_nodes__read_AddressSpace_free_variant service_add_nodes_1__read_AddressSpace_free_variant
#define service_add_nodes__read_Node_Attribute service_add_nodes_1__read_Node_Attribute
#define service_add_nodes__read_variable_compat_type service_add_nodes_1__read_variable_compat_type
#define service_add_nodes__readall_AddressSpace_Node service_add_nodes_1__readall_AddressSpace_Node
#define service_add_nodes__set_local_service_treatment service_add_nodes_1__set_local_service_treatment
#define service_add_nodes__treat_write_request service_add_nodes_1__treat_write_request

/*--------------------------
   LOCAL_OPERATIONS Clause
  --------------------------*/
extern void service_add_nodes__treat_add_nodes_items(
   const constants__t_user_i service_add_nodes__p_user,
   const constants__t_msg_i service_add_nodes__p_req_msg,
   const constants__t_msg_i service_add_nodes__p_resp_msg,
   const t_entier4 service_add_nodes__p_nb_nodes_to_add);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void service_add_nodes__treat_add_nodes_request(
   const constants__t_user_i service_add_nodes__p_user,
   const constants__t_msg_i service_add_nodes__p_req_msg,
   const constants__t_msg_i service_add_nodes__p_resp_msg,
   constants_statuscodes_bs__t_StatusCode_i * const service_add_nodes__StatusCode_service);

#endif
