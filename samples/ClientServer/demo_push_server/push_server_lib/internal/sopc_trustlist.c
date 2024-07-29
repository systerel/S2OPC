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
#include "sopc_key_manager.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_pki_struct_lib_internal.h"
#include "sopc_service_call_context.h"

#include "sopc_trustlist.h"
#include "sopc_trustlist_meth.h"

#include "opcua_identifiers.h"

/*---------------------------------------------------------------------------
 *                             Global variables
 *---------------------------------------------------------------------------*/

static SOPC_Dict* gObjIdToTrustList = NULL;
static bool gbTypeMetIsConfigure = false;
static bool gbGroupAppIsSet = false;
static bool gbGroupUsrIsSet = false;

static int32_t gTombstoneKey = -1;

/* NodeIds of the TrustList instance of the DefaultApplicationGroup */
static const SOPC_NodeId gAppTrustListId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultApplicationGroup_TrustList};
static const SOPC_NodeId gAppOpenId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultApplicationGroup_TrustList_Open};
static const SOPC_NodeId gAppCloseId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultApplicationGroup_TrustList_Close};
static const SOPC_NodeId gAppReadId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultApplicationGroup_TrustList_Read};
static const SOPC_NodeId gAppWriteId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultApplicationGroup_TrustList_Write};
static const SOPC_NodeId gAppGetPositionId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultApplicationGroup_TrustList_GetPosition};
static const SOPC_NodeId gAppSetPositionId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultApplicationGroup_TrustList_SetPosition};
static const SOPC_NodeId gAppOpenWithMasksId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultApplicationGroup_TrustList_OpenWithMasks};
static const SOPC_NodeId gAppCloseAndUpdateId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultApplicationGroup_TrustList_CloseAndUpdate};
static const SOPC_NodeId gAppAddCertificateId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultApplicationGroup_TrustList_AddCertificate};
static const SOPC_NodeId gAppRemoveCertificateId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultApplicationGroup_TrustList_RemoveCertificate};
static const SOPC_NodeId gAppOpenCountId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultApplicationGroup_TrustList_OpenCount};
static const SOPC_NodeId gAppSizeId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultApplicationGroup_TrustList_Size};
static const SOPC_NodeId gAppWritableId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultApplicationGroup_TrustList_Writable};
static const SOPC_NodeId gAppUserWritableId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultApplicationGroup_TrustList_UserWritable};
static const SOPC_NodeId gAppLastUpdateTimeId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultApplicationGroup_TrustList_LastUpdateTime};

/* NodeIds of the TrustList instance of the DefaultUserTokenGroup */
static const SOPC_NodeId gUsrTrustListId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultUserTokenGroup_TrustList};
static const SOPC_NodeId gUsrOpenId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultUserTokenGroup_TrustList_Open};
static const SOPC_NodeId gUsrCloseId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultUserTokenGroup_TrustList_Close};
static const SOPC_NodeId gUsrReadId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultUserTokenGroup_TrustList_Read};
static const SOPC_NodeId gUsrWriteId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultUserTokenGroup_TrustList_Write};
static const SOPC_NodeId gUsrGetPositionId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultUserTokenGroup_TrustList_GetPosition};
static const SOPC_NodeId gUsrSetPositionId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultUserTokenGroup_TrustList_SetPosition};
static const SOPC_NodeId gUsrOpenWithMasksId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultUserTokenGroup_TrustList_OpenWithMasks};
static const SOPC_NodeId gUsrCloseAndUpdateId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultUserTokenGroup_TrustList_CloseAndUpdate};
static const SOPC_NodeId gUsrAddCertificateId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultUserTokenGroup_TrustList_AddCertificate};
static const SOPC_NodeId gUsrRemoveCertificateId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultUserTokenGroup_TrustList_RemoveCertificate};
static const SOPC_NodeId gUsrOpenCountId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultUserTokenGroup_TrustList_OpenCount};
static const SOPC_NodeId gUsrSizeId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultUserTokenGroup_TrustList_Size};
static const SOPC_NodeId gUsrWritableId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultUserTokenGroup_TrustList_Writable};
static const SOPC_NodeId gUsrUserWritableId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultUserTokenGroup_TrustList_UserWritable};
static const SOPC_NodeId gUsrLastUpdateTimeId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultUserTokenGroup_TrustList_LastUpdateTime};

