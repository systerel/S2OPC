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
#include <assert.h>
#include "sopc_dict.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"

typedef struct SOPC_MethodCallManager_FuncWrapper
{
    SOPC_Method_Func methodFunc;
} SOPC_MethodCallManager_FuncWrapper;

static void SOPC_MethodCallManager_Free(SOPC_MethodCallManager* mcm)
{
    if (NULL == mcm)
    {
        return;
    }
    SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
    *(SOPC_MethodCallManager_Free_Func*) (&mcm->pFnFree) = NULL;
    *(SOPC_MethodCallManager_Get_Func*) (&mcm->pFnGetMethod) = NULL;
    SOPC_GCC_DIAGNOSTIC_RESTORE
    if (NULL != mcm->pUserData)
    {
        SOPC_Dict_Delete(mcm->pUserData);
        mcm->pUserData = NULL;
    }
}

static SOPC_Method_Func SOPC_MethodCallManager_Get(SOPC_MethodCallManager* mcm, SOPC_NodeId* methodId)
{
    if (NULL == mcm || NULL == methodId)
    {
        return NULL;
    }
    SOPC_Dict* dict = (SOPC_Dict*) mcm->pUserData;
    assert(NULL != dict);
    SOPC_Method_Func methodFunc = NULL;
    SOPC_MethodCallManager_FuncWrapper* wrapper =
        (SOPC_MethodCallManager_FuncWrapper*) SOPC_Dict_Get(dict, methodId, NULL);
    if (NULL != wrapper)
    {
        methodFunc = wrapper->methodFunc;
    }
    return methodFunc;
}

SOPC_MethodCallManager* SOPC_MethodCallManager_Create()
{
    SOPC_MethodCallManager* mcm = SOPC_Calloc(1, sizeof(SOPC_MethodCallManager));
    if (NULL == mcm)
    {
        return NULL;
    }

    mcm->pUserData = SOPC_NodeId_Dict_Create(false, &SOPC_Free);
    if (NULL == mcm->pUserData)
    {
        SOPC_Free(mcm);
    }
    else
    {
        SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
        *(SOPC_MethodCallManager_Free_Func*) (&mcm->pFnFree) = &SOPC_MethodCallManager_Free;
        *(SOPC_MethodCallManager_Get_Func*) (&mcm->pFnGetMethod) = &SOPC_MethodCallManager_Get;
        SOPC_GCC_DIAGNOSTIC_RESTORE
    }

    return mcm;
}

/**
 * \brief Associate a C function to a NodeId of a Method.
 * This function should be used only with the basic implementation of SOPC_MethodCallManager provided by the toolkit.
 *
 * \param mcm   a valid pointer on a SOPC_MethodCallManager returned by SOPC_MethodCallManager_Create().
 * \param methodId        a valid pointer on a SOPC_NodeId of the method.
 * \param methodFunc      a valid pointer on a C function to associate with the given methodId.
 *
 * \return SOPC_STATUS_OK when the function succeed, SOPC_STATUS_INVALID_PARAMETERS or SOPC_STATUS_OUT_OF_MEMORY.
 */
SOPC_ReturnStatus SOPC_MethodCallManager_AddMethod(SOPC_MethodCallManager* mcm,
                                                   SOPC_NodeId* methodId,
                                                   SOPC_Method_Func methodFunc)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_Dict* dict = (SOPC_Dict*) mcm->pUserData;
    assert(NULL != dict);

    SOPC_MethodCallManager_FuncWrapper* wrapper = SOPC_Calloc(1, sizeof(SOPC_MethodCallManager_FuncWrapper));
    if (NULL == wrapper)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }
    else
    {
        wrapper->methodFunc = methodFunc;
        bool b = SOPC_Dict_Insert(dict, methodId, wrapper);
        if (false == b)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }
    return status;
}
