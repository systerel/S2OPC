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
 *                             Constants
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------
 *                             Internal types
 *---------------------------------------------------------------------------*/

/**
 * \brief Internal structure to gather nodeIds.
 */
typedef struct TrustList_NodeIds
{
    SOPC_NodeId* pTrustListId;         /*!< The nodeId of the FileType object. */
    SOPC_NodeId* pOpenId;              /*!< The nodeId of the Open method. */
    SOPC_NodeId* pOpenWithMasksId;     /*!< The nodeId of the OpenWithMasks method. */
    SOPC_NodeId* pCloseAndUpdateId;    /*!< The nodeId of the CloseAndUpdate method. */
    SOPC_NodeId* pAddCertificateId;    /*!< The nodeId of the AddCertificate method. */
    SOPC_NodeId* pRemoveCertificateId; /*!< The nodeId of the RemoveCertificate method. */
    SOPC_NodeId* pCloseId;             /*!< The nodeId of the Close method. */
    SOPC_NodeId* pReadId;              /*!< The nodeId of the Read method. */
    SOPC_NodeId* pWriteId;             /*!< The nodeId of the Write method. */
    SOPC_NodeId* pGetPosId;            /*!< The nodeId of the GetPosition method. */
    SOPC_NodeId* pSetPosId;            /*!< The nodeId of the SetPosition method. */
    SOPC_NodeId* pSizeId;              /*!< The nodeId of the Size variable. */
    SOPC_NodeId* pOpenCountId;         /*!< The nodeId of the OpenCount variable. */
    SOPC_NodeId* pUserWritableId;      /*!< The nodeId of the UserWritable variable. */
    SOPC_NodeId* pWritableId;          /*!< The nodeId of the Writable variable. */
    SOPC_NodeId* pLastUpdateTimeId;    /*!< The nodeId of the LastUpdateTime variable */
} TrustList_NodeIds;

/**
 * \brief Structure to gather TrustList configuration data.
 */
struct SOPC_TrustList_Config
{
    TrustList_NodeIds* pIds;       /*!< Defined all the nodeId of the TrustList. */
    bool bDoNotClearIds;           /*!< Defined if the method nodeIds shall be clear */
    SOPC_TrustList_Type groupType; /*!< Defined the certificate group type of the TrustList. */
    SOPC_PKIProvider* pPKI;        /*!< A valid pointer to the PKI of the TrustList. */
    uint32_t maxTrustListSize;     /*!< Defined the maximum size in byte for the TrustList. */
};

/*---------------------------------------------------------------------------
 *                             Global variables
 *---------------------------------------------------------------------------*/

static SOPC_Dict* gObjIdToTrustList = NULL;
static bool gbTypeMetIsConfigure = false;
static bool gbGroupAppIsSet = false;
static bool gbGroupUsrIsSet = false;

static int32_t gTombstoneKey = -1;

/* NodeIds of the TrustList instance of the DefaultApplicationGroup */
static SOPC_NodeId gAppTrustListId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultApplicationGroup_TrustList};
static SOPC_NodeId gAppOpenId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultApplicationGroup_TrustList_Open};
static SOPC_NodeId gAppCloseId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultApplicationGroup_TrustList_Close};
static SOPC_NodeId gAppReadId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultApplicationGroup_TrustList_Read};
static SOPC_NodeId gAppWriteId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultApplicationGroup_TrustList_Write};
static SOPC_NodeId gAppGetPositionId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultApplicationGroup_TrustList_GetPosition};
static SOPC_NodeId gAppSetPositionId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultApplicationGroup_TrustList_SetPosition};
static SOPC_NodeId gAppOpenWithMasksId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultApplicationGroup_TrustList_OpenWithMasks};
static SOPC_NodeId gAppCloseAndUpdateId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultApplicationGroup_TrustList_CloseAndUpdate};
static SOPC_NodeId gAppAddCertificateId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultApplicationGroup_TrustList_AddCertificate};
static SOPC_NodeId gAppRemoveCertificateId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultApplicationGroup_TrustList_RemoveCertificate};
static SOPC_NodeId gAppOpenCountId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultApplicationGroup_TrustList_OpenCount};
static SOPC_NodeId gAppSizeId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultApplicationGroup_TrustList_Size};
static SOPC_NodeId gAppWritableId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultApplicationGroup_TrustList_Writable};
static SOPC_NodeId gAppUserWritableId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultApplicationGroup_TrustList_UserWritable};
static SOPC_NodeId gAppLastUpdateTimeId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultApplicationGroup_TrustList_LastUpdateTime};

