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

#include <assert.h>

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"

#include "monitored_item_pointer_impl.h"

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
    *monitored_item_queue_bs__queue = SOPC_SLinkedList_Create(0);
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
    // No deallocation: done in UNINITIALISATION in monitored_item_pointer_bs
    SOPC_SLinkedList_Delete(monitored_item_queue_bs__p_queue);
}

void monitored_item_queue_bs__add_monitored_item_to_queue(
    const constants__t_monitoredItemQueue_i monitored_item_queue_bs__p_queue,
    const constants__t_monitoredItemPointer_i monitored_item_queue_bs__p_monitoredItem,
    t_bool* const monitored_item_queue_bs__bres)
{
    SOPC_InternalMontitoredItem* mi = (SOPC_InternalMontitoredItem*) monitored_item_queue_bs__p_monitoredItem;
    void* res = SOPC_SLinkedList_Append(monitored_item_queue_bs__p_queue, mi->monitoredItemId, mi);
    *monitored_item_queue_bs__bres = (res == mi);
}

void monitored_item_queue_bs__remove_monitored_item(
    const constants__t_monitoredItemQueue_i monitored_item_queue_bs__p_queue,
    const constants__t_monitoredItemPointer_i monitored_item_queue_bs__p_monitoredItem,
    t_bool* const monitored_item_queue_bs__bres)
{
    SOPC_InternalMontitoredItem* mi = (SOPC_InternalMontitoredItem*) monitored_item_queue_bs__p_monitoredItem;

    constants__t_monitoredItemPointer_i* res =
        SOPC_SLinkedList_RemoveFromId(monitored_item_queue_bs__p_queue, mi->monitoredItemId);
    *monitored_item_queue_bs__bres = (res != NULL);
}
