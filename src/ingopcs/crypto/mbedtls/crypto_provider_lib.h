/** \file
 *
 * Defines the library specific structure CryptolibContext.
 *
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

#ifndef SOPC_CRYPTO_PROVIDER_LIB_H_
#define SOPC_CRYPTO_PROVIDER_LIB_H_

#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"


typedef struct CryptolibContext {
    mbedtls_entropy_context ctxEnt;
    mbedtls_ctr_drbg_context ctxDrbg;
} CryptolibContext;


#endif /* SOPC_CRYPTO_PROVIDER_LIB_H_ */
