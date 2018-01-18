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

/** \file
 *
 * An offline test for a WriteRequest. Sends as WriteRequest to the B model
 *  as if it was from a remote client, tests the address space for effects.
 */

#include <stdio.h>
#include <stdlib.h>

#include "address_space_bs.h"
#include "sopc_addspace.h"
#include "sopc_toolkit_config_internal.h"
#include "testlib_write.h"
#include "toolkit_header_init.h"

int main(void)
{
    bool bTest;
    int retCode;

    /* This test is offline, local, "server-side". */

    /* Set address space */
    SOPC_Internal_ToolkitServer_SetAddressSpaceConfig(&addressSpace);

    /* Init */
    INITIALISATION();

    /* Creates a WriteRequest and the message */
    OpcUa_WriteRequest* pWriteReq = tlibw_new_WriteRequest();
    if (NULL == pWriteReq)
        exit(1);

    /* Main service test.
     * The Write differs from the Read: the response cache will be checked instead of the OpcUa_*Response,
     *  as the B model does it in two steps.
     */
    tlibw_stimulateB_with_message(pWriteReq);
    bTest = tlibw_verify_effects_local(pWriteReq);

    /* Uninit the address space */
    address_space_bs__UNINITIALISATION();

    /* Free the request */
    tlibw_free_WriteRequest(&pWriteReq);

    if (bTest)
    {
        printf("Internal test: OK\n");
        retCode = 0;
    }
    else
    {
        printf("Internal test: NOT ok\n");
        retCode = 1;
    }

    return retCode;
}
