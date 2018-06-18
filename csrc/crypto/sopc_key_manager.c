/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
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

/** \file sopc_key_manager.c
 *
 * KeyManager provides functions for Asymmetric Key Management such as loading a signed public key,
 *  the corresponding private key, and provides the ability to verify signatures with x509 certificates.
 * KeyManager replaces the old concept of PKIProvider. PrivateKey should not be in the PublicKeyInfrastructure...
 *
 * Most of the functions are lib-dependent. This file defines the others.
 */

#include <stdlib.h>
#include <string.h>

#include "key_manager_lib.h"
#include "sopc_key_manager.h"

#include "sopc_crypto_decl.h"
#include "sopc_crypto_profiles.h"

/* ------------------------------------------------------------------------------------------------
 * AsymetricKey API
 * ------------------------------------------------------------------------------------------------
 */

/* ------------------------------------------------------------------------------------------------
 * Cert API
 * ------------------------------------------------------------------------------------------------
 */
SOPC_ReturnStatus SOPC_KeyManager_Certificate_CopyDER(const SOPC_Certificate* pCert,
                                                      uint8_t** ppDest,
                                                      uint32_t* pLenAllocated)
{
    uint32_t lenToAllocate = 0;

    if (NULL == pCert || NULL == ppDest || 0 == pLenAllocated)
        return SOPC_STATUS_INVALID_PARAMETERS;

    // Allocation
    lenToAllocate = pCert->len_der;
    if (lenToAllocate == 0)
        return SOPC_STATUS_NOK;

    (*ppDest) = (uint8_t*) malloc(lenToAllocate);
    if (NULL == *ppDest)
        return SOPC_STATUS_NOK;

    // Copy
    memcpy((void*) (*ppDest), (void*) (pCert->crt_der), lenToAllocate);
    *pLenAllocated = lenToAllocate;

    return SOPC_STATUS_OK;
}
