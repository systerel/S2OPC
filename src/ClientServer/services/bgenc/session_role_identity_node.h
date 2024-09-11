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

 File Name            : session_role_identity_node.h

 Date                 : 11/09/2024 10:08:08

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _session_role_identity_node_h
#define _session_role_identity_node_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "address_space_itf.h"
#include "constants.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void session_role_identity_node__INITIALISATION(void);

/*--------------------------
   LOCAL_OPERATIONS Clause
  --------------------------*/
extern void session_role_identity_node__l_check_node_NodeClass_and_BrowseName(
   const constants__t_Node_i session_role_identity_node__p_node,
   t_bool * const session_role_identity_node__p_bres);
extern void session_role_identity_node__l_check_ref_isForward_and_RefTypeProperty(
   const constants__t_Reference_i session_role_identity_node__p_ref,
   t_bool * const session_role_identity_node__p_bres);
extern void session_role_identity_node__l_ref_get_node(
   const constants__t_Reference_i session_role_identity_node__p_ref,
   constants__t_Node_i * const session_role_identity_node__p_node,
   constants__t_NodeId_i * const session_role_identity_node__p_nodeId);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void session_role_identity_node__ref_maybe_get_Identity(
   const constants__t_Reference_i session_role_identity_node__p_ref,
   constants__t_Node_i * const session_role_identity_node__p_maybe_node_Identity,
   constants__t_NodeId_i * const session_role_identity_node__p_maybe_nodeId_Identity);

#endif
