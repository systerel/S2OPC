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
 * \brief Internal API implementation to manage methods, properties and variables of the ServerConfigurationType
 * according the Push model.
 */

#include <string.h>

#include "sopc_assert.h"
#include "sopc_logger.h"
#include "sopc_mem_alloc.h"

#include "sopc_certificate_group.h"
#include "sopc_push_server_config.h"
#include "sopc_push_server_config_itf.h"
#include "sopc_push_server_config_meth.h"

#include "opcua_identifiers.h"
#include "opcua_statuscodes.h"

/*---------------------------------------------------------------------------
 *                             Constants
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
 *                             Internal types
 *---------------------------------------------------------------------------*/
/**
 * \brief Internal structure to share context through method call.
 */
typedef struct PushServerContext
{
    bool bIsInit;                        /*!< Defined if the API is configured */
    bool bIsConfigure;                   /*!< Defined if the API is initialized. */
    SOPC_NodeId* pServerConfigurationId; /*!< The nodeId of the ServerConfiguration object. */
    SOPC_NodeId* pAppGroupId;            /*!< The nodeId of the application Group. */
    SOPC_NodeId* pUsrGroupId;            /*!< The nodeId of the userToken group. */
} PushServerContext;

/**
 * \brief Internal structure to gather nodeIds.
 */
typedef struct PushServerConfig_NodeIds
{
    SOPC_NodeId* pServerConfigurationId;  /*!< The nodeId of the ServerConfiguration object. */
    SOPC_NodeId* pUpdateCertificateId;    /*!< The nodeId of the UpdateCertificate method. */
    SOPC_NodeId* pApplyChangesId;         /*!< The nodeId of the ApplyChanges method.*/
    SOPC_NodeId* pCreateSigningRequestId; /*!< The nodeId of the CreateSigningRequest method. */
    SOPC_NodeId* pGetRejectedListId;      /*!< The nodeId of the CreateSigningRequest method. */
    SOPC_NodeId* pAppGroupId;             /*!< The nodeId of the application group. */
    SOPC_NodeId* pUsrGroupId;             /*!< The nodeId of the userToken group. */
} PushServerConfig_NodeIds;

/**
 * \brief Structure to gather the ServerConfiguration object data
 */
struct SOPC_PushServerConfig_Config
{
    PushServerConfig_NodeIds* pIds;                 /*!< Defined all the nodeId of the ServerConfiguration. */
    bool bDoNotClearIds;                            /*!< Defined if the method nodeIds shall be clear */
    SOPC_CertificateGroup_Config* pAppCertGroupCfg; /*!< Application certificate group configuration that
                                                         belongs to the CertificateGroupeFolderType */
    SOPC_CertificateGroup_Config* pUsrCertGroupCfg; /*!< Users certificate group configuration that belongs to the
                                                         CertificateGroupeFolderType (NULL if not used) */
};

/*---------------------------------------------------------------------------
 *                             Global variables
 *---------------------------------------------------------------------------*/

/* NodeIds of the ServerConfiguration instance  */
static SOPC_NodeId gServerConfigurationId = {.IdentifierType = SOPC_IdentifierType_Numeric,
                                             .Namespace = 0,
                                             .Data.Numeric = OpcUaId_ServerConfiguration};
static SOPC_NodeId gUpdateCertificateId = {.IdentifierType = SOPC_IdentifierType_Numeric,
                                           .Namespace = 0,
                                           .Data.Numeric = OpcUaId_ServerConfiguration_UpdateCertificate};
static SOPC_NodeId gApplyChangesId = {.IdentifierType = SOPC_IdentifierType_Numeric,
                                      .Namespace = 0,
                                      .Data.Numeric = OpcUaId_ServerConfiguration_ApplyChanges};
static SOPC_NodeId gCreateSigningRequestId = {.IdentifierType = SOPC_IdentifierType_Numeric,
                                              .Namespace = 0,
                                              .Data.Numeric = OpcUaId_ServerConfiguration_CreateSigningRequest};
static SOPC_NodeId gGetRejectedListId = {.IdentifierType = SOPC_IdentifierType_Numeric,
                                         .Namespace = 0,
                                         .Data.Numeric = OpcUaId_ServerConfiguration_GetRejectedList};
static SOPC_NodeId gAppGroupId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultApplicationGroup};
static SOPC_NodeId gUsrGroupId = {.IdentifierType = SOPC_IdentifierType_Numeric,
                                  .Namespace = 0,
                                  .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultUserTokenGroup};
