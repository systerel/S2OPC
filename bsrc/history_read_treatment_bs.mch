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
    history_read_treatment_bs

SEES
    constants,
    constants_statuscodes_bs,
    msg_history_read_request

OPERATIONS

    p_StatusCode, p_contPoint, p_nbDataValues, p_DataValues
        <-- external_history_raw_read (p_readRawDetails, p_TSrequired, p_ContinuationPoint, p_singleValueId) =
    PRE
        p_readRawDetails : t_readRawModifiedDetails_i &
        p_readRawDetails : t_readRawModifiedDetails &
        p_TSrequired : BOOL &
        p_ContinuationPoint : BOOL &
        p_singleValueId : t_historyReadValueId_i &
        p_singleValueId : t_historyReadValueId
    THEN
        p_StatusCode, p_contPoint, p_nbDataValues, p_DataValues :(
            p_StatusCode : t_StatusCode_i &
            p_StatusCode : {e_sc_ok, e_sc_bad_internal_error, e_sc_bad_node_id_unknown, e_sc_bad_node_id_invalid,
                            e_sc_bad_continuation_point_invalid, e_sc_bad_index_range_invalid, e_sc_bad_index_range_no_data,
                            e_sc_bad_no_continuation_points, e_sc_bad_out_of_memory} &
            p_contPoint : t_Nonce_i &
            p_nbDataValues : NAT &
            p_DataValues : t_DataValue_array_i &

            (p_StatusCode /= e_sc_ok =>
                p_contPoint = c_Nonce_indet &
                p_nbDataValues = 0 &
                p_DataValues = c_DataValue_array_indet) &
            (p_StatusCode = e_sc_ok =>
                p_contPoint : t_Nonce &
                p_nbDataValues > 0 &
                p_DataValues : t_DataValue_array)
        )
    END
    ;

    set_ts_srv_dataValues(p_nbDataValues, p_DataValues, p_currentTs) =
    PRE
        p_nbDataValues : NAT &
        p_DataValues : t_DataValue_array_i &
        p_DataValues : t_DataValue_array &
        p_currentTs : t_Timestamp
    THEN
        /* Fill the fields ServerTimestamp and ServerPicoSeconds of p_DataValues */
        skip
    END

END
