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


#ifndef TESTLIB_WRITE_H_
#define TESTLIB_WRITE_H_


#include <stdbool.h>

#include "sopc_types.h"

/**
 * Generates a new WriteRequest. Must be coherent with current Address Space (see gen_addspace.c).
 * It is also recommended to write values that are different that the default values from gen_addspace.
 * Do not try to write a value of a different type than the current type in the Address Space, it would work.
 */
OpcUa_WriteRequest *tlibw_new_WriteRequest(void);
/** One does not simply free a request, it must also free its content (NodesToWrite) and the content of its content (ByteString) */
void                tlibw_free_WriteRequest(OpcUa_WriteRequest **ppWriteReq);
/** Calls the depths of the B model to inject the request as if it was a client request, returns the service status */
bool                tlibw_stimulateB_with_message(void *pMsg);
/** Verifies that the effects of the Write are taken into account by the server, locally */
bool                tlibw_verify_effects_local(OpcUa_WriteRequest *pWriteReq);

/** Verifies that the response is ok and the response of each request is ok too */
bool                tlibw_verify_response(OpcUa_WriteRequest *pWriteReq, OpcUa_WriteResponse *pWriteResp);
/** Generates a new ReadRequest, that asks for the modified values. Must be coherent with tlibw_new_WriteRequest... */
OpcUa_ReadRequest  *tlibw_new_ReadRequest_check(void);
/** Client-side, after sending the tlib_new_ReadRequest, verifies the ReadResponse against the initial WriteRequest. Values should match. */
bool                tlibw_verify_response_remote(OpcUa_WriteRequest *pWriteReq, OpcUa_ReadResponse *pReadResp);


#endif /* TESTLIB_WRITE_H_ */
