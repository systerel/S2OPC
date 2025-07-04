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

#include "service_register_server2_offline_bs.h"

#include "service_discovery_servers_internal.h"
#include "sopc_assert.h"
#include "sopc_date_time.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_singly_linked_list.h"

static SOPC_SLinkedList* registeredServer2OfflineList = NULL;
static SOPC_SLinkedListIterator registeredServer2OfflineListIt;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void service_register_server2_offline_bs__INITIALISATION(void)
{
    registeredServer2OfflineList = SOPC_SLinkedList_Create(0);
    SOPC_ASSERT(NULL != registeredServer2OfflineList);
    registeredServer2OfflineListIt = NULL;
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void service_register_server2_offline_bs__add_to_registered_server2_offline_set(
    const t_entier4 service_register_server2_offline_bs__p_recordId)
{
    uintptr_t added = SOPC_SLinkedList_Append(registeredServer2OfflineList,
                                              (uint32_t) service_register_server2_offline_bs__p_recordId,
                                              (uintptr_t) service_register_server2_offline_bs__p_recordId);
    SOPC_ASSERT(0 != added);
}

void service_register_server2_offline_bs__init_iter_registered_server2_offline_set(
    t_bool* const service_register_server2_offline_bs__continue)
{
    registeredServer2OfflineListIt = SOPC_SLinkedList_GetIterator(registeredServer2OfflineList);
    if (NULL != registeredServer2OfflineListIt)
    {
        *service_register_server2_offline_bs__continue = SOPC_SLinkedList_HasNext(&registeredServer2OfflineListIt);
    }
    else
    {
        *service_register_server2_offline_bs__continue = false;
    }
}

void service_register_server2_offline_bs__continue_iter_registered_server2_offline_set(
    t_bool* const service_register_server2_offline_bs__continue,
    t_entier4* const service_register_server2_offline_bs__p_recordId)
{
    SOPC_ASSERT(NULL != registeredServer2OfflineListIt);
    *service_register_server2_offline_bs__p_recordId =
        (t_entier4) SOPC_SLinkedList_Next(&registeredServer2OfflineListIt);
    *service_register_server2_offline_bs__continue = SOPC_SLinkedList_HasNext(&registeredServer2OfflineListIt);
}

static void freeRecord(uint32_t id, uintptr_t val)
{
    SOPC_UNUSED_ARG(id);
    SOPC_RegisterServer2Record_Internal* record = (SOPC_RegisterServer2Record_Internal*) val;

    if (NULL != record)
    {
        OpcUa_RegisteredServer_Clear(&record->registeredServer);
        OpcUa_MdnsDiscoveryConfiguration_Clear(&record->mDNSconfig);
    }
    SOPC_Free(record);
}

void service_register_server2_offline_bs__reset_registered_server2_offline_set(void)
{
    SOPC_SLinkedList_Clear(registeredServer2OfflineList);
}

void service_register_server2_offline_bs__service_register_server2_offline_bs_UNINITIALISATION(void)
{
    if (NULL != registeredServer2OfflineList)
    {
        SOPC_SLinkedList_Apply(registeredServer2OfflineList, freeRecord);
    }
    SOPC_SLinkedList_Delete(registeredServer2OfflineList);
    registeredServer2OfflineList = NULL;
}
