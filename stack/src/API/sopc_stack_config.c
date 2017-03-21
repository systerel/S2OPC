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

#include "sopc_stack_config.h"

#include <stdlib.h>
#include <string.h>

#include "sopc_raw_sockets.h"
#include "sopc_types.h"
#include "sopc_sockets.h"
#include "platform_deps.h"
#include "sopc_action_queue_manager.h"
#include "sopc_threads.h"

typedef struct SOPC_StackConfiguration {
    SOPC_NamespaceTable*     nsTable;
    SOPC_EncodeableType**    encTypesTable;
    uint32_t                 nbEncTypesTable;
} SOPC_StackConfiguration;

static SOPC_StackConfiguration g_stackConfiguration;
uint8_t g_lockedConfig = FALSE;

static uint8_t initDone = FALSE;

static struct
{
    uint8_t  initDone;
    uint8_t  stopFlag;
    Mutex    tMutex;
    Thread   thread;
} receptionThread = {
    .initDone = FALSE,
    .stopFlag = FALSE,
};

void* SOPC_Stack_ReceptionThread_Loop(void* nullData){
    (void) nullData;
    const uint32_t sleepTimeout = 500;
    SOPC_StatusCode status = STATUS_OK;
    while(STATUS_OK == status && receptionThread.stopFlag == FALSE){
        status = SOPC_SocketManager_TreatSocketsEvents(SOPC_SocketManager_GetGlobal(),
                                         sleepTimeout);
        if(STATUS_OK == status){
            SOPC_Sleep(1);
        }
    }

    return NULL;
}

SOPC_StatusCode SOPC_Stack_ReceptionThread_Start(){
    SOPC_StatusCode status = STATUS_INVALID_STATE;
    if(receptionThread.initDone == FALSE){
        status = STATUS_OK;
        Mutex_Initialization(&receptionThread.tMutex);
        Mutex_Lock(&receptionThread.tMutex);
        receptionThread.initDone = 1;
        receptionThread.stopFlag = FALSE;
        SOPC_Thread_Create(&receptionThread.thread, SOPC_Stack_ReceptionThread_Loop, NULL);
        Mutex_Unlock(&receptionThread.tMutex);
    }
    return status;
}

void SOPC_Stack_ReceptionThread_Stop(){
    if(receptionThread.initDone != FALSE){
        Mutex_Lock(&receptionThread.tMutex);
        // stop the reception thread
        receptionThread.stopFlag = 1;
        if(receptionThread.stopFlag != FALSE){
            SOPC_Thread_Join(receptionThread.thread);
        }
        Mutex_Unlock(&receptionThread.tMutex);
        Mutex_Clear(&receptionThread.tMutex);
        receptionThread.initDone = FALSE;
    }
}

SOPC_StatusCode SOPC_StackConfiguration_Initialize(){
    SOPC_StatusCode status = STATUS_INVALID_STATE;
    if(initDone == FALSE){
        SOPC_StackConfiguration_Clear();
        Namespace_Initialize(g_stackConfiguration.nsTable);
        status = Socket_Network_Initialize();
        if(STATUS_OK == status){
            status = SOPC_SocketManager_Config_Init();
        }
        if(STATUS_OK == status){
            appCallbackQueueMgr = SOPC_ActionQueueManager_CreateAndStart();
            if(NULL == appCallbackQueueMgr){
                status = STATUS_NOK;
            }
        }
        if(STATUS_OK == status){
            stackActionQueueMgr = SOPC_ActionQueueManager_CreateAndStart();
            if(NULL == stackActionQueueMgr){
                status = STATUS_NOK;
            }
        }
        if(STATUS_OK == status){
            status = SOPC_Stack_ReceptionThread_Start();
        }
        InitPlatformDependencies();
        initDone = 1;
    }
    return status;
}

void SOPC_StackConfiguration_Locked(){
    g_lockedConfig = 1;
}

void SOPC_StackConfiguration_Unlocked(){
    g_lockedConfig = FALSE;
}

