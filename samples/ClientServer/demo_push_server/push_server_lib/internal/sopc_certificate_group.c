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
 * \brief Internal API implementation to manage methods, properties and variables of the CertificateGroupType according
 * the Push model.
 */

#include <stdio.h>
#include <string.h>

#include "sopc_assert.h"
#include "sopc_mem_alloc.h"

#include "opcua_identifiers.h"
#include "opcua_statuscodes.h"
#include "sopc_certificate_group.h"
#include "sopc_helper_string.h"
#include "sopc_helper_uri.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_pki_struct_lib_internal.h"
#include "sopc_toolkit_async_api.h"
#include "sopc_toolkit_config_internal.h"

/*---------------------------------------------------------------------------
 *                             Global variables
 *---------------------------------------------------------------------------*/

static SOPC_Dict* gObjIdToCertGroup = NULL;
static int32_t gTombstoneKey = -1;

/* NodeIds of the CertificateGroup instance of the DefaultApplicationGroup */
static const SOPC_NodeId appCertificateGroupId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultApplicationGroup};
static const SOPC_NodeId appCertificateTypesId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultApplicationGroup_CertificateTypes};
static const SOPC_NodeId appTrustListId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultApplicationGroup_TrustList};

static const CertificateGroup_NodeIds appNodeIds = {
    .pCertificateGroupId = &appCertificateGroupId,
    .pCertificateTypesId = &appCertificateTypesId,
    .pTrustListId = &appTrustListId,
};

/* NodeIds of the CertificateGroup instance of the DefaultUserTokenGroup */
static const SOPC_NodeId usrCertificateGroupId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultUserTokenGroup};
static const SOPC_NodeId usrCertificateTypesId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultUserTokenGroup_CertificateTypes};
static const SOPC_NodeId usrTrustListId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultUserTokenGroup_TrustList};

static const CertificateGroup_NodeIds usrNodeIds = {
    .pCertificateGroupId = &usrCertificateGroupId,
    .pCertificateTypesId = &usrCertificateTypesId,
    .pTrustListId = &usrTrustListId,
};

/* NodeIds for CertificateTypeValue */
static const SOPC_NodeId certTypeRsaSha256Id = {.IdentifierType = SOPC_IdentifierType_Numeric,
                                                .Namespace = 0,
                                                .Data.Numeric = OpcUaId_RsaSha256ApplicationCertificateType};
static const SOPC_NodeId certTypeRsaMinId = {.IdentifierType = SOPC_IdentifierType_Numeric,
                                             .Namespace = 0,
                                             .Data.Numeric = OpcUaId_RsaMinApplicationCertificateType};

/*---------------------------------------------------------------------------
 *                      Prototype of static functions
 *---------------------------------------------------------------------------*/

static SOPC_ReturnStatus cert_group_create_context(SOPC_CertGroupContext** ppCertGroup);
static void cert_group_clear_context(SOPC_CertGroupContext* pCertGroup);
static void cert_group_delete_context(SOPC_CertGroupContext** ppCertGroup);
static void cert_group_dict_free_context_value(uintptr_t value);
static SOPC_ReturnStatus cert_group_set_cert_type(SOPC_CertGroupContext* pCertGroup, SOPC_Certificate_Type certType);
static SOPC_ReturnStatus certificate_group_write_file(const char* filePath, const uint8_t* data, uint32_t length);
static bool cert_group_register_ctx_from_node_id(const SOPC_NodeId* pObjectId, SOPC_CertGroupContext* pContext);

/*---------------------------------------------------------------------------
 *                       Static functions (implementation)
 *---------------------------------------------------------------------------*/

static SOPC_ReturnStatus cert_group_create_context(SOPC_CertGroupContext** ppCertGroup)
{
    if (NULL == ppCertGroup)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_CertGroupContext* pCertGroup = NULL;
    pCertGroup = SOPC_Calloc(1, sizeof(SOPC_CertGroupContext));
    if (NULL == pCertGroup)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }
    *ppCertGroup = pCertGroup;
    return SOPC_STATUS_OK;
}

static void cert_group_clear_context(SOPC_CertGroupContext* pCertGroup)
{
    if (NULL == pCertGroup)
    {
        return;
    }
    SOPC_Free(pCertGroup->cStrId);
    SOPC_Free(pCertGroup->pKeyPath);
    SOPC_Free(pCertGroup->pCertPath);
    CertificateGroup_DiscardNewKey(pCertGroup);
    pCertGroup->pKeyPath = NULL;
    pCertGroup->pCertPath = NULL;
    /* Safely unreference the keyCertPair */
    pCertGroup->pKeyCertPair = NULL;
}

static void cert_group_delete_context(SOPC_CertGroupContext** ppCertGroup)
{
    SOPC_CertGroupContext* pCertGroup = *ppCertGroup;
    if (NULL == pCertGroup)
    {
        return;
    }
    cert_group_clear_context(pCertGroup);
    SOPC_Free(pCertGroup);
    *ppCertGroup = NULL;
}

static void cert_group_dict_free_context_value(uintptr_t value)
{
    if (NULL != (void*) value)
    {
        cert_group_delete_context((SOPC_CertGroupContext**) &value);
    }
}

static SOPC_ReturnStatus cert_group_set_cert_type(SOPC_CertGroupContext* pCertGroup, SOPC_Certificate_Type certType)
{
    SOPC_ASSERT(NULL != pCertGroup);
    SOPC_ASSERT(NULL == pCertGroup->pCertificateTypeValueId);

    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    switch (certType)
    {
    case SOPC_CERT_TYPE_RSA_MIN_APPLICATION:
        pCertGroup->pCertificateTypeValueId = &certTypeRsaMinId;
        break;
    case SOPC_CERT_TYPE_RSA_SHA256_APPLICATION:
        pCertGroup->pCertificateTypeValueId = &certTypeRsaSha256Id;
        break;
    default:
        status = SOPC_STATUS_INVALID_PARAMETERS;
        break;
    }
    return status;
}

