/*
 *  Copyright (C) 2016 Systerel and others.
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

#include <unistd.h>
#include <errno.h>

#include "sopc_mutexes.h"
#include "sopc_threads.h"

SOPC_StatusCode Condition_Init(Condition* cond){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    int retCode = 0;
    if(cond != NULL){
        retCode = pthread_cond_init(cond, NULL);
        if(retCode == 0){
            status = STATUS_OK;
        }else{
            status = STATUS_NOK;
        }
    }
    return status;
}

SOPC_StatusCode Condition_Clear(Condition* cond){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    int retCode = 0;
    if(cond != NULL){
        retCode = pthread_cond_destroy(cond);
        if(retCode == 0){
            status = STATUS_OK;
        }else{
            status = STATUS_NOK;
        }
    }
    return status;
}

SOPC_StatusCode Condition_SignalAll(Condition* cond){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    int retCode = 0;
    if(cond != NULL){
        retCode = pthread_cond_broadcast(cond);
        if(retCode == 0){
            status = STATUS_OK;
        }else{
            status = STATUS_NOK;
        }
    }
    return status;
}

SOPC_StatusCode Mutex_Initialization(Mutex* mut){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    int retCode = 0;
    if(mut != NULL){
        retCode = pthread_mutex_init(mut, NULL);
        if(retCode == 0){
            status = STATUS_OK;
        }else{
            status = STATUS_NOK;
        }
    }
    return status;
}

SOPC_StatusCode Mutex_Clear(Mutex* mut){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    int retCode = 0;
    if(mut != NULL){
        retCode = pthread_mutex_destroy(mut);
        if(retCode == 0){
            status = STATUS_OK;
        }else{
            status = STATUS_NOK;
        }
    }
    return status;
}

SOPC_StatusCode Mutex_Lock(Mutex* mut){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    int retCode = 0;
    if(mut != NULL){
        retCode = pthread_mutex_lock(mut);
        if(retCode == 0){
            status = STATUS_OK;
        }else{
            status = STATUS_NOK;
        }
    }
    return status;
}

SOPC_StatusCode Mutex_Unlock(Mutex* mut){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    int retCode = 0;
    if(mut != NULL){
        retCode = pthread_mutex_unlock(mut);
        if(retCode == 0){
            status = STATUS_OK;
        }else{
            status = STATUS_NOK;
        }
    }
    return status;
}

SOPC_StatusCode Mutex_UnlockAndWaitCond(Condition* cond, Mutex* mut){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    int retCode = 0;
    if(NULL != cond && NULL != mut){
        retCode = pthread_cond_wait(cond, mut);
        if(retCode == 0){
            status = STATUS_OK;
        }else{
            status = STATUS_NOK;
        }
    }
    return status;
}

SOPC_StatusCode Mutex_UnlockAndTimedWaitCond(Condition* cond, Mutex* mut, uint32_t milliSecs){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    struct timespec absoluteTimeout;

    int retCode = 0;
    if(NULL != cond && NULL != mut && milliSecs > 0){
        // Retrieve current time
        clock_gettime(CLOCK_REALTIME, &absoluteTimeout);
        absoluteTimeout.tv_sec = absoluteTimeout.tv_sec+(milliSecs/1000);
        absoluteTimeout.tv_nsec = absoluteTimeout.tv_nsec+(milliSecs%1000);

        retCode = pthread_cond_timedwait(cond, mut, &absoluteTimeout);
        if(retCode == 0){
            status = STATUS_OK;
        }else if(ETIMEDOUT == retCode){
            status = OpcUa_BadTimeout;
        }else{
            status = STATUS_NOK;
        }
    }
    return status;
}

SOPC_StatusCode SOPC_Thread_Create(Thread* thread, void *(*startFct) (void *), void *startArgs){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(thread != NULL && startFct != NULL){
        if(pthread_create(thread, NULL, startFct, startArgs) == 0){
            status = STATUS_OK;
        }else{
            status = STATUS_NOK;
        }
    }
    return status;
}

SOPC_StatusCode SOPC_Thread_Join(Thread thread){
    SOPC_StatusCode status = STATUS_NOK;
    if(pthread_join(thread, NULL) == 0){
        status = STATUS_OK;
    }
    return status;
}

void SOPC_Sleep(unsigned int milliseconds){
    usleep(milliseconds * 1000);
}
