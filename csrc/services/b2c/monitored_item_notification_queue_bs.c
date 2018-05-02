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

#include "monitored_item_notification_queue_bs.h"

#include <assert.h>

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void monitored_item_notification_queue_bs__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void monitored_item_notification_queue_bs__allocate_new_monitored_item_notification_queue(
    t_bool* const monitored_item_notification_queue_bs__bres,
    constants__t_notificationQueue_i* const monitored_item_notification_queue_bs__queue)
{
    *monitored_item_notification_queue_bs__queue = SOPC_SLinkedList_Create(0);
    if (*monitored_item_notification_queue_bs__queue == NULL)
    {
        *monitored_item_notification_queue_bs__bres = false;
    }
    else
    {
        *monitored_item_notification_queue_bs__bres = true;
    }
}

void monitored_item_notification_queue_bs__clear_and_deallocate_monitored_item_notification_queue(
    const constants__t_notificationQueue_i monitored_item_notification_queue_bs__p_queue)
{
    SOPC_SLinkedList_Delete(monitored_item_notification_queue_bs__p_queue);
}

void monitored_item_notification_queue_bs__add_monitored_item_notification_to_queue(
    const constants__t_notificationQueue_i monitored_item_notification_queue_bs__p_queue,
    const constants__t_monitoredItem_i monitored_item_notification_queue_bs__p_monitoredItem,
    const constants__t_WriteValuePointer_i monitored_item_notification_queue_bs__p_writeValuePointer,
    t_bool* const monitored_item_notification_queue_bs__bres)
{
    assert(false);
}

void monitored_item_notification_queue_bs__continue_pop_iter_monitor_item_notification(
    const constants__t_notificationQueueIterator_i monitored_item_notification_queue_bs__p_iterator,
    const constants__t_notificationQueue_i monitored_item_notification_queue_bs__p_queue,
    t_bool* const monitored_item_notification_queue_bs__p_continue,
    constants__t_monitoredItem_i* const monitored_item_notification_queue_bs__p_monitoredItem,
    constants__t_WriteValuePointer_i* const monitored_item_notification_queue_bs__p_writeValuePointer)
{
    assert(false);
}

void monitored_item_notification_queue_bs__init_iter_monitored_item_notification(
    const constants__t_notificationQueue_i monitored_item_notification_queue_bs__p_queue,
    t_bool* const monitored_item_notification_queue_bs__continue,
    constants__t_monitoredItemQueueIterator_i* const monitored_item_notification_queue_bs__iterator)
{
    assert(false);
}
