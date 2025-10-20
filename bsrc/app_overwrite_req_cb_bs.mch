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
    /* Machine is used to call the
    */
    app_overwrite_req_cb_bs
SEES
    constants,
    constants_statuscodes_bs,
    message_in_bs

VARIABLES
    hasOverwriteReqCb

INVARIANT

    hasOverwriteReqCb : BOOL

INITIALISATION
    hasOverwriteReqCb :: BOOL

OPERATIONS

    bres <-- has_server_overwrite_req_cb (p_endpoint_config_idx) =
    PRE
        p_endpoint_config_idx : t_endpoint_config_idx_i &
        p_endpoint_config_idx : t_endpoint_config_idx
    THEN
        bres := hasOverwriteReqCb
    END
    ;

    /**
     * new_req_msg = (modified) p_req_msg
     * p_sc =  e_sc_ok => treatment continue
     * p_sc /= e_sc_ok => treatment stopped and service fault returned
     */
    p_sc, new_req_msg <-- overwrite_service_request (p_req_msg) =
    PRE
        hasOverwriteReqCb = TRUE &
        p_req_msg : t_msg_i &
        p_req_msg : t_msg &
        p_req_msg = c_msg_in
    THEN
        p_sc, new_req_msg :(p_sc : t_StatusCode_i &
                            p_sc : t_StatusCode &
                            new_req_msg : t_msg_i & new_req_msg : t_msg)
    END

END