/* NodeIds of the TrustListType */
static const SOPC_NodeId gTypeOpenId = {.IdentifierType = SOPC_IdentifierType_Numeric,
                                        .Namespace = 0,
                                        .Data.Numeric = OpcUaId_FileType_Open};
static const SOPC_NodeId gTypeCloseId = {.IdentifierType = SOPC_IdentifierType_Numeric,
                                         .Namespace = 0,
                                         .Data.Numeric = OpcUaId_FileType_Close};
static const SOPC_NodeId gTypeReadId = {.IdentifierType = SOPC_IdentifierType_Numeric,
                                        .Namespace = 0,
                                        .Data.Numeric = OpcUaId_FileType_Read};
static const SOPC_NodeId gTypeWriteId = {.IdentifierType = SOPC_IdentifierType_Numeric,
                                         .Namespace = 0,
                                         .Data.Numeric = OpcUaId_FileType_Write};
static const SOPC_NodeId gTypeGetPositionId = {.IdentifierType = SOPC_IdentifierType_Numeric,
                                               .Namespace = 0,
                                               .Data.Numeric = OpcUaId_FileType_GetPosition};
static const SOPC_NodeId gTypeSetPositionId = {.IdentifierType = SOPC_IdentifierType_Numeric,
                                               .Namespace = 0,
                                               .Data.Numeric = OpcUaId_FileType_SetPosition};
static const SOPC_NodeId gTypeOpenWithMasksId = {.IdentifierType = SOPC_IdentifierType_Numeric,
                                                 .Namespace = 0,
                                                 .Data.Numeric = OpcUaId_TrustListType_OpenWithMasks};
static const SOPC_NodeId gTypeCloseAndUpdateId = {.IdentifierType = SOPC_IdentifierType_Numeric,
                                                  .Namespace = 0,
                                                  .Data.Numeric = OpcUaId_TrustListType_CloseAndUpdate};
static const SOPC_NodeId gTypeAddCertificateId = {.IdentifierType = SOPC_IdentifierType_Numeric,
                                                  .Namespace = 0,
                                                  .Data.Numeric = OpcUaId_TrustListType_AddCertificate};
static const SOPC_NodeId gTypeRemoveCertificateId = {.IdentifierType = SOPC_IdentifierType_Numeric,
                                                     .Namespace = 0,
                                                     .Data.Numeric = OpcUaId_TrustListType_RemoveCertificate};

static TrustList_NodeIds gAppNodeIds = {
    .pTrustListId = &gAppTrustListId,
    .pOpenId = &gAppOpenId,
    .pOpenWithMasksId = &gAppOpenWithMasksId,
    .pCloseAndUpdateId = &gAppCloseAndUpdateId,
    .pAddCertificateId = &gAppAddCertificateId,
    .pRemoveCertificateId = &gAppRemoveCertificateId,
    .pCloseId = &gAppCloseId,
    .pReadId = &gAppReadId,
    .pWriteId = &gAppWriteId,
    .pGetPosId = &gAppGetPositionId,
    .pSetPosId = &gAppSetPositionId,
    .pSizeId = &gAppSizeId,
    .pOpenCountId = &gAppOpenCountId,
    .pUserWritableId = &gAppUserWritableId,
    .pWritableId = &gAppWritableId,
    .pLastUpdateTimeId = &gAppLastUpdateTimeId,
};

