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

 File Name            : session_core_it.c

 Date                 : 29/03/2018 14:46:13

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "session_core_it.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
t_entier4 session_core_it__session_i;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void session_core_it__INITIALISATION(void)
{
    session_core_it__session_i = 0;
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void session_core_it__init_iter_session(t_bool* const session_core_it__p_continue)
{
    constants__get_card_t_session(&session_core_it__session_i);
    *session_core_it__p_continue = (1 <= session_core_it__session_i);
}

void session_core_it__continue_iter_session(t_bool* const session_core_it__p_continue,
                                            constants__t_session_i* const session_core_it__p_session)
{
    constants__get_cast_t_session(session_core_it__session_i, session_core_it__p_session);
    session_core_it__session_i = session_core_it__session_i - 1;
    *session_core_it__p_continue = (1 <= session_core_it__session_i);
}