#if SOPC_HAS_FILESYSTEM
static SOPC_ReturnStatus certificate_group_write_file(const char* filePath, const uint8_t* data, uint32_t length)
{
    if (NULL == filePath || NULL == data || 0 == length)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_ReturnStatus status = SOPC_GoodGenericStatus;
    /* Open in binary format and erase if existing */
    FILE* fp = fopen(filePath, "wb+");
    if (NULL == fp)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }
    if (SOPC_STATUS_OK == status)
    {
        size_t nb_written = fwrite(data, 1, length, fp);
        if (length != nb_written)
        {
            status = SOPC_STATUS_NOK;
        }
    }
    /* Close*/
    if (NULL != fp)
    {
        fclose(fp);
    }
    return status;
}
#else
static SOPC_ReturnStatus certificate_group_write_file(const char* filePath, const uint8_t* data, uint32_t length)
{
    SOPC_UNUSED_ARG(filePath);
    SOPC_UNUSED_ARG(data);
    SOPC_UNUSED_ARG(length);
    return SOPC_STATUS_NOT_SUPPORTED;
}
#endif /* SOPC_HAS_FILESYSTEM  */

/* Insert a new objectId key and CertificateGroup context value */
bool cert_group_register_ctx_from_node_id(const SOPC_NodeId* pObjectId, SOPC_CertGroupContext* pContext)
{
    if (NULL == gObjIdToCertGroup || NULL == pObjectId || NULL == pContext)
    {
        return false;
    }
    bool res = SOPC_Dict_Insert(gObjIdToCertGroup, (uintptr_t) pObjectId, (uintptr_t) pContext);
    return res;
}

/*---------------------------------------------------------------------------
 *                             ITF Functions (implementation)
 *---------------------------------------------------------------------------*/

SOPC_ReturnStatus SOPC_CertificateGroup_Initialize(void)
{
    if (NULL != gObjIdToCertGroup)
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    /* The key is include in the value (CertificateGroup context) */
    gObjIdToCertGroup = SOPC_NodeId_Dict_Create(false, cert_group_dict_free_context_value);
    if (NULL == gObjIdToCertGroup)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }
    /* Mandatory for the use of SOPC_Dict_Remove */
    SOPC_Dict_SetTombstoneKey(gObjIdToCertGroup, (uintptr_t) &gTombstoneKey);
    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_CertificateGroup_GetDefaultConfiguration(const SOPC_TrustList_Type groupType,
                                                                const SOPC_Certificate_Type certType,
                                                                SOPC_PKIProvider* pPKI,
                                                                const uint32_t maxTrustListSize,
                                                                SOPC_KeyCertPair* pKeyCertPair,
                                                                const char* pKeyPath,
                                                                const char* pCertPath,
                                                                SOPC_CertificateGroup_Config** ppConfig)
{
    if (NULL == pPKI || 0 == maxTrustListSize || NULL == ppConfig)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    const CertificateGroup_NodeIds* pNodeIds = NULL;
    if (SOPC_TRUSTLIST_GROUP_APP == groupType)
    {
        if (SOPC_CERT_TYPE_RSA_MIN_APPLICATION != certType && SOPC_CERT_TYPE_RSA_SHA256_APPLICATION != certType)
        {
            return SOPC_STATUS_INVALID_PARAMETERS;
        }
        pNodeIds = &appNodeIds;
    }
    else if (SOPC_TRUSTLIST_GROUP_USR == groupType)
    {
        pNodeIds = &usrNodeIds;
    }
    else
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    /* keyCertPair <=> key and cert paths */
    if ((NULL != pKeyCertPair) && ((NULL != pKeyPath && NULL == pCertPath) || (NULL == pKeyPath && NULL != pCertPath)))
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    /* key and cert has no meaning for user group */
    if (SOPC_TRUSTLIST_GROUP_USR == groupType && NULL != pKeyCertPair)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (SOPC_TRUSTLIST_GROUP_USR != groupType && NULL == pKeyCertPair)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_CertificateGroup_Config* pCfg = SOPC_Calloc(1, sizeof(SOPC_CertificateGroup_Config));
    if (NULL == pCfg)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    char* keyPath = NULL;
    char* certPath = NULL;
    if (NULL != pKeyPath)
    {
        keyPath = SOPC_strdup(pKeyPath);
        if (NULL == keyPath)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }
    if (NULL != pCertPath)
    {
        certPath = SOPC_strdup(pCertPath);
        if (NULL == certPath)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }
    SOPC_TrustList_Config* pTrustListCfg = NULL;
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_TrustList_GetDefaultConfiguration(groupType, pPKI, maxTrustListSize, &pTrustListCfg);
    }
    if (SOPC_STATUS_OK == status)
    {
        pCfg->pIds = pNodeIds;
        pCfg->pTrustListCfg = pTrustListCfg;
        pCfg->pPKI = pPKI;
        pCfg->groupType = groupType;
        pCfg->certType = certType;
        pCfg->pKeyCertPair = pKeyCertPair;
        pCfg->pKeyPath = keyPath;
        pCfg->pCertPath = certPath;
        pCfg->bIsTOFUSate = false;
    }
    else
    {
        SOPC_Free(pCfg);
        SOPC_Free(keyPath);
        SOPC_Free(certPath);
        pCfg = NULL;
    }
    *ppConfig = pCfg;
    return status;
}

SOPC_ReturnStatus SOPC_CertificateGroup_GetTOFUConfiguration(const SOPC_Certificate_Type certType,
                                                             SOPC_PKIProvider* pPKI,
                                                             const uint32_t maxTrustListSize,
                                                             SOPC_TrustList_UpdateCompleted_Fct* pFnUpdateCompleted,
                                                             SOPC_CertificateGroup_Config** ppConfig)
{
    if (NULL == pPKI || 0 == maxTrustListSize || NULL == pFnUpdateCompleted || NULL == ppConfig)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (SOPC_CERT_TYPE_RSA_MIN_APPLICATION != certType && SOPC_CERT_TYPE_RSA_SHA256_APPLICATION != certType)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    const CertificateGroup_NodeIds* pNodeIds = &appNodeIds;

    SOPC_CertificateGroup_Config* pCfg = SOPC_Calloc(1, sizeof(SOPC_CertificateGroup_Config));
    if (NULL == pCfg)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_TrustList_Config* pTrustListCfg = NULL;
    status = SOPC_TrustList_GetTOFUConfiguration(SOPC_TRUSTLIST_GROUP_APP, pPKI, maxTrustListSize, pFnUpdateCompleted,
                                                 &pTrustListCfg);
    if (SOPC_STATUS_OK == status)
    {
        pCfg->pIds = pNodeIds;
        pCfg->pTrustListCfg = pTrustListCfg;
        pCfg->pPKI = pPKI;
        pCfg->groupType = SOPC_TRUSTLIST_GROUP_APP;
        pCfg->certType = certType;
        pCfg->pKeyCertPair = NULL;
        pCfg->pKeyPath = NULL;
        pCfg->pCertPath = NULL;
        pCfg->bIsTOFUSate = true;
    }
    else
    {
        SOPC_Free(pCfg);
        pCfg = NULL;
    }
    *ppConfig = pCfg;
    return status;
}

