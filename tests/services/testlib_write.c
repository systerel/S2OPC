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

#include "address_space_impl.h"
#include "testlib_write.h"
#include "util_variant.h"

#include "address_space.h"
#include "address_space_bs.h"
#include "io_dispatch_mgr.h"

#include "util_b2c.h"

/* http://stackoverflow.com/questions/7265583/combine-designated-initializers-and-malloc-in-c99 */
#define DESIGNATE_NEW(T, ...) memcpy(malloc(sizeof(T)), &(T const){__VA_ARGS__}, sizeof(T))

const uint32_t N_GROUPS = 6; // Each group is a different type of variable
const uint32_t N_VARS = 6;   // Test on variables with Node Id 1001 to 1001 + N_VARS only
// Note: There is N_VARS/N_GROUPS variables of each type (variables shall be sorted by type in predefined order below)

OpcUa_WriteRequest* tlibw_new_WriteRequest(void)
{
    // Multiple of number of groups
    assert(N_VARS % N_GROUPS == 0);
    assert(N_VARS <= INT32_MAX);

    OpcUa_WriteValue* lwv = (OpcUa_WriteValue*) malloc(N_VARS * sizeof(OpcUa_WriteValue));
    size_t i;
    SOPC_ByteString buf;
    SOPC_ByteString_Initialize(&buf);
    uint32_t j;

    if (NULL == lwv)
        exit(1);

    /* First batch: variables are divided in n groups,
     * where n is the current number of supported types in the Address Space */

    /* int64 */
    for (i = 0; i < N_VARS / N_GROUPS; ++i)
    {
        lwv[i] =
            (OpcUa_WriteValue){.NodeId = {.IdentifierType = SOPC_IdentifierType_Numeric,
                                          .Data.Numeric = (uint32_t) i + 1000 + 1,
                                          .Namespace = 1},
                               .AttributeId = constants__e_aid_Value,
                               .IndexRange = {.Length = 0},
                               .Value = {.Value = {.BuiltInTypeId = SOPC_Int64_Id,
                                                   .ArrayType = SOPC_VariantArrayType_SingleValue,
                                                   .Value.Int64 = (10000 + (int64_t) i) * ((int64_t) i % 2 ? 1 : -1)},
                                         .Status = SOPC_GoodGenericStatus}};
    }

    /* uint32 */
    for (i = 0; i < N_VARS / N_GROUPS; ++i)
    {
        lwv[i + (N_VARS / N_GROUPS)] =
            (OpcUa_WriteValue){.NodeId = {.IdentifierType = SOPC_IdentifierType_Numeric,
                                          .Data.Numeric = (uint32_t) i + (N_VARS / N_GROUPS) + 1000 + 1,
                                          .Namespace = 1},
                               .AttributeId = constants__e_aid_Value,
                               .IndexRange = {.Length = 0},
                               .Value = {.Value = {.BuiltInTypeId = SOPC_UInt32_Id,
                                                   .ArrayType = SOPC_VariantArrayType_SingleValue,
                                                   .Value.Uint32 = 1000 + (uint32_t) i},
                                         .Status = SOPC_GoodGenericStatus}};
    }

    /* double */
    for (i = 0; i < N_VARS / N_GROUPS; ++i)
    {
        lwv[i + (N_VARS / N_GROUPS) * 2] =
            (OpcUa_WriteValue){.NodeId = {.IdentifierType = SOPC_IdentifierType_Numeric,
                                          .Data.Numeric = (uint32_t) i + 2 * (N_VARS / N_GROUPS) + 1000 + 1,
                                          .Namespace = 1},
                               .AttributeId = constants__e_aid_Value,
                               .IndexRange = {.Length = 0},
                               .Value = {.Value = {.BuiltInTypeId = SOPC_Double_Id,
                                                   .ArrayType = SOPC_VariantArrayType_SingleValue,
                                                   .Value.Doublev = pow(2, (double) (i + 1))},
                                         .Status = SOPC_GoodGenericStatus}};
    }

    /* String */
    for (i = 0; i < N_VARS / N_GROUPS; ++i)
    {
        buf.Length = 8;
        buf.Data = (SOPC_Byte*) malloc(8);
        if (NULL == buf.Data)
            exit(1);
        j = (uint32_t) i;
        memcpy((void*) (buf.Data), "FOO ", 4);
        memcpy((void*) (buf.Data + 4), (void*) &j, 4);

        lwv[i + 3 * (N_VARS / N_GROUPS)] =
            (OpcUa_WriteValue){.NodeId = {.IdentifierType = SOPC_IdentifierType_Numeric,
                                          .Data.Numeric = (uint32_t) i + 3 * (N_VARS / N_GROUPS) + 1000 + 1,
                                          .Namespace = 1},
                               .AttributeId = constants__e_aid_Value,
                               .IndexRange = {.Length = 0},
                               .Value = {.Value = {.BuiltInTypeId = SOPC_String_Id,
                                                   .ArrayType = SOPC_VariantArrayType_SingleValue,
                                                   .Value.String = buf},
                                         .Status = SOPC_GoodGenericStatus}};
    }

    /* ByteString */
    for (i = 0; i < N_VARS / N_GROUPS; ++i)
    {
        buf.Length = 8;
        buf.Data = (SOPC_Byte*) malloc(8);
        if (NULL == buf.Data)
            exit(1);
        j = (uint32_t) i;
        memcpy((void*) (buf.Data), "BySt", 4);
        memcpy((void*) (buf.Data + 4), (void*) &j, 4);

        lwv[i + 4 * (N_VARS / N_GROUPS)] =
            (OpcUa_WriteValue){.NodeId = {.IdentifierType = SOPC_IdentifierType_Numeric,
                                          .Data.Numeric = (uint32_t) i + 4 * (N_VARS / N_GROUPS) + 1000 + 1,
                                          .Namespace = 1},
                               .AttributeId = constants__e_aid_Value,
                               .IndexRange = {.Length = 0},
                               .Value = {.Value = {.BuiltInTypeId = SOPC_ByteString_Id,
                                                   .ArrayType = SOPC_VariantArrayType_SingleValue,
                                                   .Value.Bstring = buf},
                                         .Status = SOPC_GoodGenericStatus}};
    }

    /* XmlElt */
    for (i = 0; i < N_VARS / N_GROUPS; ++i)
    {
        buf.Length = 8;
        buf.Data = (SOPC_Byte*) malloc(8);
        if (NULL == buf.Data)
            exit(1);
        j = (uint32_t) i;
        memcpy((void*) (buf.Data), "XML ", 4);
        memcpy((void*) (buf.Data + 4), (void*) &j, 4);

        lwv[i + 5 * (N_VARS / N_GROUPS)] =
            (OpcUa_WriteValue){.NodeId = {.IdentifierType = SOPC_IdentifierType_Numeric,
                                          .Data.Numeric = (uint32_t) i + 5 * (N_VARS / N_GROUPS) + 1000 + 1,
                                          .Namespace = 1},
                               .AttributeId = constants__e_aid_Value,
                               .IndexRange = {.Length = 0},
                               .Value = {.Value = {.BuiltInTypeId = SOPC_XmlElement_Id,
                                                   .ArrayType = SOPC_VariantArrayType_SingleValue,
                                                   .Value.XmlElt = buf},
                                         .Status = OpcUa_BadDataUnavailable}};
    }

    OpcUa_WriteRequest* pReq = DESIGNATE_NEW(OpcUa_WriteRequest, .encodeableType = &OpcUa_WriteRequest_EncodeableType,
                                             .NoOfNodesToWrite = (int32_t) N_VARS, .NodesToWrite = lwv);
    if (NULL == pReq)
        exit(1);

    return pReq;
}

