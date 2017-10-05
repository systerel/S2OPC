/*
 *  Copyright (C) 2017 Systerel and others.
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
 * A main that implements the init of a ReadRequest,
 *  and the call to the machine that should treat it.
 */


#include <stdlib.h>

#include "toolkit_header_init.h"
#include "address_space_bs.h"

#include "wrap_read.h"
#include "add.h"
#include "sopc_toolkit_config_internal.h"

int main()
{
    bool bTest;

    /* Set address space */
    SOPC_Internal_ToolkitServer_SetAddressSpaceConfig(&addressSpace);

    /* Init */
    INITIALISATION();

    /* Creates a ReadRequest */
    OpcUa_ReadRequest *pReadReq = read_new_read_request();

    /* Main service test */
    bTest = read_service_test(pReadReq);

    /* Uninit the address space */
    address_space_bs__UNINITIALISATION();

    /* Free the request */
    free(pReadReq->NodesToRead);
    free(pReadReq);

    return false == bTest;
}
