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

/** Defaults configuration values for the sample */

#define IS_LOOPBACK "0"

/* Default paths */
#define PUBSUB_XML_CONFIG_EMIT "./config_rtt_emitter.xml"
#define PUBSUB_XML_CONFIG_LOOP "./config_rtt_loopback.xml"
#define LOG_PATH "./logs/"
#define SKS_SIGNING_KEY "./signingKey.key"
#define SKS_ENCRYPTION_KEY "./encryptKey.key"
#define SKS_KEY_NONCE "./keyNonce.key"

/* Sleep period of the main loop */
#define SLEEP_TIMEOUT 100

/* NodeIds of the counter, shall match the configuration */
#define NODEID_COUNTER_SEND {.IdentifierType=SOPC_IdentifierType_Numeric, .Namespace=1, .Data.Numeric=0}
#define NODEID_COUNTER_RECV {.IdentifierType=SOPC_IdentifierType_Numeric, .Namespace=1, .Data.Numeric=1}
