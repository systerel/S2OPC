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

#include <stdbool.h>
#include <stddef.h>

#include "sopc_assert.h"
#include "sopc_atomic.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_sub_sockets_mgr.h"
#include "sopc_threads.h"
#include "sopc_time_reference.h"

// Default time for socket set timeout if no timeout event is provided
#define DEFAULT_SOCKET_SET_TIMEOUT_MS 500

// No cpu affinity
#ifndef SOPC_PUBSUB_CPU_AFFINITY
#define SOPC_PUBSUB_CPU_AFFINITY (-1)
#endif

static struct
{
    int32_t initDone;
    int32_t stopFlag;
    SOPC_Thread thread;
    void* sockContextArray;
    size_t sizeOfSockContextElt;
    SOPC_Socket* socketArray;
    uint16_t nbSockets;
    SOPC_ReadyToReceive* pCallback;
    void* callbackCtx;
    SOPC_Sub_Sockets_Timeout timeout;

} receptionThread = {.initDone = false,
                     .stopFlag = 0,
                     .sockContextArray = NULL,
                     .sizeOfSockContextElt = 0,
                     .socketArray = NULL,
                     .nbSockets = 0,
                     .pCallback = NULL,
                     .timeout.callback = NULL,
                     .timeout.pContext = NULL,
                     .timeout.period_ms = 0,
                     .thread = SOPC_INVALID_THREAD};

static void* SOPC_Sub_SocketsMgr_ThreadLoop(void* nullData)
{
    SOPC_UNUSED_ARG(nullData);
    SOPC_TimeReference lastTick = SOPC_TimeReference_GetCurrent();
    SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_PUBSUB, "Starting SOPC_Sub_SocketsMgr_ThreadLoop with %d sockets",
                          receptionThread.nbSockets);

    SOPC_SocketSet* readSet = SOPC_SocketSet_Create();
    SOPC_SocketSet* writeSet = SOPC_SocketSet_Create();
    SOPC_SocketSet* exceptSet = SOPC_SocketSet_Create();

    while (SOPC_Atomic_Int_Get(&receptionThread.stopFlag) == false)
    {
        bool isTimeout = false;
        /*
         * This thread reads from all sockets in a socketSet.
         * However, in the case of specific protocols, there may be no sockets to read from
         * (e.g. MQTT). In this case, socket reading is replaced by a single Sleep to simulate
         * reception timeout (actual reading will anyway be provided by protocol callbacks.).
         * Then the thread must ensure that the 'timeout' event is called periodically (even in case of
         * reception), since the called event is responsible for checking the actual timeout of each
         * DSM.
         */
        if (receptionThread.nbSockets > 0)
        {
            int32_t nbReady = 0;
            SOPC_SocketSet_Clear(readSet);
            SOPC_SocketSet_Clear(writeSet);
            SOPC_SocketSet_Clear(exceptSet);

            for (uint16_t i = 0; i < receptionThread.nbSockets; i++)
            {
                SOPC_SocketSet_Add(receptionThread.socketArray[i], readSet);
            }

            // Returns number of ready descriptor or -1 in case of error
            nbReady = SOPC_Socket_WaitSocketEvents(readSet, writeSet, exceptSet, receptionThread.timeout.period_ms);

            if (nbReady < 0)
            {
                // This will happen if SOPC_Socket_WaitSocketEvents fails. In this case switch to non
                // socket mode to avoid infinite loop
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "SOPC_Socket_WaitSocketEvents failed.");
                receptionThread.nbSockets = 0;
            }
            else if (nbReady == 0)
            {
                isTimeout = true;
            }
            else
            {
                // Evaluate if tick shall be called regarding time elapsed since last call
                SOPC_TimeReference now = SOPC_TimeReference_GetCurrent();
                if (now - lastTick >= receptionThread.timeout.period_ms)
                {
                    isTimeout = true;
                }

                for (uint16_t i = 0; i < receptionThread.nbSockets; i++)
                {
                    if (SOPC_SocketSet_IsPresent(receptionThread.socketArray[i], readSet) != false)
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
        else
        {
            // No socket to read.
            SOPC_Sleep(receptionThread.timeout.period_ms);
            isTimeout = true;
        }
        if (isTimeout && NULL != receptionThread.timeout.callback)
        {
            lastTick = SOPC_TimeReference_GetCurrent();
            (*receptionThread.timeout.callback)(receptionThread.timeout.pContext);
        }
    }

    SOPC_SocketSet_Delete(&readSet);
    SOPC_SocketSet_Delete(&writeSet);
    SOPC_SocketSet_Delete(&exceptSet);

    return NULL;
}

static bool SOPC_Sub_SocketsMgr_LoopThreadStart(void* sockContextArray,
                                                size_t sizeOfSockContextElt,
                                                SOPC_Socket* socketArray,
                                                uint16_t nbSockets,
                                                SOPC_ReadyToReceive* pCallback,
                                                const SOPC_Sub_Sockets_Timeout* pTimeout,
                                                int threadPriority)
{
    SOPC_ASSERT(NULL != pCallback);

    if (SOPC_Atomic_Int_Get(&receptionThread.initDone))
    {
        return false;
    }

    receptionThread.sockContextArray = sockContextArray;
    receptionThread.sizeOfSockContextElt = sizeOfSockContextElt;
    receptionThread.socketArray = socketArray;
    receptionThread.nbSockets = nbSockets;
    receptionThread.pCallback = pCallback;
    if (NULL != pTimeout)
    {
        receptionThread.timeout = *pTimeout;
    }
    else
    {
        receptionThread.timeout.callback = NULL;
        receptionThread.timeout.pContext = NULL;
        receptionThread.timeout.period_ms = DEFAULT_SOCKET_SET_TIMEOUT_MS;
    }

    receptionThread.stopFlag = 0;

    SOPC_ReturnStatus threadReturnStatus = SOPC_STATUS_OK;
    if (0 == threadPriority && -1 == SOPC_PUBSUB_CPU_AFFINITY)
    {
        threadReturnStatus =
            SOPC_Thread_Create(&receptionThread.thread, SOPC_Sub_SocketsMgr_ThreadLoop, NULL, "SubSocketMgr");
    }
    else
    {
        threadReturnStatus =
            SOPC_Thread_CreatePrioritized(&receptionThread.thread, SOPC_Sub_SocketsMgr_ThreadLoop, NULL, threadPriority,
                                          SOPC_PUBSUB_CPU_AFFINITY, "SubSocketMgr");
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

    SOPC_Thread_Join(&receptionThread.thread);

    // set to null thread handle, because well joined
    receptionThread.thread = 0;

    SOPC_Atomic_Int_Set(&receptionThread.initDone, false);
}

void SOPC_Sub_SocketsMgr_Initialize(void* sockContextArray,
                                    size_t sizeOfSockContextElt,
                                    SOPC_Socket* socketArray,
                                    uint16_t nbSockets,
                                    SOPC_ReadyToReceive* pCallback,
                                    const SOPC_Sub_Sockets_Timeout* pTimeout,
                                    int threadPriority)
{
    SOPC_ASSERT(NULL != socketArray);
    bool result = SOPC_Sub_SocketsMgr_LoopThreadStart(sockContextArray, sizeOfSockContextElt, socketArray, nbSockets,
                                                      pCallback, pTimeout, threadPriority);
    SOPC_ASSERT(result);
}

void SOPC_Sub_SocketsMgr_Clear(void)
{
    SOPC_SocketsNetworkEventMgr_LoopThreadStop();
}
