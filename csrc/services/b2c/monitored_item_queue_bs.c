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

#include "monitored_item_queue_bs.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void monitored_item_queue_bs__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void monitored_item_queue_bs__allocate_new_monitored_item_queue(
    t_bool* const monitored_item_queue_bs__bres,
    constants__t_monitoredItemQueue_i* const monitored_item_queue_bs__queue)
{
}

void monitored_item_queue_bs__clear_and_deallocate_monitored_item_queue(
    const constants__t_monitoredItemQueue_i monitored_item_queue_bs__p_queue)
{
}

void monitored_item_queue_bs__add_monitored_item_to_queue(
    const constants__t_monitoredItemQueue_i monitored_item_queue_bs__p_queue,
    const constants__t_monitoredItem_i monitored_item_queue_bs__p_monitoredItem,
    const constants__t_subscription_i monitored_item_queue_bs__p_subscription,
    const constants__t_Node_i monitored_item_queue_bs__p_node,
    const constants__t_AttributeId_i monitored_item_queue_bs__p_aid,
    const constants__t_TimestampsToReturn_i monitored_item_queue_bs__p_timestampToReturn,
    const constants__t_monitoringMode_i monitored_item_queue_bs__p_monitoringMode,
    const constants__t_client_handle_i monitored_item_queue_bs__p_clientHandle,
    t_bool* const monitored_item_queue_bs__bres)
{
}

void monitored_item_queue_bs__continue_iter_monitored_item(
    const constants__t_monitoredItemQueueIterator_i monitored_item_queue_bs__p_iterator,
    const constants__t_monitoredItemQueue_i monitored_item_queue_bs__p_queue,
    t_bool* const monitored_item_queue_bs__continue,
    constants__t_monitoredItem_i* const monitored_item_queue_bs__p_monitoredItem,
    constants__t_subscription_i* const monitored_item_queue_bs__p_subscription,
    constants__t_Node_i* const monitored_item_queue_bs__p_node,
    constants__t_AttributeId_i* const monitored_item_queue_bs__p_aid,
    constants__t_TimestampsToReturn_i* const monitored_item_queue_bs__p_timestampToReturn,
    constants__t_monitoringMode_i* const monitored_item_queue_bs__p_monitoringMode,
    constants__t_client_handle_i* const monitored_item_queue_bs__p_clientHandle)
{
}

void monitored_item_queue_bs__get_monitored_item(
    const constants__t_monitoredItemQueue_i monitored_item_queue_bs__p_queue,
    const constants__t_monitoredItem_i monitored_item_queue_bs__p_monitoredItem,
    t_bool* const monitored_item_queue_bs__p_bres,
    constants__t_subscription_i* const monitored_item_queue_bs__p_subscription,
    constants__t_Node_i* const monitored_item_queue_bs__p_node,
    constants__t_AttributeId_i* const monitored_item_queue_bs__p_aid,
    constants__t_TimestampsToReturn_i* const monitored_item_queue_bs__p_timestampToReturn,
    constants__t_monitoringMode_i* const monitored_item_queue_bs__p_monitoringMode,
    constants__t_client_handle_i* const monitored_item_queue_bs__p_clientHandle)
{
}

void monitored_item_queue_bs__init_iter_monitored_item(
    const constants__t_monitoredItemQueue_i monitored_item_queue_bs__p_queue,
    t_bool* const monitored_item_queue_bs__continue,
    constants__t_monitoredItemQueueIterator_i* const monitored_item_queue_bs__iterator)
{
}

void monitored_item_queue_bs__remove_monitored_item(
    const constants__t_monitoredItemQueue_i monitored_item_queue_bs__p_queue,
    const constants__t_monitoredItem_i monitored_item_queue_bs__p_monitoredItem,
    t_bool* const monitored_item_queue_bs__bres)
{
}
