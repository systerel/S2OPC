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

 File Name            : channel_mgr_it.h

 Date                 : 15/02/2018 15:01:11

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _channel_mgr_it_h
#define _channel_mgr_it_h

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
extern t_entier4 channel_mgr_it__channel_i;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void channel_mgr_it__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void channel_mgr_it__continue_iter_channel(t_bool* const channel_mgr_it__p_continue,
                                                  constants__t_channel_i* const channel_mgr_it__p_channel);
extern void channel_mgr_it__init_iter_channel(t_bool* const channel_mgr_it__p_continue);

#endif
