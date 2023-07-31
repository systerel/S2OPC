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

#include "sopc_certificate_group_internal.h"
#include "sopc_certificate_group_itf.h"

/*---------------------------------------------------------------------------
 *                             Constants
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
 *                             Internal types
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
 *                             Global variables
 *---------------------------------------------------------------------------*/

static SOPC_Dict* gObjIdToCertGroup = NULL;

SOPC_CertificateGroup_Config gCertGroup_DefaultAddSpace_App = {
    .certificateGroupNodeId = "ns=0;i=14156",
    .varCertificateTypesNodeId = "ns=0;i=14161",
    .pTrustListCfg = NULL,
    .certType = SOPC_CERT_TYPE_UNKNOW,
};

SOPC_CertificateGroup_Config gCertGroup_DefaultAddSpace_Usr = {
    .certificateGroupNodeId = "ns=0;i=14122",
    .varCertificateTypesNodeId = "ns=0;i=14155",
    .pTrustListCfg = NULL,
    .certType = SOPC_CERT_TYPE_UNKNOW,
};

/*---------------------------------------------------------------------------
 *                      Prototype of static functions
 *---------------------------------------------------------------------------*/

static SOPC_ReturnStatus cert_group_create(SOPC_CertificateGroup** ppCertGroup);
static void cert_group_initialize(SOPC_CertificateGroup* pCertGroup);
static void cert_group_clear(SOPC_CertificateGroup* pCertGroup);
static void cert_group_delete(SOPC_CertificateGroup** ppCertGroup);
static void cert_group_free(uintptr_t value);
static SOPC_ReturnStatus cert_group_set_cert_type(SOPC_CertificateGroup* pCertGroup, SOPC_Certificate_Type certType);

/*---------------------------------------------------------------------------
 *                       Static functions (implementation)
 *---------------------------------------------------------------------------*/

static SOPC_ReturnStatus cert_group_create(SOPC_CertificateGroup** ppCertGroup)
{
    if (NULL == ppCertGroup)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_CertificateGroup* pCertGroup = NULL;
    pCertGroup = SOPC_Calloc(1, sizeof(SOPC_CertificateGroup));
    if (NULL == pCertGroup)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }
    cert_group_initialize(pCertGroup);
    *ppCertGroup = pCertGroup;
    return SOPC_STATUS_OK;
}

static void cert_group_initialize(SOPC_CertificateGroup* pCertGroup)
{
    SOPC_ASSERT(NULL != pCertGroup);

    pCertGroup->pCertificateTypeId = NULL;
    pCertGroup->pCertificateTypeValueId = NULL;
    pCertGroup->pTrustListId = NULL;
    pCertGroup->pKey = NULL;
    pCertGroup->pCert = NULL;
    pCertGroup->pPKI = NULL;
}

static void cert_group_clear(SOPC_CertificateGroup* pCertGroup)
{
    if (NULL == pCertGroup)
    {
        return;
    }
    SOPC_NodeId_Clear(pCertGroup->pCertificateTypeId);
    SOPC_NodeId_Clear(pCertGroup->pCertificateTypeValueId);
    SOPC_NodeId_Clear(pCertGroup->pTrustListId);
    SOPC_Free(pCertGroup->pCertificateTypeId);
    SOPC_Free(pCertGroup->pCertificateTypeValueId);
    SOPC_Free(pCertGroup->pTrustListId);
    /* Safely unreference crypto pointer */
    pCertGroup->pKey = NULL;
    pCertGroup->pCert = NULL;
    pCertGroup->pPKI = NULL;
}

static void cert_group_delete(SOPC_CertificateGroup** ppCertGroup)
{
    cert_group_clear(*ppCertGroup);
    SOPC_Free(*ppCertGroup);
    *ppCertGroup = NULL;
}

static void cert_group_free(uintptr_t value)
{
    if (NULL != (void*) value)
    {
        cert_group_delete((SOPC_CertificateGroup**) &value);
    }
}

static SOPC_ReturnStatus cert_group_set_cert_type(SOPC_CertificateGroup* pCertGroup, SOPC_Certificate_Type certType)
{
    SOPC_ASSERT(NULL != pCertGroup);

    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    switch (certType)
    {
    case SOPC_CERT_TYPE_RSA_MIN_APPLICATION:
        pCertGroup->pCertificateTypeValueId = SOPC_NodeId_FromCString(
            SOPC_RsaMinApplicationCertificateTypeId, (int32_t) strlen(SOPC_RsaMinApplicationCertificateTypeId));
        status = NULL == pCertGroup->pCertificateTypeValueId ? SOPC_STATUS_OUT_OF_MEMORY : SOPC_STATUS_OK;
        break;
    case SOPC_CERT_TYPE_RSA_SHA256_APPLICATION:
        pCertGroup->pCertificateTypeValueId = SOPC_NodeId_FromCString(
            SOPC_RsaSha256ApplicationCertificateTypeId, (int32_t) strlen(SOPC_RsaSha256ApplicationCertificateTypeId));
        status = NULL == pCertGroup->pCertificateTypeValueId ? SOPC_STATUS_OUT_OF_MEMORY : SOPC_STATUS_OK;
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

    gObjIdToCertGroup = SOPC_NodeId_Dict_Create(true, cert_group_free);
    if (NULL == gObjIdToCertGroup)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }
    return SOPC_STATUS_OK;
}

