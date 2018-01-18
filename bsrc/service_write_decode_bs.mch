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
    service_write_decode_bs

SEES
    constants,
    message_in_bs

DEFINITIONS
    d_variables ==
        nb_WriteValue,
        WriteValue_NodeId,
        WriteValue_AttributeId,
        WriteValue_Value /* The field value of the DataValue */
        /* TODO: StatusCode, SourceTimestamp (determine if ServerTimestamp is useful) */
    ;
    d_inv ==
        nb_WriteValue : NAT &
        nb_WriteValue <= k_n_WriteResponse_max &
        WriteValue_NodeId : t_WriteValue +-> t_NodeId &
        WriteValue_AttributeId : t_WriteValue +-> t_AttributeId &
        WriteValue_Value : t_WriteValue +-> t_Variant
    ;
    d_init ==
        nb_WriteValue := 0 ||
        WriteValue_NodeId :: {{}} ||
        WriteValue_AttributeId :: {{}} ||
        WriteValue_Value :: {{}}

ABSTRACT_VARIABLES
    d_variables

INVARIANT
    d_inv

INITIALISATION
    d_init

OPERATIONS
    StatusCode_service <-- decode_write_request(write_msg) =
    PRE
        write_msg : t_msg_i &
        write_msg = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type = e_msg_attribute_write_req
    THEN
        StatusCode_service,
        d_variables
        :(
            StatusCode_service : t_StatusCode_i &
            StatusCode_service : t_StatusCode &
            d_inv
        )
    END
    ;

    free_write_request =
    BEGIN
        d_init
    END
    ;

    nb_req <-- get_nb_WriteValue =
    BEGIN
        nb_req := nb_WriteValue
    END
    ;

    isvalid, status, nid, aid, value <-- getall_WriteValue(wvi) =
    PRE
        wvi : t_WriteValue_i &
        wvi : cast_t_WriteValue[1..nb_WriteValue]
    THEN
        IF wvi : dom(WriteValue_NodeId) &
            wvi : dom(WriteValue_AttributeId) &
            wvi : dom(WriteValue_Value)
        THEN
            IF WriteValue_AttributeId(wvi) : {e_aid_NodeId, e_aid_NodeClass, e_aid_Value} THEN
                isvalid := TRUE ||
                status :: t_StatusCode_i ||
                nid,
                aid,
                value :(nid : t_NodeId_i  & nid = WriteValue_NodeId(wvi) &     
                        aid : t_AttributeId_i & aid = WriteValue_AttributeId(wvi) &
                        value : t_Variant_i & value = WriteValue_Value(wvi))
            ELSE
                isvalid := FALSE ||
                status := e_sc_bad_attribute_id_invalid ||
                nid := WriteValue_NodeId(wvi) ||
                aid := WriteValue_AttributeId(wvi) ||
                value := WriteValue_Value(wvi)
            END
        ELSE
            isvalid := FALSE ||
            status := e_sc_bad_internal_error ||
            nid :: t_NodeId_i ||
            aid :: t_AttributeId_i ||
            value :: t_Variant_i
        END
    END

END
