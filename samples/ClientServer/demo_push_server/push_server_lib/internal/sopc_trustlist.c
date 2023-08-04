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

/** \file
 *
 * \brief Interface implementation to manage the TrustListType according the
 *        Push model.
 */

#include <string.h>

#include "sopc_assert.h"
#include "sopc_helper_string.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_service_call_context.h"

#include "sopc_trustlist.h"
#include "sopc_trustlist_itf.h"
#include "sopc_trustlist_meth.h"

/*---------------------------------------------------------------------------
 *                             Constants
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------
 *                             Internal types
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------
 *                             Global variables
 *---------------------------------------------------------------------------*/

static SOPC_Dict* gObjIdToTrustList = NULL;
static int32_t gTombstoneKey = -1;

/* TODO : Uaexpert use the method nodId of the TrustListType but the method is call from the trustList objectId of the
   server object.
*/
SOPC_TrustList_Config gTrustList_DefaultAddSpace_App = {
    .groupType = SOPC_TRUSTLIST_GROUP_APP,
    .pPKI = NULL,
    .trustListNodeId = "ns=0;i=12642",
    .metOpenNodeId = "ns=0;i=11580",
    .metOpenWithMasksNodeId = "ns=0;i=12543",
    .metCloseNodeId = "ns=0;i=11583",
    .metCloseAndUpdateNodeId = "ns=0;i=12546",
    .metAddCertificateNodeId = "ns=0;i=12548",
    .metRemoveCertificateNodeId = "ns=0;i=12550",
    .metReadNodeId = "ns=0;i=11585",
    .metWriteNodeId = "ns=0;i=11588",
    .metGetPosNodeId = "ns=0;i=11590",
    .metSetPosNodeId = "ns=0;i=11593",
    .varSizeNodeId = "ns=0;i=11576",
    .varWritableNodeId = "ns=0;i=12686",
    .varUserWritableNodeId = "ns=0;i=12687",
    .varOpenCountNodeId = "ns=0;i=11579",
};

/* TODO : Uaexpert use the method nodId of the TrustListType but the method is call from the trustList objectId of the
   server object.
*/
SOPC_TrustList_Config gTrustList_DefaultAddSpace_Usr = {
    .groupType = SOPC_TRUSTLIST_GROUP_USR,
    .pPKI = NULL,
    .trustListNodeId = "ns=0;i=14123",
    .metOpenNodeId = "ns=0;i=11580",
    .metOpenWithMasksNodeId = "ns=0;i=12543",
    .metCloseNodeId = "ns=0;i=11583",
    .metCloseAndUpdateNodeId = "ns=0;i=12546",
    .metAddCertificateNodeId = "ns=0;i=12548",
    .metRemoveCertificateNodeId = "ns=0;i=12550",
    .metReadNodeId = "ns=0;i=11585",
    .metWriteNodeId = "ns=0;i=11588",
    .metGetPosNodeId = "ns=0;i=11590",
    .metSetPosNodeId = "ns=0;i=11593",
    .varSizeNodeId = "ns=0;i=11576",
    .varWritableNodeId = "ns=0;i=12686",
    .varUserWritableNodeId = "ns=0;i=12687",
    .varOpenCountNodeId = "ns=0;i=11579",
};

/*---------------------------------------------------------------------------
 *                      Prototype of static functions
 *---------------------------------------------------------------------------*/

static SOPC_ReturnStatus trustlist_create_context(SOPC_TrustListContext** ppTrustList,
                                                  SOPC_TrustList_Type groupType,
                                                  SOPC_PKIProvider* pPKI,
                                                  size_t maxTrustListSize);
static void trustlist_initialize_context(SOPC_TrustListContext* pTrustList,
                                         SOPC_TrustList_Type groupType,
                                         SOPC_PKIProvider* pPKI,
                                         size_t maxTrustListSize);
static void trustlist_clear_context(SOPC_TrustListContext* pTrustList);
static void trustlist_delete_context(SOPC_TrustListContext** ppTrustList);
static void trustlist_dict_free_context_value(uintptr_t value);
static SOPC_ReturnStatus trustlist_add_method(SOPC_MethodCallManager* pMcm,
                                              const char* pCStringNodeId,
                                              SOPC_MethodCallFunc_Ptr* pTrustListMet,
                                              char* name);

/*---------------------------------------------------------------------------
 *                       Static functions (implementation)
 *---------------------------------------------------------------------------*/

static SOPC_ReturnStatus trustlist_create_context(SOPC_TrustListContext** ppTrustList,
                                                  SOPC_TrustList_Type groupType,
                                                  SOPC_PKIProvider* pPKI,
                                                  size_t maxTrustListSize)
{
    if (NULL == ppTrustList || NULL == pPKI)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_TrustListContext* pTrustList = NULL;
    pTrustList = SOPC_Calloc(1, sizeof(SOPC_TrustListContext));
    if (NULL == pTrustList)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }
    trustlist_initialize_context(pTrustList, groupType, pPKI, maxTrustListSize);
    *ppTrustList = pTrustList;
    return SOPC_STATUS_OK;
}

