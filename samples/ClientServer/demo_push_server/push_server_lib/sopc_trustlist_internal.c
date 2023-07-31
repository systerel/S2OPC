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
 * \brief Internal API implementation to manage methods, properties and variables of the TrustListType according the
 * Push model.
 */

#include <string.h>

#include "sopc_assert.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_service_call_context.h"

#include "sopc_push_itf_glue.h"
#include "sopc_trustlist_internal.h"
#include "sopc_trustlist_itf.h"
#include "sopc_trustlist_meth_internal.h"

/*---------------------------------------------------------------------------
 *                             Constants
 *---------------------------------------------------------------------------*/

/*
    Buffer size of th Ua Binary encoded stream containing an instance of TrustListDataType :

    buffer size =  SOPC_EMPTY_TRUSTLIST_ENCODED_BYTE_SIZE + (SOPC_LENGTH_BSTRING_ENCODED_BYTE_SIZE *
   SOPC_TRUSTLIST_NB_CERTS_MAX)
                   + SOPC_TRUSTLIST_NB_CERTS_MAX * SOPC_TRUSTLIST_CERT_SIZE_MAX
*/
#define SOPC_EMPTY_TRUSTLIST_ENCODED_BYTE_SIZE 20u
#define SOPC_LENGTH_BSTRING_ENCODED_BYTE_SIZE 4u
/*---------------------------------------------------------------------------
 *                             Internal types
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
 *                             Global variables
 *---------------------------------------------------------------------------*/

static SOPC_Dict* gObjIdToTrustList = NULL;

// const SOPC_TrustList_Config gTrustList_DefaultAddSpace_App = {
//     .trustListNodeId = "ns=0;i=12642",
//     .metOpenNodeId = "ns=0;i=12647",
//     .metOpenWithMasksNodeId = "ns=0;i=12663",
//     .metCloseNodeId = "ns=0;i=12650",
//     .metCloseAndUpdateNodeId = "ns=0;i=12666",
//     .metAddCertificateNodeId = "ns=0;i=12668",
//     .metRemoveCertificateNodeId = "ns=0;i=12670",
//     .metReadNodeId = "ns=0;i=12652",
//     .metWriteNodeId = "ns=0;i=12655",
//     .metGetPosNodeId = "ns=0;i=12657",
//     .metSetPosNodeId = "ns=0;i=12660",
//     .varSizeNodeId = "ns=0;i=12643",
//     .varWritableNodeId = "ns=0;i=14157",
//     .varUserWritableNodeId = "ns=0;i=14158",
//     .varOpenCountNodeId = "ns=0;i=12646",
// };

/* TODO : Uaexpert use the method nodId of the TrustListType but the method is call from the trustList objectId of the
   server object We shall try an other GDS push client.
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
   server object. We shall try another GDS push client.
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

static SOPC_ReturnStatus trustlist_create(SOPC_TrustList** ppTrustList,
                                          SOPC_TrustList_Type groupType,
                                          SOPC_PKIProvider* pPKI);
static void trustlist_initialize(SOPC_TrustList* pTrustList, SOPC_TrustList_Type groupType, SOPC_PKIProvider* pPKI);
static void trustlist_clear(SOPC_TrustList* pTrustList);
static void trustlist_delete(SOPC_TrustList** ppTrustList);
static void trustlist_free(uintptr_t value);
static SOPC_ReturnStatus trustlist_add_method(SOPC_MethodCallManager* pMcm,
                                              const char* pCStringNodeId,
                                              SOPC_MethodCallFunc_Ptr* pTrustListMet,
                                              char* name);

/*---------------------------------------------------------------------------
 *                       Static functions (implementation)
 *---------------------------------------------------------------------------*/

static SOPC_ReturnStatus trustlist_create(SOPC_TrustList** ppTrustList,
                                          SOPC_TrustList_Type groupType,
                                          SOPC_PKIProvider* pPKI)
{
    if (NULL == ppTrustList || NULL == pPKI)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_TrustList* pTrustList = NULL;
    pTrustList = SOPC_Calloc(1, sizeof(SOPC_TrustList));
    if (NULL == pTrustList)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }
    trustlist_initialize(pTrustList, groupType, pPKI);
    *ppTrustList = pTrustList;
    return SOPC_STATUS_OK;
}

static void trustlist_initialize(SOPC_TrustList* pTrustList, SOPC_TrustList_Type groupType, SOPC_PKIProvider* pPKI)
{
    SOPC_ASSERT(NULL != pTrustList);
    SOPC_ASSERT(NULL != pPKI);

    pTrustList->handle = INVALID_HANDLE_VALUE;
    pTrustList->bIsOpen = false;
    pTrustList->groupType = groupType;
    pTrustList->openingMode = SOPC_TL_OPEN_MODE_UNKNOWN;
    pTrustList->openingMask = SOPC_TL_MASK_NONE;
    pTrustList->openCount = 0u;
    pTrustList->size = 0u;
    pTrustList->pPKI = pPKI;
    pTrustList->pTrustListEncoded = NULL;
    pTrustList->pTrustedCerts = NULL;
    pTrustList->pIssuerCerts = NULL;
    pTrustList->pTrustedCRLs = NULL;
    pTrustList->pIssuerCRLs = NULL;
    memset(&pTrustList->varIds, 0, sizeof(SOPC_TrLst_VarCfg));
}

