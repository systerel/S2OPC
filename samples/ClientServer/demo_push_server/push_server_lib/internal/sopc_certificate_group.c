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
#include "sopc_certificate_group.h"
#include "sopc_certificate_group_itf.h"

/*---------------------------------------------------------------------------
 *                             Constants
 *---------------------------------------------------------------------------*/

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
    pCertGroup->pCertificateTypeId = NULL;
    pCertGroup->pCertificateTypeValueId = NULL;
    pCertGroup->pTrustListId = NULL;
    pCertGroup->pKey = NULL;
    pCertGroup->pCert = NULL;
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
    SOPC_Free(pCertGroup->pCertificateTypeId);
    SOPC_Free(pCertGroup->pCertificateTypeValueId);
    SOPC_Free(pCertGroup->pTrustListId);
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
                                                                const size_t maxTrustListSize,
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
}

/* Get the CertificateGroup context from the nodeId */
SOPC_CertGroupContext* CertificateGroup_DictGet(const SOPC_NodeId* pObjectId, bool* found)
{
    if (NULL == gObjIdToCertGroup || NULL == pObjectId)
    {
        *found = false;
        return NULL;
    }
    SOPC_CertGroupContext* pCtx = NULL;
    pCtx = (SOPC_CertGroupContext*) SOPC_Dict_Get(gObjIdToCertGroup, (const uintptr_t) pObjectId, found);
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