/* Methods NodeId of the ServerConfigurationType*/
static SOPC_NodeId gTypeUpdateCertificateId = {.IdentifierType = SOPC_IdentifierType_Numeric,
                                               .Namespace = 0,
                                               .Data.Numeric = OpcUaId_ServerConfigurationType_UpdateCertificate};
static SOPC_NodeId gTypeApplyChangesId = {.IdentifierType = SOPC_IdentifierType_Numeric,
                                          .Namespace = 0,
                                          .Data.Numeric = OpcUaId_ServerConfigurationType_ApplyChanges};
static SOPC_NodeId gTypeCreateSigningRequestId = {.IdentifierType = SOPC_IdentifierType_Numeric,
                                                  .Namespace = 0,
                                                  .Data.Numeric = OpcUaId_ServerConfigurationType_CreateSigningRequest};
static SOPC_NodeId gTypeGetRejectedListId = {.IdentifierType = SOPC_IdentifierType_Numeric,
                                             .Namespace = 0,
                                             .Data.Numeric = OpcUaId_ServerConfigurationType_GetRejectedList};

static PushServerConfig_NodeIds gNodeIds = {
    .pServerConfigurationId = &gServerConfigurationId,
    .pApplyChangesId = &gApplyChangesId,
    .pCreateSigningRequestId = &gCreateSigningRequestId,
    .pGetRejectedListId = &gGetRejectedListId,
    .pUpdateCertificateId = &gUpdateCertificateId,
    .pAppGroupId = &gAppGroupId,
    .pUsrGroupId = &gUsrGroupId,
};
static PushServerConfig_NodeIds gTypeNodeIds = {
    .pServerConfigurationId = NULL,
    .pApplyChangesId = &gTypeApplyChangesId,
    .pCreateSigningRequestId = &gTypeCreateSigningRequestId,
    .pGetRejectedListId = &gTypeGetRejectedListId,
    .pUpdateCertificateId = &gTypeUpdateCertificateId,
    .pAppGroupId = NULL,
    .pUsrGroupId = NULL,
};

static PushServerContext gServerContext = {0};

/*---------------------------------------------------------------------------
 *                      Prototype of static functions
 *---------------------------------------------------------------------------*/

static void push_server_config_clear_context(PushServerContext* pContext);
static void push_server_config_initialize_context(PushServerContext* pContext);

static void push_server_config_delete_single_node_id(SOPC_NodeId** ppNodeId);
static void push_server_config_delete_node_ids(PushServerConfig_NodeIds** ppNodeIds, bool bDoNotClear);
static SOPC_ReturnStatus push_server_config_copy_meth_node_ids(PushServerConfig_NodeIds* pSrc,
                                                               PushServerConfig_NodeIds** ppDest);
static SOPC_ReturnStatus push_server_config_copy_node_ids(PushServerConfig_NodeIds* pSrc,
                                                          PushServerConfig_NodeIds** ppDest,
                                                          const bool bIncludeUser);

/*---------------------------------------------------------------------------
 *                       Static functions (implementation)
 *---------------------------------------------------------------------------*/

static void push_server_config_clear_context(PushServerContext* pContext)
{
    if (NULL == pContext)
    {
        return;
    }
    push_server_config_delete_single_node_id(&pContext->pServerConfigurationId);
    push_server_config_delete_single_node_id(&pContext->pAppGroupId);
    push_server_config_delete_single_node_id(&pContext->pUsrGroupId);
}

static void push_server_config_initialize_context(PushServerContext* pContext)
{
    if (NULL == pContext)
    {
        return;
    }
    pContext->bIsInit = false;
    pContext->bIsConfigure = false;
    pContext->pServerConfigurationId = NULL;
    pContext->pAppGroupId = NULL;
    pContext->pUsrGroupId = NULL;
}

static void push_server_config_delete_single_node_id(SOPC_NodeId** ppNodeId)
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

