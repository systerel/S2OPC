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

 File Name            : address_space_bs.h

 Date                 : 04/08/2022 14:53:27

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _address_space_bs_h
#define _address_space_bs_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"
#include "constants_statuscodes_bs.h"
#include "data_value_pointer_bs.h"
#include "user_authorization_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void address_space_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void address_space_bs__get_AccessLevel(
   const constants__t_Node_i address_space_bs__p_node,
   constants__t_access_level * const address_space_bs__p_access_level);
extern void address_space_bs__get_BrowseName(
   const constants__t_Node_i address_space_bs__p_node,
   constants__t_QualifiedName_i * const address_space_bs__p_browse_name);
extern void address_space_bs__get_DataType(
   const constants__t_Node_i address_space_bs__p_node,
   constants__t_NodeId_i * const address_space_bs__p_data_type);
extern void address_space_bs__get_DisplayName(
   const constants__t_Node_i address_space_bs__p_node,
   constants__t_LocalizedText_i * const address_space_bs__p_display_name);
extern void address_space_bs__get_Executable(
   const constants__t_Node_i address_space_bs__p_node,
   t_bool * const address_space_bs__p_bool);
extern void address_space_bs__get_InputArguments(
   const constants__t_Node_i address_space_bs__p_node,
   constants__t_Variant_i * const address_space_bs__p_input_arg);
extern void address_space_bs__get_NodeClass(
   const constants__t_Node_i address_space_bs__p_node,
   constants__t_NodeClass_i * const address_space_bs__p_node_class);
extern void address_space_bs__get_Node_RefIndexEnd(
   const constants__t_Node_i address_space_bs__p_node,
   t_entier4 * const address_space_bs__p_ref_index);
extern void address_space_bs__get_RefIndex_Reference(
   const constants__t_Node_i address_space_bs__p_node,
   const t_entier4 address_space_bs__p_ref_index,
   constants__t_Reference_i * const address_space_bs__p_ref);
extern void address_space_bs__get_Reference_IsForward(
   const constants__t_Reference_i address_space_bs__p_ref,
   t_bool * const address_space_bs__p_IsForward);
extern void address_space_bs__get_Reference_ReferenceType(
   const constants__t_Reference_i address_space_bs__p_ref,
   constants__t_NodeId_i * const address_space_bs__p_RefType);
extern void address_space_bs__get_Reference_TargetNode(
   const constants__t_Reference_i address_space_bs__p_ref,
   constants__t_ExpandedNodeId_i * const address_space_bs__p_TargetNode);
extern void address_space_bs__get_TypeDefinition(
   const constants__t_Node_i address_space_bs__p_node,
   constants__t_ExpandedNodeId_i * const address_space_bs__p_type_def);
extern void address_space_bs__get_ValueRank(
   const constants__t_Node_i address_space_bs__p_node,
   t_entier4 * const address_space_bs__p_value_rank);
extern void address_space_bs__get_Value_StatusCode(
   const constants__t_user_i address_space_bs__p_user,
   const constants__t_Node_i address_space_bs__node,
   constants__t_RawStatusCode * const address_space_bs__sc);
extern void address_space_bs__get_conv_Variant_Type(
   const constants__t_Variant_i address_space_bs__p_variant,
   constants__t_NodeId_i * const address_space_bs__p_type);
extern void address_space_bs__get_conv_Variant_ValueRank(
   const constants__t_Variant_i address_space_bs__p_variant,
   t_entier4 * const address_space_bs__p_valueRank);
extern void address_space_bs__is_IndexRangeDefined(
   const constants__t_IndexRange_i address_space_bs__p_index_range,
   t_bool * const address_space_bs__bres);
extern void address_space_bs__is_NodeId_equal(
   const constants__t_NodeId_i address_space_bs__nid1,
   const constants__t_NodeId_i address_space_bs__nid2,
   t_bool * const address_space_bs__bres);
extern void address_space_bs__read_AddressSpace_AccessLevel_value(
   const constants__t_Node_i address_space_bs__p_node,
   constants_statuscodes_bs__t_StatusCode_i * const address_space_bs__sc,
   constants__t_Variant_i * const address_space_bs__variant);
extern void address_space_bs__read_AddressSpace_ArrayDimensions_value(
   const constants__t_Node_i address_space_bs__p_node,
   constants_statuscodes_bs__t_StatusCode_i * const address_space_bs__sc,
   constants__t_Variant_i * const address_space_bs__variant);
extern void address_space_bs__read_AddressSpace_BrowseName_value(
   const constants__t_Node_i address_space_bs__p_node,
   constants_statuscodes_bs__t_StatusCode_i * const address_space_bs__sc,
   constants__t_Variant_i * const address_space_bs__variant);
extern void address_space_bs__read_AddressSpace_ContainsNoLoops_value(
   const constants__t_Node_i address_space_bs__p_node,
   constants_statuscodes_bs__t_StatusCode_i * const address_space_bs__sc,
   constants__t_Variant_i * const address_space_bs__variant);
extern void address_space_bs__read_AddressSpace_DataType_value(
   const constants__t_Node_i address_space_bs__p_node,
   constants_statuscodes_bs__t_StatusCode_i * const address_space_bs__sc,
   constants__t_Variant_i * const address_space_bs__variant);
