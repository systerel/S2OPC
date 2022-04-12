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

 File Name            : address_space.c

 Date                 : 05/08/2022 08:40:20

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "address_space.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void address_space__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void address_space__local_is_mandatory_attribute(
   const constants__t_NodeClass_i address_space__p_ncl,
   const constants__t_AttributeId_i address_space__p_aid,
   t_bool * const address_space__bres) {
   *address_space__bres = ((((address_space__p_aid == constants__e_aid_BrowseName) ||
      (address_space__p_aid == constants__e_aid_DisplayName)) ||
      (address_space__p_aid == constants__e_aid_NodeClass)) ||
      (address_space__p_aid == constants__e_aid_NodeId));
   if (*address_space__bres == false) {
      switch (address_space__p_ncl) {
      case constants__e_ncl_Variable:
         *address_space__bres = (((((((address_space__p_aid == constants__e_aid_AccessLevel) ||
            (address_space__p_aid == constants__e_aid_DataType)) ||
            (address_space__p_aid == constants__e_aid_Historizing)) ||
            (address_space__p_aid == constants__e_aid_UserAccessLevel)) ||
            (address_space__p_aid == constants__e_aid_Value)) ||
            (address_space__p_aid == constants__e_aid_ValueRank)) ||
            (address_space__p_aid == constants__e_aid_ArrayDimensions));
         break;
      case constants__e_ncl_VariableType:
         *address_space__bres = ((((address_space__p_aid == constants__e_aid_DataType) ||
            (address_space__p_aid == constants__e_aid_IsAbstract)) ||
            (address_space__p_aid == constants__e_aid_ValueRank)) ||
            (address_space__p_aid == constants__e_aid_ArrayDimensions));
         break;
      case constants__e_ncl_Object:
         *address_space__bres = (address_space__p_aid == constants__e_aid_EventNotifier);
         break;
      case constants__e_ncl_ObjectType:
         *address_space__bres = (address_space__p_aid == constants__e_aid_IsAbstract);
         break;
      case constants__e_ncl_ReferenceType:
         *address_space__bres = ((address_space__p_aid == constants__e_aid_IsAbstract) ||
            (address_space__p_aid == constants__e_aid_Symmetric));
         break;
      case constants__e_ncl_DataType:
         *address_space__bres = (address_space__p_aid == constants__e_aid_IsAbstract);
         break;
      case constants__e_ncl_Method:
         *address_space__bres = ((address_space__p_aid == constants__e_aid_Executable) ||
            (address_space__p_aid == constants__e_aid_UserExecutable));
         break;
      case constants__e_ncl_View:
         *address_space__bres = ((address_space__p_aid == constants__e_aid_EventNotifier) ||
            (address_space__p_aid == constants__e_aid_ContainsNoLoops));
         break;
      default:
         break;
      }
   }
}

void address_space__is_mandatory_attribute(
   const constants__t_NodeClass_i address_space__p_ncl,
   const constants__t_AttributeId_i address_space__p_aid,
   t_bool * const address_space__bres) {
   address_space__local_is_mandatory_attribute(address_space__p_ncl,
      address_space__p_aid,
      address_space__bres);
}

