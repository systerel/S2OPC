/*
 *  Copyright (C) 2018 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

MACHINE
    msg_subscription_publish_ack_bs
    
SEES
    constants,
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
        p_nb_acks, nb_subscription_acks, subscription_ack_req_ackSeqNum 
        :(p_nb_acks : NAT & p_nb_acks = nb_subscription_acks &
          subscription_ack_req_ackSeqNum : 1..p_nb_acks --> t_sub_seq_num_i)
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
    
    setall_msg_republish_response (p_resp_msg, l_notifMsg) =
    PRE
        p_resp_msg : t_msg_i &
        p_resp_msg : t_msg &
        p_resp_msg = c_msg_out &
        a_msg_out_type = e_msg_subscription_republish_resp &
        l_notifMsg : t_notif_msg_i & 
        l_notifMsg : t_notif_msg
    THEN
        skip
    END

END
