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

#include "test_results.h"

#include "sopc_atomic.h"

static int32_t valid_service_result = 0;
static OpcUa_WriteRequest* current_pWriteRequest = NULL;

void test_results_set_service_result(t_bool res)
{
    SOPC_Atomic_Int_Set(&valid_service_result, res ? 1 : 0);
}

void test_results_set_WriteRequest(OpcUa_WriteRequest* pWriteReq)
{
    SOPC_Atomic_Ptr_Set((void**) &current_pWriteRequest, pWriteReq);
}

t_bool test_results_get_service_result(void)
{
    return SOPC_Atomic_Int_Get(&valid_service_result) == 1;
}

OpcUa_WriteRequest* test_results_get_WriteRequest(void)
{
    return (OpcUa_WriteRequest*) SOPC_Atomic_Ptr_Get((void**) &current_pWriteRequest);
}
