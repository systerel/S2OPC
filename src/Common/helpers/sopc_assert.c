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
#include <string.h>

#include "sopc_assert.h"
#include "sopc_common_constants.h"
#include "sopc_logger.h"
#include "sopc_time.h"

// Note : This indirection aims at avoiding the rule checker to detect the only allowed call to assert
#define REAL_ASSERT assert
#define MAX_CONTEXT_LEN 80u

static SOPC_Assert_UserCallback* gUserCallback = NULL;

void SOPC_Assert_Failure(const char* context)
{
    static bool once = true;
    if (once)
    {
        // Avoid recursive calls
        once = false;
        if (NULL == context)
        {
            context = "<NULL>";
        }

        SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON, "Assertion failed. Context = %s", context);
        if (NULL != gUserCallback)
        {
            gUserCallback(context);
        }
        else
        {
            const size_t len = strlen(context);
            // MAX_CONTEXT_LEN is used because some specific "print" implementation may truncate the
            // output if exceeding a given size. So as to ensure that the displayed part of context
            // (containing the file/line failed assertion reference) contains the useful information,
            // the end of the context is privileged. (because the file name and line is at the end of
            // the context)
            if (len > MAX_CONTEXT_LEN)
            {
                context = &context[len - MAX_CONTEXT_LEN];
            }
            // Note that assertions may happen before logs are initialized, so that we must ensure
            // an error message is at least displayed somewhere...
            // We can assume that a specific application that uses "SOPC_Assert_Set_UserCallback" will
            // manage the message by its own.

            SOPC_CONSOLE_PRINTF("Assertion failed. Context = \n");
            SOPC_CONSOLE_PRINTF("%s", context);
            SOPC_CONSOLE_PRINTF("\n");
        }
    }
    REAL_ASSERT(false);
}

void SOPC_Assert_Set_UserCallback(SOPC_Assert_UserCallback* callback)
{
    gUserCallback = callback;
}
