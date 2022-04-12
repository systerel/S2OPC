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

 Date                 : 05/08/2022 08:40:21

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
#include "service_write.h"

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
#define address_space_itf__check_nodeId_isValid service_write__check_nodeId_isValid
#define address_space_itf__check_object_has_method service_write__check_object_has_method
#define address_space_itf__get_BrowseName service_write__get_BrowseName
#define address_space_itf__get_DisplayName service_write__get_DisplayName
#define address_space_itf__get_Executable service_write__get_Executable
#define address_space_itf__get_InputArguments service_write__get_InputArguments
#define address_space_itf__get_NodeClass service_write__get_NodeClass
#define address_space_itf__get_Node_RefIndexEnd service_write__get_Node_RefIndexEnd
#define address_space_itf__get_RefIndex_Reference service_write__get_RefIndex_Reference
#define address_space_itf__get_Reference_IsForward service_write__get_Reference_IsForward
#define address_space_itf__get_Reference_ReferenceType service_write__get_Reference_ReferenceType
#define address_space_itf__get_Reference_TargetNode service_write__get_Reference_TargetNode
#define address_space_itf__get_TypeDefinition service_write__get_TypeDefinition
#define address_space_itf__get_Value_StatusCode service_write__get_Value_StatusCode
#define address_space_itf__get_conv_Variant_Type service_write__get_conv_Variant_Type
#define address_space_itf__get_conv_Variant_ValueRank service_write__get_conv_Variant_ValueRank
#define address_space_itf__get_user_authorization service_write__get_user_authorization
#define address_space_itf__is_IndexRangeDefined service_write__is_IndexRangeDefined
#define address_space_itf__is_NodeId_equal service_write__is_NodeId_equal
#define address_space_itf__is_local_service_treatment service_write__is_local_service_treatment
#define address_space_itf__is_mandatory_attribute service_write__is_mandatory_attribute
#define address_space_itf__is_transitive_subtype service_write__is_transitive_subtype
#define address_space_itf__is_valid_ReferenceTypeId service_write__is_valid_ReferenceTypeId
#define address_space_itf__read_AddressSpace_clear_value service_write__read_AddressSpace_clear_value
#define address_space_itf__read_AddressSpace_free_variant service_write__read_AddressSpace_free_variant
#define address_space_itf__read_Node_Attribute service_write__read_Node_Attribute
#define address_space_itf__read_variable_compat_type service_write__read_variable_compat_type
#define address_space_itf__readall_AddressSpace_Node service_write__readall_AddressSpace_Node
#define address_space_itf__set_local_service_treatment service_write__set_local_service_treatment
#define address_space_itf__treat_write_request service_write__treat_write_request

#endif
