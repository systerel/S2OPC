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
    msg_read_request_bs

SEES
    constants,
    constants_statuscodes_bs,
    message_in_bs

ABSTRACT_VARIABLES
    ReadValue_NodeId,
    ReadValue_AttributeId,
    ReadValue_IndexRange,
    ReadValue_MaxAge

INVARIANT
    ReadValue_NodeId      : t_ReadValue +-> t_NodeId &
    ReadValue_AttributeId : t_ReadValue +-> t_AttributeId_i &
    ReadValue_IndexRange  : t_ReadValue +-> t_IndexRange_i &
    ReadValue_MaxAge : INT

INITIALISATION
    ReadValue_NodeId,
    ReadValue_AttributeId,
    ReadValue_MaxAge,
    ReadValue_IndexRange
    :(
        ReadValue_NodeId      = {} &
        ReadValue_AttributeId = {} &
        ReadValue_IndexRange  = {} &
        ReadValue_MaxAge : INT &
        ReadValue_MaxAge < 0
     )

OPERATIONS
    nb <-- read_req_nb_ReadValue(msg) =
    PRE
        msg  : t_msg_i &
        msg  = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type = e_msg_attribute_read_req
    THEN
        nb,
        ReadValue_NodeId,
        ReadValue_AttributeId,
        ReadValue_IndexRange,
        ReadValue_MaxAge
         :(nb : INT &
             (nb > 0 =>
               ReadValue_NodeId      : cast_t_ReadValue[1..nb] --> t_NodeId &
               ReadValue_AttributeId : cast_t_ReadValue[1..nb] --> t_AttributeId_i &
               ReadValue_IndexRange  : cast_t_ReadValue[1..nb] --> t_IndexRange_i) &
             (nb <= 0 =>
               ReadValue_NodeId      = {} &
               ReadValue_AttributeId = {} &
               ReadValue_IndexRange  = {}) &
            ReadValue_MaxAge : INT)
    END
    ;

    p_tsToReturn <-- read_req_TimestampsToReturn(p_msg) =
    PRE
        p_msg : t_msg_i &
        p_msg = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type = e_msg_attribute_read_req
    THEN
        p_tsToReturn :: t_TimestampsToReturn_i
    END
    ;

    p_maxAge_valid <-- read_req_MaxAge(p_msg) =
    PRE
        p_msg : t_msg_i &
        p_msg = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type = e_msg_attribute_read_req
    THEN
        p_maxAge_valid := bool(ReadValue_MaxAge >= 0)
    END
    ;

    /* Returns the NodeId (pointer or hash_id, or...)
     * Does not check if there is a node in the AddressSpace corresponding to this nid */
    nid <-- getall_req_ReadValue_NodeId(msg, rvi) =
    PRE
        msg  : t_msg_i &
        msg  = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type = e_msg_attribute_read_req &
        rvi : t_ReadValue_i &
        rvi : t_ReadValue &
        rvi : dom(ReadValue_NodeId)
    THEN
        nid :(nid : t_NodeId_i &
              nid : t_NodeId &
              nid = ReadValue_NodeId(rvi))
    END
    ;

    is_known_encoding, data_encoding <-- getall_req_ReadValue_DataEncoding(msg, rvi) =
    PRE
        msg  : t_msg_i &
        msg  = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type = e_msg_attribute_read_req &
        rvi : t_ReadValue_i &
        rvi : t_ReadValue &
        rvi : dom(ReadValue_NodeId)
    THEN
        is_known_encoding,
        data_encoding :(data_encoding : t_QualifiedName_i &
                        is_known_encoding : BOOL &
                        /* If encoding is not defined, it is considered as known (use default) */
                        (data_encoding = c_QualifiedName_indet => is_known_encoding = TRUE))
    END
    ;

    index_range <-- getall_req_ReadValue_IndexRange(msg, rvi) =
    PRE
        msg  : t_msg_i &
        msg  = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type = e_msg_attribute_read_req &
        rvi : t_ReadValue_i &
        rvi : t_ReadValue &
        rvi : dom(ReadValue_IndexRange)
    THEN
        index_range :(index_range : t_IndexRange_i &
                      index_range = ReadValue_IndexRange(rvi))
    END
    ;

    p_sc, aid <-- getall_req_ReadValue_AttributeId(msg, rvi) =
    PRE
        msg  : t_msg_i &
        msg  = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type = e_msg_attribute_read_req &
        rvi : t_ReadValue_i &
        rvi : t_ReadValue &
        rvi : dom(ReadValue_AttributeId)
    THEN
        aid := ReadValue_AttributeId(rvi) ||
        IF ReadValue_AttributeId(rvi) = c_AttributeId_indet THEN
            p_sc := e_sc_bad_attribute_id_invalid
        ELSE
            p_sc := e_sc_ok
        END
    END

END
