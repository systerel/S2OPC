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

#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "sopc_sockets_network_event_mgr.h"

#include "sopc_sockets_api.h"
#include "sopc_sockets_internal_ctx.h"

#include "sopc_event_timer_manager.h"
#include "sopc_logger.h"
#include "sopc_mutexes.h"
#include "sopc_threads.h"
#include "sopc_time.h"

static struct
{
    bool initDone;
    bool stopFlag;
    Mutex tMutex;
    Thread thread;
} receptionThread = {
    .initDone = false,
    .stopFlag = false,
};

static void SOPC_Internal_TriggerEventToSocketsManager(SOPC_Socket* socket,
                                                       SOPC_Sockets_InputEvent socketEvent,
                                                       uint32_t socketIdx)
{
    // Do not treat any network event until triggered event treated
    socket->waitTreatNetworkEvent = true;
    SOPC_Sockets_EnqueueEvent(socketEvent, socketIdx, NULL, 0);
}

// Treat sockets events if some are present or wait for events (until timeout)
static bool SOPC_SocketsNetworkEventMgr_TreatSocketsEvents(uint32_t msecTimeout)
{
    bool result = true;
    uint32_t idx = 0;
    bool socketsUsed = false;
    int32_t nbReady = 0;
    SOPC_Socket* uaSock = NULL;
    SocketSet readSet, writeSet, exceptSet;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;

    SocketSet_Clear(&readSet);
    SocketSet_Clear(&writeSet);
    SocketSet_Clear(&exceptSet);

    Mutex_Lock(&socketsMutex);

    // Add used sockets in the correct socket sets
    for (idx = 0; idx < SOPC_MAX_SOCKETS; idx++)
    {
        uaSock = &(socketsArray[idx]);
        if (uaSock->isUsed != false && false == uaSock->waitTreatNetworkEvent &&
            (uaSock->state == SOCKET_STATE_CONNECTED || uaSock->state == SOCKET_STATE_CONNECTING ||
             uaSock->state == SOCKET_STATE_LISTENING))
        { // Note: accepted state is a state in which we do not know what to do
          //       in case of data received (no high-level connection set).
          //       Wait CONNECTED state for those sockets to treat events again.
            socketsUsed = true;
            if (uaSock->state == SOCKET_STATE_CONNECTING ||
                (uaSock->state == SOCKET_STATE_CONNECTED && uaSock->isNotWritable != false))
            {
                // Wait for an event indicating connection succeeded/failed in CONNECTING state
                // or Wait for an event indicating connection is writable again in CONNECTED state
                SocketSet_Add(uaSock->sock, &writeSet);
            }
            else
            {
                SocketSet_Add(uaSock->sock, &readSet);
            }
            SocketSet_Add(uaSock->sock, &exceptSet);
        }
    }

    Mutex_Unlock(&socketsMutex);

    if (socketsUsed == false)
    {
        // In case no socket is in use, do not call select (*WaitSocketEvents)
        // since it returns an error with WinSockets
        SOPC_Sleep(msecTimeout);
        nbReady = 0;
    }
    else
    {
        // Returns number of ready descriptor or -1 in case of error
        nbReady = Socket_WaitSocketEvents(&readSet, &writeSet, &exceptSet, msecTimeout);
    }

    Mutex_Lock(&socketsMutex);

    if (nbReady < 0)
    {
        // Call to wait events failed
        result = false;
    }
    else if (nbReady > 0)
    {
        for (idx = 0; idx < SOPC_MAX_SOCKETS; idx++)
        {
            uaSock = &(socketsArray[idx]);
            if (uaSock->isUsed != false)
            {
                if (uaSock->state == SOCKET_STATE_CONNECTING)
                {
                    /* Socket is currently in connecting attempt: check WRITE events */

                    if (SocketSet_IsPresent(uaSock->sock, &writeSet) != false)
                    {
                        // Check connection errors: mandatory when non blocking connection
                        status = Socket_CheckAckConnect(uaSock->sock);
                        if (SOPC_STATUS_OK != status)
                        {
                            SOPC_Internal_TriggerEventToSocketsManager(uaSock, INT_SOCKET_CONNECTION_ATTEMPT_FAILED,
                                                                       idx);
                        }
                        else
                        {
                            SOPC_Internal_TriggerEventToSocketsManager(uaSock, INT_SOCKET_CONNECTED, idx);
                        }
                    }
                }
                else
                {
                    /* Socket is not in connecting state: check READ and WRITE events */

                    if (SocketSet_IsPresent(uaSock->sock, &readSet) != false)
                    {
                        if (uaSock->state == SOCKET_STATE_CONNECTED)
                        {
                            SOPC_Internal_TriggerEventToSocketsManager(uaSock, INT_SOCKET_READY_TO_READ, idx);
                        }
                        else if (uaSock->state == SOCKET_STATE_LISTENING)
                        {
                            SOPC_Internal_TriggerEventToSocketsManager(uaSock, INT_SOCKET_LISTENER_CONNECTION_ATTEMPT,
                                                                       uaSock->socketIdx);
                        }
                        else
                        {
                            SOPC_Logger_TraceError("SocketNetworkMgr: unexpected read event on socketIdx=%" PRIu32,
                                                   idx);
                            SOPC_Internal_TriggerEventToSocketsManager(uaSock, INT_SOCKET_CLOSE, idx);
                        }
                    }
                    else if (SocketSet_IsPresent(uaSock->sock, &writeSet) != false)
                    {
                        if (uaSock->state == SOCKET_STATE_CONNECTED)
                        {
                            SOPC_Internal_TriggerEventToSocketsManager(uaSock, INT_SOCKET_READY_TO_WRITE, idx);
                        }
                        else
                        {
                            SOPC_Logger_TraceError("SocketNetworkMgr: unexpected write event on socketIdx=%" PRIu32,
                                                   idx);
                            SOPC_Internal_TriggerEventToSocketsManager(uaSock, INT_SOCKET_CLOSE, idx);
                        }
                    }
                }

                // In any state check EXCEPT events
                if (SocketSet_IsPresent(uaSock->sock, &exceptSet) != false)
                {
                    // TODO: retrieve exception code
                    SOPC_Logger_TraceError("SocketNetworkMgr: exception event on socketIdx=%" PRIu32, idx);
                    SOPC_Internal_TriggerEventToSocketsManager(uaSock, INT_SOCKET_CLOSE, idx);
                }
            }
        }
    }

    Mutex_Unlock(&socketsMutex);

    return result;
}

