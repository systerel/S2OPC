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

/*------------------------
   Exported Declarations
  ------------------------*/
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "constants.h"
#include "message_out_bs.h"
#include "service_mgr_bs.h"
#include "util_b2c.h"

#include "sopc_assert.h"
#include "sopc_internal_app_dispatcher.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_secure_channels_api.h"
#include "sopc_services_api.h"
#include "sopc_singly_linked_list.h"
#include "sopc_toolkit_config_internal.h"

#define SOPC_MAX_WAITING_DISCOVERY_REQUESTS 5

static SOPC_SLinkedList*
    discovery_reqs_to_send[constants_bs__t_channel_config_idx_i_max + 1]; // index 0 is undet channel config

typedef struct SOPC_DiscoveryRequest_ToSend
{
    constants__t_msg_i msgToSend;
    constants__t_msg_type_i msgType;
    constants__t_application_context_i msgAppContext;
} SOPC_DiscoveryRequest_ToSend;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void service_mgr_bs__INITIALISATION(void)
{
    memset(discovery_reqs_to_send, 0, (constants_bs__t_channel_config_idx_i_max + 1) * sizeof(SOPC_SLinkedList*));
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void service_mgr_bs__client_async_discovery_request_without_channel(
    const constants__t_channel_config_idx_i service_mgr_bs__channel_config_idx,
    const constants__t_msg_type_i service_mgr_bs__req_typ,
    const constants__t_msg_i service_mgr_bs__req_msg,
    const constants__t_application_context_i service_mgr_bs__app_context,
    t_bool* const service_mgr_bs__bres)
{
    void* addedMsg = NULL;
    SOPC_SLinkedList* sLinkedList = NULL;
    SOPC_DiscoveryRequest_ToSend* elt = NULL;
    *service_mgr_bs__bres = false;
    if (service_mgr_bs__channel_config_idx > 0 &&
        service_mgr_bs__channel_config_idx <= constants_bs__t_channel_config_idx_i_max)
    {
        sLinkedList = discovery_reqs_to_send[service_mgr_bs__channel_config_idx];
        if (NULL == sLinkedList)
        {
            sLinkedList = SOPC_SLinkedList_Create(SOPC_MAX_WAITING_DISCOVERY_REQUESTS);
            discovery_reqs_to_send[service_mgr_bs__channel_config_idx] = sLinkedList;
        }
        if (NULL != sLinkedList)
        {
            elt = SOPC_Malloc(sizeof(SOPC_DiscoveryRequest_ToSend));
        }
        if (NULL != sLinkedList && NULL != elt)
        {
            elt->msgToSend = service_mgr_bs__req_msg;
            elt->msgType = service_mgr_bs__req_typ;
            elt->msgAppContext = service_mgr_bs__app_context;
            addedMsg = (void*) SOPC_SLinkedList_Append(sLinkedList, 0, (uintptr_t) elt);
        }
        if (NULL != elt && addedMsg == (void*) elt)
        {
            *service_mgr_bs__bres = true;
        }
    }
}

void service_mgr_bs__client_channel_connected_event_discovery(
    const constants__t_channel_config_idx_i service_mgr_bs__channel_config_idx,
    const constants__t_reverse_endpoint_config_idx_i service_mgr_bs__reverse_endpoint_config_idx,
    const constants__t_channel_i service_mgr_bs__channel)
{
    SOPC_UNUSED_ARG(service_mgr_bs__channel);
    SOPC_SLinkedList* sLinkedList = NULL;
    SOPC_SLinkedListIterator listIt = NULL;
    SOPC_DiscoveryRequest_ToSend* elt = NULL;

    if (service_mgr_bs__channel_config_idx > 0 &&
        service_mgr_bs__channel_config_idx <= constants_bs__t_channel_config_idx_i_max)
    {
        sLinkedList = discovery_reqs_to_send[service_mgr_bs__channel_config_idx];
        if (NULL != sLinkedList)
        {
            listIt = SOPC_SLinkedList_GetIterator(sLinkedList);
            if (NULL != listIt)
            {
                while (NULL != listIt)
                {
                    elt = (SOPC_DiscoveryRequest_ToSend*) SOPC_SLinkedList_Next(&listIt);
                    if (elt != NULL)
                    {
                        SOPC_Internal_DiscoveryContext* discoveryContext = SOPC_Calloc(1, sizeof(*discoveryContext));
                        if (NULL != discoveryContext)
                        {
                            discoveryContext->opcuaMessage = elt->msgToSend;
                            discoveryContext->discoveryAppContext = elt->msgAppContext;
                            SOPC_Services_EnqueueEvent(
                                APP_TO_SE_SEND_DISCOVERY_REQUEST, service_mgr_bs__channel_config_idx,
                                service_mgr_bs__reverse_endpoint_config_idx, (uintptr_t) discoveryContext);
                        }
                        else
                        {
                            SOPC_ASSERT(NULL != elt->msgToSend);
                            SOPC_EncodeableType* encType = *(SOPC_EncodeableType**) elt->msgToSend;
                            SOPC_ReturnStatus status =
                                SOPC_App_EnqueueComEvent(SE_SND_REQUEST_FAILED, SOPC_STATUS_OUT_OF_MEMORY,
                                                         (uintptr_t) encType, elt->msgAppContext);
                            SOPC_ASSERT(SOPC_STATUS_OK == status);

                            SOPC_Logger_TraceWarning(
                                SOPC_LOG_MODULE_CLIENTSERVER,
                                "ServicesMgr: APP_TO_SE_SEND_DISCOVERY_REQUEST failed after SC connected" PRIu32
                                " msgType=%s ctx=%" PRIuPTR,
                                SOPC_EncodeableType_GetName(encType), elt->msgAppContext);
                            encType->Clear(elt->msgToSend);
                            SOPC_Free(elt->msgToSend);
                        }
                    }
                    SOPC_Free(elt);
                }
                SOPC_SLinkedList_Clear(sLinkedList);
            }
        }
    }
}

static void SOPC_ServiceMgrBs_DicoveryReqSendingFailure(uint32_t id, uintptr_t val)
{
    SOPC_UNUSED_ARG(id);
    SOPC_DiscoveryRequest_ToSend* elt = (SOPC_DiscoveryRequest_ToSend*) val;
    SOPC_EncodeableType* reqEncType = NULL;
    SOPC_EncodeableType* respEncType = NULL;
    bool isReq = false;
    if (NULL != elt)
    {
        util_message__get_encodeable_type(elt->msgType, &reqEncType, &respEncType, &isReq);
        if (isReq == false)
        {
            reqEncType = NULL; // request type expected
        }
        SOPC_App_EnqueueComEvent(SE_SND_REQUEST_FAILED, SOPC_STATUS_CLOSED, (uintptr_t) NULL, elt->msgAppContext);
        message_out_bs__dealloc_msg_out(elt->msgToSend);
        SOPC_Free(elt);
    }
}

void service_mgr_bs__client_discovery_req_failures_on_final_connection_failure(
    const constants__t_channel_config_idx_i service_mgr_bs__channel_config_idx)
{
    SOPC_SLinkedList* sLinkedList = NULL;
    sLinkedList = discovery_reqs_to_send[service_mgr_bs__channel_config_idx];
    if (NULL != sLinkedList)
    {
        // Generate a send request failure event for each discovery message waiting to be sent
        SOPC_SLinkedList_Apply(sLinkedList, SOPC_ServiceMgrBs_DicoveryReqSendingFailure);
        SOPC_SLinkedList_Delete(sLinkedList);
        discovery_reqs_to_send[service_mgr_bs__channel_config_idx] = NULL;
    }
}

static void SOPC_ServiceMgrBs_DeallocateMsgs(uint32_t id, uintptr_t val)
{
    SOPC_UNUSED_ARG(id);
    SOPC_DiscoveryRequest_ToSend* elt = (SOPC_DiscoveryRequest_ToSend*) val;
    if (NULL != elt)
    {
        message_out_bs__dealloc_msg_out(elt->msgToSend);
        SOPC_Free(elt);
    }
}

void service_mgr_bs__service_mgr_bs_UNINITIALISATION(void)
{
    SOPC_SLinkedList* sLinkedList = NULL;
    for (constants_bs__t_channel_config_idx_i idx = 0; idx <= constants_bs__t_channel_config_idx_i_max; idx++)
    {
        sLinkedList = discovery_reqs_to_send[idx];
        if (NULL != sLinkedList)
        {
            SOPC_SLinkedList_Apply(sLinkedList, SOPC_ServiceMgrBs_DeallocateMsgs);
            SOPC_SLinkedList_Delete(sLinkedList);
            discovery_reqs_to_send[idx] = NULL;
        }
    }
}

void service_mgr_bs__send_channel_error_msg(const constants__t_channel_i service_mgr_bs__channel,
                                            const constants_statuscodes_bs__t_StatusCode_i service_mgr_bs__status_code,
                                            const constants__t_request_context_i service_mgr_bs__request_context)
{
    SOPC_StatusCode status = SOPC_BadStatusMask;
    util_status_code__B_to_C(service_mgr_bs__status_code, &status);
    SOPC_SecureChannels_EnqueueEvent(SC_SERVICE_SND_ERR, service_mgr_bs__channel, (uintptr_t) status,
                                     service_mgr_bs__request_context);
}

void service_mgr_bs__send_channel_msg_buffer(const constants__t_channel_i service_mgr_bs__channel,
                                             const constants__t_byte_buffer_i service_mgr_bs__buffer,
                                             const constants__t_request_context_i service_mgr_bs__request_context)
{
    SOPC_SecureChannels_EnqueueEvent(SC_SERVICE_SND_MSG, service_mgr_bs__channel, (uintptr_t) service_mgr_bs__buffer,
                                     service_mgr_bs__request_context);
}
