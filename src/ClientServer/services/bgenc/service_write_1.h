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

 File Name            : service_write_1.h

 Date                 : 05/08/2022 08:40:42

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _service_write_1_h
#define _service_write_1_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*-----------------
   IMPORTS Clause
  -----------------*/
#include "address_space.h"
#include "response_write_bs.h"
#include "service_write_1_it.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"
#include "constants_statuscodes_bs.h"
#include "data_value_pointer_bs.h"
#include "service_response_cb_bs.h"
#include "service_write_decode_bs.h"
#include "write_value_pointer_bs.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
extern t_bool service_write_1__ResponseWrite_allocated;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void service_write_1__INITIALISATION(void);

/*-------------------------------
   PROMOTES and EXTENDS Clauses
  -------------------------------*/
#define service_write_1__check_nodeId_isValid address_space__check_nodeId_isValid
#define service_write_1__check_object_has_method address_space__check_object_has_method
#define service_write_1__get_BrowseName address_space__get_BrowseName
#define service_write_1__get_DisplayName address_space__get_DisplayName
#define service_write_1__get_Executable address_space__get_Executable
#define service_write_1__get_InputArguments address_space__get_InputArguments
#define service_write_1__get_NodeClass address_space__get_NodeClass
#define service_write_1__get_Node_RefIndexEnd address_space__get_Node_RefIndexEnd
#define service_write_1__get_RefIndex_Reference address_space__get_RefIndex_Reference
#define service_write_1__get_Reference_IsForward address_space__get_Reference_IsForward
#define service_write_1__get_Reference_ReferenceType address_space__get_Reference_ReferenceType
#define service_write_1__get_Reference_TargetNode address_space__get_Reference_TargetNode
#define service_write_1__get_TypeDefinition address_space__get_TypeDefinition
#define service_write_1__get_Value_StatusCode address_space__get_Value_StatusCode
#define service_write_1__get_conv_Variant_Type address_space__get_conv_Variant_Type
#define service_write_1__get_conv_Variant_ValueRank address_space__get_conv_Variant_ValueRank
#define service_write_1__get_user_authorization address_space__get_user_authorization
#define service_write_1__is_IndexRangeDefined address_space__is_IndexRangeDefined
#define service_write_1__is_NodeId_equal address_space__is_NodeId_equal
#define service_write_1__is_local_service_treatment address_space__is_local_service_treatment
#define service_write_1__is_mandatory_attribute address_space__is_mandatory_attribute
#define service_write_1__is_transitive_subtype address_space__is_transitive_subtype
#define service_write_1__is_valid_ReferenceTypeId address_space__is_valid_ReferenceTypeId
#define service_write_1__read_AddressSpace_clear_value address_space__read_AddressSpace_clear_value
#define service_write_1__read_AddressSpace_free_variant address_space__read_AddressSpace_free_variant
#define service_write_1__read_Node_Attribute address_space__read_Node_Attribute
#define service_write_1__read_variable_compat_type address_space__read_variable_compat_type
#define service_write_1__readall_AddressSpace_Node address_space__readall_AddressSpace_Node
#define service_write_1__set_local_service_treatment address_space__set_local_service_treatment
#define service_write_1__write_WriteResponse_msg_out response_write_bs__write_WriteResponse_msg_out

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void service_write_1__alloc_write_request_responses(
   const t_entier4 service_write_1__nb_req,
   t_bool * const service_write_1__bret);
extern void service_write_1__dealloc_write_request_responses(void);
extern void service_write_1__treat_write_request_WriteValues(
   const constants__t_user_i service_write_1__p_user,
   const constants__t_LocaleIds_i service_write_1__p_locales,
   constants_statuscodes_bs__t_StatusCode_i * const service_write_1__StatusCode_service);

#endif
