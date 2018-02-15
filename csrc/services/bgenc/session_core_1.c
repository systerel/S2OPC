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

/******************************************************************************

 File Name            : session_core_1.c

 Date                 : 15/02/2018 15:01:15

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "session_core_1.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void session_core_1__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void session_core_1__l_set_session_state(const constants__t_session_i session_core_1__p_session,
                                         const constants__t_sessionState session_core_1__p_state)
{
    {
        constants__t_sessionState session_core_1__l_prec_state;

        session_core_2__get_session_state(session_core_1__p_session, &session_core_1__l_prec_state);
        session_core_2__set_session_state_1(session_core_1__p_session, session_core_1__p_state);
        session_core_bs__notify_set_session_state(session_core_1__p_session, session_core_1__l_prec_state,
                                                  session_core_1__p_state);
    }
}

void session_core_1__delete_session(const constants__t_session_i session_core_1__session)
{
    session_core_2__remove_session(session_core_1__session);
    session_core_2__reset_session_channel(session_core_1__session);
    session_core_1__l_set_session_state(session_core_1__session, constants__e_session_closed);
    session_core_2__reset_session_orphaned(session_core_1__session);
    session_core_bs__delete_session_token(session_core_1__session);
}

void session_core_1__init_new_session(constants__t_session_i* const session_core_1__p_session)
{
    {
        t_bool session_core_1__l_is_session;
        t_bool session_core_1__l_continue;

        *session_core_1__p_session = constants__c_session_indet;
        session_core_1__l_is_session = true;
        session_core_1_it__init_iter_session(&session_core_1__l_continue);
        while ((session_core_1__l_continue == true) && (session_core_1__l_is_session == true))
        {
            session_core_1_it__continue_iter_session(&session_core_1__l_continue, session_core_1__p_session);
            session_core_2__is_valid_session(*session_core_1__p_session, &session_core_1__l_is_session);
        }
        if (session_core_1__l_is_session == true)
        {
            *session_core_1__p_session = constants__c_session_indet;
        }
        else
        {
            session_core_2__add_session(*session_core_1__p_session);
            session_core_1__l_set_session_state(*session_core_1__p_session, constants__e_session_init);
        }
    }
}

void session_core_1__create_session(const constants__t_session_i session_core_1__session,
                                    const constants__t_channel_i session_core_1__channel,
                                    const constants__t_sessionState session_core_1__state)
{
    session_core_2__set_session_channel(session_core_1__session, session_core_1__channel);
    session_core_1__l_set_session_state(session_core_1__session, session_core_1__state);
}

void session_core_1__create_session_failure(const constants__t_session_i session_core_1__session)
{
    session_core_2__reset_session_channel(session_core_1__session);
    session_core_1__l_set_session_state(session_core_1__session, constants__e_session_init);
}

void session_core_1__set_session_state(const constants__t_session_i session_core_1__session,
                                       const constants__t_sessionState session_core_1__state)
{
    session_core_1__l_set_session_state(session_core_1__session, session_core_1__state);
}

void session_core_1__set_session_state_closed(const constants__t_session_i session_core_1__session,
                                              const t_bool session_core_1__is_client)
{
    {
        constants__t_sessionState session_core_1__l_prec_state;

        session_core_2__remove_session(session_core_1__session);
        session_core_2__reset_session_channel(session_core_1__session);
        session_core_2__reset_session_orphaned(session_core_1__session);
        session_core_2__get_session_state(session_core_1__session, &session_core_1__l_prec_state);
        session_core_bs__prepare_close_session(session_core_1__session, session_core_1__l_prec_state,
                                               session_core_1__is_client);
        session_core_1__l_set_session_state(session_core_1__session, constants__e_session_closed);
        session_core_bs__delete_session_application_context(session_core_1__session);
        session_core_bs__delete_session_token(session_core_1__session);
    }
}

void session_core_1__set_session_orphaned(const constants__t_session_i session_core_1__session,
                                          const constants__t_channel_config_idx_i session_core_1__channel_config_idx)
{
    {
        t_bool session_core_1__l_bool;

        session_core_2__reset_session_channel(session_core_1__session);
        constants__is_t_channel_config_idx(session_core_1__channel_config_idx, &session_core_1__l_bool);
        if (session_core_1__l_bool == true)
        {
            session_core_2__set_session_orphaned_1(session_core_1__session, session_core_1__channel_config_idx);
        }
    }
}