static TrustList_NodeIds gUsrNodeIds = {
    .pTrustListId = &gUsrTrustListId,
    .pOpenId = &gUsrOpenId,
    .pOpenWithMasksId = &gUsrOpenWithMasksId,
    .pCloseAndUpdateId = &gUsrCloseAndUpdateId,
    .pAddCertificateId = &gUsrAddCertificateId,
    .pRemoveCertificateId = &gUsrRemoveCertificateId,
    .pCloseId = &gUsrCloseId,
    .pReadId = &gUsrReadId,
    .pWriteId = &gUsrWriteId,
    .pGetPosId = &gUsrGetPositionId,
    .pSetPosId = &gUsrSetPositionId,
    .pSizeId = &gUsrSizeId,
    .pOpenCountId = &gUsrOpenCountId,
    .pUserWritableId = &gUsrUserWritableId,
    .pWritableId = &gUsrWritableId,
    .pLastUpdateTimeId = &gUsrLastUpdateTimeId,
};

static const TrustList_NodeIds gTypeNodeIds = {
    .pTrustListId = NULL,
    .pOpenId = &gTypeOpenId,
    .pOpenWithMasksId = &gTypeOpenWithMasksId,
    .pCloseAndUpdateId = &gTypeCloseAndUpdateId,
    .pAddCertificateId = &gTypeAddCertificateId,
    .pRemoveCertificateId = &gTypeRemoveCertificateId,
    .pCloseId = &gTypeCloseId,
    .pReadId = &gTypeReadId,
    .pWriteId = &gTypeWriteId,
    .pGetPosId = &gTypeGetPositionId,
    .pSetPosId = &gTypeSetPositionId,
    .pSizeId = NULL,
    .pOpenCountId = NULL,
    .pUserWritableId = NULL,
    .pWritableId = NULL,
    .pLastUpdateTimeId = NULL,
};

/*---------------------------------------------------------------------------
 *                      Prototype of static functions
 *---------------------------------------------------------------------------*/

static SOPC_ReturnStatus trustlist_create_context(SOPC_TrustListContext** ppTrustList,
                                                  SOPC_TrustList_Type groupType,
                                                  SOPC_PKIProvider* pPKI,
                                                  uint32_t maxTrustListSize,
                                                  bool isTOFU);
static void trustlist_clear_context(SOPC_TrustListContext* pTrustList);
static void trustlist_delete_context(SOPC_TrustListContext** ppTrustList);
static void trustlist_dict_free_context_value(uintptr_t value);

static void sopc_trustlist_activity_timeout_cb(SOPC_EventHandler* pHandler,
                                               int32_t event,
                                               uint32_t eltId,
                                               uintptr_t pParams,
                                               uintptr_t pAuxParam);
static bool trustlist_register_ctx_from_node_id(const SOPC_NodeId* pObjectId, SOPC_TrustListContext* pContext);

/*---------------------------------------------------------------------------
 *                       Static functions (implementation)
 *---------------------------------------------------------------------------*/

static SOPC_ReturnStatus trustlist_create_context(SOPC_TrustListContext** ppTrustList,
                                                  SOPC_TrustList_Type groupType,
                                                  SOPC_PKIProvider* pPKI,
                                                  uint32_t maxTrustListSize,
                                                  bool isTOFU)
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
    pTrustList->maxTrustListSize = maxTrustListSize;
    pTrustList->groupType = groupType;
    pTrustList->pPKI = pPKI;
    pTrustList->isTOFU = isTOFU;

    *ppTrustList = pTrustList;
    return SOPC_STATUS_OK;
}

static void trustlist_clear_context(SOPC_TrustListContext* pTrustList)
{
    if (NULL == pTrustList)
    {
        return;
    }
    SOPC_Free(pTrustList->cStrObjectId);
    SOPC_Buffer_Delete(pTrustList->opnCtx.pTrustListEncoded);
    SOPC_KeyManager_Certificate_Free(pTrustList->opnCtx.pTrustedCerts);
    SOPC_KeyManager_Certificate_Free(pTrustList->opnCtx.pIssuerCerts);
    SOPC_KeyManager_CRL_Free(pTrustList->opnCtx.pTrustedCRLs);
    SOPC_KeyManager_CRL_Free(pTrustList->opnCtx.pIssuerCRLs);
    SOPC_Looper_Delete(pTrustList->eventMgr.pLooper);
}

