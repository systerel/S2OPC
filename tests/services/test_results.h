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

#ifndef _test_results_h
#define _test_results_h

#include "sopc_types.h"
#include "b2c.h"

void test_results_set_service_result(t_bool res);
void test_results_set_WriteRequest(OpcUa_WriteRequest *pWriteReq);

t_bool test_results_get_service_result(void);
OpcUa_WriteRequest *test_results_get_WriteRequest(void);

#endif