void address_space__treat_write_request_WriteValue(
   const constants__t_user_i address_space__p_user,
   const constants__t_LocaleIds_i address_space__p_locales,
   const constants__t_WriteValue_i address_space__p_wvi,
   constants_statuscodes_bs__t_StatusCode_i * const address_space__p_status) {
   {
      constants__t_AttributeId_i address_space__l_aid;
      constants__t_NodeId_i address_space__l_nid;
      constants__t_DataValue_i address_space__l_dataValue;
      constants__t_IndexRange_i address_space__l_index_range;
      constants_statuscodes_bs__t_StatusCode_i address_space__l_status1;
      constants__t_DataValue_i address_space__l_prev_dataValue;
      constants__t_Node_i address_space__l_node;
      constants__t_access_level address_space__l_access_lvl;
      t_bool address_space__l_access_read;
      t_bool address_space__l_prev_local_treatment;
      t_bool address_space__l_isvalid;
      t_bool address_space__l_local_treatment;
      constants__t_WriteValuePointer_i address_space__l_wv;
      t_bool address_space__l_bres_wv_copy;
      constants__t_WriteValuePointer_i address_space__l_wv_copy;
      constants_statuscodes_bs__t_StatusCode_i address_space__l_new_sc;
      constants__t_Variant_i address_space__l_new_val;
      constants__t_RawStatusCode address_space__l_new_val_sc;
      constants__t_Timestamp address_space__l_new_val_ts_src;
      constants__t_Timestamp address_space__l_new_val_ts_srv;
      
      service_write_decode_bs__getall_WriteValue(address_space__p_wvi,
         &address_space__l_isvalid,
         &address_space__l_status1,
         &address_space__l_nid,
         &address_space__l_aid,
         &address_space__l_dataValue,
         &address_space__l_index_range);
      address_space__treat_write_1(address_space__l_isvalid,
         address_space__l_status1,
         address_space__p_user,
         address_space__p_locales,
         address_space__l_nid,
         address_space__l_aid,
         address_space__l_dataValue,
         address_space__l_index_range,
         address_space__p_status,
         &address_space__l_prev_dataValue,
         &address_space__l_node);
      address_space__l_new_val = constants__c_Variant_indet;
      service_write_decode_bs__getall_WriteValuePointer(address_space__p_wvi,
         &address_space__l_wv);
      if (*address_space__p_status == constants_statuscodes_bs__e_sc_ok) {
         address_space_bs__get_AccessLevel(address_space__l_node,
            &address_space__l_access_lvl);
         constants__is_t_access_level_currentRead(address_space__l_access_lvl,
            &address_space__l_access_read);
         if (address_space__l_access_read == true) {
            address_space_local__is_local_service_treatment(&address_space__l_prev_local_treatment);
            address_space_local__set_local_service_treatment(true);
            address_space__read_AddressSpace_Attribute_value(address_space__p_user,
               address_space__p_locales,
               address_space__l_node,
               address_space__l_nid,
               address_space__l_aid,
               constants__c_IndexRange_indet,
               &address_space__l_new_sc,
               &address_space__l_new_val,
               &address_space__l_new_val_sc,
               &address_space__l_new_val_ts_src,
               &address_space__l_new_val_ts_srv);
            address_space_local__set_local_service_treatment(address_space__l_prev_local_treatment);
            if (address_space__l_new_sc == constants_statuscodes_bs__e_sc_ok) {
               gen_subscription_event_bs__gen_data_changed_event(address_space__l_nid,
                  address_space__l_aid,
                  address_space__l_prev_dataValue,
                  address_space__l_new_val,
                  address_space__l_new_val_sc,
                  address_space__l_new_val_ts_src,
                  address_space__l_new_val_ts_srv);
            }
            else {
               gen_subscription_event_bs__gen_data_changed_event_failed();
            }
         }
      }
      address_space_bs__write_AddressSpace_free_dataValue(address_space__l_prev_dataValue);
      address_space_bs__read_AddressSpace_free_variant(address_space__l_new_val);
      address_space_local__is_local_service_treatment(&address_space__l_local_treatment);
      if (address_space__l_local_treatment == false) {
         write_value_pointer_bs__copy_write_value_pointer_content(address_space__l_wv,
            &address_space__l_bres_wv_copy,
            &address_space__l_wv_copy);
         if (address_space__l_bres_wv_copy == true) {
            service_response_cb_bs__srv_write_notification(address_space__l_wv_copy,
               *address_space__p_status);
         }
         else {
            ;
         }
      }
   }
}

