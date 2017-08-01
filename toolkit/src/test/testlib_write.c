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

#include "sopc_sc_events.h"

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


SOPC_Toolkit_Msg *tlibw_new_message_WriteRequest(OpcUa_WriteRequest *pWriteReq)
{
    SOPC_Toolkit_Msg *pMsg = (SOPC_Toolkit_Msg *) calloc(1, sizeof(SOPC_Toolkit_Msg));
    if(NULL == pMsg)
        return NULL;
    
    pMsg->msgType = &OpcUa_WriteRequest_EncodeableType;
    pMsg->isRequest = (!FALSE);
    pMsg->msgStruct = pWriteReq;
    
    return pMsg;
}


bool tlibw_stimulateB_with_message(SOPC_Toolkit_Msg *pMsg)
{
    constants__t_StatusCode_i sc = constants__c_StatusCode_indet;

    /* Calls treat */
    io_dispatch_mgr__treat_write_request(
        (constants__t_msg_i) pMsg,
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
    int32_t cmp;
    SOPC_StatusCode ssc;

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

        ssc = SOPC_Variant_Compare(pVariant, &lwv[i].Value.Value, &cmp);
        /* The last request is redundant with the first, and because of the way our iterators are coded, it should be ignored. So its test is different. The request shall not be taken into account. */
        if(i < OFFSET_REQUESTS_OTHERS && (ssc != STATUS_OK || cmp != 0))
        {
            printf("Request[wvi = %zd] did not change the address space (Compare sc = %d, cmp = %d)\n+ Expected value:\n", i, ssc, cmp);
            util_variant__print_SOPC_Variant(&lwv[i].Value.Value);
            printf("+ Read value:\n");
            util_variant__print_SOPC_Variant(pVariant);
            bVerif = false;
        }
        else if(i >= OFFSET_REQUESTS_OTHERS && (ssc != STATUS_OK || cmp == 0))
        {
            printf("Request[wvi = %zd] was not ignored and changed the address space (Compare sc = %d, cmp = %d)\n+ Read value:\n", i, ssc, cmp);
            util_variant__print_SOPC_Variant(pVariant);
            bVerif = false;
        }
        /* else it is ok */

        free(pVariant);
    }

    /* Free the response's internals */
    response_write_bs__reset_ResponseWrite();

    return bVerif;
}


bool tlibw_verify_response(OpcUa_WriteRequest *pWriteReq, OpcUa_WriteResponse *pWriteResp)
{
    bool bVerif = true;
    int32_t i;

    if(NULL == pWriteReq || NULL == pWriteResp || pWriteReq->NoOfNodesToWrite < 0)
    {
        printf("Invalid pWriteReq or pWriteResp or number of Nodes < 0\n");
        return false;
    }

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

    printf("--> twlib_verify_response test result: ");
    if(bVerif)
        printf("OK\n");
    else
        printf("NOK\n");

    return bVerif;
}


OpcUa_ReadRequest *tlibw_new_ReadRequest_check(void)
{
    OpcUa_ReadValueId *lrv = (OpcUa_ReadValueId *)malloc(N_REQUESTS_VALS*sizeof(OpcUa_ReadValueId));
    size_t i, j;

    if(NULL == lrv)
        exit(1);

    /* We only check that the values of the variables that were modified.
     * For the duplicate WriteRequest, there is a single request.
     * It should match (in the current implementation) the first of the two WriteValue. */
    for(i=0; i<N_REQUESTS_VALS/4; ++i)
    {
        for(j=0; j<4; ++j)
        {
            lrv[4*i+j] = (OpcUa_ReadValueId) {
                .NodeId = {
                    .IdentifierType = IdentifierType_Numeric,
                    .Data.Numeric = 10+i+j*(NB_NODES-8)/4 },
                .AttributeId = e_aid_Value,
                .IndexRange = {.Length = 0},
                .DataEncoding = {.Name.Length = 0} };
        }
    }

    OpcUa_ReadRequest *pReadReq = DESIGNATE_NEW(OpcUa_ReadRequest,
            /*.RequestHeader = , */
            .MaxAge = 0.,
            .TimestampsToReturn = OpcUa_TimestampsToReturn_Neither,
            .NoOfNodesToRead = N_REQUESTS_VALS,
            .NodesToRead = lrv
        );

    if(NULL == pReadReq)
        exit(1);

    return pReadReq;
}


bool tlibw_verify_response_remote(OpcUa_WriteRequest *pWriteReq, OpcUa_ReadResponse *pReadResp)
{
    bool bVerif = true;
    int32_t i;
    int32_t cmp;
    SOPC_StatusCode sc;
    
    if(NULL == pWriteReq || NULL == pReadResp)
    {
        printf("Invalid pWriteReq or pReadResp\n");
        return false;
    }

    if(pWriteReq->NoOfNodesToWrite < pReadResp->NoOfResults)
    {
        printf("Number of request (%d) < number of response (%d)\n",
               pWriteReq->NoOfNodesToWrite, pReadResp->NoOfResults);
        return false;
    }

    /* Verify that the read value is the requested write value */
    for(i=0; i<pReadResp->NoOfResults; ++i)
    {
        sc = SOPC_Variant_Compare(&pReadResp->Results[i].Value, /* <-- Variant */
                                  &pWriteReq->NodesToWrite[i].Value.Value, /* <-- Variant */
                                  &cmp);
        if(sc != STATUS_OK || cmp != 0)
        {
            printf("Response[rvi = %d] is different from Request[wvi = %d] (Compare sc = %d, cmp = %d)\n+ Expected value:\n",
                   i, i, sc, cmp);
            util_variant__print_SOPC_Variant(&pWriteReq->NodesToWrite[i].Value.Value);
            printf("+ Read value:\n");
            util_variant__print_SOPC_Variant(&pReadResp->Results[i].Value);
            bVerif = false;
        }
    }

    printf("--> twlib_verify_response_remote test result: ");
    if(bVerif)
        printf("OK\n");
    else
        printf("NOK\n");

    return bVerif;
}

