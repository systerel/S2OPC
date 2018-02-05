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

 File Name            : service_read_it.c

 Date                 : 05/02/2018 16:15:21

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "service_read_it.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
t_entier4 service_read_it__rreqs_i;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void service_read_it__INITIALISATION(void)
{
    service_read_it__rreqs_i = 0;
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void service_read_it__init_iter_write_request(const t_entier4 service_read_it__nb_req,
                                              t_bool* const service_read_it__continue)
{
    service_read_it__rreqs_i = service_read_it__nb_req;
    *service_read_it__continue = (0 < service_read_it__nb_req);
}

void service_read_it__continue_iter_write_request(t_bool* const service_read_it__continue,
                                                  constants__t_ReadValue_i* const service_read_it__rvi)
{
    constants__read_cast_t_ReadValue(service_read_it__rreqs_i, service_read_it__rvi);
    service_read_it__rreqs_i = service_read_it__rreqs_i - 1;
    *service_read_it__continue = (0 < service_read_it__rreqs_i);
}
