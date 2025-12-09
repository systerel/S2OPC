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
    a_cli_pending_requests,

    a_session_request_handle_it,
    a_session_request_handle_to_iterate,
    a_session_request_handle_iterated

INVARIANT
    a_cli_pending_requests : t_client_request_handle +-> t_session &

    /* Iterator used to clear the request handles associated to a session */
    a_session_request_handle_it : t_session_i &
    a_session_request_handle_to_iterate <: t_client_request_handle &
    a_session_request_handle_iterated <: t_client_request_handle /* iterated will be removed (during iteration) */

INITIALISATION
    a_cli_pending_requests :: {} ||
    a_session_request_handle_it := c_session_indet ||
    a_session_request_handle_to_iterate :: {} ||
    a_session_request_handle_iterated:: {}

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

    /* Iterator on request handle for a given session */

    continue <-- init_session_req_handle_it (session) =
    PRE
        session : t_session_i &
        session : s_session
    THEN
        continue,
        a_session_request_handle_it,
        a_session_request_handle_to_iterate,
        a_session_request_handle_iterated
        :(continue : BOOL &
          a_session_request_handle_it = session &
          a_session_request_handle_to_iterate = a_cli_pending_requests~[{session}] &
          a_session_request_handle_iterated = {} &
          continue = bool(a_session_request_handle_to_iterate /= {})
        )
    END
    ;

    continue, req_handle <-- continue_and_remove_session_req_handle_it =
    PRE
        a_session_request_handle_to_iterate /= {}
    THEN
        ANY l_req_handle WHERE
            l_req_handle : t_client_request_handle_i &
            l_req_handle : a_session_request_handle_to_iterate
        THEN
            continue := bool(a_session_request_handle_to_iterate /= {l_req_handle}) ||
            req_handle := l_req_handle ||
            a_session_request_handle_to_iterate := a_session_request_handle_to_iterate - {l_req_handle} ||
            a_session_request_handle_iterated := a_session_request_handle_iterated \/ {l_req_handle} ||
            a_cli_pending_requests := a_cli_pending_requests - {l_req_handle |-> a_session_request_handle_it}
        END
    END

END
