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

/** \file
 *
 * Implementations of the tests details for the WriteRequest.
 */

#include <assert.h>
#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "address_space.h"
#include "address_space_bs.h"
#include "address_space_impl.h"
#include "io_dispatch_mgr.h"
#include "sopc_mem_alloc.h"
#include "testlib_write.h"
#include "util_b2c.h"
#include "util_variant.h"

/* http://stackoverflow.com/questions/7265583/combine-designated-initializers-and-malloc-in-c99 */
#define DESIGNATE_NEW(T, ...) memcpy(SOPC_Malloc(sizeof(T)), &(T const){__VA_ARGS__}, sizeof(T))

const uint32_t N_VARS = 1;

OpcUa_WriteRequest* tlibw_new_WriteRequest(const SOPC_AddressSpace* address_space)
{
    OpcUa_WriteValue* lwv = SOPC_Malloc(N_VARS * sizeof(OpcUa_WriteValue));
    SOPC_ByteString buf;
    SOPC_ByteString_Initialize(&buf);

    if (NULL == lwv)
        exit(1);

    lwv[0] = (OpcUa_WriteValue){.encodeableType = &OpcUa_WriteValue_EncodeableType,
                                .NodeId = {.IdentifierType = SOPC_IdentifierType_String,
                                           .Data.String = {sizeof("PubBool") - 1, 1, (SOPC_Byte*) "PubBool"},
                                           .Namespace = 1},
                                .AttributeId = constants__e_aid_Value,
                                .IndexRange = {.Length = 0},
                                .Value = {.Value = {.BuiltInTypeId = SOPC_Boolean_Id,
                                                    .ArrayType = SOPC_VariantArrayType_SingleValue,
                                                    .Value.Boolean = true},
                                          .Status = SOPC_GoodGenericStatus}};

    OpcUa_WriteRequest* pReq = DESIGNATE_NEW(OpcUa_WriteRequest, .encodeableType = &OpcUa_WriteRequest_EncodeableType,
                                             .NoOfNodesToWrite = (int32_t) N_VARS, .NodesToWrite = lwv);
    if (NULL == pReq)
        exit(1);

    return pReq;
}

void tlibw_free_WriteRequest(OpcUa_WriteRequest** ppWriteReq)
{
    OpcUa_WriteRequest* pReq;

    if (NULL == ppWriteReq || NULL == *ppWriteReq)
        return;

    pReq = *ppWriteReq;

    /* Free the lwv */
    SOPC_Free(pReq->NodesToWrite);
    /* Free the request */
    SOPC_Free(pReq);
    /* Reset the pointer */
    *ppWriteReq = NULL;
}

bool tlibw_verify_response(OpcUa_WriteRequest* pWriteReq, OpcUa_WriteResponse* pWriteResp)
{
    bool bVerif = true;
    int32_t i;

    if (NULL == pWriteReq || NULL == pWriteResp || pWriteReq->NoOfNodesToWrite < 0)
    {
        printf("Invalid pWriteReq or pWriteResp or number of Nodes < 0\n");
        return false;
    }

    if (pWriteResp->NoOfResults != pWriteReq->NoOfNodesToWrite)
    {
        printf("Number of responses (%" PRIi32 ") differs from number of requests (%" PRIi32 ")\n",
               pWriteResp->NoOfResults, pWriteReq->NoOfNodesToWrite);
        return false; /* Can't continue, as there might be something very wrong here */
    }

    /* Verify the vector of StatusCode, should all be OK */
    for (i = 0; i < pWriteReq->NoOfNodesToWrite; ++i)
    {
        if (pWriteResp->Results[i] != 0x00000000)
        {
            printf("Response[wvi = %" PRIi32 "] is not OK (%" PRIi32 ")\n", i, pWriteResp->Results[i]);
            bVerif = false;
        }
    }

    /* Don't verify Diagnostics, don't care */

    printf("--> twlib_verify_response test result: ");
    if (bVerif)
        printf("OK\n");
    else
        printf("NOK\n");

    return bVerif;
}

OpcUa_ReadRequest* tlibw_new_ReadRequest_check(void)
{
    OpcUa_ReadValueId* lrv = SOPC_Calloc(N_VARS, sizeof(OpcUa_ReadValueId));

    if (NULL == lrv)
        exit(1);

    lrv[0] = (OpcUa_ReadValueId){.encodeableType = &OpcUa_ReadValueId_EncodeableType,
                                 .NodeId = {.IdentifierType = SOPC_IdentifierType_String,
                                            .Data.String = {sizeof("PubBool") - 1, 1, (SOPC_Byte*) "PubBool"},
                                            .Namespace = 1},
                                 .AttributeId = constants__e_aid_Value,
                                 .IndexRange = {.Length = 0},
                                 .DataEncoding = {.Name.Length = 0}};

    OpcUa_ReadRequest* pReadReq = DESIGNATE_NEW(OpcUa_ReadRequest, .encodeableType = &OpcUa_ReadRequest_EncodeableType,
                                                .MaxAge = 0., .TimestampsToReturn = OpcUa_TimestampsToReturn_Neither,
                                                .NoOfNodesToRead = (int32_t) N_VARS, .NodesToRead = lrv);

    if (NULL == pReadReq)
        exit(1);

    return pReadReq;
}

bool tlibw_verify_response_remote(OpcUa_WriteRequest* pWriteReq, OpcUa_ReadResponse* pReadResp)
{
    bool bVerif = true;
    int32_t i;
    int32_t cmp = -1;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;

    if (NULL == pWriteReq || NULL == pReadResp)
    {
        printf("Invalid pWriteReq or pReadResp\n");
        return false;
    }

    if (pWriteReq->NoOfNodesToWrite < pReadResp->NoOfResults)
    {
        printf("Number of request (%" PRIi32 ") < number of response (%" PRIi32 ")\n", pWriteReq->NoOfNodesToWrite,
               pReadResp->NoOfResults);
        return false;
    }

    /* Verify that the read value is the requested write value */
    for (i = 0; i < pReadResp->NoOfResults; ++i)
    {
        // Check statusCode of the value is the same
        if (pReadResp->Results[i].Status == pWriteReq->NodesToWrite[i].Value.Status)
        {
            status = SOPC_STATUS_OK;
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
        // Check value is the same
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_Variant_Compare(&pReadResp->Results[i].Value,            /* <-- Variant */
                                          &pWriteReq->NodesToWrite[i].Value.Value, /* <-- Variant */
                                          &cmp);
        }
        if (status != SOPC_STATUS_OK || cmp != 0)
        {
            printf("Response[rvi = %" PRIi32 "] is different from Request[wvi = %" PRIi32 "] (Compare sc = %" PRIi32
                   ", cmp = %" PRIi32
                   ")\n+ Expected "
                   "value:\n",
                   i, i, (long int) status, (long int) cmp);
            printf("Value status = 0x%X\n", (unsigned int) pWriteReq->NodesToWrite[i].Value.Status);
            util_variant__print_SOPC_Variant(&pWriteReq->NodesToWrite[i].Value.Value);
            printf("+ Read value:\n");
            printf("Value status = 0x%X\n", (unsigned int) pReadResp->Results[i].Status);
            util_variant__print_SOPC_Variant(&pReadResp->Results[i].Value);
            bVerif = false;
        }
    }

    printf("--> twlib_verify_response_remote test result: ");
    if (bVerif)
        printf("OK\n");
    else
        printf("NOK\n");

    return bVerif;
}