const SOPC_CertificateGroup_Config* SOPC_CertificateGroup_GetDefaultConfiguration(const SOPC_TrustList_Type groupType,
                                                                                  const SOPC_Certificate_Type certType,
                                                                                  SOPC_PKIProvider* pPKI)
{
    bool bFound = false;
    SOPC_CertificateGroup_Config* pCfg = NULL;
    const SOPC_TrustList_Config* pTrustListCfg = SOPC_TrustList_GetDefaultConfiguration(groupType, pPKI);
    if (NULL == pTrustListCfg)
    {
        return NULL;
    }
    switch (groupType)
    {
    case SOPC_TRUSTLIST_GROUP_APP:
        pCfg = &gCertGroup_DefaultAddSpace_App;
        bFound = true;
        break;
    case SOPC_TRUSTLIST_GROUP_USR:
        pCfg = &gCertGroup_DefaultAddSpace_Usr;
        bFound = true;
        break;
    default:
        break;
    }
    if (bFound)
    {
        pCfg->certType = certType;
        pCfg->pTrustListCfg = pTrustListCfg;
    }
    return (const SOPC_CertificateGroup_Config*) pCfg;
}

SOPC_ReturnStatus SOPC_CertificateGroup_Configure(const SOPC_CertificateGroup_Config* pCfg,
                                                  SOPC_SerializedAsymmetricKey* pKey,
                                                  SOPC_SerializedCertificate* pCert,
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
    if (NULL == pCfg->pTrustListCfg || NULL == pCfg->certificateGroupNodeId)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (NULL == pCfg->pTrustListCfg->pPKI)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    /* key <=> cert */
    if ((NULL == pKey && NULL != pCert) || (NULL != pKey && NULL == pCert))
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    /* key and cert has no meaning for user group */
    if (SOPC_TRUSTLIST_GROUP_USR == pCfg->pTrustListCfg->groupType && NULL != pKey && NULL != pCert)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    /* Configure the TrustList belongs this group */
    SOPC_ReturnStatus status = SOPC_TrustList_Configure(pCfg->pTrustListCfg, pMcm);
    if (SOPC_STATUS_OK != status)
    {
        return status;
    }
    /* Configure the CertificateGroup */
    SOPC_CertificateGroup* pCertGroup = NULL;
    SOPC_NodeId* pObjId = NULL;
    SOPC_NodeId* pTrustListId = NULL;
    if (SOPC_STATUS_OK == status)
    {
        status = cert_group_create(&pCertGroup);
    }
    if (SOPC_STATUS_OK == status)
    {
        pObjId = SOPC_NodeId_FromCString(pCfg->certificateGroupNodeId, (int32_t) strlen(pCfg->certificateGroupNodeId));
        status = NULL == pObjId ? SOPC_STATUS_OUT_OF_MEMORY : SOPC_STATUS_OK;
    }
    if (SOPC_STATUS_OK == status)
    {
        pTrustListId = SOPC_NodeId_FromCString(pCfg->pTrustListCfg->trustListNodeId,
                                               (int32_t) strlen(pCfg->pTrustListCfg->trustListNodeId));
        status = NULL == pTrustListId ? SOPC_STATUS_OUT_OF_MEMORY : SOPC_STATUS_OK;
    }
    /* Add certificate type */
    if (SOPC_STATUS_OK == status)
    {
        status = cert_group_set_cert_type(pCertGroup, pCfg->certType);
    }

    /* Finally add the certificateGroup to the dictionary */
    if (SOPC_STATUS_OK == status)
    {
        pCertGroup->pKey = pKey;
        pCertGroup->pCert = pCert;
        pCertGroup->pPKI = pCfg->pTrustListCfg->pPKI;
        pCertGroup->pTrustListId = pTrustListId;
        bool res = SOPC_Dict_Insert(gObjIdToCertGroup, (uintptr_t) pObjId, (uintptr_t) pCertGroup);
        status = !res ? SOPC_STATUS_NOK : SOPC_STATUS_OK;
    }

    if (SOPC_STATUS_OK != status)
    {
        SOPC_CertificateGroup_Clear();
        cert_group_delete(&pCertGroup);
        SOPC_NodeId_Clear(pObjId);
        SOPC_NodeId_Clear(pTrustListId);
        SOPC_Free(pObjId);
        SOPC_Free(pTrustListId);
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

/* Get the trustList internal object from the nodeId */
SOPC_CertificateGroup* CertificateGroup_DictGet(const SOPC_NodeId* objectId, bool* found)
{
    return (SOPC_CertificateGroup*) SOPC_Dict_Get(gObjIdToCertGroup, (const uintptr_t) objectId, found);
}
