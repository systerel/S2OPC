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

 File Name            : service_browse_seq_it.h

 Date                 : 29/03/2018 14:46:10

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _service_browse_seq_it_h
#define _service_browse_seq_it_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "address_space.h"
#include "constants.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
extern t_entier4 service_browse_seq_it__RefIndex;
extern t_entier4 service_browse_seq_it__RefIndexEnd;
extern t_entier4 service_browse_seq_it__bri_i;
extern t_entier4 service_browse_seq_it__bvi_i;
extern t_entier4 service_browse_seq_it__nb_bri;
extern t_entier4 service_browse_seq_it__nb_bvi;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void service_browse_seq_it__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void service_browse_seq_it__continue_iter_browse_request(
    t_bool* const service_browse_seq_it__p_continue,
    constants__t_BrowseValue_i* const service_browse_seq_it__p_bvi);
extern void service_browse_seq_it__continue_iter_browse_result(
    t_bool* const service_browse_seq_it__p_continue,
    constants__t_BrowseResult_i* const service_browse_seq_it__p_bri);
extern void service_browse_seq_it__continue_iter_reference(
    t_bool* const service_browse_seq_it__p_continue,
    constants__t_Reference_i* const service_browse_seq_it__p_ref);
extern void service_browse_seq_it__init_iter_browse_request(const t_entier4 service_browse_seq_it__p_nb_req,
                                                            t_bool* const service_browse_seq_it__p_continue);
extern void service_browse_seq_it__init_iter_browse_result(const t_entier4 service_browse_seq_it__p_nb_bri,
                                                           t_bool* const service_browse_seq_it__p_continue);
extern void service_browse_seq_it__init_iter_reference(const constants__t_Node_i service_browse_seq_it__p_node,
                                                       t_bool* const service_browse_seq_it__p_continue);

#endif
