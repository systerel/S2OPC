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
    service_response_cb_bs

SEES
    constants,
    constants_statuscodes_bs,
    message_in_bs,
    message_out_bs

OPERATIONS

    cli_service_response (session, resp_msg, app_context) =
    PRE
        session : t_session_i &
        resp_msg   : t_msg_i &
        resp_msg   = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type : s_service_on_session_resps \/ s_discovery_service_resps &
        app_context : t_application_context_i
    THEN
        skip
    END
    ;

    cli_snd_failure (req_typ, app_context, error_status) =
    PRE
        req_typ : t_msg_type_i &
        req_typ : s_service_reqs &
        app_context : t_application_context_i &
        error_status : t_StatusCode_i
    THEN
        skip
    END
    ;

    srv_service_response (endpoint_config_idx, resp_msg, app_context) =
    PRE
        endpoint_config_idx : t_endpoint_config_idx_i &
        endpoint_config_idx : t_endpoint_config_idx &
        resp_msg   : t_msg_i &
        resp_msg   = c_msg_out &
        c_msg_out : t_msg &
        a_msg_out_type : s_service_on_session_resps \/ s_discovery_service_resps &
        app_context : t_application_context_i
    THEN
        skip
    END
    ;

    srv_write_notification (write_value_pointer, write_status) =
    PRE
        write_value_pointer : t_WriteValuePointer_i &
        write_status : t_StatusCode_i &
        write_status : t_StatusCode
    THEN
        skip
    END

END