static void trustlist_delete_context(SOPC_TrustListContext** ppTrustList)
{
    SOPC_TrustListContext* pTrustList = *ppTrustList;
    if (NULL == pTrustList)
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

static void sopc_trustlist_activity_timeout_cb(SOPC_EventHandler* pHandler,
                                               int32_t event,
                                               uint32_t eltId,
                                               uintptr_t pParams,
                                               uintptr_t pAuxParam)
{
    SOPC_UNUSED_ARG(pHandler);
    SOPC_UNUSED_ARG(event);
    SOPC_UNUSED_ARG(eltId);
    SOPC_UNUSED_ARG(pAuxParam);
    SOPC_TrustListContext* pCtx = (void*) pParams;
    if (NULL != pCtx)
    {
        SOPC_Atomic_Int_Set(&pCtx->eventMgr.timeoutElapsed, 1);
    }
}

/* Insert a new objectId key and TrustList context value */
bool trustlist_register_ctx_from_node_id(const SOPC_NodeId* pObjectId, SOPC_TrustListContext* pContext)
{
    if (NULL == gObjIdToTrustList || NULL == pObjectId || NULL == pContext)
    {
        return false;
    }
    bool res = SOPC_Dict_Insert(gObjIdToTrustList, (uintptr_t) pObjectId, (uintptr_t) pContext);
    if (!res)
    {
        char* cStrId = SOPC_NodeId_ToCString(pObjectId);
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList: unable to insert TrustList context to %s",
                               cStrId);
        SOPC_Free(cStrId);
    }
    return res;
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

SOPC_ReturnStatus SOPC_TrustList_GetDefaultConfiguration(const SOPC_TrustList_Type groupType,
                                                         SOPC_PKIProvider* pPKI,
                                                         const uint32_t maxTrustListSize,
                                                         SOPC_TrustList_Config** ppConfig)
{
    if (NULL == pPKI || 0 == maxTrustListSize || NULL == ppConfig)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    TrustList_NodeIds* pNodeIds = NULL;
    if (SOPC_TRUSTLIST_GROUP_APP == groupType)
    {
        pNodeIds = &gAppNodeIds;
    }
    else if (SOPC_TRUSTLIST_GROUP_USR == groupType)
    {
        pNodeIds = &gUsrNodeIds;
    }
    else
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    *ppConfig = NULL;
    SOPC_TrustList_Config* pCfg = SOPC_Calloc(1, sizeof(SOPC_TrustList_Config));
    if (NULL == pCfg)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }
    pCfg->pIds = pNodeIds;
    pCfg->groupType = groupType;
    pCfg->pPKI = pPKI;
    pCfg->maxTrustListSize = maxTrustListSize;
    pCfg->isTOFU = false;

    *ppConfig = pCfg;
    return SOPC_STATUS_OK;
    ;
}

SOPC_ReturnStatus SOPC_TrustList_GetTOFUConfiguration(const SOPC_TrustList_Type groupType,
                                                      SOPC_PKIProvider* pPKI,
                                                      const uint32_t maxTrustListSize,
                                                      SOPC_TrustList_Config** ppConfig)
{
    SOPC_ReturnStatus status = SOPC_TrustList_GetDefaultConfiguration(groupType, pPKI, maxTrustListSize, ppConfig);
    if (SOPC_STATUS_OK == status)
    {
        SOPC_TrustList_Config* pCfg = *ppConfig;
        pCfg->isTOFU = true;
    }
    return status;
}

void SOPC_TrustList_DeleteConfiguration(SOPC_TrustList_Config** ppConfig)
{
    if (NULL == ppConfig)
    {
        return;
    }
    SOPC_TrustList_Config* pConfig = *ppConfig;
    if (NULL == pConfig)
    {
        return;
    }
    memset(pConfig, 0, sizeof(SOPC_TrustList_Config));
    SOPC_Free(pConfig);
    pConfig = NULL;
    *ppConfig = pConfig;
}

