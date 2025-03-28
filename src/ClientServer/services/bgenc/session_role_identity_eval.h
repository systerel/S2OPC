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

 File Name            : session_role_identity_eval.h

 Date                 : 08/08/2024 17:07:11

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _session_role_identity_eval_h
#define _session_role_identity_eval_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*-----------------
   IMPORTS Clause
  -----------------*/
#include "session_role_identity_bs.h"
#include "user_bs.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void session_role_identity_eval__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void session_role_identity_eval__user_and_identity_match(
   const constants__t_user_i session_role_identity_eval__p_user,
   const constants__t_Identity_i session_role_identity_eval__p_identity,
   t_bool * const session_role_identity_eval__p_bres);

#endif
