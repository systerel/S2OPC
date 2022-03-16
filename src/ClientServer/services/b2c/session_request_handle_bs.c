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

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

/* Note: due to request handle generation on client side, request handle is unique regardless the session */
/* Same size of array than request handles array */
static constants__t_session_i client_requests[SOPC_MAX_PENDING_REQUESTS + 1];

/* Store number of pending requests remaining for session */
static uint32_t session_pending_requests_nb[SOPC_MAX_SESSIONS + 1];

/*@ predicate Replaces{K,L}(constants__t_session_i* a, int n, constants__t_session_i val1, constants__t_session_i val2)
  = \forall integer i; 1 <= i < n ==> \let ai = \at(a[i], K); \let bi = \at(a[i], L); (ai == val1 ==> bi == val2) &&
  (ai != val1 ==> bi == ai);
 */

/*@ predicate Unchanged{K, L}(constants__t_session_i* a, int m, int n) =
    \forall integer k; m <= k < n ==> \at(a[k], K) == \at(a[k], L);
 */

/*@ axiomatic CountSessionHandle
  @ {
  @     logic integer count_handle(constants__t_session_i* tab, integer start, integer n, constants__t_session_i val);
  @
  @     axiom count_zero:
  @         \forall constants__t_session_i* tab, integer start, integer n, constants__t_session_i val;
  n <= start ==> count_handle(tab, start, n, val) == 0;
  @
  @     axiom count_hit:
  @         \forall constants__t_session_i* tab, integer start, integer n, constants__t_session_i val;
  n >= start && tab[n] == val ==> count_handle(tab, start, n, val) + 1 == count_handle(tab, start, n+1, val);
  @
  @     axiom count_miss:
  @         \forall constants__t_session_i* tab, integer start, integer n, constants__t_session_i val;
  n >= start && tab[n] != val ==> count_handle(tab, start, n, val) == count_handle(tab, start, n+1, val);
  @ }
 */

/*@ axiomatic HandleProperties
  @ {
  @     axiom no_handles:
  @         \forall constants__t_session_i val; count_handle((constants__t_session_i*) client_requests, 1,
  SOPC_MAX_PENDING_REQUESTS + 1, val) == 0 <==> session_pending_requests_nb[val] == 0;
  @
  @     axiom no_more_handles{L1, L2} :
  @         \forall constants__t_session_i* tab, constants__t_session_i val, integer i;
  \at(session_pending_requests_nb[val], L2) == 0 ==> Replaces{L1, L2}(tab, (int) i, val, (constants__t_session_i)
  constants__c_session_indet);
  @
  @ }
 */

// Global invariant not supported yet, replaced with previous axiom
/*@ global invariant NumberOfHandle: \forall constants__t_session_i val;
  count_handle((constants__t_session_i*)client_requests, 1, SOPC_MAX_PENDING_REQUESTS + 1, val) <==>
  session_pending_requests_nb[val]; */

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
/*@ requires \valid(client_requests+(session_request_handle_bs__req_handle));
  @ requires \valid(session_pending_requests_nb + (session_request_handle_bs__session));
  @ requires 0 <= session_request_handle_bs__session < SOPC_MAX_SESSIONS + 1;
  @ requires 0 <= session_request_handle_bs__req_handle < SOPC_MAX_PENDING_REQUESTS + 1;
  @ requires 0 <= session_pending_requests_nb[session_request_handle_bs__session] < SOPC_MAX_PENDING_REQUESTS;
  @ // assert requirements, which are supposedly proven by B model
  @ requires session_request_handle_bs__session != constants__c_session_indet;
  @ requires session_request_handle_bs__req_handle != constants__c_client_request_handle_indet;
  @ requires client_requests[session_request_handle_bs__req_handle] == constants__c_session_indet;
  @
  @ assigns client_requests[session_request_handle_bs__req_handle];
  @ assigns session_pending_requests_nb[session_request_handle_bs__session];
  @
  @ ensures client_requests[session_request_handle_bs__req_handle] == session_request_handle_bs__session;
  @ ensures session_pending_requests_nb[session_request_handle_bs__session] ==
  (1+\old(session_pending_requests_nb[session_request_handle_bs__session]));
 */
void session_request_handle_bs__client_add_session_request_handle(
    const constants__t_session_i session_request_handle_bs__session,
    const constants__t_client_request_handle_i session_request_handle_bs__req_handle)
{
    assert(session_request_handle_bs__session != constants__c_session_indet);
    assert(session_request_handle_bs__req_handle != constants__c_client_request_handle_indet);
    // Request handle freshness is guaranteed by request_handle_bs,
    // in degraded cases an old session number could be overwritten here
    assert(client_requests[session_request_handle_bs__req_handle] == constants__c_session_indet);
    client_requests[session_request_handle_bs__req_handle] = session_request_handle_bs__session;
    session_pending_requests_nb[session_request_handle_bs__session]++;
}

