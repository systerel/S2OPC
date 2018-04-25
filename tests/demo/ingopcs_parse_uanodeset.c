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
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <uanodeset_expat/loader.h>

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

    SOPC_AddressSpace_Description* desc = SOPC_AddressSpace_Description_Create();
    assert(desc != NULL);

    SOPC_ReturnStatus status = SOPC_UANodeSet_Parse(fd, desc);
    fclose(fd);

    if (status == SOPC_STATUS_OK)
    {
        SOPC_AddressSpace space;
        status = SOPC_AddressSpace_Generate(desc, &space);
        SOPC_AddressSpace_Clear(&space);
    }

    SOPC_AddressSpace_Description_Delete(desc);

    if (status == SOPC_STATUS_OK)
    {
        printf("All nodes parsed successfully.\n");
    }

    return (status == SOPC_STATUS_OK) ? 0 : 1;
}
