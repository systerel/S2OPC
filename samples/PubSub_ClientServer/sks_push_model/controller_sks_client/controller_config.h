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
#define SERVER_CONFIG_XML "controller_server_config.xml"
#define PASSWORD_ENV_NAME "TEST_PASSWORD_PRIVATE_KEY"
#define USER_PASSWORD_ENV_NAME "TEST_PASSWORD_USER"
#define ADDRESS_SPACE_PATH "./s2opc_pubsub_nodeset.xml"
#define SLEEP_TIMEOUT 100

/* Client Configuration */
#define CLIENT_CONFIG_XML "controller_client_config.xml"

// Key Lifetime is 5s
#define SKS_KEYLIFETIME 5000
// Number of keys generated randomly
#define SKS_NB_GENERATED_KEYS 5
// Maximum number of Security Keys managed. When the number of keys exceed this limit, only the valid Keys are kept
#define SKS_NB_MAX_KEYS 20

#define SKS_SCHEDULER_INIT_MSPERIOD 1000 // Period to init the scheduler is 1s

/* Sub Configuration */
#define PUBSUB_CONFIG_PATH "./controller_pubsub_SKS_client.xml"

#endif /* CONFIG_H_ */
