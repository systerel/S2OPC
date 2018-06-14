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
    
    bres <-- add_republish_notif_to_queue (p_queue, p_seq_num, p_notif_msg) =
    PRE
        p_queue : t_notifRepublishQueue_i &
        p_queue : s_republishQueue &
        p_seq_num : t_sub_seq_num_i &
        p_seq_num : t_sub_seq_num &
        p_notif_msg : t_notif_msg_i &
        p_notif_msg : t_notif_msg
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
    
    bres, p_notif_msg <-- remove_republish_notif_from_queue (p_queue, p_seq_num) =
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
                p_notif_msg := l_notif_msg ||
                a_republish_notification := a_republish_notification - {l_notif_msg |-> p_queue} ||
                a_notification_seq_num := a_notification_seq_num - {l_notif_msg |-> p_seq_num}
            END
        ELSE
            p_notif_msg := c_notif_msg_indet ||
            bres := FALSE
        END
    END

END
