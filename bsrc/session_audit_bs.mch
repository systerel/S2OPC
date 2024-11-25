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
    session_audit_bs
SEES
    constants,
    constants_statuscodes_bs,
    channel_mgr,
    message_in_bs,
    message_out_bs

ABSTRACT_VARIABLES
    ReqAuditEntryInfo

INVARIANT
    ReqAuditEntryInfo : t_msg_header_i

INITIALISATION
    ReqAuditEntryInfo := c_msg_header_indet

OPERATIONS

    set_request_audit_info (p_channel_config, req_msg_header) =
    PRE
        p_channel_config : t_channel_config_idx_i &
        p_channel_config : t_channel_config_idx   &
        req_msg_header : t_msg_header_i &
        req_msg_header : t_msg_header &
        req_msg_header = c_msg_in_header &
        ReqAuditEntryInfo = c_msg_header_indet
    THEN
        /* Uses the message header context to set the current AuditEntryId / ActionTimestamp context.
           If header AuditEntryId is empty, uses the channel peer info data. */
        ReqAuditEntryInfo :: t_msg_header
    END
    ;

    set_no_req_audit_info (p_channel_config) =
    PRE
        p_channel_config : t_channel_config_idx_i &
        p_channel_config : t_channel_config_idx   &
        ReqAuditEntryInfo = c_msg_header_indet
    THEN
        /* Uses only channel peer info data only */
        ReqAuditEntryInfo :: t_msg_header
    END
    ;

    clear_audit_info =
    BEGIN
        ReqAuditEntryInfo := c_msg_header_indet
    END
    ;

    server_notify_session_create(p_channel_config, req_msg, resp_msg, session, oper_ret_code) =
    PRE
        p_channel_config : t_channel_config_idx_i &
        p_channel_config : t_channel_config_idx   &
        req_msg : t_msg_i &
        req_msg : t_msg &
        req_msg = c_msg_in &
        a_msg_in_type = e_msg_session_create_req &
        resp_msg : t_msg_i &
        resp_msg : t_msg &
        resp_msg = c_msg_out &
        a_msg_out_type = e_msg_session_create_resp &
        session : t_session_i &
        session : t_session &
        oper_ret_code : t_StatusCode_i &
        oper_ret_code : t_StatusCode
    THEN
        skip
    END;

    server_notify_session_activate(p_channel_config, req_msg, session, oper_ret_code) =
    PRE
        p_channel_config : t_channel_config_idx_i &
        p_channel_config : t_channel_config_idx   &
        req_msg : t_msg_i &
        req_msg : t_msg &
        req_msg = c_msg_in &
        a_msg_in_type = e_msg_session_activate_req &
        session : t_session_i &
        session : t_session &
        oper_ret_code : t_StatusCode_i &
        oper_ret_code : t_StatusCode
    THEN
        skip
    END;

    server_notify_session_closed(session, oper_ret_code) =
    PRE
        session : t_session_i &
        session : t_session &
        oper_ret_code : t_StatusCode_i &
        oper_ret_code : t_StatusCode
    THEN
        skip
    END

END
