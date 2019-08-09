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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wrap_read.h"

#include "b2c.h"

/*#include "header.h"*/
#include "address_space.h"
#include "address_space_bs.h"
#include "address_space_impl.h"
#include "constants.h"
#include "constants_bs.h"
#include "io_dispatch_mgr.h"
#include "msg_read_request.h"
#include "msg_read_request_bs.h"
#include "sopc_mem_alloc.h"
#include "testlib_read_response.h"

/* http://stackoverflow.com/questions/7265583/combine-designated-initializers-and-malloc-in-c99 */
#define DESIGNATE_NEW(T, ...) memcpy(SOPC_Malloc(sizeof(T)), &(T const){__VA_ARGS__}, sizeof(T))

/**
 * Creates a request of length N_REQUEST
 */
OpcUa_ReadRequest* read_new_read_request(void)
{
    const uint32_t N_REQUESTS = 1; // Read 1 attributes on one node

    OpcUa_ReadValueId* lrv = SOPC_Calloc(N_REQUESTS, sizeof(OpcUa_ReadValueId));

    if (NULL == lrv)
        exit(1);

    /* Request for the Value */
    lrv[0] = (OpcUa_ReadValueId){.encodeableType = &OpcUa_ReadValueId_EncodeableType,
                                 .NodeId = {.IdentifierType = SOPC_IdentifierType_String,
                                            .Data.String = {sizeof("PubBool") - 1, 1, (SOPC_Byte*) "PubBool"},
                                            .Namespace = 1},
                                 .AttributeId = constants__e_aid_Value,
                                 .IndexRange = {.Length = 0},
                                 .DataEncoding = {.Name.Length = 0}};

    OpcUa_ReadRequest* pReadReq = DESIGNATE_NEW(OpcUa_ReadRequest, .encodeableType = &OpcUa_ReadRequest_EncodeableType,
                                                .MaxAge = 0., .TimestampsToReturn = OpcUa_TimestampsToReturn_Neither,
                                                .NoOfNodesToRead = (int32_t) N_REQUESTS, .NodesToRead = lrv);
    if (NULL == pReadReq)
        exit(1);

    return pReadReq;
}