void tlibw_free_WriteRequest(OpcUa_WriteRequest** ppWriteReq)
{
    size_t i;
    OpcUa_WriteRequest* pReq;

    if (NULL == ppWriteReq || NULL == *ppWriteReq)
        return;

    pReq = *ppWriteReq;

    /* Free the ByteStrings */
    for (i = 0; i < N_VARS / N_GROUPS; ++i)
    {
        free(pReq->NodesToWrite[i + 3 * (N_VARS / N_GROUPS)].Value.Value.Value.String.Data);
        free(pReq->NodesToWrite[i + 4 * (N_VARS / N_GROUPS)].Value.Value.Value.Bstring.Data);
        free(pReq->NodesToWrite[i + 5 * (N_VARS / N_GROUPS)].Value.Value.Value.XmlElt.Data);
    }
    /* Free the lwv */
    free(pReq->NodesToWrite);
    /* Free the request */
    free(pReq);
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
    OpcUa_ReadValueId* lrv = (OpcUa_ReadValueId*) calloc(N_VARS, sizeof(OpcUa_ReadValueId));
    size_t i;

    if (NULL == lrv)
        exit(1);

    /* We only check that the values of the variables that were modified.
     * For the duplicate WriteRequest, there is a single request.
     * It should match (in the current implementation) the first of the two WriteValue. */
    for (i = 0; i < N_VARS; ++i)
    {
        lrv[i] = (OpcUa_ReadValueId){.NodeId = {.IdentifierType = SOPC_IdentifierType_Numeric,
                                                .Data.Numeric = (uint32_t) i + 1000 + 1,
                                                .Namespace = 1},
                                     .AttributeId = constants__e_aid_Value,
                                     .IndexRange = {.Length = 0},
                                     .DataEncoding = {.Name.Length = 0}};
    }

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
                   i, i, status, cmp);
            printf("Value status = 0x%X\n", pWriteReq->NodesToWrite[i].Value.Status);
            util_variant__print_SOPC_Variant(&pWriteReq->NodesToWrite[i].Value.Value);
            printf("+ Read value:\n");
            printf("Value status = 0x%X\n", pReadResp->Results[i].Status);
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
