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
 * Test the ReadRequest.
 * Tests that a received request is conform to the address space (that will be reproduced) and the sent request.
 *
 * This test is laborious to maintain, as it requires the re-implementation of specific functionalities used in the B
 * base machines.
 */

#ifndef TEST_READ_RESPONSE_H_
#define TEST_READ_RESPONSE_H_

#include <stdbool.h>
#include <stdio.h>

#include "wrap_read.h"

#include "sopc_types.h"

/* Uncomment the following to print more verbose info */
/*#define VERBOSE*/

/**
 * Test the provided ReadResponse against a locally created address space and request
 *  (the test is independent from the B model, even if the address space and request
 *  generation are the same C functions).
 * Verbosity:
 *  0 -> only test result
 *  1 -> 0 + prints the response
 *  2 -> 1 + prints the comparisons
 * Returns true when ReadResponse is deemed ok.
 */
bool test_read_request_response(OpcUa_ReadResponse* pReadResp, SOPC_StatusCode status_code, int verbose_level);

#endif /* TEST_READ_RESPONSE_H_ */