void SOPC_CertificateGroup_DeleteConfiguration(SOPC_CertificateGroup_Config** ppConfig)
{
    if (NULL == ppConfig)
    {
        return;
    }
    SOPC_CertificateGroup_Config* pConfig = *ppConfig;
    if (NULL == pConfig)
    {
        return;
    }
    SOPC_TrustList_DeleteConfiguration(&pConfig->pTrustListCfg);
    SOPC_Free(pConfig->pKeyPath);
    SOPC_Free(pConfig->pCertPath);
    memset(pConfig, 0, sizeof(SOPC_CertificateGroup_Config));
    SOPC_Free(pConfig);
    *ppConfig = NULL;
}

SOPC_ReturnStatus SOPC_CertificateGroup_Configure(const SOPC_CertificateGroup_Config* pCfg,
                                                  SOPC_MethodCallManager* pMcm)
{
    /* The API is not initialized. */
    if (NULL == gObjIdToCertGroup)
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    /* Check parameters */
    if (NULL == pCfg || NULL == pMcm)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (NULL == pCfg->pIds || NULL == pCfg->pPKI || NULL == pCfg->pTrustListCfg)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    /* keyCertPair <=> key and cert paths */
    if (NULL != pCfg->pKeyCertPair &&
        ((NULL == pCfg->pKeyPath && NULL != pCfg->pCertPath) || (NULL != pCfg->pKeyPath && NULL == pCfg->pCertPath)))
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    /* key and cert has no meaning for user group */
    if (SOPC_TRUSTLIST_GROUP_USR == pCfg->groupType && NULL != pCfg->pKeyCertPair)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (SOPC_TRUSTLIST_GROUP_USR != pCfg->groupType && NULL == pCfg->pKeyCertPair && !pCfg->bIsTOFUSate)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    const CertificateGroup_NodeIds* pIds = pCfg->pIds;
    if (NULL == pIds->pCertificateGroupId || NULL == pIds->pCertificateTypesId)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    /* Configure the TrustList that belongs to this group */
    SOPC_ReturnStatus status = SOPC_TrustList_Configure(pCfg->pTrustListCfg, pMcm);
    if (SOPC_STATUS_OK != status)
    {
        return status;
    }
    /* Configure the CertificateGroup */
    SOPC_CertGroupContext* pCertGroup = NULL;
    if (SOPC_STATUS_OK == status)
    {
        status = cert_group_create_context(&pCertGroup);
    }
    if (SOPC_STATUS_OK == status)
    {
        pCertGroup->pObjectId = pIds->pCertificateGroupId;
        pCertGroup->pTrustListId = pIds->pTrustListId;
        pCertGroup->pCertificateTypeId = pIds->pCertificateTypesId;
        /* CertificateType has no meaning for user group, as only TrustList can be updated.
            §7.8.3.3 part 12 v1.05:  DefaultUserTokenGroup Object shall leave CertificateTypes list empty*/
        if (SOPC_TRUSTLIST_GROUP_USR != pCfg->groupType)
        {
            status = cert_group_set_cert_type(pCertGroup, pCfg->certType);
        }
        if (SOPC_STATUS_OK == status)
        {
            pCertGroup->cStrId = SOPC_NodeId_ToCString(pIds->pTrustListId);
            if (NULL == pCertGroup->cStrId)
            {
                status = SOPC_STATUS_OUT_OF_MEMORY;
            }
        }
    }
    /* Add the paths */
    if (SOPC_STATUS_OK == status && !pCfg->bIsTOFUSate)
    {
        if (NULL != pCfg->pKeyPath)
        {
            pCertGroup->pKeyPath = SOPC_strdup(pCfg->pKeyPath);
            if (NULL == pCertGroup->pKeyPath)
            {
                status = SOPC_STATUS_OUT_OF_MEMORY;
            }
        }
        if (NULL != pCfg->pCertPath)
        {
            pCertGroup->pCertPath = SOPC_strdup(pCfg->pCertPath);
            if (NULL == pCertGroup->pCertPath)
            {
                status = SOPC_STATUS_OUT_OF_MEMORY;
            }
        }
    }
    /* Finally add the certificateGroup to the dictionary */
    if (SOPC_STATUS_OK == status)
    {
        pCertGroup->pKeyCertPair = pCfg->pKeyCertPair;
        bool res = cert_group_register_ctx_from_node_id(pCertGroup->pObjectId, pCertGroup);
        status = !res ? SOPC_STATUS_NOK : SOPC_STATUS_OK;
    }

    if (SOPC_STATUS_OK != status)
    {
        if (NULL != pCertGroup)
        {
            TrustList_RemoveFromNodeId(pCertGroup->pTrustListId);
            cert_group_delete_context(&pCertGroup);
        }
    }
    return status;
}

void SOPC_CertificateGroup_Clear(void)
{
    SOPC_Dict_Delete(gObjIdToCertGroup);
    gObjIdToCertGroup = NULL;
}

/*---------------------------------------------------------------------------
 *                       Internal Functions (implementation)
 *---------------------------------------------------------------------------*/

