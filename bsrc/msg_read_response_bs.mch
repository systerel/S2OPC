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
    msg_read_response_bs

SEES
    constants,
    constants_statuscodes_bs,
    message_out_bs

DEFINITIONS
    d_inv_response == (
        response_StatusCode : t_StatusCode_i &
        data_Value_Value : t_ReadValue +-> t_Variant &
        data_Value_StatusCode : t_ReadValue +-> t_RawStatusCode &
        data_Value_SourceTimestamp : t_ReadValue +-> t_Timestamp &
        data_Value_ServerTimestamp : t_ReadValue +-> t_Timestamp
    )

ABSTRACT_VARIABLES
    response_StatusCode,
    data_Value_Value,
    data_Value_StatusCode,
    data_Value_SourceTimestamp,
    data_Value_ServerTimestamp

INVARIANT
    d_inv_response

INITIALISATION
    data_Value_Value      :: {{}} ||
    data_Value_StatusCode :: {{}} ||
    data_Value_SourceTimestamp :: {{}} ||
    data_Value_ServerTimestamp :: {{}} ||
    response_StatusCode := c_StatusCode_indet

    /*data_Value_SourceTimestamp, data_Value_ServerTimestamp := {}, {} ||*/

OPERATIONS

    p_isvalid <-- alloc_read_response(p_nb_resps, p_resp_msg) =
    PRE
        p_nb_resps : NAT &
        p_nb_resps <= k_n_read_resp_max &
        p_resp_msg : t_msg_i &
        p_resp_msg = c_msg_out &
        c_msg_out  : t_msg
    THEN
        p_isvalid,
        response_StatusCode
        :(
            p_isvalid : BOOL &  /* malloc result */
            response_StatusCode : t_StatusCode_i &

            (p_isvalid = FALSE
             =>
             response_StatusCode = e_sc_bad_out_of_memory)
         )
    END
    ;

    set_read_response(p_resp_msg, p_rvi, p_value, p_raw_sc, p_ts_src, p_ts_srv) =
    PRE
        p_resp_msg    : t_msg_i         &
        p_resp_msg    = c_msg_out       &
        c_msg_out   : t_msg             &
        p_rvi         : t_ReadValue_i   &
        p_rvi         : t_ReadValue     &
        p_value       : t_Variant_i     &
        p_raw_sc : t_RawStatusCode      &
        p_ts_src : t_Timestamp          &
        p_ts_srv : t_Timestamp
    THEN
        data_Value_Value(p_rvi) := p_value ||
        data_Value_StatusCode(p_rvi) := p_raw_sc ||
        data_Value_SourceTimestamp(p_rvi) := p_ts_src ||
        data_Value_ServerTimestamp(p_rvi) := p_ts_srv
    END

END
