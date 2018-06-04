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

#ifndef SOPC_MONITORED_ITEM_POINTER_IMPL_H_
#define SOPC_MONITORED_ITEM_POINTER_IMPL_H_

#include "constants.h"

typedef struct SOPC_InternalMontitoredItem
{
    uint32_t monitoredItemId;
    constants__t_subscription_i subId;
    constants__t_NodeId_i nid;
    constants__t_AttributeId_i aid;
    constants__t_TimestampsToReturn_i timestampToReturn;
    constants__t_monitoringMode_i monitoringMode;
    constants__t_client_handle_i clientHandle;
} SOPC_InternalMontitoredItem;

#endif /* SOPC_MONITORED_ITEM_POINTER_IMPL_H_ */
