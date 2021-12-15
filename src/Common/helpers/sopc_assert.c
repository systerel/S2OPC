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

#include <assert.h>
#include <stdio.h>

#include "sopc_assert.h"
#include "sopc_logger.h"
#include "sopc_time.h"

// Note : This indirection aims at avoiding the rule checker to detect the only allowed call to assert
#define REAL_ASSERT assert

static SOPC_Assert_UserCallback gUserCallback = NULL;

void SOPC_Assert_Failure(const char* context)
{
    static bool once = true;
    if (once)
    {
        // Avoid recursive calls
        once = false;
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON, "Assertion failed. Context = %s", context);
        if (NULL != gUserCallback)
        {
            gUserCallback(context);
        }
        else
        {
            // Note that assertions may happen before logs are initialized, so that we must ensure
            // an error message is at least displayed somewhere...
            // We can assume that a specific application that uses "SOPC_Assert_Set_UserCallback" will
            // manage the message by its own.
            printf("Assertion failed. Context = %s\n", context);
        }
    }
    REAL_ASSERT(false);
}

void SOPC_Assert_Set_UserCallback(SOPC_Assert_UserCallback callback)
{
    gUserCallback = callback;
}
