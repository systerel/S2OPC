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
#ifdef __TRUSTINSOFT_TMPBUG__
      // avoid separated alarm when initializing a struct (maybe TRUS-1281?)
      OpcUa_WriteValue tis_tmp =
#else
        lwv[i] =
#endif
            (OpcUa_WriteValue){.NodeId = {.IdentifierType = SOPC_IdentifierType_Numeric, .Data.Numeric = i + 1000 + 1},
                               .AttributeId = e_aid_Value,
                               .IndexRange = {.Length = 0},
                               .Value = {.Value = {.BuiltInTypeId = SOPC_Int64_Id,
                                                   .ArrayType = SOPC_VariantArrayType_SingleValue,
                                                   .Value.Int64 = (10000 + i) * (i % 2 ? 1 : -1)}}};
#ifdef __TRUSTINSOFT_TMPBUG__
      // avoid separated alarm when initializing a struct (maybe TRUS-1281?)
        lwv[i] = tis_tmp;
#endif
    }

    /* uint32 */
    for (i = 0; i < N_VARS / N_GROUPS; ++i)
    {
#ifdef __TRUSTINSOFT_TMPBUG__
      // avoid separated alarm when initializing a struct (maybe TRUS-1281?)
      OpcUa_WriteValue tis_tmp =
#else
        lwv[i + (N_VARS / N_GROUPS)] =
#endif
            (OpcUa_WriteValue){.NodeId = {.IdentifierType = SOPC_IdentifierType_Numeric,
                                          .Data.Numeric = i + (N_VARS / N_GROUPS) + 1000 + 1},
                               .AttributeId = e_aid_Value,
                               .IndexRange = {.Length = 0},
                               .Value = {.Value = {.BuiltInTypeId = SOPC_UInt32_Id,
                                                   .ArrayType = SOPC_VariantArrayType_SingleValue,
                                                   .Value.Uint32 = 1000 + i}}};
#ifdef __TRUSTINSOFT_TMPBUG__
      // avoid separated alarm when initializing a struct (maybe TRUS-1281?)
      lwv[i+(N_VARS/N_GROUPS)] = tis_tmp;
#endif
    }

    /* double */
    for (i = 0; i < N_VARS / N_GROUPS; ++i)
    {
#ifdef __TRUSTINSOFT_TMPBUG__
      // avoid separated alarm when initializing a struct (maybe TRUS-1281?)
      OpcUa_WriteValue tis_tmp =
#else
        lwv[i + (N_VARS / N_GROUPS) * 2] =
#endif
            (OpcUa_WriteValue){.NodeId = {.IdentifierType = SOPC_IdentifierType_Numeric,
                                          .Data.Numeric = i + 2 * (N_VARS / N_GROUPS) + 1000 + 1},
                               .AttributeId = e_aid_Value,
                               .IndexRange = {.Length = 0},
                               .Value = {.Value = {.BuiltInTypeId = SOPC_Double_Id,
                                                   .ArrayType = SOPC_VariantArrayType_SingleValue,
                                                   .Value.Doublev = pow(2, i + 1)}}};
#ifdef __TRUSTINSOFT_TMPBUG__
      // avoid separated alarm when initializing a struct (maybe TRUS-1281?)
        lwv[i + (N_VARS / N_GROUPS) * 2] = tis_tmp;
#endif
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

#ifdef __TRUSTINSOFT_TMPBUG__
      // avoid separated alarm when initializing a struct (maybe TRUS-1281?)
      OpcUa_WriteValue tis_tmp =
#else
        lwv[i + 3 * (N_VARS / N_GROUPS)] =
#endif
            (OpcUa_WriteValue){.NodeId = {.IdentifierType = SOPC_IdentifierType_Numeric,
                                          .Data.Numeric = i + 3 * (N_VARS / N_GROUPS) + 1000 + 1},
                               .AttributeId = e_aid_Value,
                               .IndexRange = {.Length = 0},
                               .Value = {.Value = {.BuiltInTypeId = SOPC_String_Id,
                                                   .ArrayType = SOPC_VariantArrayType_SingleValue,
                                                   .Value.String = buf}}};
#ifdef __TRUSTINSOFT_TMPBUG__
      // avoid separated alarm when initializing a struct (maybe TRUS-1281?)
        lwv[i + 3 * (N_VARS / N_GROUPS)] = tis_tmp;
#endif
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

#ifdef __TRUSTINSOFT_TMPBUG__
      // avoid separated alarm when initializing a struct (maybe TRUS-1281?)
      OpcUa_WriteValue tis_tmp =
#else
        lwv[i + 4 * (N_VARS / N_GROUPS)] =
#endif
            (OpcUa_WriteValue){.NodeId = {.IdentifierType = SOPC_IdentifierType_Numeric,
                                          .Data.Numeric = i + 4 * (N_VARS / N_GROUPS) + 1000 + 1},
                               .AttributeId = e_aid_Value,
                               .IndexRange = {.Length = 0},
                               .Value = {.Value = {.BuiltInTypeId = SOPC_ByteString_Id,
                                                   .ArrayType = SOPC_VariantArrayType_SingleValue,
                                                   .Value.Bstring = buf}}};
#ifdef __TRUSTINSOFT_TMPBUG__
      // avoid separated alarm when initializing a struct (maybe TRUS-1281?)
        lwv[i + 4 * (N_VARS / N_GROUPS)] = tis_tmp;
#endif
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

#ifdef __TRUSTINSOFT_TMPBUG__
      // avoid separated alarm when initializing a struct (maybe TRUS-1281?)
      OpcUa_WriteValue tis_tmp =
#else
        lwv[i + 5 * (N_VARS / N_GROUPS)] =
#endif
            (OpcUa_WriteValue){.NodeId = {.IdentifierType = SOPC_IdentifierType_Numeric,
                                          .Data.Numeric = i + 5 * (N_VARS / N_GROUPS) + 1000 + 1},
                               .AttributeId = e_aid_Value,
                               .IndexRange = {.Length = 0},
                               .Value = {.Value = {.BuiltInTypeId = SOPC_XmlElement_Id,
                                                   .ArrayType = SOPC_VariantArrayType_SingleValue,
                                                   .Value.XmlElt = buf}}};
#ifdef __TRUSTINSOFT_TMPBUG__
      // avoid separated alarm when initializing a struct (maybe TRUS-1281?)
        lwv[i + 5 * (N_VARS / N_GROUPS)] = tis_tmp;
#endif
    }

