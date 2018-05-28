/*
 *  Copyright (C) 2018 Systerel and others.
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
