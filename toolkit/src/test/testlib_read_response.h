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
 * Test the ReadRequest.
 * Tests that a received request is conform to the address space (that will be reproduced) and the sent request.
 *
 * This test is laborious to maintain, as it requires the re-implementation of specific functionalities used in the B base machines.
 */


#ifndef _test_read_response_h
#define _test_read_response_h


#include <stdbool.h>
#include <stdio.h>

#include "wrap_read.h"
#include "util_variant.h"

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
bool test_read_request_response(OpcUa_ReadResponse *pReadResp,
                                constants__t_StatusCode_i status_code,
                                int verbose_level);


#endif /* _test_read_response_h */
