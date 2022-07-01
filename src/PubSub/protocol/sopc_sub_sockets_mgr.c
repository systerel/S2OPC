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

#include <assert.h>
#include <stdbool.h>

#include "sopc_assert.h"
#include "sopc_atomic.h"
#include "sopc_macros.h"
#include "sopc_sub_sockets_mgr.h"
#include "sopc_threads.h"
#include "sopc_time.h"

#define TIMEOUT_MS 500

static struct
{
    int32_t initDone;
    int32_t stopFlag;
    Thread thread;
    void* sockContextArray;
    size_t sizeOfSockContextElt;
    Socket* socketArray;
    uint16_t nbSockets;
    SOPC_ReadyToReceive* pCallback;
    void* callbackCtx;
    SOPC_PeriodicTick* pTickCb;
    void* tickCbCtx;

} receptionThread = {.initDone = false,
                     .stopFlag = 0,
                     .sockContextArray = NULL,
                     .sizeOfSockContextElt = 0,
                     .socketArray = NULL,
                     .nbSockets = 0,
                     .pCallback = NULL,
                     .pTickCb = NULL,
                     .tickCbCtx = NULL,
                     .thread = 0};

static void* SOPC_Sub_SocketsMgr_ThreadLoop(void* nullData)
{
    SOPC_UNUSED_ARG(nullData);
    int32_t nbReady = 0;
    SOPC_SocketSet readSet, writeSet, exceptSet;
    SOPC_TimeReference lastTick = SOPC_TimeReference_GetCurrent();

    while (SOPC_Atomic_Int_Get(&receptionThread.stopFlag) == false)
    {
        SOPC_SocketSet_Clear(&readSet);
        SOPC_SocketSet_Clear(&writeSet);
        SOPC_SocketSet_Clear(&exceptSet);

        for (uint16_t i = 0; i < receptionThread.nbSockets; i++)
        {
            SOPC_SocketSet_Add(receptionThread.socketArray[i], &readSet);
        }

        // Returns number of ready descriptor or -1 in case of error
        nbReady = SOPC_Socket_WaitSocketEvents(&readSet, &writeSet, &exceptSet, TIMEOUT_MS);

        if (nbReady < 0)
        {
            assert(false);
        }
        else if (nbReady == 0)
        {
            // Timeout: tick shall be called
            if (receptionThread.pTickCb != NULL)
            {
                lastTick = SOPC_TimeReference_GetCurrent();
                (*receptionThread.pTickCb)(receptionThread.tickCbCtx);
            }
        }
        else
        {
            if (receptionThread.pTickCb != NULL)
            {
                // Evaluate if tick shall be called regarding time elapsed since last call
                SOPC_TimeReference current = SOPC_TimeReference_GetCurrent();
                if (current - lastTick >= TIMEOUT_MS)
                {
                    lastTick = current;
                    (*receptionThread.pTickCb)(receptionThread.tickCbCtx);
                }
            }

            for (uint16_t i = 0; i < receptionThread.nbSockets; i++)
            {
                if (SOPC_SocketSet_IsPresent(receptionThread.socketArray[i], &readSet) != false)
                {
                    if (receptionThread.pCallback != NULL)
                    {
                        void* sockContext = NULL;
                        if (receptionThread.sockContextArray != NULL)
                        {
                            sockContext = (void*) ((char*) receptionThread.sockContextArray +
                                                   i * receptionThread.sizeOfSockContextElt);
                        }
                        (*receptionThread.pCallback)(sockContext, receptionThread.socketArray[i]);
                    }
                }
            }
        }
    }
    return NULL;
}

static bool SOPC_Sub_SocketsMgr_LoopThreadStart(void* sockContextArray,
                                                size_t sizeOfSockContextElt,
                                                Socket* socketArray,
                                                uint16_t nbSockets,
                                                SOPC_ReadyToReceive* pCallback,
                                                SOPC_PeriodicTick* pTickCb,
                                                void* tickCbCtx,
                                                int threadPriority)
{
    if (SOPC_Atomic_Int_Get(&receptionThread.initDone))
    {
        return false;
    }

    receptionThread.sockContextArray = sockContextArray;
    receptionThread.sizeOfSockContextElt = sizeOfSockContextElt;
    receptionThread.socketArray = socketArray;
    receptionThread.nbSockets = nbSockets;
    receptionThread.pCallback = pCallback;
    receptionThread.pTickCb = pTickCb;
    receptionThread.tickCbCtx = tickCbCtx;

    receptionThread.stopFlag = 0;

    SOPC_ReturnStatus threadReturnStatus = SOPC_STATUS_OK;
    if (0 == threadPriority)
    {
        threadReturnStatus =
            SOPC_Thread_Create(&receptionThread.thread, SOPC_Sub_SocketsMgr_ThreadLoop, NULL, "SubSocketMgr");
    }
    else
    {
        threadReturnStatus = SOPC_Thread_CreatePrioritized(&receptionThread.thread, SOPC_Sub_SocketsMgr_ThreadLoop,
                                                           NULL, threadPriority, "SubSocketMgr");
    }
    if (threadReturnStatus != SOPC_STATUS_OK)
    {
        return false;
    }

    SOPC_Atomic_Int_Set(&receptionThread.initDone, true);

    return true;
}

static void SOPC_SocketsNetworkEventMgr_LoopThreadStop(void)
{
    if (!SOPC_Atomic_Int_Get(&receptionThread.initDone))
    {
        return;
    }

    // stop the reception thread
    SOPC_Atomic_Int_Set(&receptionThread.stopFlag, true);

    SOPC_Thread_Join(receptionThread.thread);

    // set to null thread handle, because well joined
    receptionThread.thread = 0;

    SOPC_Atomic_Int_Set(&receptionThread.initDone, false);
}

void SOPC_Sub_SocketsMgr_Initialize(void* sockContextArray,
                                    size_t sizeOfSockContextElt,
                                    Socket* socketArray,
                                    uint16_t nbSockets,
                                    SOPC_ReadyToReceive* pCallback,
                                    SOPC_PeriodicTick* pTickCb,
                                    void* tickCbCtx,
                                    int threadPriority)
{
    assert(NULL != socketArray);
    assert(NULL != pCallback);
    bool result = SOPC_Sub_SocketsMgr_LoopThreadStart(sockContextArray, sizeOfSockContextElt, socketArray, nbSockets,
                                                      pCallback, pTickCb, tickCbCtx, threadPriority);
    SOPC_ASSERT(result);
}

void SOPC_Sub_SocketsMgr_Clear(void)
{
    SOPC_SocketsNetworkEventMgr_LoopThreadStop();
}
