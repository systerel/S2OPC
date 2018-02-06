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
    service_read

SEES
    constants,
    address_space,
    message_in_bs,
    message_out_bs

DEFINITIONS
    "definitions.def";
    d_inv_read == (
        nb_ReadValue          : NAT                           &
        nb_ReadValue         <= k_n_read_resp_max             &
        ReadValue_NodeId      : t_ReadValue +-> t_NodeId      &
        ReadValue_AttributeId : t_ReadValue +-> t_AttributeId
    )
    ;
    d_inv_response == (
        data_Value_Value           : t_ReadValue +-> t_Variant &
        data_Value_StatusCode      : t_ReadValue +-> t_StatusCode /*&
        data_Value_SourceTimestamp : set_read_elements +-> t_Timestamp &
        data_Value_ServerTimestamp : set_read_elements --> t_Timestamp*/
    )

ABSTRACT_VARIABLES
    nb_ReadValue,
    ReadValue_NodeId,
    ReadValue_AttributeId,
    data_Value_Value,
    data_Value_StatusCode

INVARIANT
    d_inv_read &
    d_inv_response

INITIALISATION
    nb_ReadValue          := 0  ||
    ReadValue_NodeId      := {} ||
    ReadValue_AttributeId := {} ||
    data_Value_Value      := {} ||
    data_Value_StatusCode := {}

OPERATIONS

    p_read_ok, p_nb_ReadValue, p_tsToReturn, p_statusCode <-- check_ReadRequest(p_msg) =
    PRE
        p_msg    : t_msg_i  &
        p_msg    = c_msg_in &
        c_msg_in : t_msg    &
        a_msg_in_type = e_msg_attribute_read_req
    THEN
        p_read_ok,
        p_nb_ReadValue,
        p_tsToReturn,
        p_statusCode,
        nb_ReadValue,
        ReadValue_NodeId,
        ReadValue_AttributeId
        :(  p_read_ok             : BOOL                         &
            p_nb_ReadValue        : INT                          &
            p_tsToReturn          : t_TimestampsToReturn_i       &
            p_statusCode          : {e_sc_ok, e_sc_bad_nothing_to_do, e_sc_bad_too_many_ops, 
                                     e_sc_bad_max_age_invalid, e_sc_bad_timestamps_to_return_invalid} &
            data_Value_Value      : t_ReadValue +-> t_Variant    &
            data_Value_StatusCode : t_ReadValue +-> t_StatusCode
        )
    END
    ;

    p_isvalid <-- alloc_read_response(p_nb_resps, p_TimestampsToReturn, p_resp_msg) =
    PRE
        p_nb_resps : NAT                              &
        p_nb_resps <= k_n_read_resp_max               &
        p_TimestampsToReturn : t_TimestampsToReturn_i &
        p_TimestampsToReturn : t_TimestampsToReturn   &
        p_resp_msg : t_msg_i                          &
        p_resp_msg = c_msg_out                        &
        c_msg_out  : t_msg
    THEN
        p_isvalid :: BOOL  /* malloc result */
    END
    ;

    fill_read_response(req_msg, resp_msg) =
    PRE
        req_msg   : t_msg_i &
        req_msg   = c_msg_in &
        c_msg_in  : t_msg &
        resp_msg  : t_msg_i &
        resp_msg  = c_msg_out &
        c_msg_out : t_msg
    THEN
        data_Value_Value,
        data_Value_StatusCode
        :(
            d_inv_response
         )
    END

END
