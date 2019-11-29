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

 File Name            : service_set_discovery_server_data_bs.h

 Date                 : 29/11/2019 15:10:31

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _service_set_discovery_server_data_bs_h
#define _service_set_discovery_server_data_bs_h

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
extern void service_set_discovery_server_data_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void service_set_discovery_server_data_bs__get_RegisteredServer_ServerUri(
   const constants__t_RegisteredServer_i service_set_discovery_server_data_bs__p_reg_server,
   constants__t_ServerUri * const service_set_discovery_server_data_bs__p_server_uri);
extern void service_set_discovery_server_data_bs__is_empty_ServerUri(
   const constants__t_ServerUri service_set_discovery_server_data_bs__p_server_uri,
   t_bool * const service_set_discovery_server_data_bs__p_bool);
extern void service_set_discovery_server_data_bs__is_equal_ServerUri(
   const constants__t_ServerUri service_set_discovery_server_data_bs__p_left,
   const constants__t_ServerUri service_set_discovery_server_data_bs__p_right,
   t_bool * const service_set_discovery_server_data_bs__p_bool);

#endif
