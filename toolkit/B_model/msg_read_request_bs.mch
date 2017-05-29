/*
 *  Copyright (C) 2017 Systerel and others.
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
    msg_read_request_bs

SEES
    constants, 
    message_in_bs

ABSTRACT_VARIABLES
    nb_reqs_curr

INVARIANT
    nb_reqs_curr : NAT &
    nb_reqs_curr <= k_n_read_resp_max

INITIALISATION
    nb_reqs_curr :: 0..k_n_read_resp_max

OPERATIONS
    nb <-- read_req_nb_ReadValue(msg) =
    PRE
        msg  : t_msg_i &
        msg  = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type = e_msg_session_read_req
    THEN
        nb, nb_reqs_curr :( nb : 0..k_n_read_resp_max & nb_reqs_curr = nb )
    END
    ;

    /* Returns the NodeId (pointer or hash_id, or...)
     * Does not check if there is a node in the AddressSpace corresponding to this nid */
    nid <-- read_req_ReadValue_NodeId(msg, rvi) =
    PRE
        msg  : t_msg_i &
        msg  = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type = e_msg_session_read_req &
        rvi : t_ReadValue_i &
        rvi : t_ReadValue &
        rvi : cast_t_ReadValue[0..nb_reqs_curr]
    THEN
        nid :: t_NodeId_i
    END
    ;

    /* Returns the AttributeId (c_*_indet if not valid) */
    /* ... Could have merged them */
    aid <-- read_req_ReadValue_AttributeId(msg, rvi) =
    PRE
        msg  : t_msg_i &
        msg  = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type = e_msg_session_read_req &
        rvi : t_ReadValue_i &
        rvi : t_ReadValue &
        rvi : cast_t_ReadValue[0..nb_reqs_curr]
    THEN
        aid :: t_AttributeId_i
    END

END
