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

 File Name            : service_write.h

 Date                 : 05/08/2022 08:40:43

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _service_write_h
#define _service_write_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*-----------------
   IMPORTS Clause
  -----------------*/
#include "service_write_1.h"
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
extern void service_write__INITIALISATION(void);

/*-------------------------------
   PROMOTES and EXTENDS Clauses
  -------------------------------*/
#define service_write__check_nodeId_isValid service_write_1__check_nodeId_isValid
#define service_write__check_object_has_method service_write_1__check_object_has_method
#define service_write__get_BrowseName service_write_1__get_BrowseName
#define service_write__get_DisplayName service_write_1__get_DisplayName
#define service_write__get_Executable service_write_1__get_Executable
#define service_write__get_InputArguments service_write_1__get_InputArguments
#define service_write__get_NodeClass service_write_1__get_NodeClass
#define service_write__get_Node_RefIndexEnd service_write_1__get_Node_RefIndexEnd
#define service_write__get_RefIndex_Reference service_write_1__get_RefIndex_Reference
#define service_write__get_Reference_IsForward service_write_1__get_Reference_IsForward
#define service_write__get_Reference_ReferenceType service_write_1__get_Reference_ReferenceType
#define service_write__get_Reference_TargetNode service_write_1__get_Reference_TargetNode
#define service_write__get_TypeDefinition service_write_1__get_TypeDefinition
#define service_write__get_Value_StatusCode service_write_1__get_Value_StatusCode
#define service_write__get_conv_Variant_Type service_write_1__get_conv_Variant_Type
#define service_write__get_conv_Variant_ValueRank service_write_1__get_conv_Variant_ValueRank
#define service_write__get_user_authorization service_write_1__get_user_authorization
#define service_write__is_IndexRangeDefined service_write_1__is_IndexRangeDefined
#define service_write__is_NodeId_equal service_write_1__is_NodeId_equal
#define service_write__is_local_service_treatment service_write_1__is_local_service_treatment
#define service_write__is_mandatory_attribute service_write_1__is_mandatory_attribute
#define service_write__is_transitive_subtype service_write_1__is_transitive_subtype
#define service_write__is_valid_ReferenceTypeId service_write_1__is_valid_ReferenceTypeId
#define service_write__read_AddressSpace_clear_value service_write_1__read_AddressSpace_clear_value
#define service_write__read_AddressSpace_free_variant service_write_1__read_AddressSpace_free_variant
#define service_write__read_Node_Attribute service_write_1__read_Node_Attribute
#define service_write__read_variable_compat_type service_write_1__read_variable_compat_type
#define service_write__readall_AddressSpace_Node service_write_1__readall_AddressSpace_Node
#define service_write__set_local_service_treatment service_write_1__set_local_service_treatment

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void service_write__treat_write_request(
   const constants__t_user_i service_write__p_user,
   const constants__t_LocaleIds_i service_write__p_locales,
   const constants__t_msg_i service_write__write_msg,
   const constants__t_msg_i service_write__resp_msg,
   constants_statuscodes_bs__t_StatusCode_i * const service_write__StatusCode_service);

#endif
