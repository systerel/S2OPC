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

/** \file sopc_askpass.c
 *
 * \brief A platform independent API to ask for passwords.
 */

#include "sopc_platform_time.h"

#include "sopc_mem_alloc.h"

/***************************************************/
SOPC_RealTime* SOPC_RealTime_Create(const SOPC_RealTime* copy)
{
    SOPC_RealTime* ret = SOPC_Calloc(1, sizeof(SOPC_RealTime));
    if (NULL != copy && NULL != ret)
    {
        *ret = *copy;
    }
    else if (NULL != ret)
    {
        bool ok = SOPC_RealTime_GetTime(ret);
        if (!ok)
        {
            SOPC_RealTime_Delete(&ret);
        }
    }

    return ret;
}

/***************************************************/
void SOPC_RealTime_Delete(SOPC_RealTime** t)
{
    if (NULL == t)
    {
        return;
    }
    SOPC_Free(*t);
    *t = NULL;
}
