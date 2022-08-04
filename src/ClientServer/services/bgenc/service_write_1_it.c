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

 File Name            : service_write_1_it.c

 Date                 : 04/08/2022 14:53:15

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "service_write_1_it.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
t_entier4 service_write_1_it__wreqs_i;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void service_write_1_it__INITIALISATION(void) {
   service_write_1_it__wreqs_i = 0;
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void service_write_1_it__init_iter_write_request(
   const t_entier4 service_write_1_it__nb_req,
   t_bool * const service_write_1_it__continue) {
   service_write_1_it__wreqs_i = service_write_1_it__nb_req;
   *service_write_1_it__continue = (0 < service_write_1_it__nb_req);
}

void service_write_1_it__continue_iter_write_request(
   t_bool * const service_write_1_it__continue,
   constants__t_WriteValue_i * const service_write_1_it__wvi) {
   constants__get_cast_t_WriteValue(service_write_1_it__wreqs_i,
      service_write_1_it__wvi);
   service_write_1_it__wreqs_i = service_write_1_it__wreqs_i -
      1;
   *service_write_1_it__continue = (0 < service_write_1_it__wreqs_i);
}

