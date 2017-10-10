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

#include "sopc_user_app_itf.h"
#include "sopc_toolkit_config_internal.h"
#include "sopc_services_api.h"

void service_response_cli_cb_bs__INITIALISATION(void)
{
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void service_response_cli_cb_bs__cli_service_response(
   const constants__t_msg_i service_response_cli_cb_bs__resp_msg,
   const constants__t_StatusCode_i service_response_cli_cb_bs__status){
    SOPC_ServicesToApp_EnqueueEvent(SOPC_AppEvent_ComEvent_Create(SE_RCV_SESSION_RESPONSE),
                                    0, // unused
                                    service_response_cli_cb_bs__resp_msg,
                                    service_response_cli_cb_bs__status);
}

