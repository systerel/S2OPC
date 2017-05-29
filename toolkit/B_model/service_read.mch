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
        ReadValue_Node : t_ReadValue <-> t_Node &
        ReadValue_AttributeId : t_ReadValue <-> t_AttributeId
    )
    ;
    d_inv_response == (
        data_Value_Value : t_ReadValue <-> t_Variant &
        data_Value_StatusCode : t_ReadValue <-> t_StatusCode /*&
        data_Value_SourceTimestamp : set_read_elements +-> t_Timestamp &
        data_Value_ServerTimestamp : set_read_elements --> t_Timestamp*/
    )

ABSTRACT_VARIABLES
    nb_ReadValue, ReadValue_Node, ReadValue_AttributeId,
    data_Value_Value, data_Value_StatusCode/*, data_Value_SourceTimestamp, data_Value_ServerTimestamp*/

INVARIANT
    d_inv_read &
    d_inv_response

INITIALISATION
    nb_ReadValue := 0  ||
    ReadValue_Node := {} ||
    ReadValue_AttributeId := {} ||
    data_Value_Value, data_Value_StatusCode := {}, {} /* ||
    data_Value_SourceTimestamp, data_Value_ServerTimestamp := {}, {} */

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
        nb_ReadValue, ReadValue_Node, ReadValue_AttributeId,
        data_Value_Value, data_Value_StatusCode/*,
        data_Value_SourceTimestamp, data_Value_ServerTimestamp */
        :(
            /* TODO: It would be nice to specify that a response is always sent, even when there is an error */
            d_inv_read &
            data_Value_Value : t_ReadValue +-> t_Variant &
            data_Value_Value = % rvi. (
                rvi : t_ReadValue &
                /* ith node id has a node in the Address Space */
                ReadValue_Node(rvi) : dom(a_NodeId) &
                /* Node is in the domain of the attribute */
                ReadValue_Node(rvi) : dom(AttributeID_Node_Variant(ReadValue_AttributeId(rvi)))
                /* TODO: add use-access levels */
                |
                AttributeID_Node_Variant(ReadValue_AttributeId(rvi)) (ReadValue_Node(rvi))
            ) &

            data_Value_StatusCode : t_ReadValue --> t_StatusCode &
            data_Value_StatusCode = {rvi,sc |
                rvi : t_ReadValue & sc : t_StatusCode &
                (
                    rvi /: dom(data_Value_Value) /* TODO: split in two different noks (nodeId invalid, or attrId invalid) */
                        => sc = e_sc_nok
                ) &
                (
                    rvi : dom(data_Value_Value)
                    /* Soit on est dans l'attribut Value d'une Variable,
                       -> le StatusCode associé à la Value de la Var
                       Sinon, c'est le StatusCode de l'opération (ok) */
                    =>
                    (
                        (ReadValue_AttributeId(rvi) = e_aid_Value &
                         a_NodeClass(ReadValue_Node(rvi)) = e_ncl_Variable
                            => sc = a_Value_StatusCode(ReadValue_Node(rvi)) )
                        &
                        ( not(ReadValue_AttributeId(rvi) = e_aid_Value &
                              a_NodeClass(ReadValue_Node(rvi)) = e_ncl_Variable)
                           => sc = e_sc_ok )
                    )
                )
            }

            /*(!ii.ii : dom(data_Value_Value) & ReadValue_Node(ii) : dom(att(ReadValue_AttributeId(ii)))
                => data_Value_Value(ii) = att(ReadValue_AttributeId(ii))(ReadValue_Node(ii)) ) &
            data_Value_StatusCode : set_read_elements --> t_StatusCode &
            data_Value_SourceTimestamp
            data_Value_ServerTimestamp */
        )
    END

END
