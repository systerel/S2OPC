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

#include "testlib_write.h"

#include "libs2opc_request_builder.h"
#include "opcua_statuscodes.h"
#include "sopc_mem_alloc.h"

extern SOPC_AddressSpace* address_space_bs__nodes;

/* http://stackoverflow.com/questions/7265583/combine-designated-initializers-and-malloc-in-c99 */
#define DESIGNATE_NEW(T, ...) memcpy(SOPC_Malloc(sizeof(T)), &(T const){__VA_ARGS__}, sizeof(T))

const uint32_t N_VARS = 11; // Test on variables with Node Id 1001 to 1006, plus 5 other variables

static const SOPC_NodeId nodeId_int64 = SOPC_NODEID_NUMERIC(1, 1001);
static const SOPC_NodeId nodeId_uint32 = SOPC_NODEID_NUMERIC(1, 1002);
static const SOPC_NodeId nodeId_double = SOPC_NODEID_NUMERIC(1, 1003);
static const SOPC_NodeId nodeId_string = SOPC_NODEID_NUMERIC(1, 1004);
static const SOPC_NodeId nodeId_byteString = SOPC_NODEID_NUMERIC(1, 1005);
static const SOPC_NodeId nodeId_xmlElt = SOPC_NODEID_NUMERIC(1, 1006);
static const SOPC_NodeId nodeId_Boolean = SOPC_NODEID_NUMERIC(1, 1029);
static const SOPC_NodeId nodeId_DateTime = SOPC_NODEID_STRING(1, "DateTimeVar");
static const SOPC_NodeId nodeId_Guid = SOPC_NODEID_STRING(1, "GuidVar");
static const SOPC_NodeId nodeId_LocalizedText = SOPC_NODEID_NUMERIC(1, 1033);
static const SOPC_NodeId nodeId_Qname = SOPC_NODEID_NUMERIC(1, 1034);