/* NodeIds of the TrustList instance of the DefaultUserTokenGroup */
static SOPC_NodeId gUsrTrustListId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultUserTokenGroup_TrustList};
static SOPC_NodeId gUsrOpenId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultUserTokenGroup_TrustList_Open};
static SOPC_NodeId gUsrCloseId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultUserTokenGroup_TrustList_Close};
static SOPC_NodeId gUsrReadId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultUserTokenGroup_TrustList_Read};
static SOPC_NodeId gUsrWriteId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultUserTokenGroup_TrustList_Write};
static SOPC_NodeId gUsrGetPositionId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultUserTokenGroup_TrustList_GetPosition};
static SOPC_NodeId gUsrSetPositionId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultUserTokenGroup_TrustList_SetPosition};
static SOPC_NodeId gUsrOpenWithMasksId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultUserTokenGroup_TrustList_OpenWithMasks};
static SOPC_NodeId gUsrCloseAndUpdateId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultUserTokenGroup_TrustList_CloseAndUpdate};
static SOPC_NodeId gUsrAddCertificateId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultUserTokenGroup_TrustList_AddCertificate};
static SOPC_NodeId gUsrRemoveCertificateId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultUserTokenGroup_TrustList_RemoveCertificate};
static SOPC_NodeId gUsrOpenCountId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultUserTokenGroup_TrustList_OpenCount};
static SOPC_NodeId gUsrSizeId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultUserTokenGroup_TrustList_Size};
static SOPC_NodeId gUsrWritableId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultUserTokenGroup_TrustList_Writable};
static SOPC_NodeId gUsrUserWritableId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultUserTokenGroup_TrustList_UserWritable};
static SOPC_NodeId gUsrLastUpdateTimeId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultUserTokenGroup_TrustList_LastUpdateTime};

/* NodeIds of the TrustListType */
static SOPC_NodeId gTypeOpenId = {.IdentifierType = SOPC_IdentifierType_Numeric,
                                  .Namespace = 0,
                                  .Data.Numeric = OpcUaId_FileType_Open};
static SOPC_NodeId gTypeCloseId = {.IdentifierType = SOPC_IdentifierType_Numeric,
                                   .Namespace = 0,
                                   .Data.Numeric = OpcUaId_FileType_Close};
static SOPC_NodeId gTypeReadId = {.IdentifierType = SOPC_IdentifierType_Numeric,
                                  .Namespace = 0,
                                  .Data.Numeric = OpcUaId_FileType_Read};
static SOPC_NodeId gTypeWriteId = {.IdentifierType = SOPC_IdentifierType_Numeric,
                                   .Namespace = 0,
                                   .Data.Numeric = OpcUaId_FileType_Write};
static SOPC_NodeId gTypeGetPositionId = {.IdentifierType = SOPC_IdentifierType_Numeric,
                                         .Namespace = 0,
                                         .Data.Numeric = OpcUaId_FileType_GetPosition};
static SOPC_NodeId gTypeSetPositionId = {.IdentifierType = SOPC_IdentifierType_Numeric,
                                         .Namespace = 0,
                                         .Data.Numeric = OpcUaId_FileType_SetPosition};
static SOPC_NodeId gTypeOpenWithMasksId = {.IdentifierType = SOPC_IdentifierType_Numeric,
                                           .Namespace = 0,
                                           .Data.Numeric = OpcUaId_TrustListType_OpenWithMasks};
static SOPC_NodeId gTypeCloseAndUpdateId = {.IdentifierType = SOPC_IdentifierType_Numeric,
                                            .Namespace = 0,
                                            .Data.Numeric = OpcUaId_TrustListType_CloseAndUpdate};
static SOPC_NodeId gTypeAddCertificateId = {.IdentifierType = SOPC_IdentifierType_Numeric,
                                            .Namespace = 0,
                                            .Data.Numeric = OpcUaId_TrustListType_AddCertificate};
