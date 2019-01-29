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
        data_Value_StatusCode : t_ReadValue +-> t_RawStatusCode

        /*data_Value_SourceTimestamp : t_ReadValue +-> t_Timestamp &
        data_Value_ServerTimestamp : t_ReadValue +-> t_Timestamp*/
    )

ABSTRACT_VARIABLES
    response_StatusCode,
    data_Value_Value,
    data_Value_StatusCode

INVARIANT
    d_inv_response

INITIALISATION
    data_Value_Value      :: {{}} ||
    data_Value_StatusCode :: {{}} ||
    response_StatusCode := c_StatusCode_indet

    /*data_Value_SourceTimestamp, data_Value_ServerTimestamp := {}, {} ||*/

OPERATIONS

    p_isvalid <-- alloc_read_response(p_nb_resps, p_TimestampsToReturn, p_resp_msg) =
    PRE
        p_nb_resps : NAT &
        p_nb_resps <= k_n_read_resp_max &
        p_TimestampsToReturn : t_TimestampsToReturn_i &
        p_TimestampsToReturn : t_TimestampsToReturn &
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

    set_read_response(resp_msg, rvi, val, raw_sc, aid) =
    PRE
        resp_msg    : t_msg_i         &
        resp_msg    = c_msg_out       &
        c_msg_out   : t_msg           &
        rvi         : t_ReadValue_i   &
        rvi         : t_ReadValue     &
        val         : t_Variant_i     &
        (val /= c_Variant_indet
         =>
         val : t_Variant)             &
        raw_sc          : t_RawStatusCode &
        aid         : t_AttributeId_i &
        (raw_sc = conv_StatusCode_To_RawStatusCode(e_sc_ok)
         =>
         aid : t_AttributeId)
    THEN
        IF  val /= c_Variant_indet THEN
            data_Value_Value(rvi) := val
        ELSE
            data_Value_Value := {rvi} <<| data_Value_Value
        END ||
        data_Value_StatusCode(rvi) := raw_sc
    END

END
