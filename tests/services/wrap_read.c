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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wrap_read.h"

#include "b2c.h"

/*#include "header.h"*/
#include "address_space.h"
#include "constants.h"
#include "io_dispatch_mgr.h"
#include "msg_read_request.h"

#include "address_space_bs.h"
#include "constants_bs.h"
#include "msg_read_request_bs.h"

#include "address_space_impl.h"
#include "testlib_read_response.h"

/* http://stackoverflow.com/questions/7265583/combine-designated-initializers-and-malloc-in-c99 */
#define DESIGNATE_NEW(T, ...) memcpy(malloc(sizeof(T)), &(T const){__VA_ARGS__}, sizeof(T))

const uint32_t N_READ_NODES = 7; // Read nodes with Node Id 1000 to 1000 + N_READ_NODES only
const uint32_t N_READ_VARS = 6;  // Read variables values with Node Id 1001 to 1001 + N_READ_VARS only

/**
 * Creates a request of length N_REQUEST
 */
OpcUa_ReadRequest* read_new_read_request(void)
{
    const uint32_t N_REQUESTS =
        N_READ_NODES * 2 + N_READ_VARS; // Read 2 attributes on each node + value attribute on each variable

    OpcUa_ReadValueId* lrv = (OpcUa_ReadValueId*) malloc(N_REQUESTS * sizeof(OpcUa_ReadValueId));
    uint32_t i;

    if (NULL == lrv)
        exit(1);

    /* All nodes Ids and classes */
    for (i = 0; i < N_READ_NODES; ++i)
    {
        /* Request for the NodeId (...) */
#ifdef __TRUSTINSOFT_TMPBUG__
      // avoid separated alarm when initializing a struct (maybe TRUS-1281?)
        OpcUa_ReadValueId tis_tmp_1 =
#else
        lrv[2 * i + 0] =
#endif
            (OpcUa_ReadValueId){.NodeId = {.IdentifierType = SOPC_IdentifierType_Numeric, .Data.Numeric = i + 1000},
                                .AttributeId = e_aid_NodeId,
                                .IndexRange = {.Length = 0},
                                .DataEncoding = {.Name.Length = 0}};
        /* Request for the NodeClass */
#ifdef __TRUSTINSOFT_TMPBUG__
        // avoid separated alarm when initializing a struct (maybe TRUS-1281?)
        lrv[2*i+0] = tis_tmp_1;
        OpcUa_ReadValueId tis_tmp_2 =
#else
        lrv[2 * i + 1] =
#endif
            (OpcUa_ReadValueId){.NodeId = {.IdentifierType = SOPC_IdentifierType_Numeric, .Data.Numeric = i + 1000},
                                .AttributeId = e_aid_NodeClass,
                                .IndexRange = {.Length = 0},
                                .DataEncoding = {.Name.Length = 0}};
#ifdef __TRUSTINSOFT_TMPBUG__
        // avoid separated alarm when initializing a struct (maybe TRUS-1281?)
        lrv[2*i+1] = tis_tmp_2;
#endif
    }

    /* Note: variables have the last numeric node ids in the @space used for this test*/
    /* All variables */
    for (i = 0; i < N_READ_VARS; ++i)
    {
        /* Request for the Value */
#ifdef __TRUSTINSOFT_TMPBUG__
        // avoid separated alarm when initializing a struct (maybe TRUS-1281?)
        OpcUa_ReadValueId tis_tmp = (OpcUa_ReadValueId) {
#else
        lrv[2 * N_READ_NODES + i] = (OpcUa_ReadValueId){
#endif
            .NodeId = {.IdentifierType = SOPC_IdentifierType_Numeric, .Data.Numeric = (N_READ_NODES - 1 - i) + 1000},
            .AttributeId = e_aid_Value,
            .IndexRange = {.Length = 0},
            .DataEncoding = {.Name.Length = 0}};
#ifdef __TRUSTINSOFT_TMPBUG__
        // avoid separated alarm when initializing a struct (maybe TRUS-1281?)
        lrv[2 * N_READ_NODES + i] = tis_tmp;
#endif
    }

#ifdef __TRUSTINSOFT_BUGFIX__
  // fix missing test of malloc return (TODO: report!) + TRUS-1281
    OpcUa_ReadRequest* pReadReq = malloc (sizeof (OpcUa_ReadRequest));
    if (NULL == pReadReq)
        exit(1);
    OpcUa_ReadRequest tis_tmp = (OpcUa_ReadRequest) { .encodeableType = &OpcUa_ReadRequest_EncodeableType,
                                                .MaxAge = 0., .TimestampsToReturn = OpcUa_TimestampsToReturn_Neither,
                                                .NoOfNodesToRead = N_REQUESTS, .NodesToRead = lrv};
    *pReadReq = tis_tmp;
#else
    OpcUa_ReadRequest* pReadReq = DESIGNATE_NEW(OpcUa_ReadRequest, .encodeableType = &OpcUa_ReadRequest_EncodeableType,
                                                .MaxAge = 0., .TimestampsToReturn = OpcUa_TimestampsToReturn_Neither,
                                                .NoOfNodesToRead = N_REQUESTS, .NodesToRead = lrv);
    if (NULL == pReadReq)
        exit(1);
#endif

    return pReadReq;
}

bool read_service_test(OpcUa_ReadRequest* pReadReq)
{
    bool bTest = false;
    constants__t_StatusCode_i status;

    /* Prepares the response message */
    OpcUa_ReadResponse readResp;

    /* Calls treat */
    service_mgr__treat_read_request((constants__t_msg_i) pReadReq, (constants__t_msg_i) &readResp, &status);

    /* Tests the response */
    if (constants__e_sc_ok == status)
    {
        /* TODO: this does not check anymore the service status code (because it is not accessible yet) */
        bTest = test_read_request_response(&readResp, SOPC_GoodGenericStatus, 1);
    }

    /* Don't forget to free your response */
    free(readResp.Results);

    return bTest;
}