static void* SOPC_SocketsNetworkEventMgr_CyclicThreadLoop(void* nullData)
{
    (void) nullData;
    while (receptionThread.stopFlag == false)
    {
        SOPC_SocketsNetworkEventMgr_TreatSocketsEvents(SOPC_MAX_CYCLE_TIMEOUT_MS);
        SOPC_EventTimer_CyclicTimersEvaluation();
        // Sleep used to avoid select (used by TreatSocketsEvents) to block other threads
        SOPC_Sleep(1);
    }
    return NULL;
}

static bool SOPC_SocketsNetworkEventMgr_CyclicThreadStart(void)
{
    bool result = false;
    if (receptionThread.initDone == false)
    {
        Mutex_Initialization(&receptionThread.tMutex);
        Mutex_Lock(&receptionThread.tMutex);
        receptionThread.initDone = true;
        receptionThread.stopFlag = false;
        SOPC_Thread_Create(&receptionThread.thread, SOPC_SocketsNetworkEventMgr_CyclicThreadLoop, NULL);
        Mutex_Unlock(&receptionThread.tMutex);
        result = true;
    }
    return result;
}

static void SOPC_SocketsNetworkEventMgr_CyclicThreadStop(void)
{
    if (receptionThread.initDone != false)
    {
        Mutex_Lock(&receptionThread.tMutex);
        // stop the reception thread
        receptionThread.stopFlag = true;
        if (receptionThread.stopFlag != false)
        {
            SOPC_Thread_Join(receptionThread.thread);
        }
        Mutex_Unlock(&receptionThread.tMutex);
        Mutex_Clear(&receptionThread.tMutex);
        receptionThread.initDone = false;
    }
}

void SOPC_SocketsNetworkEventMgr_Initialize()
{
    SOPC_SocketsNetworkEventMgr_CyclicThreadStart();
}

void SOPC_SocketsNetworkEventMgr_Clear()
{
    SOPC_SocketsNetworkEventMgr_CyclicThreadStop();
}
