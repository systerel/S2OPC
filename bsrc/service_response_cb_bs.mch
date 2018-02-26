/*
 *  Copyright (C) 2018 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

MACHINE
    service_response_cb_bs
    
SEES
    constants,
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
    
    cli_request_timeout (req_typ, app_context) =
    PRE
        req_typ : t_msg_type_i &
        req_typ : s_service_reqs &
        app_context : t_application_context_i
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
    
    srv_write_notification (write_request_pointer, write_status) =
    PRE
        write_request_pointer : t_WriteValuePointer_i &
        write_status : t_StatusCode_i &
        write_status : t_StatusCode
    THEN
        skip
    END

END
