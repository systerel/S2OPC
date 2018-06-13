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
    msg_subscription_delete_bs

SEES
    constants,
    message_in_bs,
    message_out_bs
    
OPERATIONS
    
    l_nb_subs, p_first_sub_in_request <-- get_msg_delete_subscriptions_req_first_subscription (p_req_msg) =
    PRE
        p_req_msg : t_msg_i &
        p_req_msg : t_msg &
        p_req_msg = c_msg_in &
        a_msg_in_type = e_msg_subscription_delete_subscriptions_req
    THEN
        p_first_sub_in_request :(p_first_sub_in_request : t_subscription_i) ||
        l_nb_subs :: NAT
    END
    ;
    
    bres <-- allocate_msg_delete_subscriptions_resp_results_array (p_resp_msg, l_nb_subs) =
    PRE
        p_resp_msg : t_msg_i &
        p_resp_msg : t_msg &
        p_resp_msg = c_msg_out &
        a_msg_out_type = e_msg_subscription_delete_subscriptions_resp &
        l_nb_subs : NAT &
        l_nb_subs > 0
    THEN
        bres :: BOOL
    END
    ;
    
    set_msg_delete_subscriptions_resp_first_subscription (p_resp_msg, p_valid_first_sub) = 
    PRE
        p_resp_msg : t_msg_i &
        p_resp_msg : t_msg &
        p_resp_msg = c_msg_out &
        a_msg_out_type = e_msg_subscription_delete_subscriptions_resp &
        p_valid_first_sub : BOOL
    THEN
        skip
    END

END
