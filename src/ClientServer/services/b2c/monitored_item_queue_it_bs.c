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

#include "monitored_item_queue_it_bs.h"

#include <assert.h>

#include "sopc_macros.h"
#include "sopc_mem_alloc.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void monitored_item_queue_it_bs__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/

void monitored_item_queue_it_bs__continue_iter_monitored_item(
    const constants__t_monitoredItemQueueIterator_i monitored_item_queue_it_bs__p_iterator,
    const constants__t_monitoredItemQueue_i monitored_item_queue_it_bs__p_queue,
    t_bool* const monitored_item_queue_it_bs__continue,
    constants__t_monitoredItemPointer_i* const monitored_item_queue_it_bs__p_monitoredItem)
{
    SOPC_UNUSED_ARG(monitored_item_queue_it_bs__p_queue);
    SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
    SOPC_SLinkedListIterator* it = (SOPC_SLinkedListIterator*) monitored_item_queue_it_bs__p_iterator;
    SOPC_GCC_DIAGNOSTIC_RESTORE
    *monitored_item_queue_it_bs__p_monitoredItem = SOPC_SLinkedList_Next(it);
    *monitored_item_queue_it_bs__continue = SOPC_SLinkedList_HasNext(it);
}

void monitored_item_queue_it_bs__init_iter_monitored_item(
    const constants__t_monitoredItemQueue_i monitored_item_queue_it_bs__p_queue,
    t_bool* const monitored_item_queue_it_bs__continue,
    constants__t_monitoredItemQueueIterator_i* const monitored_item_queue_it_bs__iterator)
{
    SOPC_SLinkedListIterator* it = SOPC_Malloc(sizeof(SOPC_SLinkedListIterator));
    *monitored_item_queue_it_bs__iterator = it;
    *monitored_item_queue_it_bs__continue = false;
    if (it != NULL && SOPC_SLinkedList_GetLength(monitored_item_queue_it_bs__p_queue) > 0)
    {
        *it = SOPC_SLinkedList_GetIterator(monitored_item_queue_it_bs__p_queue);
        if (*monitored_item_queue_it_bs__iterator != NULL)
        {
            *monitored_item_queue_it_bs__continue = SOPC_SLinkedList_HasNext(it);
        }
    }
}

void monitored_item_queue_it_bs__clear_iter_monitored_item(
    const constants__t_monitoredItemQueueIterator_i monitored_item_queue_it_bs__p_iterator)
{
    SOPC_Free(monitored_item_queue_it_bs__p_iterator);
}
