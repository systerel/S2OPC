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

#ifndef CONFIG_H_
#define CONFIG_H_

/** Defaults configuration values for the sample */

/* Default paths */
#define SAFETY_XML_PROVIDER_DEMO "./safety_demo_prov.xml"
#define SAFETY_XML_CONSUMER_DEMO "./safety_demo_cons.xml"
#define LOG_PATH "./logs"
#define SKS_SIGNING_KEY "./signingKey.key"
#define SKS_ENCRYPTION_KEY "./encryptKey.key"
#define SKS_KEY_NONCE "./keyNonce.key"

/* Sleep period of the main loop */
#define SLEEP_TIMEOUT (100)

/* NodeIds of SPDUs, shall match the configuration */
#define NODEID_SPDU_REQUEST_NUM 0x01000001

#define NODEID_SPDU_RESPONSE_NUM 0x01000002

/** User cyclic application time (ms) */
#define USER_APP_CYCLE_DURATION_MS (50u)

#endif /* CONFIG_H_ */