SOPC_ReturnStatus SOPC_TrustList_Configure(SOPC_TrustList_Config* pCfg, SOPC_MethodCallManager* pMcm)
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
    /* Protection in case default groups are already recorded */
    if (SOPC_TRUSTLIST_GROUP_APP == pCfg->groupType && gbGroupAppIsSet)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (SOPC_TRUSTLIST_GROUP_USR == pCfg->groupType && gbGroupUsrIsSet)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (NULL == pCfg->pIds || NULL == pCfg->pPKI)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    const TrustList_NodeIds* pTypeIds = &gTypeNodeIds;
    const TrustList_NodeIds* pIds = pCfg->pIds;

    if (NULL == pIds->pTrustListId || NULL == pIds->pOpenId || NULL == pIds->pOpenWithMasksId ||
        NULL == pIds->pCloseAndUpdateId || NULL == pIds->pAddCertificateId || NULL == pIds->pRemoveCertificateId ||
        NULL == pIds->pCloseId || NULL == pIds->pReadId || NULL == pIds->pWriteId || NULL == pIds->pGetPosId ||
        NULL == pIds->pSetPosId || NULL == pIds->pSizeId || NULL == pIds->pOpenCountId ||
        NULL == pIds->pUserWritableId || NULL == pIds->pWritableId || NULL == pIds->pLastUpdateTimeId)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_TrustListContext* pTrustList = NULL;
    SOPC_ReturnStatus status =
        trustlist_create_context(&pTrustList, pCfg->groupType, pCfg->pPKI, pCfg->maxTrustListSize, pCfg->isTOFU);
    if (SOPC_STATUS_OK != status)
    {
        return status;
    }
    pTrustList->cStrObjectId = SOPC_NodeId_ToCString(pIds->pTrustListId);
    if (NULL == pTrustList->cStrObjectId)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }
    /* Configure the event for the activity timeout */
    if (SOPC_STATUS_OK == status)
    {
        pTrustList->eventMgr.pLooper = SOPC_Looper_Create(pTrustList->cStrObjectId);
        if (NULL == pTrustList->eventMgr.pLooper)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        pTrustList->eventMgr.pHandler =
            SOPC_EventHandler_Create(pTrustList->eventMgr.pLooper, sopc_trustlist_activity_timeout_cb);
        if (NULL == pTrustList->eventMgr.pHandler)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }
    /* Add methods ... */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_MethodCallManager_AddMethodWithType(pMcm, pIds->pOpenWithMasksId, pTypeIds->pOpenWithMasksId,
                                                          &TrustList_Method_OpenWithMasks, "OpenWithMasks", NULL);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_MethodCallManager_AddMethodWithType(pMcm, pIds->pOpenId, pTypeIds->pOpenId,
                                                          &TrustList_Method_Open, "Open", NULL);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_MethodCallManager_AddMethodWithType(pMcm, pIds->pCloseId, pTypeIds->pCloseId,
                                                          &TrustList_Method_Close, "Close", NULL);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_MethodCallManager_AddMethodWithType(pMcm, pIds->pCloseAndUpdateId, pTypeIds->pCloseAndUpdateId,
                                                          &TrustList_Method_CloseAndUpdate, "CloseAndUpdate", NULL);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_MethodCallManager_AddMethodWithType(pMcm, pIds->pAddCertificateId, pTypeIds->pAddCertificateId,
                                                          &TrustList_Method_AddCertificate, "AddCertificate", NULL);
    }
    if (SOPC_STATUS_OK == status)
    {
        status =
            SOPC_MethodCallManager_AddMethodWithType(pMcm, pIds->pRemoveCertificateId, pTypeIds->pRemoveCertificateId,
                                                     &TrustList_Method_RemoveCertificate, "RemoveCertificate", NULL);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_MethodCallManager_AddMethodWithType(pMcm, pIds->pReadId, pTypeIds->pReadId,
                                                          &TrustList_Method_Read, "Read", NULL);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_MethodCallManager_AddMethodWithType(pMcm, pIds->pWriteId, pTypeIds->pWriteId,
                                                          &TrustList_Method_Write, "Write", NULL);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_MethodCallManager_AddMethodWithType(pMcm, pIds->pGetPosId, pTypeIds->pGetPosId,
                                                          &TrustList_Method_GetPosition, "GetPosition", NULL);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_MethodCallManager_AddMethodWithType(pMcm, pIds->pSetPosId, pTypeIds->pSetPosId,
                                                          &TrustList_Method_SetPosition, "SetPosition", NULL);
    }
    /* Add variables ... */
    if (SOPC_STATUS_OK == status)
    {
        pTrustList->pObjectId = pIds->pTrustListId;
        pTrustList->varIds.pSizeId = pIds->pSizeId;
        pTrustList->varIds.pWritableId = pIds->pWritableId;
        pTrustList->varIds.pUserWritableId = pIds->pUserWritableId;
        pTrustList->varIds.pOpenCountId = pIds->pOpenCountId;
        pTrustList->varIds.pLastUpdateTimeId = pIds->pLastUpdateTimeId;
        bool res = trustlist_register_ctx_from_node_id(pTrustList->pObjectId, pTrustList);
        status = !res ? SOPC_STATUS_NOK : SOPC_STATUS_OK;
    }
    if (SOPC_STATUS_OK != status)
    {
        trustlist_delete_context(&pTrustList);
    }
    else
    {
        if (SOPC_TRUSTLIST_GROUP_APP == pCfg->groupType)
        {
            gbGroupAppIsSet = true;
        }
        if (SOPC_TRUSTLIST_GROUP_USR == pCfg->groupType)
        {
            gbGroupUsrIsSet = true;
        }
    }
    return status;
}

