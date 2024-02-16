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

#include "sopc_push_server_config.h"
#include "sopc_push_server_config_itf.h"
#include "sopc_push_server_config_meth.h"

#include "opcua_identifiers.h"
#include "opcua_statuscodes.h"

/*---------------------------------------------------------------------------
 *                             Global variables
 *---------------------------------------------------------------------------*/

/* NodeIds of the ServerConfiguration instance  */
static const SOPC_NodeId gServerConfigurationId = {.IdentifierType = SOPC_IdentifierType_Numeric,
                                                   .Namespace = 0,
                                                   .Data.Numeric = OpcUaId_ServerConfiguration};
static const SOPC_NodeId gUpdateCertificateId = {.IdentifierType = SOPC_IdentifierType_Numeric,
                                                 .Namespace = 0,
                                                 .Data.Numeric = OpcUaId_ServerConfiguration_UpdateCertificate};
static const SOPC_NodeId gApplyChangesId = {.IdentifierType = SOPC_IdentifierType_Numeric,
                                            .Namespace = 0,
                                            .Data.Numeric = OpcUaId_ServerConfiguration_ApplyChanges};
static const SOPC_NodeId gCreateSigningRequestId = {.IdentifierType = SOPC_IdentifierType_Numeric,
                                                    .Namespace = 0,
                                                    .Data.Numeric = OpcUaId_ServerConfiguration_CreateSigningRequest};
static const SOPC_NodeId gGetRejectedListId = {.IdentifierType = SOPC_IdentifierType_Numeric,
                                               .Namespace = 0,
                                               .Data.Numeric = OpcUaId_ServerConfiguration_GetRejectedList};
static const SOPC_NodeId gAppGroupId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultApplicationGroup};
static const SOPC_NodeId gUsrGroupId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultUserTokenGroup};
/* Methods NodeId of the ServerConfigurationType*/
static const SOPC_NodeId gTypeUpdateCertificateId = {.IdentifierType = SOPC_IdentifierType_Numeric,
                                                     .Namespace = 0,
                                                     .Data.Numeric = OpcUaId_ServerConfigurationType_UpdateCertificate};
static const SOPC_NodeId gTypeApplyChangesId = {.IdentifierType = SOPC_IdentifierType_Numeric,
                                                .Namespace = 0,
                                                .Data.Numeric = OpcUaId_ServerConfigurationType_ApplyChanges};
