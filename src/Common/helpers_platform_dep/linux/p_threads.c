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
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "sopc_mutexes.h"
#include "sopc_threads.h"
#include "sopc_time.h"

// 10^9
#define SOPC_SECOND_TO_NANOSECONDS 1000000000
// 10^6
#define SOPC_MILLISECOND_TO_NANOSECONDS 1000000

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
        uint64_t nanoseconds = (milliSecs % 1000) * SOPC_MILLISECOND_TO_NANOSECONDS;

        assert(absoluteTimeout.tv_nsec < SOPC_SECOND_TO_NANOSECONDS);
        if (SOPC_SECOND_TO_NANOSECONDS - (uint64_t) absoluteTimeout.tv_nsec < nanoseconds)
        {
            // Additional second
            absoluteTimeout.tv_sec += 1;
            absoluteTimeout.tv_nsec = (long) nanoseconds - (SOPC_SECOND_TO_NANOSECONDS - absoluteTimeout.tv_nsec);
        }
        else
        {
            absoluteTimeout.tv_nsec = absoluteTimeout.tv_nsec + (long) nanoseconds;
        }

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

static inline SOPC_ReturnStatus create_thread(Thread* thread,
                                              pthread_attr_t* attr,
                                              void* (*startFct)(void*),
                                              void* startArgs,
                                              const char* taskName)
{
    int ret = pthread_create(thread, attr, startFct, startArgs);

    SOPC_ReturnStatus status = (0 == ret) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
    if (SOPC_STATUS_OK != status)
    {
        fprintf(stderr, "Error cannot create thread: %d\n", ret);
    }
    else
    {
        const char* name = taskName;
        char tmpTaskName[16];

        if (strlen(taskName) >= 16)
        {
            strncpy(tmpTaskName, taskName, 15);
            tmpTaskName[15] = '\0';
            name = tmpTaskName;
        }

        /* pthread_setname_np calls can fail. It is not a sufficient reason to stop processing. */
        ret = pthread_setname_np(*thread, name);
        if (0 != ret)
        {
            fprintf(stderr, "Error during set name \"%s\" to thread: %d\n", taskName, ret);
        }
    }

    return status;
}

SOPC_ReturnStatus SOPC_Thread_Create(Thread* thread, void* (*startFct)(void*), void* startArgs, const char* taskName)
{
    if (NULL == thread || NULL == startFct)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    return create_thread(thread, NULL, startFct, startArgs, taskName);
}

SOPC_ReturnStatus SOPC_Thread_CreatePrioritized(Thread* thread,
                                                void* (*startFct)(void*),
                                                void* startArgs,
                                                int priority,
                                                const char* taskName)
{
    if (NULL == thread || NULL == startFct || priority < 1 || priority > 99)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    /* Initialize scheduling policy and priority */
    pthread_attr_t attr;
    int ret = pthread_attr_init(&attr);
    if (0 != ret)
    {
        fprintf(stderr, "Could not initialize pthread attributes: %d\n", ret);
    }
    if (0 == ret)
    {
        ret = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
        if (0 != ret)
        {
            fprintf(stderr, "Could not unset scheduler inheritance in thread creation attributes: %d\n", ret);
        }
    }
    if (0 == ret)
    {
        ret = pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
        if (0 != ret)
        {
            fprintf(stderr, "Could not set thread scheduling policy: %d\n", ret);
        }
    }
    if (0 == ret)
    {
        struct sched_param scp;
        scp.sched_priority = priority;
        ret = pthread_attr_setschedparam(&attr, &scp);
        if (0 != ret)
        {
            fprintf(stderr, "Could not set thread priority: %d\n", ret);
        }
    }

    /* Create thread with created attributes */
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    if (0 == ret)
    {
        status = create_thread(thread, &attr, startFct, startArgs, taskName);
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
