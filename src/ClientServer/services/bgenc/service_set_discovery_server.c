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

 Date                 : 04/08/2022 14:53:13

 C Translator Version : tradc Java V1.2 (06/02/2022)

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
         service_register_server2__continue_iter_registered_server2_set(&service_set_discovery_server__l_continue,
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
         service_register_server2__continue_iter_registered_server2_set(&service_set_discovery_server__l_continue,
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

void service_set_discovery_server__local_get_nb_servers_to_return(
   const t_entier4 service_set_discovery_server__p_nbServerUri,
   const constants__t_ServerUris service_set_discovery_server__p_ServerUris,
   t_entier4 * const service_set_discovery_server__p_nb_servers) {
   {
      t_bool service_set_discovery_server__l_continue;
      constants__t_RegisteredServer2Info_i service_set_discovery_server__l_registeredServerInfo;
      constants__t_RegisteredServer_i service_set_discovery_server__l_registeredServer;
      constants__t_ServerUri service_set_discovery_server__l_serverUri;
      t_bool service_set_discovery_server__l_compatServerUri;
      
      if (service_set_discovery_server__p_nbServerUri <= 0) {
         service_register_server2__get_card_register2_set(service_set_discovery_server__p_nb_servers);
      }
      else {
         *service_set_discovery_server__p_nb_servers = 0;
         service_register_server2__init_iter_registered_server2_set(&service_set_discovery_server__l_continue);
         while (service_set_discovery_server__l_continue == true) {
            service_register_server2__continue_iter_registered_server2_set(&service_set_discovery_server__l_continue,
               &service_set_discovery_server__l_registeredServerInfo);
            service_register_server2__get_registered_server2_registered_server(service_set_discovery_server__l_registeredServerInfo,
               &service_set_discovery_server__l_registeredServer);
            service_set_discovery_server_data_bs__get_RegisteredServer_ServerUri(service_set_discovery_server__l_registeredServer,
               &service_set_discovery_server__l_serverUri);
            service_set_discovery_server_data_bs__has_ServerUri(service_set_discovery_server__l_serverUri,
               service_set_discovery_server__p_nbServerUri,
               service_set_discovery_server__p_ServerUris,
               &service_set_discovery_server__l_compatServerUri);
            if (service_set_discovery_server__l_compatServerUri == true) {
               *service_set_discovery_server__p_nb_servers = *service_set_discovery_server__p_nb_servers +
                  1;
            }
         }
         service_register_server2__clear_iter_registered_server2_set();
      }
   }
}

void service_set_discovery_server__local_add_self_server_to_return(
   const constants__t_endpoint_config_idx_i service_set_discovery_server__p_endpoint_config_idx,
   const t_entier4 service_set_discovery_server__p_nbServerUri,
   const constants__t_ServerUris service_set_discovery_server__p_ServerUris,
   const t_entier4 service_set_discovery_server__p_nbServersIn,
   t_bool * const service_set_discovery_server__p_compatSelf,
   constants__t_ApplicationDescription_i * const service_set_discovery_server__p_appDesc,
   t_entier4 * const service_set_discovery_server__p_nbServersOut) {
   {
      constants__t_ServerUri service_set_discovery_server__l_serverUri;
      
      *service_set_discovery_server__p_nbServersOut = service_set_discovery_server__p_nbServersIn;
      *service_set_discovery_server__p_compatSelf = false;
      service_set_discovery_server_data_bs__get_ApplicationDescription(service_set_discovery_server__p_endpoint_config_idx,
         service_set_discovery_server__p_appDesc);
      service_set_discovery_server_data_bs__get_ApplicationDescription_ServerUri(*service_set_discovery_server__p_appDesc,
         &service_set_discovery_server__l_serverUri);
      if (service_set_discovery_server__p_nbServerUri > 0) {
         service_set_discovery_server_data_bs__has_ServerUri(service_set_discovery_server__l_serverUri,
            service_set_discovery_server__p_nbServerUri,
            service_set_discovery_server__p_ServerUris,
            service_set_discovery_server__p_compatSelf);
      }
      else {
         *service_set_discovery_server__p_compatSelf = true;
      }
      if (*service_set_discovery_server__p_compatSelf == true) {
         *service_set_discovery_server__p_nbServersOut = service_set_discovery_server__p_nbServersIn +
            1;
      }
   }
}

void service_set_discovery_server__local_set_servers_to_return(
   const constants__t_msg_i service_set_discovery_server__p_resp,
   const constants__t_LocaleIds_i service_set_discovery_server__p_localeIds,
   const t_entier4 service_set_discovery_server__p_nbServerUri,
   const constants__t_ServerUris service_set_discovery_server__p_ServerUris,
   const t_entier4 service_set_discovery_server__p_nb_servers,
   constants_statuscodes_bs__t_StatusCode_i * const service_set_discovery_server__ret) {
   {
      t_bool service_set_discovery_server__l_continue;
      t_entier4 service_set_discovery_server__l_nb_servers;
      constants__t_RegisteredServer2Info_i service_set_discovery_server__l_registeredServerInfo;
      constants__t_RegisteredServer_i service_set_discovery_server__l_registeredServer;
      constants__t_ServerUri service_set_discovery_server__l_serverUri;
      t_bool service_set_discovery_server__l_compatServerUri;
      
      *service_set_discovery_server__ret = constants_statuscodes_bs__e_sc_ok;
      service_set_discovery_server__l_nb_servers = 0;
      service_register_server2__init_iter_registered_server2_set(&service_set_discovery_server__l_continue);
      while (((*service_set_discovery_server__ret == constants_statuscodes_bs__e_sc_ok) &&
         (service_set_discovery_server__l_continue == true)) &&
         (service_set_discovery_server__l_nb_servers < service_set_discovery_server__p_nb_servers)) {
         service_register_server2__continue_iter_registered_server2_set(&service_set_discovery_server__l_continue,
            &service_set_discovery_server__l_registeredServerInfo);
         service_register_server2__get_registered_server2_registered_server(service_set_discovery_server__l_registeredServerInfo,
            &service_set_discovery_server__l_registeredServer);
         if (service_set_discovery_server__p_nbServerUri > 0) {
            service_set_discovery_server_data_bs__get_RegisteredServer_ServerUri(service_set_discovery_server__l_registeredServer,
               &service_set_discovery_server__l_serverUri);
            service_set_discovery_server_data_bs__has_ServerUri(service_set_discovery_server__l_serverUri,
               service_set_discovery_server__p_nbServerUri,
               service_set_discovery_server__p_ServerUris,
               &service_set_discovery_server__l_compatServerUri);
            if (service_set_discovery_server__l_compatServerUri == true) {
               msg_find_servers_bs__set_find_servers_server(service_set_discovery_server__p_resp,
                  service_set_discovery_server__l_nb_servers,
                  service_set_discovery_server__p_localeIds,
                  service_set_discovery_server__l_registeredServer,
                  service_set_discovery_server__ret);
               service_set_discovery_server__l_nb_servers = service_set_discovery_server__l_nb_servers +
                  1;
            }
         }
         else {
            msg_find_servers_bs__set_find_servers_server(service_set_discovery_server__p_resp,
               service_set_discovery_server__l_nb_servers,
               service_set_discovery_server__p_localeIds,
               service_set_discovery_server__l_registeredServer,
               service_set_discovery_server__ret);
            service_set_discovery_server__l_nb_servers = service_set_discovery_server__l_nb_servers +
               1;
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
         service_register_server2__register_server2_remove(service_set_discovery_server__l_registered_server);
         if (service_set_discovery_server__l_is_online == true) {
            service_register_server2__register_server2_create(service_set_discovery_server__l_registered_server,
               service_set_discovery_server__l_mdns_config,
               service_set_discovery_server__ret);
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

void service_set_discovery_server__treat_find_servers_request(
   const constants__t_msg_i service_set_discovery_server__req_msg,
   const constants__t_msg_i service_set_discovery_server__resp_msg,
   const constants__t_endpoint_config_idx_i service_set_discovery_server__endpoint_config_idx,
   constants_statuscodes_bs__t_StatusCode_i * const service_set_discovery_server__ret) {
   {
      constants__t_LocaleIds_i service_set_discovery_server__l_LocaleIds;
      t_entier4 service_set_discovery_server__l_nbServerUri;
      constants__t_ServerUris service_set_discovery_server__l_ServerUris;
      t_entier4 service_set_discovery_server__l_nbServers;
      t_entier4 service_set_discovery_server__l_nbServersPlusSelf;
      constants__t_ApplicationDescription_i service_set_discovery_server__l_appDesc;
      t_bool service_set_discovery_server__l_isCurrentCompat;
      t_bool service_set_discovery_server__l_allocSuccess;
      
      *service_set_discovery_server__ret = constants_statuscodes_bs__e_sc_ok;
      msg_find_servers_bs__get_find_servers_req_params(service_set_discovery_server__req_msg,
         &service_set_discovery_server__l_LocaleIds,
         &service_set_discovery_server__l_nbServerUri,
         &service_set_discovery_server__l_ServerUris);
      service_set_discovery_server__local_get_nb_servers_to_return(service_set_discovery_server__l_nbServerUri,
         service_set_discovery_server__l_ServerUris,
         &service_set_discovery_server__l_nbServers);
      service_set_discovery_server__local_add_self_server_to_return(service_set_discovery_server__endpoint_config_idx,
         service_set_discovery_server__l_nbServerUri,
         service_set_discovery_server__l_ServerUris,
         service_set_discovery_server__l_nbServers,
         &service_set_discovery_server__l_isCurrentCompat,
         &service_set_discovery_server__l_appDesc,
         &service_set_discovery_server__l_nbServersPlusSelf);
      if (service_set_discovery_server__l_nbServersPlusSelf > 0) {
         msg_find_servers_bs__alloc_find_servers(service_set_discovery_server__resp_msg,
            service_set_discovery_server__l_nbServersPlusSelf,
            &service_set_discovery_server__l_allocSuccess);
         if (service_set_discovery_server__l_allocSuccess == true) {
            service_set_discovery_server__local_set_servers_to_return(service_set_discovery_server__resp_msg,
               service_set_discovery_server__l_LocaleIds,
               service_set_discovery_server__l_nbServerUri,
               service_set_discovery_server__l_ServerUris,
               service_set_discovery_server__l_nbServers,
               service_set_discovery_server__ret);
            if ((*service_set_discovery_server__ret == constants_statuscodes_bs__e_sc_ok) &&
               (service_set_discovery_server__l_isCurrentCompat == true)) {
               msg_find_servers_bs__set_find_servers_server_ApplicationDescription(service_set_discovery_server__resp_msg,
                  service_set_discovery_server__l_nbServersPlusSelf -
                  1,
                  service_set_discovery_server__l_LocaleIds,
                  service_set_discovery_server__endpoint_config_idx,
                  service_set_discovery_server__l_appDesc,
                  service_set_discovery_server__ret);
            }
         }
         else {
            *service_set_discovery_server__ret = constants_statuscodes_bs__e_sc_bad_out_of_memory;
         }
      }
      constants__free_LocaleIds(service_set_discovery_server__l_LocaleIds);
   }
}

void service_set_discovery_server__service_set_discovery_server_UNINITIALISATION(void) {
   service_register_server2__service_register_server2_UNINITIALISATION();
}