void SOPC_StackConfiguration_Clear(){
    if(g_stackConfiguration.encTypesTable != NULL){
        free(g_stackConfiguration.encTypesTable);
    }
    g_stackConfiguration.nsTable = NULL;
    g_stackConfiguration.encTypesTable = NULL;
    g_stackConfiguration.nbEncTypesTable = 0;
    SOPC_SocketManager_Config_Clear();
    Socket_Network_Clear();
    SOPC_ActionQueueManager_StopAndDelete(&stackActionQueueMgr);
    SOPC_ActionQueueManager_StopAndDelete(&appCallbackQueueMgr);
    SOPC_Stack_ReceptionThread_Stop();
    SOPC_StackConfiguration_Unlocked();
    initDone = FALSE;
}

SOPC_StatusCode SOPC_StackConfiguration_SetNamespaceUris(SOPC_NamespaceTable* nsTable){
    SOPC_StatusCode status = STATUS_INVALID_STATE;
    if(initDone != FALSE && g_lockedConfig == FALSE){
        if(nsTable == NULL){
            status = STATUS_INVALID_PARAMETERS;
        }else{
            g_stackConfiguration.nsTable = nsTable;
        }
    }
    return status;
}

static uint32_t GetKnownEncodeableTypesLength(){
    uint32_t result = 0;
    for(result = 0; SOPC_KnownEncodeableTypes[result] != NULL; result++);
    return result + 1;
}

SOPC_StatusCode SOPC_StackConfiguration_AddTypes(SOPC_EncodeableType** encTypesTable,
                                                 uint32_t              nbTypes){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    uint32_t idx = 0;
    uint32_t nbKnownTypes = 0;
    SOPC_EncodeableType** additionalTypes = NULL;

    if(initDone == FALSE  || g_lockedConfig != FALSE){
        return STATUS_INVALID_STATE;
    }

    if(encTypesTable != NULL && nbTypes > 0 ){
        status = STATUS_OK;
        if(g_stackConfiguration.encTypesTable == NULL){
            // known types to be added
            nbKnownTypes = GetKnownEncodeableTypesLength();
            // +1 for null value termination
            g_stackConfiguration.encTypesTable = malloc(sizeof(SOPC_EncodeableType*) * (nbKnownTypes + nbTypes + 1));
            if(g_stackConfiguration.encTypesTable == NULL ||
               g_stackConfiguration.encTypesTable != memcpy(g_stackConfiguration.encTypesTable,
                                                            SOPC_KnownEncodeableTypes,
                                                            nbKnownTypes * sizeof(SOPC_EncodeableType*)))
            {
                g_stackConfiguration.encTypesTable = NULL;
            }else{
                additionalTypes = g_stackConfiguration.encTypesTable;
                g_stackConfiguration.nbEncTypesTable = nbKnownTypes;
            }
        }else{
            // +1 for null value termination
            additionalTypes = realloc(g_stackConfiguration.encTypesTable,
                                      sizeof(SOPC_EncodeableType*) * g_stackConfiguration.nbEncTypesTable + nbTypes + 1);
        }

        if(additionalTypes != NULL){
            g_stackConfiguration.encTypesTable = additionalTypes;

            for(idx = 0; idx < nbTypes; idx++){
                g_stackConfiguration.encTypesTable[g_stackConfiguration.nbEncTypesTable + idx] = encTypesTable[idx];
            }
            g_stackConfiguration.nbEncTypesTable += nbTypes;
            // NULL terminated table

        }else{
            status = STATUS_NOK;
        }
    }
    return status;
}

SOPC_EncodeableType** SOPC_StackConfiguration_GetEncodeableTypes()
{
    if (g_stackConfiguration.encTypesTable != NULL && g_stackConfiguration.nbEncTypesTable > 0){
        // Additional types are present: contains known types + additional
        return g_stackConfiguration.encTypesTable;
    }else{
        // No additional types: return static known types
        return SOPC_KnownEncodeableTypes;
    }
}

SOPC_NamespaceTable* SOPC_StackConfiguration_GetNamespaces()
{
    return g_stackConfiguration.nsTable;
}
