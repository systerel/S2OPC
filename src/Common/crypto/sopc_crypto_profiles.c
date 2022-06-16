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

#include <string.h>

#include "sopc_crypto_decl.h"
#include "sopc_crypto_profiles.h"
#include "sopc_helper_string.h"

const SOPC_CryptoProfile* SOPC_CryptoProfile_Get(const char* uri)
{
    if (NULL == uri)
    {
        return NULL;
    }

    /* Compares len+1 to include the trailing \0 of the zero-terminated #defined URI.
     * This avoids false positives with strings prefixed by a valid security policy. */
    if (SOPC_strncmp_ignore_case(uri, SOPC_SecurityPolicy_Aes256Sha256RsaPss_URI,
                                 strlen(SOPC_SecurityPolicy_Aes256Sha256RsaPss_URI) + 1) == 0)
    {
        return &sopc_g_cpAes256Sha256RsaPss;
    }
    if (SOPC_strncmp_ignore_case(uri, SOPC_SecurityPolicy_Aes128Sha256RsaOaep_URI,
                                 strlen(SOPC_SecurityPolicy_Aes128Sha256RsaOaep_URI) + 1) == 0)
    {
        return &sopc_g_cpAes128Sha256RsaOaep;
    }
    if (SOPC_strncmp_ignore_case(uri, SOPC_SecurityPolicy_Basic256Sha256_URI,
                                 strlen(SOPC_SecurityPolicy_Basic256Sha256_URI) + 1) == 0)
    {
        return &sopc_g_cpBasic256Sha256;
    }
    if (SOPC_strncmp_ignore_case(uri, SOPC_SecurityPolicy_Basic256_URI, strlen(SOPC_SecurityPolicy_Basic256_URI) + 1) ==
        0)
    {
        return &sopc_g_cpBasic256;
    }
    if (SOPC_strncmp_ignore_case(uri, SOPC_SecurityPolicy_None_URI, strlen(SOPC_SecurityPolicy_None_URI) + 1) == 0)
    {
        return &sopc_g_cpNone;
    }

    return NULL;
}

const SOPC_CryptoProfile_PubSub* SOPC_CryptoProfile_PubSub_Get(const char* uri)
{
    if (NULL == uri)
    {
        return NULL;
    }

    /* Compares len+1 to include the trailing \0 of the zero-terminated #defined URI.
     * This avoids false positives with strings prefixed by a valid security policy. */
    if (SOPC_strncmp_ignore_case(uri, SOPC_SecurityPolicy_PubSub_Aes256_URI,
                                 strlen(SOPC_SecurityPolicy_PubSub_Aes256_URI) + 1) == 0)
    {
        return &sopc_g_cppsPubSubAes256;
    }
    if (SOPC_strncmp_ignore_case(uri, SOPC_SecurityPolicy_None_URI, strlen(SOPC_SecurityPolicy_None_URI) + 1) == 0)
    {
        return &sopc_g_cppsNone;
    }

    return NULL;
}
