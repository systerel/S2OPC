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
    request_handle_bs

SEES
    constants

ABSTRACT_VARIABLES
    s_client_request_handle,
    a_client_request_type,
    a_client_request_resp_type,
    a_client_request_channel,
    a_client_request_context

INVARIANT
    s_client_request_handle <: t_client_request_handle &
    /* Associate the request handle of a client request with its response */
    a_client_request_type : s_client_request_handle --> s_service_reqs &
    a_client_request_resp_type : s_client_request_handle --> s_service_resps &
    a_client_request_channel : s_client_request_handle --> t_channel &
    a_client_request_context : s_client_request_handle --> t_application_context_i

INITIALISATION
    s_client_request_handle :: {{}} ||
    a_client_request_type :: {{}} ||
    a_client_request_resp_type :: {{}} ||
    a_client_request_channel :: {{}} ||
    a_client_request_context :: {{}}

OPERATIONS

    request_handle <-- client_fresh_req_handle (req_typ, resp_typ, is_applicative, app_context) =
    PRE
        req_typ : t_msg_type_i &
        req_typ : s_service_reqs &
        resp_typ : t_msg_type_i &
        resp_typ : s_service_resps &
        is_applicative : BOOL &
        app_context : t_application_context_i
    THEN
        IF s_client_request_handle = t_client_request_handle
        THEN
            request_handle := c_client_request_handle_indet
        ELSE
            ANY l_request_handle WHERE
                l_request_handle : t_client_request_handle_i &
                l_request_handle : t_client_request_handle &
                l_request_handle /: s_client_request_handle
            THEN
                request_handle := l_request_handle ||
                s_client_request_handle := s_client_request_handle \/ {l_request_handle} ||
                a_client_request_type (l_request_handle) := req_typ ||
                a_client_request_resp_type (l_request_handle) := resp_typ ||
                IF is_applicative = TRUE
                THEN
                    a_client_request_context (l_request_handle) := app_context
                END
            END
        END
    END
    ;

    set_req_handle_channel (req_handle, channel) =
    PRE
        req_handle : t_client_request_handle_i &
        req_handle : s_client_request_handle &
        channel : t_channel_i &
        channel : t_channel
    THEN
        a_client_request_channel (req_handle) := channel
    END
    ;

    ret <-- is_valid_req_handle (req_handle) =
    PRE
        req_handle : t_client_request_handle_i
    THEN
        ret := bool (req_handle : s_client_request_handle)
    END
    ;

    channel <-- get_req_handle_channel (req_handle) =
    PRE
        req_handle : t_client_request_handle_i
    THEN
        IF req_handle /: s_client_request_handle
        THEN
            channel := c_channel_indet
        ELSE
            channel := a_client_request_channel (req_handle)
        END
    END
    ;

    is_applicative, app_context <-- get_req_handle_app_context (req_handle) =
    PRE
        req_handle : t_client_request_handle_i &
        req_handle : s_client_request_handle
    THEN
        IF req_handle : dom(a_client_request_context)
        THEN
            is_applicative := TRUE ||
            app_context := a_client_request_context (req_handle)
        ELSE
            is_applicative := FALSE ||
            app_context := c_no_application_context
        END
    END;

    resp_typ <-- get_req_handle_resp_typ (req_handle) =
    PRE
        req_handle : t_client_request_handle_i &
        req_handle : s_client_request_handle
    THEN
        resp_typ :(resp_typ : t_msg_type_i & a_client_request_resp_type (req_handle) = resp_typ)
    END;

    req_typ <-- get_req_handle_req_typ (req_handle) =
    PRE
        req_handle : t_client_request_handle_i &
        req_handle : s_client_request_handle
    THEN
        req_typ :(req_typ : t_msg_type_i & a_client_request_type (req_handle) = req_typ)
    END;

    client_remove_req_handle (req_handle) =
    PRE
        req_handle : t_client_request_handle_i &
        req_handle : s_client_request_handle
    THEN
        s_client_request_handle := s_client_request_handle - {req_handle} ||
        a_client_request_type := {req_handle} <<| a_client_request_type ||
        a_client_request_resp_type := {req_handle} <<| a_client_request_resp_type ||
        a_client_request_context := {req_handle} <<| a_client_request_context ||
        a_client_request_channel := {req_handle} <<| a_client_request_channel
    END
    ;

    ret <-- client_validate_response_request_handle (channel, req_handle, resp_typ) =
    PRE
        channel : t_channel_i &
        channel : t_channel &
        req_handle : t_client_request_handle_i &
        resp_typ : t_msg_type_i &
        resp_typ : s_service_resps
    THEN
        ret := bool(req_handle : s_client_request_handle &
            a_client_request_resp_type (req_handle) = resp_typ &
            a_client_request_channel (req_handle) = channel)
    END
    ;

    /* Operation used to provide request handle to SC layer
    (instead of requestId which is not used for client) */
    request_id <-- client_req_handle_to_request_id (req_handle) =
    PRE
        req_handle : t_client_request_handle_i
    THEN
        request_id :: t_request_context_i
    END
    ;

    /* Operation used to retrieve request handle from SC layer
    (stored in place of requestId which is not used for client) */
    request_handle <-- client_request_id_to_req_handle (request_id) =
    PRE
        request_id : t_request_context_i
    THEN
        request_handle :: t_client_request_handle_i
    END

END