#ifdef __TRUSTINSOFT_BUGFIX__
  // fix missing test of malloc return (TODO: report!) + TRUS-1281
  OpcUa_WriteRequest* pReq = malloc (sizeof (OpcUa_WriteRequest));
    if (NULL == pReq)
        exit(1);
    OpcUa_WriteRequest tis_tmp = (OpcUa_WriteRequest) { .encodeableType = &OpcUa_WriteRequest_EncodeableType,
                                             .NoOfNodesToWrite = N_VARS, .NodesToWrite = lwv};
    *pReq = tis_tmp;
#else
    OpcUa_WriteRequest* pReq = DESIGNATE_NEW(OpcUa_WriteRequest, .encodeableType = &OpcUa_WriteRequest_EncodeableType,
                                             .NoOfNodesToWrite = N_VARS, .NodesToWrite = lwv);
    if (NULL == pReq)
        exit(1);
#endif

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

bool tlibw_stimulateB_with_message(void* pMsg)
{
    constants__t_StatusCode_i sc = constants__c_StatusCode_indet;

    /* Calls treat */
    service_mgr__treat_write_request((constants__t_msg_i) pMsg, (constants__t_StatusCode_i*) &sc);

    return sc == constants__e_sc_ok;
}

bool tlibw_verify_effects_local(OpcUa_WriteRequest* pWriteReq)
{
    OpcUa_WriteValue* lwv;
    int32_t i;
    t_bool isvalid;
    constants__t_StatusCode_i sc = constants__e_sc_bad_generic;
    constants__t_Node_i node;
    SOPC_Variant* pVariant = NULL;
    bool bVerif = true;
    int32_t cmp;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;

    if (NULL == pWriteReq)
        exit(1);

    lwv = pWriteReq->NodesToWrite;

    for (i = 0; i < pWriteReq->NoOfNodesToWrite; ++i)
    {
        /* Checks that the response [i] is ok */
        response_write_bs__getall_ResponseWrite_StatusCode(i + 1, &isvalid, &sc);
        if (!isvalid || constants__e_sc_ok != sc)
        {
#ifdef __TRUSTINSOFT_BUGFIX__
          // initialize sc to a dummy value to avoid alarm (+minor: printf)
          sc = constants__c_StatusCode_indet;
            printf("Response[wvi = %" PRIi32 "] is invalid (isvalid = %d, sc = %d)\n", i + 1, isvalid, (int)sc);
#else
            printf("Response[wvi = %" PRIi32 "] is invalid (isvalid = %d, sc = %d)\n", i + 1, isvalid, sc);
#endif
            bVerif = false;
        }
        /* Directly checks in the address space that the request [i] is effective */
        /* .. but first, get a Node from a nid... */
        /* TODO: this must disappear when the read interface is complete (and an application can fetch a value) */
        address_space_bs__readall_AddressSpace_Node((constants__t_NodeId_i) &lwv[i].NodeId, &isvalid, &node);
        if (!isvalid)
        {
            printf("Cannot find NodeId[wvi = %" PRIi32 "]\n", i + 1);
            bVerif = false;
        }
        else
        {
            address_space_bs__read_AddressSpace_Attribute_value(node, constants__e_ncl_Variable, constants__e_aid_Value,
                                                                &sc, (constants__t_Variant_i*) &pVariant);
        }

        if (sc == constants__e_sc_ok)
        {
            status = SOPC_Variant_Compare(pVariant, &lwv[i].Value.Value, &cmp);
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
        /* The last request is redundant with the first, and because of the way our iterators are coded, it should be
         * ignored. So its test is different. The request shall not be taken into account. */
        if (status != SOPC_STATUS_OK || cmp != 0)
        {
#ifdef __TRUSTINSOFT_BUGFIX__
          // initialize cmp to a dummy value to avoid alarm
          cmp = -1000;
#endif
            printf("Request[wvi = %" PRIi32 "] did not change the address space (Compare sc = %d, cmp = %" PRIi32
                   ")\n+ Expected value:\n",
#ifdef __TRUSTINSOFT_BUGFIX__
                   //minor (printf format)
                   i, (int)status, cmp);
#else
                   i, status, cmp);
#endif
            util_variant__print_SOPC_Variant(&lwv[i].Value.Value);
            printf("+ Read value:\n");
            util_variant__print_SOPC_Variant(pVariant);
            bVerif = false;
        }
        if (NULL != pVariant)
        {
            free(pVariant);
        }
    }

    /* Free the response's internals */
    response_write_bs__reset_ResponseWrite();

    return bVerif;
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
    OpcUa_ReadValueId* lrv = (OpcUa_ReadValueId*) malloc(N_VARS * sizeof(OpcUa_ReadValueId));
    size_t i;

    if (NULL == lrv)
        exit(1);

    /* We only check that the values of the variables that were modified.
     * For the duplicate WriteRequest, there is a single request.
     * It should match (in the current implementation) the first of the two WriteValue. */
    for (i = 0; i < N_VARS; ++i)
    {
        lrv[i] =
            (OpcUa_ReadValueId){.NodeId = {.IdentifierType = SOPC_IdentifierType_Numeric, .Data.Numeric = i + 1000 + 1},
                                .AttributeId = e_aid_Value,
                                .IndexRange = {.Length = 0},
                                .DataEncoding = {.Name.Length = 0}};
    }

    OpcUa_ReadRequest* pReadReq = DESIGNATE_NEW(OpcUa_ReadRequest, .encodeableType = &OpcUa_ReadRequest_EncodeableType,
                                                .MaxAge = 0., .TimestampsToReturn = OpcUa_TimestampsToReturn_Neither,
                                                .NoOfNodesToRead = N_VARS, .NodesToRead = lrv);

    if (NULL == pReadReq)
        exit(1);

    return pReadReq;
}

bool tlibw_verify_response_remote(OpcUa_WriteRequest* pWriteReq, OpcUa_ReadResponse* pReadResp)
{
    bool bVerif = true;
    int32_t i;
    int32_t cmp;
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
        status = SOPC_Variant_Compare(&pReadResp->Results[i].Value,            /* <-- Variant */
                                      &pWriteReq->NodesToWrite[i].Value.Value, /* <-- Variant */
                                      &cmp);
        if (status != SOPC_STATUS_OK || cmp != 0)
        {
            printf("Response[rvi = %" PRIi32 "] is different from Request[wvi = %" PRIi32 "] (Compare sc = %" PRIi32
                   ", cmp = %" PRIi32
                   ")\n+ Expected "
                   "value:\n",
                   i, i, status, cmp);
            util_variant__print_SOPC_Variant(&pWriteReq->NodesToWrite[i].Value.Value);
            printf("+ Read value:\n");
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
