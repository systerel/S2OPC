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

#include "sopc_mutexes.h"
#include "sopc_threads.h"
#include "sopc_time.h"

SOPC_ReturnStatus Condition_Init(Condition* cond)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (cond != NULL)
    {
        InitializeConditionVariable(cond);
        status = SOPC_STATUS_OK;
    }
    return status;
}

SOPC_ReturnStatus Condition_Clear(Condition* cond)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (cond != NULL)
    {
        status = SOPC_STATUS_OK;
    }
    return status;
}

SOPC_ReturnStatus Condition_SignalAll(Condition* cond)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (cond != NULL)
    {
        WakeAllConditionVariable(cond);
        status = SOPC_STATUS_OK;
    }
    return status;
}

SOPC_ReturnStatus Mutex_Initialization(Mutex* mut)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (mut != NULL)
    {
        InitializeSRWLock(mut);
        status = SOPC_STATUS_OK;
    }
    return status;
}

SOPC_ReturnStatus Mutex_Clear(Mutex* mut)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (mut != NULL)
    {
        status = SOPC_STATUS_OK;
    }
    return status;
}

SOPC_ReturnStatus Mutex_Lock(Mutex* mut)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (mut != NULL)
    {
        AcquireSRWLockExclusive(mut);
        status = SOPC_STATUS_OK;
    }
    return status;
}

SOPC_ReturnStatus Mutex_Unlock(Mutex* mut)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (mut != NULL)
    {
        ReleaseSRWLockExclusive(mut);
        status = SOPC_STATUS_OK;
    }
    return status;
}

SOPC_ReturnStatus Mutex_UnlockAndWaitCond(Condition* cond, Mutex* mut)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (cond != NULL && mut != NULL)
    {
        BOOL res = SleepConditionVariableSRW(cond, mut, INFINITE, 0);
        if (res == 0)
        {
            // Possible to retrieve error with GetLastError (see msdn doc)
            status = SOPC_STATUS_NOK;
        }
        else
        {
            status = SOPC_STATUS_OK;
        }
    }
    return status;
}

SOPC_ReturnStatus Mutex_UnlockAndTimedWaitCond(Condition* cond, Mutex* mut, uint32_t milliSecs)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (cond != NULL && mut != NULL && milliSecs > 0)
    {
        BOOL res = SleepConditionVariableSRW(cond, mut, (DWORD) milliSecs, 0);
        if (res == 0)
        {
            status = SOPC_STATUS_NOK;
            if (ERROR_TIMEOUT == GetLastError())
            {
                status = SOPC_STATUS_TIMEOUT;
            }
        }
        else
        {
            status = SOPC_STATUS_OK;
        }
    }
    return status;
}

DWORD WINAPI SOPC_Thread_StartFct(LPVOID args)
{
    Thread* thread = (Thread*) args;
    // void* res =
    thread->startFct(thread->args);
    // TODO: deal with returned value ?
    return 0;
}

SOPC_ReturnStatus SOPC_Thread_Create(Thread* thread, void* (*startFct)(void*), void* startArgs)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    DWORD threadId = 0;
    if (thread != NULL && startFct != NULL)
    {
        thread->args = startArgs;
        thread->startFct = startFct;
        thread->thread = CreateThread(NULL, // default security attributes
                                      0,    // use default stack size
                                      SOPC_Thread_StartFct, thread,
                                      0, // use default creation flags
                                      &threadId);
        if (NULL == thread->thread)
        {
            status = SOPC_STATUS_NOK;
        }
        else
        {
            status = SOPC_STATUS_OK;
        }
    }
    else if (thread != NULL)
    {
        thread->thread = NULL;
    }
    return status;
}

SOPC_ReturnStatus SOPC_Thread_Join(Thread thread)
{
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    if (thread.thread != NULL)
    {
        DWORD retCode = WaitForSingleObject(thread.thread, INFINITE);
        if (WAIT_OBJECT_0 == retCode)
        {
            thread.thread = NULL;
            status = SOPC_STATUS_OK;
        }
    }
    return status;
}

void SOPC_Sleep(unsigned int milliseconds)
{
    Sleep(milliseconds);
}
