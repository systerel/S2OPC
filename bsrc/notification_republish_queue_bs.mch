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
    notification_republish_queue_bs

SEES
    constants

ABSTRACT_VARIABLES
    s_republishQueue,
    a_republish_notification,
    a_notification_seq_num

INVARIANT
    s_republishQueue <: t_notifRepublishQueue &
    a_republish_notification : t_notif_msg +-> t_notifRepublishQueue &
    a_notification_seq_num : t_notif_msg --> t_sub_seq_num

INITIALISATION
    s_republishQueue :(s_republishQueue = {}) ||
    a_republish_notification :(a_republish_notification = {}) ||
    a_notification_seq_num :(a_notification_seq_num = {})

OPERATIONS

    bres, queue <-- allocate_new_republish_queue =
    BEGIN
        IF s_republishQueue = t_notifRepublishQueue
        THEN
            bres := FALSE ||
            queue := c_notifRepublishQueue_indet
        ELSE
            ANY l_queue WHERE
                l_queue : t_notifRepublishQueue_i &
                l_queue : t_notifRepublishQueue &
                l_queue /: s_republishQueue
            THEN
                s_republishQueue := s_republishQueue \/ {l_queue} ||
                queue := l_queue ||
                bres := TRUE
            END
        END
    END
    ;

    clear_republish_queue (p_queue) =
    PRE
        p_queue : t_notifRepublishQueue_i &
        p_queue : s_republishQueue
    THEN
        a_republish_notification :(a_republish_notification : t_notif_msg +-> t_notifRepublishQueue &
                                   a_republish_notification |> {p_queue} = {} &
                                   a_republish_notification |>> {p_queue} = a_republish_notification$0 |>> {p_queue}) ||
        a_notification_seq_num :(a_notification_seq_num : t_notif_msg --> t_sub_seq_num &
                                  a_republish_notification~[{p_queue}] <| a_notification_seq_num = {} &
                                  a_republish_notification~[{p_queue}] <<| a_notification_seq_num =
                                   a_republish_notification~[{p_queue}] <<| a_notification_seq_num$0)
    END
    ;

    clear_and_deallocate_republish_queue (p_queue) =
    PRE
        p_queue : t_notifRepublishQueue_i &
        p_queue : s_republishQueue
    THEN
        a_republish_notification :(a_republish_notification : t_notif_msg +-> t_notifRepublishQueue &
                                   a_republish_notification |> {p_queue} = {} &
                                   a_republish_notification |>> {p_queue} = a_republish_notification$0 |>> {p_queue}) ||
        a_notification_seq_num :(a_notification_seq_num : t_notif_msg --> t_sub_seq_num &
                                  a_republish_notification~[{p_queue}] <| a_notification_seq_num = {} &
                                  a_republish_notification~[{p_queue}] <<| a_notification_seq_num =
                                    a_republish_notification~[{p_queue}] <<| a_notification_seq_num$0) ||
        s_republishQueue := s_republishQueue - {p_queue}
    END
    ;

    nb_notifs <-- get_nb_republish_notifs (p_queue) =
    PRE
        p_queue : t_notifRepublishQueue_i &
        p_queue : s_republishQueue
    THEN
        nb_notifs :(nb_notifs = card(a_republish_notification~[{p_queue}]) & nb_notifs <= k_n_republishNotifPerSub_max)
    END
    ;

    discard_oldest_republish_notif (p_queue) =
    PRE
        p_queue : t_notifRepublishQueue_i &
        p_queue : s_republishQueue &
        card(a_republish_notification~[{p_queue}]) = k_n_republishNotifPerSub_max
    THEN
        ANY l_old_notif_msg, l_seq_num WHERE
            l_old_notif_msg : t_notif_msg_i &
            (l_old_notif_msg |-> p_queue) : a_republish_notification &
            l_seq_num : t_sub_seq_num_i &
            (l_old_notif_msg |-> l_seq_num) : a_notification_seq_num
        THEN
            a_republish_notification := a_republish_notification - {l_old_notif_msg |-> p_queue} ||
            a_notification_seq_num := a_notification_seq_num - {l_old_notif_msg |-> l_seq_num}
        END
    END
    ;

    bres <-- add_republish_notif_to_queue (p_queue, p_seq_num, p_notif_msg) =
    PRE
        p_queue : t_notifRepublishQueue_i &
        p_queue : s_republishQueue &
        p_seq_num : t_sub_seq_num_i &
        p_seq_num : t_sub_seq_num &
        p_notif_msg : t_notif_msg_i &
        p_notif_msg : t_notif_msg &
        card(a_republish_notification~[{p_queue}]) < k_n_republishNotifPerSub_max
    THEN
        CHOICE
            bres := FALSE
        OR
            bres := TRUE ||
            a_republish_notification := a_republish_notification \/ {p_notif_msg |-> p_queue} ||
            a_notification_seq_num := a_notification_seq_num \/ {p_notif_msg |-> p_seq_num}
        END
    END
    ;

    bres, p_notif_msg <-- get_republish_notif_from_queue (p_queue, p_seq_num) =
    PRE
        p_queue : t_notifRepublishQueue_i &
        p_queue : s_republishQueue &
        p_seq_num : t_sub_seq_num_i &
        p_seq_num : t_sub_seq_num &
        p_notif_msg : t_notif_msg_i &
        p_notif_msg : t_notif_msg
    THEN
        IF p_seq_num : a_notification_seq_num[a_republish_notification~[{p_queue}]]
        THEN
            bres := TRUE ||
            p_notif_msg
            :(p_notif_msg : t_notif_msg_i & p_notif_msg : t_notif_msg &
              p_notif_msg : a_republish_notification~[{p_queue}] &
              a_notification_seq_num(p_notif_msg) = p_seq_num)
        ELSE
            p_notif_msg := c_notif_msg_indet ||
            bres := FALSE
        END
    END
    ;

    bres <-- remove_republish_notif_from_queue (p_queue, p_seq_num) =
    PRE
        p_queue : t_notifRepublishQueue_i &
        p_queue : s_republishQueue &
        p_seq_num : t_sub_seq_num_i &
        p_seq_num : t_sub_seq_num
    THEN
        IF p_seq_num : a_notification_seq_num[a_republish_notification~[{p_queue}]]
        THEN
            bres := TRUE ||
            ANY l_notif_msg WHERE
                l_notif_msg : t_notif_msg_i &
                l_notif_msg : a_republish_notification~[{p_queue}] &
                a_notification_seq_num(l_notif_msg) = p_seq_num
            THEN
                a_republish_notification := a_republish_notification - {l_notif_msg |-> p_queue} ||
                a_notification_seq_num := a_notification_seq_num - {l_notif_msg |-> p_seq_num}
            END
        ELSE
            bres := FALSE
        END
    END

END
