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

/*------------------------
   Exported Declarations
  ------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include "request_handle_bs.h"

static int cpt = 0;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void request_handle_bs__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void request_handle_bs__fresh_req_handle(
   constants__t_request_handle_i * const request_handle_bs__request_handle) {
  if(cpt + 1 > 0){
    cpt++;
    *request_handle_bs__request_handle = (constants__t_request_handle_i) cpt;
  }else{
    *request_handle_bs__request_handle = constants__c_request_handle_indet;
  }
}

void request_handle_bs__is_valid_req_handle(
   const constants__t_request_handle_i request_handle_bs__req_handle,
   t_bool * const request_handle_bs__ret) {
  *request_handle_bs__ret = request_handle_bs__req_handle != constants__c_request_handle_indet;
}

void request_handle_bs__remove_req_handle(
   const constants__t_request_handle_i request_handle_bs__req_handle) {
  (void) request_handle_bs__req_handle;
}

