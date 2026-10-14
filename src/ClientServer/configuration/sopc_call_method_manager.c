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
#include "sopc_atomic.h"
#include "sopc_dict.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_mutexes.h"

/* Tombstone key required for SOPC_Dict_Remove (NodeId keys are pointers, never equal to UINTPTR_MAX) */
static const uintptr_t METHOD_DICT_TOMBSTONE = UINTPTR_MAX;

/** Type of the function to get a C function associated to a ::SOPC_NodeId of a Method */
typedef SOPC_MethodCallFunc* SOPC_MethodCallManager_Get_Func(SOPC_MethodCallManager* mcm, const SOPC_NodeId* methodId);

/** Type of the function to free ::SOPC_MethodCallManager internal data */
typedef void SOPC_MethodCallManager_Free_Func(void* data);

/**
 * \brief The ::SOPC_MethodCallManager structure contains the data necessary for a MCM instance.
 */
struct SOPC_MethodCallManager
{
    /**
     * \brief the mutex used to make the ::SOPC_MethodCallManager instance thread-safe
     */
    SOPC_Mutex mut;

    /**
     * \brief dictionary containing NodeId to ::SOPC_MethodCallEntry
     */
    SOPC_Dict* nodeIdToMethod;
};

/**
 * \brief Method parameter shared by the registrations using it and the ongoing calls.
 *        The parameter is freed (by fnFree if any) when the last reference is released:
 *        no registration nor ongoing call uses it anymore.
 */
typedef struct SOPC_MethodCallParam
{
    void* param;
    SOPC_MethodCallFunc_Free_Func* fnFree;
    int32_t refCount; // atomic access only
} SOPC_MethodCallParam;

/**
 * \brief Registration of a method in the dictionary
 */
typedef struct SOPC_MethodCallEntry
{
    // First member: returned by the deprecated SOPC_MethodCallManager_GetMethod (fnFree shall not be called by user)
    SOPC_MethodCallFunc method;
    // Reference held on the parameter by this registration
    SOPC_MethodCallParam* sharedParam;
} SOPC_MethodCallEntry;

// Creates a shared parameter with one reference held by the caller
static SOPC_MethodCallParam* SOPC_MethodCallParam_Create(void* param, SOPC_MethodCallFunc_Free_Func* fnFree)
{
    SOPC_MethodCallParam* sharedParam = SOPC_Calloc(1, sizeof(*sharedParam));
    if (NULL != sharedParam)
    {
        sharedParam->param = param;
        sharedParam->fnFree = fnFree;
        SOPC_Atomic_Int_Set(&sharedParam->refCount, 1);
    }
    return sharedParam;
}

static void SOPC_MethodCallParam_AddRef(SOPC_MethodCallParam* sharedParam)
{
    int32_t prevCount = SOPC_Atomic_Int_Add(&sharedParam->refCount, 1);
    SOPC_ASSERT(prevCount > 0);
}

// Releases a reference: the parameter is freed on last release
static void SOPC_MethodCallParam_Release(SOPC_MethodCallParam** ppSharedParam)
{
    SOPC_MethodCallParam* sharedParam = *ppSharedParam;
    int32_t prevCount = SOPC_Atomic_Int_Add(&sharedParam->refCount, -1);
    SOPC_ASSERT(prevCount > 0);
    if (1 == prevCount)
    {
        if (NULL != sharedParam->fnFree && NULL != sharedParam->param)
        {
            sharedParam->fnFree(sharedParam->param);
        }
        SOPC_Free(sharedParam);
    }
    *ppSharedParam = NULL;
}

// Used when clear nodeIdToMethod entry: releases the registration reference on the parameter
static void SOPC_MethodCallManager_Free_MF(uintptr_t data)
{
    if (NULL == (void*) data)
    {
        return;
    }

    SOPC_MethodCallEntry* entry = (SOPC_MethodCallEntry*) data;
    SOPC_MethodCallParam_Release(&entry->sharedParam);
    entry->method.pMethodFunc = NULL;
    entry->method.pParam = NULL;
    SOPC_Free(entry);
}