static void push_server_config_delete_node_ids(PushServerConfig_NodeIds** ppNodeIds, bool bDoNotClear)
{
    if (NULL == ppNodeIds)
    {
        return;
    }
    PushServerConfig_NodeIds* pNodeIds = *ppNodeIds;
    if (NULL == pNodeIds)
    {
        return;
    }
    if (!bDoNotClear)
    {
        /* Deleted by the MethodCall manager */
        push_server_config_delete_single_node_id(&pNodeIds->pApplyChangesId);
        push_server_config_delete_single_node_id(&pNodeIds->pCreateSigningRequestId);
        push_server_config_delete_single_node_id(&pNodeIds->pGetRejectedListId);
        push_server_config_delete_single_node_id(&pNodeIds->pUpdateCertificateId);
    }
    push_server_config_delete_single_node_id(&pNodeIds->pServerConfigurationId);
    push_server_config_delete_single_node_id(&pNodeIds->pAppGroupId);
    push_server_config_delete_single_node_id(&pNodeIds->pUsrGroupId);
    SOPC_Free(pNodeIds);
    pNodeIds = NULL;
    *ppNodeIds = pNodeIds;
}

static SOPC_ReturnStatus push_server_config_copy_meth_node_ids(PushServerConfig_NodeIds* pSrc,
                                                               PushServerConfig_NodeIds** ppDest)
{
    if (NULL == pSrc || NULL == ppDest)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (NULL == pSrc->pApplyChangesId || NULL == pSrc->pCreateSigningRequestId || NULL == pSrc->pGetRejectedListId ||
        NULL == pSrc->pUpdateCertificateId)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    *ppDest = NULL;
    PushServerConfig_NodeIds* pDest = NULL;
    pDest = SOPC_Calloc(1, sizeof(PushServerConfig_NodeIds));
    if (NULL == pDest)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }
    pDest->pApplyChangesId = SOPC_Calloc(1, sizeof(SOPC_NodeId));
    SOPC_ReturnStatus status = SOPC_NodeId_Copy(pDest->pApplyChangesId, pSrc->pApplyChangesId);
    if (SOPC_STATUS_OK == status)
    {
        pDest->pCreateSigningRequestId = SOPC_Calloc(1, sizeof(SOPC_NodeId));
        status = SOPC_NodeId_Copy(pDest->pCreateSigningRequestId, pSrc->pCreateSigningRequestId);
    }
    if (SOPC_STATUS_OK == status)
    {
        pDest->pGetRejectedListId = SOPC_Calloc(1, sizeof(SOPC_NodeId));
        status = SOPC_NodeId_Copy(pDest->pGetRejectedListId, pSrc->pGetRejectedListId);
    }
    if (SOPC_STATUS_OK == status)
    {
        pDest->pUpdateCertificateId = SOPC_Calloc(1, sizeof(SOPC_NodeId));
        status = SOPC_NodeId_Copy(pDest->pUpdateCertificateId, pSrc->pUpdateCertificateId);
    }
    if (SOPC_STATUS_OK != status)
    {
        push_server_config_delete_node_ids(&pDest, false);
    }
    *ppDest = pDest;
    return status;
}

static SOPC_ReturnStatus push_server_config_copy_node_ids(PushServerConfig_NodeIds* pSrc,
                                                          PushServerConfig_NodeIds** ppDest,
                                                          const bool bIncludeUser)
{
    if (NULL == pSrc || NULL == ppDest)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (NULL == pSrc->pServerConfigurationId)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    *ppDest = NULL;
    PushServerConfig_NodeIds* pDest = NULL;
    SOPC_ReturnStatus status = push_server_config_copy_meth_node_ids(pSrc, &pDest);
    if (SOPC_STATUS_OK != status)
    {
        return status;
    }

    pDest->pServerConfigurationId = SOPC_Calloc(1, sizeof(SOPC_NodeId));
    status = SOPC_NodeId_Copy(pDest->pServerConfigurationId, pSrc->pServerConfigurationId);
    if (SOPC_STATUS_OK == status)
    {
        pDest->pAppGroupId = SOPC_Calloc(1, sizeof(SOPC_NodeId));
        status = SOPC_NodeId_Copy(pDest->pAppGroupId, pSrc->pAppGroupId);
    }
    if (SOPC_STATUS_OK == status)
    {
        if (bIncludeUser)
        {
            pDest->pUsrGroupId = SOPC_Calloc(1, sizeof(SOPC_NodeId));
            status = SOPC_NodeId_Copy(pDest->pUsrGroupId, pSrc->pUsrGroupId);
        }
        else
        {
            pDest->pUsrGroupId = NULL;
        }
    }
    if (SOPC_STATUS_OK != status)
    {
        push_server_config_delete_node_ids(&pDest, false);
    }
    *ppDest = pDest;
    return status;
}

