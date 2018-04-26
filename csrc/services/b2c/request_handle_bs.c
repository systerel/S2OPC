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

#include "sopc_event_timer_manager.h"
#include "sopc_services_api.h"
#include "sopc_toolkit_constants.h"

typedef struct
{
    constants__t_msg_type_i request;
    constants__t_msg_type_i response;
    constants__t_application_context_i appContext;
    bool hasAppContext;
} SOPC_Internal_RequestContext;

static uint16_t cpt = 0;
static SOPC_Internal_RequestContext client_requests_context[SOPC_MAX_PENDING_REQUESTS + 1];
static constants__t_channel_i client_requests_channel[SOPC_MAX_PENDING_REQUESTS + 1];

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void request_handle_bs__INITIALISATION(void)
{
    memset(client_requests_context, 0, (SOPC_MAX_PENDING_REQUESTS + 1) * sizeof(SOPC_Internal_RequestContext));
    memset(client_requests_channel, constants__c_channel_indet,
           (SOPC_MAX_PENDING_REQUESTS + 1) * sizeof(constants__t_channel_i));
}

/*--------------------
   OPERATIONS Clause
  --------------------*/

void request_handle_bs__client_validate_response_request_handle(
    const constants__t_channel_i request_handle_bs__channel,
    const constants__t_request_handle_i request_handle_bs__req_handle,
    const constants__t_msg_type_i request_handle_bs__resp_typ,
    t_bool* const request_handle_bs__ret)
{
    if (request_handle_bs__resp_typ != constants__c_msg_type_indet &&
        client_requests_context[request_handle_bs__req_handle].response == request_handle_bs__resp_typ &&
        client_requests_channel[request_handle_bs__req_handle] == request_handle_bs__channel)
    {
        *request_handle_bs__ret = true;
    }
    else
    {
        *request_handle_bs__ret = false;
    }
}

void request_handle_bs__client_fresh_req_handle(const constants__t_msg_type_i request_handle_bs__req_typ,
                                                const constants__t_msg_type_i request_handle_bs__resp_typ,
                                                const t_bool request_handle_bs__is_applicative,
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
            cpt = (uint16_t)((cpt + 1) % (SOPC_MAX_PENDING_REQUESTS + 1));
            if (cpt == startedIdx)
            {
                // Note: startedIdx content is never tested (simplest implem)
                noHandleAvailable = true;
            }
            else if (cpt != constants__c_request_handle_indet) // avoid 0 which is undetermined in B model
            {
                if (client_requests_context[cpt].response == constants__c_msg_type_indet)
                {
                    client_requests_context[cpt].request = request_handle_bs__req_typ;
                    client_requests_context[cpt].response = request_handle_bs__resp_typ;
                    client_requests_context[cpt].hasAppContext = request_handle_bs__is_applicative;
                    client_requests_context[cpt].appContext = request_handle_bs__app_context;
                    *request_handle_bs__request_handle = (constants__t_request_handle_i) cpt;
                }
            }
        }
    }
}

void request_handle_bs__is_valid_req_handle(const constants__t_request_handle_i request_handle_bs__req_handle,
                                            t_bool* const request_handle_bs__ret)
{
    if (request_handle_bs__req_handle != constants__c_request_handle_indet && request_handle_bs__req_handle > 0 &&
        request_handle_bs__req_handle <= SOPC_MAX_PENDING_REQUESTS)
    {
        *request_handle_bs__ret = true;
    }
    else
    {
        *request_handle_bs__ret = false;
    }
}

void request_handle_bs__get_req_handle_resp_typ(const constants__t_request_handle_i request_handle_bs__req_handle,
                                                constants__t_msg_type_i* const request_handle_bs__resp_typ)
{
    *request_handle_bs__resp_typ = client_requests_context[request_handle_bs__req_handle].response;
}

void request_handle_bs__get_req_handle_req_typ(const constants__t_request_handle_i request_handle_bs__req_handle,
                                               constants__t_msg_type_i* const request_handle_bs__req_typ)
{
    *request_handle_bs__req_typ = client_requests_context[request_handle_bs__req_handle].request;
}

void request_handle_bs__get_req_handle_app_context(
    const constants__t_request_handle_i request_handle_bs__req_handle,
    t_bool* const request_handle_bs__is_applicative,
    constants__t_application_context_i* const request_handle_bs__app_context)
{
    *request_handle_bs__is_applicative = client_requests_context[request_handle_bs__req_handle].hasAppContext;
    *request_handle_bs__app_context = client_requests_context[request_handle_bs__req_handle].appContext;
}

void request_handle_bs__get_req_handle_channel(const constants__t_request_handle_i request_handle_bs__req_handle,
                                               constants__t_channel_i* const request_handle_bs__channel)
{
    *request_handle_bs__channel = client_requests_channel[request_handle_bs__req_handle];
}

void request_handle_bs__client_remove_req_handle(const constants__t_request_handle_i request_handle_bs__req_handle)
{
    if (constants__c_request_handle_indet != request_handle_bs__req_handle)
    {
        memset(&client_requests_context[request_handle_bs__req_handle], 0, sizeof(SOPC_Internal_RequestContext));
        client_requests_channel[request_handle_bs__req_handle] = constants__c_channel_indet;
    }
}

void request_handle_bs__req_handle_do_nothing(const constants__t_request_handle_i request_handle_bs__req_handle)
{
    (void) request_handle_bs__req_handle;
}

void request_handle_bs__client_req_handle_to_request_id(
    const constants__t_request_handle_i request_handle_bs__req_handle,
    constants__t_request_context_i* const request_handle_bs__request_id)
{
    *request_handle_bs__request_id = request_handle_bs__req_handle;
}

void request_handle_bs__client_request_id_to_req_handle(
    const constants__t_request_context_i request_handle_bs__request_id,
    constants__t_request_handle_i* const request_handle_bs__request_handle)
{
    *request_handle_bs__request_handle = request_handle_bs__request_id;
}

void request_handle_bs__set_req_handle_channel(const constants__t_request_handle_i request_handle_bs__req_handle,
                                               const constants__t_channel_i request_handle_bs__channel)
{
    client_requests_channel[request_handle_bs__req_handle] = request_handle_bs__channel;
}
