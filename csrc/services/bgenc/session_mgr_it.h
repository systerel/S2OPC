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

 File Name            : session_mgr_it.h

 Date                 : 29/03/2018 14:46:14

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _session_mgr_it_h
#define _session_mgr_it_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
extern t_entier4 session_mgr_it__session_i;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void session_mgr_it__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void session_mgr_it__continue_iter_session(t_bool* const session_mgr_it__p_continue,
                                                  constants__t_session_i* const session_mgr_it__p_session);
extern void session_mgr_it__init_iter_session(t_bool* const session_mgr_it__p_continue);

#endif