static const SOPC_NodeId gTypeCreateSigningRequestId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfigurationType_CreateSigningRequest};
static const SOPC_NodeId gTypeGetRejectedListId = {.IdentifierType = SOPC_IdentifierType_Numeric,
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

static const PushServerConfig_MethodFunc_Ptr gMethodFunc_Ptr = {
    .UpdateCertificate = &PushSrvCfg_Method_UpdateCertificate,
    .ApplyChanges = &PushSrvCfg_Method_ApplyChanges,
    .CreateSigningRequest = &PushSrvCfg_Method_CreateSigningRequest,
    .GetRejectedList = &PushSrvCfg_Method_GetRejectedList,
};
static const PushServerConfig_MethodFunc_Ptr gTOFUMethodFunc_Ptr = {
    .UpdateCertificate = &PushSrvCfg_Method_TOFUNotSuported,
    .ApplyChanges = &PushSrvCfg_Method_ApplyChanges,
    .CreateSigningRequest = &PushSrvCfg_Method_TOFUNotSuported,
    .GetRejectedList = &PushSrvCfg_Method_GetRejectedList,
};

static PushServerContext gServerContext = {0};

/*---------------------------------------------------------------------------
 *                      Prototype of static functions
 *---------------------------------------------------------------------------*/

static void push_server_config_initialize_context(PushServerContext* pContext);

/*---------------------------------------------------------------------------
 *                       Static functions (implementation)
 *---------------------------------------------------------------------------*/

static void push_server_config_initialize_context(PushServerContext* pContext)
{
    if (NULL == pContext)
    {
        return;
    }
    pContext->bIsInit = false;
    pContext->bIsConfigured = false;
    pContext->pServerConfigurationId = NULL;
    pContext->pAppGroupId = NULL;
    pContext->pUsrGroupId = NULL;
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
                                                                SOPC_KeyCertPair* pServerKeyCertPair,
                                                                const char* pServerKeyPath,
                                                                const char* pServerCertPath,
                                                                SOPC_PKIProvider* pPKIUsr,
                                                                const uint32_t maxTrustListSize,
                                                                SOPC_PushServerConfig_Config** ppConfig)
{
    if (NULL == pPKIApp || 0 == maxTrustListSize || NULL == pServerKeyCertPair || NULL == ppConfig)
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
        status = SOPC_CertificateGroup_GetDefaultConfiguration(SOPC_TRUSTLIST_GROUP_APP, appCertType, pPKIApp,
                                                               maxTrustListSize, pServerKeyCertPair, pServerKeyPath,
                                                               pServerCertPath, &pAppCertGroupCfg);
    }
    if (NULL != pPKIUsr && SOPC_STATUS_OK == status)
    {
        status = SOPC_CertificateGroup_GetDefaultConfiguration(SOPC_TRUSTLIST_GROUP_USR, SOPC_CERT_TYPE_UNKNOW, pPKIUsr,
                                                               maxTrustListSize, NULL, NULL, NULL, &pUsrCertGroupCfg);
    }
    if (SOPC_STATUS_OK == status)
    {
        pCfg->pIds = &gNodeIds;
        if (NULL == pPKIUsr)
        {
            pCfg->pIds->pUsrGroupId = NULL;
        }
        pCfg->pAppCertGroupCfg = pAppCertGroupCfg;
        pCfg->pUsrCertGroupCfg = pUsrCertGroupCfg;
        pCfg->bIsTOFUSate = false;
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

SOPC_ReturnStatus SOPC_PushServerConfig_GetTOFUConfiguration(SOPC_PKIProvider* pPKIApp,
                                                             const SOPC_Certificate_Type appCertType,
                                                             const uint32_t maxTrustListSize,
                                                             SOPC_TrustList_UpdateCompleted_Fct* pFnUpdateCompleted,
                                                             SOPC_PushServerConfig_Config** ppConfig)
{
    if (NULL == pPKIApp || 0 == maxTrustListSize || NULL == pFnUpdateCompleted || NULL == ppConfig)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (SOPC_CERT_TYPE_RSA_MIN_APPLICATION != appCertType && SOPC_CERT_TYPE_RSA_SHA256_APPLICATION != appCertType)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_CertificateGroup_Config* pAppCertGroupCfg = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    *ppConfig = NULL;
    SOPC_PushServerConfig_Config* pCfg = SOPC_Calloc(1, sizeof(SOPC_PushServerConfig_Config));
    if (NULL == pCfg)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }
    status = SOPC_CertificateGroup_GetTOFUConfiguration(appCertType, pPKIApp, maxTrustListSize, pFnUpdateCompleted,
                                                        &pAppCertGroupCfg);

    if (SOPC_STATUS_OK == status)
    {
        pCfg->pIds = &gNodeIds;
        pCfg->pIds->pUsrGroupId = NULL;
        pCfg->pAppCertGroupCfg = pAppCertGroupCfg;
        pCfg->pUsrCertGroupCfg = NULL;
        pCfg->bIsTOFUSate = true;
    }
    else
    {
        SOPC_CertificateGroup_DeleteConfiguration(&pAppCertGroupCfg);
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
    SOPC_CertificateGroup_DeleteConfiguration(&pConfig->pAppCertGroupCfg);
    SOPC_CertificateGroup_DeleteConfiguration(&pConfig->pUsrCertGroupCfg);
    memset(pConfig, 0, sizeof(SOPC_PushServerConfig_Config));
    SOPC_Free(pConfig);
    *ppConfig = NULL;
}

SOPC_ReturnStatus SOPC_PushServerConfig_Configure(SOPC_PushServerConfig_Config* pCfg, SOPC_MethodCallManager* pMcm)
{
    /* The API is not initialized or already configured */
    if (!gServerContext.bIsInit || gServerContext.bIsConfigured)
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
    PushServerConfig_NodeIds* pTypeIds = &gTypeNodeIds;
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
        gServerContext.pServerConfigurationId = pIds->pServerConfigurationId;
        gServerContext.pAppGroupId = pIds->pAppGroupId;
        if (NULL != pIds->pUsrGroupId)
        {
            gServerContext.pUsrGroupId = pIds->pUsrGroupId;
        }
    }
    const PushServerConfig_MethodFunc_Ptr* pMethodFunc = NULL;
    if (pCfg->bIsTOFUSate)
    {
        pMethodFunc = &gTOFUMethodFunc_Ptr;
    }
    else
    {
        pMethodFunc = &gMethodFunc_Ptr;
    }
    /* Add methods ... */
    if (SOPC_STATUS_OK == status)
    {
        status =
            SOPC_MethodCallManager_AddMethodWithType(pMcm, pIds->pUpdateCertificateId, pTypeIds->pUpdateCertificateId,
                                                     pMethodFunc->UpdateCertificate, "UpdateCertificate", NULL);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_MethodCallManager_AddMethodWithType(pMcm, pIds->pApplyChangesId, pTypeIds->pApplyChangesId,
                                                          pMethodFunc->ApplyChanges, "ApplyChanges", NULL);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_MethodCallManager_AddMethodWithType(
            pMcm, pIds->pCreateSigningRequestId, pTypeIds->pCreateSigningRequestId, pMethodFunc->CreateSigningRequest,
            "CreateSigningRequest", NULL);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_MethodCallManager_AddMethodWithType(pMcm, pIds->pGetRejectedListId, pTypeIds->pGetRejectedListId,
                                                          pMethodFunc->GetRejectedList, "GetRejectedList", NULL);
    }
    if (SOPC_STATUS_OK != status)
    {
        SOPC_PushServerConfig_Clear();
    }
    else
    {
        gServerContext.bIsConfigured = true;
    }
    return status;
}

void SOPC_PushServerConfig_Clear(void)
{
    SOPC_CertificateGroup_Clear();
    SOPC_TrustList_Clear();
    push_server_config_initialize_context(&gServerContext);
}

SOPC_StatusCode PushServer_GetRejectedList(SOPC_ByteString** ppBsCertArray, uint32_t* pLengthArray)
{
    if (!gServerContext.bIsInit || !gServerContext.bIsConfigured)
    {
        return OpcUa_BadUnexpectedError;
    }
    if (NULL == gServerContext.pAppGroupId)
    {
        // Application group is mandatory
        return OpcUa_BadUnexpectedError;
    }

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
    pGroupCtx = CertificateGroup_GetFromNodeId(gServerContext.pAppGroupId, &bFound);
    if (pGroupCtx == NULL || !bFound)
    {
        return OpcUa_BadUnexpectedError;
    }
    stCode = CertificateGroup_GetRejectedList(pGroupCtx, &pBsAppCertArray, &lenApp);

    /* Retrieve the user group */
    if (NULL != gServerContext.pUsrGroupId && SOPC_IsGoodStatus(stCode))
    {
        pGroupCtx = CertificateGroup_GetFromNodeId(gServerContext.pUsrGroupId, &bFound);
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
    for (idx = 0; idx < lenApp && NULL != pBsAppCertArray && NULL != pCertArray && SOPC_IsGoodStatus(stCode); idx++)
    {
        status = SOPC_ByteString_Copy(&pCertArray[idx], &pBsAppCertArray[idx]);
        stCode = SOPC_STATUS_OK == status ? SOPC_GoodGenericStatus : OpcUa_BadUnexpectedError;
    }
    uint32_t idxStartUsr = lenApp == 0 ? lenApp : lenUsr;
    for (idx = idxStartUsr;
         idx < lenApp + lenUsr && NULL != pBsUsrCertArray && NULL != pCertArray && SOPC_IsGoodStatus(stCode); idx++)
    {
        status = SOPC_ByteString_Copy(&pCertArray[idx], &pBsUsrCertArray[idx - idxStartUsr]);
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

SOPC_StatusCode PushServer_ExportRejectedList(void)
{
    if (!gServerContext.bIsInit || !gServerContext.bIsConfigured)
    {
        return OpcUa_BadUnexpectedError;
    }
    if (NULL == gServerContext.pAppGroupId)
    {
        // Application group is mandatory
        return OpcUa_BadUnexpectedError;
    }

    SOPC_StatusCode stCode = SOPC_GoodGenericStatus;
    SOPC_CertGroupContext* pGroupCtx = NULL;
    bool bFound = false;
    /* Retrieve the application group */
    pGroupCtx = CertificateGroup_GetFromNodeId(gServerContext.pAppGroupId, &bFound);
    if (pGroupCtx == NULL || !bFound)
    {
        return OpcUa_BadUnexpectedError;
    }
    stCode = CertificateGroup_ExportRejectedList(pGroupCtx);
    /* Retrieve the user group */
    if (NULL != gServerContext.pUsrGroupId && SOPC_IsGoodStatus(stCode))
    {
        pGroupCtx = CertificateGroup_GetFromNodeId(gServerContext.pUsrGroupId, &bFound);
        if (pGroupCtx == NULL || !bFound)
        {
            stCode = OpcUa_BadUnexpectedError;
        }
        if (SOPC_IsGoodStatus(stCode))
        {
            stCode = CertificateGroup_ExportRejectedList(pGroupCtx);
        }
    }
    return stCode;
}
