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

#include <string.h>

#include "sopc_assert.h"
#include "sopc_mem_alloc.h"

#include "opcua_identifiers.h"
#include "opcua_statuscodes.h"
#include "sopc_certificate_group.h"
#include "sopc_certificate_group_itf.h"
#include "sopc_helper_string.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_trustlist.h"
#include "sopc_trustlist_itf.h"

/*---------------------------------------------------------------------------
 *                             Constants
 *---------------------------------------------------------------------------*/

#define SOPC_CERT_GRP_MIN_KEY_SIZE 2048
#define SOPC_CERT_GRP_MIN_MD_ALG "SHA256"
#define SOPC_CERT_GRP_MAX_KEY_SIZE 4096
#define SOPC_CERT_GRP_MAX_MD_ALG "SHA256"

/*---------------------------------------------------------------------------
 *                             Internal types
 *---------------------------------------------------------------------------*/

/**
 * \brief Internal structure to gather nodeIds.
 */
typedef struct CertificateGroup_NodeIds
{
    SOPC_NodeId* pCertificateGroupId; /*!< The NodeId of the Certificate Group Object. */
    SOPC_NodeId* pCertificateTypesId; /*!< The nodeId of the CertificateTypes variable. */
    SOPC_NodeId* pTrustListId;        /*!< The nodeId of the TrustList that belongs to the group */
} CertificateGroup_NodeIds;

/**
 * \brief Structure to gather CertificateGroup configuration data.
 */
struct SOPC_CertificateGroup_Config
{
    const CertificateGroup_NodeIds* pIds; /*!< Defined all the nodeId of the CertificateGroup. */
    SOPC_TrustList_Config* pTrustListCfg; /*!< the TrustList configuration that belongs to the CertificateGroup. */
    SOPC_Certificate_Type certType;       /*!< The CertificateType. */
    const SOPC_PKIProvider* pPKI;         /*!< The PKI that belongs to the group. */
    SOPC_TrustList_Type groupType;        /*!< Define the group type (user or app) */
    SOPC_SerializedAsymmetricKey* pKey;   /*!< The private key that belongs to the group.*/
    SOPC_SerializedCertificate* pCert;    /*!< The certificate that belongs to the group.*/
};

/*---------------------------------------------------------------------------
 *                             Global variables
 *---------------------------------------------------------------------------*/

static SOPC_Dict* gObjIdToCertGroup = NULL;
static int32_t gTombstoneKey = -1;

/* NodeIds of the CertificateGroup instance of the DefaultApplicationGroup */
static SOPC_NodeId appCertificateGroupId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultApplicationGroup};
static SOPC_NodeId appCertificateTypesId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultApplicationGroup_CertificateTypes};
static SOPC_NodeId appTrustListId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultApplicationGroup_TrustList};

static const CertificateGroup_NodeIds appNodeIds = {
    .pCertificateGroupId = &appCertificateGroupId,
    .pCertificateTypesId = &appCertificateTypesId,
    .pTrustListId = &appTrustListId,
};

/* NodeIds of the CertificateGroup instance of the DefaultUserTokenGroup */
static SOPC_NodeId usrCertificateGroupId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultUserTokenGroup};
static SOPC_NodeId usrCertificateTypesId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultUserTokenGroup_CertificateTypes};
static SOPC_NodeId usrTrustListId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultUserTokenGroup_TrustList};

static const CertificateGroup_NodeIds usrNodeIds = {
    .pCertificateGroupId = &usrCertificateGroupId,
    .pCertificateTypesId = &usrCertificateTypesId,
    .pTrustListId = &usrTrustListId,
};

/*---------------------------------------------------------------------------
 *                      Prototype of static functions
 *---------------------------------------------------------------------------*/

static SOPC_ReturnStatus cert_group_create_context(SOPC_CertGroupContext** ppCertGroup);
static void cert_group_initialize_context(SOPC_CertGroupContext* pCertGroup);
static void cert_group_clear_context(SOPC_CertGroupContext* pCertGroup);
static void cert_group_delete_context(SOPC_CertGroupContext** ppCertGroup);
static void cert_group_dict_free_context_value(uintptr_t value);
static SOPC_ReturnStatus cert_group_set_cert_type(SOPC_CertGroupContext* pCertGroup, SOPC_Certificate_Type certType);

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
    cert_group_initialize_context(pCertGroup);
    *ppCertGroup = pCertGroup;
    return SOPC_STATUS_OK;
}

