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
#include <errno.h>
#include <unistd.h>

#include "sopc_mutexes.h"
#include "sopc_threads.h"
#include "sopc_time.h"

SOPC_ReturnStatus Condition_Init(Condition* cond)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    int retCode = 0;
    if (cond != NULL)
    {
        retCode = pthread_cond_init(cond, NULL);
        if (retCode == 0)
        {
            status = SOPC_STATUS_OK;
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
    }
    return status;
}

SOPC_ReturnStatus Condition_Clear(Condition* cond)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    int retCode = 0;
    if (cond != NULL)
    {
        retCode = pthread_cond_destroy(cond);
        if (retCode == 0)
        {
            status = SOPC_STATUS_OK;
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
    }
    return status;
}

SOPC_ReturnStatus Condition_SignalAll(Condition* cond)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    int retCode = 0;
    if (cond != NULL)
    {
        retCode = pthread_cond_broadcast(cond);
        if (retCode == 0)
        {
            status = SOPC_STATUS_OK;
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
    }
    return status;
}

SOPC_ReturnStatus Mutex_Initialization(Mutex* mut)
{
    assert(NULL != mut);

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    pthread_mutexattr_t attr;

    if (pthread_mutexattr_init(&attr) != 0)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    if (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE) != 0)
    {
        status = SOPC_STATUS_NOK;
    }
    if (SOPC_STATUS_OK == status)
    {
        if (pthread_mutex_init(mut, &attr) != 0)
        {
            status = SOPC_STATUS_NOK;
        }
    }

    pthread_mutexattr_destroy(&attr);

    return status;
}

SOPC_ReturnStatus Mutex_Clear(Mutex* mut)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    int retCode = 0;
    if (mut != NULL)
    {
        retCode = pthread_mutex_destroy(mut);
        if (retCode == 0)
        {
            status = SOPC_STATUS_OK;
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
    }
    return status;
}

SOPC_ReturnStatus Mutex_Lock(Mutex* mut)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    int retCode = 0;
    if (mut != NULL)
    {
        retCode = pthread_mutex_lock(mut);
        if (retCode == 0)
        {
            status = SOPC_STATUS_OK;
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
    }
    return status;
}

SOPC_ReturnStatus Mutex_Unlock(Mutex* mut)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    int retCode = 0;
    if (mut != NULL)
    {
        retCode = pthread_mutex_unlock(mut);
        if (retCode == 0)
        {
            status = SOPC_STATUS_OK;
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
    }
    return status;
}

SOPC_ReturnStatus Mutex_UnlockAndWaitCond(Condition* cond, Mutex* mut)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    int retCode = 0;
    if (NULL != cond && NULL != mut)
    {
        retCode = pthread_cond_wait(cond, mut);
        if (retCode == 0)
        {
            status = SOPC_STATUS_OK;
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
    }
    return status;
}

SOPC_ReturnStatus Mutex_UnlockAndTimedWaitCond(Condition* cond, Mutex* mut, uint32_t milliSecs)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    struct timespec absoluteTimeout;

    int retCode = 0;
    if (NULL != cond && NULL != mut && milliSecs > 0)
    {
        // Retrieve current time
        clock_gettime(CLOCK_REALTIME, &absoluteTimeout);
        absoluteTimeout.tv_sec = absoluteTimeout.tv_sec + (time_t)(milliSecs / 1000);
        absoluteTimeout.tv_nsec = absoluteTimeout.tv_nsec + (long) (milliSecs % 1000);

        retCode = pthread_cond_timedwait(cond, mut, &absoluteTimeout);
        if (retCode == 0)
        {
            status = SOPC_STATUS_OK;
        }
        else if (ETIMEDOUT == retCode)
        {
            status = SOPC_STATUS_TIMEOUT;
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
    }
    return status;
}

SOPC_ReturnStatus SOPC_Thread_Create(Thread* thread, void* (*startFct)(void*), void* startArgs)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (thread != NULL && startFct != NULL)
    {
        if (pthread_create(thread, NULL, startFct, startArgs) == 0)
        {
            status = SOPC_STATUS_OK;
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
    }
    return status;
}

SOPC_ReturnStatus SOPC_Thread_Join(Thread thread)
{
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    if (pthread_join(thread, NULL) == 0)
    {
        status = SOPC_STATUS_OK;
    }
    return status;
}

void SOPC_Sleep(unsigned int milliseconds)
{
    usleep(milliseconds * 1000);
}
