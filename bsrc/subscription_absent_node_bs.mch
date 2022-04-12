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

MACHINE
    subscription_absent_node_bs
SEES
    constants,
    constants_statuscodes_bs
OPERATIONS

    bres_knownNode, knownNodeClass, valueSc <-- absent_Node_is_known(p_endpoint_config_idx, p_nid) =
    PRE
        p_endpoint_config_idx : t_endpoint_config_idx_i &
        p_endpoint_config_idx : t_endpoint_config_idx &
        p_nid : t_NodeId_i &
        p_nid : t_NodeId
    THEN
        /* TODO: add a way to indicate RawStatus has Bad mask set instead of using conversion result /= ok which includes Uncertain values */
        bres_knownNode, knownNodeClass, valueSc :(
            (bres_knownNode : BOOL & knownNodeClass : t_NodeClass_i & valueSc : t_RawStatusCode) &
            (bres_knownNode = TRUE => knownNodeClass : t_NodeClass & conv_RawStatusCode_To_StatusCode(valueSc) /= e_sc_ok))
    END
    ;

    /* Used to avoid unused variable in father machine */
    nodeId_do_nothing(p_nid) =
    PRE
        p_nid : t_NodeId_i &
        p_nid : t_NodeId
    THEN
        skip
    END

END
