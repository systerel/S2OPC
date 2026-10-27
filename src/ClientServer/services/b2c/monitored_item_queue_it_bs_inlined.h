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

/**
 * \file monitored_item_queue_it_bs_inlined.h
 * \brief Manual implementation of the basic machine monitored_item_queue_it_bs_inlined.mch.
 *
 * Iterator over the monitored items of a queue, used on each publish cycle and each data change.
 * Operations are strongly inlined (the B to C translator does not translate *_bs_inlined machines, see
 * bsrc/Makefile). The single iterator state is defined once in monitored_item_queue_it_bs_inlined.c:
 * it must not be static in this header, otherwise each translation unit would get its own copy.
 */

#ifndef MONITORED_ITEM_QUEUE_IT_BS_INLINED_H_
#define MONITORED_ITEM_QUEUE_IT_BS_INLINED_H_

#include <stdbool.h>

#include "constants.h"

#include "sopc_assert.h"
#include "sopc_circular_queue.h"
#include "sopc_circular_queue_autogrow.h"
#include "sopc_macros.h"

/** Iterator state, valid only between init_iter (with continue = true) and clear_iter. */
extern SOPC_CircularQueueIterator monitored_item_queue_it_bs_inlined__gIterator;
/** True while ::monitored_item_queue_it_bs_inlined__gIterator is in use (one iteration at a time). */
extern bool monitored_item_queue_it_bs_inlined__gIteratorValid;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
static SOPC_STRONG_INLINE void monitored_item_queue_it_bs_inlined__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/
static SOPC_STRONG_INLINE void monitored_item_queue_it_bs_inlined__continue_iter_monitored_item(
    const constants__t_monitoredItemQueueIterator_i monitored_item_queue_it_bs_inlined__p_iterator,
    const constants__t_monitoredItemQueue_i monitored_item_queue_it_bs_inlined__p_queue,
    t_bool* const monitored_item_queue_it_bs_inlined__continue,
    constants__t_monitoredItemPointer_i* const monitored_item_queue_it_bs_inlined__p_monitoredItem)
{
    SOPC_UNUSED_ARG(monitored_item_queue_it_bs_inlined__p_queue);
    SOPC_UNUSED_ARG(monitored_item_queue_it_bs_inlined__p_iterator);
    SOPC_ASSERT(monitored_item_queue_it_bs_inlined__gIteratorValid);
    *monitored_item_queue_it_bs_inlined__p_monitoredItem =
        (constants__t_monitoredItemPointer_i) SOPC_CircularQueue_Next(&monitored_item_queue_it_bs_inlined__gIterator);
    *monitored_item_queue_it_bs_inlined__continue =
        SOPC_CircularQueue_HasNext(&monitored_item_queue_it_bs_inlined__gIterator);
}

static SOPC_STRONG_INLINE void monitored_item_queue_it_bs_inlined__init_iter_monitored_item(
    const constants__t_monitoredItemQueue_i monitored_item_queue_it_bs_inlined__p_queue,
    t_bool* const monitored_item_queue_it_bs_inlined__continue,
    constants__t_monitoredItemQueueIterator_i* const monitored_item_queue_it_bs_inlined__iterator)
{
    *monitored_item_queue_it_bs_inlined__continue = false;
    *monitored_item_queue_it_bs_inlined__iterator = constants__c_monitoredItemQueueIterator_indet;
    SOPC_ASSERT(!monitored_item_queue_it_bs_inlined__gIteratorValid);
    if (SOPC_CircularQueueAutoGrow_GetLength(monitored_item_queue_it_bs_inlined__p_queue) > 0)
    {
        monitored_item_queue_it_bs_inlined__gIterator =
            SOPC_CircularQueueAutoGrow_GetIterator(monitored_item_queue_it_bs_inlined__p_queue);
        monitored_item_queue_it_bs_inlined__gIteratorValid = true;
        *monitored_item_queue_it_bs_inlined__iterator = &monitored_item_queue_it_bs_inlined__gIterator;
        *monitored_item_queue_it_bs_inlined__continue =
            SOPC_CircularQueue_HasNext(&monitored_item_queue_it_bs_inlined__gIterator);
    }
}

static SOPC_STRONG_INLINE void monitored_item_queue_it_bs_inlined__clear_iter_monitored_item(
    const constants__t_monitoredItemQueueIterator_i monitored_item_queue_it_bs_inlined__p_iterator)
{
    SOPC_UNUSED_ARG(monitored_item_queue_it_bs_inlined__p_iterator);
    monitored_item_queue_it_bs_inlined__gIteratorValid = false;
}

#endif /* MONITORED_ITEM_QUEUE_IT_BS_INLINED_H_ */
