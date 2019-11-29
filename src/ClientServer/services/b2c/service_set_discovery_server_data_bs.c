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

#include "service_set_discovery_server_data_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void service_set_discovery_server_data_bs__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void service_set_discovery_server_data_bs__get_RegisteredServer_ServerUri(
    const constants__t_RegisteredServer_i service_set_discovery_server_data_bs__p_reg_server,
    constants__t_ServerUri* const service_set_discovery_server_data_bs__p_server_uri)
{
    *service_set_discovery_server_data_bs__p_server_uri =
        &service_set_discovery_server_data_bs__p_reg_server->ServerUri;
}

void service_set_discovery_server_data_bs__is_empty_ServerUri(
    const constants__t_ServerUri service_set_discovery_server_data_bs__p_server_uri,
    t_bool* const service_set_discovery_server_data_bs__p_bool)
{
    *service_set_discovery_server_data_bs__p_bool = service_set_discovery_server_data_bs__p_server_uri->Length <= 0;
}

void service_set_discovery_server_data_bs__is_equal_ServerUri(
    const constants__t_ServerUri service_set_discovery_server_data_bs__p_left,
    const constants__t_ServerUri service_set_discovery_server_data_bs__p_right,
    t_bool* const service_set_discovery_server_data_bs__p_bool)
{
    int32_t comparison = -1;
    SOPC_ReturnStatus status = SOPC_String_Compare(service_set_discovery_server_data_bs__p_left,
                                                   service_set_discovery_server_data_bs__p_right, true, &comparison);
    *service_set_discovery_server_data_bs__p_bool = (SOPC_STATUS_OK == status && 0 == comparison);
}
