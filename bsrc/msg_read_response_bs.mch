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
    msg_read_response_bs

SEES
    constants,
    message_out_bs

DEFINITIONS
    d_inv_response == (
        response_StatusCode : t_StatusCode_i &
        data_Value_Value : t_ReadValue +-> t_Variant &
        data_Value_StatusCode : t_ReadValue +-> t_StatusCode

        /*data_Value_SourceTimestamp : t_ReadValue +-> t_Timestamp &
        data_Value_ServerTimestamp : t_ReadValue +-> t_Timestamp*/
    )

ABSTRACT_VARIABLES
    response_StatusCode,
    data_Value_Value,
    data_Value_StatusCode

INVARIANT
    d_inv_response

INITIALISATION
    data_Value_Value      :: {{}} ||
    data_Value_StatusCode :: {{}} ||
    response_StatusCode := c_StatusCode_indet

    /*data_Value_SourceTimestamp, data_Value_ServerTimestamp := {}, {} ||*/

OPERATIONS

    p_isvalid <-- alloc_read_response(p_nb_resps, p_TimestampsToReturn, p_resp_msg) =
    PRE
        p_nb_resps : NAT &
        p_nb_resps <= k_n_read_resp_max &
        p_TimestampsToReturn : t_TimestampsToReturn_i &
        p_TimestampsToReturn : t_TimestampsToReturn &
        p_resp_msg : t_msg_i &
        p_resp_msg = c_msg_out &
        c_msg_out  : t_msg
    THEN
        p_isvalid,
        response_StatusCode
        :(
            p_isvalid : BOOL &  /* malloc result */
            response_StatusCode : t_StatusCode_i &

            (p_isvalid = FALSE
             =>
             response_StatusCode = e_sc_bad_out_of_memory)
         )
    END
    ;

    set_read_response(resp_msg, rvi, val, sc, aid) =
    PRE
        resp_msg    : t_msg_i         &
        resp_msg    = c_msg_out       &
        c_msg_out   : t_msg           &
        rvi         : t_ReadValue_i   &
        rvi         : t_ReadValue     &
        val         : t_Variant_i     &
        (val /= c_Variant_indet
         =>
         val : t_Variant)             &
        sc          : t_StatusCode_i  &
        sc          : t_StatusCode    &
        aid         : t_AttributeId_i &
        (sc = e_sc_ok
         =>
         aid : t_AttributeId)
    THEN
        IF  val /= c_Variant_indet THEN
            data_Value_Value(rvi) := val
        ELSE
            data_Value_Value := {rvi} <<| data_Value_Value
        END ||
        data_Value_StatusCode(rvi) := sc
    END

END
