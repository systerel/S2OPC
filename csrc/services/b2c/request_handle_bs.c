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

/*------------------------
   Exported Declarations
  ------------------------*/
#include "request_handle_bs.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sopc_toolkit_constants.h"

static uint16_t cpt = 0;
static constants__t_msg_type_i client_requests[SOPC_MAX_PENDING_REQUESTS + 1];
static constants__t_application_context_i client_requests_context[SOPC_MAX_PENDING_REQUESTS + 1];

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void request_handle_bs__INITIALISATION(void)
{
    memset(client_requests, constants__c_msg_type_indet,
           (SOPC_MAX_PENDING_REQUESTS + 1) * sizeof(constants__t_msg_type_i));
    memset(client_requests_context, constants__c_no_application_context,
           (SOPC_MAX_PENDING_REQUESTS + 1) * sizeof(constants__t_application_context_i));
}

/*--------------------
   OPERATIONS Clause
  --------------------*/

void request_handle_bs__client_validate_response_request_handle(
    const constants__t_request_handle_i request_handle_bs__req_handle,
    const constants__t_msg_type_i request_handle_bs__resp_typ,
    t_bool* const request_handle_bs__ret)
{
    if (request_handle_bs__resp_typ != constants__c_msg_type_indet &&
        client_requests[request_handle_bs__req_handle] == request_handle_bs__resp_typ)
    {
        client_requests[request_handle_bs__req_handle] = constants__c_msg_type_indet;
        *request_handle_bs__ret = true;
    }
    else
    {
        *request_handle_bs__ret = false;
    }
}

void request_handle_bs__client_fresh_req_handle(const constants__t_msg_type_i request_handle_bs__resp_typ,
                                                const constants__t_application_context_i request_handle_bs__app_context,
                                                constants__t_request_handle_i* const request_handle_bs__request_handle)
{
    uint32_t startedIdx = cpt;
    uint8_t noHandleAvailable = false;
    *request_handle_bs__request_handle = constants__c_request_handle_indet;
    if (request_handle_bs__resp_typ != constants__c_msg_type_indet)
    {
        while (false == noHandleAvailable && *request_handle_bs__request_handle == constants__c_request_handle_indet)
        {
            cpt = (cpt + 1) % (SOPC_MAX_PENDING_REQUESTS + 1);
            if (cpt == startedIdx)
            {
                // Note: startedIdx content is never tested (simplest implem)
                noHandleAvailable = true;
            }
            else if (cpt != constants__c_request_handle_indet) // avoid 0 which is undetermined in B model
            {
                if (client_requests[cpt] == constants__c_msg_type_indet)
                {
                    client_requests[cpt] = request_handle_bs__resp_typ;
                    client_requests_context[cpt] = request_handle_bs__app_context;
                    *request_handle_bs__request_handle = (constants__t_request_handle_i) cpt;
                }
            }
        }
    }
}

void request_handle_bs__is_valid_req_handle(const constants__t_request_handle_i request_handle_bs__req_handle,
                                            t_bool* const request_handle_bs__ret)
{
    *request_handle_bs__ret = request_handle_bs__req_handle != constants__c_request_handle_indet;
}

void request_handle_bs__get_req_handle_app_context(
    const constants__t_request_handle_i request_handle_bs__req_handle,
    constants__t_application_context_i* const request_handle_bs__app_context)
{
    *request_handle_bs__app_context = client_requests_context[request_handle_bs__req_handle];
}

void request_handle_bs__client_remove_req_handle(const constants__t_request_handle_i request_handle_bs__req_handle)
{
    (void) request_handle_bs__req_handle;
}

void request_handle_bs__req_handle_do_nothing(const constants__t_request_handle_i request_handle_bs__req_handle)
{
    (void) request_handle_bs__req_handle;
}