/*@ requires \valid(client_requests+(session_request_handle_bs__req_handle));
  @ requires \valid(session_request_handle_bs__session);
  @ requires 0 <= client_requests[session_request_handle_bs__req_handle] < SOPC_MAX_SESSIONS + 1;
  @ requires 0 <= session_request_handle_bs__req_handle < SOPC_MAX_PENDING_REQUESTS + 1;
  @ requires 0 < session_pending_requests_nb[client_requests[session_request_handle_bs__req_handle]] <=
  SOPC_MAX_PENDING_REQUESTS;
  @ // assert requirements, which are supposedly proven by B model
  @ requires session_request_handle_bs__req_handle != constants__c_client_request_handle_indet;
  @ requires session_pending_requests_nb[client_requests[session_request_handle_bs__req_handle]] > 0;
  @
  @ assigns *session_request_handle_bs__session;
  @ assigns client_requests[session_request_handle_bs__req_handle];
  @ assigns session_pending_requests_nb[client_requests[session_request_handle_bs__req_handle]];
  @
  @ ensures *session_request_handle_bs__session == \old(client_requests[session_request_handle_bs__req_handle]);
  @ ensures client_requests[session_request_handle_bs__req_handle] == constants__c_session_indet;
  @ ensures session_pending_requests_nb[*session_request_handle_bs__session] ==
  \at(session_pending_requests_nb[\at(*session_request_handle_bs__session, Post)], Pre) - 1;
 */
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

            assert(session_pending_requests_nb[*session_request_handle_bs__session] > 0);
            session_pending_requests_nb[*session_request_handle_bs__session]--;
        }
    }
}

/*@ requires \valid(session_pending_requests_nb+(session_request_handle_bs__session));
  @ requires \valid(client_requests+(0 .. SOPC_MAX_PENDING_REQUESTS));
  @ requires session_request_handle_bs__session != 0;
  @
  @ assigns client_requests[1 .. SOPC_MAX_PENDING_REQUESTS];
  @ assigns session_pending_requests_nb[session_request_handle_bs__session];
  @
  @ ensures Replaces{Pre, Here}((constants__t_session_i*) client_requests, (int) (SOPC_MAX_PENDING_REQUESTS + 1),
  session_request_handle_bs__session, (constants__t_session_i) constants__c_session_indet);
  @ ensures \forall integer x; 1 <= x < (SOPC_MAX_PENDING_REQUESTS + 1) ==> client_requests[x] !=
  session_request_handle_bs__session;
  @ ensures \forall integer x; \old(client_requests[x]) == session_request_handle_bs__session ==>
  client_requests[x] == constants__c_session_indet;
  @ ensures session_pending_requests_nb[session_request_handle_bs__session] == 0;

 */

void session_request_handle_bs__client_remove_all_request_handles(
    const constants__t_session_i session_request_handle_bs__session)
{
    assert(session_request_handle_bs__session != constants__c_session_indet);
/*@ loop invariant 1 <= idx <= SOPC_MAX_PENDING_REQUESTS + 1;
      @ loop invariant session_pending_requests_nb[session_request_handle_bs__session] >= 0;
      @ loop invariant replaced: Replaces{Pre, Here}((constants__t_session_i*) client_requests, (int) idx,
      session_request_handle_bs__session, (constants__t_session_i) constants__c_session_indet);
      @ loop invariant unchanged: Unchanged{Pre, Here}((constants__t_session_i*) client_requests, (int) idx,
      (int) (SOPC_MAX_PENDING_REQUESTS + 1));
      @ loop invariant
      \at(count_handle((constants__t_session_i*)client_requests, 1, idx, session_request_handle_bs__session), LoopEntry)
      == \at(session_pending_requests_nb[session_request_handle_bs__session], LoopEntry) -
      \at(session_pending_requests_nb[session_request_handle_bs__session], LoopCurrent);
      @ loop invariant session_pending_requests_nb[session_request_handle_bs__session] == 0 ==>
      count_handle((constants__t_session_i*)client_requests, (int)idx, (int)SOPC_MAX_PENDING_REQUESTS + 1,
      session_request_handle_bs__session) == 0;
      @
      @ loop assigns idx;
      @ loop assigns client_requests[1 .. SOPC_MAX_PENDING_REQUESTS];
      @ loop assigns session_pending_requests_nb[session_request_handle_bs__session];
      @ loop variant ((SOPC_MAX_PENDING_REQUESTS + 1) - idx) *
      session_pending_requests_nb[session_request_handle_bs__session];
     */
    for (uint32_t idx = 1;
         idx <= SOPC_MAX_PENDING_REQUESTS && session_pending_requests_nb[session_request_handle_bs__session] > 0; idx++)
    {
        if (client_requests[idx] == session_request_handle_bs__session)
        {
            client_requests[idx] = constants__c_session_indet;
            session_pending_requests_nb[session_request_handle_bs__session]--;
        }
    }
}
