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

/** Hard-coded configuration */

#ifndef CONFIG_H_
#define CONFIG_H_

#include <stdbool.h>

#define xstr(s) str(s)
#define str(s) #s

/* Server Configuration */
#define SERVER_CONFIG_XML "device_server_config.xml"
#define SERVER_USER_CONFIG_XML "pubsub_server_users_config.xml"
#define PASSWORD_ENV_NAME "TEST_PASSWORD_PRIVATE_KEY"
#define ADDRESS_SPACE_PATH "./s2opc_pubsub_nodeset_with_SKS_push.xml"
#define SLEEP_TIMEOUT 100

/* Sub Configuration */
#define PUBSUB_CONFIG_PATH "./device_pubsub_SKS_push_server.xml"

#endif /* CONFIG_H_ */
