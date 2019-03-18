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
 */
#include <assert.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include "constants.h"

#include "address_space_impl.h"
#include "testlib_read_response.h"

extern SOPC_AddressSpace* address_space_bs__nodes;

/**
 * You should free() the returned Variant* afterwards.
 */
static SOPC_Variant* get_attribute_variant(SOPC_AddressSpace_Item* item, uint32_t attr_id)
{
    switch (attr_id)
    {
    case constants__e_aid_NodeId:
        return util_variant__new_Variant_from_NodeId(SOPC_AddressSpace_Item_Get_NodeId(item));
    case constants__e_aid_NodeClass:
        return util_variant__new_Variant_from_NodeClass(item->node_class);
    case constants__e_aid_Value:
        return util_variant__new_Variant_from_Variant(SOPC_AddressSpace_Item_Get_Value(item));
    /* TODO: rest of mandatory attributes */
    default:
        return NULL;
    }
}

bool test_read_request_response(OpcUa_ReadResponse* pReadResp, SOPC_StatusCode statusCode, int verbose)
{
    printf("--> ReadRequest test result: ");
    if (verbose > 0)
        printf("\n");

    bool bTestOk = false;
    int32_t comp = 0;
    SOPC_Variant* pvar;
    int32_t i;

    /* Check the service StatusCode */
    if (verbose > 0)
        printf("Service status code: %" PRIX32 " (should be %d)\n", statusCode, SOPC_STATUS_OK);
    bTestOk = SOPC_STATUS_OK == statusCode;

    /* Creates a Request */
    OpcUa_ReadRequest* pReadReq = read_new_read_request();

    /* Prints the Response */
    if (verbose > 0)
    {
        printf("pReadResp->NoOfResults: %" PRIi32 "\n", pReadResp->NoOfResults);
        for (i = 0; i < pReadResp->NoOfResults; ++i)
        {
            /* Note: Status is a B-StatusCode */
            printf("pReadResp->Results[%" PRIi32 "].Status: 0x%08" PRIX32 "\n", i, pReadResp->Results[i].Status);
            util_variant__print_SOPC_Variant(&pReadResp->Results[i].Value);
        }
    }

    /* Test number of results */
    if (bTestOk)
        bTestOk = pReadReq->NoOfNodesToRead == pReadResp->NoOfResults;

    /* Analyze each response element */
    for (i = 0; bTestOk && i < pReadReq->NoOfNodesToRead; ++i)

    {
        SOPC_AddressSpace_Item* item =
            SOPC_Dict_Get(address_space_bs__nodes, &pReadReq->NodesToRead[i].NodeId, &bTestOk);

        /* Find desired attribute and wrap it in a new SOPC_Variant* */
        if (bTestOk)
        {
            pvar = get_attribute_variant(item, pReadReq->NodesToRead[i].AttributeId);
        }
        else
        {
            pvar = NULL;
        }
        /* Compares the wrapped value with the response to the request */
        bTestOk = bTestOk && SOPC_STATUS_OK == SOPC_Variant_Compare(&pReadResp->Results[i].Value, pvar, &comp);
        bTestOk = bTestOk && comp == 0;
        if (verbose > 1)
        {
            printf("-- Comparing pvar:\n");
            util_variant__print_SOPC_Variant(pvar);
            printf("-- with response[%d]:\n", i);
            util_variant__print_SOPC_Variant(&pReadResp->Results[i].Value);
            if (bTestOk)
                printf("-- ok\n");
            else
                printf("-- nok\n");
        }
        free(pvar); /* It's ok to free a NULL */
    }

    /* Free the Request */
    free(pReadReq->NodesToRead);
    free(pReadReq);

    if (bTestOk)
        printf("OK\n");
    else
        printf("NOT ok\n");

    return bTestOk;
}
