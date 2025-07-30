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
    msg_history_read_response_bs

SEES
    constants,
    constants_statuscodes_bs,
    message_in_bs,
    message_out_bs,
    msg_history_read_request

ABSTRACT_VARIABLES
    historyReadResult_historyDataValue,
    statusCode_alloc

INVARIANT
    historyReadResult_historyDataValue : t_historyReadReqIndex +-> t_DataValue_array &
    statusCode_alloc : t_StatusCode_i

INITIALISATION
    historyReadResult_historyDataValue :( historyReadResult_historyDataValue = {} ) ||
    statusCode_alloc := c_StatusCode_indet

OPERATIONS

    p_is_valid <-- alloc_msg_hist_read_resp_results(p_nb_to_read, p_resp_msg) =
    PRE
        p_nb_to_read : NAT     &
        p_nb_to_read = nbNodesToRead &
        p_resp_msg : t_msg_i   &
        p_resp_msg = c_msg_out &
        c_msg_out  : t_msg
    THEN
        p_is_valid, statusCode_alloc :(
            p_is_valid : BOOL & /* Malloc result */
            statusCode_alloc : t_StatusCode_i &
            (p_is_valid = TRUE  => statusCode_alloc = e_sc_ok) &
            (p_is_valid = FALSE => statusCode_alloc = e_sc_bad_out_of_memory)
        )
    END
    ;

    set_msg_hist_read_response(p_resp_msg, p_index, p_sc, p_contPoint, p_nbDataValues, p_DataValues) =
    PRE
        p_resp_msg   : t_msg_i               &
        p_resp_msg   = c_msg_out             &
        c_msg_out    : t_msg                 &
        p_index      : NAT                   &
        p_index      : t_historyReadReqIndex &
        p_sc         : t_StatusCode_i        &
        p_sc         : t_StatusCode          &
        p_contPoint  : t_Nonce_i             &
        p_nbDataValues : NAT                 &
        p_DataValues : t_DataValue_array_i   &
        statusCode_alloc = e_sc_ok           &
        p_index     <= nbNodesToRead
    THEN
        IF p_DataValues : t_DataValue_array THEN
            historyReadResult_historyDataValue(p_index) := p_DataValues
        END
    END

END
