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

#include "service_response_cli_cb_bs.h"

#include "test_results.h"

#include "internal_msg.h"

#include "sopc_types.h"

#include "testlib_read_response.h"

#include "util_b2c.h"

void service_response_cli_cb_bs__INITIALISATION(void)
{
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void service_response_cli_cb_bs__cli_service_response(
   const constants__t_msg_i service_response_cli_cb_bs__resp_msg,
   const constants__t_StatusCode_i service_response_cli_cb_bs__status){

  message__message* msg = (message__message*) service_response_cli_cb_bs__resp_msg;
  OpcUa_ReadResponse* readResp = (OpcUa_ReadResponse*) msg->msg;

  if(msg->encType == &OpcUa_ReadResponse_EncodeableType){
    test_results_set_read_result(test_read_request_response(readResp,
                                                            service_response_cli_cb_bs__status,
                                                            0)
                                 ? (!FALSE):FALSE);
  }else if(msg->encType == &OpcUa_WriteResponse_EncodeableType){
    NULL;
  }
}

