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
    msg_read_request

SEES
    constants,
    constants_statuscodes_bs,
    message_in_bs

DEFINITIONS
    d_inv == (
        nb_ReadValue          : NAT                      &
        nb_ReadValue         <= k_n_read_resp_max        &
        ReadValue_NodeId      : t_ReadValue +-> t_NodeId &
        ReadValue_AttributeId : t_ReadValue +-> t_AttributeId_i &
        ReadValue_IndexRange  : t_ReadValue +-> t_IndexRange_i
    )

ABSTRACT_VARIABLES
    nb_ReadValue,
    ReadValue_NodeId,
    ReadValue_AttributeId,
    ReadValue_IndexRange

INVARIANT
    d_inv

INITIALISATION
    nb_ReadValue          := 0  ||
    ReadValue_NodeId      := {} ||
    ReadValue_AttributeId := {} ||
    ReadValue_IndexRange  := {}

OPERATIONS

    /* Makes the ReadRequest available to the machine (if possible) */
    p_read_ok, p_nb_ReadValue, p_tsToReturn, p_statusCode <-- check_ReadRequest(p_msg) =
    PRE
        p_msg  : t_msg_i &
        p_msg  = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type = e_msg_attribute_read_req
    THEN
        p_read_ok,
        p_nb_ReadValue,
        p_tsToReturn,
        p_statusCode,
        nb_ReadValue,
        ReadValue_NodeId,
        ReadValue_AttributeId,
        ReadValue_IndexRange
        :(  p_read_ok      : BOOL & /* Error code, for now a simple bool */
            p_nb_ReadValue : INT  &
            p_tsToReturn   : t_TimestampsToReturn_i &
            p_statusCode   : {e_sc_ok, e_sc_bad_too_many_ops, e_sc_bad_nothing_to_do,
                              e_sc_bad_timestamps_to_return_invalid, e_sc_bad_max_age_invalid} &
            (p_read_ok = TRUE =>
             d_inv &
             nb_ReadValue > 0 &
             ReadValue_NodeId      : cast_t_ReadValue[1..nb_ReadValue] --> t_NodeId &
             ReadValue_AttributeId : cast_t_ReadValue[1..nb_ReadValue] --> t_AttributeId_i &
             ReadValue_IndexRange  : cast_t_ReadValue[1..nb_ReadValue] --> t_IndexRange_i &
             p_nb_ReadValue = nb_ReadValue &
             p_tsToReturn : t_TimestampsToReturn &
             p_statusCode = e_sc_ok) &
            (p_read_ok = FALSE =>
             nb_ReadValue = 0 &
             ReadValue_NodeId = {} &
             ReadValue_AttributeId = {} &
             ReadValue_IndexRange = {} &
             p_statusCode /= e_sc_ok
            )
        )
    END
    ;

    p_sc, p_nid, p_aid, p_index_range <-- getall_ReadValue_NodeId_AttributeId(p_msg, p_rvi) =
    PRE
        nb_ReadValue > 0 &
        p_msg    : t_msg_i                     &
        p_msg    = c_msg_in                    &
        c_msg_in : t_msg                       &
        a_msg_in_type = e_msg_attribute_read_req &
        p_rvi : t_ReadValue_i                  &
        p_rvi : t_ReadValue                    &
        p_rvi : cast_t_ReadValue[0..nb_ReadValue] &
        p_rvi : dom(ReadValue_NodeId) &
        p_rvi : dom(ReadValue_AttributeId) &
        p_rvi : dom(ReadValue_IndexRange)
    THEN
        p_sc,
        p_nid,
        p_aid,
        p_index_range
        :(p_sc : t_StatusCode_i &
          p_sc : {e_sc_ok, e_sc_bad_node_id_invalid, e_sc_bad_attribute_id_invalid, e_sc_bad_index_range_invalid, e_sc_bad_data_encoding_invalid} &
            p_nid : t_NodeId_i                   &
            p_nid : t_NodeId                     &
            p_aid : t_AttributeId_i              &
            p_index_range : t_IndexRange_i       &
            p_nid = ReadValue_NodeId(p_rvi)      &
            p_aid = ReadValue_AttributeId(p_rvi) &
            p_index_range = ReadValue_IndexRange(p_rvi) &
            (p_sc = e_sc_ok => p_aid : t_AttributeId)
        )
    END
    ;

    p_nb_ReadValue <-- get_nb_ReadValue =
    PRE
        nb_ReadValue > 0
    THEN
        p_nb_ReadValue := nb_ReadValue
    END

END
