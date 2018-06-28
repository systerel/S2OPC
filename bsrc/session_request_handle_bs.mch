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
    session_request_handle_bs
SEES
    constants,
    session_core,
    request_handle_bs

ABSTRACT_VARIABLES
    a_cli_pending_requests

INVARIANT
    a_cli_pending_requests : t_client_request_handle +-> t_session

INITIALISATION
    a_cli_pending_requests :: {}

OPERATIONS

    client_add_session_request_handle (session, req_handle) =
    PRE
        session : t_session_i &
        session : s_session &
        req_handle : t_client_request_handle_i &
        req_handle : s_client_request_handle
    THEN
        /* req_handle not present in a_cli_pending_requests by construction (fresh requ_handle provided) */
        a_cli_pending_requests := a_cli_pending_requests <+ {req_handle |-> session}
    END
    ;

    session <-- client_get_session_and_remove_request_handle (req_handle) =
    PRE
        req_handle : t_client_request_handle_i
    THEN
        IF req_handle : dom(a_cli_pending_requests)
        THEN
            session :(session : t_session_i & session = a_cli_pending_requests (req_handle)) ||
            a_cli_pending_requests := a_cli_pending_requests - {req_handle |-> session}
        ELSE
            session := c_session_indet
        END
    END
    ;

    client_remove_all_request_handles (session) =
    PRE
        session : t_session_i &
        session : s_session
    THEN
        a_cli_pending_requests := a_cli_pending_requests |>> {session}
    END

END
