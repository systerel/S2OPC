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
    service_set_view

SEES
    constants,
    constants_statuscodes_bs,
    message_in_bs,
    message_out_bs

OPERATIONS

    StatusCode_service <-- treat_browse_request (p_session, p_req_msg, p_resp_msg) =
    PRE
        p_session : t_session_i &
        p_req_msg : t_msg_i &
        p_req_msg : t_msg &
        p_req_msg = c_msg_in &
        a_msg_in_type = e_msg_view_browse_req &
        p_resp_msg : t_msg_i &
        p_resp_msg : t_msg &
        p_resp_msg = c_msg_out &
        a_msg_out_type = e_msg_view_browse_resp
    THEN
        StatusCode_service
        :(  StatusCode_service : t_StatusCode_i &
            StatusCode_service : t_StatusCode
        )
    END
    ;

    StatusCode_service <-- treat_browse_next_request (p_session, p_req_msg, p_resp_msg) =
    PRE
        p_session : t_session_i &
        p_req_msg : t_msg_i &
        p_req_msg : t_msg &
        p_req_msg = c_msg_in &
        a_msg_in_type = e_msg_view_browse_next_req &
        p_resp_msg : t_msg_i &
        p_resp_msg : t_msg &
        p_resp_msg = c_msg_out &
        a_msg_out_type = e_msg_view_browse_next_resp
    THEN
        StatusCode_service
        :(  StatusCode_service : t_StatusCode_i &
            StatusCode_service : t_StatusCode
        )
    END
    ;

    /* Remove associated continuation point */
    service_set_view_set_session_closed(p_session) =
    PRE
        p_session : t_session_i &
        p_session : t_session
    THEN
        skip
    END
    ;

    /* Address space modification => clear all continuation points */
    service_set_view_service_node_management_used =
    BEGIN
        skip
    END
    ;

    service_set_view_UNINITIALISATION =
    BEGIN
        skip
    END
    ;


    StatusCode_service <-- treat_translate_browse_paths_request (p_req_msg, p_resp_msg) =
    PRE
        p_req_msg : t_msg_i &
        p_req_msg : t_msg &
        p_req_msg = c_msg_in &
        a_msg_in_type = e_msg_view_translate_browse_paths_to_node_ids_req &
        p_resp_msg : t_msg_i &
        p_resp_msg : t_msg &
        p_resp_msg = c_msg_out
    THEN
        StatusCode_service
        :(  StatusCode_service : t_StatusCode_i &
            StatusCode_service : t_StatusCode
        )
    END

END