/* Get the CertificateGroup context from the nodeId */
SOPC_CertGroupContext* CertificateGroup_GetFromNodeId(const SOPC_NodeId* pObjectId, bool* bFound)
{
    if (NULL == gObjIdToCertGroup || NULL == pObjectId)
    {
        *bFound = false;
        return NULL;
    }
    SOPC_CertGroupContext* pCtx = NULL;
    pCtx = (SOPC_CertGroupContext*) SOPC_Dict_Get(gObjIdToCertGroup, (const uintptr_t) pObjectId, bFound);
    if (!bFound || NULL == pCtx)
    {
        char* cStrId = SOPC_NodeId_ToCString(pObjectId);
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "CertificateGroup: unable to retrieve the CertificateGroup context from %s", cStrId);
        SOPC_Free(cStrId);
    }
    return pCtx;
}

const char* CertificateGroup_GetStrNodeId(const SOPC_CertGroupContext* pGroupCtx)
{
    SOPC_ASSERT(NULL != pGroupCtx);
    return (const char*) pGroupCtx->cStrId;
}

bool CertificateGroup_CheckIsApplicationGroup(const SOPC_NodeId* pGroupId)
{
    if (NULL == pGroupId)
    {
        return false;
    }
    bool bIsUserGrp = SOPC_NodeId_Equal(pGroupId, &usrCertificateGroupId);
    return !bIsUserGrp;
}

bool CertificateGroup_CheckType(const SOPC_CertGroupContext* pGroupCtx, const SOPC_NodeId* pExpectedCertTypeId)
{
    SOPC_ASSERT(NULL != pGroupCtx);
    SOPC_ASSERT(NULL != pExpectedCertTypeId);

    bool bIsEqual = SOPC_NodeId_Equal(pGroupCtx->pCertificateTypeValueId, pExpectedCertTypeId);
    return bIsEqual;
}

bool CertificateGroup_CheckSubjectName(SOPC_CertGroupContext* pGroupCtx, const SOPC_String* pSubjectName)
{
    SOPC_ASSERT(NULL != pGroupCtx);
    SOPC_ASSERT(NULL != pSubjectName);

    /* TODO: Check the given subjectName (cf public issue #1289)*/
    if (NULL != pSubjectName->Data || -1 != pSubjectName->Length)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "PushSrvCfg:Method_CreateSigningRequest:CertGroup:%s: custom subjectName is not allowed",
                               pGroupCtx->cStrId);
        return false;
    }
    return true;
}

void CertificateGroup_DiscardNewKey(SOPC_CertGroupContext* pGroupCtx)
{
    SOPC_ASSERT(NULL != pGroupCtx);
    SOPC_KeyManager_AsymmetricKey_Free(pGroupCtx->pNewKeyPair);
    pGroupCtx->pNewKeyPair = NULL;
}

