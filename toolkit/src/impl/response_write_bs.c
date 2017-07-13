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

/** \file
 *
 * Implements the structures behind the address space.
 */


#include "b2c.h"
#include "response_write_bs.h"

#include "sopc_base_types.h"


/* Globals */
static SOPC_StatusCode *arr_StatusCode; /* Indexed from 1, first element is never used. */
static t_entier4 nb_req;


/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void response_write_bs__INITIALISATION(void)
{
    arr_StatusCode = NULL;
    nb_req = 0;
}


/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void response_write_bs__alloc_write_request_responses_malloc(
   const t_entier4 response_write_bs__nb_req,
   t_bool * const response_write_bs__ResponseWrite_allocated)
{
    response_write_bs__ResponseWrite_allocated = false; /* TODO: set a true and false in b2c.h */
    nb_req = 0;

    arr_StatusCode = (SOPC_StatusCode *)malloc(sizeof(SOPC_StatusCode)*(response_write_bs__nb_req+1));
    if(NULL != arr_StatusCode)
    {
        response_write_bs__ResponseWrite_allocated = true;
        nb_req = response_write_bs__nb_req;
    }
}


extern void wr__free() /* TODO: not defined yet in the B model */
{
    free(arr_StatusCode);
    arr_StatusCode = NULL;
    nb_req = 0;
}


extern void response_write_bs__getall_ResponseWrite_StatusCode(
   const constants__t_WriteValue_i response_write_bs__wvi,
   t_bool * const response_write_bs__isvalid,
   constants__t_StatusCode_i * const response_write_bs__sc)
{
    response_write_bs__isvalid = false;

    if(response_write_bs__wvi <= nb_req) /* It is not necessary to test arr_StatusCode */
    {
        response_write_bs__isvalid = true;
        response_write_bs__sc = arr_StatusCode[response_write_bs__wvi];
    }
}


extern void response_write_bs__set_ResponseWrite_StatusCode(
   const constants__t_WriteValue_i response_write_bs__wvi,
   const constants__t_StatusCode_i response_write_bs__sc)
{
    arr_StatusCode[response_write_bs__wvi] = response_write_bs__sc;
}

