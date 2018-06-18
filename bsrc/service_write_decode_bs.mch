/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
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
    ;
    
    /* inter B to C code use only: used to return the writeValue as address space modification notification */
    wvPointer <-- getAndClean_WriteValuePointer(wvi) =
    PRE
        wvi : t_WriteValue_i &
        wvi : cast_t_WriteValue[1..nb_WriteValue]
    THEN
        wvPointer :: t_WriteValuePointer_i
    END

END
