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
#include "sopc_mem_alloc.h"

#include "sopc_push_server_config_itf.h"
#include "sopc_push_server_config_meth_internal.h"

/*---------------------------------------------------------------------------
 *                             Constants
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
 *                             Internal types
 *---------------------------------------------------------------------------*/

typedef struct PushServerConfig_Vars
{
    SOPC_NodeId* pServerCapabilitiesId;
    SOPC_NodeId* pSupportedPrivateKeyFormatsId;
    SOPC_NodeId* pMaxTrustListSizeId;
    SOPC_NodeId* pMulticastDnsEnabledId;

} PushServerConfig_Vars;

typedef struct PushServerConfig
{
    SOPC_NodeId* pId;
    PushServerConfig_Vars varIds;

} PushServerConfig;

/*---------------------------------------------------------------------------
 *                             Global variables
 *---------------------------------------------------------------------------*/

SOPC_PushServerConfig_Config gServerConfig_DefaultAddSpace = {
    .serverConfigurationNodeId = "ns=0;i=12637",
    .metUpdateCertificateNodeId = "ns=0;i=13737",
    .metApplyChangesNodeId = "ns=0;i=12740",
    .metCreateSigningRequestNodeId = "ns=0;i=12737",
    .metGetRejectedListNodeId = "ns=0;i=12777",
    .varServerCapabilitiesNodeId = "ns=0;i=12710",
    .varSupportedPrivateKeyFormatsNodeId = "ns=0;i=12639",
    .varMaxTrustListSizeNodeId = "ns=0;i=12640",
    .varMulticastDnsEnabledNodeId = "ns=0;i=12641",
    .pAppCertGroupCfg = NULL,
    .pUsrCertGroupCfg = NULL,
    .appCertType = SOPC_CERT_TYPE_UNKNOW,
    .usrCertType = SOPC_CERT_TYPE_UNKNOW,
};

PushServerConfig* gpServerConfig = NULL;

/*---------------------------------------------------------------------------
 *                      Prototype of static functions
 *---------------------------------------------------------------------------*/

static SOPC_ReturnStatus push_server_config_add_method(SOPC_MethodCallManager* pMcm,
                                                       const char* pCStringNodeId,
                                                       SOPC_MethodCallFunc_Ptr* pTrustListMet,
                                                       char* name);

/*---------------------------------------------------------------------------
 *                       Static functions (implementation)
 *---------------------------------------------------------------------------*/

static SOPC_ReturnStatus push_server_config_add_method(SOPC_MethodCallManager* pMcm,
                                                       const char* pCStringNodeId,
                                                       SOPC_MethodCallFunc_Ptr* pPushSvrCfgMet,
                                                       char* name)
{
    SOPC_ASSERT(NULL != pMcm);
    SOPC_ASSERT(NULL != pCStringNodeId);
    SOPC_ASSERT(NULL != pPushSvrCfgMet);
    SOPC_ASSERT(NULL != name);

    SOPC_NodeId* pNodeId = NULL;
    pNodeId = SOPC_NodeId_FromCString(pCStringNodeId, (int32_t) strlen(pCStringNodeId));
    SOPC_ReturnStatus status = NULL == pNodeId ? SOPC_STATUS_OUT_OF_MEMORY : SOPC_STATUS_OK;
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_MethodCallManager_AddMethod(pMcm, pNodeId, pPushSvrCfgMet, name, NULL);
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

SOPC_ReturnStatus SOPC_PushServerConfig_Initialize(void)
{
    if (NULL != gpServerConfig)
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    gpServerConfig = SOPC_Calloc(1, sizeof(PushServerConfig));
    if (NULL == gpServerConfig)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }
    gpServerConfig->pId = NULL;
    gpServerConfig->varIds.pMaxTrustListSizeId = NULL;
    gpServerConfig->varIds.pMulticastDnsEnabledId = NULL;
    gpServerConfig->varIds.pServerCapabilitiesId = NULL;
    gpServerConfig->varIds.pSupportedPrivateKeyFormatsId = NULL;

    SOPC_ReturnStatus status = SOPC_CertificateGroup_Initialize();
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_TrustList_Initialize();
    }

    if (SOPC_STATUS_OK != status)
    {
        SOPC_PushServerConfig_Clear();
    }

    return status;
}

