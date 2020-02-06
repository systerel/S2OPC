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

/** \file
 *
 * Implements the structures behind the address space.
 */

#include "service_get_endpoints_bs.h"
#include "b2c.h"
#include "util_discovery_services.h"

#include "sopc_types.h"

#include <assert.h>
#include <stdbool.h>

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void service_get_endpoints_bs__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void service_get_endpoints_bs__treat_get_endpoints_request(
    const constants__t_msg_i service_get_endpoints_bs__req_msg,
    const constants__t_msg_i service_get_endpoints_bs__resp_msg,
    const constants__t_endpoint_config_idx_i service_get_endpoints_bs__endpoint_config_idx,
    constants_statuscodes_bs__t_StatusCode_i* const service_get_endpoints_bs__ret)
{
    bool getEndpoints = false;
    OpcUa_GetEndpointsRequest* getEndpointsReq = (OpcUa_GetEndpointsRequest*) service_get_endpoints_bs__req_msg;
    OpcUa_GetEndpointsResponse* getEndpointsResp = (OpcUa_GetEndpointsResponse*) service_get_endpoints_bs__resp_msg;

    if (getEndpointsReq->NoOfProfileUris > 0)
    {
        // Check if profile URI TCP UA binary is requested, we de not provide any other
        getEndpoints = SOPC_Discovery_ContainsBinaryProfileURI((uint32_t) getEndpointsReq->NoOfProfileUris,
                                                               getEndpointsReq->ProfileUris);
    }
    else
    {
        getEndpoints = true;
    }
    if (true == getEndpoints)
    {
        *service_get_endpoints_bs__ret = SOPC_Discovery_GetEndPointsDescriptions(
            service_get_endpoints_bs__endpoint_config_idx, false, &getEndpointsReq->EndpointUrl,
            getEndpointsReq->NoOfLocaleIds, getEndpointsReq->LocaleIds, (uint32_t*) &getEndpointsResp->NoOfEndpoints,
            &getEndpointsResp->Endpoints);
    }
    else
    {
        // No endpoint to return in case incompatible profile URI provided
        *service_get_endpoints_bs__ret = constants_statuscodes_bs__e_sc_ok;
        getEndpointsResp->NoOfEndpoints = 0;
    }
}
