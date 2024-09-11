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

 File Name            : session_role_eval.h

 Date                 : 11/09/2024 10:08:07

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _session_role_eval_h
#define _session_role_eval_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*-----------------
   IMPORTS Clause
  -----------------*/
#include "role_references_it.h"
#include "session_role_identities_bs.h"
#include "session_role_identities_it.h"
#include "session_role_identity_eval.h"
#include "session_role_identity_node.h"

/*--------------
   SEES Clause
  --------------*/
#include "address_space_itf.h"
#include "constants.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void session_role_eval__INITIALISATION(void);

/*--------------------------
   LOCAL_OPERATIONS Clause
  --------------------------*/
extern void session_role_eval__l_check_node_NodeClass_and_TypeDef(
   const constants__t_Node_i session_role_eval__p_node,
   t_bool * const session_role_eval__p_bres);
extern void session_role_eval__l_check_ref_isForward_and_RefTypeComponent(
   const constants__t_Reference_i session_role_eval__p_ref,
   t_bool * const session_role_eval__p_bres);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void session_role_eval__is_ref_role(
   const constants__t_Reference_i session_role_eval__p_ref,
   t_bool * const session_role_eval__p_bres,
   constants__t_Node_i * const session_role_eval__p_maybe_role_node,
   constants__t_NodeId_i * const session_role_eval__p_maybe_role_nodeId);
extern void session_role_eval__role_eval_user(
   const constants__t_user_i session_role_eval__p_user,
   const constants__t_LocaleIds_i session_role_eval__p_locales,
   const constants__t_Node_i session_role_eval__p_role_node,
   t_bool * const session_role_eval__p_bres);

#endif