SOPC_ReturnStatus SOPC_PushServerConfig_GetDefaultConfiguration(SOPC_PKIProvider* pPKIApp,
                                                                const SOPC_Certificate_Type appCertType,
                                                                SOPC_PKIProvider* pPKIUsers,
                                                                const SOPC_Certificate_Type usrCertType,
                                                                SOPC_PushServerConfig_Config* pCfg)
{
    if (NULL == pPKIApp || NULL == pCfg)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (SOPC_CERT_TYPE_RSA_MIN_APPLICATION != appCertType && SOPC_CERT_TYPE_RSA_SHA256_APPLICATION != appCertType)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    const SOPC_CertificateGroup_Config* pUsrCertGroupCfg = NULL;
    const SOPC_CertificateGroup_Config* pAppCertGroupCfg =
        SOPC_CertificateGroup_GetDefaultConfiguration(SOPC_TRUSTLIST_GROUP_APP, appCertType, pPKIApp);
    if (NULL == pAppCertGroupCfg)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (NULL != pPKIUsers)
    {
        if (SOPC_CERT_TYPE_RSA_MIN_APPLICATION != usrCertType && SOPC_CERT_TYPE_RSA_SHA256_APPLICATION != usrCertType)
        {
            return SOPC_STATUS_INVALID_PARAMETERS;
        }
        pUsrCertGroupCfg =
            SOPC_CertificateGroup_GetDefaultConfiguration(SOPC_TRUSTLIST_GROUP_USR, usrCertType, pPKIUsers);
        if (NULL == pUsrCertGroupCfg)
        {
            return SOPC_STATUS_INVALID_PARAMETERS;
        }
    }
    *pCfg = gServerConfig_DefaultAddSpace;
    pCfg->pAppCertGroupCfg = (const SOPC_CertificateGroup_Config*) pAppCertGroupCfg;
    pCfg->pUsrCertGroupCfg = (const SOPC_CertificateGroup_Config*) pUsrCertGroupCfg;
    pCfg->appCertType = (const SOPC_Certificate_Type) appCertType;
    pCfg->usrCertType = usrCertType;
    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_PushServerConfig_Configure(const SOPC_PushServerConfig_Config* pCfg,
                                                  SOPC_SerializedAsymmetricKey* pServerKey,
                                                  SOPC_SerializedCertificate* pServerCert,
                                                  SOPC_MethodCallManager* pMcm)
{
    /* The API is not initialized. */
    if (NULL == gpServerConfig)
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    if (NULL == pCfg || NULL == pServerKey || NULL == pServerCert || NULL == pMcm)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (NULL == pCfg->metApplyChangesNodeId || NULL == pCfg->metCreateSigningRequestNodeId ||
        NULL == pCfg->metGetRejectedListNodeId || NULL == pCfg->metUpdateCertificateNodeId ||
        NULL == pCfg->pAppCertGroupCfg || NULL == pCfg->serverConfigurationNodeId ||
        NULL == pCfg->varMaxTrustListSizeNodeId || NULL == pCfg->varMulticastDnsEnabledNodeId ||
        NULL == pCfg->varServerCapabilitiesNodeId || NULL == pCfg->varSupportedPrivateKeyFormatsNodeId)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus status = SOPC_CertificateGroup_Configure(pCfg->pAppCertGroupCfg, pServerKey, pServerCert, pMcm);
    if (SOPC_STATUS_OK != status)
    {
        return status;
    }
    if (NULL != pCfg->pUsrCertGroupCfg)
    {
        status = SOPC_CertificateGroup_Configure(pCfg->pUsrCertGroupCfg, NULL, NULL, pMcm);
    }
    if (SOPC_STATUS_OK == status)
    {
        gpServerConfig->pId =
            SOPC_NodeId_FromCString(pCfg->serverConfigurationNodeId, (int32_t) strlen(pCfg->serverConfigurationNodeId));
        status = NULL == gpServerConfig->pId ? SOPC_STATUS_OUT_OF_MEMORY : SOPC_STATUS_OK;
    }
    /* Add methods ... */
    if (SOPC_STATUS_OK == status)
    {
        status = push_server_config_add_method(pMcm, pCfg->metUpdateCertificateNodeId,
                                               &PushSrvCfg_Method_UpdateCertificate, "UpdateCertificate");
    }
    if (SOPC_STATUS_OK == status)
    {
        status = push_server_config_add_method(pMcm, pCfg->metApplyChangesNodeId, &PushSrvCfg_Method_ApplyChanges,
                                               "ApplyChanges");
    }
    if (SOPC_STATUS_OK == status)
    {
        status = push_server_config_add_method(pMcm, pCfg->metCreateSigningRequestNodeId,
                                               &PushSrvCfg_Method_CreateSigningRequest, "CreateSigningRequest");
    }
    if (SOPC_STATUS_OK == status)
    {
        status = push_server_config_add_method(pMcm, pCfg->metGetRejectedListNodeId, &PushSrvCfg_Method_GetRejectedList,
                                               "GetRejectedList");
    }
    /* Add variables ... */
    if (SOPC_STATUS_OK == status)
    {
        gpServerConfig->varIds.pMaxTrustListSizeId =
            SOPC_NodeId_FromCString(pCfg->varMaxTrustListSizeNodeId, (int32_t) strlen(pCfg->varMaxTrustListSizeNodeId));
        status = NULL == gpServerConfig->varIds.pMaxTrustListSizeId ? SOPC_STATUS_OUT_OF_MEMORY : SOPC_STATUS_OK;
    }
    if (SOPC_STATUS_OK == status)
    {
        gpServerConfig->varIds.pMulticastDnsEnabledId = SOPC_NodeId_FromCString(
            pCfg->varMulticastDnsEnabledNodeId, (int32_t) strlen(pCfg->varMulticastDnsEnabledNodeId));
        status = NULL == gpServerConfig->varIds.pMulticastDnsEnabledId ? SOPC_STATUS_OUT_OF_MEMORY : SOPC_STATUS_OK;
    }
    if (SOPC_STATUS_OK == status)
    {
        gpServerConfig->varIds.pServerCapabilitiesId = SOPC_NodeId_FromCString(
            pCfg->varServerCapabilitiesNodeId, (int32_t) strlen(pCfg->varServerCapabilitiesNodeId));
        status = NULL == gpServerConfig->varIds.pServerCapabilitiesId ? SOPC_STATUS_OUT_OF_MEMORY : SOPC_STATUS_OK;
    }
    if (SOPC_STATUS_OK == status)
    {
        gpServerConfig->varIds.pSupportedPrivateKeyFormatsId = SOPC_NodeId_FromCString(
            pCfg->varSupportedPrivateKeyFormatsNodeId, (int32_t) strlen(pCfg->varSupportedPrivateKeyFormatsNodeId));
        status =
            NULL == gpServerConfig->varIds.pSupportedPrivateKeyFormatsId ? SOPC_STATUS_OUT_OF_MEMORY : SOPC_STATUS_OK;
    }

    if (SOPC_STATUS_OK != status)
    {
        SOPC_PushServerConfig_Clear();
    }
    return status;
}

void SOPC_PushServerConfig_Clear(void)
{
    SOPC_CertificateGroup_Clear();
    SOPC_TrustList_Clear();
    SOPC_NodeId_Clear(gpServerConfig->pId);
    SOPC_NodeId_Clear(gpServerConfig->varIds.pMaxTrustListSizeId);
    SOPC_NodeId_Clear(gpServerConfig->varIds.pMulticastDnsEnabledId);
    SOPC_NodeId_Clear(gpServerConfig->varIds.pServerCapabilitiesId);
    SOPC_NodeId_Clear(gpServerConfig->varIds.pSupportedPrivateKeyFormatsId);
    SOPC_Free(gpServerConfig->pId);
    SOPC_Free(gpServerConfig->varIds.pMaxTrustListSizeId);
    SOPC_Free(gpServerConfig->varIds.pMulticastDnsEnabledId);
    SOPC_Free(gpServerConfig->varIds.pServerCapabilitiesId);
    SOPC_Free(gpServerConfig->varIds.pSupportedPrivateKeyFormatsId);
    SOPC_Free(gpServerConfig);
}
