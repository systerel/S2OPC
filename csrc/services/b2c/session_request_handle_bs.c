/*
 *  Copyright (C) 2017 Systerel and others.
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

/*------------------------
   Exported Declarations
  ------------------------*/
#include "session_request_handle_bs.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sopc_toolkit_constants.h"

/* Note: due to request handle generation on client side, request handle is unique regardless the session */
/* Same size of array than request handles array */
static constants__t_session_i client_requests[SOPC_MAX_PENDING_REQUESTS];

/* Store number of pending requests remaining for session */
static uint32_t session_pending_requests_nb[SOPC_MAX_SESSIONS];

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void session_request_handle_bs__INITIALISATION()
{
    memset(client_requests, constants__c_session_indet, SOPC_MAX_PENDING_REQUESTS * sizeof(constants__t_session_i));
    memset(session_pending_requests_nb, 0, SOPC_MAX_SESSIONS * sizeof(uint32_t));
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void session_request_handle_bs__client_add_session_request_handle(
    const constants__t_session_i session_request_handle_bs__session,
    const constants__t_request_handle_i session_request_handle_bs__req_handle)
{
    assert(session_request_handle_bs__session != constants__c_session_indet);
    assert(session_request_handle_bs__req_handle != constants__c_request_handle_indet &&
           session_request_handle_bs__req_handle >= 0);
    // It shall be a fresh request handle which means it cannot be assigned to another session
    assert(client_requests[session_request_handle_bs__req_handle] == constants__c_session_indet);
    client_requests[session_request_handle_bs__req_handle] = session_request_handle_bs__session;
    session_pending_requests_nb[session_request_handle_bs__session]++;
}

void session_request_handle_bs__client_get_session_and_remove_request_handle(
    const constants__t_request_handle_i session_request_handle_bs__req_handle,
    constants__t_session_i* const session_request_handle_bs__session)
{
    assert(session_request_handle_bs__req_handle != constants__c_request_handle_indet);
    assert(client_requests[session_request_handle_bs__req_handle] != constants__c_session_indet);

    *session_request_handle_bs__session = client_requests[session_request_handle_bs__req_handle];
    client_requests[session_request_handle_bs__req_handle] = constants__c_session_indet;

    assert(session_pending_requests_nb[*session_request_handle_bs__session] > 0);
    session_pending_requests_nb[*session_request_handle_bs__session]--;
}

void session_request_handle_bs__client_remove_all_request_handles(
    const constants__t_session_i session_request_handle_bs__session)
{
    assert(session_request_handle_bs__session != constants__c_session_indet);
    for (uint32_t idx = 0;
         idx < SOPC_MAX_PENDING_REQUESTS && session_pending_requests_nb[session_request_handle_bs__session] > 0; idx++)
    {
        if (client_requests[idx] == session_request_handle_bs__session)
        {
            client_requests[idx] = constants__c_session_indet;
            session_pending_requests_nb[session_request_handle_bs__session]--;
        }
    }
}
