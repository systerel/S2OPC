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
 * \file monitored_item_notification_queue_bs_inlined.h
 * \brief Manual implementation of the basic machine monitored_item_notification_queue_bs_inlined.mch.
 *
 * Read-only accessors to the notification queue of a monitored item, called for each monitored item on each publish
 * cycle. Operations are strongly inlined (the B to C translator does not translate *_bs_inlined machines, see
 * bsrc/Makefile).
 */

#ifndef MONITORED_ITEM_NOTIFICATION_QUEUE_BS_INLINED_H_
#define MONITORED_ITEM_NOTIFICATION_QUEUE_BS_INLINED_H_

#include <inttypes.h>
#include <stdint.h>

#include "constants.h"
#include "monitored_item_notification_queue_bs.h"
#include "monitored_item_pointer_impl.h"

#include "sopc_assert.h"
#include "sopc_circular_queue.h"
#include "sopc_logger.h"
#include "sopc_macros.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
/** \brief No state to initialize: the machine only reads monitored_item_notification_queue_bs state. */
static SOPC_STRONG_INLINE void monitored_item_notification_queue_bs_inlined__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/
static SOPC_STRONG_INLINE void monitored_item_notification_queue_bs_inlined__get_monitored_item_notification_queue(
    const constants__t_monitoredItemPointer_i monitored_item_notification_queue_bs_inlined__p_monitoredItem,
    t_bool* const monitored_item_notification_queue_bs_inlined__bres,
    constants__t_notificationQueue_i* const monitored_item_notification_queue_bs_inlined__queue)
{
    *monitored_item_notification_queue_bs_inlined__bres = false;
    *monitored_item_notification_queue_bs_inlined__queue = NULL;
    const SOPC_InternalMonitoredItem* monitoredItemPointer =
        (const SOPC_InternalMonitoredItem*) monitored_item_notification_queue_bs_inlined__p_monitoredItem;
    if (NULL != monitoredItemPointer->notifQueue)
    {
        *monitored_item_notification_queue_bs_inlined__bres = true;
        *monitored_item_notification_queue_bs_inlined__queue = monitoredItemPointer->notifQueue;
    }
    else
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "MonitoredItem (%" PRIu32 ") notification queue is not allocated",
                               monitoredItemPointer->monitoredItemId);
    }
}

static SOPC_STRONG_INLINE void
monitored_item_notification_queue_bs_inlined__get_length_monitored_item_notification_queue(
    const constants__t_notificationQueue_i monitored_item_notification_queue_bs_inlined__p_mi_notif_queue,
    t_entier4* const monitored_item_notification_queue_bs_inlined__p_nb_available_notifs)
{
    SOPC_ASSERT(NULL != monitored_item_notification_queue_bs_inlined__p_mi_notif_queue);
    uint32_t length = SOPC_CircularQueue_GetLength(monitored_item_notification_queue_bs_inlined__p_mi_notif_queue);
    SOPC_ASSERT(length <= INT32_MAX); // Guaranteed by add functions
    *monitored_item_notification_queue_bs_inlined__p_nb_available_notifs = (int32_t) length;
}

static SOPC_STRONG_INLINE void monitored_item_notification_queue_bs_inlined__is_event_monitored_item_notification_queue(
    const constants__t_monitoredItemPointer_i monitored_item_notification_queue_bs_inlined__p_monitoredItem,
    const constants__t_notificationQueue_i monitored_item_notification_queue_bs_inlined__p_mi_notif_queue,
    t_bool* const monitored_item_notification_queue_bs_inlined__p_is_event_queue)
{
    SOPC_UNUSED_ARG(monitored_item_notification_queue_bs_inlined__p_mi_notif_queue);
    *monitored_item_notification_queue_bs_inlined__p_is_event_queue =
        is_eventMI(monitored_item_notification_queue_bs_inlined__p_monitoredItem);
}

#endif /* MONITORED_ITEM_NOTIFICATION_QUEUE_BS_INLINED_H_ */
