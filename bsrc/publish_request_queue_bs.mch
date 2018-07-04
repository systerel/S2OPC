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
    publish_request_queue_bs

SEES
    constants,
    message_out_bs,
    request_handle_bs

ABSTRACT_VARIABLES

    s_publishReqQueue,
    a_publish_responses,
    a_publish_expiration_time,
    s_expired_time_ref,
    validPublishReqQueued

INVARIANT
    s_publishReqQueue <: t_publishReqQueue &
    a_publish_responses : t_msg +-> t_publishReqQueue &
    a_publish_expiration_time : t_msg +-> t_timeref &
    dom(a_publish_expiration_time) = dom(a_publish_responses) &
    s_expired_time_ref <: t_timeref & /* Represents the set of expired time references (for model only) */
    validPublishReqQueued : BOOL

INITIALISATION
    s_publishReqQueue :(s_publishReqQueue = {}) ||
    a_publish_responses :(a_publish_responses = {}) ||
    a_publish_expiration_time :(a_publish_expiration_time = {}) ||
    s_expired_time_ref :(s_expired_time_ref <: t_timeref) ||
    validPublishReqQueued := FALSE

OPERATIONS

    bres, queue <-- allocate_new_publish_queue =
    BEGIN
        IF s_publishReqQueue = t_publishReqQueue
        THEN
            bres := FALSE ||
            queue := c_publishReqQueue_indet
        ELSE
            ANY l_queue WHERE
                l_queue : t_publishReqQueue_i &
                l_queue : t_publishReqQueue &
                l_queue /: s_publishReqQueue
            THEN
                s_publishReqQueue := s_publishReqQueue \/ {l_queue} ||
                queue := l_queue ||
                bres := TRUE
            END
        END
    END
    ;

    clear_publish_queue (p_queue) =
    PRE
        p_queue : t_publishReqQueue_i &
        p_queue : s_publishReqQueue
    THEN
        a_publish_responses :(a_publish_responses : t_msg +-> t_publishReqQueue &
                              a_publish_responses |> {p_queue} = {} &
                              a_publish_responses |>> {p_queue} = a_publish_responses$0 |>> {p_queue})
    END
    ;

    clear_and_deallocate_publish_queue (p_queue) =
    PRE
        p_queue : t_publishReqQueue_i &
        p_queue : s_publishReqQueue
    THEN
        a_publish_responses :(a_publish_responses : t_msg +-> t_publishReqQueue &
                              a_publish_responses |> {p_queue} = {} &
                              a_publish_responses |>> {p_queue} = a_publish_responses$0 |>> {p_queue}) ||
        s_publishReqQueue := s_publishReqQueue - {p_queue}
    END
    ;

    nb_pub_reqs <-- get_nb_publish_requests (p_queue) =
    PRE
        p_queue : t_publishReqQueue_i &
        p_queue : s_publishReqQueue
    THEN
        nb_pub_reqs
        :(nb_pub_reqs = card(a_publish_responses~[{p_queue}]) & nb_pub_reqs <= k_n_publishRequestPerSub_max)
    END
    ;

    old_session, old_resp_msg, old_req_handle, old_req_ctx
      <-- discard_oldest_publish_request (p_queue) =
    PRE
        p_queue : t_publishReqQueue_i &
        p_queue : s_publishReqQueue &
        card(a_publish_responses~[{p_queue}]) = k_n_publishRequestPerSub_max
    THEN
        ANY l_old_resp_msg, l_req_exp_time WHERE
            l_old_resp_msg : t_msg_i &
            (l_old_resp_msg |-> p_queue) : a_publish_responses &
            l_req_exp_time : t_timeref_i &
            (l_old_resp_msg |-> l_req_exp_time) : a_publish_expiration_time
        THEN
            a_publish_responses := a_publish_responses - {l_old_resp_msg |-> p_queue} ||
            a_publish_expiration_time := a_publish_expiration_time - {l_old_resp_msg |-> l_req_exp_time} ||
            old_resp_msg := l_old_resp_msg ||
            old_session, old_req_handle, old_req_ctx
            :(old_session : t_session_i &
                old_session : t_session &
                old_req_handle : t_server_request_handle_i &
                old_req_ctx : t_request_context_i &
                old_req_ctx : t_request_context
            )
        END
    END
    ;

    bres <-- append_publish_request_to_queue (p_queue, p_session, p_req_exp_time, p_req_handle, p_req_ctx, p_resp_msg) =
    PRE
        p_queue : t_publishReqQueue_i &
        p_queue : s_publishReqQueue &
        p_session : t_session_i &
        p_session : t_session &
        p_req_exp_time : t_timeref_i &
        p_req_exp_time : t_timeref &
        p_req_handle : t_server_request_handle_i &
        p_req_ctx : t_request_context_i &
        p_req_ctx : t_request_context &
        p_resp_msg : t_msg_i &
        p_resp_msg : t_msg &
        p_resp_msg = c_msg_out &
        a_msg_out_type = e_msg_subscription_publish_resp &
        card(a_publish_responses~[{p_queue}]) < k_n_publishRequestPerSub_max
    THEN
        CHOICE
            bres := FALSE
        OR
            bres := TRUE ||
            a_publish_responses := a_publish_responses \/ {p_resp_msg |-> p_queue} ||
            a_publish_expiration_time := a_publish_expiration_time \/ {p_resp_msg |-> p_req_exp_time}
        END
    END
    ;

    continue <-- init_iter_publish_request (p_queue) =
    PRE
        p_queue : t_publishReqQueue_i &
        p_queue : s_publishReqQueue
    THEN
        continue := bool(a_publish_responses~[{p_queue}] /= {})
    END
    ;

    p_continue, p_session, p_req_exp_time, p_req_handle, p_req_ctx, p_resp_msg
      <-- continue_pop_head_iter_publish_request (p_queue) =
    PRE
        p_queue : t_publishReqQueue_i &
        p_queue : s_publishReqQueue &
        a_publish_responses~[{p_queue}] /= {}
    THEN
        ANY l_resp_msg WHERE
            l_resp_msg : t_msg_i &
            l_resp_msg : t_msg &
            l_resp_msg |-> p_queue : a_publish_responses
        THEN
            p_continue := bool(a_publish_responses~[{p_queue}] /= {l_resp_msg}) ||
            a_publish_responses := a_publish_responses - {l_resp_msg |-> p_queue} ||
            p_resp_msg := l_resp_msg ||
            a_publish_expiration_time, p_session, p_req_exp_time, p_req_handle, p_req_ctx
            :(  p_session : t_session_i &
                p_session : t_session &
                p_req_exp_time : t_timeref_i &
                p_req_exp_time : t_timeref &
                p_req_handle : t_server_request_handle_i &
                p_req_ctx : t_request_context_i &
                p_req_ctx : t_request_context &
                a_publish_expiration_time = a_publish_expiration_time$0 - {l_resp_msg |-> p_req_exp_time}
            )
        END
    END
    ;

    bres <-- is_request_expired (p_req_exp_time) =
    PRE
        p_req_exp_time : t_timeref_i &
        p_req_exp_time : t_timeref
    THEN
        s_expired_time_ref,
        bres
        :(s_expired_time_ref <: t_timeref & bres = bool(p_req_exp_time /: s_expired_time_ref))
        /* Update s_expired_time_ref and evaluation of p_req_exp_time expiration */
    END
    ;

    /* Used to re-enqueue the first non expired publish request found in queue */
    bres <-- prepend_publish_request_to_queue (p_queue, p_session, p_req_exp_time, p_req_handle, p_req_ctx, p_resp_msg) =
    PRE
        p_queue : t_publishReqQueue_i &
        p_queue : s_publishReqQueue &
        p_session : t_session_i &
        p_session : t_session &
        p_req_exp_time : t_timeref_i &
        p_req_exp_time : t_timeref &
        p_req_exp_time /: s_expired_time_ref & /* Validated as non expired time reference only */
        p_req_handle : t_server_request_handle_i &
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
            a_publish_responses := a_publish_responses \/ {p_resp_msg |-> p_queue} ||
            a_publish_expiration_time := a_publish_expiration_time \/ {p_resp_msg |-> p_req_exp_time} ||
            validPublishReqQueued := TRUE
        END
    END
    ;

    /* Used to dequeue a non expired publish request:  */
    p_session, p_req_exp_time, p_req_handle, p_req_ctx, p_resp_msg
      <-- pop_valid_publish_request_queue (p_queue) =
    PRE
        p_queue : t_publishReqQueue_i &
        p_queue : s_publishReqQueue &
        a_publish_responses~[{p_queue}] /= {} &
        validPublishReqQueued = TRUE
    THEN
        ANY l_resp_msg WHERE
            l_resp_msg : t_msg_i &
            l_resp_msg : t_msg &
            l_resp_msg |-> p_queue : a_publish_responses
        THEN
            a_publish_responses := a_publish_responses - {l_resp_msg |-> p_queue} ||
            validPublishReqQueued := FALSE ||
            p_resp_msg := l_resp_msg ||
            a_publish_expiration_time, p_session, p_req_exp_time, p_req_handle, p_req_ctx
            :(
                p_session : t_session_i &
                p_session : t_session &
                p_req_exp_time : t_timeref_i &
                p_req_exp_time : t_timeref &
                p_req_exp_time /: s_expired_time_ref & /* Validated as non expired time reference only */
                p_req_handle : t_server_request_handle_i &
                p_req_ctx : t_request_context_i &
                p_req_ctx : t_request_context &
                a_publish_expiration_time = a_publish_expiration_time$0 - {l_resp_msg |-> p_req_exp_time}
            )
        END
    END

END
