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
