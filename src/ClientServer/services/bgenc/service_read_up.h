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

 File Name            : service_read_up.h

 Date                 : 01/06/2022 13:53:00

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _service_read_up_h
#define _service_read_up_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*-----------------
   IMPORTS Clause
  -----------------*/
#include "service_read_1.h"

/*--------------
   SEES Clause
  --------------*/
#include "address_space.h"
#include "constants.h"
#include "constants_statuscodes_bs.h"
#include "message_in_bs.h"
#include "message_out_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void service_read_up__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void service_read_up__treat_read_request(
   const constants__t_user_i service_read_up__p_user,
   const constants__t_LocaleIds_i service_read_up__p_locales,
   const constants__t_msg_i service_read_up__p_request_msg,
   const constants__t_msg_i service_read_up__p_response_msg,
   constants_statuscodes_bs__t_StatusCode_i * const service_read_up__StatusCode_service);

#endif
