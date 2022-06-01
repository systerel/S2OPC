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

 Date                 : 01/06/2022 17:07:46

 C Translator Version : tradc Java V1.0 (14/03/2012)

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
#include "address_space.h"
#include "service_write_decode_bs.h"

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
#define address_space_itf__check_nodeId_isValid address_space__check_nodeId_isValid
#define address_space_itf__check_object_has_method address_space__check_object_has_method
#define address_space_itf__get_BrowseName address_space__get_BrowseName
#define address_space_itf__get_DisplayName address_space__get_DisplayName
#define address_space_itf__get_Executable address_space__get_Executable
#define address_space_itf__get_InputArguments address_space__get_InputArguments
#define address_space_itf__get_NodeClass address_space__get_NodeClass
#define address_space_itf__get_Node_RefIndexEnd address_space__get_Node_RefIndexEnd
#define address_space_itf__get_RefIndex_Reference address_space__get_RefIndex_Reference
#define address_space_itf__get_Reference_IsForward address_space__get_Reference_IsForward
#define address_space_itf__get_Reference_ReferenceType address_space__get_Reference_ReferenceType
#define address_space_itf__get_Reference_TargetNode address_space__get_Reference_TargetNode
#define address_space_itf__get_TypeDefinition address_space__get_TypeDefinition
#define address_space_itf__get_Value_StatusCode address_space__get_Value_StatusCode
#define address_space_itf__get_conv_Variant_Type address_space__get_conv_Variant_Type
#define address_space_itf__get_conv_Variant_ValueRank address_space__get_conv_Variant_ValueRank
#define address_space_itf__get_user_authorization address_space__get_user_authorization
#define address_space_itf__is_NodeId_equal address_space__is_NodeId_equal
#define address_space_itf__is_transitive_subtype address_space__is_transitive_subtype
#define address_space_itf__is_valid_ReferenceTypeId address_space__is_valid_ReferenceTypeId
#define address_space_itf__read_AddressSpace_clear_value address_space__read_AddressSpace_clear_value
#define address_space_itf__read_AddressSpace_free_variant address_space__read_AddressSpace_free_variant
#define address_space_itf__read_Node_Attribute address_space__read_Node_Attribute
#define address_space_itf__read_variable_compat_type address_space__read_variable_compat_type
#define address_space_itf__readall_AddressSpace_Node address_space__readall_AddressSpace_Node
#define address_space_itf__set_local_service_treatment address_space__set_local_service_treatment
#define address_space_itf__unset_local_service_treatment address_space__unset_local_service_treatment

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void address_space_itf__treat_write_request(
   const constants__t_user_i address_space_itf__p_user,
   const constants__t_LocaleIds_i address_space_itf__p_locales,
   const constants__t_msg_i address_space_itf__write_msg,
   const constants__t_msg_i address_space_itf__resp_msg,
   constants_statuscodes_bs__t_StatusCode_i * const address_space_itf__StatusCode_service);

#endif
