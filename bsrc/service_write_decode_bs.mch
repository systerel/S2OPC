/*
 * Licensed to Systerel under one or more contributor license
 * agreements. See the NOTICE file distributed with this work
 * for additional information regarding copyright ownership.
 * Systerel licenses this file to you under the Apache
 * License, Version 2.0 (the "License"); you may not use this
 * file except in compliance with the License. You may obtain
 * a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
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
        WriteValue_Value, /* The field value of the DataValue */
        WriteValue_IndexRange
        /* TODO: StatusCode, SourceTimestamp (determine if ServerTimestamp is useful) */
    ;
    d_inv ==
        nb_WriteValue : NAT &
        nb_WriteValue <= k_n_WriteResponse_max &
        WriteValue_NodeId : t_WriteValue +-> t_NodeId &
        WriteValue_AttributeId : t_WriteValue +-> t_AttributeId_i &
        WriteValue_Value : t_WriteValue +-> t_Variant &
        WriteValue_IndexRange : t_WriteValue +-> t_IndexRange
    ;
    d_init ==
        nb_WriteValue := 0 ||
        WriteValue_NodeId :: {{}} ||
        WriteValue_AttributeId :: {{}} ||
        WriteValue_Value :: {{}} ||
        WriteValue_IndexRange :: {{}}

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
            StatusCode_service : {e_sc_ok, e_sc_bad_nothing_to_do, e_sc_bad_too_many_ops} &
            d_inv &
            (StatusCode_service = e_sc_ok =>
              nb_WriteValue > 0 &
              WriteValue_NodeId : cast_t_WriteValue[1..nb_WriteValue] --> t_NodeId &
              WriteValue_AttributeId : cast_t_WriteValue[1..nb_WriteValue] --> t_AttributeId_i &
              WriteValue_Value : cast_t_WriteValue[1..nb_WriteValue] --> t_Variant &
              WriteValue_IndexRange : cast_t_WriteValue[1..nb_WriteValue] --> t_IndexRange) &
            (StatusCode_service /= e_sc_ok =>
                nb_WriteValue = 0 &
                WriteValue_NodeId = {} &
                WriteValue_AttributeId = {} &
                WriteValue_Value = {} &
                WriteValue_IndexRange = {})
        )
    END
    ;

    free_write_request =
    BEGIN
        d_init
    END
    ;

    nb_req <-- get_nb_WriteValue =
    PRE
        nb_WriteValue > 0
    THEN
        nb_req := nb_WriteValue
    END
    ;

    isvalid, status, nid, aid, value, index_range <-- getall_WriteValue(wvi) =
    PRE
        nb_WriteValue > 0 &
        wvi : t_WriteValue_i &
        wvi : cast_t_WriteValue[1..nb_WriteValue] &
        wvi : dom(WriteValue_NodeId) &
        wvi : dom(WriteValue_AttributeId) &
        wvi : dom(WriteValue_Value)
    THEN
        IF WriteValue_AttributeId(wvi) : {e_aid_NodeId, e_aid_NodeClass, e_aid_Value}
        THEN
            isvalid := TRUE ||
            status := e_sc_ok ||
            nid,
            aid,
            value,
            index_range
            :(nid : t_NodeId_i  & nid = WriteValue_NodeId(wvi) &
                aid : t_AttributeId_i & aid : {e_aid_NodeId, e_aid_NodeClass, e_aid_Value} &
                value : t_Variant_i & value = WriteValue_Value(wvi) &
                index_range : t_IndexRange_i & index_range = WriteValue_IndexRange(wvi))
        ELSE
            isvalid := FALSE ||
            nid := c_NodeId_indet ||
            aid := c_AttributeId_indet ||
            value := c_Variant_indet ||
            index_range := c_IndexRange_indet ||
            status := e_sc_bad_attribute_id_invalid
        END
    END
    ;

    /* inter B to C code use only: used to return the writeValue as address space modification notification */
    wvPointer <-- getall_WriteValuePointer(wvi) =
    PRE
        wvi : t_WriteValue_i &
        wvi : cast_t_WriteValue[1..nb_WriteValue]
    THEN
        wvPointer :: t_WriteValuePointer_i
    END

END