static SOPC_NodeId gTypeRemoveCertificateId = {.IdentifierType = SOPC_IdentifierType_Numeric,
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

static TrustList_NodeIds gTypeNodeIds = {
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
                                                  uint32_t maxTrustListSize);
static void trustlist_initialize_context(SOPC_TrustListContext* pTrustList,
                                         SOPC_TrustList_Type groupType,
                                         SOPC_PKIProvider* pPKI,
                                         uint32_t maxTrustListSize);
static void trustlist_clear_context(SOPC_TrustListContext* pTrustList);
static void trustlist_delete_context(SOPC_TrustListContext** ppTrustList);
static void trustlist_dict_free_context_value(uintptr_t value);

static void trustlist_delete_single_node_id(SOPC_NodeId** ppNodeId);
static void trustlist_delete_node_ids(TrustList_NodeIds** ppNodeIds, bool bDoNotClear);
static SOPC_ReturnStatus trustlist_copy_meth_node_ids(TrustList_NodeIds* pSrc, TrustList_NodeIds** ppDest);
static SOPC_ReturnStatus trustlist_copy_node_ids(TrustList_NodeIds* pSrc, TrustList_NodeIds** ppDest);

static SOPC_ReturnStatus trustlist_add_method_type_nodeId(SOPC_MethodCallManager* pMcm);

static void sopc_trustlist_activity_timeout_cb(SOPC_EventHandler* pHandler,
                                               int32_t event,
                                               uint32_t eltId,
                                               uintptr_t pParams,
                                               uintptr_t pAuxParam);

/*---------------------------------------------------------------------------
 *                       Static functions (implementation)
 *---------------------------------------------------------------------------*/

static SOPC_ReturnStatus trustlist_create_context(SOPC_TrustListContext** ppTrustList,
                                                  SOPC_TrustList_Type groupType,
                                                  SOPC_PKIProvider* pPKI,
                                                  uint32_t maxTrustListSize)
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
                                         uint32_t maxTrustListSize)
{
    SOPC_ASSERT(NULL != pTrustList);
    SOPC_ASSERT(NULL != pPKI);

    pTrustList->pObjectId = NULL;
    pTrustList->cStrObjectId = NULL;
    pTrustList->varIds.pOpenCountId = NULL;
    pTrustList->varIds.pSizeId = NULL;
    pTrustList->varIds.pUserWritableId = NULL;
    pTrustList->varIds.pWritableId = NULL;
    pTrustList->varIds.pLastUpdateTimeId = NULL;
    pTrustList->maxTrustListSize = maxTrustListSize;
    pTrustList->groupType = groupType;
    pTrustList->pPKI = pPKI;
    pTrustList->event.pLooper = NULL;
    pTrustList->event.pHandler = NULL;
    pTrustList->event.activityTimeoutTimId = 0;
    TrustList_Reset(pTrustList, NULL);
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
    trustlist_delete_single_node_id(&pTrustList->pObjectId);
    trustlist_delete_single_node_id(&pTrustList->varIds.pSizeId);
    trustlist_delete_single_node_id(&pTrustList->varIds.pWritableId);
    trustlist_delete_single_node_id(&pTrustList->varIds.pUserWritableId);
    trustlist_delete_single_node_id(&pTrustList->varIds.pOpenCountId);
    trustlist_delete_single_node_id(&pTrustList->varIds.pLastUpdateTimeId);
    SOPC_Looper_Delete(pTrustList->event.pLooper);
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

static void trustlist_delete_single_node_id(SOPC_NodeId** ppNodeId)
{
    if (NULL == ppNodeId)
    {
        return;
    }
    SOPC_NodeId* pNodeId = *ppNodeId;
    if (NULL == pNodeId)
    {
        return;
    }
    SOPC_NodeId_Clear(pNodeId);
    SOPC_Free(pNodeId);
    pNodeId = NULL;
    *ppNodeId = pNodeId;
}

static void trustlist_delete_node_ids(TrustList_NodeIds** ppNodeIds, bool bDoNotClear)
{
    if (NULL == ppNodeIds)
    {
        return;
    }
    TrustList_NodeIds* pNodeIds = *ppNodeIds;
    if (NULL == pNodeIds)
    {
        return;
    }
    if (!bDoNotClear)
    {
        /* Deleted by the MethodCall manager */
        trustlist_delete_single_node_id(&pNodeIds->pOpenId);
        trustlist_delete_single_node_id(&pNodeIds->pOpenWithMasksId);
        trustlist_delete_single_node_id(&pNodeIds->pCloseAndUpdateId);
        trustlist_delete_single_node_id(&pNodeIds->pAddCertificateId);
        trustlist_delete_single_node_id(&pNodeIds->pRemoveCertificateId);
        trustlist_delete_single_node_id(&pNodeIds->pCloseId);
        trustlist_delete_single_node_id(&pNodeIds->pReadId);
        trustlist_delete_single_node_id(&pNodeIds->pWriteId);
        trustlist_delete_single_node_id(&pNodeIds->pGetPosId);
        trustlist_delete_single_node_id(&pNodeIds->pSetPosId);
    }
    trustlist_delete_single_node_id(&pNodeIds->pTrustListId);
    trustlist_delete_single_node_id(&pNodeIds->pSizeId);
    trustlist_delete_single_node_id(&pNodeIds->pOpenCountId);
    trustlist_delete_single_node_id(&pNodeIds->pUserWritableId);
    trustlist_delete_single_node_id(&pNodeIds->pWritableId);
    trustlist_delete_single_node_id(&pNodeIds->pLastUpdateTimeId);
    SOPC_Free(pNodeIds);
    pNodeIds = NULL;
    *ppNodeIds = pNodeIds;
}

static SOPC_ReturnStatus trustlist_copy_meth_node_ids(TrustList_NodeIds* pSrc, TrustList_NodeIds** ppDest)
{
    if (NULL == pSrc || NULL == ppDest)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (NULL == pSrc->pOpenId || NULL == pSrc->pOpenWithMasksId || NULL == pSrc->pCloseAndUpdateId ||
        NULL == pSrc->pAddCertificateId || NULL == pSrc->pRemoveCertificateId || NULL == pSrc->pCloseId ||
        NULL == pSrc->pReadId || NULL == pSrc->pWriteId || NULL == pSrc->pGetPosId || NULL == pSrc->pSetPosId)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    *ppDest = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    TrustList_NodeIds* pDest = NULL;
    pDest = SOPC_Calloc(1, sizeof(TrustList_NodeIds));
    if (NULL == pDest)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }
    pDest->pOpenId = SOPC_Calloc(1, sizeof(SOPC_NodeId));
    status = SOPC_NodeId_Copy(pDest->pOpenId, pSrc->pOpenId);
    if (SOPC_STATUS_OK == status)
    {
        pDest->pOpenWithMasksId = SOPC_Calloc(1, sizeof(SOPC_NodeId));
        status = SOPC_NodeId_Copy(pDest->pOpenWithMasksId, pSrc->pOpenWithMasksId);
    }
    if (SOPC_STATUS_OK == status)
    {
        pDest->pCloseAndUpdateId = SOPC_Calloc(1, sizeof(SOPC_NodeId));
        status = SOPC_NodeId_Copy(pDest->pCloseAndUpdateId, pSrc->pCloseAndUpdateId);
    }
    if (SOPC_STATUS_OK == status)
    {
        pDest->pAddCertificateId = SOPC_Calloc(1, sizeof(SOPC_NodeId));
        status = SOPC_NodeId_Copy(pDest->pAddCertificateId, pSrc->pAddCertificateId);
    }
    if (SOPC_STATUS_OK == status)
    {
        pDest->pRemoveCertificateId = SOPC_Calloc(1, sizeof(SOPC_NodeId));
        status = SOPC_NodeId_Copy(pDest->pRemoveCertificateId, pSrc->pRemoveCertificateId);
    }
    if (SOPC_STATUS_OK == status)
    {
        pDest->pCloseId = SOPC_Calloc(1, sizeof(SOPC_NodeId));
        status = SOPC_NodeId_Copy(pDest->pCloseId, pSrc->pCloseId);
    }
    if (SOPC_STATUS_OK == status)
    {
        pDest->pReadId = SOPC_Calloc(1, sizeof(SOPC_NodeId));
        status = SOPC_NodeId_Copy(pDest->pReadId, pSrc->pReadId);
    }
    if (SOPC_STATUS_OK == status)
    {
        pDest->pWriteId = SOPC_Calloc(1, sizeof(SOPC_NodeId));
        status = SOPC_NodeId_Copy(pDest->pWriteId, pSrc->pWriteId);
    }
    if (SOPC_STATUS_OK == status)
    {
        pDest->pGetPosId = SOPC_Calloc(1, sizeof(SOPC_NodeId));
        status = SOPC_NodeId_Copy(pDest->pGetPosId, pSrc->pGetPosId);
    }
    if (SOPC_STATUS_OK == status)
    {
        pDest->pSetPosId = SOPC_Calloc(1, sizeof(SOPC_NodeId));
        status = SOPC_NodeId_Copy(pDest->pSetPosId, pSrc->pSetPosId);
    }
    /* Clear */
    if (SOPC_STATUS_OK != status)
    {
        trustlist_delete_node_ids(&pDest, false);
    }
    *ppDest = pDest;
    return status;
}

