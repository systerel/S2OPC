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
 */
#include <assert.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include "constants.h"

#include "address_space_impl.h"
#include "testlib_read_response.h"

/**
 * You should free() the returned Variant* afterwards.
 */
SOPC_Variant* new_variant_rvi(SOPC_NodeId** pnids,
                              OpcUa_NodeClass* pncls,
                              SOPC_Variant* pvars,
                              SOPC_StatusCode* pscs,
                              uint32_t attr_id,
                              int32_t rvi)
{
    if (NULL == pnids || NULL == pncls || NULL == pvars || NULL == pscs)
        return NULL;

    switch (attr_id)
    {
    case e_aid_NodeId:
        return util_variant__new_Variant_from_NodeId(pnids[rvi]);
    case e_aid_NodeClass:
        return util_variant__new_Variant_from_NodeClass(pncls[rvi]);
    case e_aid_Value:
        assert(address_space_bs__nViews >= 0);
        assert(address_space_bs__nObjects >= 0);
        assert(rvi >= address_space_bs__nViews + address_space_bs__nObjects);
        assert(rvi - (address_space_bs__nViews + address_space_bs__nObjects) <=
               address_space_bs__nVariables + address_space_bs__nVariableTypes);
        return util_variant__new_Variant_from_Variant(
            &pvars[rvi - (address_space_bs__nViews + address_space_bs__nObjects)]);
    default:
        return NULL;
    }
}

/** Returns false on not found or compare-error */
static bool get_rvi(SOPC_NodeId** pnids, SOPC_NodeId* target_nid, int32_t* prvi)
{
    if (NULL == pnids || NULL == target_nid || NULL == prvi)
        return false;

    int32_t i;
    int32_t comp;

    for (i = 1; i <= address_space_bs__nNodeIds; i++)
    {
        if (SOPC_STATUS_OK != SOPC_NodeId_Compare(pnids[i], target_nid, &comp))
            return false;
        if (comp == 0)
        {
            *prvi = i;
            return true;
        }
    }

    return true;
}

bool test_read_request_response(OpcUa_ReadResponse* pReadResp, SOPC_StatusCode statusCode, int verbose)
{
    printf("--> ReadRequest test result: ");
    if (verbose > 0)
        printf("\n");

    bool bTestOk = false;
    int32_t comp = 0;
    SOPC_Variant* pvar;
    int32_t i, rvi = 0;

    /* Check the service StatusCode */
    if (verbose > 0)
#ifdef __TRUSTINSOFT_BUGFIX__
      // minor: fix printf format
        printf("Service status code: %d (should be %d)\n", (int)statusCode, SOPC_STATUS_OK);
#else
        printf("Service status code: %" PRIX32 " (should be %d)\n", statusCode, SOPC_STATUS_OK);
#endif
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
        /* Find NodeId's rvi */
        bTestOk = get_rvi(address_space_bs__a_NodeId, &pReadReq->NodesToRead[i].NodeId, &rvi);
        /* Find desired attribute and wrap it in a new SOPC_Variant* */
        if (bTestOk)
        {
            pvar = new_variant_rvi(address_space_bs__a_NodeId, address_space_bs__a_NodeClass, address_space_bs__a_Value,
                                   address_space_bs__a_Value_StatusCode, pReadReq->NodesToRead[i].AttributeId, rvi);
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