static void cert_group_initialize_context(SOPC_CertGroupContext* pCertGroup)
{
    SOPC_ASSERT(NULL != pCertGroup);

    pCertGroup->pObjectId = NULL;
    pCertGroup->cStrId = NULL;
    pCertGroup->pCertificateTypeId = NULL;
    pCertGroup->pCertificateTypeValueId = NULL;
    pCertGroup->pTrustListId = NULL;
    pCertGroup->pKey = NULL;
    pCertGroup->pCert = NULL;
    pCertGroup->pNewKey = NULL;
    pCertGroup->bDoNotDelete = false;
}

static void cert_group_clear_context(SOPC_CertGroupContext* pCertGroup)
{
    if (NULL == pCertGroup)
    {
        return;
    }
    SOPC_NodeId_Clear(pCertGroup->pObjectId);
    SOPC_NodeId_Clear(pCertGroup->pCertificateTypeId);
    SOPC_NodeId_Clear(pCertGroup->pCertificateTypeValueId);
    SOPC_NodeId_Clear(pCertGroup->pTrustListId);
    SOPC_Free(pCertGroup->pObjectId);
    SOPC_Free(pCertGroup->cStrId);
    SOPC_Free(pCertGroup->pCertificateTypeId);
    SOPC_Free(pCertGroup->pCertificateTypeValueId);
    SOPC_Free(pCertGroup->pTrustListId);
    SOPC_KeyManager_AsymmetricKey_Free(pCertGroup->pNewKey);
    pCertGroup->pNewKey = NULL;
    /* Safely unreference crypto pointer */
    pCertGroup->pKey = NULL;
    pCertGroup->pCert = NULL;
    pCertGroup->bDoNotDelete = true;
}

