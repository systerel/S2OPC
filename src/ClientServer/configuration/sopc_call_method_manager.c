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

#include "sopc_call_method_manager.h"
#include "sopc_assert.h"
#include "sopc_dict.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"

// Use when clear internal Dict
static void SOPC_MethodCallManager_Free_MF(uintptr_t data)
{
    if (NULL == (void*) data)
    {
        return;
    }

    SOPC_MethodCallFunc* mf = (SOPC_MethodCallFunc*) data;

    if (NULL != mf->pFnFree && NULL != mf->pParam)
    {
        mf->pFnFree(mf->pParam);
    }

    mf->pMethodFunc = NULL;
    mf->pParam = NULL;
    SOPC_Free(mf);
}

static void SOPC_MethodCallManager_InternalData_Free(void* data)
{
    if (NULL == data)
    {
        return;
    }
    SOPC_Dict_Delete((SOPC_Dict*) data);
}

static SOPC_MethodCallFunc* SOPC_MethodCallManager_Get(SOPC_MethodCallManager* mcm, SOPC_NodeId* methodId)
{
    if (NULL == mcm || NULL == methodId)
    {
        return NULL;
    }
    SOPC_Dict* dict = (SOPC_Dict*) mcm->pUserData;
    SOPC_ASSERT(NULL != dict);
    SOPC_MethodCallFunc* methodFunc = (SOPC_MethodCallFunc*) SOPC_Dict_Get(dict, (uintptr_t) methodId, NULL);
    return methodFunc;
}

SOPC_MethodCallManager* SOPC_MethodCallManager_Create(void)
{
    SOPC_MethodCallManager* mcm = SOPC_Calloc(1, sizeof(SOPC_MethodCallManager));
    if (NULL == mcm)
    {
        return NULL;
    }

    mcm->pUserData = SOPC_NodeId_Dict_Create(true, &SOPC_MethodCallManager_Free_MF);
    if (NULL == mcm->pUserData)
    {
        SOPC_Free(mcm);
        mcm = NULL;
    }
    else
    {
        SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
        *((SOPC_MethodCallManager_Free_Func**) (&mcm->pFnFree)) = &SOPC_MethodCallManager_InternalData_Free;
        *((SOPC_MethodCallManager_Get_Func**) (&mcm->pFnGetMethod)) = &SOPC_MethodCallManager_Get;
        SOPC_GCC_DIAGNOSTIC_RESTORE
    }

    return mcm;
}

void SOPC_MethodCallManager_Free(SOPC_MethodCallManager* mcm)
{
    if (NULL == mcm)
    {
        return;
    }
    if (NULL != mcm->pFnFree && NULL != mcm->pUserData)
    {
        mcm->pFnFree(mcm->pUserData);
    }

    SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
    *((SOPC_MethodCallManager_Free_Func**) (&mcm->pFnFree)) = NULL;
    *((SOPC_MethodCallManager_Get_Func**) (&mcm->pFnGetMethod)) = NULL;
    SOPC_GCC_DIAGNOSTIC_RESTORE
    mcm->pUserData = NULL;
    SOPC_Free(mcm);
}

SOPC_ReturnStatus SOPC_MethodCallManager_AddMethod(SOPC_MethodCallManager* mcm,
                                                   const SOPC_NodeId* methodId,
                                                   SOPC_MethodCallFunc_Ptr* methodFunc,
                                                   void* param,
                                                   SOPC_MethodCallFunc_Free_Func* fnFree)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_Dict* dict = (SOPC_Dict*) mcm->pUserData;
    SOPC_ASSERT(NULL != dict);

    SOPC_MethodCallFunc* wrapper = SOPC_Calloc(1, sizeof(SOPC_MethodCallFunc));
    SOPC_NodeId* methodIdCopy = SOPC_Calloc(1, sizeof(SOPC_NodeId));
    if (NULL == wrapper || NULL == methodIdCopy)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_NodeId_Copy(methodIdCopy, methodId);
    }
    if (SOPC_STATUS_OK == status)
    {
        wrapper->pFnFree = fnFree;
        wrapper->pMethodFunc = methodFunc;
        wrapper->pParam = param;

        bool bres = SOPC_Dict_Insert(dict, (uintptr_t) methodIdCopy, (uintptr_t) wrapper);
        status = bres ? SOPC_STATUS_OK : SOPC_STATUS_OUT_OF_MEMORY;
    }
    if (SOPC_STATUS_OK != status)
    {
        SOPC_NodeId_Clear(methodIdCopy);
        SOPC_Free(methodIdCopy);
        SOPC_Free(wrapper);
    }
    return status;
}

SOPC_ReturnStatus SOPC_MethodCallManager_AddMethodWithType(SOPC_MethodCallManager* mcm,
                                                           const SOPC_NodeId* methodInstanceId,
                                                           const SOPC_NodeId* methodTypeId,
                                                           SOPC_MethodCallFunc_Ptr* methodFunc,
                                                           void* param,
                                                           SOPC_MethodCallFunc_Free_Func* fnFree)
{
    SOPC_ReturnStatus status = SOPC_MethodCallManager_AddMethod(mcm, methodTypeId, methodFunc, param, fnFree);
    if (SOPC_STATUS_OK != status)
    {
        return status;
    }
    status = SOPC_MethodCallManager_AddMethod(mcm, methodInstanceId, methodFunc, param, fnFree);
    if (SOPC_STATUS_OK != status)
    {
        // remove already inserted method
        SOPC_Dict_Remove((SOPC_Dict*) mcm->pUserData, (uintptr_t) methodTypeId);
    }
    return status;
}
