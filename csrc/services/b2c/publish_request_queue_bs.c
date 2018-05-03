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

#include <assert.h>

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

void publish_request_queue_bs__add_publish_request_to_queue(
    const constants__t_publishReqQueue_i publish_request_queue_bs__p_queue,
    const constants__t_session_i publish_request_queue_bs__p_session,
    const constants__t_timeref_i publish_request_queue_bs__p_req_exp_time,
    const constants__t_server_request_handle_i publish_request_queue_bs__p_req_handle,
    const constants__t_request_context_i publish_request_queue_bs__p_req_ctx,
    const constants__t_msg_i publish_request_queue_bs__p_resp_msg,
    t_bool* const publish_request_queue_bs__bres)
{
    assert(false);
}

void publish_request_queue_bs__continue_pop_iter_publish_request(
    const constants__t_publishReqQueue_i publish_request_queue_bs__p_queue,
    t_bool* const publish_request_queue_bs__p_continue,
    constants__t_session_i* const publish_request_queue_bs__p_session,
    constants__t_timeref_i* const publish_request_queue_bs__p_req_exp_time,
    constants__t_server_request_handle_i* const publish_request_queue_bs__p_req_handle,
    constants__t_request_context_i* const publish_request_queue_bs__p_req_ctx,
    constants__t_msg_i* const publish_request_queue_bs__p_resp_msg)
{
    assert(false);
}

void publish_request_queue_bs__init_iter_publish_request(
    const constants__t_publishReqQueue_i publish_request_queue_bs__p_queue,
    t_bool* const publish_request_queue_bs__continue)
{
    assert(false);
}
