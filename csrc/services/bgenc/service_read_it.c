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

 File Name            : service_read_it.c

 Date                 : 29/01/2019 09:56:43

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "service_read_it.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
t_entier4 service_read_it__rreqs_i;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void service_read_it__INITIALISATION(void) {
   service_read_it__rreqs_i = 0;
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void service_read_it__init_iter_write_request(
   const t_entier4 service_read_it__nb_req,
   t_bool * const service_read_it__continue) {
   service_read_it__rreqs_i = service_read_it__nb_req;
   *service_read_it__continue = (0 < service_read_it__nb_req);
}

void service_read_it__continue_iter_write_request(
   t_bool * const service_read_it__continue,
   constants__t_ReadValue_i * const service_read_it__rvi) {
   constants__read_cast_t_ReadValue(service_read_it__rreqs_i,
      service_read_it__rvi);
   service_read_it__rreqs_i = service_read_it__rreqs_i -
      1;
   *service_read_it__continue = (0 < service_read_it__rreqs_i);
}

