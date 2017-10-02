/*
 *  Copyright (C) 2017 Systerel and others.
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

 File Name            : msg_read_request.h

 Date                 : 29/09/2017 10:51:57

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _msg_read_request_h
#define _msg_read_request_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*-----------------
   IMPORTS Clause
  -----------------*/
#include "msg_read_request_bs.h"

/*--------------
   SEES Clause
  --------------*/
#include "address_space.h"
#include "constants.h"
#include "message_in_bs.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
extern t_entier4 msg_read_request__nb_ReadValue;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void msg_read_request__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void msg_read_request__check_ReadRequest(
   const constants__t_msg_i msg_read_request__p_msg,
   t_bool * const msg_read_request__p_read_ok,
   t_entier4 * const msg_read_request__p_nb_ReadValue);
extern void msg_read_request__get_nb_ReadValue(
   t_entier4 * const msg_read_request__p_nb_ReadValue);
extern void msg_read_request__getall_ReadValue_NodeId_AttributeId(
   const constants__t_msg_i msg_read_request__p_msg,
   const constants__t_ReadValue_i msg_read_request__p_rvi,
   t_bool * const msg_read_request__p_isvalid,
   constants__t_NodeId_i * const msg_read_request__p_nid,
   constants__t_AttributeId_i * const msg_read_request__p_aid);

#endif
