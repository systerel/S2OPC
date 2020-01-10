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

 File Name            : session_channel_it.h

 Date                 : 10/01/2020 17:41:33

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _session_channel_it_h
#define _session_channel_it_h

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
extern t_entier4 session_channel_it__channel_i;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void session_channel_it__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void session_channel_it__continue_iter_channel(
   t_bool * const session_channel_it__p_continue,
   constants__t_channel_i * const session_channel_it__p_channel);
extern void session_channel_it__init_iter_channel(
   t_bool * const session_channel_it__p_continue);

#endif