extern void address_space_bs__read_AddressSpace_DisplayName_value(
   const constants__t_LocaleIds_i address_space_bs__p_locales,
   const constants__t_Node_i address_space_bs__p_node,
   constants_statuscodes_bs__t_StatusCode_i * const address_space_bs__sc,
   constants__t_Variant_i * const address_space_bs__variant);
extern void address_space_bs__read_AddressSpace_EventNotifier_value(
   const constants__t_Node_i address_space_bs__p_node,
   constants_statuscodes_bs__t_StatusCode_i * const address_space_bs__sc,
   constants__t_Variant_i * const address_space_bs__variant);
extern void address_space_bs__read_AddressSpace_Executable_value(
   const constants__t_Node_i address_space_bs__p_node,
   constants_statuscodes_bs__t_StatusCode_i * const address_space_bs__sc,
   constants__t_Variant_i * const address_space_bs__variant);
extern void address_space_bs__read_AddressSpace_Historizing_value(
   const constants__t_Node_i address_space_bs__p_node,
   constants_statuscodes_bs__t_StatusCode_i * const address_space_bs__sc,
   constants__t_Variant_i * const address_space_bs__variant);
extern void address_space_bs__read_AddressSpace_IsAbstract_value(
   const constants__t_Node_i address_space_bs__p_node,
   constants_statuscodes_bs__t_StatusCode_i * const address_space_bs__sc,
   constants__t_Variant_i * const address_space_bs__variant);
extern void address_space_bs__read_AddressSpace_NodeClass_value(
   const constants__t_Node_i address_space_bs__p_node,
   constants_statuscodes_bs__t_StatusCode_i * const address_space_bs__sc,
   constants__t_Variant_i * const address_space_bs__variant);
extern void address_space_bs__read_AddressSpace_NodeId_value(
   const constants__t_Node_i address_space_bs__p_node,
   constants_statuscodes_bs__t_StatusCode_i * const address_space_bs__sc,
   constants__t_Variant_i * const address_space_bs__variant);
extern void address_space_bs__read_AddressSpace_Symmetric_value(
   const constants__t_Node_i address_space_bs__p_node,
   constants_statuscodes_bs__t_StatusCode_i * const address_space_bs__sc,
   constants__t_Variant_i * const address_space_bs__variant);
extern void address_space_bs__read_AddressSpace_UserAccessLevel_value(
   const constants__t_Node_i address_space_bs__p_node,
   const t_bool address_space_bs__p_is_user_read_auth,
   const t_bool address_space_bs__p_is_user_write_auth,
   constants_statuscodes_bs__t_StatusCode_i * const address_space_bs__sc,
   constants__t_Variant_i * const address_space_bs__variant);
extern void address_space_bs__read_AddressSpace_UserExecutable_value(
   const constants__t_Node_i address_space_bs__p_node,
   const t_bool address_space_bs__p_is_user_executable_auth,
   constants_statuscodes_bs__t_StatusCode_i * const address_space_bs__sc,
   constants__t_Variant_i * const address_space_bs__variant);
extern void address_space_bs__read_AddressSpace_ValueRank_value(
   const constants__t_Node_i address_space_bs__p_node,
   constants_statuscodes_bs__t_StatusCode_i * const address_space_bs__sc,
   constants__t_Variant_i * const address_space_bs__variant);
extern void address_space_bs__read_AddressSpace_Value_value(
   const constants__t_LocaleIds_i address_space_bs__p_locales,
   const constants__t_Node_i address_space_bs__p_node,
   const constants__t_IndexRange_i address_space_bs__index_range,
   constants_statuscodes_bs__t_StatusCode_i * const address_space_bs__sc,
   constants__t_Variant_i * const address_space_bs__variant,
   constants__t_RawStatusCode * const address_space_bs__val_sc,
   constants__t_Timestamp * const address_space_bs__val_ts_src);
extern void address_space_bs__read_AddressSpace_clear_value(
   const constants__t_Variant_i address_space_bs__val);
extern void address_space_bs__read_AddressSpace_free_variant(
   const constants__t_Variant_i address_space_bs__val);
extern void address_space_bs__readall_AddressSpace_Node(
   const constants__t_NodeId_i address_space_bs__nid,
   t_bool * const address_space_bs__nid_valid,
   constants__t_Node_i * const address_space_bs__node);
extern void address_space_bs__set_Value(
   const constants__t_user_i address_space_bs__p_user,
   const constants__t_LocaleIds_i address_space_bs__p_locales,
   const constants__t_Node_i address_space_bs__node,
   const constants__t_Variant_i address_space_bs__variant,
   const t_bool address_space_bs__toConvert,
   const constants__t_IndexRange_i address_space_bs__index_range,
   constants_statuscodes_bs__t_StatusCode_i * const address_space_bs__serviceStatusCode,
   constants__t_DataValue_i * const address_space_bs__prev_dataValue);
extern void address_space_bs__set_Value_SourceTimestamp(
   const constants__t_user_i address_space_bs__p_user,
   const constants__t_Node_i address_space_bs__p_node,
   const constants__t_Timestamp address_space_bs__p_ts);
extern void address_space_bs__set_Value_StatusCode(
   const constants__t_user_i address_space_bs__p_user,
   const constants__t_Node_i address_space_bs__p_node,
   const constants__t_RawStatusCode address_space_bs__p_sc);
extern void address_space_bs__write_AddressSpace_free_dataValue(
   const constants__t_DataValue_i address_space_bs__data);

#endif
