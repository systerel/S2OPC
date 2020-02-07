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

#ifndef TESTLIB_WRITE_H_
#define TESTLIB_WRITE_H_

#include <stdbool.h>

#include "sopc_address_space.h"
#include "sopc_types.h"

/**
 * Generates a new WriteRequest. Must be coherent with current Address Space (see gen_addspace.c).
 * It is also recommended to write values that are different that the default values from gen_addspace.
 * Do not try to write a value of a different type than the current type in the Address Space, it would work.
 *
 * Note: address_space is necessary to know if the status code is modifiable or not (const address space case)
 */
OpcUa_WriteRequest* tlibw_new_WriteRequest(const SOPC_AddressSpace* address_space);
/** One does not simply free a request, it must also free its content (NodesToWrite) and the content of its content
 * (ByteString) */
void tlibw_free_WriteRequest(OpcUa_WriteRequest** ppWriteReq);

/** Verifies that the response is ok and the response of each request is ok too */
bool tlibw_verify_response(OpcUa_WriteRequest* pWriteReq, OpcUa_WriteResponse* pWriteResp);
/** Generates a new ReadRequest, that asks for the modified values. Must be coherent with tlibw_new_WriteRequest... */
OpcUa_ReadRequest* tlibw_new_ReadRequest_check(void);
/** Client-side, after sending the tlib_new_ReadRequest, verifies the ReadResponse against the initial WriteRequest.
 * Values should match. */
bool tlibw_verify_response_remote(OpcUa_WriteRequest* pWriteReq, OpcUa_ReadResponse* pReadResp);

#endif /* TESTLIB_WRITE_H_ */
