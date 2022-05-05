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

#include "publish_request_queue_bs.h"
#include "publish_request_queue_impl.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"
#include "message_out_bs.h"
#include "request_handle_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void publish_request_queue_bs__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void publish_request_queue_bs__allocate_new_publish_queue(
    t_bool* const publish_request_queue_bs__bres,
    constants__t_publishReqQueue_i* const publish_request_queue_bs__queue)
{
    *publish_request_queue_bs__queue = SOPC_SLinkedList_Create(0);
    if (*publish_request_queue_bs__queue == NULL)
    {
        *publish_request_queue_bs__bres = false;
    }
    else
    {
        *publish_request_queue_bs__bres = true;
    }
}

static void SOPC_InternalPublishRequestQueueElement_Free(uint32_t id, void* val)
{
    SOPC_UNUSED_ARG(id);
    SOPC_InternalPublishRequestQueueElement* elt = (SOPC_InternalPublishRequestQueueElement*) val;
    OpcUa_PublishResponse_Clear(elt->resp_msg);
    SOPC_Free(elt->resp_msg);
    SOPC_Free(elt);
}

void publish_request_queue_bs__clear_and_deallocate_publish_queue(
    const constants__t_publishReqQueue_i publish_request_queue_bs__p_queue)
{
    SOPC_SLinkedList_Apply(publish_request_queue_bs__p_queue, SOPC_InternalPublishRequestQueueElement_Free);
    SOPC_SLinkedList_Delete(publish_request_queue_bs__p_queue);
}

void publish_request_queue_bs__clear_publish_queue(
    const constants__t_publishReqQueue_i publish_request_queue_bs__p_queue)
{
    SOPC_SLinkedList_Apply(publish_request_queue_bs__p_queue, SOPC_InternalPublishRequestQueueElement_Free);
    SOPC_SLinkedList_Clear(publish_request_queue_bs__p_queue);
}

void publish_request_queue_bs__append_publish_request_to_queue(
    const constants__t_publishReqQueue_i publish_request_queue_bs__p_queue,
    const constants__t_session_i publish_request_queue_bs__p_session,
    const constants__t_timeref_i publish_request_queue_bs__p_req_exp_time,
    const constants__t_server_request_handle_i publish_request_queue_bs__p_req_handle,
    const constants__t_request_context_i publish_request_queue_bs__p_req_ctx,
    const constants__t_msg_i publish_request_queue_bs__p_resp_msg,
    t_bool* const publish_request_queue_bs__bres)
{
    *publish_request_queue_bs__bres = false;
    SOPC_InternalPublishRequestQueueElement* elt = SOPC_Malloc(sizeof(SOPC_InternalPublishRequestQueueElement));
    if (NULL != elt)
    {
        elt->req_ctx = publish_request_queue_bs__p_req_ctx;
        elt->req_exp_time = publish_request_queue_bs__p_req_exp_time;
        elt->req_handle = publish_request_queue_bs__p_req_handle;
        elt->resp_msg = publish_request_queue_bs__p_resp_msg;
        elt->session = publish_request_queue_bs__p_session;

        void* res = SOPC_SLinkedList_Append(publish_request_queue_bs__p_queue, 0, elt);
        if (res == elt)
        {
            *publish_request_queue_bs__bres = true;
        }
        else
        {
            SOPC_Free(elt);
        }
    }
}

void publish_request_queue_bs__discard_oldest_publish_request(
    const constants__t_publishReqQueue_i publish_request_queue_bs__p_queue,
    constants__t_session_i* const publish_request_queue_bs__old_session,
    constants__t_msg_i* const publish_request_queue_bs__old_resp_msg,
    constants__t_server_request_handle_i* const publish_request_queue_bs__old_req_handle,
    constants__t_request_context_i* const publish_request_queue_bs__old_req_ctx)
{
    /* Dequeue oldest publish request */
    SOPC_InternalPublishRequestQueueElement* elt = SOPC_SLinkedList_PopLast(publish_request_queue_bs__p_queue);
    *publish_request_queue_bs__old_session = elt->session;
    *publish_request_queue_bs__old_req_handle = elt->req_handle;
    *publish_request_queue_bs__old_req_ctx = elt->req_ctx;
    *publish_request_queue_bs__old_resp_msg = elt->resp_msg;
    SOPC_Free(elt);
}

void publish_request_queue_bs__get_nb_publish_requests(
    const constants__t_publishReqQueue_i publish_request_queue_bs__p_queue,
    t_entier4* const publish_request_queue_bs__nb_pub_reqs)
{
    *publish_request_queue_bs__nb_pub_reqs = (int32_t) SOPC_SLinkedList_GetLength(publish_request_queue_bs__p_queue);
}

