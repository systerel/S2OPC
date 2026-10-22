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
 * \file monitored_item_pointer_bs_inlined.h
 * \brief Manual implementation of the basic machine monitored_item_pointer_bs_inlined.mch.
 *
 * Operations of *_bs_inlined machines are performance critical accessors, defined here as strongly inlined
 * functions instead of being declared in a translated header: the B to C translator does not translate
 * *_bs_inlined machines (see bsrc/Makefile) and the generated callers include this header directly.
 */

#ifndef MONITORED_ITEM_POINTER_BS_INLINED_H_
#define MONITORED_ITEM_POINTER_BS_INLINED_H_

#include "constants.h"
#include "monitored_item_pointer_bs.h"
#include "monitored_item_pointer_impl.h"

#include "sopc_assert.h"
#include "sopc_macros.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
/** \brief No state to initialize: the machine only reads monitored_item_pointer_bs state. */
static SOPC_STRONG_INLINE void monitored_item_pointer_bs_inlined__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/
static SOPC_STRONG_INLINE void monitored_item_pointer_bs_inlined__getall_monitoredItemPointer(
    const constants__t_monitoredItemPointer_i monitored_item_pointer_bs_inlined__p_monitoredItemPointer,
    constants__t_monitoredItemId_i* const monitored_item_pointer_bs_inlined__p_monitoredItemId,
    constants__t_subscription_i* const monitored_item_pointer_bs_inlined__p_subscription,
    constants__t_NodeId_i* const monitored_item_pointer_bs_inlined__p_nid,
    constants__t_AttributeId_i* const monitored_item_pointer_bs_inlined__p_aid,
    constants__t_IndexRange_i* const monitored_item_pointer_bs_inlined__p_indexRange,
    constants__t_TimestampsToReturn_i* const monitored_item_pointer_bs_inlined__p_timestampToReturn,
    constants__t_monitoringMode_i* const monitored_item_pointer_bs_inlined__p_monitoringMode,
    constants__t_client_handle_i* const monitored_item_pointer_bs_inlined__p_clientHandle)
{
    SOPC_ASSERT(NULL != monitored_item_pointer_bs_inlined__p_monitoredItemPointer); // Guaranteed by B model
    const SOPC_InternalMonitoredItem* monitItem =
        (const SOPC_InternalMonitoredItem*) monitored_item_pointer_bs_inlined__p_monitoredItemPointer;
    *monitored_item_pointer_bs_inlined__p_monitoredItemId = monitItem->monitoredItemId;
    *monitored_item_pointer_bs_inlined__p_subscription = monitItem->subId;
    *monitored_item_pointer_bs_inlined__p_nid = monitItem->nid;
    *monitored_item_pointer_bs_inlined__p_aid = monitItem->aid;
    *monitored_item_pointer_bs_inlined__p_indexRange = monitItem->indexRangeString;
    *monitored_item_pointer_bs_inlined__p_timestampToReturn = monitItem->timestampToReturn;
    *monitored_item_pointer_bs_inlined__p_monitoringMode = monitItem->monitoringMode;
    *monitored_item_pointer_bs_inlined__p_clientHandle = monitItem->clientHandle;
}

#endif /* MONITORED_ITEM_POINTER_BS_INLINED_H_ */
