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

 File Name            : address_space_it.h

 Date                 : 05/02/2018 16:15:16

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _address_space_it_h
#define _address_space_it_h

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
extern t_entier4 address_space_it__wreqs_i;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void address_space_it__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void address_space_it__continue_iter_write_request(t_bool* const address_space_it__continue,
                                                          constants__t_WriteValue_i* const address_space_it__wvi);
extern void address_space_it__init_iter_write_request(const t_entier4 address_space_it__nb_req,
                                                      t_bool* const address_space_it__continue);

#endif