OpcUa_WriteRequest* tlibw_new_WriteRequest(const SOPC_AddressSpace* address_space)
{
    assert(N_VARS <= INT32_MAX);

    OpcUa_WriteRequest* pReq = SOPC_WriteRequest_Create(N_VARS);
    if (NULL == pReq)
        exit(1);

    SOPC_ByteString buf;
    SOPC_ByteString_Initialize(&buf);
    uint32_t j = 0;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    /* int64 */
    SOPC_DataValue dataValue_int64 = {
        .Value = {.BuiltInTypeId = SOPC_Int64_Id, .ArrayType = SOPC_VariantArrayType_SingleValue, .Value.Int64 = 10001},
        .Status = SOPC_GoodGenericStatus};

    status = SOPC_WriteRequest_SetWriteValue(pReq, 0, &nodeId_int64, SOPC_AttributeId_Value, NULL, &dataValue_int64);
    if (SOPC_STATUS_OK != status)
    {
        exit(1);
    }

    /* uint32 */
    SOPC_DataValue dataValue_uint32 = {.Value = {.BuiltInTypeId = SOPC_UInt32_Id,
                                                 .ArrayType = SOPC_VariantArrayType_SingleValue,
                                                 .Value.Uint32 = 1000},
                                       .Status = SOPC_GoodGenericStatus};
    status = SOPC_WriteRequest_SetWriteValue(pReq, 1, &nodeId_uint32, SOPC_AttributeId_Value, NULL, &dataValue_uint32);
    if (SOPC_STATUS_OK != status)
    {
        exit(1);
    }

    /* double */
    SOPC_DataValue dataValue_double = {.Value = {.BuiltInTypeId = SOPC_Double_Id,
                                                 .ArrayType = SOPC_VariantArrayType_SingleValue,
                                                 .Value.Doublev = pow(2, 1)},
                                       .Status = SOPC_GoodGenericStatus};
    status = SOPC_WriteRequest_SetWriteValue(pReq, 2, &nodeId_double, SOPC_AttributeId_Value, NULL, &dataValue_double);
    if (SOPC_STATUS_OK != status)
    {
        exit(1);
    }

    /* String */
    buf.Length = 8;
    buf.Data = SOPC_Malloc(8);
    if (NULL == buf.Data)
    {
        exit(1);
    }

    memcpy((void*) (buf.Data), "FOO ", 4);
    memcpy((void*) (buf.Data + 4), &j, 4);
    SOPC_DataValue dataValue_string = {
        .Value = {.BuiltInTypeId = SOPC_String_Id, .ArrayType = SOPC_VariantArrayType_SingleValue, .Value.String = buf},
        .Status = SOPC_GoodGenericStatus};
    status = SOPC_WriteRequest_SetWriteValue(pReq, 3, &nodeId_string, SOPC_AttributeId_Value, NULL, &dataValue_string);
    SOPC_ByteString_Clear(&buf);
    if (SOPC_STATUS_OK != status)
        exit(1);

    /* ByteString */
    buf.Length = 8;
    buf.Data = SOPC_Malloc(8);
    if (NULL == buf.Data)
    {
        exit(1);
    }
    memcpy((void*) (buf.Data), "BySt", 4);
    memcpy((void*) (buf.Data + 4), &j, 4);
    SOPC_DataValue dataValue_byteString = {.Value = {.BuiltInTypeId = SOPC_ByteString_Id,
                                                     .ArrayType = SOPC_VariantArrayType_SingleValue,
                                                     .Value.Bstring = buf},
                                           .Status = SOPC_GoodGenericStatus};
    status = SOPC_WriteRequest_SetWriteValue(pReq, 4, &nodeId_byteString, SOPC_AttributeId_Value, NULL,
                                             &dataValue_byteString);
    SOPC_ByteString_Clear(&buf);
    if (SOPC_STATUS_OK != status)
    {
        exit(1);
    }

    /* XmlElt */
    buf.Length = 8;
    buf.Data = SOPC_Malloc(8);
    if (NULL == buf.Data)
    {
        exit(1);
    }
    memcpy((void*) (buf.Data), "XML", 4);
    memcpy((void*) (buf.Data + 4), &j, 4);
    SOPC_DataValue dataValue_xmlElt = {.Value = {.BuiltInTypeId = SOPC_XmlElement_Id,
                                                 .ArrayType = SOPC_VariantArrayType_SingleValue,
                                                 .Value.XmlElt = buf},
                                       .Status = SOPC_AddressSpace_AreReadOnlyNodes(address_space)
                                                     ? SOPC_GoodGenericStatus
                                                     : OpcUa_BadDataUnavailable};
    status = SOPC_WriteRequest_SetWriteValue(pReq, 5, &nodeId_xmlElt, SOPC_AttributeId_Value, NULL, &dataValue_xmlElt);
    SOPC_ByteString_Clear(&buf);
    if (SOPC_STATUS_OK != status)
    {
        exit(1);
    }

    // Boolean
    SOPC_DataValue dataValue_Boolean = {.Value = {.BuiltInTypeId = SOPC_Boolean_Id,
                                                  .ArrayType = SOPC_VariantArrayType_SingleValue,
                                                  .Value.Boolean = false},
                                        .Status = SOPC_AddressSpace_AreReadOnlyNodes(address_space)
                                                      ? SOPC_GoodGenericStatus
                                                      : OpcUa_BadDataUnavailable};
    status =
        SOPC_WriteRequest_SetWriteValue(pReq, 6, &nodeId_Boolean, SOPC_AttributeId_Value, NULL, &dataValue_Boolean);
    if (SOPC_STATUS_OK != status)
    {
        exit(1);
    }

    // DateTime
    SOPC_DataValue dataValue_DateTime = {.Value = {.BuiltInTypeId = SOPC_DateTime_Id,
                                                   .ArrayType = SOPC_VariantArrayType_SingleValue,
                                                   .Value.Date = 10367},
                                         .Status = SOPC_AddressSpace_AreReadOnlyNodes(address_space)
                                                       ? SOPC_GoodGenericStatus
                                                       : OpcUa_BadDataUnavailable};
    status =
        SOPC_WriteRequest_SetWriteValue(pReq, 7, &nodeId_DateTime, SOPC_AttributeId_Value, NULL, &dataValue_DateTime);
    if (SOPC_STATUS_OK != status)
    {
        exit(1);
    }

    // Guid
    SOPC_Guid* guid = SOPC_Malloc(sizeof(SOPC_Guid));
    const char* strGuid = "53f484c1-c9bd-4b5d-803a-767c3a45e4e0";
    status = SOPC_Guid_FromCString(guid, strGuid, strlen(strGuid));
    if (SOPC_STATUS_OK != status)
    {
        exit(1);
    }
    SOPC_DataValue dataValue_Guid = {
        .Value = {.BuiltInTypeId = SOPC_Guid_Id, .ArrayType = SOPC_VariantArrayType_SingleValue, .Value.Guid = guid},
        .Status =
            SOPC_AddressSpace_AreReadOnlyNodes(address_space) ? SOPC_GoodGenericStatus : OpcUa_BadDataUnavailable};
    status = SOPC_WriteRequest_SetWriteValue(pReq, 8, &nodeId_Guid, SOPC_AttributeId_Value, NULL, &dataValue_Guid);
    SOPC_Guid_Clear(guid);
    SOPC_Free(guid);
    if (SOPC_STATUS_OK != status)
    {
        exit(1);
    }

    // LocalizedText
    SOPC_LocalizedText lt;
    SOPC_LocalizedText_Initialize(&lt);
    status = SOPC_String_AttachFromCstring(&lt.defaultLocale, "en-US");
    if (SOPC_STATUS_OK != status)
    {
        exit(1);
    }
    status = SOPC_String_AttachFromCstring(&lt.defaultText, "English american localized text");
    if (SOPC_STATUS_OK != status)
    {
        exit(1);
    }
    SOPC_DataValue dataValue_LocalizedText = {.Value = {.BuiltInTypeId = SOPC_LocalizedText_Id,
                                                        .ArrayType = SOPC_VariantArrayType_SingleValue,
                                                        .Value.LocalizedText = &lt},
                                              .Status = SOPC_AddressSpace_AreReadOnlyNodes(address_space)
                                                            ? SOPC_GoodGenericStatus
                                                            : OpcUa_BadDataUnavailable};
    status = SOPC_WriteRequest_SetWriteValue(pReq, 9, &nodeId_LocalizedText, SOPC_AttributeId_Value, NULL,
                                             &dataValue_LocalizedText);
    SOPC_LocalizedText_Clear(&lt);
    if (SOPC_STATUS_OK != status)
    {
        exit(1);
    }

    // QualifiedName
    SOPC_QualifiedName qn = SOPC_QUALIFIED_NAME(19, "TestQName");
    SOPC_DataValue dataValue_Qname = {.Value = {.BuiltInTypeId = SOPC_QualifiedName_Id,
                                                .ArrayType = SOPC_VariantArrayType_SingleValue,
                                                .Value.Qname = &qn},
                                      .Status = SOPC_AddressSpace_AreReadOnlyNodes(address_space)
                                                    ? SOPC_GoodGenericStatus
                                                    : OpcUa_BadDataUnavailable};
    status = SOPC_WriteRequest_SetWriteValue(pReq, 10, &nodeId_Qname, SOPC_AttributeId_Value, NULL, &dataValue_Qname);
    if (SOPC_STATUS_OK != status)
    {
        exit(1);
    }
    SOPC_QualifiedName_Clear(&qn);

    return pReq;
}