void SOPC_TrustList_Clear(void)
{
    SOPC_Dict_Delete(gObjIdToTrustList);
    gObjIdToTrustList = NULL;
    gbTypeMetIsConfigure = false;
    gbGroupAppIsSet = false;
    gbGroupUsrIsSet = false;
}

/*---------------------------------------------------------------------------
 *                       Internal Functions (implementation)
 *---------------------------------------------------------------------------*/

/* Get the trustList context from the nodeId */
SOPC_TrustListContext* TrustList_GetFromNodeId(const SOPC_NodeId* pObjectId,
                                               bool bCheckActivityTimeout,
                                               const SOPC_CallContext* callContextPtr,
                                               bool* bFound)
{
    if (NULL == gObjIdToTrustList || NULL == pObjectId)
    {
        *bFound = false;
        return NULL;
    }
    SOPC_TrustListContext* pCtx = NULL;
    pCtx = (SOPC_TrustListContext*) SOPC_Dict_Get(gObjIdToTrustList, (const uintptr_t) pObjectId, bFound);
    if (!bFound || NULL == pCtx)
    {
        char* cStrId = SOPC_NodeId_ToCString(pObjectId);
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList: unable to retrieve TrustList context from %s",
                               cStrId);
        SOPC_Free(cStrId);
    }
    else
    {
        if (bCheckActivityTimeout)
        {
            /* Check the elapsed period and close the TrustList if necessary */
            if (1 == SOPC_Atomic_Int_Get(&pCtx->eventMgr.timeoutElapsed))
            {
                /* Close the TrustList */
                SOPC_AddressSpaceAccess* pAddSpAccess = NULL;
                if (NULL != callContextPtr)
                {
                    pAddSpAccess = SOPC_CallContext_GetAddressSpaceAccess(callContextPtr);
                }
                TrustList_Reset(pCtx, pAddSpAccess);
                SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                         "TrustList:%s: activity timeout period elapsed (TrustList has been closed)",
                                         pCtx->cStrObjectId);
            }
        }
    }
    return pCtx;
}

/* Removes a TrustList context from the nodeId */
void TrustList_RemoveFromNodeId(const SOPC_NodeId* pObjectId)
{
    if (NULL == gObjIdToTrustList || NULL == pObjectId)
    {
        return;
    }
    SOPC_Dict_Remove(gObjIdToTrustList, (const uintptr_t) pObjectId);
}
