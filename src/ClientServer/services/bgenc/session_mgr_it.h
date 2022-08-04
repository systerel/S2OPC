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

 File Name            : session_mgr_it.h

 Date                 : 04/08/2022 14:53:19

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _session_mgr_it_h
#define _session_mgr_it_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
extern t_entier4 session_mgr_it__session_i;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void session_mgr_it__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void session_mgr_it__continue_iter_session(
   t_bool * const session_mgr_it__p_continue,
   constants__t_session_i * const session_mgr_it__p_session);
extern void session_mgr_it__init_iter_session(
   t_bool * const session_mgr_it__p_continue);

#endif
