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

 File Name            : address_space.h

 Date                 : 29/01/2019 12:57:45

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _address_space_h
#define _address_space_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*-----------------
   IMPORTS Clause
  -----------------*/
#include "address_space_bs.h"
#include "address_space_it.h"
#include "address_space_typing.h"
#include "gen_subscription_event_bs.h"
#include "response_write_bs.h"
#include "user_authorization_bs.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"
#include "constants_statuscodes_bs.h"
#include "data_value_pointer_bs.h"
#include "service_mgr_1.h"
#include "service_response_cb_bs.h"
#include "service_write_decode_bs.h"
#include "write_value_pointer_bs.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
extern t_bool address_space__ResponseWrite_allocated;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void address_space__INITIALISATION(void);

/*-------------------------------
   PROMOTES and EXTENDS Clauses
  -------------------------------*/
#define address_space__get_BrowseName address_space_bs__get_BrowseName
#define address_space__get_DisplayName address_space_bs__get_DisplayName
#define address_space__get_NodeClass address_space_bs__get_NodeClass
#define address_space__get_Node_RefIndexEnd address_space_bs__get_Node_RefIndexEnd
#define address_space__get_RefIndex_Reference address_space_bs__get_RefIndex_Reference
#define address_space__get_Reference_IsForward address_space_bs__get_Reference_IsForward
#define address_space__get_Reference_ReferenceType address_space_bs__get_Reference_ReferenceType
#define address_space__get_Reference_TargetNode address_space_bs__get_Reference_TargetNode
#define address_space__get_TypeDefinition address_space_bs__get_TypeDefinition
#define address_space__get_Value_StatusCode address_space_bs__get_Value_StatusCode
#define address_space__get_user_authorization user_authorization_bs__get_user_authorization
#define address_space__is_NodeId_equal address_space_bs__is_NodeId_equal
#define address_space__is_transitive_subtype address_space_typing__is_transitive_subtype
#define address_space__read_AddressSpace_clear_value address_space_bs__read_AddressSpace_clear_value
#define address_space__read_AddressSpace_free_variant address_space_bs__read_AddressSpace_free_variant
#define address_space__readall_AddressSpace_Node address_space_bs__readall_AddressSpace_Node
#define address_space__write_WriteResponse_msg_out response_write_bs__write_WriteResponse_msg_out

/*--------------------------
   LOCAL_OPERATIONS Clause
  --------------------------*/
extern void address_space__is_variable_compat_type(
   const constants__t_NodeId_i address_space__p_dv_typ_nid,
   const t_entier4 address_space__p_dv_vr,
   const constants__t_NodeId_i address_space__p_var_typ_nid,
   const t_entier4 address_space__p_var_vr,
   t_bool * const address_space__btyp_ok,
   t_bool * const address_space__btyp_need_conv);
extern void address_space__treat_write_1(
   const t_bool address_space__isvalid,
   const constants_statuscodes_bs__t_StatusCode_i address_space__status,
   const constants__t_user_i address_space__p_user,
   const constants__t_NodeId_i address_space__nid,
   const constants__t_AttributeId_i address_space__aid,
   const constants__t_DataValue_i address_space__dataValue,
   const constants__t_IndexRange_i address_space__index_range,
   constants_statuscodes_bs__t_StatusCode_i * const address_space__serviceStatusCode,
   constants__t_DataValue_i * const address_space__prev_dataValue);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void address_space__alloc_write_request_responses(
   const t_entier4 address_space__nb_req,
   t_bool * const address_space__bret);
extern void address_space__dealloc_write_request_responses(void);
extern void address_space__read_NodeClass_Attribute(
   const constants__t_user_i address_space__p_user,
   const constants__t_Node_i address_space__node,
   const constants__t_AttributeId_i address_space__aid,
   const constants__t_IndexRange_i address_space__index_range,
   constants_statuscodes_bs__t_StatusCode_i * const address_space__sc,
   constants__t_NodeClass_i * const address_space__ncl,
   constants__t_Variant_i * const address_space__val);
extern void address_space__treat_write_request_WriteValues(
   const constants__t_user_i address_space__p_user,
   constants_statuscodes_bs__t_StatusCode_i * const address_space__StatusCode_service);

#endif
