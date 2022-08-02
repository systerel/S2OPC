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
#include <stdlib.h>
#include <winerror.h>

#include "sopc_mem_alloc.h"
#include "sopc_mutexes.h"
#include "sopc_threads.h"
#include "sopc_time.h"

typedef HRESULT(WINAPI* pSetThreadDescription)(HANDLE, PCWSTR);

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
        InitializeCriticalSection(mut);
        status = SOPC_STATUS_OK;
    }
    return status;
}

SOPC_ReturnStatus Mutex_Clear(Mutex* mut)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (mut != NULL)
    {
        DeleteCriticalSection(mut);
        status = SOPC_STATUS_OK;
    }
    return status;
}

SOPC_ReturnStatus Mutex_Lock(Mutex* mut)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (mut != NULL)
    {
        EnterCriticalSection(mut);
        status = SOPC_STATUS_OK;
    }
    return status;
}

SOPC_ReturnStatus Mutex_Unlock(Mutex* mut)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (mut != NULL)
    {
        LeaveCriticalSection(mut);
        status = SOPC_STATUS_OK;
    }
    return status;
}

SOPC_ReturnStatus Mutex_UnlockAndWaitCond(Condition* cond, Mutex* mut)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (cond != NULL && mut != NULL)
    {
        BOOL res = SleepConditionVariableCS(cond, mut, INFINITE);
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
        BOOL res = SleepConditionVariableCS(cond, mut, (DWORD) milliSecs);
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

static DWORD WINAPI SOPC_Thread_StartFct(LPVOID args)
{
    assert(args != NULL);
    Thread* thread = (Thread*) args;
    void* returnValue = thread->startFct(thread->args);
    assert(NULL == returnValue);
    return 0;
}

SOPC_ReturnStatus SOPC_Thread_Create(Thread* thread, void* (*startFct)(void*), void* startArgs, const char* taskName)
{
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    DWORD threadId = 0;

    if (NULL == thread)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (NULL == startFct)
    {
        thread->thread = NULL;
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

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
        /* API is available starting from Windows 10 */
        /* We need to check if the API is available at execution time */
        HMODULE kernel32 = LoadLibraryW(L"kernel32");
        assert(kernel32 != NULL);
        pSetThreadDescription funcAddress = (pSetThreadDescription) GetProcAddress(kernel32, "SetThreadDescription");

        if (NULL != funcAddress)
        {
            size_t returnValue;
            size_t taskNameLength = strlen(taskName);
            size_t bytesRequired = (taskNameLength + 1) * sizeof(wchar_t);
            wchar_t* wcstr = SOPC_Malloc(bytesRequired);
            errno_t ret = mbstowcs_s(&returnValue, wcstr, taskNameLength + 1, taskName, taskNameLength);

            if (0 == ret)
            {
                HRESULT result = funcAddress(thread->thread, wcstr);
                if (SUCCEEDED(result))
                {
                    status = SOPC_STATUS_OK;
                }
                else
                {
                    SOPC_Free(wcstr);
                    status = SOPC_STATUS_NOK;
                }
            }
            else
            {
                SOPC_Free(wcstr);
                status = SOPC_STATUS_NOK;
            }
        }
        else
        {
            status = SOPC_STATUS_OK;
        }
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
