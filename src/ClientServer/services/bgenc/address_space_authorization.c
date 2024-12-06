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

 File Name            : address_space_authorization.c

 Date                 : 04/11/2024 10:51:41

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "address_space_authorization.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
constants__t_sessionRoles_i address_space_authorization__a_set_roles_i;
constants__t_user_i address_space_authorization__a_set_user_i;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void address_space_authorization__INITIALISATION(void) {
   address_space_authorization__a_set_user_i = constants__c_user_indet;
   address_space_authorization__a_set_roles_i = constants__c_sessionRoles_indet;
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void address_space_authorization__has_access_level_read(
   const constants__t_Node_i address_space_authorization__node,
   t_bool * const address_space_authorization__bres) {
   {
      constants__t_access_level address_space_authorization__l_access_level;
      t_bool address_space_authorization__l_local_service_treatment;
      
      address_space_local__is_local_service_treatment(&address_space_authorization__l_local_service_treatment);
      if (address_space_authorization__l_local_service_treatment == true) {
         *address_space_authorization__bres = true;
      }
      else {
         address_space_bs__get_AccessLevel(address_space_authorization__node,
            &address_space_authorization__l_access_level);
         constants__is_t_access_level_currentRead(address_space_authorization__l_access_level,
            address_space_authorization__bres);
      }
   }
}

void address_space_authorization__has_access_level_write(
   const constants__t_Node_i address_space_authorization__node,
   const constants__t_RawStatusCode address_space_authorization__raw_sc,
   const constants__t_Timestamp address_space_authorization__source_ts,
   t_bool * const address_space_authorization__bres) {
   {
      constants__t_access_level address_space_authorization__l_access_level;
      t_bool address_space_authorization__l_access_write;
      t_bool address_space_authorization__l_access_write_status;
      t_bool address_space_authorization__l_access_write_timestamp;
      constants_statuscodes_bs__t_StatusCode_i address_space_authorization__l_sc;
      t_bool address_space_authorization__l_local_service_treatment;
      
      address_space_local__is_local_service_treatment(&address_space_authorization__l_local_service_treatment);
      if (address_space_authorization__l_local_service_treatment == true) {
         *address_space_authorization__bres = true;
      }
      else {
         address_space_bs__get_AccessLevel(address_space_authorization__node,
            &address_space_authorization__l_access_level);
         constants__is_t_access_level_currentWrite(address_space_authorization__l_access_level,
            &address_space_authorization__l_access_write);
         constants__is_t_access_level_statusWrite(address_space_authorization__l_access_level,
            &address_space_authorization__l_access_write_status);
         constants__is_t_access_level_timestampWrite(address_space_authorization__l_access_level,
            &address_space_authorization__l_access_write_timestamp);
         if (address_space_authorization__l_access_write_status == false) {
            constants_statuscodes_bs__getall_conv_RawStatusCode_To_StatusCode(address_space_authorization__raw_sc,
               &address_space_authorization__l_sc);
            address_space_authorization__l_access_write_status = (address_space_authorization__l_sc == constants_statuscodes_bs__e_sc_ok);
         }
         if (address_space_authorization__l_access_write_timestamp == false) {
            constants__is_Timestamps_Null(address_space_authorization__source_ts,
               &address_space_authorization__l_access_write_timestamp);
         }
         *address_space_authorization__bres = (((address_space_authorization__l_access_write == true) &&
            (address_space_authorization__l_access_write_status == true)) &&
            (address_space_authorization__l_access_write_timestamp == true));
      }
   }
}

void address_space_authorization__has_access_level_executable(
   const constants__t_Node_i address_space_authorization__node,
   t_bool * const address_space_authorization__bres) {
   {
      t_bool address_space_authorization__l_local_service_treatment;
      
      address_space_local__is_local_service_treatment(&address_space_authorization__l_local_service_treatment);
      if (address_space_authorization__l_local_service_treatment == true) {
         *address_space_authorization__bres = true;
      }
      else {
         address_space_bs__get_Executable(address_space_authorization__node,
            address_space_authorization__bres);
      }
   }
}

void address_space_authorization__set_user_roles(
   const constants__t_user_i address_space_authorization__p_user,
   const constants__t_sessionRoles_i address_space_authorization__p_roles) {
   address_space_authorization__a_set_user_i = address_space_authorization__p_user;
   address_space_authorization__a_set_roles_i = address_space_authorization__p_roles;
}

void address_space_authorization__get_user_roles(
   const constants__t_user_i address_space_authorization__p_user,
   constants__t_sessionRoles_i * const address_space_authorization__p_roles) {
   if (address_space_authorization__p_user == address_space_authorization__a_set_user_i) {
      *address_space_authorization__p_roles = address_space_authorization__a_set_roles_i;
   }
   else {
      *address_space_authorization__p_roles = constants__c_sessionRoles_indet;
   }
}

void address_space_authorization__clear_user_roles(void) {
   address_space_authorization__a_set_user_i = constants__c_user_indet;
   address_space_authorization__a_set_roles_i = constants__c_sessionRoles_indet;
}

void address_space_authorization__get_user_authorization(
   const constants__t_operation_type_i address_space_authorization__p_operation_type,
   const constants__t_NodeId_i address_space_authorization__p_node_id,
   const constants__t_AttributeId_i address_space_authorization__p_attribute_id,
   const constants__t_user_i address_space_authorization__p_user,
   const constants__t_sessionRoles_i address_space_authorization__p_roles,
   t_bool * const address_space_authorization__p_authorized) {
   {
      t_bool address_space_authorization__l_local_service_treatment;
      t_bool address_space_authorization__l_has_permissions;
      constants__t_PermissionType_i address_space_authorization__l_permissions;
      
      address_space_local__is_local_service_treatment(&address_space_authorization__l_local_service_treatment);
      if (address_space_authorization__l_local_service_treatment == true) {
         *address_space_authorization__p_authorized = true;
      }
      else {
         user_authorization_bs__get_user_authorization_bs(address_space_authorization__p_operation_type,
            address_space_authorization__p_node_id,
            address_space_authorization__p_attribute_id,
            address_space_authorization__p_user,
            address_space_authorization__p_authorized);
         address_space_user_permissions__has_AddressSpace_RolePermissions(address_space_authorization__p_node_id,
            &address_space_authorization__l_has_permissions);
         if ((*address_space_authorization__p_authorized == true) &&
            (address_space_authorization__l_has_permissions == true)) {
            address_space_user_permissions__read_AddressSpace_UserRolePermissions(address_space_authorization__p_node_id,
               address_space_authorization__p_user,
               address_space_authorization__p_roles,
               &address_space_authorization__l_permissions);
            address_space_user_permissions__is_operation_authorized(address_space_authorization__l_permissions,
               address_space_authorization__p_operation_type,
               address_space_authorization__p_authorized);
         }
      }
   }
}