bool tlibw_verify_response(OpcUa_WriteRequest* pWriteReq, const OpcUa_WriteResponse* pWriteResp)
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
    OpcUa_ReadRequest* pReadReq = SOPC_ReadRequest_Create(N_VARS, OpcUa_TimestampsToReturn_Neither);

    if (NULL == pReadReq)
        exit(1);

    SOPC_ReturnStatus status = SOPC_STATUS_NOK;

    /* We only check that the values of the variables that were modified.
     * For the duplicate WriteRequest, there is a single request.
     * It should match (in the current implementation) the first of the two WriteValue. */
    // First NodeIds are numeric ns=1;i=1001...1006
    for (size_t i = 0; i < 6; ++i)
    {
        SOPC_NodeId nodeId = {
            .IdentifierType = SOPC_IdentifierType_Numeric, .Data.Numeric = (uint32_t) i + 1000 + 1, .Namespace = 1};
        status = SOPC_ReadRequest_SetReadValue(pReadReq, i, &nodeId, SOPC_AttributeId_Value, NULL);
        if (SOPC_STATUS_OK != status)
            exit(1);
    }
    // Other NodeIds are string or numeric and there is no continuity between them,
    // they must be managed individually.
    // Boolean
    status = SOPC_ReadRequest_SetReadValue(pReadReq, 6, &nodeId_Boolean, SOPC_AttributeId_Value, NULL);
    if (SOPC_STATUS_OK != status)
    {
        exit(1);
    }

    // DateTime
    status = SOPC_ReadRequest_SetReadValue(pReadReq, 7, &nodeId_DateTime, SOPC_AttributeId_Value, NULL);
    if (SOPC_STATUS_OK != status)
    {
        exit(1);
    }

    // Guid
    status = SOPC_ReadRequest_SetReadValue(pReadReq, 8, &nodeId_Guid, SOPC_AttributeId_Value, NULL);
    if (SOPC_STATUS_OK != status)
    {
        exit(1);
    }

    // LocalizedText
    status = SOPC_ReadRequest_SetReadValue(pReadReq, 9, &nodeId_LocalizedText, SOPC_AttributeId_Value, NULL);
    if (SOPC_STATUS_OK != status)
    {
        exit(1);
    }

    // QualifiedName
    status = SOPC_ReadRequest_SetReadValue(pReadReq, 10, &nodeId_Qname, SOPC_AttributeId_Value, NULL);
    if (SOPC_STATUS_OK != status)
    {
        exit(1);
    }

    return pReadReq;
}

