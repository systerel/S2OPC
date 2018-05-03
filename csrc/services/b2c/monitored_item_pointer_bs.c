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

#include "monitored_item_pointer_bs.h"
#include "monitored_item_pointer_impl.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"

#include <assert.h>

#include "sopc_dict.h"

void SOPC_InternalMontitoredItem_Free(void* data)
{
    SOPC_InternalMontitoredItem* mi = (SOPC_InternalMontitoredItem*) data;
    if (NULL != mi)
    {
        SOPC_NodeId_Clear(mi->nid);
        free(mi);
    }
}

void SOPC_InternalMontitoredItemId_Free(void* data)
{
    (void) data;
    // Nothing to do: uintptr_t value
}

uint64_t SOPC_InternalMontitoredItemId_Hash(const void* data)
{
    uintptr_t id = (uintptr_t) data;
    return (uint64_t) id;
}

bool SOPC_InternalMontitoredItemId_Equal(const void* a, const void* b)
{
    // Compare uintptr_t id values
    return a == b;
}

static SOPC_Dict* monitoredItemIdDict = NULL;
static SOPC_SLinkedList* monitoredItemIdFreed = NULL;

static uintptr_t monitoredItemIdMax = 0;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void monitored_item_pointer_bs__INITIALISATION(void)
{
    if (monitoredItemIdDict != NULL)
    {
        SOPC_Dict_Delete(monitoredItemIdDict);
        monitoredItemIdDict = NULL;
    }

    if (monitoredItemIdFreed != NULL)
    {
        SOPC_SLinkedList_Delete(monitoredItemIdFreed);
        monitoredItemIdFreed = NULL;
    }

    monitoredItemIdDict = SOPC_Dict_Create(0, SOPC_InternalMontitoredItemId_Hash, SOPC_InternalMontitoredItemId_Equal,
                                           SOPC_InternalMontitoredItemId_Free, SOPC_InternalMontitoredItem_Free);
    assert(monitoredItemIdDict != NULL);
    monitoredItemIdFreed = SOPC_SLinkedList_Create(0);
    assert(monitoredItemIdFreed != NULL);
    monitoredItemIdMax = 0;
}

