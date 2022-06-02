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
    address_space

SEES
    constants,
    constants_statuscodes_bs,
    service_write_decode_bs, /* TODO: move to refinment */
    data_value_pointer_bs

DEFINITIONS
    "address_space.def"

ABSTRACT_VARIABLES
    d_adsp_variables

INVARIANT
    d_adsp_invariant

INITIALISATION
    d_adsp_variables
    :(
        d_adsp_invariant
    )

OPERATIONS

    d_adsp_operations;

    p_status <-- treat_write_request_WriteValue (p_user, p_locales, p_wvi) =
    PRE
        p_user : t_user_i &
        p_user : t_user &
        p_locales : t_LocaleIds_i &
        p_locales : t_LocaleIds &
        p_wvi : t_WriteValue_i &
        p_wvi : t_WriteValue
    THEN
        LET dataValue, index_range, nid, aid BE
            dataValue = WriteValue_DataValue(p_wvi) &
            index_range = WriteValue_IndexRange(p_wvi) &
            nid = WriteValue_NodeId(p_wvi) &
            aid = WriteValue_AttributeId(p_wvi)
        IN
        p_status,
        a_Value,
        a_Value_StatusCode,
        a_Value_SourceTimestamp :(
            p_status : t_StatusCode_i &
            p_status : t_StatusCode &

            (aid : t_AttributeId
            =>
                p_status : {
                    e_sc_bad_node_id_unknown,
                    e_sc_bad_not_writable,
                    e_sc_bad_type_mismatch,
                    e_sc_bad_not_writable,
                    e_sc_bad_user_access_denied,
                    e_sc_bad_index_range_invalid,
                    e_sc_bad_index_range_no_data,
                    e_sc_bad_invalid_argument,
                    e_sc_bad_out_of_memory,
                    e_sc_ok
                }
            ) &
            (not(aid : t_AttributeId)
            =>
                p_status = e_sc_bad_attribute_id_invalid
            ) &
            (p_status = e_sc_ok =>
                a_Value = a_Value$0 <+ {a_NodeId~(nid) |-> DataValue_Variant(dataValue)} &

                (local_service_treatment = TRUE
                =>
                    a_Value_StatusCode = a_Value_StatusCode$0
                        <+ {a_NodeId~(nid) |-> DataValue_StatusCode(dataValue)} &
                    a_Value_SourceTimestamp = a_Value_SourceTimestamp$0
                        <+ {a_NodeId~(nid) |-> DataValue_SourceTimestamp(dataValue)}
                ) &
                (local_service_treatment = FALSE
                =>
                    (conv_AccessLevel_StatusWrite(a_AccessLevel(a_NodeId~(nid))) = TRUE
                    =>
                        a_Value_StatusCode = a_Value_StatusCode$0
                            <+ {a_NodeId~(nid) |-> DataValue_StatusCode(dataValue)}
                    ) &
                    (not(conv_AccessLevel_StatusWrite(a_AccessLevel(a_NodeId~(nid))) = TRUE)
                    =>
                        a_Value_StatusCode = a_Value_StatusCode$0) &

                    (conv_AccessLevel_TimestampWrite (a_AccessLevel(a_NodeId~(nid))) = TRUE
                    =>
                        a_Value_SourceTimestamp = a_Value_SourceTimestamp$0
                            <+ {a_NodeId~(nid) |-> DataValue_SourceTimestamp(dataValue)}
                    ) &
                    (not(conv_AccessLevel_TimestampWrite (a_AccessLevel(a_NodeId~(nid))) = TRUE)
                    =>
                        a_Value_SourceTimestamp = a_Value_SourceTimestamp$0)
                )
            ) &
            (not (p_status = e_sc_ok)
            =>
                a_Value = a_Value$0 &
                a_Value_StatusCode = a_Value_StatusCode$0 &
                a_Value_SourceTimestamp = a_Value_SourceTimestamp$0
            )
        )
        END
    END

END
