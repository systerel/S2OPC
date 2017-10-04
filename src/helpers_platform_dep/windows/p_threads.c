/*
 *  Copyright (C) 2017 Systerel and others.
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

#include "opcua_statuscodes.h"

#include "sopc_mutexes.h"
#include "sopc_threads.h"
#include "sopc_time.h"

SOPC_StatusCode Condition_Init(Condition* cond){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(cond != NULL){
        InitializeConditionVariable(cond);
        status = STATUS_OK;
    }
    return status;
}

SOPC_StatusCode Condition_Clear(Condition* cond){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(cond != NULL){
        status = STATUS_OK;
    }
    return status;
}

SOPC_StatusCode Condition_SignalAll(Condition* cond){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(cond != NULL){
        WakeAllConditionVariable(cond);
        status = STATUS_OK;
    }
    return status;
}

SOPC_StatusCode Mutex_Initialization(Mutex* mut){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(mut != NULL){
        InitializeSRWLock(mut);
        status = STATUS_OK;
    }
    return status;
}

SOPC_StatusCode Mutex_Clear(Mutex* mut){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(mut != NULL){
        status = STATUS_OK;
    }
    return status;
}

SOPC_StatusCode Mutex_Lock(Mutex* mut){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(mut != NULL){
        AcquireSRWLockExclusive(mut);
        status = STATUS_OK;
    }
    return status;
}

SOPC_StatusCode Mutex_Unlock(Mutex* mut){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(mut != NULL){
        ReleaseSRWLockExclusive(mut);
        status = STATUS_OK;
    }
    return status;
}

SOPC_StatusCode Mutex_UnlockAndWaitCond(Condition* cond, Mutex* mut){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(cond != NULL && mut != NULL){
        BOOL res = SleepConditionVariableSRW (cond, mut, INFINITE, 0);
        if(res == 0){
            // Possible to retrieve error with GetLastError (see msdn doc)
            status = STATUS_NOK;
        }else{
            status = STATUS_OK;
        }
    }
    return status;
}

SOPC_StatusCode Mutex_UnlockAndTimedWaitCond(Condition* cond, Mutex* mut, uint32_t milliSecs){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(cond != NULL && mut != NULL && milliSecs > 0){
        BOOL res = SleepConditionVariableSRW (cond, mut, (DWORD) milliSecs, 0);
        if(res == 0){
            status = STATUS_NOK;
            if(ERROR_TIMEOUT == GetLastError()){
                status = OpcUa_BadTimeout;
            }
        }else{
            status = STATUS_OK;
        }
    }
    return status;
}

DWORD WINAPI SOPC_Thread_StartFct(LPVOID args){
    Thread* thread = (Thread*) args;
    //void* res =
    thread->startFct(thread->args);
    // TODO: deal with returned value ?
    return 0;
}

SOPC_StatusCode SOPC_Thread_Create(Thread* thread, void *(*startFct) (void *), void *startArgs){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    DWORD threadId = 0;
    if(thread != NULL && startFct != NULL){
        thread->args = startArgs;
        thread->startFct = startFct;
    	thread->thread = CreateThread(NULL,       // default security attributes
    						          0,          // use default stack size
    						          SOPC_Thread_StartFct,
							          thread,
							          0,          // use default creation flags
							          &threadId);
    	if(thread->thread == NULL){
    		status = STATUS_NOK;
    	}else{
    		status = STATUS_OK;
    	}
    }else if(thread != NULL){
    	thread->thread = NULL;
    }
    return status;
}

SOPC_StatusCode SOPC_Thread_Join(Thread thread){
    SOPC_StatusCode status = STATUS_NOK;
    if(thread.thread != NULL){
		DWORD retCode = WaitForSingleObject(thread.thread, INFINITE);
		if(retCode == WAIT_OBJECT_0){
			thread.thread = NULL;
			status = STATUS_OK;
		}
    }
    return status;
}

void SOPC_Sleep(unsigned int milliseconds){
    Sleep(milliseconds);
}