SOPC_ReturnStatus CertificateGroup_CreateSigningRequest(SOPC_CertGroupContext* pGroupCtx,
                                                        const SOPC_String* pSubjectName,
                                                        const bool bRegeneratePrivateKey,
                                                        SOPC_ByteString* pCertificateRequest,
                                                        const uint32_t endpointConfigIdx)
{
    SOPC_ASSERT(NULL != pGroupCtx);
    SOPC_ASSERT(NULL != pGroupCtx->pCertificateTypeValueId);
    SOPC_UNUSED_ARG(pSubjectName); // TODO: if not NULL then use pSubjectName instead of the current subjectName (cf
                                   // public issue 1289)

    if (NULL == pCertificateRequest)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (NULL != pCertificateRequest->Data || -1 != pCertificateRequest->Length)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_CertificateList* pCert = NULL;
    SOPC_CSR* pNewCSR = NULL;
    char** pDNSArray = NULL;
    uint32_t DNSArrayLen = 0;
    char* pURI = NULL;
    char* pCertSubjectName = NULL;
    uint32_t subjectNameLen = 0;
    uint8_t* pCSR_DER = NULL;
    uint32_t CSR_DERLen = 0;

    uint32_t keySize = 0;
    char* mdAlg = NULL;

    SOPC_AsymmetricKey* pNewKey = NULL;
    SOPC_AsymmetricKey* pCurKey = NULL;
    SOPC_AsymmetricKey* pKey = NULL;

    char* pEndPointHostName = NULL;
    char* pEndpointPort = NULL;
    char** DNSToUse = NULL;
    uint32_t nameCount = 0;

    /* Get properties form group */
    if (OpcUaId_RsaMinApplicationCertificateType == pGroupCtx->pCertificateTypeValueId->Data.Numeric)
    {
        keySize = SOPC_CERT_GRP_MIN_KEY_SIZE;
        mdAlg = SOPC_strdup(SOPC_CERT_GRP_MIN_MD_ALG);
    }
    else if (OpcUaId_RsaSha256ApplicationCertificateType == pGroupCtx->pCertificateTypeValueId->Data.Numeric)
    {
        keySize = SOPC_CERT_GRP_MAX_KEY_SIZE;
        mdAlg = SOPC_strdup(SOPC_CERT_GRP_MAX_MD_ALG);
    }
    else
    {
        char* pToPrint = SOPC_NodeId_ToCString(pGroupCtx->pCertificateTypeValueId);
        const char* toPrint = NULL == pToPrint ? "NULL" : pToPrint;
        SOPC_Logger_TraceError(
            SOPC_LOG_MODULE_CLIENTSERVER,
            "PushSrvCfg:Method_CreateSigningRequest:CertificateGroup:%s: rcv unexpected certificate type nodeId : %s",
            pGroupCtx->cStrId, toPrint);
        SOPC_Free(pToPrint);
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    status = SOPC_KeyCertPair_GetCertCopy(pGroupCtx->pKeyCertPair, &pCert);
    /* Get the subjectName of the current certificate */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_KeyManager_Certificate_GetSubjectName(pCert, &pCertSubjectName, &subjectNameLen);
    }
    /* Get the URI of the current certificate */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_KeyManager_Certificate_GetMaybeApplicationUri(pCert, &pURI, NULL);
    }
    /* Get the DNS names of the current certificate */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_KeyManager_Certificate_GetSanDnsNames(pCert, &pDNSArray, &DNSArrayLen);
        if (0 == DNSArrayLen)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "PushSrvCfg:Method_CreateSigningRequest:CertificateGroup:%s: DNS name is not "
                                   "defined for the current server certificate (x509 SubjectAlternativeName extension)",
                                   pGroupCtx->cStrId);
            status = SOPC_STATUS_INVALID_STATE;
        }
    }
    /* Get HostName name of the current endpoint */
    if (SOPC_STATUS_OK == status)
    {
        SOPC_Endpoint_Config* pEndpointCfg = SOPC_ToolkitServer_GetEndpointConfig(endpointConfigIdx);
        SOPC_ASSERT(NULL != pEndpointCfg);

        SOPC_UriType type = SOPC_URI_UNDETERMINED;
        status = SOPC_Helper_URI_SplitUri(pEndpointCfg->endpointURL, &type, &pEndPointHostName, &pEndpointPort);
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON,
                                   "PushSrvCfg:Method_CreateSigningRequest:CertificateGroup:%s: Unable to split the "
                                   "given endpoint url %s to retrieve the hostName",
                                   pGroupCtx->cStrId, pEndpointCfg->endpointURL);
        }
        else
        {
            if (NULL == pEndPointHostName)
            {
                status = SOPC_STATUS_OUT_OF_MEMORY;
            }
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        DNSToUse = SOPC_Calloc((size_t) DNSArrayLen + 1, sizeof(char*));
        if (NULL == DNSToUse)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }
    /* Append the HostName of the current endpoint to the array of DNS name */
    if (SOPC_STATUS_OK == status)
    {
        size_t endpointHostNameLen = strlen(pEndPointHostName);
        int match = -1;
        /* Exchange the data */
        for (size_t idx = 0; idx < DNSArrayLen; idx++)
        {
            if (0 != match)
            {
                if (endpointHostNameLen == strlen(pDNSArray[idx]))
                {
                    match = SOPC_strncmp_ignore_case(pEndPointHostName, pDNSArray[idx], endpointHostNameLen);
                }
            }
            DNSToUse[idx] = pDNSArray[idx];
        }
        nameCount = DNSArrayLen;
        if (0 != match)
        {
            DNSToUse[DNSArrayLen] = pEndPointHostName;
            nameCount = DNSArrayLen + 1;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_KeyManager_CSR_Create(pCertSubjectName, true, mdAlg, pURI, DNSToUse, nameCount, &pNewCSR);
    }
    if (SOPC_STATUS_OK == status)
    {
        if (bRegeneratePrivateKey)
        {
            status = SOPC_KeyManager_AsymmetricKey_GenRSA(keySize, &pNewKey);
        }
        else
        {
            status = SOPC_KeyCertPair_GetKeyCopy(pGroupCtx->pKeyCertPair, &pCurKey);
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        pKey = bRegeneratePrivateKey ? pNewKey : pCurKey;
        status = SOPC_KeyManager_CSR_ToDER(pNewCSR, pKey, &pCSR_DER, &CSR_DERLen);
        if (SOPC_STATUS_OK == status)
        {
            if (INT32_MAX < CSR_DERLen)
            {
                status = SOPC_STATUS_OUT_OF_MEMORY;
            }
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        SOPC_ByteString_Initialize(pCertificateRequest);
        pCertificateRequest->Data = pCSR_DER;
        pCertificateRequest->Length = (int32_t) CSR_DERLen;
    }

    /* Clear */
    SOPC_KeyManager_Certificate_Free(pCert);
    SOPC_Free(pCertSubjectName);
    SOPC_Free(pURI);
    SOPC_Free(pEndpointPort);
    SOPC_Free(pEndPointHostName);
    SOPC_Free(DNSToUse);
    if (NULL != pDNSArray)
    {
        for (uint32_t idx = 0; idx < DNSArrayLen; idx++)
        {
            SOPC_Free(pDNSArray[idx]);
        }
        SOPC_Free(pDNSArray);
    }
    SOPC_Free(mdAlg);
    SOPC_KeyManager_CSR_Free(pNewCSR);
    SOPC_KeyManager_AsymmetricKey_Free(pCurKey);
    pCurKey = NULL;
    if (SOPC_STATUS_OK != status)
    {
        CertificateGroup_DiscardNewKey(pGroupCtx);
        pNewKey = NULL;
        SOPC_Free(pCSR_DER);
        pCSR_DER = NULL;
        SOPC_ByteString_Initialize(pCertificateRequest);
    }

    pGroupCtx->pNewKeyPair = pNewKey;
    return status;
}

SOPC_StatusCode CertificateGroup_GetRejectedList(const SOPC_CertGroupContext* pGroupCtx,
                                                 SOPC_ByteString** ppBsCertArray,
                                                 uint32_t* pLength)
{
    SOPC_ASSERT(NULL != pGroupCtx);
    SOPC_ASSERT(NULL != ppBsCertArray);
    SOPC_ASSERT(NULL != pLength);

    *ppBsCertArray = NULL;
    *pLength = 0;

    if (NULL == pGroupCtx->pTrustListId)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "CertificateGroup:%s: no TrustList is configured",
                               pGroupCtx->cStrId);
        return OpcUa_BadUnexpectedError;
    }

    bool bFound = false;
    SOPC_StatusCode stCode = SOPC_GoodGenericStatus;
    SOPC_TrustListContext* pCtx = TrustList_GetFromNodeId(pGroupCtx->pTrustListId, false, NULL, &bFound);
    if (NULL == pCtx || !bFound)
    {
        return OpcUa_BadUnexpectedError;
    }
    /* Get the rejected list */
    uint32_t lenArray = 0;
    SOPC_SerializedCertificate* pRawCertArray = NULL;
    SOPC_ByteString* pBsCertArray = NULL;
    SOPC_CertificateList* pCerts = NULL;
    SOPC_ByteString* pByteStr = NULL;
    const SOPC_Buffer* pRawBuffer = NULL;
    uint32_t bufLength = 0;

    SOPC_ASSERT(NULL != pCtx->pPKI);
    SOPC_ReturnStatus status = SOPC_PKIProvider_CopyRejectedList(pCtx->pPKI, &pCerts);
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceError(
            SOPC_LOG_MODULE_CLIENTSERVER,
            "CertificateGroup:%s: unable to retrieve the rejected list from the PKI that belongs to the TrustList %s",
            pGroupCtx->cStrId, pCtx->cStrObjectId);
        return OpcUa_BadUnexpectedError;
    }
    if (NULL == pCerts)
    {
        /* Nothing to do, the rejected list is empty */
        return SOPC_GoodGenericStatus;
    }
    status = SOPC_KeyManager_CertificateList_AttachToSerializedArray(pCerts, &pRawCertArray, &lenArray);
    stCode = SOPC_STATUS_OK == status ? SOPC_GoodGenericStatus : OpcUa_BadUnexpectedError;
    if (SOPC_STATUS_OK == status)
    {
        pBsCertArray = SOPC_Calloc((size_t) lenArray, sizeof(SOPC_ByteString));
        if (NULL == pBsCertArray)
        {
            stCode = OpcUa_BadOutOfMemory;
        }
    }

    for (uint32_t i = 0; i < lenArray && SOPC_IsGoodStatus(stCode); i++)
    {
        pRawBuffer = SOPC_KeyManager_SerializedCertificate_Data(&pRawCertArray[i]);
        if (NULL == pRawBuffer)
        {
            stCode = OpcUa_BadOutOfMemory;
        }
        /* Check before casting */
        if (SOPC_IsGoodStatus(stCode) && NULL != pRawBuffer)
        {
            bufLength = pRawBuffer->length;
            if (INT32_MAX < bufLength)
            {
                stCode = OpcUa_BadOutOfMemory;
            }
        }
        if (SOPC_IsGoodStatus(stCode) && NULL != pBsCertArray)
        {
            pByteStr = &pBsCertArray[i];
        }
        if (SOPC_IsGoodStatus(stCode) && NULL != pByteStr)
        {
            SOPC_ByteString_Initialize(pByteStr);
            pByteStr->Data = SOPC_Calloc((size_t) bufLength, sizeof(SOPC_Byte));
            if (NULL == pByteStr->Data)
            {
                stCode = OpcUa_BadOutOfMemory;
            }
        }
        if (SOPC_IsGoodStatus(stCode) && NULL != pByteStr && NULL != pRawBuffer)
        {
            if (NULL != pByteStr->Data && NULL != pRawBuffer->data && bufLength <= INT32_MAX)
            {
                memcpy(pByteStr->Data, pRawBuffer->data, bufLength);
                pByteStr->Length = (int32_t) bufLength;
            }
            else
            {
                stCode = OpcUa_BadInvalidState;
            }
        }
    }

    if (!SOPC_IsGoodStatus(stCode))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "CertificateGroup:%s: unable to get the rejected list",
                               pGroupCtx->cStrId);
        uint32_t idx = 0;
        for (idx = 0; idx < lenArray && NULL != pBsCertArray; idx++)
        {
            SOPC_ByteString_Clear(&pBsCertArray[idx]);
        }
        SOPC_Free(pBsCertArray);
        pBsCertArray = NULL;
        lenArray = 0;
    }

    SOPC_KeyManager_Certificate_Free(pCerts);
    SOPC_Free(pRawCertArray);

    *ppBsCertArray = pBsCertArray;
    *pLength = lenArray;

    return stCode;
}