void address_space__treat_write_1(
   const t_bool address_space__isvalid,
   const constants_statuscodes_bs__t_StatusCode_i address_space__status,
   const constants__t_user_i address_space__p_user,
   const constants__t_LocaleIds_i address_space__p_locales,
   const constants__t_NodeId_i address_space__nid,
   const constants__t_AttributeId_i address_space__aid,
   const constants__t_DataValue_i address_space__dataValue,
   const constants__t_IndexRange_i address_space__index_range,
   constants_statuscodes_bs__t_StatusCode_i * const address_space__serviceStatusCode,
   constants__t_DataValue_i * const address_space__prev_dataValue,
   constants__t_Node_i * const address_space__node) {
   {
      t_bool address_space__l_isvalid;
      constants__t_NodeClass_i address_space__l_ncl;
      constants__t_access_level address_space__l_access_lvl;
      t_bool address_space__l_access_write;
      t_bool address_space__l_access_write_status;
      t_bool address_space__l_access_write_timestamp;
      t_bool address_space__l_authorized_write;
      t_bool address_space__l_compatible_type;
      t_bool address_space__l_compat_with_conv;
      constants__t_NodeId_i address_space__l_var_datatype_nid;
      t_entier4 address_space__l_var_vr;
      constants__t_NodeId_i address_space__l_dv_datatype_nid;
      t_entier4 address_space__l_dv_datatype_vr;
      t_bool address_space__l_local_treatment;
      constants__t_Variant_i address_space__l_variant;
      constants__t_Timestamp address_space__l_source_ts;
      constants__t_RawStatusCode address_space__l_raw_sc;
      
      *address_space__node = constants__c_Node_indet;
      *address_space__prev_dataValue = constants__c_DataValue_indet;
      if (address_space__isvalid == true) {
         address_space_bs__readall_AddressSpace_Node(address_space__nid,
            &address_space__l_isvalid,
            address_space__node);
         if (address_space__l_isvalid == true) {
            address_space_bs__get_NodeClass(*address_space__node,
               &address_space__l_ncl);
            if ((address_space__aid == constants__e_aid_Value) &&
               (address_space__l_ncl == constants__e_ncl_Variable)) {
               address_space_bs__get_DataType(*address_space__node,
                  &address_space__l_var_datatype_nid);
               address_space_bs__get_ValueRank(*address_space__node,
                  &address_space__l_var_vr);
               data_value_pointer_bs__get_conv_DataValue_LocalDataType(address_space__dataValue,
                  &address_space__l_dv_datatype_nid);
               data_value_pointer_bs__get_conv_DataValue_ValueRank(address_space__dataValue,
                  &address_space__l_dv_datatype_vr);
               address_space__is_variable_compat_type(address_space__l_dv_datatype_nid,
                  address_space__l_dv_datatype_vr,
                  address_space__l_var_datatype_nid,
                  address_space__l_var_vr,
                  &address_space__l_compatible_type,
                  &address_space__l_compat_with_conv);
               if (address_space__l_compatible_type == true) {
                  address_space_local__is_local_service_treatment(&address_space__l_local_treatment);
                  data_value_pointer_bs__get_conv_DataValue_Variant(address_space__dataValue,
                     &address_space__l_variant);
                  data_value_pointer_bs__get_conv_DataValue_SourceTimestamp(address_space__dataValue,
                     &address_space__l_source_ts);
                  data_value_pointer_bs__get_conv_DataValue_Status(address_space__dataValue,
                     &address_space__l_raw_sc);
                  if (address_space__l_local_treatment == true) {
                     address_space_bs__set_Value(address_space__p_user,
                        address_space__p_locales,
                        *address_space__node,
                        address_space__l_variant,
                        address_space__l_compat_with_conv,
                        address_space__index_range,
                        address_space__serviceStatusCode,
                        address_space__prev_dataValue);
                     if (*address_space__serviceStatusCode == constants_statuscodes_bs__e_sc_ok) {
                        address_space_bs__set_Value_StatusCode(address_space__p_user,
                           *address_space__node,
                           address_space__l_raw_sc);
                        address_space_bs__set_Value_SourceTimestamp(address_space__p_user,
                           *address_space__node,
                           address_space__l_source_ts);
                     }
                  }
                  else {
                     address_space_bs__get_AccessLevel(*address_space__node,
                        &address_space__l_access_lvl);
                     constants__is_t_access_level_currentWrite(address_space__l_access_lvl,
                        &address_space__l_access_write);
                     constants__is_t_access_level_statusWrite(address_space__l_access_lvl,
                        &address_space__l_access_write_status);
                     constants__is_t_access_level_timestampWrite(address_space__l_access_lvl,
                        &address_space__l_access_write_timestamp);
                     if (address_space__l_access_write == true) {
                        user_authorization_bs__get_user_authorization(constants__e_operation_type_write,
                           address_space__nid,
                           address_space__aid,
                           address_space__p_user,
                           &address_space__l_authorized_write);
                        if (address_space__l_authorized_write == true) {
                           address_space_bs__set_Value(address_space__p_user,
                              address_space__p_locales,
                              *address_space__node,
                              address_space__l_variant,
                              address_space__l_compat_with_conv,
                              address_space__index_range,
                              address_space__serviceStatusCode,
                              address_space__prev_dataValue);
                           if (*address_space__serviceStatusCode == constants_statuscodes_bs__e_sc_ok) {
                              if (address_space__l_access_write_status == true) {
                                 address_space_bs__set_Value_StatusCode(address_space__p_user,
                                    *address_space__node,
                                    address_space__l_raw_sc);
                              }
                              if (address_space__l_access_write_timestamp == true) {
                                 address_space_bs__set_Value_SourceTimestamp(address_space__p_user,
                                    *address_space__node,
                                    address_space__l_source_ts);
                              }
                           }
                        }
                        else {
                           *address_space__serviceStatusCode = constants_statuscodes_bs__e_sc_bad_user_access_denied;
                        }
                     }
                     else {
                        *address_space__serviceStatusCode = constants_statuscodes_bs__e_sc_bad_not_writable;
                     }
                  }
               }
               else {
                  *address_space__serviceStatusCode = constants_statuscodes_bs__e_sc_bad_type_mismatch;
               }
            }
            else {
               *address_space__serviceStatusCode = constants_statuscodes_bs__e_sc_bad_not_writable;
            }
         }
         else {
            *address_space__serviceStatusCode = constants_statuscodes_bs__e_sc_bad_node_id_unknown;
         }
      }
      else {
         *address_space__serviceStatusCode = address_space__status;
      }
   }
}

