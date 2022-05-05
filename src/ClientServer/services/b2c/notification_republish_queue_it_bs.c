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

#include "notification_republish_queue_it_bs.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"

#include <assert.h>

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void notification_republish_queue_it_bs__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void notification_republish_queue_it_bs__clear_notif_republish_iterator(
    const constants__t_notifRepublishQueue_i notification_republish_queue_it_bs__p_queue,
    const constants__t_notifRepublishQueueIterator_i notification_republish_queue_it_bs__p_iterator)
{
    SOPC_UNUSED_ARG(notification_republish_queue_it_bs__p_queue);
    SOPC_Free(notification_republish_queue_it_bs__p_iterator);
}

void notification_republish_queue_it_bs__continue_iter_notif_republish(
    const constants__t_notifRepublishQueue_i notification_republish_queue_it_bs__p_queue,
    const constants__t_notifRepublishQueueIterator_i notification_republish_queue_it_bs__p_iterator,
    t_bool* const notification_republish_queue_it_bs__continue,
    constants__t_sub_seq_num_i* const notification_republish_queue_it_bs__seq_num)
{
    SOPC_UNUSED_ARG(notification_republish_queue_it_bs__p_queue);

    void* notifMsg = SOPC_SLinkedList_NextWithId(notification_republish_queue_it_bs__p_iterator,
                                                 notification_republish_queue_it_bs__seq_num);
    assert(notifMsg != NULL);
    *notification_republish_queue_it_bs__continue =
        SOPC_SLinkedList_HasNext(notification_republish_queue_it_bs__p_iterator);
}

void notification_republish_queue_it_bs__get_available_republish(
    const constants__t_notifRepublishQueue_i notification_republish_queue_it_bs__p_queue,
    t_entier4* const notification_republish_queue_it_bs__nb_seq_nums)
{
    uint32_t length = SOPC_SLinkedList_GetLength(notification_republish_queue_it_bs__p_queue);
    if (length <= INT32_MAX)
    {
        *notification_republish_queue_it_bs__nb_seq_nums = (int32_t) length;
    }
    else
    {
        *notification_republish_queue_it_bs__nb_seq_nums = INT32_MAX;
    }
}

void notification_republish_queue_it_bs__init_iter_notif_republish(
    const constants__t_notifRepublishQueue_i notification_republish_queue_it_bs__p_queue,
    t_bool* const notification_republish_queue_it_bs__continue,
    constants__t_notifRepublishQueueIterator_i* const notification_republish_queue_it_bs__iterator)
{
    *notification_republish_queue_it_bs__continue = false;
    *notification_republish_queue_it_bs__iterator = SOPC_Malloc(sizeof(SOPC_SLinkedListIterator));

    if (*notification_republish_queue_it_bs__iterator == NULL)
    {
        return;
    }

    **notification_republish_queue_it_bs__iterator =
        SOPC_SLinkedList_GetIterator(notification_republish_queue_it_bs__p_queue);
    *notification_republish_queue_it_bs__continue =
        SOPC_SLinkedList_HasNext(*notification_republish_queue_it_bs__iterator);
}