bool tlibw_verify_response_remote(OpcUa_WriteRequest* pWriteReq, const OpcUa_ReadResponse* pReadResp)
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
        // Update local address space to be the same in case of client only test
        if (SOPC_STATUS_OK == status && 0 == cmp)
        {
            bool found = false;
            SOPC_AddressSpace_Node* node =
                SOPC_AddressSpace_Get_Node(address_space_bs__nodes, &pWriteReq->NodesToWrite[i].NodeId, &found);
            assert(found);
            SOPC_Variant* variant = SOPC_AddressSpace_Get_Value(address_space_bs__nodes, node);
            SOPC_Variant_Clear(variant);
            *variant = pReadResp->Results[i].Value;
            SOPC_Variant_Initialize(&pReadResp->Results[i].Value);
        }
        if (status != SOPC_STATUS_OK || cmp != 0)
        {
            printf("Response[rvi = %" PRIi32 "] is different from Request[wvi = %" PRIi32 "] (Compare sc = %" PRIi32
                   ", cmp = %" PRIi32
                   ")\n+ Expected "
                   "value:\n",
                   i, i, status, cmp);
            printf("Value status = 0x%X\n", pWriteReq->NodesToWrite[i].Value.Status);
            SOPC_Variant_Print(&pWriteReq->NodesToWrite[i].Value.Value);
            printf("+ Read value:\n");
            printf("Value status = 0x%X\n", pReadResp->Results[i].Status);
            SOPC_Variant_Print(&pReadResp->Results[i].Value);
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
