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

        response_ok : BOOL &

        nb_resps : NAT &
        nb_resps <= k_n_read_resp_max &

        resps_to_iterate <: t_ReadValue &
        resps_iterated <: t_ReadValue &
        resps_to_iterate /\ resps_iterated = {} &

        msg : t_msg_i
    )
    ;
    /* Not used yet */
    d_prop_response == (
        card(resps_iterated \/ resps_to_iterate) = nb_resps &

        response_StatusCode : t_StatusCode &
        data_Value_Value : resps_iterated +-> t_Variant &
        data_Value_StatusCode : resps_iterated --> t_StatusCode &
        /*data_Value_SourceTimestamp : xxx +-> t_Timestamp &
        data_Value_ServerTimestamp : xxx --> t_Timestamp*/
        msg : t_msg
    )

ABSTRACT_VARIABLES
    response_StatusCode, data_Value_Value, data_Value_StatusCode,
    /* data_Value_SourceTimestamp, data_Value_ServerTimestamp*/
    response_ok,
    nb_resps,
    resps_to_iterate,
    resps_iterated,
    msg /* Response message */

INVARIANT
    d_inv_response

INITIALISATION
    data_Value_Value, data_Value_StatusCode := {}, {} ||
    /*data_Value_SourceTimestamp, data_Value_ServerTimestamp := {}, {} ||*/
    response_StatusCode := c_StatusCode_indet ||

    nb_resps, response_ok, resps_to_iterate, resps_iterated, msg :(d_inv_response)

OPERATIONS
    continue <-- init_iterwrite_DataValue(a_nb_resps, resp_msg) =
    PRE
        a_nb_resps : NAT &
        a_nb_resps <= k_n_read_resp_max &
        resp_msg   : t_msg_i &
        resp_msg   = c_msg_out &
        c_msg_out : t_msg
    THEN
        nb_resps := a_nb_resps ||
        resps_to_iterate := cast_t_ReadValue[1..a_nb_resps] ||
        resps_iterated := {} ||

        data_Value_Value := {} ||
        data_Value_StatusCode := {} ||

        response_StatusCode :: t_StatusCode ||
        continue := bool(0 < a_nb_resps) ||

        msg := resp_msg
    END
    ;

    continue <-- continue_iterwrite_DataValue(rvi, val, sc) =
    PRE
        rvi : t_ReadValue_i &
        rvi : t_ReadValue &
        rvi : resps_to_iterate &
        val : t_Variant_i &
        sc : t_StatusCode_i &
        sc : t_StatusCode
    THEN
        IF val /= c_Variant_indet THEN
            data_Value_Value(rvi) := val
        END
        ||
        data_Value_StatusCode(rvi) := sc ||
        resps_iterated := resps_iterated \/ {rvi} ||
        resps_to_iterate := resps_to_iterate - {rvi} ||
        continue := bool(resps_to_iterate /= {rvi}) /* Pb: malloc may fail... */
    END
    ;

    write_response_error(sc, resp_msg) =
    PRE
        sc : t_StatusCode_i &
        sc : t_StatusCode &
        resp_msg   : t_msg_i &
        resp_msg   = c_msg_out &
        c_msg_out : t_msg
        /*data_Value_Value = {} &
        data_Value_StatusCode = {} *//*&
        data_Value_SourceTimestamp = {} &
        data_Value_ServerTimestamp = {}*/
    THEN
        nb_resps := 0 ||
        response_StatusCode := sc
    END

END