/*---------------------------------------------------------------------------
 *                             ITF Functions (implementation)
 *---------------------------------------------------------------------------*/

SOPC_ReturnStatus SOPC_PushServerConfig_Initialize(void)
{
    if (gServerContext.bIsInit)
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    push_server_config_initialize_context(&gServerContext);

    SOPC_ReturnStatus status = SOPC_CertificateGroup_Initialize();
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_TrustList_Initialize();
    }

    if (SOPC_STATUS_OK != status)
    {
        SOPC_PushServerConfig_Clear();
    }
    else
    {
        gServerContext.bIsInit = true;
    }

    return status;
}

SOPC_ReturnStatus SOPC_PushServerConfig_GetDefaultConfiguration(SOPC_PKIProvider* pPKIApp,
                                                                const SOPC_Certificate_Type appCertType,
                                                                SOPC_SerializedAsymmetricKey* pServerKey,
                                                                SOPC_SerializedCertificate* pServerCert,
                                                                SOPC_PKIProvider* pPKIUsr,
                                                                const SOPC_Certificate_Type usrCertType,
                                                                const uint32_t maxTrustListSize,
                                                                SOPC_PushServerConfig_Config** ppConfig)
{
    if (NULL == pPKIApp || 0 == maxTrustListSize || NULL == pServerKey || NULL == pServerCert || NULL == ppConfig)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (SOPC_CERT_TYPE_RSA_MIN_APPLICATION != appCertType && SOPC_CERT_TYPE_RSA_SHA256_APPLICATION != appCertType)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_CertificateGroup_Config* pUsrCertGroupCfg = NULL;
    SOPC_CertificateGroup_Config* pAppCertGroupCfg = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    *ppConfig = NULL;
    SOPC_PushServerConfig_Config* pCfg = SOPC_Calloc(1, sizeof(SOPC_PushServerConfig_Config));
    if (NULL == pCfg)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }
    if (SOPC_STATUS_OK == status)
    {
        status =
            SOPC_CertificateGroup_GetDefaultConfiguration(SOPC_TRUSTLIST_GROUP_APP, appCertType, pPKIApp,
                                                          maxTrustListSize, pServerKey, pServerCert, &pAppCertGroupCfg);
    }
    if (NULL != pPKIUsr && SOPC_STATUS_OK == status)
    {
        if (SOPC_CERT_TYPE_RSA_MIN_APPLICATION != usrCertType && SOPC_CERT_TYPE_RSA_SHA256_APPLICATION != usrCertType)
        {
            status = SOPC_STATUS_INVALID_PARAMETERS;
        }
    }
    if (NULL != pPKIUsr && SOPC_STATUS_OK == status)
    {
        status = SOPC_CertificateGroup_GetDefaultConfiguration(SOPC_TRUSTLIST_GROUP_USR, usrCertType, pPKIUsr,
                                                               maxTrustListSize, NULL, NULL, &pUsrCertGroupCfg);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = push_server_config_copy_node_ids(&gNodeIds, &pCfg->pIds, NULL != pPKIUsr);
    }
    if (SOPC_STATUS_OK == status)
    {
        pCfg->pAppCertGroupCfg = pAppCertGroupCfg;
        pCfg->pUsrCertGroupCfg = pUsrCertGroupCfg;
        pCfg->bDoNotClearIds = false;
    }
    else
    {
        SOPC_CertificateGroup_DeleteConfiguration(&pAppCertGroupCfg);
        SOPC_CertificateGroup_DeleteConfiguration(&pUsrCertGroupCfg);
        SOPC_Free(pCfg);
        pCfg = NULL;
    }
    *ppConfig = pCfg;
    return status;
}

void SOPC_PushServerConfig_DeleteConfiguration(SOPC_PushServerConfig_Config** ppConfig)
{
    if (NULL == ppConfig)
    {
        return;
    }
    SOPC_PushServerConfig_Config* pConfig = *ppConfig;
    if (NULL == pConfig)
    {
        return;
    }
    push_server_config_delete_node_ids(&pConfig->pIds, pConfig->bDoNotClearIds);
    SOPC_CertificateGroup_DeleteConfiguration(&pConfig->pAppCertGroupCfg);
    SOPC_CertificateGroup_DeleteConfiguration(&pConfig->pUsrCertGroupCfg);
    memset(pConfig, 0, sizeof(SOPC_PushServerConfig_Config));
    SOPC_Free(pConfig);
    *ppConfig = NULL;
}

