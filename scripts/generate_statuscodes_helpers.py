#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Licensed to Systerel under one or more contributor license
# agreements. See the NOTICE file distributed with this work
# for additional information regarding copyright ownership.
# Systerel licenses this file to you under the Apache
# License, Version 2.0 (the "License"); you may not use this
# file except in compliance with the License. You may obtain
# a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.

#
#  Generate the ``sopc_statuscode_to_string.[hc]`` files.
#

import re
import sys
import os
import os.path

def parse_status_codes(header_file):
    codes = {}
    with open(header_file, 'r') as f:
        content = f.read()

    # Find all status code definitions using regex - simplified to ignore description
    pattern = r'#define\s+(OpcUa_\w+)\s+(0x[0-9A-Fa-f]+)'
    matches = re.finditer(pattern, content)

    for match in matches:
        name = match.group(1)
        code = match.group(2)
        codes[code] = name

    return codes

C_STATUSCODE_TOSTRING_HEADER='''/*
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

#include "{statuscode_tostring_h}"
#include "{opcua_statuscodes_h}"

#include <stdio.h>

#include "sopc_assert.h"
#include "sopc_helper_string.h"
#include "sopc_mem_alloc.h"
'''

def generate_c_code(codes, opcua_statuscodes_h, statuscode_tostring_h):
    output = []
    output.append(C_STATUSCODE_TOSTRING_HEADER.format(opcua_statuscodes_h=opcua_statuscodes_h,
                                                      statuscode_tostring_h=statuscode_tostring_h))
    output.append('''static const char* SOPC_Internal_StatusCodeToString(SOPC_StatusCode status, bool* found)
{
    SOPC_ASSERT(found != NULL);
    *found = true;
    switch (status)
    {''')

    for name in sorted(codes.values()):
        output.append(f'    case {name}:')
        output.append(f'        return "{name}";')

    output.append('''    case 0x00000000:
        return "OpcUa_Good";
    default:
        *found = false;
        if (SOPC_IsBadStatus(status))
        {
            return "(Unknown) Bad Status Code";
        }
        else if (SOPC_IsUncertainStatus(status))
        {
            return "(Unknown) Uncertain Status Code";
        }
        else if (SOPC_IsGoodStatus(status))
        {
            return "(Unknown) Good Status Code";
        }
        else
        {
            return "Unknown Status Code";
        }
    }
}

const char* SOPC_StatusCodeToString(SOPC_StatusCode status)
{
    bool found = false;
    return SOPC_Internal_StatusCodeToString(status, &found);
}

char* SOPC_StatusCodeToStringAlloc(SOPC_StatusCode status)
{
    bool found = false;
    const char* result = SOPC_Internal_StatusCodeToString(status, &found);
    if (found)
    {
        return SOPC_strdup(result);
    }
    else
    {
        char* statusCode = SOPC_Calloc(11, sizeof(char));
        if (NULL == statusCode)
        {
            return NULL;
        }
        snprintf(statusCode, 11, "0x%08X", (unsigned) status);
        return statusCode;
    }
}
''')

    return '\n'.join(output)

def generate_h_code(statuscode_tostring_h):
    # Extract filename and convert to uppercase guard
    header_guard = os.path.basename(statuscode_tostring_h).upper().replace('.', '_')+'_'

    return f'''/*
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

#ifndef {header_guard}
#define {header_guard}

#include "sopc_builtintypes.h"

/**
 * \\brief Convert a status code to its string representation
 *
 * \\param status  The status code to convert
 * \\return A const string pointer containing the name of the status code constant,
 *         or a generic message if the status code is not recognized.
 *         The returned string is valid for the lifetime of the program.
 */
const char* SOPC_StatusCodeToString(SOPC_StatusCode status);

/**
 * \\brief Convert a status code to its string representation (allocating memory)
 *
 * \\param status  The status code to convert
 * \\return An allocated string containing the name of the status code constant,
 *         or the hexadecimal representation if the status code is not recognized.
 *         The caller is responsible for freeing the returned string with SOPC_Free().
 *         Returns NULL if memory allocation fails.
 */
char* SOPC_StatusCodeToStringAlloc(SOPC_StatusCode status);

#endif /* {header_guard} */
'''

def main():
    if len(sys.argv) != 1 and len(sys.argv) != 4:
        print(f"Usage: {sys.argv[0]} [opcua_statuscodes.h output.c output.h]")
        print(f"Default usage: {sys.argv[0]}")
        sys.exit(1)

    if len(sys.argv) == 4:
        opcua_statuscodes = sys.argv[1]
        statuscode_tostring_c = sys.argv[2]
        statuscode_tostring_h = sys.argv[3]
    else:
        opcua_statuscodes = "src/Common/opcua_types/opcua_statuscodes.h"
        statuscode_tostring_c = "src/Common/opcua_types/sopc_helper_statuscodes.c"
        statuscode_tostring_h = "src/Common/opcua_types/sopc_helper_statuscodes.h"

    codes = parse_status_codes(opcua_statuscodes)

    with open(statuscode_tostring_c, 'w') as f:
        # Get just the filenames for the includes
        opcua_header = os.path.basename(opcua_statuscodes)
        statuscode_header = os.path.basename(statuscode_tostring_h)
        f.write(generate_c_code(codes, opcua_header, statuscode_header))

    with open(statuscode_tostring_h, 'w') as f:
        f.write(generate_h_code(statuscode_tostring_h))

    print(f"Generated {statuscode_tostring_c} and {statuscode_tostring_h}")

if __name__ == "__main__":
    main()