SOPC_MethodCallManager* SOPC_MethodCallManager_Create(void)
{
    SOPC_MethodCallManager* mcm = SOPC_Calloc(1, sizeof(SOPC_MethodCallManager));
    if (NULL == mcm)
    {
        return NULL;
    }

    mcm->nodeIdToMethod = SOPC_NodeId_Dict_Create(true, &SOPC_MethodCallManager_Free_MF);
    if (NULL == mcm->nodeIdToMethod)
    {
        SOPC_Free(mcm);
        mcm = NULL;
    }
    else
    {
        SOPC_Dict_SetTombstoneKey(mcm->nodeIdToMethod, METHOD_DICT_TOMBSTONE);
        SOPC_ReturnStatus mutStatus = SOPC_Mutex_Initialization(&mcm->mut);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    }

    return mcm;
}

void SOPC_MethodCallManager_Free(SOPC_MethodCallManager* mcm)
{
    if (NULL == mcm)
    {
        return;
    }
    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&mcm->mut);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    SOPC_Dict_Delete(mcm->nodeIdToMethod);
    mcm->nodeIdToMethod = NULL;
    mutStatus = SOPC_Mutex_Unlock(&mcm->mut);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    mutStatus = SOPC_Mutex_Clear(&mcm->mut);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    SOPC_Free(mcm);
}

// Registers methodFunc for methodId with a new reference on sharedParam (replacing a previous registration if any)
static SOPC_ReturnStatus SOPC_MethodCallManager_AddEntry(SOPC_MethodCallManager* mcm,
                                                         const SOPC_NodeId* methodId,
                                                         SOPC_MethodCallFunc_Ptr* methodFunc,
                                                         SOPC_MethodCallParam* sharedParam)
{
    SOPC_MethodCallEntry* entry = SOPC_Calloc(1, sizeof(*entry));
    SOPC_NodeId* methodIdCopy = SOPC_Calloc(1, sizeof(*methodIdCopy));
    SOPC_ReturnStatus status = (NULL == entry || NULL == methodIdCopy) ? SOPC_STATUS_OUT_OF_MEMORY : SOPC_STATUS_OK;
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_NodeId_Copy(methodIdCopy, methodId);
    }
    if (SOPC_STATUS_OK == status)
    {
        entry->method.pFnFree = sharedParam->fnFree;
        entry->method.pMethodFunc = methodFunc;
        entry->method.pParam = sharedParam->param;
        SOPC_MethodCallParam_AddRef(sharedParam);
        entry->sharedParam = sharedParam;

        SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&mcm->mut);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
        SOPC_ASSERT(NULL != mcm->nodeIdToMethod);
        bool bres = SOPC_Dict_Insert(mcm->nodeIdToMethod, (uintptr_t) methodIdCopy, (uintptr_t) entry);
        mutStatus = SOPC_Mutex_Unlock(&mcm->mut);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
        if (!bres)
        {
            // Not the last reference: the caller holds its own
            SOPC_MethodCallParam_Release(&entry->sharedParam);
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }
    if (SOPC_STATUS_OK != status)
    {
        SOPC_NodeId_Clear(methodIdCopy);
        SOPC_Free(methodIdCopy);
        SOPC_Free(entry);
    }
    return status;
}

// Releases the caller reference on sharedParam, on failure the parameter remains owned by the caller (not freed)
static void SOPC_MethodCallManager_ReleaseCreatedParam(SOPC_MethodCallParam** ppSharedParam, SOPC_ReturnStatus status)
{
    if (SOPC_STATUS_OK != status)
    {
        (*ppSharedParam)->fnFree = NULL;
    }
    SOPC_MethodCallParam_Release(ppSharedParam);
}

SOPC_ReturnStatus SOPC_MethodCallManager_AddMethod(SOPC_MethodCallManager* mcm,
                                                   const SOPC_NodeId* methodId,
                                                   SOPC_MethodCallFunc_Ptr* methodFunc,
                                                   void* param,
                                                   SOPC_MethodCallFunc_Free_Func* fnFree)
{
    if (NULL == mcm || NULL == methodId || NULL == methodFunc)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_MethodCallParam* sharedParam = SOPC_MethodCallParam_Create(param, fnFree);
    if (NULL == sharedParam)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }
    SOPC_ReturnStatus status = SOPC_MethodCallManager_AddEntry(mcm, methodId, methodFunc, sharedParam);
    SOPC_MethodCallManager_ReleaseCreatedParam(&sharedParam, status);
    return status;
}