SOPC_ReturnStatus SOPC_PushServerConfig_Configure(SOPC_PushServerConfig_Config* pCfg, SOPC_MethodCallManager* pMcm)
{
    /* The API is not initialized or already configured */
    if (!gServerContext.bIsInit || gServerContext.bIsConfigure)
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    if (NULL == pCfg || NULL == pMcm)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (NULL == pCfg->pIds)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    PushServerConfig_NodeIds* pIds = pCfg->pIds;
    if (NULL == pIds->pServerConfigurationId || NULL == pIds->pApplyChangesId ||
        NULL == pIds->pCreateSigningRequestId || NULL == pIds->pGetRejectedListId ||
        NULL == pIds->pUpdateCertificateId || NULL == pIds->pAppGroupId)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_ReturnStatus status = SOPC_CertificateGroup_Configure(pCfg->pAppCertGroupCfg, pMcm);
    if (SOPC_STATUS_OK != status)
    {
        return status;
    }
    if (NULL != pCfg->pUsrCertGroupCfg)
    {
        status = SOPC_CertificateGroup_Configure(pCfg->pUsrCertGroupCfg, pMcm);
    }
    if (SOPC_STATUS_OK == status)
    {
        gServerContext.pServerConfigurationId = SOPC_Calloc(1, sizeof(SOPC_NodeId));
        status = SOPC_NodeId_Copy(gServerContext.pServerConfigurationId, pIds->pServerConfigurationId);
    }
    if (SOPC_STATUS_OK == status)
    {
        gServerContext.pAppGroupId = SOPC_Calloc(1, sizeof(SOPC_NodeId));
        status = SOPC_NodeId_Copy(gServerContext.pAppGroupId, pIds->pAppGroupId);
    }
    if (SOPC_STATUS_OK == status && NULL != pIds->pUsrGroupId)
    {
        gServerContext.pUsrGroupId = SOPC_Calloc(1, sizeof(SOPC_NodeId));
        status = SOPC_NodeId_Copy(gServerContext.pUsrGroupId, pIds->pUsrGroupId);
    }
    PushServerConfig_NodeIds* pTypeIds = NULL;
    if (SOPC_STATUS_OK == status)
    {
        status = push_server_config_copy_meth_node_ids(&gTypeNodeIds, &pTypeIds);
        status = NULL == pTypeIds ? SOPC_STATUS_NOK : SOPC_STATUS_OK;
    }
    /* Add methods ... */
    if (SOPC_STATUS_OK == status)
    {
        /* Method nodeIds are clear by the methodCall manager */
        pCfg->bDoNotClearIds = true;
        SOPC_ReturnStatus statusMcm = SOPC_MethodCallManager_AddMethod(
            pMcm, pIds->pUpdateCertificateId, &PushSrvCfg_Method_UpdateCertificate, "UpdateCertificate", NULL);
        if (SOPC_STATUS_OK != statusMcm)
        {
            /* But if AddMethod failed, the manager do not clear the nodeId */
            push_server_config_delete_single_node_id(&pIds->pUpdateCertificateId);
            status = SOPC_STATUS_INVALID_STATE;
        }
        statusMcm = SOPC_MethodCallManager_AddMethod(
            pMcm, pTypeIds->pUpdateCertificateId, &PushSrvCfg_Method_UpdateCertificate, "TypeUpdateCertificate", NULL);
        if (SOPC_STATUS_OK != statusMcm)
        {
            /* But if AddMethod failed, the manager do not clear the nodeId */
            push_server_config_delete_single_node_id(&pTypeIds->pUpdateCertificateId);
            status = SOPC_STATUS_INVALID_STATE;
        }
        statusMcm = SOPC_MethodCallManager_AddMethod(pMcm, pIds->pApplyChangesId, &PushSrvCfg_Method_ApplyChanges,
                                                     "ApplyChanges", NULL);
        if (SOPC_STATUS_OK != statusMcm)
        {
            /* But if AddMethod failed, the manager do not clear the nodeId */
            push_server_config_delete_single_node_id(&pIds->pApplyChangesId);
            status = SOPC_STATUS_INVALID_STATE;
        }
        statusMcm = SOPC_MethodCallManager_AddMethod(pMcm, pTypeIds->pApplyChangesId, &PushSrvCfg_Method_ApplyChanges,
                                                     "TypeApplyChanges", NULL);
        if (SOPC_STATUS_OK != statusMcm)
        {
            /* But if AddMethod failed, the manager do not clear the nodeId */
            push_server_config_delete_single_node_id(&pTypeIds->pApplyChangesId);
            status = SOPC_STATUS_INVALID_STATE;
        }
        statusMcm = SOPC_MethodCallManager_AddMethod(
            pMcm, pIds->pCreateSigningRequestId, &PushSrvCfg_Method_CreateSigningRequest, "CreateSigningRequest", NULL);
        if (SOPC_STATUS_OK != statusMcm)
        {
            /* But if AddMethod failed, the manager do not clear the nodeId */
            push_server_config_delete_single_node_id(&pIds->pCreateSigningRequestId);
            status = SOPC_STATUS_INVALID_STATE;
        }
        statusMcm =
            SOPC_MethodCallManager_AddMethod(pMcm, pTypeIds->pCreateSigningRequestId,
                                             &PushSrvCfg_Method_CreateSigningRequest, "TypeCreateSigningRequest", NULL);
        if (SOPC_STATUS_OK != statusMcm)
        {
            /* But if AddMethod failed, the manager do not clear the nodeId */
            push_server_config_delete_single_node_id(&pTypeIds->pCreateSigningRequestId);
            status = SOPC_STATUS_INVALID_STATE;
        }
        statusMcm = SOPC_MethodCallManager_AddMethod(pMcm, pIds->pGetRejectedListId, &PushSrvCfg_Method_GetRejectedList,
                                                     "GetRejectedList", NULL);
        if (SOPC_STATUS_OK != statusMcm)
        {
            /* But if AddMethod failed, the manager do not clear the nodeId */
            push_server_config_delete_single_node_id(&pIds->pGetRejectedListId);
            status = SOPC_STATUS_INVALID_STATE;
        }
        statusMcm = SOPC_MethodCallManager_AddMethod(pMcm, pTypeIds->pGetRejectedListId,
                                                     &PushSrvCfg_Method_GetRejectedList, "TypeGetRejectedList", NULL);
        if (SOPC_STATUS_OK != statusMcm)
        {
            /* But if AddMethod failed, the manager do not clear the nodeId */
            push_server_config_delete_single_node_id(&pTypeIds->pGetRejectedListId);
            status = SOPC_STATUS_INVALID_STATE;
        }
    }

    SOPC_Free(pTypeIds);

    if (SOPC_STATUS_OK != status)
    {
        SOPC_PushServerConfig_Clear();
    }
    else
    {
        gServerContext.bIsConfigure = true;
    }
    return status;
}

