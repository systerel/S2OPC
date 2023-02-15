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
    service_add_nodes

SEES
    constants,
    constants_statuscodes_bs,
    message_in_bs,
    message_out_bs

DEFINITIONS
    "address_space.def"

ABSTRACT_VARIABLES
    d_adsp_variables

INVARIANT
    d_adsp_invariant

INITIALISATION
    d_adsp_variables
    :(
        d_adsp_invariant
    )

OPERATIONS

    d_adsp_operations;
    d_adsp_write_operation;
    d_adsp_call_operation;

    StatusCode_service <-- treat_add_nodes_request(p_user, p_req_msg, p_resp_msg) =
    PRE
        p_user : t_user_i        &
        p_user : t_user          &
        p_req_msg : t_msg_i  &
        p_req_msg = c_msg_in &
        c_msg_in : t_msg         &
        a_msg_in_type = e_msg_node_add_nodes_req &
        p_resp_msg : t_msg_i   &
        p_resp_msg = c_msg_out &
        c_msg_out : t_msg &
        a_msg_out_type = e_msg_node_add_nodes_resp
    THEN
        IF ConstantAddressSpace = TRUE THEN
            StatusCode_service := e_sc_bad_service_unsupported
        ELSE
            StatusCode_service,
            d_adsp_nodes_variables
            :(
                StatusCode_service       : t_StatusCode_i &
                StatusCode_service       : {e_sc_ok, e_sc_bad_nothing_to_do, e_sc_bad_too_many_ops, e_sc_bad_out_of_memory}  &
                d_adsp_nodes_invariant
            )
        END
    END

END
