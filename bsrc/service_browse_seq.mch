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
    service_browse_seq

SEES
    constants,
    message_in_bs

OPERATIONS

    StatusCode_service <-- treat_browse_request_BrowseValues =
    /* TODOPAB: add PRE to ensure decode decode_browse_request is called before this one */
    /* ... and resets the variable at the end of the op */
    BEGIN
        StatusCode_service :: t_StatusCode_i
    END
    ;

    /* Promoted from service_browse_decode_bs */
    StatusCode_service <-- decode_browse_request(req_payload) =
    PRE
        req_payload : t_msg_i &
        req_payload = c_msg_in
    THEN
        StatusCode_service
        :(
            StatusCode_service : t_StatusCode_i &
            StatusCode_service : t_StatusCode
        )
    END
    ;
    
    /* Browse request cannot be used anymore */
    free_browse_request =
    BEGIN
        skip
    END
    ;

    /* Promoted from msg_browse_response_bs */
    p_isvalid <-- write_BrowseResponse_msg_out(p_msg_out) =
    PRE
        p_msg_out : t_msg_i &
        p_msg_out : t_msg
    THEN
        p_isvalid :: BOOL
    END
    ;

    /* Promoted from msg_browse_response_bs */
    free_browse_result =
    BEGIN
        skip
    END
    ;

    /* Promoted from msg_browse_response_bs */
    set_ResponseBrowse_BrowseStatus(p_bvi, p_sc) =
    PRE
        p_bvi  : t_BrowseValue_i &
        p_bvi  : t_BrowseValue   &
        p_sc : t_StatusCode_i &
        p_sc : t_StatusCode
    THEN
        skip
    END

END