static void trustlist_initialize_context(SOPC_TrustListContext* pTrustList,
                                         SOPC_TrustList_Type groupType,
                                         SOPC_PKIProvider* pPKI,
                                         size_t maxTrustListSize)
{
    SOPC_ASSERT(NULL != pTrustList);
    SOPC_ASSERT(NULL != pPKI);

    pTrustList->pObjectId = NULL;
    pTrustList->cStrObjectId = NULL;
    pTrustList->varIds.pOpenCountId = NULL;
    pTrustList->varIds.pSizeId = NULL;
    pTrustList->varIds.pUserWritableId = NULL;
    pTrustList->varIds.pWritableId = NULL;
    pTrustList->maxTrustListSize = maxTrustListSize;
    pTrustList->handle = SOPC_TRUSTLIST_INVALID_HANDLE;
    pTrustList->groupType = groupType;
    pTrustList->openingMode = SOPC_TL_OPEN_MODE_UNKNOWN;
    pTrustList->openingMask = SOPC_TL_MASK_NONE;
    pTrustList->pPKI = pPKI;
    pTrustList->pTrustListEncoded = NULL;
    pTrustList->pTrustedCerts = NULL;
    pTrustList->pIssuerCerts = NULL;
    pTrustList->pTrustedCRLs = NULL;
    pTrustList->pIssuerCRLs = NULL;
    pTrustList->bDoNotDelete = false;
}

static void trustlist_clear_context(SOPC_TrustListContext* pTrustList)
{
    if (NULL == pTrustList)
    {
        return;
    }
    SOPC_Free(pTrustList->cStrObjectId);
    SOPC_Buffer_Delete(pTrustList->pTrustListEncoded);
    SOPC_KeyManager_Certificate_Free(pTrustList->pTrustedCerts);
    SOPC_KeyManager_Certificate_Free(pTrustList->pIssuerCerts);
    SOPC_KeyManager_CRL_Free(pTrustList->pTrustedCRLs);
    SOPC_KeyManager_CRL_Free(pTrustList->pIssuerCRLs);
    SOPC_NodeId_Clear(pTrustList->pObjectId);
    SOPC_NodeId_Clear(pTrustList->varIds.pSizeId);
    SOPC_NodeId_Clear(pTrustList->varIds.pWritableId);
    SOPC_NodeId_Clear(pTrustList->varIds.pUserWritableId);
    SOPC_NodeId_Clear(pTrustList->varIds.pOpenCountId);
    SOPC_Free(pTrustList->pObjectId);
    SOPC_Free(pTrustList->varIds.pSizeId);
    SOPC_Free(pTrustList->varIds.pWritableId);
    SOPC_Free(pTrustList->varIds.pUserWritableId);
    SOPC_Free(pTrustList->varIds.pOpenCountId);
    pTrustList->bDoNotDelete = true;
}

static void trustlist_delete_context(SOPC_TrustListContext** ppTrustList)
{
    SOPC_TrustListContext* pTrustList = *ppTrustList;
    if (pTrustList->bDoNotDelete)
    {
        return;
    }
    trustlist_clear_context(pTrustList);
    SOPC_Free(pTrustList);
    *ppTrustList = NULL;
}

static void trustlist_dict_free_context_value(uintptr_t value)
{
    if (NULL != (void*) value)
    {
        SOPC_TrustListContext* pTrustList = (SOPC_TrustListContext*) value;
        trustlist_delete_context(&pTrustList);
    }
}

static SOPC_ReturnStatus trustlist_add_method(SOPC_MethodCallManager* pMcm,
                                              const char* pCStringNodeId,
                                              SOPC_MethodCallFunc_Ptr* pTrustListMet,
                                              char* name)
{
    SOPC_ASSERT(NULL != pMcm);
    SOPC_ASSERT(NULL != pCStringNodeId);
    SOPC_ASSERT(NULL != pTrustListMet);
    SOPC_ASSERT(NULL != name);

    SOPC_NodeId* pNodeId = NULL;
    pNodeId = SOPC_NodeId_FromCString(pCStringNodeId, (int32_t) strlen(pCStringNodeId));
    SOPC_ReturnStatus status = NULL == pNodeId ? SOPC_STATUS_OUT_OF_MEMORY : SOPC_STATUS_OK;
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_MethodCallManager_AddMethod(pMcm, pNodeId, pTrustListMet, name, NULL);
    }
    if (SOPC_STATUS_OK != status)
    {
        SOPC_NodeId_Clear(pNodeId);
        SOPC_Free(pNodeId);
    }
    return status;
}

