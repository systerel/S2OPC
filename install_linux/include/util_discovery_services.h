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

#ifndef UTIL_DISCOVERY_SERVICES_H_
#define UTIL_DISCOVERY_SERVICES_H_

#include "constants.h"
#include "constants_statuscodes_bs.h"
#include "sopc_builtintypes.h"
#include "sopc_types.h"

bool SOPC_Discovery_ContainsBinaryProfileURI(uint32_t nbOfProfileURI, SOPC_String* profileUris);

constants_statuscodes_bs__t_StatusCode_i SOPC_Discovery_GetEndPointsDescriptions(
    const constants__t_endpoint_config_idx_i endpoint_config_idx,
    bool isCreateSessionResponse,
    SOPC_String* EndpointUrl,
    int32_t nbOfLocaleIds,
    SOPC_String* localeIdArray,
    uint32_t* nbOfEndpointDescriptions,                // output param
    OpcUa_EndpointDescription** endpointDescriptions); // output param

#endif /* UTIL_DISCOVERY_SERVICES_H_ */
