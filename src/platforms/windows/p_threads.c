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

#include "sopc_mutexes.h"
#include "sopc_threads.h"

SOPC_StatusCode Mutex_Inititalization(Mutex* mut){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(mut != NULL && *mut == NULL){
        *mut = CreateMutex(NULL,              // default security attributes
                           FALSE,             // initially not owned
                           NULL);             // unnamed mutex
        if(*mut != NULL){
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
    if(mut != NULL && *mut != NULL){
    	retCode = CloseHandle(*mut);
        if(retCode != 0){
            status = STATUS_OK;
        }else{
            status = STATUS_NOK;
        }
    }
    return status;
}

SOPC_StatusCode Mutex_Lock(Mutex* mut){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    DWORD retCode = 0;
    if(mut != NULL && *mut != NULL){
        retCode = WaitForSingleObject(*mut, INFINITE);
        if(retCode == WAIT_OBJECT_0){
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
    if(mut != NULL && *mut != NULL){
        retCode = ReleaseMutex(*mut);
        if(retCode != 0){
            status = STATUS_OK;
        }else{
            status = STATUS_NOK;
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