SOPC_ReturnStatus SOPC_MethodCallManager_AddMethodWithType(SOPC_MethodCallManager* mcm,
                                                           const SOPC_NodeId* methodInstanceId,
                                                           const SOPC_NodeId* methodTypeId,
                                                           SOPC_MethodCallFunc_Ptr* methodFunc,
                                                           void* param,
                                                           SOPC_MethodCallFunc_Free_Func* fnFree)
{
    if (NULL == mcm || NULL == methodInstanceId || NULL == methodTypeId || NULL == methodFunc)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    // Both registrations share the parameter: it is freed once, when no registration nor call uses it anymore
    SOPC_MethodCallParam* sharedParam = SOPC_MethodCallParam_Create(param, fnFree);
    if (NULL == sharedParam)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }
    SOPC_ReturnStatus status = SOPC_MethodCallManager_AddEntry(mcm, methodTypeId, methodFunc, sharedParam);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_MethodCallManager_AddEntry(mcm, methodInstanceId, methodFunc, sharedParam);
        if (SOPC_STATUS_OK != status)
        {
            // Parameter not freed on rollback: the caller holds a reference
            SOPC_ReturnStatus removeStatus = SOPC_MethodCallManager_RemoveMethod(mcm, methodTypeId);
            SOPC_UNUSED_RESULT(removeStatus);
        }
    }
    SOPC_MethodCallManager_ReleaseCreatedParam(&sharedParam, status);
    return status;
}

SOPC_ReturnStatus SOPC_MethodCallManager_RemoveMethod(SOPC_MethodCallManager* mcm, const SOPC_NodeId* methodId)
{
    if (NULL == mcm || NULL == methodId)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&mcm->mut);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    SOPC_ASSERT(NULL != mcm->nodeIdToMethod);
    bool found = false;
    uintptr_t method = SOPC_Dict_Get(mcm->nodeIdToMethod, (const uintptr_t) methodId, &found);
    SOPC_UNUSED_RESULT(method);
    if (found)
    {
        // Frees the copied NodeId key and the entry (releasing its reference on the parameter)
        SOPC_Dict_Remove(mcm->nodeIdToMethod, (const uintptr_t) methodId);
    }
    mutStatus = SOPC_Mutex_Unlock(&mcm->mut);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    return found ? SOPC_STATUS_OK : SOPC_STATUS_INVALID_STATE;
}

bool SOPC_MethodCallManager_CallMethod(SOPC_MethodCallManager* mcm,
                                       const SOPC_NodeId* methodId,
                                       const SOPC_CallContext* callContextPtr,
                                       const SOPC_NodeId* objectId,
                                       uint32_t nbInputArgs,
                                       const SOPC_Variant* inputArgs,
                                       uint32_t* nbOutputArgs,
                                       SOPC_Variant** outputArgs,
                                       SOPC_StatusCode* outCallStatus)
{
    if (NULL == mcm || NULL == methodId || NULL == outCallStatus)
    {
        return false;
    }
    SOPC_MethodCallFunc_Ptr* methodFunc = NULL;
    SOPC_MethodCallParam* sharedParam = NULL;

    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&mcm->mut);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    SOPC_ASSERT(NULL != mcm->nodeIdToMethod);
    const SOPC_MethodCallEntry* entry =
        (const SOPC_MethodCallEntry*) SOPC_Dict_Get(mcm->nodeIdToMethod, (const uintptr_t) methodId, NULL);
    if (NULL != entry)
    {
        // Reference held during the call: parameter kept even if the method is removed or replaced meanwhile
        methodFunc = entry->method.pMethodFunc;
        sharedParam = entry->sharedParam;
        SOPC_MethodCallParam_AddRef(sharedParam);
    }
    mutStatus = SOPC_Mutex_Unlock(&mcm->mut);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    if (NULL == entry)
    {
        return false;
    }
    // Method called without the manager locked (it may add or remove methods)
    *outCallStatus =
        methodFunc(callContextPtr, objectId, nbInputArgs, inputArgs, nbOutputArgs, outputArgs, sharedParam->param);
    SOPC_MethodCallParam_Release(&sharedParam);
    return true;
}

SOPC_MethodCallFunc* SOPC_MethodCallManager_GetMethod(SOPC_MethodCallManager* mcm, const SOPC_NodeId* methodId)
{
    if (NULL == mcm || NULL == methodId)
    {
        return NULL;
    }
    SOPC_ASSERT(NULL != mcm->nodeIdToMethod);

    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&mcm->mut);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    SOPC_MethodCallFunc* methodFunc =
        (SOPC_MethodCallFunc*) SOPC_Dict_Get(mcm->nodeIdToMethod, (const uintptr_t) methodId, NULL);
    mutStatus = SOPC_Mutex_Unlock(&mcm->mut);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    return methodFunc;
}
