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

#include "notification_republish_queue_it_bs.h"

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
    (void) notification_republish_queue_it_bs__p_queue;
    free(notification_republish_queue_it_bs__p_iterator);
}

void notification_republish_queue_it_bs__continue_iter_notif_republish(
    const constants__t_notifRepublishQueue_i notification_republish_queue_it_bs__p_queue,
    const constants__t_notifRepublishQueueIterator_i notification_republish_queue_it_bs__p_iterator,
    t_bool* const notification_republish_queue_it_bs__continue,
    constants__t_sub_seq_num_i* const notification_republish_queue_it_bs__seq_num)
{
    (void) notification_republish_queue_it_bs__p_queue;

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
    *notification_republish_queue_it_bs__iterator = malloc(sizeof(SOPC_SLinkedListIterator));

    if (*notification_republish_queue_it_bs__iterator == NULL)
    {
        return;
    }

    **notification_republish_queue_it_bs__iterator =
        SOPC_SLinkedList_GetIterator(notification_republish_queue_it_bs__p_queue);
    *notification_republish_queue_it_bs__continue =
        SOPC_SLinkedList_HasNext(*notification_republish_queue_it_bs__iterator);
}
