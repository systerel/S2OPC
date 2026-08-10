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

#include "monitored_item_queue_bs.h"

#include "sopc_assert.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"

#include "monitored_item_pointer_impl.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void monitored_item_queue_bs__INITIALISATION(void)
{ /*Translated from B but an intialisation is not needed from this module.*/
}

/*--------------------
   OPERATIONS Clause
  --------------------*/

void monitored_item_queue_bs__allocate_new_monitored_item_queue(
    t_bool* const monitored_item_queue_bs__bres,
    constants__t_monitoredItemQueue_i* const monitored_item_queue_bs__queue)
{
    *monitored_item_queue_bs__queue =
        SOPC_CircularQueueAutoGrow_Create(SOPC_SUBSCRIPTION_MONITORED_ITEM_QUEUE_INITIAL_SIZE);
    if (*monitored_item_queue_bs__queue == NULL)
    {
        *monitored_item_queue_bs__bres = false;
    }
    else
    {
        *monitored_item_queue_bs__bres = true;
    }
}

void monitored_item_queue_bs__clear_and_deallocate_monitored_item_queue(
    const constants__t_monitoredItemQueue_i monitored_item_queue_bs__p_queue)
{
    // No deallocation for queue elements: done in UNINITIALISATION in monitored_item_pointer_bs
    SOPC_CircularQueueAutoGrow_Delete(monitored_item_queue_bs__p_queue);
}

void monitored_item_queue_bs__add_monitored_item_to_queue(
    const constants__t_monitoredItemQueue_i monitored_item_queue_bs__p_queue,
    const constants__t_monitoredItemPointer_i monitored_item_queue_bs__p_monitoredItem,
    t_bool* const monitored_item_queue_bs__bres)
{
    SOPC_InternalMonitoredItem* mi = (SOPC_InternalMonitoredItem*) monitored_item_queue_bs__p_monitoredItem;
    uintptr_t res = SOPC_CircularQueueAutoGrow_Append(monitored_item_queue_bs__p_queue, (uintptr_t) mi);
    *monitored_item_queue_bs__bres = (res == (uintptr_t) mi);
}

void monitored_item_queue_bs__remove_monitored_item(
    const constants__t_monitoredItemQueue_i monitored_item_queue_bs__p_queue,
    const constants__t_monitoredItemPointer_i monitored_item_queue_bs__p_monitoredItem,
    t_bool* const monitored_item_queue_bs__bres)
{
    SOPC_InternalMonitoredItem* mi = (SOPC_InternalMonitoredItem*) monitored_item_queue_bs__p_monitoredItem;

    uintptr_t res = SOPC_CircularQueueAutoGrow_RemoveValue(monitored_item_queue_bs__p_queue, (uintptr_t) mi);
    *monitored_item_queue_bs__bres = (res != 0);
}
