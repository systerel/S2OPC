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
    service_read_1

SEES
    constants,
    constants_statuscodes_bs,
    address_space_itf,
    message_in_bs,
    message_out_bs

DEFINITIONS
    "definitions.def";
    d_inv_read == (
        nb_ReadValue          : NAT                           &
        nb_ReadValue         <= k_n_read_resp_max             &
        ReadValues_TimestampToReturn : t_TimestampsToReturn_i &
        ReadValue_NodeId      : t_ReadValue +-> t_NodeId      &
        ReadValue_AttributeId : t_ReadValue +-> t_AttributeId_i
    )
    ;
    d_inv_response == (
        data_Value_Value           : t_ReadValue +-> t_Variant &
        data_Value_StatusCode      : t_ReadValue +-> t_RawStatusCode &
        data_Value_SourceTimestamp : t_ReadValue +-> t_Timestamp &
        data_Value_ServerTimestamp : t_ReadValue +-> t_Timestamp
    )

ABSTRACT_VARIABLES
    nb_ReadValue,
    ReadValues_TimestampToReturn,
    ReadValue_NodeId,
    ReadValue_AttributeId,
    data_Value_Value,
    data_Value_StatusCode,
    data_Value_SourceTimestamp,
    data_Value_ServerTimestamp

INVARIANT
    d_inv_read &
    d_inv_response

INITIALISATION
    nb_ReadValue          := 0  ||
    ReadValues_TimestampToReturn := c_TimestampsToReturn_indet ||
    ReadValue_NodeId      := {} ||
    ReadValue_AttributeId := {} ||
    data_Value_Value      := {} ||
    data_Value_StatusCode := {} ||
    data_Value_SourceTimestamp := {} ||
    data_Value_ServerTimestamp := {}

OPERATIONS

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
        ReadValues_TimestampToReturn,
        ReadValue_NodeId,
        ReadValue_AttributeId
        :(  p_read_ok      : BOOL & /* Error code, for now a simple bool */
            p_nb_ReadValue : INT  &
            ReadValues_TimestampToReturn : t_TimestampsToReturn_i &
            p_tsToReturn   : t_TimestampsToReturn_i &
            p_statusCode   : {e_sc_ok, e_sc_bad_too_many_ops, e_sc_bad_nothing_to_do,
                              e_sc_bad_timestamps_to_return_invalid, e_sc_bad_max_age_invalid} &
            (p_read_ok = TRUE =>
             d_inv_read &
             nb_ReadValue > 0 &
             ReadValue_NodeId      : cast_t_ReadValue[1..nb_ReadValue] --> t_NodeId &
             ReadValue_AttributeId : cast_t_ReadValue[1..nb_ReadValue] --> t_AttributeId_i &
             p_nb_ReadValue = nb_ReadValue &
             p_tsToReturn : t_TimestampsToReturn &
             ReadValues_TimestampToReturn = p_tsToReturn &
             p_statusCode = e_sc_ok) &
            (p_read_ok = FALSE =>
             nb_ReadValue = 0 &
             ReadValues_TimestampToReturn = c_TimestampsToReturn_indet &
             ReadValue_NodeId = {} &
             ReadValue_AttributeId = {} &
             p_statusCode /= e_sc_ok
            )
        )
    END
    ;

    p_isvalid <-- alloc_read_response(p_nb_resps, p_resp_msg) =
    PRE
        p_nb_resps : NAT                              &
        p_nb_resps <= k_n_read_resp_max               &
        p_resp_msg : t_msg_i                          &
        p_resp_msg = c_msg_out                        &
        c_msg_out  : t_msg
    THEN
        p_isvalid :: BOOL  /* malloc result */
    END
    ;

    fill_read_response(p_TimestampsToReturn, p_user, p_locales, req_msg, resp_msg) =
    PRE
        p_TimestampsToReturn : t_TimestampsToReturn_i &
        p_TimestampsToReturn : t_TimestampsToReturn   &
        p_user    : t_user_i &
        p_user    : t_user   &
        p_locales : t_LocaleIds_i &
        p_locales : t_LocaleIds &
        req_msg   : t_msg_i &
        req_msg   = c_msg_in &
        c_msg_in  : t_msg &
        resp_msg  : t_msg_i &
        resp_msg  = c_msg_out &
        c_msg_out : t_msg &
        nb_ReadValue > 0
    THEN
        data_Value_Value,
        data_Value_StatusCode,
        data_Value_SourceTimestamp,
        data_Value_ServerTimestamp
        :(
            d_inv_response
         )
    END

END