void monitored_item_pointer_bs__UNINITIALISATION_monitored_item_bs(void)
{
    if (monitoredItemIdDict != NULL)
    {
        SOPC_Dict_Delete(monitoredItemIdDict);
        monitoredItemIdDict = NULL;
    }

    if (monitoredItemIdFreed != NULL)
    {
        SOPC_SLinkedList_Delete(monitoredItemIdFreed);
        monitoredItemIdFreed = NULL;
    }

    monitoredItemIdMax = 0;
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void monitored_item_pointer_bs__create_monitored_item_pointer(
    const constants__t_subscription_i monitored_item_pointer_bs__p_subscription,
    const constants__t_NodeId_i monitored_item_pointer_bs__p_nid,
    const constants__t_AttributeId_i monitored_item_pointer_bs__p_aid,
    const constants__t_TimestampsToReturn_i monitored_item_pointer_bs__p_timestampToReturn,
    const constants__t_monitoringMode_i monitored_item_pointer_bs__p_monitoringMode,
    const constants__t_client_handle_i monitored_item_pointer_bs__p_clientHandle,
    t_bool* const monitored_item_pointer_bs__bres,
    constants__t_monitoredItemPointer_i* const monitored_item_pointer_bs__monitoredItemPointer,
    constants__t_monitoredItemId_i* const monitored_item_pointer_bs__monitoredItemId)
{
    *monitored_item_pointer_bs__bres = false;
    uintptr_t freshId = 0;
    SOPC_InternalMontitoredItem* monitItem = malloc(sizeof(SOPC_InternalMontitoredItem));
    if (NULL != monitItem)
    {
        monitItem->subId = monitored_item_pointer_bs__p_subscription;
        monitItem->nid = monitored_item_pointer_bs__p_nid;
        monitItem->aid = monitored_item_pointer_bs__p_aid;
        monitItem->timestampToReturn = monitored_item_pointer_bs__p_timestampToReturn;
        monitItem->monitoringMode = monitored_item_pointer_bs__p_monitoringMode;
        monitItem->clientHandle = monitored_item_pointer_bs__p_clientHandle;

        if (0 == SOPC_SLinkedList_GetLength(monitoredItemIdFreed))
        {
            // No free unique Id, create a new one
            if (monitoredItemIdMax < UINTPTR_MAX && monitoredItemIdMax < UINT32_MAX)
            {
                monitoredItemIdMax++;
                monitItem->monitoredItemId = (uint32_t) monitoredItemIdMax;
                SOPC_Dict_Insert(monitoredItemIdDict, (void*) monitoredItemIdMax, monitItem);
                *monitored_item_pointer_bs__bres = true;
                *monitored_item_pointer_bs__monitoredItemPointer = monitItem;
                *monitored_item_pointer_bs__monitoredItemId = monitItem->monitoredItemId;
            } // else: all Ids already in use
        }
        else
        {
            // Reuse freed id
            freshId = (uintptr_t) SOPC_SLinkedList_PopHead(monitoredItemIdFreed);
            if (freshId != 0)
            {
                monitItem->monitoredItemId = (uint32_t) freshId;
                SOPC_Dict_Insert(monitoredItemIdDict, (void*) freshId, monitItem);
                *monitored_item_pointer_bs__bres = true;
                *monitored_item_pointer_bs__monitoredItemPointer = monitItem;
                *monitored_item_pointer_bs__monitoredItemId = monitItem->monitoredItemId;
            }
        }
    }
}

void monitored_item_pointer_bs__delete_monitored_item_pointer(
    const constants__t_monitoredItemPointer_i monitored_item_pointer_bs__p_monitoredItemPointer)
{
    SOPC_InternalMontitoredItem* monitItem =
        (SOPC_InternalMontitoredItem*) monitored_item_pointer_bs__p_monitoredItemPointer;
    // Reset monitored item associated
    SOPC_Dict_Insert(monitoredItemIdDict, (void*) (uintptr_t) monitItem->monitoredItemId, NULL);
    SOPC_SLinkedList_Append(monitoredItemIdFreed, monitItem->monitoredItemId,
                            (void*) (uintptr_t) monitItem->monitoredItemId);
    free(monitItem);
}

void monitored_item_pointer_bs__getall_monitoredItemPointer(
    const constants__t_monitoredItemPointer_i monitored_item_pointer_bs__p_monitoredItemPointer,
    constants__t_monitoredItemId_i* const monitored_item_pointer_bs__p_monitoredItemId,
    constants__t_subscription_i* const monitored_item_pointer_bs__p_subscription,
    constants__t_NodeId_i* const monitored_item_pointer_bs__p_nid,
    constants__t_AttributeId_i* const monitored_item_pointer_bs__p_aid,
    constants__t_TimestampsToReturn_i* const monitored_item_pointer_bs__p_timestampToReturn,
    constants__t_monitoringMode_i* const monitored_item_pointer_bs__p_monitoringMode,
    constants__t_client_handle_i* const monitored_item_pointer_bs__p_clientHandle)
{
    assert(NULL != monitored_item_pointer_bs__p_monitoredItemPointer); // Guaranteed by B model
    SOPC_InternalMontitoredItem* monitItem =
        (SOPC_InternalMontitoredItem*) monitored_item_pointer_bs__p_monitoredItemPointer;
    *monitored_item_pointer_bs__p_monitoredItemId = monitItem->monitoredItemId;
    *monitored_item_pointer_bs__p_subscription = monitItem->subId;
    *monitored_item_pointer_bs__p_nid = monitItem->nid;
    *monitored_item_pointer_bs__p_aid = monitItem->aid;
    *monitored_item_pointer_bs__p_timestampToReturn = monitItem->timestampToReturn;
    *monitored_item_pointer_bs__p_monitoringMode = monitItem->monitoringMode;
    *monitored_item_pointer_bs__p_clientHandle = monitItem->clientHandle;
}