/*---------------------------------------------------------------------------
 *                             ITF Functions (implementation)
 *---------------------------------------------------------------------------*/

SOPC_ReturnStatus SOPC_TrustList_Initialize(void)
{
    if (NULL != gObjIdToTrustList)
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    /* The key is include in the value (TrustList context) */
    gObjIdToTrustList = SOPC_NodeId_Dict_Create(false, trustlist_dict_free_context_value);
    if (NULL == gObjIdToTrustList)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }
    /* Mandatory for the use of SOPC_Dict_Remove */
    SOPC_Dict_SetTombstoneKey(gObjIdToTrustList, (uintptr_t) &gTombstoneKey);
    return SOPC_STATUS_OK;
}

const SOPC_TrustList_Config* SOPC_TrustList_GetDefaultConfiguration(const SOPC_TrustList_Type groupType,
                                                                    SOPC_PKIProvider* pPKI)
{
    SOPC_TrustList_Config* pCfg = NULL;
    switch (groupType)
    {
    case SOPC_TRUSTLIST_GROUP_APP:
        pCfg = &gTrustList_DefaultAddSpace_App;
        break;
    case SOPC_TRUSTLIST_GROUP_USR:
        pCfg = &gTrustList_DefaultAddSpace_Usr;
        break;
    default:
        break;
    }
    pCfg->pPKI = pPKI;
    pCfg->maxTrustListSize = SOPC_TRUSTLIST_DEFAULT_MAX_SIZE;
    return (const SOPC_TrustList_Config*) pCfg;
}

SOPC_ReturnStatus SOPC_TrustList_Configure(const SOPC_TrustList_Config* pCfg, SOPC_MethodCallManager* pMcm)
{
    /* The API is not initialized. */
    if (NULL == gObjIdToTrustList)
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    if (NULL == pCfg || NULL == pMcm)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (NULL == pCfg->trustListNodeId || NULL == pCfg->metOpenNodeId || NULL == pCfg->metOpenWithMasksNodeId ||
        NULL == pCfg->metCloseAndUpdateNodeId || NULL == pCfg->metAddCertificateNodeId ||
        NULL == pCfg->metRemoveCertificateNodeId || NULL == pCfg->metCloseNodeId || NULL == pCfg->metReadNodeId ||
        NULL == pCfg->metWriteNodeId || NULL == pCfg->metGetPosNodeId || NULL == pCfg->metSetPosNodeId ||
        NULL == pCfg->varSizeNodeId || NULL == pCfg->varOpenCountNodeId || NULL == pCfg->varUserWritableNodeId ||
        NULL == pCfg->varWritableNodeId || NULL == pCfg->pPKI)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_TrustListContext* pTrustList = NULL;
    SOPC_ReturnStatus status =
        trustlist_create_context(&pTrustList, pCfg->groupType, pCfg->pPKI, pCfg->maxTrustListSize);
    if (SOPC_STATUS_OK != status)
    {
        return status;
    }
    pTrustList->cStrObjectId = SOPC_strdup(pCfg->trustListNodeId);
    if (NULL == pTrustList->cStrObjectId)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }
    if (SOPC_STATUS_OK == status)
    {
        pTrustList->pObjectId = SOPC_NodeId_FromCString(pCfg->trustListNodeId, (int32_t) strlen(pCfg->trustListNodeId));
        if (NULL == pTrustList->pObjectId)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }
    /* Add methods ... */
    if (SOPC_STATUS_OK == status)
    {
        status =
            trustlist_add_method(pMcm, pCfg->metOpenWithMasksNodeId, &TrustList_Method_OpenWithMasks, "OpenWithMasks");
    }
    if (SOPC_STATUS_OK == status)
    {
        status = trustlist_add_method(pMcm, pCfg->metOpenNodeId, &TrustList_Method_Open, "Open");
    }
    if (SOPC_STATUS_OK == status)
    {
        status = trustlist_add_method(pMcm, pCfg->metCloseNodeId, &TrustList_Method_Close, "Close");
    }
    if (SOPC_STATUS_OK == status)
    {
        status = trustlist_add_method(pMcm, pCfg->metCloseAndUpdateNodeId, &TrustList_Method_CloseAndUpdate,
                                      "CloseAndUpdate");
    }
    if (SOPC_STATUS_OK == status)
    {
        status = trustlist_add_method(pMcm, pCfg->metAddCertificateNodeId, &TrustList_Method_AddCertificate,
                                      "AddCertificate");
    }
    if (SOPC_STATUS_OK == status)
    {
        status = trustlist_add_method(pMcm, pCfg->metRemoveCertificateNodeId, &TrustList_Method_RemoveCertificate,
                                      "RemoveCertificate");
    }
    if (SOPC_STATUS_OK == status)
    {
        status = trustlist_add_method(pMcm, pCfg->metReadNodeId, &TrustList_Method_Read, "Read");
    }
    if (SOPC_STATUS_OK == status)
    {
        status = trustlist_add_method(pMcm, pCfg->metWriteNodeId, &TrustList_Method_Write, "Write");
    }
    if (SOPC_STATUS_OK == status)
    {
        status = trustlist_add_method(pMcm, pCfg->metGetPosNodeId, &TrustList_Method_GetPosition, "GetPosition");
    }
    if (SOPC_STATUS_OK == status)
    {
        status = trustlist_add_method(pMcm, pCfg->metSetPosNodeId, &TrustList_Method_SetPosition, "SetPosition");
    }
    /* Add variables ... */
    if (SOPC_STATUS_OK == status)
    {
        pTrustList->varIds.pSizeId =
            SOPC_NodeId_FromCString(pCfg->varSizeNodeId, (int32_t) strlen(pCfg->varSizeNodeId));
        status = NULL == pTrustList->varIds.pSizeId ? SOPC_STATUS_OUT_OF_MEMORY : SOPC_STATUS_OK;
    }
    if (SOPC_STATUS_OK == status)
    {
        pTrustList->varIds.pWritableId =
            SOPC_NodeId_FromCString(pCfg->varWritableNodeId, (int32_t) strlen(pCfg->varWritableNodeId));
        status = NULL == pTrustList->varIds.pWritableId ? SOPC_STATUS_OUT_OF_MEMORY : SOPC_STATUS_OK;
    }
    if (SOPC_STATUS_OK == status)
    {
        pTrustList->varIds.pUserWritableId =
            SOPC_NodeId_FromCString(pCfg->varUserWritableNodeId, (int32_t) strlen(pCfg->varUserWritableNodeId));
        status = NULL == pTrustList->varIds.pUserWritableId ? SOPC_STATUS_OUT_OF_MEMORY : SOPC_STATUS_OK;
    }
    if (SOPC_STATUS_OK == status)
    {
        pTrustList->varIds.pOpenCountId =
            SOPC_NodeId_FromCString(pCfg->varOpenCountNodeId, (int32_t) strlen(pCfg->varOpenCountNodeId));
        status = NULL == pTrustList->varIds.pOpenCountId ? SOPC_STATUS_OUT_OF_MEMORY : SOPC_STATUS_OK;
    }

    if (SOPC_STATUS_OK == status)
    {
        bool res = TrustList_DictInsert(pTrustList->pObjectId, pTrustList);
        status = !res ? SOPC_STATUS_NOK : SOPC_STATUS_OK;
    }

    if (SOPC_STATUS_OK != status)
    {
        if (NULL != pTrustList->pObjectId)
        {
            TrustList_DictRemove(pTrustList->pObjectId);
        }
        trustlist_delete_context(&pTrustList);
    }
    return status;
}

