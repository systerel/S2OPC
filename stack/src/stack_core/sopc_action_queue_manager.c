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

#include "sopc_action_queue_manager.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "sopc_threads.h"

SOPC_ActionQueueManager* stackActionQueueMgr = NULL;
SOPC_ActionQueueManager* appCallbackQueueMgr = NULL;

struct SOPC_ActionQueueManager {
    SOPC_ActionQueue* queue;
    uint8_t           stopMgr;
    Thread            mgrThread;
};

void* SOPC_ThreadStartManager(void* pQueueMgr)
{
    assert(NULL != pQueueMgr);
    SOPC_ActionQueueManager* pMgr = (SOPC_ActionQueueManager *) pQueueMgr;
    uint8_t localStopMgr = FALSE;
    SOPC_StatusCode status = STATUS_NOK;
    SOPC_ActionFunction* fctPointer = NULL;
    void* fctArgument = NULL;
    const char* txt = NULL;
    while(localStopMgr == FALSE){
        status = SOPC_Action_BlockingDequeue(pMgr->queue, &fctPointer, &fctArgument, &txt);
        if(STATUS_OK == status){
            if(fctPointer != NULL){
                fctPointer(fctArgument);
            }else if(fctArgument == &pMgr->stopMgr){ // It is the stop flag address
                localStopMgr = 1;
            }else{
                status = STATUS_NOK;
            }
        }
    }
    return NULL;
}

SOPC_ActionQueueManager* SOPC_ActionQueueManager_CreateAndStart(){
    SOPC_StatusCode status = STATUS_NOK;
    SOPC_ActionQueueManager* pQueueMgr = NULL;
    pQueueMgr = malloc(sizeof(SOPC_ActionQueueManager));
    if(NULL != pQueueMgr){
        status = SOPC_ActionQueue_Init(&pQueueMgr->queue);
        if(STATUS_OK == status){
            pQueueMgr->stopMgr = FALSE;
            status = SOPC_Thread_Create(&pQueueMgr->mgrThread,
                                        SOPC_ThreadStartManager,
                                        (void*) pQueueMgr);
        }
        if(STATUS_OK != status){
            free(pQueueMgr);
            pQueueMgr = NULL;
        }
    }
    return pQueueMgr;
}

SOPC_StatusCode SOPC_ActionQueueManager_AddAction(SOPC_ActionQueueManager* queueMgr,
                                                  SOPC_ActionFunction*     fctPointer,
                                                  void*                    fctArgument,
                                                  const char*              actionText)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(NULL != queueMgr){
        status = STATUS_INVALID_STATE;
        if(queueMgr->stopMgr == FALSE){
            status = SOPC_Action_BlockingEnqueue(queueMgr->queue, fctPointer, fctArgument, actionText);
        }
    }
    return status;
}

SOPC_StatusCode SOPC_ActionQueueManager_StopAndDelete(SOPC_ActionQueueManager** queueMgr){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(NULL != queueMgr && NULL != *queueMgr){
        status = STATUS_INVALID_STATE;
        if((*queueMgr)->stopMgr == FALSE){
            (*queueMgr)->stopMgr = 1;
            // Use stopMgr flag address as indicator all precedent actions were treated
            status = SOPC_Action_BlockingEnqueue((*queueMgr)->queue, NULL, &(*queueMgr)->stopMgr, NULL);
        }
        if(STATUS_OK == status){
            status = SOPC_Thread_Join((*queueMgr)->mgrThread);
        }
        if(STATUS_OK == status){
            SOPC_ActionQueue_Free(&(*queueMgr)->queue);
            free(*queueMgr);
            *queueMgr = NULL;
        }
    }
    return status;
}
