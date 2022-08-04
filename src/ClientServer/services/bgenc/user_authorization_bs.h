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

 File Name            : user_authorization_bs.h

 Date                 : 04/08/2022 14:53:49

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _user_authorization_bs_h
#define _user_authorization_bs_h

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
extern void user_authorization_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void user_authorization_bs__get_user_authorization(
   const constants__t_operation_type_i user_authorization_bs__p_operation_type,
   const constants__t_NodeId_i user_authorization_bs__p_node_id,
   const constants__t_AttributeId_i user_authorization_bs__p_attribute_id,
   const constants__t_user_i user_authorization_bs__p_user,
   t_bool * const user_authorization_bs__p_authorized);

#endif
