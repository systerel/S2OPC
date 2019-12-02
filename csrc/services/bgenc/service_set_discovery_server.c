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

 Date                 : 03/12/2019 10:31:10

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
void service_set_discovery_server__local_get_nb_servers_on_network_to_return(
   const t_entier4 service_set_discovery_server__p_starting_record_id,
   const t_entier4 service_set_discovery_server__p_max_records_to_return,
   const constants__t_ServerCapabilities service_set_discovery_server__p_serverCapabilities,
   t_entier4 * const service_set_discovery_server__p_nb_servers) {
   {
      t_bool service_set_discovery_server__l_continue;
      constants__t_RegisteredServer2Info_i service_set_discovery_server__l_registeredServerInfo;
      t_entier4 service_set_discovery_server__l_recordId;
      constants__t_MdnsDiscoveryConfig_i service_set_discovery_server__l_mdnsConfig;
      t_bool service_set_discovery_server__l_compatServerCapabilities;
      
      *service_set_discovery_server__p_nb_servers = 0;
      service_register_server2__init_iter_registered_server2_set(&service_set_discovery_server__l_continue);
      while ((service_set_discovery_server__l_continue == true) &&
         ((service_set_discovery_server__p_max_records_to_return == 0) ||
         (*service_set_discovery_server__p_nb_servers < service_set_discovery_server__p_max_records_to_return))) {
         service_register_server2__continue_iter_monitored_item(&service_set_discovery_server__l_continue,
            &service_set_discovery_server__l_registeredServerInfo);
         service_register_server2__get_registered_server2_recordId(service_set_discovery_server__l_registeredServerInfo,
            &service_set_discovery_server__l_recordId);
         if (service_set_discovery_server__l_recordId >= service_set_discovery_server__p_starting_record_id) {
            service_register_server2__get_registered_server2_mdns_config(service_set_discovery_server__l_registeredServerInfo,
               &service_set_discovery_server__l_mdnsConfig);
            service_set_discovery_server_data_bs__has_ServerCapabilities(service_set_discovery_server__l_mdnsConfig,
               service_set_discovery_server__p_serverCapabilities,
               &service_set_discovery_server__l_compatServerCapabilities);
            if (service_set_discovery_server__l_compatServerCapabilities == true) {
               *service_set_discovery_server__p_nb_servers = *service_set_discovery_server__p_nb_servers +
                  1;
            }
         }
      }
      service_register_server2__clear_iter_registered_server2_set();
   }
}

void service_set_discovery_server__local_set_servers_on_network_to_return(
   const constants__t_msg_i service_set_discovery_server__p_resp,
   const t_entier4 service_set_discovery_server__p_starting_record_id,
   const constants__t_ServerCapabilities service_set_discovery_server__p_serverCapabilities,
   const t_entier4 service_set_discovery_server__p_nb_servers) {
   {
      t_bool service_set_discovery_server__l_continue;
      t_entier4 service_set_discovery_server__l_nb_servers;
      constants__t_RegisteredServer2Info_i service_set_discovery_server__l_registeredServerInfo;
      t_entier4 service_set_discovery_server__l_recordId;
      constants__t_RegisteredServer_i service_set_discovery_server__l_registeredServer;
      constants__t_MdnsDiscoveryConfig_i service_set_discovery_server__l_mdnsConfig;
      t_bool service_set_discovery_server__l_compatServerCapabilities;
      
      service_set_discovery_server__l_nb_servers = 0;
      service_register_server2__init_iter_registered_server2_set(&service_set_discovery_server__l_continue);
      while ((service_set_discovery_server__l_continue == true) &&
         (service_set_discovery_server__l_nb_servers < service_set_discovery_server__p_nb_servers)) {
         service_register_server2__continue_iter_monitored_item(&service_set_discovery_server__l_continue,
            &service_set_discovery_server__l_registeredServerInfo);
         service_register_server2__get_registered_server2_recordId(service_set_discovery_server__l_registeredServerInfo,
            &service_set_discovery_server__l_recordId);
         if (service_set_discovery_server__l_recordId >= service_set_discovery_server__p_starting_record_id) {
            service_register_server2__get_registered_server2_registered_server(service_set_discovery_server__l_registeredServerInfo,
               &service_set_discovery_server__l_registeredServer);
            service_register_server2__get_registered_server2_mdns_config(service_set_discovery_server__l_registeredServerInfo,
               &service_set_discovery_server__l_mdnsConfig);
            service_set_discovery_server_data_bs__has_ServerCapabilities(service_set_discovery_server__l_mdnsConfig,
               service_set_discovery_server__p_serverCapabilities,
               &service_set_discovery_server__l_compatServerCapabilities);
            if (service_set_discovery_server__l_compatServerCapabilities == true) {
               msg_find_servers_on_network_bs__set_find_servers_on_network_server(service_set_discovery_server__p_resp,
                  service_set_discovery_server__l_nb_servers,
                  service_set_discovery_server__l_recordId,
                  service_set_discovery_server__l_registeredServer,
                  service_set_discovery_server__l_mdnsConfig);
               service_set_discovery_server__l_nb_servers = service_set_discovery_server__l_nb_servers +
                  1;
            }
         }
      }
      service_register_server2__clear_iter_registered_server2_set();
   }
}

