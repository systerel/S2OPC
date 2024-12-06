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

 File Name            : address_space_user_permissions_bs.h

 Date                 : 30/10/2024 16:24:09

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _address_space_user_permissions_bs_h
#define _address_space_user_permissions_bs_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void address_space_user_permissions_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void address_space_user_permissions_bs__add_user_permissions(
   const constants__t_user_i address_space_user_permissions_bs__p_user,
   const constants__t_PermissionType_i address_space_user_permissions_bs__p_permissions);
extern void address_space_user_permissions_bs__get_merged_user_permissions(
   const constants__t_user_i address_space_user_permissions_bs__p_user,
   constants__t_PermissionType_i * const address_space_user_permissions_bs__p_permissions);
extern void address_space_user_permissions_bs__init_user_permissions(
   const constants__t_user_i address_space_user_permissions_bs__p_user);
extern void address_space_user_permissions_bs__is_operation_authorized(
   const constants__t_PermissionType_i address_space_user_permissions_bs__p_permissions,
   const constants__t_operation_type_i address_space_user_permissions_bs__p_operation_type,
   t_bool * const address_space_user_permissions_bs__p_bres);

#endif
