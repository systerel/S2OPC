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
 */


#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "constants.h"

#include "address_space_impl.h"
#include "testlib_read_response.h"
#include "address_space_impl.h"
#include "gen_addspace.h"

/**
 * You should free() the returned Variant* afterwards.
 */
SOPC_Variant* new_variant_rvi(constants__t_NodeId_i     *pnids,
                              constants__t_NodeClass_i  *pncls,
                              constants__t_Variant_i    pvars,
                              constants__t_StatusCode_i *pscs,
                              uint32_t                  attr_id,
                              size_t                    rvi)
{
    if(NULL == pnids ||
       NULL == pncls ||
       NULL == pvars ||
       NULL == pscs)
        return NULL;

    switch(attr_id)
    {
    case e_aid_NodeId:
        return util_variant__new_Variant_from_NodeId(pnids[rvi]);
    case e_aid_NodeClass:
        return util_variant__new_Variant_from_NodeClass(pncls[rvi]);
    case e_aid_Value:
        assert(address_space_bs__nViews >= 0);
        assert(address_space_bs__nObjects >= 0);
        assert(rvi >= (uint32_t) address_space_bs__nViews + address_space_bs__nObjects);
        assert(rvi - (uint32_t) (address_space_bs__nViews + address_space_bs__nObjects) <= (uint32_t) address_space_bs__nVariables + address_space_bs__nVariableTypes);
        return util_variant__new_Variant_from_Variant
                ((SOPC_Variant*) &pvars[rvi - (uint32_t) (address_space_bs__nViews + address_space_bs__nObjects)]);
    default:
        return NULL;
    }
}


/** Returns NOK on not found or compare-error */
SOPC_StatusCode get_rvi(constants__t_NodeId_i *pnids,
                        SOPC_NodeId *target_nid,
                        size_t *prvi)
{
    if(NULL == pnids ||
       NULL == target_nid ||
       NULL == prvi)
        return STATUS_INVALID_PARAMETERS;

    int32_t i;
    int32_t comp;

    for(i=1; i <= address_space_bs__nNodeIds; i++)
    {
        if(STATUS_OK != SOPC_NodeId_Compare((SOPC_NodeId *)pnids[i], target_nid, &comp))
            return STATUS_NOK;
        if(comp == 0) {
            *prvi = i;
            return STATUS_OK;
        }
    }

    return STATUS_NOK;
}


bool test_read_request_response(OpcUa_ReadResponse *pReadResp,
                                constants__t_StatusCode_i status_code,
                                int verbose)
{
    printf("--> ReadRequest test result: ");
    if(verbose > 0)
        printf("\n");

    bool bTestOk = false;
    int32_t comp = 0;
    SOPC_Variant *pvar;
    size_t i, rvi = 0;

    /* Check the service StatusCode */
    if(verbose > 0)
        printf("Service status code: %d (should be %d)\n", status_code, constants__e_sc_ok);
    bTestOk = constants__e_sc_ok == status_code;

    /* Creates a Request */
    OpcUa_ReadRequest *pReadReq = read_new_read_request();

    /* Prints the Response */
    if(verbose > 0)
    {
        printf("pReadResp->NoOfResults: %d\n", pReadResp->NoOfResults);
        for(i=0; i<(size_t)pReadResp->NoOfResults; ++i) {
            /* Note: Status is a B-StatusCode */
            printf("pReadResp->Results[%lu].Status: 0x%08X\n", i, pReadResp->Results[i].Status);
            util_variant__print_SOPC_Variant(&pReadResp->Results[i].Value);
        }
    }

    /* Test number of results */
    if(bTestOk)
        bTestOk = pReadReq->NoOfNodesToRead == pReadResp->NoOfResults;

    /* Analyze each response element */
    for(i=0; bTestOk && i<(size_t)pReadReq->NoOfNodesToRead; ++i) {
        /* Find NodeId's rvi */
        bTestOk = STATUS_OK == get_rvi(address_space_bs__a_NodeId, &pReadReq->NodesToRead[i].NodeId, &rvi);
        /* Find desired attribute and wrap it in a new SOPC_Variant* */
        if(bTestOk){
            pvar = new_variant_rvi(address_space_bs__a_NodeId, 
                                   address_space_bs__a_NodeClass, 
                                   address_space_bs__a_Value, 
                                   address_space_bs__a_Value_StatusCode,
                                   pReadReq->NodesToRead[i].AttributeId, 
                                   rvi);
        }else{
            pvar = NULL;
        }
        /* Compares the wrapped value with the response to the request */
        bTestOk = bTestOk && STATUS_OK == SOPC_Variant_Compare(&pReadResp->Results[i].Value,
                                                               pvar,
                                                               &comp);
        bTestOk = bTestOk && comp == 0;
        if(verbose > 1)
        {
            printf("-- Comparing pvar:\n");
            util_variant__print_SOPC_Variant(pvar);
            printf("-- with response[%zd]:\n", i);
            util_variant__print_SOPC_Variant(&pReadResp->Results[i].Value);
            if(bTestOk)
                printf("-- ok\n");
            else
                printf("-- nok\n");
        }
        free(pvar); /* It's ok to free a NULL */
    }

    /* Free the Request */
    free(pReadReq->NodesToRead);
    free(pReadReq);

    if(bTestOk)
        printf("OK\n");
    else
        printf("NOT ok\n");

    return bTestOk;
}
