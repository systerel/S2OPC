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

/*------------------------
   Exported Declarations
  ------------------------*/
#include "session_request_handle_bs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "sopc_assert.h"

/* Note: due to request handle generation on client side, request handle is unique regardless the session */
/* Same size of array than request handles array */
static constants__t_session_i client_requests[SOPC_MAX_PENDING_REQUESTS + 1];

/* Store number of pending requests remaining for session */
static uint32_t session_pending_requests_nb[SOPC_MAX_SESSIONS + 1];

/* Iterator variables */
static uint32_t it_current = 0;
static constants__t_session_i it_session = constants__c_session_indet;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void session_request_handle_bs__INITIALISATION(void)
{
    memset(client_requests, constants__c_session_indet,
           (SOPC_MAX_PENDING_REQUESTS + 1) * sizeof(constants__t_session_i));
    memset(session_pending_requests_nb, 0, (SOPC_MAX_SESSIONS + 1) * sizeof(uint32_t));
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void session_request_handle_bs__client_add_session_request_handle(
    const constants__t_session_i session_request_handle_bs__session,
    const constants__t_client_request_handle_i session_request_handle_bs__req_handle)
{
    SOPC_ASSERT(session_request_handle_bs__session != constants__c_session_indet);
    SOPC_ASSERT(session_request_handle_bs__req_handle != constants__c_client_request_handle_indet);
    // Request handle freshness is guaranteed by request_handle_bs,
    // in degraded cases an old session number could be overwritten here
    client_requests[session_request_handle_bs__req_handle] = session_request_handle_bs__session;
    session_pending_requests_nb[session_request_handle_bs__session]++;
}

void session_request_handle_bs__client_get_session_and_remove_request_handle(
    const constants__t_client_request_handle_i session_request_handle_bs__req_handle,
    constants__t_session_i* const session_request_handle_bs__session)
{
    // Note: validity of request handle is guaranteed by request_handle_bs
    *session_request_handle_bs__session = constants__c_session_indet;

    if (session_request_handle_bs__req_handle != constants__c_client_request_handle_indet)
    {
        if (client_requests[session_request_handle_bs__req_handle] != constants__c_session_indet)
        {
            *session_request_handle_bs__session = client_requests[session_request_handle_bs__req_handle];
            client_requests[session_request_handle_bs__req_handle] = constants__c_session_indet;

            SOPC_ASSERT(session_pending_requests_nb[*session_request_handle_bs__session] > 0);
            session_pending_requests_nb[*session_request_handle_bs__session]--;
        }
    }
}

void session_request_handle_bs__continue_and_remove_session_req_handle_it(
    t_bool* const session_request_handle_bs__continue,
    constants__t_client_request_handle_i* const session_request_handle_bs__req_handle)
{
    SOPC_ASSERT(it_session != constants__c_session_indet);
    SOPC_ASSERT(session_pending_requests_nb[it_session] > 0);
    *session_request_handle_bs__req_handle = constants__c_client_request_handle_indet;
    bool found = false;
    for (uint32_t idx = it_current; idx <= SOPC_MAX_PENDING_REQUESTS && !found; idx++)
    {
        if (client_requests[idx] == it_session)
        {
            client_requests[idx] = constants__c_session_indet;
            session_pending_requests_nb[it_session]--;

            found = true;
            *session_request_handle_bs__req_handle = idx;
            it_current = idx + 1;
        }
    }
    *session_request_handle_bs__continue = session_pending_requests_nb[it_session] > 0;
}

void session_request_handle_bs__init_session_req_handle_it(
    const constants__t_session_i session_request_handle_bs__session,
    t_bool* const session_request_handle_bs__continue)
{
    *session_request_handle_bs__continue = false;
    SOPC_ASSERT(session_request_handle_bs__session != constants__c_session_indet);
    if (session_pending_requests_nb[session_request_handle_bs__session] > 0)
    {
        *session_request_handle_bs__continue = true;
        it_current = 1;
        it_session = session_request_handle_bs__session;
    }
}
