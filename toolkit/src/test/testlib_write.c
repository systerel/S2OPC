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
 * Implementations of the tests details for the WriteRequest.
 */


#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "testlib_write.h"
#include "gen_addspace.h"
#include "address_space_impl.h"
#include "util_variant.h"

#include "address_space.h"
#include "address_space_bs.h"
#include "io_dispatch_mgr.h"


/* http://stackoverflow.com/questions/7265583/combine-designated-initializers-and-malloc-in-c99 */
#define DESIGNATE_NEW(T, ...)       \
  memcpy(malloc(sizeof(T)),         \
         &(T const){ __VA_ARGS__ }, \
         sizeof(T))


#define N_REQUESTS_VALS     20
#define N_REQUESTS_OTHERS   1
#define OFFSET_REQUESTS_OTHERS  N_REQUESTS_VALS
#define N_REQUESTS  (N_REQUESTS_VALS + N_REQUESTS_OTHERS)

OpcUa_WriteRequest *tlibw_new_WriteRequest(void)
{
    OpcUa_WriteValue *lwv = (OpcUa_WriteValue *)malloc(N_REQUESTS*sizeof(OpcUa_WriteValue));
    size_t i;
    SOPC_ByteString buf;
    uint32_t j;

    if(NULL == lwv)
        exit(1);

    /* First batch: N_REQUESTS_VALS is divided in n groups,
     * where n is the current number of supported types in the Address Space */

    /* int64 */
    for(i=0; i<N_REQUESTS_VALS/4; ++i)
    {
        lwv[4*i+0] = (OpcUa_WriteValue) {
            .NodeId = {
                .IdentifierType = IdentifierType_Numeric,
                .Data.Numeric = 10+i+0*(NB_NODES-8)/4},
            .AttributeId = e_aid_Value,
            .IndexRange = {.Length = 0},
            .Value = {
                .Value = {
                    .BuiltInTypeId = SOPC_Int64_Id,
                    .ArrayType = SOPC_VariantArrayType_SingleValue,
                    .Value.Int64 = (10000+i)*(i%2 ? 1:-1)}}
            };
    }

    /* uint32 */
    for(i=0; i<N_REQUESTS_VALS/4; ++i)
    {
        lwv[4*i+1] = (OpcUa_WriteValue) {
            .NodeId = {
                .IdentifierType = IdentifierType_Numeric,
                .Data.Numeric = 10+i+1*(NB_NODES-8)/4},
            .AttributeId = e_aid_Value,
            .IndexRange = {.Length = 0},
            .Value = {
                .Value = {
                    .BuiltInTypeId = SOPC_UInt32_Id,
                    .ArrayType = SOPC_VariantArrayType_SingleValue,
                    .Value.Uint32 = i}}
            };
    }

    /* double */
    for(i=0; i<N_REQUESTS_VALS/4; ++i)
    {
        lwv[4*i+2] = (OpcUa_WriteValue) {
            .NodeId = {
                .IdentifierType = IdentifierType_Numeric,
                .Data.Numeric = 10+i+2*(NB_NODES-8)/4},
            .AttributeId = e_aid_Value,
            .IndexRange = {.Length = 0},
            .Value = {
                .Value = {
                    .BuiltInTypeId = SOPC_Double_Id,
                    .ArrayType = SOPC_VariantArrayType_SingleValue,
                    .Value.Doublev = pow(2, i+1)}}
            };
    }

    /* ByteString. There MIGHT be mem leaks when writting ByteString in the AddS... */
    for(i=0; i<N_REQUESTS_VALS/4; ++i)
    {
        buf.Length = 8;
        buf.Data = (SOPC_Byte *)malloc(8);
        if(NULL == buf.Data)
            exit(1);
        j = (uint32_t)i;
        memcpy((void *)(buf.Data  ), "FOO ", 4);
        memcpy((void *)(buf.Data+4), (void *)&j, 4);

        lwv[4*i+3] = (OpcUa_WriteValue) {
            .NodeId = {
                .IdentifierType = IdentifierType_Numeric,
                .Data.Numeric = 10+i+3*(NB_NODES-8)/4},
            .AttributeId = e_aid_Value,
            .IndexRange = {.Length = 0},
            .Value = {
                .Value = {
                    .BuiltInTypeId = SOPC_ByteString_Id,
                    .ArrayType = SOPC_VariantArrayType_SingleValue,
                    .Value.Bstring = buf}}
            };
    }

    /* Second batch: redundant request */
    lwv[OFFSET_REQUESTS_OTHERS] = (OpcUa_WriteValue) {
        .NodeId = {
            .IdentifierType = IdentifierType_Numeric,
            .Data.Numeric = 10},
        .AttributeId = e_aid_Value,
        .IndexRange = {.Length = 0},
        .Value = {
            .Value = {
                .BuiltInTypeId = SOPC_Int64_Id,
                .ArrayType = SOPC_VariantArrayType_SingleValue,
                .Value.Int64 = 100}}
    };

    OpcUa_WriteRequest *pReq = DESIGNATE_NEW(OpcUa_WriteRequest,
            .NoOfNodesToWrite = N_REQUESTS,
            .NodesToWrite = lwv
        );
    if(NULL == pReq)
        exit(1);

    return pReq;
}


