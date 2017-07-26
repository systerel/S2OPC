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


#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wrap_read.h"

#include "b2c.h"

/*#include "header.h"*/
#include "io_dispatch_mgr.h"
#include "address_space.h"
#include "constants.h"
#include "msg_read_request.h"

#include "address_space_bs.h"
#include "constants_bs.h"
#include "msg_read_request_bs.h"

#include "address_space_impl.h"
#include "gen_addspace.h"
#include "testlib_read_response.h"

#include "sopc_sc_events.h"

/* http://stackoverflow.com/questions/7265583/combine-designated-initializers-and-malloc-in-c99 */
#define DESIGNATE_NEW(T, ...)       \
  memcpy(malloc(sizeof(T)),         \
         &(T const){ __VA_ARGS__ }, \
         sizeof(T))


#define N_REQUESTS_VARS  120
#define N_REQUESTS_OTHERS 8
#define OFFSET_REQUESTS_OTHERS  (3*N_REQUESTS_VARS)
#define OFFSET_REQUESTS_LAST    (OFFSET_REQUESTS_OTHERS + 2*N_REQUESTS_OTHERS)
#define N_REQUESTS (OFFSET_REQUESTS_LAST + 1)

/**
 * Creates a request of length N_REQUEST
 */
OpcUa_ReadRequest *read_new_read_request(void)
{
    OpcUa_ReadValueId *lrv = (OpcUa_ReadValueId *)malloc(N_REQUESTS*sizeof(OpcUa_ReadValueId));
    size_t i;

    if(NULL == lrv)
        exit(1);

    /* First batch: variables */
    for(i=0; i<N_REQUESTS_VARS; ++i)
    {
        /* Request for the NodeId (...) */
        lrv[3*i+0] = (OpcUa_ReadValueId) {
            .NodeId = {
                .IdentifierType = IdentifierType_Numeric,
                .Data.Numeric = 10+i*(NB_NODES-8)/N_REQUESTS_VARS },
            .AttributeId = e_aid_NodeId,
            .IndexRange = {.Length = 0},
            .DataEncoding = {.Name.Length = 0} };
        /* Request for the NodeClass */
        lrv[3*i+1] = (OpcUa_ReadValueId) {
            .NodeId = {
                .IdentifierType = IdentifierType_Numeric,
                .Data.Numeric = 10+i*(NB_NODES-8)/N_REQUESTS_VARS },
            .AttributeId = e_aid_NodeClass,
            .IndexRange = {.Length = 0},
            .DataEncoding = {.Name.Length = 0} };
        /* Request for the Value */
        lrv[3*i+2] = (OpcUa_ReadValueId) {
            .NodeId = {
                .IdentifierType = IdentifierType_Numeric,
                .Data.Numeric = 10+i*(NB_NODES-8)/N_REQUESTS_VARS },
            .AttributeId = e_aid_Value,
            .IndexRange = {.Length = 0},
            .DataEncoding = {.Name.Length = 0} };
    }

    /* Second batch: other classes */
    for(i=0; i<N_REQUESTS_OTHERS; ++i)
    {
        /* Request for the NodeId (...) */
        lrv[OFFSET_REQUESTS_OTHERS+2*i+0] = (OpcUa_ReadValueId) {
            .NodeId = {
                .IdentifierType = IdentifierType_Numeric,
                .Data.Numeric = 10+1000+i },
            .AttributeId = e_aid_NodeId,
            .IndexRange = {.Length = 0},
            .DataEncoding = {.Name.Length = 0} };
        /* Request for the NodeClass */
        lrv[OFFSET_REQUESTS_OTHERS+2*i+1] = (OpcUa_ReadValueId) {
            .NodeId = {
                .IdentifierType = IdentifierType_Numeric,
                .Data.Numeric = 10+1000+i },
            .AttributeId = e_aid_NodeClass,
            .IndexRange = {.Length = 0},
            .DataEncoding = {.Name.Length = 0} };
    }

    /* Third batch: a variable without value <-- that should not exist */
    lrv[OFFSET_REQUESTS_LAST] = (OpcUa_ReadValueId) {
            .NodeId = {
                .IdentifierType = IdentifierType_Numeric,
                .Data.Numeric = 10+1002 },
            .AttributeId = e_aid_Value,
            .IndexRange = {.Length = 0},
            .DataEncoding = {.Name.Length = 0} };

    OpcUa_ReadRequest *pReadReq = DESIGNATE_NEW(OpcUa_ReadRequest,
            /*.RequestHeader = , */
            .MaxAge = 0.,
            .TimestampsToReturn = OpcUa_TimestampsToReturn_Neither,
            .NoOfNodesToRead = N_REQUESTS,
            .NodesToRead = lrv
        );
    if(NULL == pReadReq)
        exit(1);

    return pReadReq;
}


bool read_service_test(OpcUa_ReadRequest *pReadReq)
{
    bool bTest;

    /* Feeds the ReadRequest to the machine */
    SOPC_Toolkit_Msg msg_in = {
            .msgStruct = (void *)pReadReq,
        };

    /* Prepares the response message */
    OpcUa_ReadResponse readResp;
    SOPC_Toolkit_Msg msg_out = {
            .msgStruct = (void *)&readResp,
        };

    /* Calls treat */
    service_mgr__treat_read_request(
        (constants__t_msg_i)&msg_in,
        (constants__t_msg_i)&msg_out);

    /* Tests the response */
    /* TODO: this does not check anymore the service status code (because it is not accessible yet) */
    bTest = test_read_request_response(&readResp, constants__e_sc_ok, 1);

    /* Don't forget to free your response */
    free(readResp.Results);

    return bTest;
}
