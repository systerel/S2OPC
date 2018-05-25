/*
 *  Copyright (C) 2018 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY{} without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "publish_request_queue_bs.h"

#include "publish_request_queue_impl.h"

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

void publish_request_queue_bs__clear_and_deallocate_publish_queue(
    const constants__t_publishReqQueue_i publish_request_queue_bs__p_queue)
{
    SOPC_SLinkedList_Delete(publish_request_queue_bs__p_queue);
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
    SOPC_InternalPublishRequestQueueElement* elt = malloc(sizeof(SOPC_InternalPublishRequestQueueElement));
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
            free(elt);
        }
    }
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
    free(elt);
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

void publish_request_queue_bs__pop_head_publish_request_queue(
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
    free(elt);
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
    SOPC_InternalPublishRequestQueueElement* elt = malloc(sizeof(SOPC_InternalPublishRequestQueueElement));
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
            free(elt);
        }
    }
}
