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

#ifndef SOPC_PUBSUB_PROTOCOL_H_
#define SOPC_PUBSUB_PROTOCOL_H_

#define UADP_PREFIX "opc.udp://"
#define MQTT_PREFIX "mqtts://"
#define ETH_PREFIX "opc.eth://"
#define ETH_ETHERTYPE 0xB62C

#include "sopc_mqtt_transport_layer.h"

typedef enum
{
    SOPC_PubSubProtocol_UNKOWN = 0,
    SOPC_PubSubProtocol_UDP = 1,
    SOPC_PubSubProtocol_MQTT = 2,
    SOPC_PubSubProtocol_ETH = 3
} SOPC_PubSubProtocol_Type;

/* Returns singleton of mqtt manager. Not thread safe. */
MqttManagerHandle* SOPC_PubSub_Protocol_GetMqttManagerHandle(void);
/* Destroy singleton of mqtt manager. Not thread safe. */
void SOPC_PubSub_Protocol_ReleaseMqttManagerHandle(void);

SOPC_PubSubProtocol_Type SOPC_PubSub_Protocol_From_URI(const char* uri);

#endif /* SOPC_PUBSUB_PROTOCOL_H_ */
