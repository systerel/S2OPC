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