SOPC_StatusCode CertificateGroup_ExportRejectedList(const SOPC_CertGroupContext* pGroupCtx)
{
    SOPC_ASSERT(NULL != pGroupCtx);

    if (NULL == pGroupCtx->pTrustListId)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "CertificateGroup:%s: no TrustList is configured",
                               pGroupCtx->cStrId);
        return OpcUa_BadUnexpectedError;
    }
    bool bFound = false;
    SOPC_TrustListContext* pCtx = TrustList_GetFromNodeId(pGroupCtx->pTrustListId, false, NULL, &bFound);
    if (NULL == pCtx || !bFound)
    {
        return OpcUa_BadUnexpectedError;
    }
    SOPC_ASSERT(NULL != pCtx->pPKI);
    SOPC_ReturnStatus status = SOPC_PKIProvider_WriteRejectedCertToStore(pCtx->pPKI);
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER, "CertificateGroup:%s: export of rejected list failed",
                                 pGroupCtx->cStrId);
        return OpcUa_BadUnexpectedError;
    }
    else
    {
        return SOPC_GoodGenericStatus;
    }
}

SOPC_StatusCode CertificateGroup_UpdateCertificate(SOPC_CertGroupContext* pGroupCtx,
                                                   const SOPC_ByteString* pCertificate,
                                                   const uint32_t endpointConfigIdx)
{
    SOPC_ASSERT(NULL != pGroupCtx);
    SOPC_ASSERT(NULL != pGroupCtx->pCertificateTypeValueId);

    /* Check parameters */
    if (NULL == pCertificate)
    {
        return OpcUa_BadCertificateInvalid;
    }
    if (NULL == pCertificate->Data || pCertificate->Length < 0)
    {
        return OpcUa_BadCertificateInvalid;
    }
    bool bFound = false;
    /* Retrieve the TrustList to validate the new certificate */
    SOPC_TrustListContext* pTrustListCtx = TrustList_GetFromNodeId(pGroupCtx->pTrustListId, false, NULL, &bFound);
    if (NULL == pTrustListCtx || !bFound)
    {
        return OpcUa_BadUnexpectedError;
    }
    SOPC_ASSERT(NULL != pTrustListCtx->pPKI);

    SOPC_StatusCode stCode = SOPC_GoodGenericStatus;
    SOPC_CertificateList* pNewCert = NULL;
    SOPC_AsymmetricKey* pNewPublicKey = NULL;
    SOPC_AsymmetricKey* pCurrentKeyPair = NULL;
    SOPC_SerializedAsymmetricKey* pSerGivenPublicKey = NULL;
    SOPC_SerializedAsymmetricKey* pSerCurrentPublicKey = NULL;
    SOPC_SerializedAsymmetricKey* pSerNewKey = NULL;
    uint32_t newPublicKeyLen = 0;
    uint32_t currentPublicKeyLen = 0;
    uint32_t newKeyLen = 0;
    const SOPC_ExposedBuffer* pRawNewPublicKey = NULL;
    const SOPC_ExposedBuffer* pRawCurrentPubicKey = NULL;
    const SOPC_ExposedBuffer* pRawNewKey = NULL;
    SOPC_CertificateList* pCurrentServerCert = NULL;
    char* pURI = NULL;
    uint32_t errorCode = 0;

    /* TODO: Add a new function declaration in KeyManager API to check if a public-private pair of keys matches (cf
     * public issue #1290)*/
    /* Load the public key from the given certificate */
    SOPC_ReturnStatus status =
        SOPC_KeyManager_Certificate_CreateOrAddFromDER(pCertificate->Data, (uint32_t) pCertificate->Length, &pNewCert);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_KeyManager_AsymmetricKey_CreateFromCertificate(pNewCert, &pNewPublicKey);
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_KeyManager_SerializedAsymmetricKey_CreateFromKey(pNewPublicKey, true, &pSerGivenPublicKey);
        }
    }
    if (SOPC_STATUS_OK != status)
    {
        stCode = OpcUa_BadCertificateInvalid;
    }
    /* Load the current public key (or the previously generated during CreateSigningRequest method) */
    if (SOPC_STATUS_OK == status)
    {
        if (NULL != pGroupCtx->pNewKeyPair)
        {
            pCurrentKeyPair = pGroupCtx->pNewKeyPair;
        }
        else
        {
            status = SOPC_KeyCertPair_GetKeyCopy(pGroupCtx->pKeyCertPair, &pCurrentKeyPair);
        }
        if (SOPC_STATUS_OK == status)
        {
            /* Get the serialized public key part */
            status =
                SOPC_KeyManager_SerializedAsymmetricKey_CreateFromKey(pCurrentKeyPair, true, &pSerCurrentPublicKey);
        }
        if (SOPC_STATUS_OK != status)
        {
            stCode = OpcUa_BadUnexpectedError;
        }
    }
    /* Check if the public key match with the existing certificate and the privateKey */
    if (SOPC_STATUS_OK == status)
    {
        newPublicKeyLen = SOPC_SecretBuffer_GetLength(pSerGivenPublicKey);
        currentPublicKeyLen = SOPC_SecretBuffer_GetLength(pSerCurrentPublicKey);
        if (newPublicKeyLen != currentPublicKeyLen)
        {
            status = SOPC_STATUS_NOK;
            stCode = OpcUa_BadSecurityChecksFailed;
        }
        else
        {
            pRawNewPublicKey = SOPC_SecretBuffer_Expose(pSerGivenPublicKey);
            pRawCurrentPubicKey = SOPC_SecretBuffer_Expose(pSerCurrentPublicKey);
            int match = memcmp(pRawNewPublicKey, pRawCurrentPubicKey, newPublicKeyLen);
            if (0 != match)
            {
                status = SOPC_STATUS_NOK;
                stCode = OpcUa_BadSecurityChecksFailed;
            }
            SOPC_SecretBuffer_Unexpose(pRawNewPublicKey, pSerGivenPublicKey);
            SOPC_SecretBuffer_Unexpose(pRawCurrentPubicKey, pSerCurrentPublicKey);
        }
    }
    /* Retrieve the application URI from the current server certificate */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_KeyCertPair_GetCertCopy(pGroupCtx->pKeyCertPair, &pCurrentServerCert);
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_KeyManager_Certificate_GetMaybeApplicationUri(pCurrentServerCert, &pURI, NULL);
        }
        if (SOPC_STATUS_OK != status)
        {
            stCode = OpcUa_BadUnexpectedError;
        }
    }
    /* Retrieve the current endpoint configuration */
    SOPC_Endpoint_Config* pEndpointCfg = SOPC_ToolkitServer_GetEndpointConfig(endpointConfigIdx);
    SOPC_ASSERT(NULL != pEndpointCfg);
    /* Set the PKI leaf profile */
    SOPC_PKI_LeafProfile leafProfile = {.mdSign = SOPC_PKI_MD_SHA256,
                                        .pkAlgo = SOPC_PKI_PK_RSA,
                                        .RSAMinimumKeySize = 1024,
                                        .RSAMaximumKeySize = 4096,
                                        .bApplySecurityPolicy = true,
                                        .keyUsage = SOPC_PKI_KU_KEY_ENCIPHERMENT | SOPC_PKI_KU_KEY_DATA_ENCIPHERMENT |
                                                    SOPC_PKI_KU_DIGITAL_SIGNATURE | SOPC_PKI_KU_NON_REPUDIATION,
                                        .extendedKeyUsage = SOPC_PKI_EKU_SERVER_AUTH,
                                        .sanApplicationUri = pURI,
                                        .sanURL = pEndpointCfg->endpointURL};
    if (SOPC_STATUS_OK == status)
    {
        /* Get properties form group */
        if (OpcUaId_RsaMinApplicationCertificateType == pGroupCtx->pCertificateTypeValueId->Data.Numeric)
        {
            leafProfile.RSAMaximumKeySize = SOPC_CERT_GRP_MIN_KEY_SIZE;
        }
        else if (OpcUaId_RsaSha256ApplicationCertificateType == pGroupCtx->pCertificateTypeValueId->Data.Numeric)
        {
            leafProfile.RSAMinimumKeySize = SOPC_CERT_GRP_MIN_KEY_SIZE;
        }
        else
        {
            char* pToPrint = SOPC_NodeId_ToCString(pGroupCtx->pCertificateTypeValueId);
            const char* toPrint = NULL == pToPrint ? "NULL" : pToPrint;
            SOPC_Logger_TraceError(
                SOPC_LOG_MODULE_CLIENTSERVER,
                "PushSrvCfg:Method_UpdateCertificate:CertificateGroup:%s: rcv unexpected certificate type nodeId : %s",
                pGroupCtx->cStrId, toPrint);
            SOPC_Free(pToPrint);
            status = SOPC_STATUS_NOK;
            stCode = OpcUa_BadUnexpectedError;
        }
    }
    /* Check the certificate properties */
    if (SOPC_STATUS_OK == status)
    {
        char* pThumb = SOPC_KeyManager_Certificate_GetCstring_SHA1(pNewCert);
        const char* thumb = NULL == pThumb ? "NULL" : pThumb;

        /* Validate the certificate with the trustList */
        SOPC_PKI_ChainProfile chainProfile = {.curves = SOPC_PKI_CURVES_ANY,
                                              .mdSign = SOPC_PKI_MD_SHA1_OR_ABOVE,
                                              .pkAlgo = SOPC_PKI_PK_RSA,
                                              .RSAMinimumKeySize = 1024,
                                              .bDisableRevocationCheck = false};
        const SOPC_PKI_Profile pkiProfile = {.bApplyLeafProfile = true,
                                             .bBackwardInteroperability = false, // Disable CA interop
                                             .chainProfile = &chainProfile,
                                             .leafProfile = &leafProfile};
        status = SOPC_PKIProvider_ValidateCertificate(pTrustListCtx->pPKI, pNewCert, &pkiProfile, &errorCode);
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "PushSrvCfg:Method_UpdateCertificate:CertificateGroup:%s: validation failed "
                                   "with error code %" PRIx32 " for certificate thumbprint <%s>",
                                   pGroupCtx->cStrId, errorCode, thumb);
            stCode = OpcUa_BadSecurityChecksFailed;
        }
        SOPC_Free(pThumb);
    }
    /* TODO: Handle that the security level of the update isn't higher than the security level of the secure channel.
     * (§7.3.4 part 2 v1.05) */

    /* Update the new key-cert pair */
    if (SOPC_STATUS_OK == status)
    {
        if (NULL != pGroupCtx->pNewKeyPair)
        {
            status = SOPC_KeyManager_SerializedAsymmetricKey_CreateFromKey(pCurrentKeyPair, false, &pSerNewKey);
            if (SOPC_STATUS_OK == status)
            {
                pRawNewKey = SOPC_SecretBuffer_Expose(pSerNewKey);
                newKeyLen = SOPC_SecretBuffer_GetLength(pSerNewKey);
            }
        }
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_KeyCertPair_UpdateFromBytes(pGroupCtx->pKeyCertPair, (uint32_t) pCertificate->Length,
                                                      (const unsigned char*) pCertificate->Data, newKeyLen, pRawNewKey);
            if (NULL != pRawNewKey)
            {
                SOPC_SecretBuffer_Unexpose(pRawNewKey, pSerNewKey);
            }
        }
    }
    /* Clear */
    SOPC_KeyManager_AsymmetricKey_Free(pNewPublicKey);
    if (NULL == pGroupCtx->pNewKeyPair)
    {
        SOPC_KeyManager_AsymmetricKey_Free(pCurrentKeyPair);
    }
    SOPC_KeyManager_Certificate_Free(pNewCert);
    SOPC_KeyManager_Certificate_Free(pCurrentServerCert);
    SOPC_Free(pURI);
    SOPC_KeyManager_SerializedAsymmetricKey_Delete(pSerCurrentPublicKey);
    SOPC_KeyManager_SerializedAsymmetricKey_Delete(pSerGivenPublicKey);
    SOPC_KeyManager_SerializedAsymmetricKey_Delete(pSerNewKey);

    return stCode;
}

