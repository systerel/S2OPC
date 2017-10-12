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
#include "session_request_handle_bs.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

static constants__t_session_i unique_session = constants__c_session_indet;
static constants__t_request_handle_i unique_req_handle = constants__c_request_handle_indet;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void session_request_handle_bs__INITIALISATION(){
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void session_request_handle_bs__client_add_session_request_handle(
   const constants__t_session_i session_request_handle_bs__session,
   const constants__t_request_handle_i session_request_handle_bs__req_handle){
  if(session_request_handle_bs__session != constants__c_session_indet &&
     session_request_handle_bs__req_handle != constants__c_request_handle_indet &&
     unique_session == constants__c_session_indet &&
     unique_req_handle == constants__c_request_handle_indet){
    unique_session = session_request_handle_bs__session;
    unique_req_handle = session_request_handle_bs__req_handle;
  }else{
    printf("session_request_handle_bs__client_add_session_request_handle\n");
    exit(1);
  }
}

void session_request_handle_bs__client_get_session_and_remove_request_handle(
   const constants__t_request_handle_i session_request_handle_bs__req_handle,
   constants__t_session_i * const session_request_handle_bs__session){
  if(unique_req_handle != constants__c_request_handle_indet &&
     unique_req_handle == session_request_handle_bs__req_handle){
    *session_request_handle_bs__session = unique_session;
    unique_session = constants__c_session_indet;
    unique_req_handle = constants__c_request_handle_indet;
  }else{
    *session_request_handle_bs__session = constants__c_session_indet;
  }
}

void session_request_handle_bs__client_remove_all_request_handles(
   const constants__t_session_i session_request_handle_bs__session){
  if(session_request_handle_bs__session == unique_session){
    unique_session = constants__c_session_indet;
    unique_req_handle = constants__c_request_handle_indet;
  }
}