void service_set_discovery_server__treat_find_servers_on_network_request(
   const constants__t_msg_i service_set_discovery_server__req_msg,
   const constants__t_msg_i service_set_discovery_server__resp_msg,
   constants_statuscodes_bs__t_StatusCode_i * const service_set_discovery_server__ret) {
   {
      t_entier4 service_set_discovery_server__l_startingRecordId;
      t_entier4 service_set_discovery_server__l_maxRecordsToReturn;
      constants__t_ServerCapabilities service_set_discovery_server__l_serverCapabilities;
      constants__t_Timestamp service_set_discovery_server__l_counter_reset_time;
      t_entier4 service_set_discovery_server__l_nb_servers;
      t_bool service_set_discovery_server__l_alloc_success;
      
      *service_set_discovery_server__ret = constants_statuscodes_bs__e_sc_ok;
      msg_find_servers_on_network_bs__get_find_servers_on_network_req_params(service_set_discovery_server__req_msg,
         &service_set_discovery_server__l_startingRecordId,
         &service_set_discovery_server__l_maxRecordsToReturn,
         &service_set_discovery_server__l_serverCapabilities);
      service_register_server2__get_registered_server2_counter_reset_time(&service_set_discovery_server__l_counter_reset_time);
      msg_find_servers_on_network_bs__set_find_servers_on_network_resp(service_set_discovery_server__resp_msg,
         service_set_discovery_server__l_counter_reset_time);
      service_set_discovery_server__local_get_nb_servers_on_network_to_return(service_set_discovery_server__l_startingRecordId,
         service_set_discovery_server__l_maxRecordsToReturn,
         service_set_discovery_server__l_serverCapabilities,
         &service_set_discovery_server__l_nb_servers);
      if (service_set_discovery_server__l_nb_servers > 0) {
         msg_find_servers_on_network_bs__alloc_find_servers_on_network_servers(service_set_discovery_server__resp_msg,
            service_set_discovery_server__l_nb_servers,
            &service_set_discovery_server__l_alloc_success);
         if (service_set_discovery_server__l_alloc_success == true) {
            service_set_discovery_server__local_set_servers_on_network_to_return(service_set_discovery_server__resp_msg,
               service_set_discovery_server__l_startingRecordId,
               service_set_discovery_server__l_serverCapabilities,
               service_set_discovery_server__l_nb_servers);
         }
         else {
            *service_set_discovery_server__ret = constants_statuscodes_bs__e_sc_bad_out_of_memory;
         }
      }
   }
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

