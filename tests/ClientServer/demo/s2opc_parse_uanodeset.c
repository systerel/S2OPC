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
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "xml_expat/sopc_uanodeset_loader.h"

static void usage(char** argv)
{
    printf(
        "Usage: %s XML_FILE\n\n"
        "Parses an XML UANodeSet into an address space description.\n",
        argv[0]);
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        usage(argv);
        return 1;
    }

    const char* xml_filename = argv[1];

    FILE* fd = fopen(xml_filename, "r");

    if (fd == NULL)
    {
        fprintf(stderr, "Error while opening %s: %s\n", xml_filename, strerror(errno));
        return 1;
    }

    SOPC_AddressSpace* space = SOPC_UANodeSet_Parse(fd);
    bool ok = (space != NULL);
    SOPC_AddressSpace_Delete(space);
    fclose(fd);

    if (ok)
    {
        printf("All nodes parsed successfully.\n");
    }

    return ok ? 0 : 1;
}