void address_space__is_variable_compat_type(
   const constants__t_NodeId_i address_space__p_dv_typ_nid,
   const t_entier4 address_space__p_dv_vr,
   const constants__t_NodeId_i address_space__p_var_typ_nid,
   const t_entier4 address_space__p_var_vr,
   t_bool * const address_space__btyp_ok,
   t_bool * const address_space__btyp_need_conv) {
   {
      t_bool address_space__l_node_ids_eq;
      t_bool address_space__l_dv_is_null_type;
      t_bool address_space__l_dv_is_sub_typ;
      t_bool address_space__l_dv_is_sub_typ_or_compat;
      t_bool address_space__l_dv_is_byte_type;
      t_bool address_space__l_dv_is_bytestring_type;
      t_bool address_space__l_var_is_byte_type;
      t_bool address_space__l_var_is_bytestring_type;
      t_bool address_space__l_var_is_scalar_vr;
      t_bool address_space__l_var_is_one_dim_vr;
      t_bool address_space__l_typ_is_ok;
      t_bool address_space__l_typ_need_conv;
      t_bool address_space__l_value_rank_is_ok;
      
      address_space__l_typ_is_ok = false;
      address_space__l_typ_need_conv = false;
      address_space__l_value_rank_is_ok = false;
      if (address_space__p_dv_typ_nid != constants__c_NodeId_indet) {
         address_space_bs__is_NodeId_equal(address_space__p_dv_typ_nid,
            address_space__p_var_typ_nid,
            &address_space__l_node_ids_eq);
         address_space_bs__is_NodeId_equal(address_space__p_dv_typ_nid,
            constants__c_Null_Type_NodeId,
            &address_space__l_dv_is_null_type);
         address_space_typing__is_included_ValueRank(address_space__p_dv_vr,
            address_space__p_var_vr,
            &address_space__l_value_rank_is_ok);
         if (address_space__l_node_ids_eq == true) {
            address_space__l_typ_is_ok = true;
         }
         else if (address_space__l_dv_is_null_type == true) {
            address_space__l_typ_is_ok = true;
         }
         else {
            address_space_typing__is_transitive_subtype(address_space__p_dv_typ_nid,
               address_space__p_var_typ_nid,
               &address_space__l_dv_is_sub_typ);
            address_space_typing__is_transitive_subtype_or_compatible_simple_type_or_enumeration(address_space__l_dv_is_sub_typ,
               address_space__p_dv_typ_nid,
               address_space__p_var_typ_nid,
               &address_space__l_dv_is_sub_typ_or_compat);
            if (address_space__l_dv_is_sub_typ_or_compat == true) {
               address_space__l_typ_is_ok = true;
            }
            else {
               address_space_bs__is_NodeId_equal(address_space__p_dv_typ_nid,
                  constants__c_ByteString_Type_NodeId,
                  &address_space__l_dv_is_bytestring_type);
               address_space_bs__is_NodeId_equal(address_space__p_dv_typ_nid,
                  constants__c_Byte_Type_NodeId,
                  &address_space__l_dv_is_byte_type);
               address_space_bs__is_NodeId_equal(address_space__p_var_typ_nid,
                  constants__c_ByteString_Type_NodeId,
                  &address_space__l_var_is_bytestring_type);
               address_space_bs__is_NodeId_equal(address_space__p_var_typ_nid,
                  constants__c_Byte_Type_NodeId,
                  &address_space__l_var_is_byte_type);
               address_space_typing__is_included_ValueRank(-1,
                  address_space__p_var_vr,
                  &address_space__l_var_is_scalar_vr);
               address_space_typing__is_included_ValueRank(1,
                  address_space__p_var_vr,
                  &address_space__l_var_is_one_dim_vr);
               if ((((address_space__l_dv_is_bytestring_type == true) &&
                  (address_space__p_dv_vr == -1)) &&
                  (address_space__l_var_is_byte_type == true)) &&
                  (address_space__l_var_is_one_dim_vr == true)) {
                  address_space__l_typ_is_ok = true;
                  address_space__l_typ_need_conv = true;
                  address_space__l_value_rank_is_ok = true;
               }
               else if ((((address_space__l_dv_is_byte_type == true) &&
                  (address_space__p_dv_vr == 1)) &&
                  (address_space__l_var_is_bytestring_type == true)) &&
                  (address_space__l_var_is_scalar_vr == true)) {
                  address_space__l_typ_is_ok = true;
                  address_space__l_typ_need_conv = true;
                  address_space__l_value_rank_is_ok = true;
               }
            }
         }
      }
      *address_space__btyp_ok = ((address_space__l_typ_is_ok == true) &&
         (address_space__l_value_rank_is_ok == true));
      *address_space__btyp_need_conv = address_space__l_typ_need_conv;
   }
}