static void trustlist_clear(SOPC_TrustList* pTrustList)
{
    if (NULL == pTrustList)
    {
        return;
    }
    SOPC_Buffer_Delete(pTrustList->pTrustListEncoded);
    SOPC_KeyManager_Certificate_Free(pTrustList->pTrustedCerts);
    SOPC_KeyManager_Certificate_Free(pTrustList->pIssuerCerts);
    SOPC_KeyManager_CRL_Free(pTrustList->pTrustedCRLs);
    SOPC_KeyManager_CRL_Free(pTrustList->pIssuerCRLs);
    SOPC_NodeId_Clear(pTrustList->varIds.pSizeId);
    SOPC_NodeId_Clear(pTrustList->varIds.pWritableId);
    SOPC_NodeId_Clear(pTrustList->varIds.pUserWritableId);
    SOPC_NodeId_Clear(pTrustList->varIds.pOpenCountId);
    SOPC_Free(pTrustList->varIds.pSizeId);
    SOPC_Free(pTrustList->varIds.pWritableId);
    SOPC_Free(pTrustList->varIds.pUserWritableId);
    SOPC_Free(pTrustList->varIds.pOpenCountId);
}

static void trustlist_delete(SOPC_TrustList** ppTrustList)
{
    trustlist_clear(*ppTrustList);
    SOPC_Free(*ppTrustList);
    *ppTrustList = NULL;
}

static void trustlist_free(uintptr_t value)
{
    if (NULL != (void*) value)
    {
        trustlist_delete((SOPC_TrustList**) &value);
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

    gObjIdToTrustList = SOPC_NodeId_Dict_Create(true, trustlist_free);
    if (NULL == gObjIdToTrustList)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }
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
        pCfg->pPKI = pPKI;
        break;
    case SOPC_TRUSTLIST_GROUP_USR:
        pCfg = &gTrustList_DefaultAddSpace_Usr;
        pCfg->pPKI = pPKI;
        break;
    default:
        break;
    }
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
    SOPC_TrustList* pTrustList = NULL;
    SOPC_ReturnStatus status = trustlist_create(&pTrustList, pCfg->groupType, pCfg->pPKI);
    if (SOPC_STATUS_OK != status)
    {
        return status;
    }
    SOPC_NodeId* pObjId = SOPC_NodeId_FromCString(pCfg->trustListNodeId, (int32_t) strlen(pCfg->trustListNodeId));
    if (NULL == pObjId)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
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
        bool res = SOPC_Dict_Insert(gObjIdToTrustList, (uintptr_t) pObjId, (uintptr_t) pTrustList);
        status = !res ? SOPC_STATUS_NOK : SOPC_STATUS_OK;
    }

    if (SOPC_STATUS_OK != status)
    {
        SOPC_TrustList_Clear();
        trustlist_delete(&pTrustList);
        SOPC_NodeId_Clear(pObjId);
        SOPC_Free(pObjId);
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

/* Get the trustList internal object from the nodeId */
SOPC_TrustList* TrustList_DictGet(const SOPC_NodeId* objectId, bool* found)
{
    return (SOPC_TrustList*) SOPC_Dict_Get(gObjIdToTrustList, (const uintptr_t) objectId, found);
}
/* Generate a random handle */
SOPC_TrLst_Handle TrustList_GenRandHandle(void)
{
    SOPC_TrLst_Handle handle = 38;
    return 38;
}
/* Read the PKI and encode the trustList in a UA Binary encoded stream containing an instance of TrustListDataType */
SOPC_ReturnStatus TrustList_Encode(const SOPC_PKIProvider* pPKI,
                                   const SOPC_TrLst_Mask specifiedLists,
                                   SOPC_Buffer* pTrustListDataType)
{
    SOPC_UNUSED_ARG(pPKI);
    SOPC_UNUSED_ARG(specifiedLists);
    SOPC_UNUSED_ARG(pTrustListDataType);
    return SOPC_STATUS_OK;
}
/* Decode the trustList UA Binary stream to a TrustListDataType */
SOPC_ReturnStatus TrustList_Decode(const SOPC_Buffer* pTrustListDataTypeEncoded, void* pTrustListDataType)
{
    SOPC_UNUSED_ARG(pTrustListDataTypeEncoded);
    SOPC_UNUSED_ARG(pTrustListDataType);
    return SOPC_STATUS_OK;
}
