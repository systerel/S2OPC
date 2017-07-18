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
    msg_read_response

SEES
    /* TODO: msg_read_request still necessary? */
    constants, 
    msg_read_request, 
    message_out_bs

DEFINITIONS
    d_inv_response == (
        response_StatusCode : t_StatusCode_i &
        data_Value_Value : t_ReadValue +-> t_Variant &
        data_Value_StatusCode : t_ReadValue +-> t_StatusCode &
        /*data_Value_SourceTimestamp : t_ReadValue +-> t_Timestamp &
        data_Value_ServerTimestamp : t_ReadValue +-> t_Timestamp*/

        nb_resps : NAT &
        nb_resps <= k_n_read_resp_max &

        response_msg : t_msg_i
    )

ABSTRACT_VARIABLES
    response_StatusCode,
    data_Value_Value,
    data_Value_StatusCode,
    nb_resps,
    response_msg /* Response message */

INVARIANT
    d_inv_response

INITIALISATION
    data_Value_Value      := {} ||
    data_Value_StatusCode := {} ||
    /*data_Value_SourceTimestamp, data_Value_ServerTimestamp := {}, {} ||*/
    response_StatusCode := c_StatusCode_indet ||

    nb_resps,
    response_msg
    :(d_inv_response)

OPERATIONS

    write_read_response_init(p_nb_resps, p_resp_msg) =
    PRE
        p_nb_resps : NAT &
        p_nb_resps <= k_n_read_resp_max &
        p_resp_msg : t_msg_i &
        p_resp_msg = c_msg_out &
        c_msg_out  : t_msg
    THEN
        nb_resps     := p_nb_resps ||
        response_msg := p_resp_msg
    END
    ;

    write_read_response(resp_msg, rvi, val, sc) =
    PRE
        resp_msg  : t_msg_i   &
        resp_msg  = c_msg_out &
        c_msg_out : t_msg     &

        rvi : t_ReadValue_i   &
        rvi : t_ReadValue     &
        val : t_Variant_i     &
        sc  : t_StatusCode_i  &
        sc  : t_StatusCode
    THEN
        data_Value_Value     (rvi) := val ||
        data_Value_StatusCode(rvi) := sc
    END
    ;

    write_response_error(sc, resp_msg) =
    PRE
        sc : t_StatusCode_i &
        sc : t_StatusCode &
        resp_msg   : t_msg_i &
        resp_msg   = c_msg_out &
        c_msg_out : t_msg
    THEN
        nb_resps := 0 ||
        response_StatusCode := sc
    END

END
