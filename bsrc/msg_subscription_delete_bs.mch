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
    msg_subscription_delete_bs

SEES
    constants,
    constants_statuscodes_bs,
    message_in_bs,
    message_out_bs

ABSTRACT_VARIABLES
    delete_subscriptions_req_sub_id,
    delete_subscriptions_resp_allocated,
    delete_subscriptions_resp_StatusCode,
    nb_delete_subscriptions

INVARIANT
    nb_delete_subscriptions : NAT &
    delete_subscriptions_req_sub_id : 1..nb_delete_subscriptions --> t_subscription_i &
    delete_subscriptions_resp_allocated : BOOL &
    delete_subscriptions_resp_StatusCode : 1..k_n_genericOperationPerReq_max +-> t_StatusCode

INITIALISATION
    delete_subscriptions_req_sub_id :(delete_subscriptions_req_sub_id = {}) ||
    delete_subscriptions_resp_allocated := FALSE ||
    delete_subscriptions_resp_StatusCode :(delete_subscriptions_resp_StatusCode = {}) ||
    nb_delete_subscriptions := 0

OPERATIONS

    p_nb_reqs <-- getall_msg_delete_subscriptions_req_params (p_req_msg) =
    PRE
        p_req_msg : t_msg_i &
        p_req_msg : t_msg &
        p_req_msg = c_msg_in &
        a_msg_in_type = e_msg_subscription_delete_subscriptions_req
    THEN
        p_nb_reqs, nb_delete_subscriptions, delete_subscriptions_req_sub_id
        :(p_nb_reqs : NAT & nb_delete_subscriptions = p_nb_reqs &
          delete_subscriptions_req_sub_id : 1..p_nb_reqs --> t_subscription_i)
    END
    ;

    p_sub_id <-- getall_msg_delete_subscriptions_at_index (p_req_msg, p_index) =
    PRE
        p_req_msg : t_msg_i &
        p_req_msg : t_msg &
        p_req_msg = c_msg_in &
        a_msg_in_type = e_msg_subscription_delete_subscriptions_req &
        p_index : NAT &
        p_index <= k_n_genericOperationPerReq_max &
        p_index : 1..nb_delete_subscriptions &
        nb_delete_subscriptions > 0
    THEN
        p_sub_id :(p_sub_id : t_subscription_i)
    END
    ;

    bres <-- allocate_msg_delete_subscriptions_resp_results_array (p_resp_msg, l_nb_reqs) =
    PRE
        p_resp_msg : t_msg_i &
        p_resp_msg : t_msg &
        p_resp_msg = c_msg_out &
        a_msg_out_type = e_msg_subscription_delete_subscriptions_resp &
        l_nb_reqs : NAT &
        l_nb_reqs <= k_n_genericOperationPerReq_max &
        l_nb_reqs = nb_delete_subscriptions &
        nb_delete_subscriptions > 0
    THEN
        bres, delete_subscriptions_resp_allocated
        :(bres : BOOL & delete_subscriptions_resp_allocated = bres)
    END
    ;

    setall_msg_subscription_delete_subscriptions_resp_at_index (p_resp_msg, p_index, p_valid_sub) =
    PRE
        p_resp_msg : t_msg_i &
        p_resp_msg : t_msg &
        p_resp_msg = c_msg_out &
        a_msg_out_type = e_msg_subscription_delete_subscriptions_resp &
        p_index : NAT &
        p_index <= k_n_genericOperationPerReq_max &
        p_index : 1..nb_delete_subscriptions &
        p_valid_sub : BOOL &
        delete_subscriptions_resp_allocated = TRUE &
        nb_delete_subscriptions > 0
    THEN
        IF p_valid_sub = TRUE
        THEN
            delete_subscriptions_resp_StatusCode(p_index) := e_sc_ok
        ELSE
            delete_subscriptions_resp_StatusCode(p_index) := e_sc_bad_subscription_id_invalid
        END
    END

END
