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

#include "libs2opc_common_monitoring.h"

#include "sopc_secure_channels_api.h"
#include "sopc_services_api.h"
#include "sopc_sockets_api.h"

SOPC_CommonMonitoring_QueueSize SOPC_CommonMonitoring_GetQueueSize(SOPC_CommonMonitoring_QueueType queueType)
{
    SOPC_CommonMonitoring_QueueSize result = 0;

    switch (queueType)
    {
    case SOPC_CommonMonitoring_QueueType_Services:
        result = SOPC_Services_Get_QueueSize();
        break;
    case SOPC_CommonMonitoring_QueueType_SecuredChannels:
        result = SOPC_SecureChannels_Get_QueueSize();
        break;
    case SOPC_CommonMonitoring_QueueType_Sockets:
        result = SOPC_Sockets_Get_QueueSize();
        break;
    default:
        break;
    }
    return result;
}
