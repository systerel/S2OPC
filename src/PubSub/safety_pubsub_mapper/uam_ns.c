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

/*============================================================================
 * DESCRIPTION
 *===========================================================================*/

/** \file
 * TODO:
 *  Using SOPC Asynchronous queues to stores SPDU & messages before forwarding
 */

/*============================================================================
 * INCLUDES
 *===========================================================================*/

#include "uam_ns.h"
#include "uam_ns2s_itf.h"
#include "uam.h"
#include "uas.h"


#include "sopc_builtintypes.h"
#include "sopc_common.h"
#include "sopc_log_manager.h"
#include "sopc_mem_alloc.h"
#include "sopc_dict.h"
#include "sopc_threads.h"

#include "sopc_async_queue.h"

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include <signal.h>

/*============================================================================
 * LOCAL TYPES
 *===========================================================================*/
typedef struct
{
    SOPC_ByteString bString;
    UAM_SessionHandle dwHandle;
} QueueElement_type;

/*============================================================================
 * LOCAL VARIABLES
 *===========================================================================*/

/**
 * A dictionary object { UAM_SessionHandle : UAM_NS_Configuration_type* }
 */
static SOPC_Dict* gSessions = NULL; // TODO : not sure that is really useful!

/**
 * A Queue containing some QueueElement_type*
 */
static SOPC_AsyncQueue* pzQueue;

/** The processing message thread */
static Thread gThread;

static volatile sig_atomic_t stopSignal = 0;

/*============================================================================
 * IMPLEMENTATION OF INTERNAL SERVICES
 *===========================================================================*/
static const UAS_UInt32 NoHandle = 0xFFFFFFFEu;

/*===========================================================================*/
static void EnqueueEvent (UAM_SessionHandle dwHandle, const void* pData, const size_t sLen)
{
    QueueElement_type* event = SOPC_Malloc(sizeof(*event));
    event->dwHandle = dwHandle;

    SOPC_ByteString_Initialize (&event->bString);
    SOPC_ByteString_CopyFromBytes(&event->bString, (const SOPC_Byte*)pData, (int32_t) sLen);
    SOPC_AsyncQueue_BlockingEnqueue (pzQueue, event);
}
/*===========================================================================*/
static void EnqueueNoEvent (void)
{
    QueueElement_type* event = SOPC_Malloc(sizeof(*event));
    event->dwHandle = NoHandle;

    SOPC_ByteString_Initialize (&event->bString);
    SOPC_AsyncQueue_BlockingEnqueue (pzQueue, event);
}

/*===========================================================================*/
static void* Thread_Impl(void* data)
{
    assert (data == NULL);
    while (stopSignal == 0)
    {
        QueueElement_type* pEvent = NULL;
        SOPC_AsyncQueue_BlockingDequeue (pzQueue, (void**) &pEvent);

        if (pEvent != NULL && pEvent->bString.Length > 0)
        {
            // TODO
            printf("Thread proc evt %d (len=%d)\n", pEvent->dwHandle, pEvent->bString.Length);

            UAM_NS2S_SendSpduImpl (pEvent->bString.Data, (size_t)pEvent->bString.Length, pEvent->dwHandle);
            SOPC_ByteString_Clear(&pEvent->bString);
            SOPC_Free(pEvent);
        }
    }
    return NULL;
}

/*===========================================================================*/
static uint64_t Session_KeyHash_Fct(const void* pKey)
{
    return (const UAM_SessionHandle)(const UAS_INVERSE_PTR)pKey;
}

/*===========================================================================*/
static bool Session_KeyEqual_Fct (const void* a, const void* b)
{
    return a == b;
}

/*===========================================================================*/
static UAM_NS_Configuration_type* Session_Get (const UAM_SessionHandle key)
{
    if (gSessions == NULL)
    {
        return NULL;
    }
    return (UAM_NS_Configuration_type*) SOPC_Dict_Get (gSessions, (void*) (UAS_INVERSE_PTR) key, NULL);
}

/*===========================================================================*/
static bool Session_Add (const UAM_NS_Configuration_type* const pzConfig)
{
    bool bResult = false;
    assert (NULL != pzConfig);

    UAM_NS_Configuration_type* pzPrevConfig = Session_Get (pzConfig->dwHandle);
    if (pzPrevConfig == NULL)
    {
        UAM_NS_Configuration_type* pzNewConfig = SOPC_Malloc (sizeof(*pzNewConfig));
        bResult = (NULL != pzNewConfig);
        if (bResult)
        {
            *pzNewConfig = *pzConfig;
            // Note : Values and Keys are freed
            bResult = SOPC_Dict_Insert (gSessions, (void*)(UAS_INVERSE_PTR)pzConfig->dwHandle, pzNewConfig);
        }
    }
    return bResult;
}

/*============================================================================
 * IMPLEMENTATION OF EXTERNAL SERVICES
 *===========================================================================*/

/*===========================================================================*/
void UAM_NS_Initialize(void)
{
    SOPC_ReturnStatus sopcResult = SOPC_STATUS_INVALID_PARAMETERS;
    assert (gSessions == NULL);
    gSessions = SOPC_Dict_Create (NULL, Session_KeyHash_Fct, Session_KeyEqual_Fct, NULL, SOPC_Free);
    assert (gSessions != NULL);

    sopcResult = SOPC_AsyncQueue_Init (&pzQueue, "UAM_NS_Events");
    if (SOPC_STATUS_OK == sopcResult)
    {
        sopcResult = SOPC_Thread_Create (&gThread, &Thread_Impl, NULL, "UAM_NS_Events task");
    }

    LOG_Trace (LOG_DEBUG, "UAM_NS_Initialize OK!");
}


/*===========================================================================*/
bool UAM_NS_CreateSpdu(const UAM_NS_Configuration_type* const pzConfig)
{
    assert (gSessions != NULL); // TODO Remove
    bool bResult = false;
    if  (gSessions != NULL &&  pzConfig != NULL)
    {
        bResult = Session_Add (pzConfig);
        if (bResult && pzConfig->pfSetup != NULL)
        {
            LOG_Trace (LOG_DEBUG, "UAM_NS_CreateSpdu, HDL=%u",(unsigned) pzConfig->dwHandle);
            bResult = (*pzConfig->pfSetup) (pzConfig->dwHandle, pzConfig->pUserParams);
        }
        if (bResult)
        {
            bResult = UAM_NS2S_Initialize(pzConfig->dwHandle);
        }
    }
    return bResult;
}

/*===========================================================================*/
void UAM_NS_MessageReceived (UAM_SessionHandle dwHandle, const void* pData, const size_t sLen)
{
    const UAM_NS_Configuration_type* pzSession  = Session_Get (dwHandle);

    if (pzSession != NULL && pzQueue != NULL)
    {
        LOG_Trace (LOG_DEBUG, "Received message on HDL=%u (len=%u)", (unsigned) dwHandle, (unsigned) sLen);

        EnqueueEvent (dwHandle, pData, sLen);
    }
}

/*===========================================================================*/
void UAM_NS_Clear(void)
{
    assert (gSessions != NULL);

    // Request the Thread to terminate, using an empty event
    stopSignal = 1;
    EnqueueNoEvent();

    SOPC_Dict_Delete(gSessions);
    gSessions = NULL;


    LOG_Trace (LOG_INFO, "UAM_NS_Clear : waiting for Thread termination...");
    SOPC_Thread_Join(gThread);
    LOG_Trace (LOG_INFO, "UAM_NS_Clear : Thread terminated");
    SOPC_AsyncQueue_Free (&pzQueue);
    UAM_NS2S_Clear();
}
