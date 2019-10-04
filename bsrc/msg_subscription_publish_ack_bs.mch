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
    msg_subscription_publish_ack_bs

SEES
    constants,
    constants_statuscodes_bs,
    message_in_bs,
    message_out_bs,
    request_handle_bs

ABSTRACT_VARIABLES
    nb_subscription_acks,
    subscription_ack_req_subscription,
    subscription_ack_req_ackSeqNum,
    subscription_ack_resp_allocated,
    subscription_ack_resp_StatusCode,
    subscription_available_seq_nums_resp_nb,
    subscription_available_seq_nums_resp_SeqNum

INVARIANT
    nb_subscription_acks : NAT &
    subscription_ack_req_subscription : 1..nb_subscription_acks --> t_subscription_i &
    subscription_ack_req_ackSeqNum : 1..nb_subscription_acks --> t_sub_seq_num_i &
    subscription_ack_resp_allocated : BOOL &
    subscription_ack_resp_StatusCode : 1..k_n_genericOperationPerReq_max +-> t_StatusCode &
    subscription_available_seq_nums_resp_nb : NAT &
    subscription_available_seq_nums_resp_SeqNum : NAT +-> t_sub_seq_num

INITIALISATION
    nb_subscription_acks := 0 ||
    subscription_ack_req_ackSeqNum :(subscription_ack_req_ackSeqNum = {}) ||
    subscription_ack_req_subscription :(subscription_ack_req_subscription = {}) ||
    subscription_ack_resp_allocated := FALSE ||
    subscription_ack_resp_StatusCode :(subscription_ack_resp_StatusCode = {}) ||
    subscription_available_seq_nums_resp_nb := 0 ||
    subscription_available_seq_nums_resp_SeqNum :(subscription_available_seq_nums_resp_SeqNum = {})

OPERATIONS

    req_expiration_time <-- get_msg_header_expiration_time (p_req_header) =
    PRE
        p_req_header : t_msg_header_i &
        p_req_header = c_msg_in_header &
        c_msg_in_header : t_msg_header &
        a_msg_in_type = e_msg_subscription_publish_req
    THEN
        req_expiration_time :(req_expiration_time : t_timeref_i & req_expiration_time : t_timeref)
    END
    ;

    p_nb_acks <-- get_msg_publish_request_ack_params (p_req_msg) =
    PRE
        p_req_msg : t_msg_i &
        p_req_msg : t_msg &
        p_req_msg = c_msg_in &
        a_msg_in_type = e_msg_subscription_publish_req
    THEN
        p_nb_acks, nb_subscription_acks, subscription_ack_req_ackSeqNum,
        subscription_ack_req_subscription
        :(p_nb_acks : NAT & p_nb_acks = nb_subscription_acks &
          subscription_ack_req_ackSeqNum : 1..p_nb_acks --> t_sub_seq_num_i &
          subscription_ack_req_subscription : 1..p_nb_acks --> t_subscription_i)
    END
    ;

    p_sub, p_seq_num <-- getall_msg_publish_request_ack (p_req_msg, p_index) =
    PRE
        p_req_msg : t_msg_i &
        p_req_msg : t_msg &
        p_req_msg = c_msg_in &
        a_msg_in_type = e_msg_subscription_publish_req &
        p_index : NAT &
        p_index : 1..nb_subscription_acks &
        p_index <= k_n_genericOperationPerReq_max
    THEN
        p_sub := subscription_ack_req_subscription (p_index) ||
        p_seq_num := subscription_ack_req_ackSeqNum (p_index)
    END
    ;

    bres <-- allocate_subscription_ack_results (p_resp_msg, p_nb_acks) =
    PRE
        p_resp_msg : t_msg_i &
        p_resp_msg : t_msg &
        p_resp_msg = c_msg_out &
        a_msg_out_type = e_msg_subscription_publish_resp &
        p_nb_acks : NAT &
        p_nb_acks : 1..nb_subscription_acks &
        p_nb_acks <= k_n_genericOperationPerReq_max
    THEN
        bres, subscription_ack_resp_allocated
        :(bres : BOOL & bres = subscription_ack_resp_allocated)
    END
    ;

    setall_msg_publish_resp_ack_result (p_resp_msg, p_index, p_sc) =
    PRE
        p_resp_msg : t_msg_i &
        p_resp_msg : t_msg &
        p_resp_msg = c_msg_out &
        a_msg_out_type = e_msg_subscription_publish_resp &
        p_index : NAT &
        p_index : 1..nb_subscription_acks &
        p_index <= k_n_genericOperationPerReq_max  &
        p_sc : t_StatusCode_i &
        p_sc : {e_sc_ok, e_sc_bad_subscription_id_invalid, e_sc_bad_sequence_number_unknown} &
        subscription_ack_resp_allocated = TRUE
    THEN
        subscription_ack_resp_StatusCode (p_index) := p_sc
    END
    ;

    bres <-- allocate_subscription_available_seq_nums (p_resp_msg, p_nb_seq_num) =
    PRE
        p_resp_msg : t_msg_i &
        p_resp_msg : t_msg &
        p_resp_msg = c_msg_out &
        a_msg_out_type = e_msg_subscription_publish_resp &
        p_nb_seq_num : NAT &
        p_nb_seq_num > 0
    THEN
        bres, subscription_available_seq_nums_resp_nb
        :(bres : BOOL & subscription_available_seq_nums_resp_nb : NAT &
            (subscription_available_seq_nums_resp_nb > 0 => bres = TRUE) &
            (subscription_available_seq_nums_resp_nb = 0 => bres = FALSE)
        )
    END
    ;

    setall_msg_publish_resp_available_seq_num (p_resp_msg, p_index, p_seq_num) =
    PRE
        p_resp_msg : t_msg_i &
        p_resp_msg : t_msg &
        p_resp_msg = c_msg_out &
        a_msg_out_type = e_msg_subscription_publish_resp &
        p_index : NAT &
        p_index : 1..subscription_available_seq_nums_resp_nb &
        subscription_available_seq_nums_resp_nb > 0 &
        p_seq_num : t_sub_seq_num_i & p_seq_num : t_sub_seq_num
    THEN
        subscription_available_seq_nums_resp_SeqNum (p_index) := p_seq_num
    END
    ;

    l_sub, l_seq_num <-- getall_msg_republish_request (p_req_msg) =
    PRE
        p_req_msg : t_msg_i &
        p_req_msg : t_msg &
        p_req_msg = c_msg_in &
        a_msg_in_type = e_msg_subscription_republish_req
    THEN
        l_sub, l_seq_num :(l_sub : t_subscription_i & l_seq_num : t_sub_seq_num_i)
    END
    ;

    sc <-- setall_msg_republish_response (p_resp_msg, p_notifMsg) =
    PRE
        p_resp_msg : t_msg_i &
        p_resp_msg : t_msg &
        p_resp_msg = c_msg_out &
        a_msg_out_type = e_msg_subscription_republish_resp &
        p_notifMsg : t_notif_msg_i &
        p_notifMsg : t_notif_msg
    THEN
        CHOICE
            sc := e_sc_ok
        OR
            sc := e_sc_bad_out_of_memory
        END
    END

END
