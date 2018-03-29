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

 File Name            : service_mgr_1.h

 Date                 : 29/03/2018 14:46:10

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _service_mgr_1_h
#define _service_mgr_1_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
extern t_bool service_mgr_1__local_service_treatment_i;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void service_mgr_1__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void service_mgr_1__is_local_service_treatment(t_bool* const service_mgr_1__bres);
extern void service_mgr_1__set_local_service_treatment(void);
extern void service_mgr_1__unset_local_service_treatment(void);

#endif