void SOPC_TrustList_Clear(void)
{
    SOPC_Dict_Delete(gObjIdToTrustList);
    gObjIdToTrustList = NULL;
}

/*---------------------------------------------------------------------------
 *                       Internal Functions (implementation)
 *---------------------------------------------------------------------------*/

/* Insert a new objectId key and TrustList context value */
bool TrustList_DictInsert(SOPC_NodeId* pObjectId, SOPC_TrustListContext* pContext)
{
    if (NULL == gObjIdToTrustList || NULL == pObjectId || NULL == pContext)
    {
        return false;
    }
    bool res = SOPC_Dict_Insert(gObjIdToTrustList, (uintptr_t) pObjectId, (uintptr_t) pContext);
    if (!res)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s: unable to insert TrustList context",
                               pObjectId);
    }
    return res;
}

/* Get the trustList context from the nodeId */
SOPC_TrustListContext* TrustList_DictGet(const SOPC_NodeId* pObjectId, bool* found)
{
    if (NULL == gObjIdToTrustList || NULL == pObjectId)
    {
        *found = false;
        return NULL;
    }
    SOPC_TrustListContext* pCtx = NULL;
    pCtx = (SOPC_TrustListContext*) SOPC_Dict_Get(gObjIdToTrustList, (const uintptr_t) pObjectId, found);
    if (!found || NULL == pCtx)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s: unable to retrieve TrustList context",
                               pObjectId);
    }
    return pCtx;
}

/* Removes a TrustList context from the nodeId */
void TrustList_DictRemove(const SOPC_NodeId* pObjectId)
{
    if (NULL == gObjIdToTrustList || NULL == pObjectId)
    {
        return;
    }
    SOPC_Dict_Remove(gObjIdToTrustList, (const uintptr_t) pObjectId);
}