static SOPC_ReturnStatus trustlist_copy_node_ids(TrustList_NodeIds* pSrc, TrustList_NodeIds** ppDest)
{
    if (NULL == pSrc || NULL == ppDest)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (NULL == pSrc->pTrustListId || NULL == pSrc->pOpenId || NULL == pSrc->pOpenWithMasksId ||
        NULL == pSrc->pCloseAndUpdateId || NULL == pSrc->pAddCertificateId || NULL == pSrc->pRemoveCertificateId ||
        NULL == pSrc->pCloseId || NULL == pSrc->pReadId || NULL == pSrc->pWriteId || NULL == pSrc->pGetPosId ||
        NULL == pSrc->pSetPosId || NULL == pSrc->pSizeId || NULL == pSrc->pOpenCountId ||
        NULL == pSrc->pUserWritableId || NULL == pSrc->pWritableId || NULL == pSrc->pLastUpdateTimeId)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    *ppDest = NULL;
    TrustList_NodeIds* pDest = NULL;
    SOPC_ReturnStatus status = trustlist_copy_meth_node_ids(pSrc, &pDest);
    if (SOPC_STATUS_OK != status)
    {
        return status;
    }
    pDest->pTrustListId = SOPC_Calloc(1, sizeof(SOPC_NodeId));
    status = SOPC_NodeId_Copy(pDest->pTrustListId, pSrc->pTrustListId);
    if (SOPC_STATUS_OK == status)
    {
        pDest->pSizeId = SOPC_Calloc(1, sizeof(SOPC_NodeId));
        status = SOPC_NodeId_Copy(pDest->pSizeId, pSrc->pSizeId);
    }
    if (SOPC_STATUS_OK == status)
    {
        pDest->pOpenCountId = SOPC_Calloc(1, sizeof(SOPC_NodeId));
        status = SOPC_NodeId_Copy(pDest->pOpenCountId, pSrc->pOpenCountId);
    }
    if (SOPC_STATUS_OK == status)
    {
        pDest->pUserWritableId = SOPC_Calloc(1, sizeof(SOPC_NodeId));
        status = SOPC_NodeId_Copy(pDest->pUserWritableId, pSrc->pUserWritableId);
    }
    if (SOPC_STATUS_OK == status)
    {
        pDest->pWritableId = SOPC_Calloc(1, sizeof(SOPC_NodeId));
        status = SOPC_NodeId_Copy(pDest->pWritableId, pSrc->pWritableId);
    }
    if (SOPC_STATUS_OK == status)
    {
        pDest->pLastUpdateTimeId = SOPC_Calloc(1, sizeof(SOPC_NodeId));
        status = SOPC_NodeId_Copy(pDest->pLastUpdateTimeId, pSrc->pLastUpdateTimeId);
    }
    if (SOPC_STATUS_OK != status)
    {
        trustlist_delete_node_ids(&pDest, false);
    }
    *ppDest = pDest;
    return status;
}

