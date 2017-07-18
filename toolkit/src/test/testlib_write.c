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

#include "testlib_write.h"
#include "gen_addspace.h"
#include "address_space_impl.h"


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


void tlibw_stimulateB_with_request(OpcUa_WriteRequest *pWriteReq)
{
}


bool tlibw_verify_effects_local(OpcUa_WriteRequest *pWriteReq)
{
    return false;
}


bool tlibw_verify_response(OpcUa_WriteResponse *pWriteResp)
{
    return false;
}


OpcUa_ReadRequest *tlibw_new_ReadRequest_check(void)
{
    return 0;
}


bool tlibw_verify_response_remote(OpcUa_WriteRequest *pWriteReq, OpcUa_ReadResponse *pReadResp)
{
    return false;
}

