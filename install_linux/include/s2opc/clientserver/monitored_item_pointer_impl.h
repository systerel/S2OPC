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

#ifndef SOPC_MONITORED_ITEM_POINTER_IMPL_H_
#define SOPC_MONITORED_ITEM_POINTER_IMPL_H_

#include "constants.h"
#include "sopc_numeric_range.h"

typedef struct SOPC_InternalMontitoredItem
{
    uint32_t monitoredItemId;
    constants__t_subscription_i subId;
    constants__t_NodeId_i nid;
    constants__t_AttributeId_i aid;
    constants__t_TimestampsToReturn_i timestampToReturn;
    constants__t_monitoringMode_i monitoringMode;
    constants__t_client_handle_i clientHandle;
    SOPC_NumericRange* indexRange;
} SOPC_InternalMontitoredItem;

#endif /* SOPC_MONITORED_ITEM_POINTER_IMPL_H_ */
