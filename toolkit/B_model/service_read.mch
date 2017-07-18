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
    service_read

SEES
    constants,
    address_space,
    message_in_bs,
    message_out_bs

DEFINITIONS
    "definitions.def";
    d_inv_read == (
        nb_ReadValue : NAT &
        nb_ReadValue <= k_n_read_resp_max &
        ReadValue_Node : t_ReadValue +-> t_Node &
        ReadValue_AttributeId : t_ReadValue +-> t_AttributeId
    )
    ;
    d_inv_response == (
        data_Value_Value : t_ReadValue +-> t_Variant &
        data_Value_StatusCode : t_ReadValue +-> t_StatusCode /*&
        data_Value_SourceTimestamp : set_read_elements +-> t_Timestamp &
        data_Value_ServerTimestamp : set_read_elements --> t_Timestamp*/
    )

ABSTRACT_VARIABLES
    nb_ReadValue,
    ReadValue_Node,
    ReadValue_AttributeId,
    data_Value_Value,
    data_Value_StatusCode

INVARIANT
    d_inv_read &
    d_inv_response

INITIALISATION
    nb_ReadValue          := 0  ||
    ReadValue_Node        := {} ||
    ReadValue_AttributeId := {} ||
    data_Value_Value      := {} ||
    data_Value_StatusCode := {}

OPERATIONS
    treat_read_request(req_msg, resp_msg) =
    PRE
        req_msg   : t_msg_i &
        req_msg   = c_msg_in &
        c_msg_in  : t_msg &
        resp_msg   : t_msg_i &
        resp_msg   = c_msg_out &
        c_msg_out : t_msg
    THEN
        nb_ReadValue,
        ReadValue_Node,
        ReadValue_AttributeId,
        data_Value_Value,
        data_Value_StatusCode
        :(
            d_inv_read &
            data_Value_Value : t_ReadValue +-> t_Variant &
            data_Value_StatusCode : t_ReadValue +-> t_StatusCode
         )
    END

END
