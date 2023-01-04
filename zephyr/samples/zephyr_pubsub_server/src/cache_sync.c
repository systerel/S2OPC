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

#include "cache_sync.h"

// System includes
#include <zephyr/kernel.h>

// S2OPC includes
#include "libs2opc_request_builder.h"
#include "libs2opc_server.h"
#include "p_time.h"
#include "sopc_assert.h"
#include "sopc_logger.h"
#include "sopc_mem_alloc.h"

// Demo includes
#include "cache.h"

#ifndef CONFIG_DEMO_CACHE_SYNCH_PERIOD_MS
#error "CONFIG_DEMO_CACHE_SYNCH_PERIOD_MS is not defined"
#endif

/***************************************************/
/**               HELPER LOG MACROS                */
/***************************************************/
#define WARNING(...) SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER, __VA_ARGS__)

/***************************************************/
/**             VARIABLES CONTENT                  */
/***************************************************/

/** A dictionary, but actually only used for Keys {NodeId* : NULL}
 * It contains the nodeId that have been received on Subscriber but not yet updated onto the server */
static SOPC_Dict* pSubscriberPendingUpdates = NULL;
K_MUTEX_DEFINE(subscriberDictgMutex);

// Thread-related data
#define SYNCH_STACK_SIZE 1024
#define SYNCH_PRIORITY (CONFIG_NUM_PREEMPT_PRIORITIES - 1)
static void synch_entry_point(void* p1, void* p2, void* p3);
K_THREAD_DEFINE(my_tid, SYNCH_STACK_SIZE, synch_entry_point, NULL, NULL, NULL, SYNCH_PRIORITY, 0, 0);

// This semaphore is used to trigger synch_entry_point task
struct k_sem my_sem;
K_SEM_DEFINE(sync_sem, 0, 1);

// Date of last reception
static int64_t gLastReceptionDateMs = 0;

typedef struct
{
    OpcUa_WriteRequest* req;
    // nodeIds is only used to monitor failing updates.
    char** nodeIds;
    size_t size;
    size_t next;
} Write_Request_Context;

/**
 * Update address space for the given NodeId*(key)
 */
static void subscriber_pushToWriteRequest(const void* key, void* value, void* user_data)
{
    (void) value;
    SOPC_ASSERT(NULL != user_data);

    Write_Request_Context* context = (Write_Request_Context*) user_data;
    OpcUa_WriteRequest* writeRequest = context->req;

    const SOPC_NodeId* pNodeId = (const SOPC_NodeId*) key;
    if (NULL != pNodeId && NULL != writeRequest)
    {
        SOPC_ReturnStatus status;
        SOPC_ASSERT(context->next < context->size);
        Cache_Lock();
        // Retrieve the value from the cache
        SOPC_DataValue* pDv = Cache_Get(pNodeId);
        status =
            SOPC_WriteRequest_SetWriteValue(writeRequest, context->next, pNodeId, SOPC_AttributeId_Value, NULL, pDv);
        SOPC_ASSERT(status == SOPC_STATUS_OK);
        Cache_Unlock();

        context->nodeIds[context->next] = SOPC_NodeId_ToCString(pNodeId);
        context->next++;
    }
}

#if CONFIG_DEMO_CACHE_SYNCH_PERIOD_MS == 0
#define CACHE_SYNCH_WAIT_NEXT_EVENT(...) \
    do                                   \
    {                                    \
    } while (0)
#define CACHE_SYNCH_INIT_NEXT_EVENT(...) \
    do                                   \
    {                                    \
    } while (0)

#else

static SOPC_RealTime* next_event = NULL;
#define CACHE_SYNCH_INIT_NEXT_EVENT() next_event = SOPC_RealTime_Create(NULL)

static inline void CACHE_SYNCH_WAIT_NEXT_EVENT(void)
{
    if (SOPC_RealTime_IsExpired(next_event, NULL))
    {
        // Immediate return and restart timer from now
        SOPC_RealTime_GetTime(next_event);
    }
    else
    {
        SOPC_RealTime_SleepUntil(next_event);
    }
    SOPC_RealTime_AddSynchedDuration(next_event, CONFIG_DEMO_CACHE_SYNCH_PERIOD_MS * 1000, 0);
}

#endif