void SOPC_PushServerConfig_Clear(void)
{
    SOPC_CertificateGroup_Clear();
    SOPC_TrustList_Clear();
    push_server_config_clear_context(&gServerContext);
    push_server_config_initialize_context(&gServerContext);
}

SOPC_StatusCode PushServer_GetRejectedList(SOPC_ByteString** ppBsCertArray, uint32_t* pLengthArray)
{
    SOPC_ASSERT(gServerContext.bIsInit && gServerContext.bIsConfigure);
    SOPC_ASSERT(NULL != gServerContext.pAppGroupId); // Application group is mandatory

    if (NULL == ppBsCertArray || NULL == pLengthArray)
    {
        return OpcUa_BadUnexpectedError;
    }

    *ppBsCertArray = NULL;
    *pLengthArray = 0;

    SOPC_ByteString* pBsAppCertArray = NULL;
    SOPC_ByteString* pBsUsrCertArray = NULL;
    SOPC_ByteString* pCertArray = NULL;
    uint32_t lenApp = 0;
    uint32_t lenUsr = 0;
    uint32_t idx = 0;
    SOPC_CertGroupContext* pGroupCtx = NULL;
    bool bFound = false;
    SOPC_StatusCode stCode = SOPC_GoodGenericStatus;
    /* Retrieve the application group */
    pGroupCtx = CertificateGroup_DictGet(gServerContext.pAppGroupId, &bFound);
    if (pGroupCtx == NULL || !bFound)
    {
        return OpcUa_BadUnexpectedError;
    }
    stCode = CertificateGroup_GetRejectedList(pGroupCtx, &pBsAppCertArray, &lenApp);
    if (!SOPC_IsGoodStatus(stCode))
    {
        return OpcUa_BadUnexpectedError;
    }
    /* Retrieve the user group */
    if (NULL != gServerContext.pUsrGroupId && SOPC_IsGoodStatus(stCode))
    {
        pGroupCtx = CertificateGroup_DictGet(gServerContext.pUsrGroupId, &bFound);
        if (pGroupCtx == NULL || !bFound)
        {
            stCode = OpcUa_BadUnexpectedError;
        }
        if (SOPC_IsGoodStatus(stCode))
        {
            stCode = CertificateGroup_GetRejectedList(pGroupCtx, &pBsUsrCertArray, &lenUsr);
        }
    }
    /* Create the output array */
    if (SOPC_IsGoodStatus(stCode))
    {
        pCertArray = SOPC_Calloc((size_t)(lenApp + lenUsr), sizeof(SOPC_ByteString));
        if (NULL == pCertArray)
        {
            stCode = OpcUa_BadOutOfMemory;
        }
    }

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    for (idx = 0; idx < lenApp && NULL != pBsAppCertArray && SOPC_IsGoodStatus(stCode); idx++)
    {
        status = SOPC_ByteString_Copy(&pCertArray[idx], &pBsAppCertArray[idx]);
        stCode = SOPC_STATUS_OK == status ? SOPC_GoodGenericStatus : OpcUa_BadUnexpectedError;
    }
    for (idx = lenUsr; idx < lenApp + lenUsr && NULL != pBsUsrCertArray && SOPC_IsGoodStatus(stCode); idx++)
    {
        status = SOPC_ByteString_Copy(&pCertArray[idx], &pBsUsrCertArray[idx - lenUsr]);
        stCode = SOPC_STATUS_OK == status ? SOPC_GoodGenericStatus : OpcUa_BadUnexpectedError;
    }
    /* Clear */
    if (NULL != pBsAppCertArray)
    {
        for (idx = 0; idx < lenApp; idx++)
        {
            SOPC_ByteString_Clear(&pBsAppCertArray[idx]);
        }
        SOPC_Free(pBsAppCertArray);
    }
    if (NULL != pBsUsrCertArray)
    {
        for (idx = 0; idx < lenApp; idx++)
        {
            SOPC_ByteString_Clear(&pBsUsrCertArray[idx]);
        }
        SOPC_Free(pBsUsrCertArray);
    }
    pBsAppCertArray = NULL;
    pBsUsrCertArray = NULL;
    if (!SOPC_IsGoodStatus(stCode) && NULL != pCertArray)
    {
        for (idx = 0; idx < lenApp + lenUsr; idx++)
        {
            SOPC_ByteString_Clear(&pCertArray[idx]);
        }
        SOPC_Free(pCertArray);
        pCertArray = NULL;
        lenApp = 0;
        lenUsr = 0;
    }

    *ppBsCertArray = pCertArray;
    *pLengthArray = lenApp + lenUsr;
    return stCode;
}

