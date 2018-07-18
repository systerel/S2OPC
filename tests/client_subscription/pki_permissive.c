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

#include <stdlib.h>

#include "pki_permissive.h"

#include "sopc_macros.h"

static SOPC_ReturnStatus PKIPermissive_ValidateAnything(const SOPC_PKIProvider* pPKI,
                                                        const SOPC_Certificate* pToValidate)
{
    (void) (pPKI);
    (void) (pToValidate);
    return SOPC_STATUS_OK;
}

static void PKIPermissive_Free(SOPC_PKIProvider* pPKI)
{
    free(pPKI);
}

SOPC_ReturnStatus SOPC_PKIPermissive_Create(SOPC_PKIProvider** ppPKI)
{
    SOPC_PKIProvider* pki = NULL;

    if (NULL == ppPKI)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    pki = (SOPC_PKIProvider*) malloc(sizeof(SOPC_PKIProvider));

    if (NULL == pki)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    // The pki function pointer shall be const after this init
    SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
    *(SOPC_PKIProvider_Free_Func*) (&pki->pFnFree) = &PKIPermissive_Free;
    *(SOPC_FnValidateCertificate*) (&pki->pFnValidateCertificate) = &PKIPermissive_ValidateAnything;
    SOPC_GCC_DIAGNOSTIC_RESTORE

    pki->pUserCertAuthList = NULL;
    pki->pUserCertRevocList = NULL; // Can be NULL
    pki->pUserData = NULL;
    *ppPKI = pki;

    return SOPC_STATUS_OK;
}
