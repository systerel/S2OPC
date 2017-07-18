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

/* Base machine used to write the response elements */
 
MACHINE
    msg_read_response_bs

SEES
    constants, 
    message_out_bs

OPERATIONS
    /* Pb: malloc may fail... */
    /* Pb: the content of *resp_msg is changed, that does not appear here */
    write_read_response_init(p_nb_resps, p_resp_msg) =
    PRE
        p_nb_resps : NAT &
        p_nb_resps <= k_n_read_resp_max &
        p_resp_msg   : t_msg_i &
        p_resp_msg   = c_msg_out &
        c_msg_out : t_msg
    THEN
        skip
    END
    ;

    /* Pb: the content of *resp_msg is changed, that does not appear here */
    write_read_response(resp_msg, rvi, val, sc) =
    PRE
        resp_msg   : t_msg_i &
        resp_msg   = c_msg_out &
        c_msg_out : t_msg &

        rvi : t_ReadValue_i &
        rvi : t_ReadValue &
        val : t_Variant_i &
        sc : t_StatusCode_i &
        sc : t_StatusCode
    THEN
        skip
    END

END