static void synch_entry_point(void* p1, void* p2, void* p3)
{
    (void) p1;
    (void) p2;
    (void) p3;
    pSubscriberPendingUpdates = SOPC_NodeId_Dict_Create(true, NULL);
    SOPC_ASSERT(NULL != pSubscriberPendingUpdates && "SOPC_Dict_Create failed");

    CACHE_SYNCH_INIT_NEXT_EVENT();
    while (true)
    {
        CACHE_SYNCH_WAIT_NEXT_EVENT();
        // wait for event!
        k_sem_take(&sync_sem, K_FOREVER);

        // Get all pending NodeId and write them to the server @space

        SOPC_ReturnStatus status;
        OpcUa_WriteRequest* request = NULL;
        OpcUa_WriteResponse* response = NULL;
        Write_Request_Context context;
        context.nodeIds = NULL;

        // Report the values received from Subscriber to AddessSpace
        k_mutex_lock(&subscriberDictgMutex, K_FOREVER);
        const size_t nbRequests = SOPC_Dict_Size(pSubscriberPendingUpdates);

        if (nbRequests > 0)
        {
            request = SOPC_WriteRequest_Create(nbRequests);
            context.next = 0;
            context.size = nbRequests;
            context.req = request;
            context.nodeIds = SOPC_Malloc(sizeof(char*) * nbRequests);
            SOPC_ASSERT(NULL != context.nodeIds);

            // Fill OpcUa_WriteRequest
            SOPC_Dict_ForEach(pSubscriberPendingUpdates, &subscriber_pushToWriteRequest, (void*) &context);

            // Clear and recreate an empty dictionary
            SOPC_Dict_Delete(pSubscriberPendingUpdates);
            pSubscriberPendingUpdates = SOPC_NodeId_Dict_Create(true, NULL);
            SOPC_ASSERT(NULL != pSubscriberPendingUpdates && "SOPC_Dict_Create failed");
        }
        k_mutex_unlock(&subscriberDictgMutex);

        // Process to update if there are some values
        if (NULL != request)
        {
            SOPC_ASSERT(context.next == context.size);
            status = SOPC_ServerHelper_LocalServiceSync(request, (void**) &response);
            if (status != SOPC_STATUS_OK)
            {
                WARNING("SOPC_ServerHelper_LocalServiceSync failed with code  (%d)", status);
                SOPC_Free(request);
            }
        }
        if (NULL != response)
        {
            for (int32_t i = 0; i < response->NoOfResults; i++)
            {
                if (response->Results[i] != SOPC_GoodGenericStatus)
                {
                    WARNING("[SYNC]Could not update node %s, code=0x%08X", context.nodeIds[i], response->Results[i]);
                }
            }

            OpcUa_WriteResponse_Clear(response);
            SOPC_Free(response);
        }

        if (NULL != context.nodeIds)
        {
            for (int32_t i = 0; i < nbRequests; i++)
            {
                SOPC_Free(context.nodeIds[i]);
            }
            SOPC_Free(context.nodeIds);
        }
    }
}

bool cacheSync_SetTargetVariables(OpcUa_WriteValue* nodesToWrite, int32_t nbValues)
{
    gLastReceptionDateMs = k_uptime_get();
    if (pSubscriberPendingUpdates != NULL)
    {
        k_mutex_lock(&subscriberDictgMutex, K_FOREVER);
        // Memorize elements updated until next refresh
        for (size_t idx = 0; idx < nbValues; idx++)
        {
            // Memorize elements updated until next refresh
            SOPC_NodeId* copy = (SOPC_NodeId*) SOPC_Malloc(sizeof(*copy));
            SOPC_ASSERT(NULL != copy);
            SOPC_NodeId_Copy(copy, &nodesToWrite[idx].NodeId);
            SOPC_Dict_Insert(pSubscriberPendingUpdates, copy, NULL);
        }
        k_mutex_unlock(&subscriberDictgMutex);
        k_sem_give(&sync_sem);
    }
    return Cache_SetTargetVariables(nodesToWrite, nbValues);
}

void cacheSync_WriteToCache(const SOPC_NodeId* pNid, const SOPC_DataValue* pDv)
{
    Cache_Lock();
    SOPC_DataValue* pDvCache = Cache_Get(pNid);

    // Only write values of cache that are already defined
    if (pDvCache != NULL)
    {
        // Replace content of Cache
        SOPC_DataValue_Clear(pDvCache);
        SOPC_DataValue_Copy(pDvCache, pDv);
    }
    Cache_Unlock();
}

int cacheSync_LastReceptionDateMs(void)
{
    const int64_t now = k_uptime_get();
    return now - gLastReceptionDateMs;
}