SOPC_ReturnStatus CertificateGroup_Export(const SOPC_CertGroupContext* pGroupCtx)
{
    SOPC_ASSERT(NULL != pGroupCtx);

    // TODO: Add a way to retrieve the password for the private encryption.
    // TODO: Add a store configuration "own" for key-cert pair with defaults value in case update or export failed (cf
    // public issue #1285)
    SOPC_SerializedAsymmetricKey* pOldKey = NULL;
    SOPC_SerializedCertificate* pOldCert = NULL;
    SOPC_SerializedCertificate* pNewCert = NULL;
    SOPC_ReturnStatus restoreStatus = SOPC_STATUS_OK;
    /* Get the new certificate */
    SOPC_ReturnStatus status = SOPC_KeyCertPair_GetSerializedCertCopy(pGroupCtx->pKeyCertPair, &pNewCert);
    /* Save the old certificate */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_KeyManager_SerializedCertificate_CreateFromFile(pGroupCtx->pCertPath, &pOldCert);
    }
    /* Get the old key */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_KeyManager_SerializedAsymmetricKey_CreateFromFile(pGroupCtx->pKeyPath, &pOldKey);
    }
    if (SOPC_STATUS_OK == status)
    {
        /* Write in binary format and erase the old certificate */
        status = certificate_group_write_file(pGroupCtx->pCertPath, pNewCert->data, pNewCert->length);
        /* If error then restore the old certificate */
        if (SOPC_STATUS_OK != status)
        {
            restoreStatus = certificate_group_write_file(pGroupCtx->pCertPath, pOldCert->data, pOldCert->length);
            SOPC_ASSERT(SOPC_STATUS_OK == restoreStatus);
        }
        /* Write the new key */
        if (SOPC_STATUS_OK == status && NULL != pGroupCtx->pNewKeyPair)
        {
            status =
                SOPC_KeyManager_AsymmetricKey_ToPEMFile(pGroupCtx->pNewKeyPair, false, pGroupCtx->pKeyPath, NULL, 0);
            /* If error then restore the old certificate */
            if (SOPC_STATUS_OK != status)
            {
                /* Set the old cert */
                restoreStatus = certificate_group_write_file(pGroupCtx->pCertPath, pOldCert->data, pOldCert->length);
                SOPC_ASSERT(SOPC_STATUS_OK == restoreStatus);
                /* Set the old key */
                uint32_t oldKeyLength = SOPC_SecretBuffer_GetLength(pOldKey);
                const SOPC_ExposedBuffer* pRawOldKey = SOPC_SecretBuffer_ExposeModify(pOldKey);
                SOPC_ASSERT(NULL != pRawOldKey);
                restoreStatus = certificate_group_write_file(pGroupCtx->pKeyPath, pRawOldKey, oldKeyLength);
                SOPC_ASSERT(SOPC_STATUS_OK == restoreStatus);
            }
        }
    }
    /* Clear */
    SOPC_KeyManager_SerializedCertificate_Delete(pOldCert);
    SOPC_KeyManager_SerializedCertificate_Delete(pNewCert);
    SOPC_KeyManager_SerializedAsymmetricKey_Delete(pOldKey);
    return status;
}
