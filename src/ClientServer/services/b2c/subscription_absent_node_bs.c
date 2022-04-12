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

#include "subscription_absent_node_bs.h"

#include "sopc_toolkit_config_internal.h"
#include "util_b2c.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void subscription_absent_node_bs__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void subscription_absent_node_bs__absent_Node_is_known(
    const constants__t_endpoint_config_idx_i subscription_absent_node_bs__p_endpoint_config_idx,
    const constants__t_NodeId_i subscription_absent_node_bs__p_nid,
    t_bool* const subscription_absent_node_bs__bres_knownNode,
    constants__t_NodeClass_i* const subscription_absent_node_bs__knownNodeClass,
    constants__t_RawStatusCode* const subscription_absent_node_bs__valueSc)
{
    *subscription_absent_node_bs__bres_knownNode = false;
    *subscription_absent_node_bs__knownNodeClass = constants__c_NodeClass_indet;
    *subscription_absent_node_bs__valueSc = constants_statuscodes_bs__e_sc_ok;
    SOPC_Endpoint_Config* endpointConfig =
        SOPC_ToolkitServer_GetEndpointConfig(subscription_absent_node_bs__p_endpoint_config_idx);
    assert(NULL != endpointConfig && NULL != endpointConfig->serverConfigPtr);
    SOPC_Server_Config* serverConfig = endpointConfig->serverConfigPtr;
    if (NULL != serverConfig->nodeAvailFunc)
    {
        OpcUa_NodeClass outNodeClass;
        SOPC_StatusCode outUnavailabilityStatus;
        bool res =
            serverConfig->nodeAvailFunc(subscription_absent_node_bs__p_nid, &outNodeClass, &outUnavailabilityStatus);
        // When res is true, the status shall be Bad and the NodeClass shall be valid
        res = res && ((outUnavailabilityStatus & SOPC_BadStatusMask) != 0);
        if (res)
        {
            res = util_NodeClass__C_to_B(outNodeClass, subscription_absent_node_bs__knownNodeClass);
        }
        if (res)
        {
            *subscription_absent_node_bs__bres_knownNode = true;
            *subscription_absent_node_bs__valueSc = outUnavailabilityStatus;
        }
    }
}

void subscription_absent_node_bs__nodeId_do_nothing(const constants__t_NodeId_i subscription_absent_node_bs__p_nid)
{
    // Used to avoid unused variable compile time warning
    (void) subscription_absent_node_bs__p_nid;
}
