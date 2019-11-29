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

 File Name            : service_set_discovery_server.c

 Date                 : 02/12/2019 12:51:29

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "service_set_discovery_server.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void service_set_discovery_server__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void service_set_discovery_server__treat_find_servers_on_network_request(
   const constants__t_msg_i service_set_discovery_server__req_msg,
   const constants__t_msg_i service_set_discovery_server__resp_msg,
   const constants__t_endpoint_config_idx_i service_set_discovery_server__endpoint_config_idx,
   constants_statuscodes_bs__t_StatusCode_i * const service_set_discovery_server__ret) {
   *service_set_discovery_server__ret = constants_statuscodes_bs__e_sc_bad_service_unsupported;
}

void service_set_discovery_server__treat_register_server2_request(
   const constants__t_msg_i service_set_discovery_server__req_msg,
   const constants__t_msg_i service_set_discovery_server__resp_msg,
   constants_statuscodes_bs__t_StatusCode_i * const service_set_discovery_server__ret) {
   {
      t_bool service_set_discovery_server__l_is_online;
      t_entier4 service_set_discovery_server__l_nb_discovery_config;
      t_entier4 service_set_discovery_server__l_mdns_config_index;
      constants__t_RegisteredServer_i service_set_discovery_server__l_registered_server;
      constants__t_MdnsDiscoveryConfig_i service_set_discovery_server__l_mdns_config;
      
      msg_register_server2__check_register_server2_req(service_set_discovery_server__req_msg,
         service_set_discovery_server__ret,
         &service_set_discovery_server__l_is_online,
         &service_set_discovery_server__l_nb_discovery_config,
         &service_set_discovery_server__l_mdns_config_index,
         &service_set_discovery_server__l_registered_server,
         &service_set_discovery_server__l_mdns_config);
      if (*service_set_discovery_server__ret == constants_statuscodes_bs__e_sc_ok) {
         if (service_set_discovery_server__l_is_online == true) {
            service_register_server2__register_server2_create_or_update(service_set_discovery_server__l_registered_server,
               service_set_discovery_server__l_mdns_config,
               service_set_discovery_server__ret);
         }
         else {
            service_register_server2__register_server2_remove(service_set_discovery_server__l_registered_server);
         }
      }
      if (*service_set_discovery_server__ret == constants_statuscodes_bs__e_sc_ok) {
         msg_register_server2__set_register_server2_resp_configuration_results(service_set_discovery_server__resp_msg,
            service_set_discovery_server__l_nb_discovery_config,
            service_set_discovery_server__l_mdns_config_index,
            service_set_discovery_server__ret);
      }
   }
}

void service_set_discovery_server__service_set_discovery_server_UNINITIALISATION(void) {
   service_register_server2__service_register_server2_UNINITIALISATION();
}

