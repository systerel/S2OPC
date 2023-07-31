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
 * \brief Internal API to manage the TrustListType according the Push model.
 */

#ifndef SOPC_TRUSTLIST_INTERNAL_
#define SOPC_TRUSTLIST_INTERNAL_

#include "sopc_buffer.h"
#include "sopc_trustlist_itf.h"

#define INVALID_HANDLE_VALUE 0u

typedef uint32_t SOPC_TrLst_Handle;

typedef enum
{
    SOPC_TL_OPEN_MODE_UNKNOWN = 0x00,
    SOPC_TL_OPEN_MODE_READ = 0x01,
    SOPC_TL_OPEN_MODE_WRITE_ERASE_EXISTING = 0x06,
} SOPC_TrLst_OpenMode;

typedef enum
{
    SOPC_TL_MASK_NONE = 0,
    SOPC_TL_MASK_TRUSTED_CERTS = 1,
    SOPC_TL_MASK_TRUSTED_CRLS = 2,
    SOPC_TL_MASK_ISSUER_CERTS = 4,
    SOPC_TL_MASK_ISSUER_CRLS = 8,
    SOPC_TL_MASK_ALL = 15,
} SOPC_TrLst_Mask;

typedef struct SOPC_TrLst_VarCfg
{
    SOPC_NodeId* pSizeId;
    SOPC_NodeId* pWritableId;
    SOPC_NodeId* pUserWritableId;
    SOPC_NodeId* pOpenCountId;
} SOPC_TrLst_VarCfg;

typedef struct SOPC_TrustList
{
    bool bIsOpen;
    SOPC_TrLst_Handle handle;
    SOPC_TrLst_OpenMode openingMode;
    SOPC_TrLst_Mask openingMask;
    SOPC_TrustList_Type groupType;
    uint16_t openCount;
    uint64_t size;
    SOPC_TrLst_VarCfg varIds;
    SOPC_Buffer* pTrustListEncoded;
    SOPC_PKIProvider* pPKI;
    SOPC_CertificateList* pTrustedCerts;
    SOPC_CertificateList* pIssuerCerts;
    SOPC_CRLList* pTrustedCRLs;
    SOPC_CRLList* pIssuerCRLs;
} SOPC_TrustList;

/* Get the trustList internal object from the nodeId */
SOPC_TrustList* TrustList_DictGet(const SOPC_NodeId* objectId, bool* found);
/* Generate a random handle */
SOPC_TrLst_Handle TrustList_GenRandHandle(void);
/* Read the PKI and encode the trustList in a UA Binary encoded stream containing an instance of TrustListDataType */
SOPC_ReturnStatus TrustList_Encode(const SOPC_PKIProvider* pPKI,
                                   const SOPC_TrLst_Mask specifiedLists,
                                   SOPC_Buffer* pTrustListDataType);
/* Decode the trustList UA Binary stream to a TrustListDataType */
SOPC_ReturnStatus TrustList_Decode(const SOPC_Buffer* pTrustListDataTypeEncoded, void* pTrustListDataType);

#endif /* SOPC_TRUSTLIST_INTERNAL_ */
