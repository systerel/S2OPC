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

#include "sopc_mutexes.h"
#include "sopc_threads.h"

SOPC_StatusCode Mutex_Inititalization(Mutex* mut){
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