void address_space__read_AddressSpace_Attribute_value(
   const constants__t_user_i address_space__p_user,
   const constants__t_LocaleIds_i address_space__p_locales,
   const constants__t_Node_i address_space__p_node,
   const constants__t_NodeId_i address_space__p_nid,
   const constants__t_AttributeId_i address_space__p_aid,
   const constants__t_IndexRange_i address_space__p_index_range,
   constants_statuscodes_bs__t_StatusCode_i * const address_space__sc,
   constants__t_Variant_i * const address_space__val,
   constants__t_RawStatusCode * const address_space__val_sc,
   constants__t_Timestamp * const address_space__val_ts_src,
   constants__t_Timestamp * const address_space__val_ts_srv) {
   {
      t_bool address_space__l_user_read_auth;
      t_bool address_space__l_user_write_auth;
      t_bool address_space__l_is_range_defined;
      t_bool address_space__l_user_executable_auth;
      
      *address_space__sc = constants_statuscodes_bs__e_sc_ok;
      constants_statuscodes_bs__get_const_RawStatusCode_Good(address_space__val_sc);
      *address_space__val = constants__c_Variant_indet;
      constants__get_CurrentTimestamp(address_space__val_ts_srv);
      *address_space__val_ts_src = constants__c_Timestamp_null;
      address_space_bs__is_IndexRangeDefined(address_space__p_index_range,
         &address_space__l_is_range_defined);
      if ((address_space__p_aid != constants__e_aid_Value) &&
         (address_space__l_is_range_defined == true)) {
         *address_space__sc = constants_statuscodes_bs__e_sc_bad_index_range_no_data;
      }
      else {
         switch (address_space__p_aid) {
         case constants__e_aid_AccessLevel:
            address_space_bs__read_AddressSpace_AccessLevel_value(address_space__p_node,
               address_space__sc,
               address_space__val);
            break;
         case constants__e_aid_BrowseName:
            address_space_bs__read_AddressSpace_BrowseName_value(address_space__p_node,
               address_space__sc,
               address_space__val);
            break;
         case constants__e_aid_ContainsNoLoops:
            address_space_bs__read_AddressSpace_ContainsNoLoops_value(address_space__p_node,
               address_space__sc,
               address_space__val);
            break;
         case constants__e_aid_DataType:
            address_space_bs__read_AddressSpace_DataType_value(address_space__p_node,
               address_space__sc,
               address_space__val);
            break;
         case constants__e_aid_DisplayName:
            address_space_bs__read_AddressSpace_DisplayName_value(address_space__p_locales,
               address_space__p_node,
               address_space__sc,
               address_space__val);
            break;
         case constants__e_aid_EventNotifier:
            address_space_bs__read_AddressSpace_EventNotifier_value(address_space__p_node,
               address_space__sc,
               address_space__val);
            break;
         case constants__e_aid_Executable:
            address_space_bs__read_AddressSpace_Executable_value(address_space__p_node,
               address_space__sc,
               address_space__val);
            break;
         case constants__e_aid_Historizing:
            address_space_bs__read_AddressSpace_Historizing_value(address_space__p_node,
               address_space__sc,
               address_space__val);
            break;
         case constants__e_aid_IsAbstract:
            address_space_bs__read_AddressSpace_IsAbstract_value(address_space__p_node,
               address_space__sc,
               address_space__val);
            break;
         case constants__e_aid_NodeClass:
            address_space_bs__read_AddressSpace_NodeClass_value(address_space__p_node,
               address_space__sc,
               address_space__val);
            break;
         case constants__e_aid_NodeId:
            address_space_bs__read_AddressSpace_NodeId_value(address_space__p_node,
               address_space__sc,
               address_space__val);
            break;
         case constants__e_aid_Symmetric:
            address_space_bs__read_AddressSpace_Symmetric_value(address_space__p_node,
               address_space__sc,
               address_space__val);
            break;
         case constants__e_aid_UserAccessLevel:
            user_authorization_bs__get_user_authorization(constants__e_operation_type_read,
               address_space__p_nid,
               constants__e_aid_Value,
               address_space__p_user,
               &address_space__l_user_read_auth);
            user_authorization_bs__get_user_authorization(constants__e_operation_type_write,
               address_space__p_nid,
               constants__e_aid_Value,
               address_space__p_user,
               &address_space__l_user_write_auth);
            address_space_bs__read_AddressSpace_UserAccessLevel_value(address_space__p_node,
               address_space__l_user_read_auth,
               address_space__l_user_write_auth,
               address_space__sc,
               address_space__val);
            break;
         case constants__e_aid_UserExecutable:
            user_authorization_bs__get_user_authorization(constants__e_operation_type_executable,
               address_space__p_nid,
               constants__e_aid_Executable,
               address_space__p_user,
               &address_space__l_user_executable_auth);
            address_space_bs__read_AddressSpace_UserExecutable_value(address_space__p_node,
               address_space__l_user_executable_auth,
               address_space__sc,
               address_space__val);
            break;
         case constants__e_aid_Value:
            address_space_bs__read_AddressSpace_Value_value(address_space__p_locales,
               address_space__p_node,
               address_space__p_index_range,
               address_space__sc,
               address_space__val,
               address_space__val_sc,
               address_space__val_ts_src);
            break;
         case constants__e_aid_ValueRank:
            address_space_bs__read_AddressSpace_ValueRank_value(address_space__p_node,
               address_space__sc,
               address_space__val);
            break;
         case constants__e_aid_ArrayDimensions:
            address_space_bs__read_AddressSpace_ArrayDimensions_value(address_space__p_node,
               address_space__sc,
               address_space__val);
            break;
         default:
            break;
         }
      }
   }
}