static SOPC_ReturnStatus trustlist_add_method_type_nodeId(SOPC_MethodCallManager* pMcm)
{
    if (NULL == pMcm)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    TrustList_NodeIds* pIds = NULL;
    SOPC_ReturnStatus status = trustlist_copy_meth_node_ids(&gTypeNodeIds, &pIds);
    if (NULL == pIds)
    {
        return status;
    }
    SOPC_ReturnStatus statusMcm = SOPC_MethodCallManager_AddMethod(
        pMcm, pIds->pOpenWithMasksId, &TrustList_Method_OpenWithMasks, "TypeOpenWithMasks", NULL);
    if (SOPC_STATUS_OK != statusMcm)
    {
        /* But if AddMethod failed, the manager do not clear the nodeId */
        trustlist_delete_single_node_id(&pIds->pOpenWithMasksId);
        status = SOPC_STATUS_INVALID_STATE;
    }
    statusMcm = SOPC_MethodCallManager_AddMethod(pMcm, pIds->pOpenId, &TrustList_Method_Open, "TypeOpen", NULL);
    if (SOPC_STATUS_OK != statusMcm)
    {
        /* But if AddMethod failed, the manager do not clear the nodeId */
        trustlist_delete_single_node_id(&pIds->pOpenId);
        status = SOPC_STATUS_INVALID_STATE;
    }
    statusMcm = SOPC_MethodCallManager_AddMethod(pMcm, pIds->pCloseId, &TrustList_Method_Close, "TypeClose", NULL);
    if (SOPC_STATUS_OK != statusMcm)
    {
        /* But if AddMethod failed, the manager do not clear the nodeId */
        trustlist_delete_single_node_id(&pIds->pCloseId);
        status = SOPC_STATUS_INVALID_STATE;
    }
    statusMcm = SOPC_MethodCallManager_AddMethod(pMcm, pIds->pCloseAndUpdateId, &TrustList_Method_CloseAndUpdate,
                                                 "TypeCloseAndUpdate", NULL);
    if (SOPC_STATUS_OK != statusMcm)
    {
        /* But if AddMethod failed, the manager do not clear the nodeId */
        trustlist_delete_single_node_id(&pIds->pCloseAndUpdateId);
        status = SOPC_STATUS_INVALID_STATE;
    }
    statusMcm = SOPC_MethodCallManager_AddMethod(pMcm, pIds->pAddCertificateId, &TrustList_Method_AddCertificate,
                                                 "TypeAddCertificate", NULL);
    if (SOPC_STATUS_OK != statusMcm)
    {
        /* But if AddMethod failed, the manager do not clear the nodeId */
        trustlist_delete_single_node_id(&pIds->pAddCertificateId);
        status = SOPC_STATUS_INVALID_STATE;
    }
    statusMcm = SOPC_MethodCallManager_AddMethod(pMcm, pIds->pRemoveCertificateId, &TrustList_Method_RemoveCertificate,
                                                 "TypeRemoveCertificate", NULL);
    if (SOPC_STATUS_OK != statusMcm)
    {
        /* But if AddMethod failed, the manager do not clear the nodeId */
        trustlist_delete_single_node_id(&pIds->pRemoveCertificateId);
        status = SOPC_STATUS_INVALID_STATE;
    }
    statusMcm = SOPC_MethodCallManager_AddMethod(pMcm, pIds->pReadId, &TrustList_Method_Read, "TypeRead", NULL);
    if (SOPC_STATUS_OK != statusMcm)
    {
        /* But if AddMethod failed, the manager do not clear the nodeId */
        trustlist_delete_single_node_id(&pIds->pReadId);
        status = SOPC_STATUS_INVALID_STATE;
    }
    statusMcm = SOPC_MethodCallManager_AddMethod(pMcm, pIds->pWriteId, &TrustList_Method_Write, "TypeWrite", NULL);
    if (SOPC_STATUS_OK != statusMcm)
    {
        /* But if AddMethod failed, the manager do not clear the nodeId */
        trustlist_delete_single_node_id(&pIds->pWriteId);
        status = SOPC_STATUS_INVALID_STATE;
    }
    statusMcm =
        SOPC_MethodCallManager_AddMethod(pMcm, pIds->pGetPosId, &TrustList_Method_GetPosition, "TypeGetPosition", NULL);
    if (SOPC_STATUS_OK != statusMcm)
    {
        /* But if AddMethod failed, the manager do not clear the nodeId */
        trustlist_delete_single_node_id(&pIds->pGetPosId);
        status = SOPC_STATUS_INVALID_STATE;
    }
    statusMcm =
        SOPC_MethodCallManager_AddMethod(pMcm, pIds->pSetPosId, &TrustList_Method_SetPosition, "TypeSetPosition", NULL);
    if (SOPC_STATUS_OK != statusMcm)
    {
        /* But if AddMethod failed, the manager do not clear the nodeId */
        trustlist_delete_single_node_id(&pIds->pSetPosId);
        status = SOPC_STATUS_INVALID_STATE;
    }
    /* Clear */
    SOPC_Free(pIds);
    return status;
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
        SOPC_Atomic_Int_Set(&pCtx->event.timeoutElapsed, 1);
    }
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
    SOPC_ReturnStatus status = trustlist_copy_node_ids(pNodeIds, &pCfg->pIds);
    if (SOPC_STATUS_OK == status)
    {
        pCfg->groupType = groupType;
        pCfg->pPKI = pPKI;
        pCfg->maxTrustListSize = maxTrustListSize;
        pCfg->bDoNotClearIds = false;
    }
    else
    {
        SOPC_Free(pCfg);
        pCfg = NULL;
    }
    *ppConfig = pCfg;
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
    trustlist_delete_node_ids(&pConfig->pIds, pConfig->bDoNotClearIds);
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
    TrustList_NodeIds* pIds = pCfg->pIds;
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
        trustlist_create_context(&pTrustList, pCfg->groupType, pCfg->pPKI, pCfg->maxTrustListSize);
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
        pTrustList->event.pLooper = SOPC_Looper_Create(pTrustList->cStrObjectId);
        if (NULL == pTrustList->event.pLooper)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        pTrustList->event.pHandler =
            SOPC_EventHandler_Create(pTrustList->event.pLooper, sopc_trustlist_activity_timeout_cb);
        if (NULL == pTrustList->event.pHandler)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        pTrustList->pObjectId = SOPC_Calloc(1, sizeof(SOPC_NodeId));
        status = SOPC_NodeId_Copy(pTrustList->pObjectId, pIds->pTrustListId);
    }
    /* Add methods that belongs to the FileType baseObject */
    if (SOPC_STATUS_OK == status && !gbTypeMetIsConfigure)
    {
        status = trustlist_add_method_type_nodeId(pMcm);
        gbTypeMetIsConfigure = true;
    }
    /* Add methods ... */
    if (SOPC_STATUS_OK == status)
    {
        /* Method nodeIds are clear by the methodCall manager */
        pCfg->bDoNotClearIds = true;
        SOPC_ReturnStatus statusMcm = SOPC_MethodCallManager_AddMethod(
            pMcm, pIds->pOpenWithMasksId, &TrustList_Method_OpenWithMasks, "OpenWithMasks", NULL);
        if (SOPC_STATUS_OK != statusMcm)
        {
            /* But if AddMethod failed, the manager do not clear the nodeId */
            trustlist_delete_single_node_id(&pIds->pOpenWithMasksId);
            status = SOPC_STATUS_INVALID_STATE;
        }
        statusMcm = SOPC_MethodCallManager_AddMethod(pMcm, pIds->pOpenId, &TrustList_Method_Open, "Open", NULL);
        if (SOPC_STATUS_OK != statusMcm)
        {
            /* But if AddMethod failed, the manager do not clear the nodeId */
            trustlist_delete_single_node_id(&pIds->pOpenId);
            status = SOPC_STATUS_INVALID_STATE;
        }
        statusMcm = SOPC_MethodCallManager_AddMethod(pMcm, pIds->pCloseId, &TrustList_Method_Close, "Close", NULL);
        if (SOPC_STATUS_OK != statusMcm)
        {
            /* But if AddMethod failed, the manager do not clear the nodeId */
            trustlist_delete_single_node_id(&pIds->pCloseId);
            status = SOPC_STATUS_INVALID_STATE;
        }
        statusMcm = SOPC_MethodCallManager_AddMethod(pMcm, pIds->pCloseAndUpdateId, &TrustList_Method_CloseAndUpdate,
                                                     "CloseAndUpdate", NULL);
        if (SOPC_STATUS_OK != statusMcm)
        {
            /* But if AddMethod failed, the manager do not clear the nodeId */
            trustlist_delete_single_node_id(&pIds->pCloseAndUpdateId);
            status = SOPC_STATUS_INVALID_STATE;
        }
        statusMcm = SOPC_MethodCallManager_AddMethod(pMcm, pIds->pAddCertificateId, &TrustList_Method_AddCertificate,
                                                     "AddCertificate", NULL);
        if (SOPC_STATUS_OK != statusMcm)
        {
            /* But if AddMethod failed, the manager do not clear the nodeId */
            trustlist_delete_single_node_id(&pIds->pAddCertificateId);
            status = SOPC_STATUS_INVALID_STATE;
        }
        statusMcm = SOPC_MethodCallManager_AddMethod(pMcm, pIds->pRemoveCertificateId,
                                                     &TrustList_Method_RemoveCertificate, "RemoveCertificate", NULL);
        if (SOPC_STATUS_OK != statusMcm)
        {
            /* But if AddMethod failed, the manager do not clear the nodeId */
            trustlist_delete_single_node_id(&pIds->pRemoveCertificateId);
            status = SOPC_STATUS_INVALID_STATE;
        }
        statusMcm = SOPC_MethodCallManager_AddMethod(pMcm, pIds->pReadId, &TrustList_Method_Read, "Read", NULL);
        if (SOPC_STATUS_OK != statusMcm)
        {
            /* But if AddMethod failed, the manager do not clear the nodeId */
            trustlist_delete_single_node_id(&pIds->pReadId);
            status = SOPC_STATUS_INVALID_STATE;
        }
        statusMcm = SOPC_MethodCallManager_AddMethod(pMcm, pIds->pWriteId, &TrustList_Method_Write, "Write", NULL);
        if (SOPC_STATUS_OK != statusMcm)
        {
            /* But if AddMethod failed, the manager do not clear the nodeId */
            trustlist_delete_single_node_id(&pIds->pWriteId);
            status = SOPC_STATUS_INVALID_STATE;
        }
        statusMcm =
            SOPC_MethodCallManager_AddMethod(pMcm, pIds->pGetPosId, &TrustList_Method_GetPosition, "GetPosition", NULL);
        if (SOPC_STATUS_OK != statusMcm)
        {
            /* But if AddMethod failed, the manager do not clear the nodeId */
            trustlist_delete_single_node_id(&pIds->pGetPosId);
            status = SOPC_STATUS_INVALID_STATE;
        }
        statusMcm =
            SOPC_MethodCallManager_AddMethod(pMcm, pIds->pSetPosId, &TrustList_Method_SetPosition, "SetPosition", NULL);
        if (SOPC_STATUS_OK != statusMcm)
        {
            /* But if AddMethod failed, the manager do not clear the nodeId */
            trustlist_delete_single_node_id(&pIds->pSetPosId);
            status = SOPC_STATUS_INVALID_STATE;
        }
    }
    /* Add variables ... */
    if (SOPC_STATUS_OK == status)
    {
        pTrustList->varIds.pSizeId = SOPC_Calloc(1, sizeof(SOPC_NodeId));
        status = SOPC_NodeId_Copy(pTrustList->varIds.pSizeId, pIds->pSizeId);
    }
    if (SOPC_STATUS_OK == status)
    {
        pTrustList->varIds.pWritableId = SOPC_Calloc(1, sizeof(SOPC_NodeId));
        status = SOPC_NodeId_Copy(pTrustList->varIds.pWritableId, pIds->pWritableId);
    }
    if (SOPC_STATUS_OK == status)
    {
        pTrustList->varIds.pUserWritableId = SOPC_Calloc(1, sizeof(SOPC_NodeId));
        status = SOPC_NodeId_Copy(pTrustList->varIds.pUserWritableId, pIds->pUserWritableId);
    }
    if (SOPC_STATUS_OK == status)
    {
        pTrustList->varIds.pOpenCountId = SOPC_Calloc(1, sizeof(SOPC_NodeId));
        status = SOPC_NodeId_Copy(pTrustList->varIds.pOpenCountId, pIds->pOpenCountId);
    }
    if (SOPC_STATUS_OK == status)
    {
        pTrustList->varIds.pLastUpdateTimeId = SOPC_Calloc(1, sizeof(SOPC_NodeId));
        status = SOPC_NodeId_Copy(pTrustList->varIds.pLastUpdateTimeId, pIds->pLastUpdateTimeId);
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
        char* cStrId = SOPC_NodeId_ToCString(pObjectId);
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList: unable to insert TrustList context to %s",
                               cStrId);
        SOPC_Free(cStrId);
    }
    return res;
}

/* Get the trustList context from the nodeId */
SOPC_TrustListContext* TrustList_DictGet(const SOPC_NodeId* pObjectId,
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
            if (1 == SOPC_Atomic_Int_Get(&pCtx->event.timeoutElapsed))
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
void TrustList_DictRemove(const SOPC_NodeId* pObjectId)
{
    if (NULL == gObjIdToTrustList || NULL == pObjectId)
    {
        return;
    }
    SOPC_Dict_Remove(gObjIdToTrustList, (const uintptr_t) pObjectId);
}