void tlibw_free_WriteRequest(OpcUa_WriteRequest **ppWriteReq)
{
    size_t i;
    OpcUa_WriteRequest *pReq;

    if(NULL == ppWriteReq || NULL == *ppWriteReq)
        return;

    pReq = *ppWriteReq;

    /* Free the ByteStrings */
    for(i=0; i<N_REQUESTS_VALS/4; ++i)
        free(pReq->NodesToWrite[4*i+3].Value.Value.Value.Bstring.Data);
    /* Free the lwv */
    free(pReq->NodesToWrite);
    /* Free the request */
    free(pReq);
    /* Reset the pointer */
    *ppWriteReq = NULL;
}


bool tlibw_stimulateB_with_request(OpcUa_WriteRequest *pWriteReq)
{
    constants__t_StatusCode_i sc = constants__c_StatusCode_indet;

    /* Calls treat */
    io_dispatch_mgr__treat_write_request(
        (constants__t_ByteString_i) pWriteReq,
        (constants__t_user_i) 0,
        (constants__t_StatusCode_i *) &sc);

    return sc == constants__e_sc_ok;
}


bool tlibw_verify_effects_local(OpcUa_WriteRequest *pWriteReq)
{
    OpcUa_WriteValue *lwv;
    size_t i;
    t_bool isvalid;
    constants__t_StatusCode_i sc;
    constants__t_Node_i node;
    SOPC_Variant *pVariant;
    bool bVerif = true;
    bool bTest;

    if(NULL == pWriteReq)
        exit(1);

    lwv = pWriteReq->NodesToWrite;

    for(i=0; i<(size_t)(pWriteReq->NoOfNodesToWrite); ++i)
    {
        /* Checks that the response [i] is ok */
        response_write_bs__getall_ResponseWrite_StatusCode(i+1, &isvalid, &sc);
        if(! isvalid || constants__e_sc_ok != sc)
        {
            printf("Response[wvi = %zd] is invalid (isvalid = %d, sc = %d)\n", i+1, isvalid, sc);
            bVerif = false;
        }
        /* Directly checks in the address space that the request [i] is effective */
        /* .. but first, get a Node from a nid... */
        /* TODO: this must disappear when the read interface is complete (and an application can fetch a value) */
        address_space_bs__readall_AddressSpace_Node((constants__t_NodeId_i) &lwv[i].NodeId, &isvalid, &node);
        if(! isvalid)
        {
            printf("Cannot find NodeId[wvi = %zd]\n", i+1);
            bVerif = false;
        }
        address_space_bs__read_AddressSpace_Attribute_value(node, constants__e_aid_Value, (constants__t_Variant_i *)&pVariant);
        if(i < OFFSET_REQUESTS_OTHERS)
            bTest = memcmp(pVariant, &lwv[i].Value.Value, sizeof(SOPC_Variant)) == 0;
        else
            /* The last request is redundant with the first, and because of the way our iterators are coded, it should be ignored. So its test is different. The request shall not be taken into account. */
            bTest = memcmp(pVariant, &lwv[i].Value.Value, sizeof(SOPC_Variant)) != 0;
        if(!bTest)
        {
            printf("Request[wvi = %zd] did not change the address space.\n+ Expected value:\n", i+1);
            util_variant__print_SOPC_Variant(&lwv[i].Value.Value);
            printf("+ Read value:\n");
            util_variant__print_SOPC_Variant(pVariant);
            bVerif = false;
        }
        free(pVariant);
    }

    return bVerif;
}


bool tlibw_verify_response(OpcUa_WriteRequest *pWriteReq, OpcUa_WriteResponse *pWriteResp)
{
    bool bVerif = true;
    int32_t i;

    if(pWriteReq->NoOfNodesToWrite < 0)
        exit(1);

    if(pWriteResp->NoOfResults != pWriteReq->NoOfNodesToWrite)
    {
        printf("Number of responses (%d) differs from number of requests (%d)\n", pWriteResp->NoOfResults, pWriteReq->NoOfNodesToWrite);
        return false; /* Can't continue, as there might be something very wrong here */
    }

    /* Verify the vector of StatusCode, should all be OK */
    for(i=0; i<pWriteReq->NoOfNodesToWrite; ++i)
    {
        if(pWriteResp->Results[i] != constants__e_sc_ok)
        {
            printf("Response[wvi = %d] is not OK (%d)\n", i, pWriteResp->Results[i]);
            bVerif = false;
        }
    }

    /* Don't verify Diagnostics, don't care */

    return bVerif;
}


OpcUa_ReadRequest *tlibw_new_ReadRequest_check(void)
{
    return 0;
}


bool tlibw_verify_response_remote(OpcUa_WriteRequest *pWriteReq, OpcUa_ReadResponse *pReadResp)
{
    return false;
}

