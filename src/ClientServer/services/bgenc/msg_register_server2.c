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

 File Name            : msg_register_server2.c

 Date                 : 04/08/2022 14:53:08

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "msg_register_server2.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void msg_register_server2__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void msg_register_server2__check_register_server2_req(
   const constants__t_msg_i msg_register_server2__p_req,
   constants_statuscodes_bs__t_StatusCode_i * const msg_register_server2__p_sc,
   t_bool * const msg_register_server2__p_is_online,
   t_entier4 * const msg_register_server2__p_nb_discovery_config,
   t_entier4 * const msg_register_server2__p_mdns_discovery_config_index,
   constants__t_RegisteredServer_i * const msg_register_server2__p_registered_server,
   constants__t_MdnsDiscoveryConfig_i * const msg_register_server2__p_mdns_config) {
   {
      t_bool msg_register_server2__l_valid_server_uri;
      t_bool msg_register_server2__l_valid_product_uri;
      t_bool msg_register_server2__l_valid_server_names;
      t_bool msg_register_server2__l_valid_server_type;
      t_bool msg_register_server2__l_valid_discovery_url;
      t_bool msg_register_server2__l_valid_semaphore_file;
      t_bool msg_register_server2__l_has_discovery_configuration;
      t_bool msg_register_server2__l_has_one_and_only_one_mdns_config;
      t_bool msg_register_server2__l_valid_mdns_server_name;
      t_bool msg_register_server2__l_valid_server_capabilities;
      
      msg_register_server2_bs__get_register_server2_req_registered_server(msg_register_server2__p_req,
         msg_register_server2__p_registered_server);
      msg_register_server2_bs__check_registered_server_uri(*msg_register_server2__p_registered_server,
         &msg_register_server2__l_valid_server_uri);
      msg_register_server2_bs__check_registered_product_uri(*msg_register_server2__p_registered_server,
         &msg_register_server2__l_valid_product_uri);
      msg_register_server2_bs__check_registered_server_names(*msg_register_server2__p_registered_server,
         &msg_register_server2__l_valid_server_names);
      msg_register_server2_bs__check_registered_server_type(*msg_register_server2__p_registered_server,
         &msg_register_server2__l_valid_server_type);
      msg_register_server2_bs__check_registered_discovery_url(*msg_register_server2__p_registered_server,
         &msg_register_server2__l_valid_discovery_url);
      msg_register_server2_bs__check_registered_semaphore_file(*msg_register_server2__p_registered_server,
         &msg_register_server2__l_valid_semaphore_file);
      msg_register_server2_bs__get_registered_server_is_online(*msg_register_server2__p_registered_server,
         msg_register_server2__p_is_online);
      msg_register_server2_bs__getall_register_server2_req_msdn_discovery_config(msg_register_server2__p_req,
         &msg_register_server2__l_has_discovery_configuration,
         &msg_register_server2__l_has_one_and_only_one_mdns_config,
         msg_register_server2__p_mdns_config,
         msg_register_server2__p_nb_discovery_config,
         msg_register_server2__p_mdns_discovery_config_index);
      if ((msg_register_server2__l_has_discovery_configuration == true) &&
         (msg_register_server2__l_has_one_and_only_one_mdns_config == true)) {
         msg_register_server2_bs__check_mdns_server_name(*msg_register_server2__p_mdns_config,
            &msg_register_server2__l_valid_mdns_server_name);
         msg_register_server2_bs__check_mdns_server_capabilities(*msg_register_server2__p_mdns_config,
            &msg_register_server2__l_valid_server_capabilities);
      }
      else {
         msg_register_server2__l_valid_mdns_server_name = false;
         msg_register_server2__l_valid_server_capabilities = false;
      }
      *msg_register_server2__p_sc = constants_statuscodes_bs__e_sc_ok;
      if (msg_register_server2__l_valid_server_uri == false) {
         *msg_register_server2__p_sc = constants_statuscodes_bs__e_sc_bad_server_uri_invalid;
      }
      else if (msg_register_server2__l_valid_product_uri == false) {
         *msg_register_server2__p_sc = constants_statuscodes_bs__e_sc_bad_invalid_argument;
      }
      else if (msg_register_server2__l_valid_server_names == false) {
         *msg_register_server2__p_sc = constants_statuscodes_bs__e_sc_bad_server_name_missing;
      }
      else if (msg_register_server2__l_valid_server_type == false) {
         *msg_register_server2__p_sc = constants_statuscodes_bs__e_sc_bad_invalid_argument;
      }
      else if (msg_register_server2__l_valid_discovery_url == false) {
         *msg_register_server2__p_sc = constants_statuscodes_bs__e_sc_bad_discovery_url_missing;
      }
      else if (msg_register_server2__l_valid_semaphore_file == false) {
         *msg_register_server2__p_sc = constants_statuscodes_bs__e_sc_bad_semaphore_file_missing;
      }
      else if ((msg_register_server2__l_has_discovery_configuration == false) ||
         (msg_register_server2__l_has_one_and_only_one_mdns_config == false)) {
         *msg_register_server2__p_sc = constants_statuscodes_bs__e_sc_bad_invalid_argument;
      }
      else if ((msg_register_server2__l_valid_mdns_server_name == false) ||
         (msg_register_server2__l_valid_server_capabilities == false)) {
         *msg_register_server2__p_sc = constants_statuscodes_bs__e_sc_bad_invalid_argument;
      }
   }
}

