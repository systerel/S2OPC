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
    publish_request_queue_bs

SEES
    constants,
    message_out_bs
    
ABSTRACT_VARIABLES

    a_publish_responses
    
INVARIANT
    a_publish_responses : t_msg +->> t_publishReqQueue
    
INITIALISATION
    a_publish_responses :(a_publish_responses : t_msg +->> t_publishReqQueue)

OPERATIONS
    
    bres <-- add_publish_request_to_queue (p_queue, p_session, p_req_exp_time, p_req_ctx, p_resp_msg) =
    PRE
        p_queue : t_publishReqQueue_i &
        p_queue : t_publishReqQueue &
        p_session : t_session_i &
        p_session : t_session &
        p_req_exp_time : t_timeref_i &
        p_req_exp_time : t_timeref &
        p_req_ctx : t_request_context_i &
        p_req_ctx : t_request_context &
        p_resp_msg : t_msg_i &
        p_resp_msg : t_msg &
        p_resp_msg = c_msg_out &
        a_msg_out_type = e_msg_subscription_publish_resp
    THEN
        CHOICE
            bres := FALSE
        OR 
            bres := TRUE ||
            a_publish_responses
            :(
                a_publish_responses : t_msg +-> t_publishReqQueue &
                a_publish_responses |>> {p_queue} = a_publish_responses$0 |>> {p_queue} &
                {p_resp_msg} <<| a_publish_responses = {p_resp_msg} <<| a_publish_responses$0 &
                p_resp_msg : dom(a_publish_responses |> {p_queue})
            )
        END
    END
    ;
    
    
    continue, iterator <-- init_iter_publish_request (p_queue) =
    PRE
        p_queue : t_publishReqQueue_i &
        p_queue : t_publishReqQueue
    THEN
        continue,
        iterator
        :(continue : BOOL &
          iterator : t_publishReqQueueIterator_i &
          (continue = TRUE => 
            iterator : t_publishReqQueueIterator)
        )
    END
    ;

    p_continue, p_session, p_req_exp_time, p_req_ctx, p_resp_msg
      <-- continue_pop_iter_monitor_item_notification (p_iterator, p_queue) =
    PRE
        p_iterator : t_notificationQueueIterator_i &
        p_iterator : t_notificationQueueIterator &
        p_queue : t_publishReqQueue_i &
        p_queue : t_publishReqQueue
    THEN
        CHOICE
            p_continue := FALSE ||
            p_session := c_session_indet ||
            p_req_exp_time := c_timeref_indet ||
            p_req_ctx := c_request_context_indet ||
            p_resp_msg := c_msg_indet
        OR 
            p_continue := TRUE ||
            a_publish_responses,
            p_session, p_req_exp_time, p_req_ctx, p_resp_msg
            :(
                a_publish_responses : t_msg +-> t_publishReqQueue &
                a_publish_responses |>> {p_queue} = a_publish_responses$0 |>> {p_queue} &
                a_publish_responses = {p_resp_msg} <<| a_publish_responses$0 &
                a_publish_responses \/ {p_resp_msg |-> p_queue} = a_publish_responses$0 &
                p_session : t_session_i &
                p_session : t_session &
                p_req_exp_time : t_timeref_i &
                p_req_exp_time : t_timeref &
                p_req_ctx : t_request_context_i &
                p_req_ctx : t_request_context
            )
        END
    END
    
END
