/** \file
 *
 * KeyManager provides functions for Asymmetric Key Management such as loading a signed public key,
 *  the corresponding private key, and provides the ability to verify signatures with x509 certificates.
 * KeyManager replaces the old concept of PKIProvider. PrivateKey should not be in the PublicKeyInfrastructure...
 *
 * Most of the functions are lib-dependent. This file defines the others.
 */
/*
 *  Copyright (C) 2016 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <stdlib.h>
#include <string.h>

#include "sopc_base_types.h"
#include "crypto_types.h"
#include "crypto_profiles.h"
#include "key_manager.h"




/* ------------------------------------------------------------------------------------------------
 * AsymetricKey API
 * ------------------------------------------------------------------------------------------------
 */

/* ------------------------------------------------------------------------------------------------
 * Cert API
 * ------------------------------------------------------------------------------------------------
 */
SOPC_StatusCode KeyManager_Certificate_CopyDER(const Certificate *pCert,
                                          uint8_t **ppDest, uint32_t *pLenAllocated)
{
    uint32_t lenToAllocate = 0;

    if(NULL == pCert || ppDest == NULL || 0 == pLenAllocated)
        return STATUS_INVALID_PARAMETERS;

    // Allocation
    lenToAllocate = pCert->len_der;
    if(lenToAllocate == 0)
        return STATUS_NOK;

    (*ppDest) = (uint8_t *)malloc(lenToAllocate);
    if(NULL == *ppDest)
        return STATUS_NOK;

    // Copy
    memcpy((void *)(*ppDest), (void *)(pCert->crt_der), lenToAllocate);
    *pLenAllocated = lenToAllocate;

    return STATUS_OK;
}




