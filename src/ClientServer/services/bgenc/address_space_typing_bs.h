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

 File Name            : address_space_typing_bs.h

 Date                 : 04/08/2022 14:53:27

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _address_space_typing_bs_h
#define _address_space_typing_bs_h

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
extern void address_space_typing_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void address_space_typing_bs__check_object_has_method(
   const constants__t_NodeId_i address_space_typing_bs__p_object,
   const constants__t_NodeId_i address_space_typing_bs__p_method,
   t_bool * const address_space_typing_bs__p_bool);
extern void address_space_typing_bs__is_compatible_simple_type_or_enumeration(
   const constants__t_NodeId_i address_space_typing_bs__p_value_type,
   const constants__t_NodeId_i address_space_typing_bs__p_data_type,
   t_bool * const address_space_typing_bs__bres);
extern void address_space_typing_bs__is_transitive_subtype(
   const constants__t_NodeId_i address_space_typing_bs__p_subtype,
   const constants__t_NodeId_i address_space_typing_bs__p_parent_type,
   t_bool * const address_space_typing_bs__bres);
extern void address_space_typing_bs__is_valid_ReferenceTypeId(
   const constants__t_NodeId_i address_space_typing_bs__p_nodeId,
   t_bool * const address_space_typing_bs__bres);

#endif
