/*
 * Licensed to Systerel under one or more contributor license
 * agreements. See the NOTICE file distributed with this work
 * for additional information regarding copyright ownership.
 * Systerel licenses this file to you under the Apache
 * License, Version 2.0 (the "License"); you may not use this
 * file except in compliance with the License. You may obtain
 * a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include "service_response_cb_bs.h"

#include "sopc_internal_app_dispatcher.h"
#include "sopc_logger.h"
#include "sopc_services_api.h"
#include "sopc_toolkit_config_internal.h"
#include "sopc_user_app_itf.h"

#include "util_b2c.h"

void service_response_cb_bs__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void service_response_cb_bs__cli_service_response(
    const constants__t_session_i service_response_cb_bs__session,
    const constants__t_msg_i service_response_cb_bs__resp_msg,
    const constants__t_application_context_i service_response_cb_bs__app_context)
{
    if (constants__c_session_indet != service_response_cb_bs__session)
    {
        SOPC_App_EnqueueComEvent(SE_RCV_SESSION_RESPONSE, service_response_cb_bs__session,
                                 (uintptr_t) service_response_cb_bs__resp_msg, service_response_cb_bs__app_context);
    }
    else
    {
        SOPC_App_EnqueueComEvent(SE_RCV_DISCOVERY_RESPONSE, 0, (uintptr_t) service_response_cb_bs__resp_msg,
                                 service_response_cb_bs__app_context);
    }
}

void service_response_cb_bs__cli_snd_failure(
    const constants__t_msg_type_i service_response_cb_bs__req_typ,
    const constants__t_application_context_i service_response_cb_bs__app_context,
    const constants_statuscodes_bs__t_StatusCode_i service_response_cb_bs__error_status)
{
    SOPC_EncodeableType* reqEncType = NULL;
    SOPC_EncodeableType* respEncType = NULL;
    bool isReq = false;
    util_message__get_encodeable_type(service_response_cb_bs__req_typ, &reqEncType, &respEncType, &isReq);
    if (isReq == false)
    {
        reqEncType = NULL; // request type expected
    }
    SOPC_App_EnqueueComEvent(SE_SND_REQUEST_FAILED,
                             util_status_code__B_to_return_status_C(service_response_cb_bs__error_status),
                             (uintptr_t) reqEncType, service_response_cb_bs__app_context);
}

void service_response_cb_bs__srv_service_response(
    const constants__t_endpoint_config_idx_i service_response_cb_bs__endpoint_config_idx,
    const constants__t_msg_i service_response_cb_bs__resp_msg,
    const constants__t_application_context_i service_response_cb_bs__app_context)
{
    SOPC_App_EnqueueComEvent(SE_LOCAL_SERVICE_RESPONSE, service_response_cb_bs__endpoint_config_idx,
                             (uintptr_t) service_response_cb_bs__resp_msg, service_response_cb_bs__app_context);
}

void service_response_cb_bs__srv_write_notification(
    const constants__t_WriteValuePointer_i service_response_cb_bs__write_request_pointer,
    const constants_statuscodes_bs__t_StatusCode_i service_response_cb_bs__write_status)
{
    SOPC_StatusCode sc;
    OpcUa_WriteValue* wv = service_response_cb_bs__write_request_pointer;
    if (NULL != wv)
    {
        util_status_code__B_to_C(service_response_cb_bs__write_status, &sc);
        // Trigger notification event
        SOPC_App_EnqueueAddressSpaceNotification(AS_WRITE_EVENT, 0, (uintptr_t) wv, (uintptr_t) sc);
    }
    else
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "AddressSpace write notification: unexpected NULL pointer avoids notification");
    }
}
