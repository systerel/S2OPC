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

#include <string.h>

#include "sopc_crypto_decl.h"
#include "sopc_crypto_profiles.h"
#include "sopc_helper_string.h"

const SOPC_CryptoProfile* SOPC_CryptoProfile_Get(const char* uri)
{
    if (NULL != uri)
    {
        // Compares len+1 to include the trailing \0 of the zero-terminated #defined URI.
        // This avoids false positives with strings prefixed by a valid security policy.
        if (SOPC_strncmp_ignore_case(uri, SOPC_SecurityPolicy_Basic256Sha256_URI,
                                     strlen(SOPC_SecurityPolicy_Basic256Sha256_URI) + 1) == 0)
            return &sopc_g_cpBasic256Sha256;
        if (SOPC_strncmp_ignore_case(uri, SOPC_SecurityPolicy_Basic256_URI,
                                     strlen(SOPC_SecurityPolicy_Basic256_URI) + 1) == 0)
            return &sopc_g_cpBasic256;
        if (SOPC_strncmp_ignore_case(uri, SOPC_SecurityPolicy_None_URI, strlen(SOPC_SecurityPolicy_None_URI) + 1) == 0)
            return &sopc_g_cpNone;
    }

    return NULL;
}
