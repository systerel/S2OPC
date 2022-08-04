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

 File Name            : service_write_1_it.h

 Date                 : 04/08/2022 14:53:15

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _service_write_1_it_h
#define _service_write_1_it_h

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
extern t_entier4 service_write_1_it__wreqs_i;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void service_write_1_it__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void service_write_1_it__continue_iter_write_request(
   t_bool * const service_write_1_it__continue,
   constants__t_WriteValue_i * const service_write_1_it__wvi);
extern void service_write_1_it__init_iter_write_request(
   const t_entier4 service_write_1_it__nb_req,
   t_bool * const service_write_1_it__continue);

#endif
