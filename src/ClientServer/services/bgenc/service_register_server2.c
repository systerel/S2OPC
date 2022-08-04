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

 File Name            : service_register_server2.c

 Date                 : 04/08/2022 14:53:13

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "service_register_server2.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
t_entier4 service_register_server2__RegisteredServer2_Counter_i;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void service_register_server2__INITIALISATION(void) {
   service_register_server2__RegisteredServer2_Counter_i = 0;
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void service_register_server2__local_need_register_server2_update(
   const constants__t_RegisteredServer_i service_register_server2__p_registered_server,
   t_bool * const service_register_server2__hasServerUri,
   t_entier4 * const service_register_server2__recordId) {
   {
      t_bool service_register_server2__l_continue;
      constants__t_RegisteredServer2Info_i service_register_server2__l_registeredServerInfo;
      constants__t_RegisteredServer_i service_register_server2__l_registeredServer;
      constants__t_ServerUri service_register_server2__l_newServerUri;
      constants__t_ServerUri service_register_server2__l_recordedServerUri;
      
      *service_register_server2__recordId = 0;
      *service_register_server2__hasServerUri = false;
      service_register_server2_set_bs__init_iter_registered_server2_set(&service_register_server2__l_continue);
      service_set_discovery_server_data_bs__get_RegisteredServer_ServerUri(service_register_server2__p_registered_server,
         &service_register_server2__l_newServerUri);
      while ((service_register_server2__l_continue == true) &&
         (*service_register_server2__hasServerUri == false)) {
         service_register_server2_set_bs__continue_iter_registered_server2_set(&service_register_server2__l_continue,
            &service_register_server2__l_registeredServerInfo);
         service_register_server2_set_bs__get_registered_server2_registered_server(service_register_server2__l_registeredServerInfo,
            &service_register_server2__l_registeredServer);
         service_set_discovery_server_data_bs__get_RegisteredServer_ServerUri(service_register_server2__l_registeredServer,
            &service_register_server2__l_recordedServerUri);
         service_set_discovery_server_data_bs__is_equal_ServerUri(service_register_server2__l_newServerUri,
            service_register_server2__l_recordedServerUri,
            service_register_server2__hasServerUri);
         if (*service_register_server2__hasServerUri == true) {
            service_register_server2_set_bs__get_registered_server2_recordId(service_register_server2__l_registeredServerInfo,
               service_register_server2__recordId);
         }
      }
      service_register_server2_set_bs__clear_iter_registered_server2_set();
   }
}

void service_register_server2__register_server2_create(
   const constants__t_RegisteredServer_i service_register_server2__p_registered_server,
   const constants__t_MdnsDiscoveryConfig_i service_register_server2__p_mdns_config,
   constants_statuscodes_bs__t_StatusCode_i * const service_register_server2__p_sc) {
   {
      t_bool service_register_server2__l_allocSuccess;
      
      *service_register_server2__p_sc = constants_statuscodes_bs__e_sc_ok;
      if (service_register_server2__RegisteredServer2_Counter_i == MAXINT) {
         service_register_server2_set_bs__reset_registered_server2_set();
         service_register_server2__RegisteredServer2_Counter_i = 0;
      }
      service_register_server2__RegisteredServer2_Counter_i = service_register_server2__RegisteredServer2_Counter_i +
         1;
      service_register_server2_set_bs__add_to_registered_server2_set(service_register_server2__p_registered_server,
         service_register_server2__p_mdns_config,
         service_register_server2__RegisteredServer2_Counter_i,
         &service_register_server2__l_allocSuccess);
      if (service_register_server2__l_allocSuccess == false) {
         service_register_server2__RegisteredServer2_Counter_i = service_register_server2__RegisteredServer2_Counter_i -
            1;
         *service_register_server2__p_sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
      }
   }
}

void service_register_server2__register_server2_remove(
   const constants__t_RegisteredServer_i service_register_server2__p_registered_server) {
   {
      t_bool service_register_server2__l_hasAlreadyServerUri;
      t_entier4 service_register_server2__l_recordIdToRemove;
      
      service_register_server2__local_need_register_server2_update(service_register_server2__p_registered_server,
         &service_register_server2__l_hasAlreadyServerUri,
         &service_register_server2__l_recordIdToRemove);
      if (service_register_server2__l_hasAlreadyServerUri == true) {
         service_register_server2_set_bs__remove_from_registered_server2_set(service_register_server2__l_recordIdToRemove);
      }
   }
}

void service_register_server2__service_register_server2_UNINITIALISATION(void) {
   service_register_server2_set_bs__service_register_server2_set_bs_UNINITIALISATION();
}