void address_space__read_Node_Attribute(
   const constants__t_user_i address_space__p_user,
   const constants__t_LocaleIds_i address_space__p_locales,
   const constants__t_Node_i address_space__p_node,
   const constants__t_NodeId_i address_space__p_nid,
   const constants__t_AttributeId_i address_space__p_aid,
   const constants__t_IndexRange_i address_space__p_index_range,
   constants_statuscodes_bs__t_StatusCode_i * const address_space__sc,
   constants__t_Variant_i * const address_space__val,
   constants__t_RawStatusCode * const address_space__val_sc,
   constants__t_Timestamp * const address_space__val_ts_src,
   constants__t_Timestamp * const address_space__val_ts_srv) {
   {
      t_bool address_space__l_is_mandatory_attribute;
      constants__t_NodeClass_i address_space__l_ncl;
      t_bool address_space__l_is_local_read;
      constants__t_access_level address_space__l_access_lvl;
      t_bool address_space__l_access_read;
      t_bool address_space__l_user_auth;
      
      *address_space__sc = constants_statuscodes_bs__e_sc_bad_attribute_id_invalid;
      constants_statuscodes_bs__get_const_RawStatusCode_BadInvalidState(address_space__val_sc);
      *address_space__val = constants__c_Variant_indet;
      *address_space__val_ts_src = constants__c_Timestamp_null;
      *address_space__val_ts_srv = constants__c_Timestamp_null;
      address_space_bs__get_NodeClass(address_space__p_node,
         &address_space__l_ncl);
      address_space__local_is_mandatory_attribute(address_space__l_ncl,
         address_space__p_aid,
         &address_space__l_is_mandatory_attribute);
      if (address_space__l_is_mandatory_attribute == true) {
         if ((address_space__l_ncl == constants__e_ncl_Variable) &&
            (address_space__p_aid == constants__e_aid_Value)) {
            address_space_local__is_local_service_treatment(&address_space__l_is_local_read);
            if (address_space__l_is_local_read == true) {
               address_space__read_AddressSpace_Attribute_value(address_space__p_user,
                  address_space__p_locales,
                  address_space__p_node,
                  address_space__p_nid,
                  address_space__p_aid,
                  address_space__p_index_range,
                  address_space__sc,
                  address_space__val,
                  address_space__val_sc,
                  address_space__val_ts_src,
                  address_space__val_ts_srv);
            }
            else {
               address_space_bs__get_AccessLevel(address_space__p_node,
                  &address_space__l_access_lvl);
               constants__is_t_access_level_currentRead(address_space__l_access_lvl,
                  &address_space__l_access_read);
               if (address_space__l_access_read == true) {
                  user_authorization_bs__get_user_authorization(constants__e_operation_type_read,
                     address_space__p_nid,
                     address_space__p_aid,
                     address_space__p_user,
                     &address_space__l_user_auth);
                  if (address_space__l_user_auth == true) {
                     address_space__read_AddressSpace_Attribute_value(address_space__p_user,
                        address_space__p_locales,
                        address_space__p_node,
                        address_space__p_nid,
                        address_space__p_aid,
                        address_space__p_index_range,
                        address_space__sc,
                        address_space__val,
                        address_space__val_sc,
                        address_space__val_ts_src,
                        address_space__val_ts_srv);
                  }
                  else {
                     *address_space__sc = constants_statuscodes_bs__e_sc_bad_user_access_denied;
                  }
               }
               else {
                  *address_space__sc = constants_statuscodes_bs__e_sc_bad_not_readable;
               }
            }
         }
         else {
            address_space__read_AddressSpace_Attribute_value(address_space__p_user,
               address_space__p_locales,
               address_space__p_node,
               address_space__p_nid,
               address_space__p_aid,
               address_space__p_index_range,
               address_space__sc,
               address_space__val,
               address_space__val_sc,
               address_space__val_ts_src,
               address_space__val_ts_srv);
         }
      }
   }
}

