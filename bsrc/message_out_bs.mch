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
    message_out_bs
SEES
    constants,
    constants_statuscodes_bs,
    message_in_bs,
    request_handle_bs

DEFINITIONS
    d_variables ==
    a_buffer_out_state,
    c_msg_out_header,
    c_msg_out,
    a_msg_out_type;

    d_typing ==
    a_buffer_out_state : t_buffer_out_state_i &
    c_msg_out_header : t_msg_header_i &
    c_msg_out : t_msg_i &
    a_msg_out_type : t_msg_type_i

ABSTRACT_VARIABLES
    d_variables

INVARIANT
    d_typing

INITIALISATION
    d_variables :( d_typing )

OPERATIONS

    nmsg_header <-- alloc_msg_header (p_is_request) =
    PRE
        c_msg_out_header = c_msg_header_indet &
        p_is_request : BOOL
    THEN
        CHOICE
            nmsg_header, c_msg_out_header
            :( nmsg_header : t_msg_header_i & nmsg_header = c_msg_out_header & c_msg_out_header : t_msg_header)
        OR
            nmsg_header :( nmsg_header = c_msg_header_indet & nmsg_header = c_msg_out_header)
        END
    END
    ;

    bres, msg_typ <-- is_valid_app_msg_out (msg) =
    PRE
        msg : t_msg_i
    THEN
        /* Validates message is not null and has a known type */
        IF msg : t_msg
        THEN
            CHOICE
                bres := TRUE ||
                msg_typ :( msg_typ : t_msg_type_i & msg_typ : t_msg_type)
            OR
                bres := FALSE ||
                msg_typ := c_msg_type_indet
            END
        ELSE
            bres := FALSE ||
            msg_typ := c_msg_type_indet
        END
    END
    ;

    /* Accept a message that was allocated by another part of the program */
    bless_msg_out (msg) =
    PRE
        msg : t_msg_i &
        msg : t_msg
    THEN
        c_msg_out := msg ||
        a_msg_out_type :: t_msg_type
    END
    ;

    /* Consider response message as deallocated since responsibility was forwarded to application or postponed */
    forget_resp_msg_out (msg_header, msg) =
    PRE
        msg_header : t_msg_header_i &
        msg_header = c_msg_out_header &
        msg : t_msg_i &
        msg = c_msg_out &
        c_msg_out : t_msg &
        c_msg_out_header : t_msg_header &
        a_msg_out_type : s_service_resps
    THEN
        c_msg_out := c_msg_indet ||
        a_msg_out_type := c_msg_type_indet ||
        c_msg_out_header := c_msg_header_indet
    END
    ;

    nmsg_header, nmsg <-- alloc_req_msg (msg_type) =
    PRE
        msg_type : t_msg_type_i &
        msg_type : t_msg_type &
        c_msg_out_header = c_msg_header_indet &
        c_msg_out = c_msg_indet
    THEN
        CHOICE
            nmsg_header, c_msg_out_header
            :( nmsg_header : t_msg_header_i & nmsg_header = c_msg_out_header & c_msg_out_header : t_msg_header) ||
            nmsg, c_msg_out :( nmsg : t_msg_i & nmsg = c_msg_out & c_msg_out : t_msg ) ||
            a_msg_out_type := msg_type
        OR
            nmsg_header :( nmsg_header = c_msg_header_indet & nmsg_header = c_msg_out_header) ||
            nmsg :( nmsg = c_msg_indet & nmsg = c_msg_out)
        END
    END
    ;

    nmsg_header, nmsg <-- alloc_resp_msg (msg_type) =
    PRE
        msg_type : t_msg_type_i &
        msg_type : t_msg_type &
        c_msg_out_header = c_msg_header_indet &
        c_msg_out = c_msg_indet &
        c_msg_in : t_msg
    THEN
        CHOICE
            nmsg_header, c_msg_out_header
            :( nmsg_header : t_msg_header_i & nmsg_header = c_msg_out_header & c_msg_out_header : t_msg_header) ||
            nmsg, c_msg_out :( nmsg : t_msg_i & nmsg = c_msg_out & c_msg_out : t_msg ) ||
            a_msg_out_type := msg_type
        OR
            nmsg_header :( nmsg_header = c_msg_header_indet & nmsg_header = c_msg_out_header) ||
            nmsg :( nmsg = c_msg_indet & nmsg = c_msg_out)
        END
    END
    ;

    dealloc_msg_header_out (msg_header) =
    PRE
        msg_header : t_msg_header_i &
        msg_header = c_msg_out_header &
        c_msg_out_header : t_msg_header
    THEN
        c_msg_out_header := c_msg_header_indet
    END
    ;

    dealloc_msg_out (msg) =
    PRE
        msg : t_msg_i &
        msg = c_msg_out &
        c_msg_out : t_msg &
        a_msg_out_type : t_msg_type
    THEN
        c_msg_out := c_msg_indet ||
        a_msg_out_type := c_msg_type_indet
    END
    ;

    sc, buffer <-- encode_msg (channel_cfg, header_type, msg_type, msg_header, msg) =
    PRE
        channel_cfg : t_channel_config_idx_i &
        a_buffer_out_state = c_buffer_out_state_indet &
        header_type : t_msg_header_type_i &
        header_type : t_msg_header_type &
        msg_type : t_msg_type_i &
        /* Note: msg_type might also become e_msg_service_fault_resp during encoding_msg in case of encoding failure */
        msg_type : {a_msg_out_type, e_msg_service_fault_resp} &
        a_msg_out_type : t_msg_type &
        msg_header : t_msg_header_i &
        msg_header = c_msg_out_header &
        c_msg_out_header : t_msg_header &
        msg : t_msg_i &
        msg = c_msg_out &
        c_msg_out : t_msg
    THEN
        CHOICE
            buffer :(buffer : t_byte_buffer_i & buffer = c_byte_buffer_indet) ||
            sc :(sc : t_StatusCode_i & sc :{e_sc_bad_request_too_large, e_sc_bad_response_too_large, e_sc_bad_out_of_memory})
        OR
            buffer :(buffer : t_byte_buffer_i & buffer : t_byte_buffer) ||
            a_buffer_out_state := e_buffer_out_msg_written ||
            sc := e_sc_ok
        END
    END
    ;

    bres <-- is_valid_buffer_out (buffer) =
    PRE
        buffer : t_byte_buffer_i
    THEN
        bres := bool(a_buffer_out_state = e_buffer_out_msg_written & buffer : t_byte_buffer)
    END
    ;

    bres <-- is_valid_msg_out (msg) =
    PRE
        msg : t_msg_i
    THEN
        bres := bool(msg = c_msg_out & c_msg_out : t_msg)
    END
    ;

    bres <-- is_valid_msg_out_header (msg_header) =
    PRE
        msg_header : t_msg_header_i
    THEN
        bres := bool(msg_header = c_msg_out_header & c_msg_out_header : t_msg_header)
    END
    ;

    msgtype <-- get_msg_out_type (msg) =
    PRE
        msg : t_msg_i &
        msg = c_msg_out &
        c_msg_out : t_msg
    THEN
        msgtype := a_msg_out_type
    END
    ;

    write_msg_out_header_session_token (msg_header, session_token) =
    PRE
        msg_header : t_msg_header_i &
        msg_header = c_msg_out_header &
        c_msg_out_header : t_msg_header &
        session_token : t_session_token_i &
        session_token : t_session_token &
        a_msg_out_type :
        {e_msg_session_activate_req,
            e_msg_session_close_req} \/ s_service_on_session_reqs
    THEN
        /* TODO: add relation on session token ? => need to retrieve after write ? */
        skip
    END
    ;

    client_write_msg_out_header_req_handle (msg_header, req_handle) =
    PRE
        msg_header : t_msg_header_i &
        msg_header = c_msg_out_header &
        c_msg_out_header : t_msg_header &
        req_handle : t_client_request_handle_i &
        req_handle : s_client_request_handle
    THEN
        skip
    END
    ;

    server_write_msg_out_header_req_handle (msg_header, req_handle) =
    PRE
        msg_header : t_msg_header_i &
        msg_header = c_msg_out_header &
        c_msg_out_header : t_msg_header &
        req_handle : t_server_request_handle_i
    THEN
        skip
    END
    ;

    write_msg_resp_header_service_status (msg_header, status_code) =
    PRE
        msg_header : t_msg_header_i &
        msg_header = c_msg_out_header &
        c_msg_out_header : t_msg_header &
        a_msg_out_type : s_service_resps &
        status_code : t_StatusCode_i &
        status_code : t_StatusCode
    THEN
        /* TODO: add relation on status ? => need to retrieve after write ? */
        skip
    END
    ;

    /* Operation to copy header content in response message
       in order to be provided to client user application */
    copy_msg_resp_header_into_msg_out (msg_header, msg) =
    PRE
        msg_header : t_msg_header_i &
        msg_header = c_msg_in_header &
        msg : t_msg_i &
        msg = c_msg_in &
        c_msg_out : t_msg &
        c_msg_out_header : t_msg_header &
        a_msg_out_type : s_service_resps
    THEN
        skip
    END

END
