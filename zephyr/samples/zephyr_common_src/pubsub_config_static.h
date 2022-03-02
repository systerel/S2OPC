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

#ifndef PUBSUB_CONF_STATIC_H_
#define PUBSUB_CONF_STATIC_H_

#include "sopc_pubsub_conf.h"

#define SERVER_CMD_PUBSUB_PERIOD_MS "ns=1;s=PubPeriodMs"
#define SERVER_CMD_PUBSUB_START_STOP "ns=1;s=PubSubStartStop"
#define SERVER_CMD_PUBSUB_SIGN_ENCRYPT "ns=1;s=PubSignAndEncypt"
#define SERVER_STATUS_SUB_VAR_INFO "ns=1;s=SubVarInfo"
#define SERVER_STATUS_SUB_VAR_RESULT "ns=1;s=SubVarResult"
#define SERVER_STATUS_PUB_VAR_LOOP "ns=1;s=PubVarLoops"
#define SERVER_STATUS_SUB_VAR_UPTIME "ns=1;s=SubVarUpTime"

SOPC_PubSubConfiguration* SOPC_PubSubConfig_GetStatic(void);
void SOPC_PubSubConfig_Clear(void);

#endif /* PUBSUB_CONF_STATIC_H_ */
