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

#include <stdlib.h>
#include <winerror.h>

#include "p_sopc_threads.h"

#include "sopc_assert.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_mutexes.h"
#include "sopc_threads.h"

typedef HRESULT(WINAPI* pSetThreadDescription)(HANDLE, PCWSTR);

SOPC_ReturnStatus SOPC_Condition_Init(SOPC_Condition* cond)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL != cond)
    {
        struct SOPC_Condition_Impl* condI = SOPC_Calloc(1, sizeof(*condI));

        if (SOPC_INVALID_COND == condI)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
        else
        {
            InitializeConditionVariable(&condI->cond);
            status = SOPC_STATUS_OK;
        }
        *cond = condI;
    }
    return status;
}

SOPC_ReturnStatus SOPC_Condition_Clear(SOPC_Condition* cond)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (cond != NULL)
    {
        status = SOPC_STATUS_OK;
        SOPC_Free(*cond);
        *cond = SOPC_INVALID_COND;
    }
    return status;
}

SOPC_ReturnStatus SOPC_Condition_SignalAll(SOPC_Condition* cond)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (cond != NULL)
    {
        struct SOPC_Condition_Impl* condI = (SOPC_Condition_Impl*) (*cond);
        SOPC_ASSERT(SOPC_INVALID_COND != condI); // see SOPC_Condition_Init

        WakeAllConditionVariable(&condI->cond);
        status = SOPC_STATUS_OK;
    }
    return status;
}

SOPC_ReturnStatus SOPC_Mutex_Initialization(SOPC_Mutex* mut)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (mut != NULL)
    {
        struct SOPC_Mutex_Impl* mutI = SOPC_Calloc(1, sizeof(*mutI));
        SOPC_ASSERT(SOPC_INVALID_MUTEX != mutI); // See SOPC_Mutex_Initialization

        InitializeCriticalSection(&mutI->mutex);
        status = SOPC_STATUS_OK;
        *mut = mutI;
    }
    return status;
}

SOPC_ReturnStatus SOPC_Mutex_Clear(SOPC_Mutex* mut)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (mut != NULL)
    {
        struct SOPC_Mutex_Impl* mutI = (SOPC_Mutex_Impl*) (*mut);
        SOPC_ASSERT(SOPC_INVALID_MUTEX != mutI); // See SOPC_Mutex_Initialization
        DeleteCriticalSection(&mutI->mutex);
        status = SOPC_STATUS_OK;
        SOPC_Free(mutI);
    }
    return status;
}

SOPC_ReturnStatus SOPC_Mutex_Lock(SOPC_Mutex* mut)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (mut != NULL)
    {
        struct SOPC_Mutex_Impl* mutI = (SOPC_Mutex_Impl*) (*mut);
        SOPC_ASSERT(SOPC_INVALID_MUTEX != mutI); // See SOPC_Mutex_Initialization
        EnterCriticalSection(&mutI->mutex);
        status = SOPC_STATUS_OK;
    }
    return status;
}

SOPC_ReturnStatus SOPC_Mutex_Unlock(SOPC_Mutex* mut)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (mut != NULL)
    {
        struct SOPC_Mutex_Impl* mutI = (SOPC_Mutex_Impl*) (*mut);
        SOPC_ASSERT(SOPC_INVALID_MUTEX != mutI); // See SOPC_Mutex_Initialization
        LeaveCriticalSection(&mutI->mutex);
        status = SOPC_STATUS_OK;
    }
    return status;
}

SOPC_ReturnStatus SOPC_Mutex_UnlockAndWaitCond(SOPC_Condition* cond, SOPC_Mutex* mut)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL != cond && NULL != mut)
    {
        struct SOPC_Mutex_Impl* mutI = (SOPC_Mutex_Impl*) (*mut);
        struct SOPC_Condition_Impl* condI = (SOPC_Condition_Impl*) (*cond);
        SOPC_ASSERT(SOPC_INVALID_COND != condI && SOPC_INVALID_MUTEX != mutI);

        BOOL res = SleepConditionVariableCS(&condI->cond, &mutI->mutex, INFINITE);
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

SOPC_ReturnStatus SOPC_Mutex_UnlockAndTimedWaitCond(SOPC_Condition* cond, SOPC_Mutex* mut, uint32_t milliSecs)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (cond != NULL && mut != NULL && milliSecs > 0)
    {
        struct SOPC_Mutex_Impl* mutI = (SOPC_Mutex_Impl*) (*mut);
        struct SOPC_Condition_Impl* condI = (SOPC_Condition_Impl*) (*cond);
        SOPC_ASSERT(SOPC_INVALID_COND != condI && SOPC_INVALID_MUTEX != mutI);

        BOOL res = SleepConditionVariableCS(&condI->cond, &mutI->mutex, (DWORD) milliSecs);
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
    SOPC_ASSERT(args != NULL);
    SOPC_Thread thread = (SOPC_Thread) args;
    void* returnValue = thread->startFct(thread->args);
    SOPC_ASSERT(NULL == returnValue);
    return 0;
}

SOPC_ReturnStatus SOPC_Thread_Create(SOPC_Thread* thread,
                                     void* (*startFct)(void*),
                                     void* startArgs,
                                     const char* taskName)
{
    if (NULL == thread || NULL == startFct)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_Thread_Impl* threadImpl = SOPC_Calloc(1, sizeof(*threadImpl));

    if (SOPC_INVALID_THREAD == threadImpl)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    DWORD threadId = 0;

    threadImpl->args = startArgs;
    threadImpl->startFct = startFct;
    threadImpl->thread = CreateThread(NULL, // default security attributes
                                      0,    // use default stack size
                                      SOPC_Thread_StartFct, threadImpl,
                                      0, // use default creation flags
                                      &threadId);
    if (NULL == threadImpl->thread)
    {
        status = SOPC_STATUS_NOK;
    }
    else
    {
        /* API is available starting from Windows 10 */
        /* We need to check if the API is available at execution time */
        HMODULE kernel32 = LoadLibraryW(L"kernel32");
        SOPC_ASSERT(kernel32 != NULL);
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
                HRESULT result = funcAddress(threadImpl->thread, wcstr);
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
    if (SOPC_STATUS_OK == status)
    {
        *thread = threadImpl;
    }
    else
    {
        SOPC_Free(threadImpl);
    }

    return status;
}

SOPC_ReturnStatus SOPC_Thread_CreatePrioritized(SOPC_Thread* thread,
                                                void* (*startFct)(void*),
                                                void* startArgs,
                                                int priority,
                                                const char* taskName)
{
    // Windows doesn't support SCHED_FIFO or real time priority in a simple manner
    SOPC_UNUSED_ARG(priority);
    return SOPC_Thread_Create(thread, startFct, startArgs, taskName);
}

SOPC_ReturnStatus SOPC_Thread_Join(SOPC_Thread* thread)
{
    if (NULL == thread)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    SOPC_Thread_Impl* threadImpl = *thread;
    if (SOPC_INVALID_THREAD != threadImpl && threadImpl->thread != NULL)
    {
        DWORD retCode = WaitForSingleObject(threadImpl->thread, INFINITE);
        if (WAIT_OBJECT_0 == retCode)
        {
            threadImpl->thread = NULL;
            SOPC_Free(threadImpl);
            *thread = SOPC_INVALID_THREAD;
            status = SOPC_STATUS_OK;
        }
    }
    return status;
}

void SOPC_Sleep(unsigned int milliseconds)
{
    Sleep(milliseconds);
}