void address_space__check_nodeId_isValid(
   const constants__t_NodeId_i address_space__nodeid,
   constants_statuscodes_bs__t_StatusCode_i * const address_space__statusCode,
   constants__t_Node_i * const address_space__node) {
   {
      t_bool address_space__l_isvalid;
      
      *address_space__statusCode = constants_statuscodes_bs__e_sc_ok;
      *address_space__node = constants__c_Node_indet;
      if (address_space__nodeid == constants__c_NodeId_indet) {
         *address_space__statusCode = constants_statuscodes_bs__e_sc_bad_node_id_invalid;
      }
      else {
         address_space_bs__readall_AddressSpace_Node(address_space__nodeid,
            &address_space__l_isvalid,
            address_space__node);
         if (address_space__l_isvalid == false) {
            *address_space__statusCode = constants_statuscodes_bs__e_sc_bad_node_id_unknown;
         }
      }
   }
}

void address_space__read_variable_compat_type(
   const constants__t_NodeId_i address_space__p_dv_typ_nid,
   const t_entier4 address_space__p_dv_vr,
   const constants__t_NodeId_i address_space__p_var_typ_nid,
   const t_entier4 address_space__p_var_vr,
   t_bool * const address_space__btyp_ok,
   t_bool * const address_space__btyp_need_conv) {
   address_space__is_variable_compat_type(address_space__p_dv_typ_nid,
      address_space__p_dv_vr,
      address_space__p_var_typ_nid,
      address_space__p_var_vr,
      address_space__btyp_ok,
      address_space__btyp_need_conv);
}

