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

const uint32_t N_READ_NODES = 7; // Read nodes with Node Id 1000 to 1000 + N_READ_NODES only
const uint32_t N_READ_VARS = 6;  // Read variables values with Node Id 1001 to 1001 + N_READ_VARS only

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

    //    /* All nodes Ids and classes */
    //    for (i = 0; i < N_READ_NODES; ++i)
    //    {
    //        /* Request for the NodeId (...) */
    //        lrv[2 * i + 0] = (OpcUa_ReadValueId){
    //            .encodeableType = &OpcUa_ReadValueId_EncodeableType,
    //            .NodeId = {.IdentifierType = SOPC_IdentifierType_Numeric, .Data.Numeric = i + 1000, .Namespace = 1},
    //            .AttributeId = constants__e_aid_NodeId,
    //            .IndexRange = {.Length = 0},
    //            .DataEncoding = {.Name.Length = 0}};
    //        /* Request for the NodeClass */
    //        lrv[2 * i + 1] = (OpcUa_ReadValueId){
    //            .encodeableType = &OpcUa_ReadValueId_EncodeableType,
    //            .NodeId = {.IdentifierType = SOPC_IdentifierType_Numeric, .Data.Numeric = i + 1000, .Namespace = 1},
    //            .AttributeId = constants__e_aid_NodeClass,
    //            .IndexRange = {.Length = 0},
    //            .DataEncoding = {.Name.Length = 0}};
    //    }

    //    /* Note: variables have the last numeric node ids in the @space used for this test*/
    //    /* All variables */
    //    for (i = 0; i < N_READ_VARS; ++i)
    //    {
    //        /* Request for the Value */
    //        lrv[2 * N_READ_NODES + i] = (OpcUa_ReadValueId){.encodeableType = &OpcUa_ReadValueId_EncodeableType,
    //                                                        .NodeId = {.IdentifierType = SOPC_IdentifierType_Numeric,
    //                                                                   .Data.Numeric = (N_READ_NODES - 1 - i) + 1000,
    //                                                                   .Namespace = 1},
    //                                                        .AttributeId = constants__e_aid_Value,
    //                                                        .IndexRange = {.Length = 0},
    //                                                        .DataEncoding = {.Name.Length = 0}};
    //    }

    OpcUa_ReadRequest* pReadReq = DESIGNATE_NEW(OpcUa_ReadRequest, .encodeableType = &OpcUa_ReadRequest_EncodeableType,
                                                .MaxAge = 0., .TimestampsToReturn = OpcUa_TimestampsToReturn_Neither,
                                                .NoOfNodesToRead = (int32_t) N_REQUESTS, .NodesToRead = lrv);
    if (NULL == pReadReq)
        exit(1);

    return pReadReq;
}
