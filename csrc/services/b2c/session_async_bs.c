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

#include "session_async_bs.h"

#include "sopc_services_api.h"
#include "sopc_toolkit_config.h"
#include "util_b2c.h"

#include <assert.h>
#include <string.h>

static bool session_to_create[SOPC_MAX_SESSIONS + 1];
static constants__t_channel_config_idx_i session_to_create_idx[SOPC_MAX_SESSIONS + 1];
static bool session_to_activate[SOPC_MAX_SESSIONS + 1];
static constants__t_user_i session_to_activate_user[SOPC_MAX_SESSIONS + 1];

void session_async_bs__INITIALISATION(void)
{
    assert(SOPC_MAX_SESSIONS + 1 <= SIZE_MAX / sizeof(bool));
    assert(SOPC_MAX_SESSIONS + 1 <= SIZE_MAX / sizeof(constants__t_channel_config_idx_i));
    assert(SOPC_MAX_SESSIONS + 1 <= SIZE_MAX / sizeof(constants__t_user_i));

    memset(session_to_create, (int) false, sizeof(bool) * (SOPC_MAX_SESSIONS + 1));
    memset(session_to_create_idx, (int) constants__c_channel_config_idx_indet,
           sizeof(constants__t_channel_config_idx_i) * (SOPC_MAX_SESSIONS + 1));
    memset(session_to_activate, (int) false, sizeof(bool) * (SOPC_MAX_SESSIONS + 1));
    memset(session_to_activate_user, (int) constants__c_user_indet,
           sizeof(constants__t_user_i) * (SOPC_MAX_SESSIONS + 1));
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void session_async_bs__add_session_to_activate(const constants__t_session_i session_async_bs__session,
                                               const constants__t_user_i session_async_bs__user,
                                               t_bool* const session_async_bs__ret)
{
    if (session_async_bs__session != constants__c_session_indet &&
        false == session_to_activate[session_async_bs__session])
    {
        session_to_activate_user[session_async_bs__session] = session_async_bs__user;
        session_to_activate[session_async_bs__session] = true;
        *session_async_bs__ret = true;
    }
    else
    {
        *session_async_bs__ret = false;
    }
}

void session_async_bs__add_session_to_create(
    const constants__t_session_i session_async_bs__session,
    const constants__t_channel_config_idx_i session_async_bs__channel_config_idx,
    t_bool* const session_async_bs__ret)
{
    bool unique_session_in_channel_config = true;
    if (session_async_bs__session != constants__c_session_indet &&
        false == session_to_create[session_async_bs__session])
    {
        // Check a session in creation is not already recorded for same channel config index
        for (constants__t_session_i session = 1;
             session <= SOPC_MAX_SESSIONS && true == unique_session_in_channel_config; session++)
        {
            if (true == session_to_create[session] &&
                session_async_bs__channel_config_idx == session_to_create_idx[session])
            {
                // A session is already waiting on this channel config => not possible with current B model
                unique_session_in_channel_config = false;
            }
        }
        if (unique_session_in_channel_config != false)
        {
            session_to_create_idx[session_async_bs__session] = session_async_bs__channel_config_idx;
            session_to_create[session_async_bs__session] = true;
            *session_async_bs__ret = true;
        }
        else
        {
            *session_async_bs__ret = false;
        }
    }
    else
    {
        *session_async_bs__ret = false;
    }
}

void session_async_bs__get_and_remove_session_user_to_activate(const constants__t_session_i session_async_bs__session,
                                                               constants__t_user_i* const session_async_bs__user)
{
    if (session_async_bs__session != constants__c_session_indet &&
        true == session_to_activate[session_async_bs__session])
    {
        *session_async_bs__user = session_to_activate_user[session_async_bs__session];
        session_to_activate_user[session_async_bs__session] = constants__c_user_indet;
        session_to_activate[session_async_bs__session] = false;
    }
    else
    {
        *session_async_bs__user = constants__c_user_indet;
    }
}

void session_async_bs__get_and_remove_session_to_create(
    const constants__t_channel_config_idx_i session_async_bs__channel_config_idx,
    constants__t_session_i* const session_async_bs__session)
{
    bool stop = false;
    *session_async_bs__session = constants__c_session_indet;
    if (session_async_bs__channel_config_idx != constants__c_channel_config_idx_indet)
    {
        for (constants__t_session_i session = 1; session <= SOPC_MAX_SESSIONS && false == stop; session++)
        {
            if (true == session_to_create[session] &&
                session_async_bs__channel_config_idx == session_to_create_idx[session])
            {
                *session_async_bs__session = session;
                stop = true;
            }
        }
    }
}

void session_async_bs__client_gen_activate_orphaned_session_internal_event(
    const constants__t_session_i session_async_bs__session,
    const constants__t_channel_config_idx_i session_async_bs__channel_config_idx)
{
    SOPC_Services_EnqueueEvent(SE_TO_SE_ACTIVATE_ORPHANED_SESSION, (uint32_t) session_async_bs__session, NULL,
                               (uint32_t) session_async_bs__channel_config_idx);
}

void session_async_bs__client_gen_activate_user_session_internal_event(
    const constants__t_session_i session_async_bs__session,
    const constants__t_user_i session_async_bs__user)
{
    SOPC_Services_EnqueueEvent(SE_TO_SE_ACTIVATE_SESSION, (uint32_t) session_async_bs__session, NULL,
                               (uint32_t) session_async_bs__user);
}

void session_async_bs__client_gen_create_session_internal_event(
    const constants__t_session_i session_async_bs__session,
    const constants__t_channel_config_idx_i session_async_bs__channel_config_idx)
{
    SOPC_Services_EnqueueEvent(SE_TO_SE_CREATE_SESSION, (uint32_t) session_async_bs__session, NULL,
                               (uint32_t) session_async_bs__channel_config_idx);
}
