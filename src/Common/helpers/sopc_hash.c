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

#include "sopc_hash.h"

uint64_t SOPC_DJBHash(const uint8_t* data, size_t len)
{
    return SOPC_DJBHash_Step(5381, data, len);
}

uint64_t SOPC_DJBHash_Step(uint64_t current, const uint8_t* data, size_t len)
{
    for (size_t i = 0; i < len; ++i)
    {
        current = (current << 5) + current + data[i];
    }

    return current;
}

/* Constants of the 64-bit FNV-1a hash, as specified by its authors: offset basis
 * 0xcbf29ce484222325 and prime 0x100000001b3. */
#define FNV1A_OFFSET_BASIS 14695981039346656037ULL
#define FNV1A_PRIME 1099511628211ULL

uint64_t SOPC_FNV1aHash(const uint8_t* data, size_t len)
{
    return SOPC_FNV1aHash_Step(FNV1A_OFFSET_BASIS, data, len);
}

uint64_t SOPC_FNV1aHash_Step(uint64_t current, const uint8_t* data, size_t len)
{
    for (size_t i = 0; i < len; ++i)
    {
        // FNV-1a: xor before multiply (FNV-1 does the opposite and diffuses less)
        current ^= data[i];
        current *= FNV1A_PRIME;
    }

    return current;
}
