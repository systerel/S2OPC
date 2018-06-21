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

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "sopc_helper_uri.h"

int LLVMFuzzerTestOneInput(const uint8_t* buf, size_t len)
{
    char* buf_copy = (char*) calloc(1 + len, sizeof(char));
    assert(buf_copy != NULL);

    memcpy(buf_copy, buf, len);

    size_t hostname_len;
    size_t port_idx;
    size_t port_len;
    SOPC_Helper_URI_ParseTcpUaUri(buf_copy, &hostname_len, &port_idx, &port_len);

    free(buf_copy);

    return 0;
}