static void cert_group_delete_context(SOPC_CertGroupContext** ppCertGroup)
{
    SOPC_CertGroupContext* pCertGroup = *ppCertGroup;
    if (pCertGroup->bDoNotDelete)
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

    pCertGroup->pCertificateTypeValueId = SOPC_Calloc(1, sizeof(SOPC_NodeId));
    if (NULL == pCertGroup->pCertificateTypeValueId)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    switch (certType)
    {
    case SOPC_CERT_TYPE_RSA_MIN_APPLICATION:
        pCertGroup->pCertificateTypeValueId->IdentifierType = SOPC_IdentifierType_Numeric;
        pCertGroup->pCertificateTypeValueId->Namespace = 0;
        pCertGroup->pCertificateTypeValueId->Data.Numeric = OpcUaId_RsaMinApplicationCertificateType;
        break;
    case SOPC_CERT_TYPE_RSA_SHA256_APPLICATION:
        pCertGroup->pCertificateTypeValueId->IdentifierType = SOPC_IdentifierType_Numeric;
        pCertGroup->pCertificateTypeValueId->Namespace = 0;
        pCertGroup->pCertificateTypeValueId->Data.Numeric = OpcUaId_RsaSha256ApplicationCertificateType;
        break;
    default:
        status = SOPC_STATUS_INVALID_PARAMETERS;
        break;
    }
    return status;
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
                                                                SOPC_SerializedAsymmetricKey* pKey,
                                                                SOPC_SerializedCertificate* pCert,
                                                                SOPC_CertificateGroup_Config** ppConfig)
{
    if (NULL == pPKI || 0 == maxTrustListSize || NULL == ppConfig)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (SOPC_CERT_TYPE_RSA_MIN_APPLICATION != certType && SOPC_CERT_TYPE_RSA_SHA256_APPLICATION != certType)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    const CertificateGroup_NodeIds* pNodeIds = NULL;
    if (SOPC_TRUSTLIST_GROUP_APP == groupType)
    {
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
    /* key <=> cert */
    if ((NULL == pKey && NULL != pCert) || (NULL != pKey && NULL == pCert))
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    /* key and cert has no meaning for user group */
    if (SOPC_TRUSTLIST_GROUP_USR == groupType && NULL != pKey && NULL != pCert)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (SOPC_TRUSTLIST_GROUP_USR != groupType && (NULL == pKey || NULL == pCert))
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_CertificateGroup_Config* pCfg = SOPC_Calloc(1, sizeof(SOPC_CertificateGroup_Config));
    if (NULL == pCfg)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }
    SOPC_TrustList_Config* pTrustListCfg = NULL;
    SOPC_ReturnStatus status =
        SOPC_TrustList_GetDefaultConfiguration(groupType, pPKI, maxTrustListSize, &pTrustListCfg);
    if (SOPC_STATUS_OK == status)
    {
        pCfg->pIds = pNodeIds;
        pCfg->pTrustListCfg = pTrustListCfg;
        pCfg->pPKI = pPKI;
        pCfg->groupType = groupType;
        pCfg->certType = certType;
        pCfg->pKey = pKey;
        pCfg->pCert = pCert;
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
    /* key <=> cert */
    if ((NULL == pCfg->pKey && NULL != pCfg->pCert) || (NULL != pCfg->pKey && NULL == pCfg->pCert))
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    /* key and cert has no meaning for user group */
    if (SOPC_TRUSTLIST_GROUP_USR == pCfg->groupType && NULL != pCfg->pKey && NULL != pCfg->pCert)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (SOPC_TRUSTLIST_GROUP_USR != pCfg->groupType && (NULL == pCfg->pKey || NULL == pCfg->pCert))
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
        pCertGroup->pObjectId = SOPC_Calloc(1, sizeof(SOPC_NodeId));
        status = SOPC_NodeId_Copy(pCertGroup->pObjectId, pIds->pCertificateGroupId);
    }
    if (SOPC_STATUS_OK == status)
    {
        pCertGroup->pTrustListId = SOPC_Calloc(1, sizeof(SOPC_NodeId));
        status = SOPC_NodeId_Copy(pCertGroup->pTrustListId, pIds->pTrustListId);
    }
    if (SOPC_STATUS_OK == status)
    {
        pCertGroup->cStrId = SOPC_NodeId_ToCString(pIds->pTrustListId);
        if (NULL == pCertGroup->cStrId)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }
    /* Add certificate type */
    if (SOPC_STATUS_OK == status)
    {
        pCertGroup->pCertificateTypeId = SOPC_Calloc(1, sizeof(SOPC_NodeId));
        status = SOPC_NodeId_Copy(pCertGroup->pCertificateTypeId, pIds->pCertificateTypesId);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = cert_group_set_cert_type(pCertGroup, pCfg->certType);
    }
    /* Finally add the certificateGroup to the dictionary */
    if (SOPC_STATUS_OK == status)
    {
        pCertGroup->pKey = pCfg->pKey;
        pCertGroup->pCert = pCfg->pCert;
        bool res = CertificateGroup_DictInsert(pCertGroup->pObjectId, pCertGroup);
        status = !res ? SOPC_STATUS_NOK : SOPC_STATUS_OK;
    }

    if (SOPC_STATUS_OK != status)
    {
        if (NULL != pCertGroup)
        {
            if (NULL != pCertGroup->pTrustListId)
            {
                TrustList_DictRemove(pCertGroup->pTrustListId);
            }
            if (NULL != pCertGroup->pObjectId)
            {
                CertificateGroup_DictRemove(pCertGroup->pObjectId);
            }
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

/* Insert a new objectId key and CertificateGroup context value */
bool CertificateGroup_DictInsert(SOPC_NodeId* pObjectId, SOPC_CertGroupContext* pContext)
{
    if (NULL == gObjIdToCertGroup || NULL == pObjectId || NULL == pContext)
    {
        return false;
    }
    bool res = SOPC_Dict_Insert(gObjIdToCertGroup, (uintptr_t) pObjectId, (uintptr_t) pContext);
    return res;
}

/* Get the CertificateGroup context from the nodeId */
SOPC_CertGroupContext* CertificateGroup_DictGet(const SOPC_NodeId* pObjectId, bool* bFound)
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

/* Removes a CertificateGroup context from the nodeId */
void CertificateGroup_DictRemove(const SOPC_NodeId* pObjectId)
{
    if (NULL == gObjIdToCertGroup || NULL == pObjectId)
    {
        return;
    }
    SOPC_Dict_Remove(gObjIdToCertGroup, (const uintptr_t) pObjectId);
}

const char* CertificateGroup_GetStrNodeId(const SOPC_CertGroupContext* pGroupCtx)
{
    SOPC_ASSERT(NULL != pGroupCtx);
    return (const char*) pGroupCtx->cStrId;
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
    SOPC_UNUSED_ARG(pSubjectName);

    SOPC_Logger_TraceWarning(
        SOPC_LOG_MODULE_CLIENTSERVER,
        "PushSrvCfg:Method_CreateSigningRequest:CertGroup:%s: custom subjectName for CSR is not supported",
        pGroupCtx->cStrId);
    return true;
}

SOPC_ReturnStatus CertificateGroup_RegeneratePrivateKey(SOPC_CertGroupContext* pGroupCtx)
{
    SOPC_ASSERT(NULL != pGroupCtx);
    SOPC_Logger_TraceWarning(
        SOPC_LOG_MODULE_CLIENTSERVER,
        "PushSrvCfg:Method_CreateSigningRequest:CertGroup:%s: regenerate server private key is not supported",
        pGroupCtx->cStrId);
    return SOPC_STATUS_OK;
}

void CertificateGroup_DiscardNewKey(SOPC_CertGroupContext* pGroupCtx)
{
    SOPC_ASSERT(NULL != pGroupCtx);
    SOPC_KeyManager_AsymmetricKey_Free(pGroupCtx->pNewKey);
    pGroupCtx->pNewKey = NULL;
}

SOPC_ReturnStatus CertificateGroup_CreateSigningRequest(SOPC_CertGroupContext* pGroupCtx,
                                                        const SOPC_String* pSubjectName,
                                                        const bool bRegeneratePrivateKey,
                                                        SOPC_ByteString* pCertificateRequest)
{
    SOPC_ASSERT(NULL != pGroupCtx);
    SOPC_UNUSED_ARG(pSubjectName);

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

    status = SOPC_KeyManager_SerializedCertificate_Deserialize(pGroupCtx->pCert, &pCert);
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
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_KeyManager_CSR_Create(pCertSubjectName, true, mdAlg, pURI, "NotSupported", &pNewCSR);
    }
    if (SOPC_STATUS_OK == status)
    {
        if (bRegeneratePrivateKey)
        {
            status = SOPC_KeyManager_AsymmetricKey_GenRSA(keySize, &pNewKey);
        }
        else
        {
            status = SOPC_KeyManager_SerializedAsymmetricKey_Deserialize(pGroupCtx->pKey, false, &pCurKey);
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
    SOPC_Free(mdAlg);
    SOPC_KeyManager_CSR_Free(pNewCSR);
    SOPC_KeyManager_AsymmetricKey_Free(pCurKey);
    pCurKey = NULL;
    if (SOPC_STATUS_OK != status)
    {
        SOPC_KeyManager_AsymmetricKey_Free(pNewKey);
        pNewKey = NULL;
        SOPC_Free(pCSR_DER);
        pCSR_DER = NULL;
        SOPC_ByteString_Initialize(pCertificateRequest);
    }

    pGroupCtx->pNewKey = pNewKey;
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
    SOPC_TrustListContext* pCtx = TrustList_DictGet(pGroupCtx->pTrustListId, false, NULL, &bFound);
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
        pByteStr = &pBsCertArray[i];
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
            if (NULL != pByteStr->Data && NULL != pRawBuffer->data)
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

SOPC_StatusCode CertificateGroup_ExportRejectedList(const SOPC_CertGroupContext* pGroupCtx, const bool bEraseExisting)
{
    SOPC_ASSERT(NULL != pGroupCtx);

    if (NULL == pGroupCtx->pTrustListId)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "CertificateGroup:%s: no TrustList is configured",
                               pGroupCtx->cStrId);
        return OpcUa_BadUnexpectedError;
    }
    bool bFound = false;
    SOPC_TrustListContext* pCtx = TrustList_DictGet(pGroupCtx->pTrustListId, false, NULL, &bFound);
    if (NULL == pCtx || !bFound)
    {
        return OpcUa_BadUnexpectedError;
    }
    SOPC_ASSERT(NULL != pCtx->pPKI);
    SOPC_ReturnStatus status = SOPC_PKIProvider_WriteRejectedCertToStore(pCtx->pPKI, bEraseExisting);
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "CertificateGroup:%s: export of rejected list failed",
                               pGroupCtx->cStrId);
        return OpcUa_BadUnexpectedError;
    }
    else
    {
        return SOPC_GoodGenericStatus;
    }
}
