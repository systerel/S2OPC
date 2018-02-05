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

 File Name            : session_async_bs.h

 Date                 : 05/02/2018 16:15:28

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _session_async_bs_h
#define _session_async_bs_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void session_async_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void session_async_bs__add_session_to_activate(const constants__t_session_i session_async_bs__session,
                                                      const constants__t_user_i session_async_bs__user,
                                                      t_bool* const session_async_bs__ret);
extern void session_async_bs__get_and_remove_session_user_to_activate(
    const constants__t_session_i session_async_bs__session,
    constants__t_user_i* const session_async_bs__user);

#endif