void publish_request_queue_bs__continue_pop_head_iter_publish_request(
    const constants__t_publishReqQueue_i publish_request_queue_bs__p_queue,
    t_bool* const publish_request_queue_bs__p_continue,
    constants__t_session_i* const publish_request_queue_bs__p_session,
    constants__t_timeref_i* const publish_request_queue_bs__p_req_exp_time,
    constants__t_server_request_handle_i* const publish_request_queue_bs__p_req_handle,
    constants__t_request_context_i* const publish_request_queue_bs__p_req_ctx,
    constants__t_msg_i* const publish_request_queue_bs__p_resp_msg)
{
    SOPC_InternalPublishRequestQueueElement* elt = SOPC_SLinkedList_PopHead(publish_request_queue_bs__p_queue);
    *publish_request_queue_bs__p_session = elt->session;
    *publish_request_queue_bs__p_req_exp_time = elt->req_exp_time;
    *publish_request_queue_bs__p_req_handle = elt->req_handle;
    *publish_request_queue_bs__p_req_ctx = elt->req_ctx;
    *publish_request_queue_bs__p_resp_msg = elt->resp_msg;
    SOPC_Free(elt);
    *publish_request_queue_bs__p_continue = SOPC_SLinkedList_GetLength(publish_request_queue_bs__p_queue) > 0;
}

void publish_request_queue_bs__init_iter_publish_request(
    const constants__t_publishReqQueue_i publish_request_queue_bs__p_queue,
    t_bool* const publish_request_queue_bs__continue)
{
    *publish_request_queue_bs__continue = SOPC_SLinkedList_GetLength(publish_request_queue_bs__p_queue) > 0;
}

void publish_request_queue_bs__is_request_expired(const constants__t_timeref_i publish_request_queue_bs__p_req_exp_time,
                                                  t_bool* const publish_request_queue_bs__bres)
{
    SOPC_TimeReference current = SOPC_TimeReference_GetCurrent();
    *publish_request_queue_bs__bres = current >= publish_request_queue_bs__p_req_exp_time;
}

void publish_request_queue_bs__pop_valid_publish_request_queue(
    const constants__t_publishReqQueue_i publish_request_queue_bs__p_queue,
    constants__t_session_i* const publish_request_queue_bs__p_session,
    constants__t_timeref_i* const publish_request_queue_bs__p_req_exp_time,
    constants__t_server_request_handle_i* const publish_request_queue_bs__p_req_handle,
    constants__t_request_context_i* const publish_request_queue_bs__p_req_ctx,
    constants__t_msg_i* const publish_request_queue_bs__p_resp_msg)
{
    SOPC_InternalPublishRequestQueueElement* elt = SOPC_SLinkedList_PopHead(publish_request_queue_bs__p_queue);
    *publish_request_queue_bs__p_session = elt->session;
    *publish_request_queue_bs__p_req_exp_time = elt->req_exp_time;
    *publish_request_queue_bs__p_req_handle = elt->req_handle;
    *publish_request_queue_bs__p_req_ctx = elt->req_ctx;
    *publish_request_queue_bs__p_resp_msg = elt->resp_msg;
    SOPC_Free(elt);
}

void publish_request_queue_bs__prepend_publish_request_to_queue(
    const constants__t_publishReqQueue_i publish_request_queue_bs__p_queue,
    const constants__t_session_i publish_request_queue_bs__p_session,
    const constants__t_timeref_i publish_request_queue_bs__p_req_exp_time,
    const constants__t_server_request_handle_i publish_request_queue_bs__p_req_handle,
    const constants__t_request_context_i publish_request_queue_bs__p_req_ctx,
    const constants__t_msg_i publish_request_queue_bs__p_resp_msg,
    t_bool* const publish_request_queue_bs__bres)
{
    *publish_request_queue_bs__bres = false;
    SOPC_InternalPublishRequestQueueElement* elt = SOPC_Malloc(sizeof(SOPC_InternalPublishRequestQueueElement));
    if (NULL != elt)
    {
        elt->req_ctx = publish_request_queue_bs__p_req_ctx;
        elt->req_exp_time = publish_request_queue_bs__p_req_exp_time;
        elt->req_handle = publish_request_queue_bs__p_req_handle;
        elt->resp_msg = publish_request_queue_bs__p_resp_msg;
        elt->session = publish_request_queue_bs__p_session;

        void* res = SOPC_SLinkedList_Prepend(publish_request_queue_bs__p_queue, 0, elt);
        if (res == elt)
        {
            *publish_request_queue_bs__bres = true;
        }
        else
        {
            SOPC_Free(elt);
        }
    }
}
