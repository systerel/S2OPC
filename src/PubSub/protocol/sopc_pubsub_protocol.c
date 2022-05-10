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

#include "sopc_pubsub_protocol.h"

#include <stdbool.h>
#include <string.h>
#include "sopc_helper_string.h"
#include "sopc_macros.h"

static MqttManagerHandle* g_ptrMqttManagerHandle = NULL;

static bool SOPC_PubSub_Protocol_StartWith(const char* uri, const char* prefix);

static bool SOPC_PubSub_Protocol_StartWith(const char* uri, const char* prefix)
{
    const size_t PREFIX_LENGTH = strlen(prefix);
    return (strlen(uri) > PREFIX_LENGTH && SOPC_strncmp_ignore_case(uri, prefix, PREFIX_LENGTH) == 0);
}

MqttManagerHandle* SOPC_PubSub_Protocol_GetMqttManagerHandle(void)
{
    if (NULL == g_ptrMqttManagerHandle)
    {
        // Ignore the function return since in case of failure g_ptrMqttManagerHandle will remain NULL
        SOPC_UNUSED_RESULT(SOPC_MQTT_MGR_Create(&g_ptrMqttManagerHandle));
    }
    return g_ptrMqttManagerHandle;
}

void SOPC_PubSub_Protocol_ReleaseMqttManagerHandle(void)
{
    if (NULL != g_ptrMqttManagerHandle)
    {
        SOPC_MQTT_MGR_Destroy(&g_ptrMqttManagerHandle);
    }
}

SOPC_PubSubProtocol_Type SOPC_PubSub_Protocol_From_URI(const char* uri)
{
    if (SOPC_PubSub_Protocol_StartWith(uri, UADP_PREFIX))
    {
        return SOPC_PubSubProtocol_UDP;
    }
    else if (SOPC_PubSub_Protocol_StartWith(uri, MQTT_PREFIX))
    {
        return SOPC_PubSubProtocol_MQTT;
    }
    else if (SOPC_PubSub_Protocol_StartWith(uri, ETH_PREFIX))
    {
        return SOPC_PubSubProtocol_ETH;
    }
    else
    {
        return SOPC_PubSubProtocol_UNKOWN;
    }
}