SOPC_StatusCode PushServer_ExportRejectedList(const bool bEraseExisting)
{
    SOPC_ASSERT(gServerContext.bIsInit && gServerContext.bIsConfigure);
    SOPC_ASSERT(NULL != gServerContext.pAppGroupId); // Application group is mandatory

    SOPC_StatusCode stCode = SOPC_GoodGenericStatus;
    SOPC_CertGroupContext* pGroupCtx = NULL;
    bool bFound = false;
    /* Retrieve the application group */
    pGroupCtx = CertificateGroup_DictGet(gServerContext.pAppGroupId, &bFound);
    if (pGroupCtx == NULL || !bFound)
    {
        return OpcUa_BadUnexpectedError;
    }
    stCode = CertificateGroup_ExportRejectedList(pGroupCtx, bEraseExisting);
    /* Retrieve the user group */
    if (NULL != gServerContext.pUsrGroupId && SOPC_IsGoodStatus(stCode))
    {
        pGroupCtx = CertificateGroup_DictGet(gServerContext.pUsrGroupId, &bFound);
        if (pGroupCtx == NULL || !bFound)
        {
            stCode = OpcUa_BadUnexpectedError;
        }
        if (SOPC_IsGoodStatus(stCode))
        {
            stCode = CertificateGroup_ExportRejectedList(pGroupCtx, bEraseExisting);
        }
    }
    return stCode;
}
