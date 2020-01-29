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

#include "event_helpers.h"

#include <assert.h>
#include <inttypes.h>
#include <stdio.h>

#include "sopc_time.h"

// Sleep timeout in milliseconds
static const uint32_t sleepTimeout = 10;
// Loop timeout in milliseconds
static const uint32_t loopTimeout = 2000;

void WaitEvent(SOPC_AsyncQueue* queue, void** event)
{
    assert(queue != NULL);

    for (uint32_t loopCount = 0; loopCount * sleepTimeout <= loopTimeout; ++loopCount)
    {
        SOPC_ReturnStatus status = SOPC_AsyncQueue_NonBlockingDequeue(queue, event);

        if (status == SOPC_STATUS_OK)
        {
            return;
        }
        else if (status == SOPC_STATUS_WOULD_BLOCK)
        {
            SOPC_Sleep(sleepTimeout);
        }
        else
        {
            assert(false && "Error while waiting for event");
        }
    }

    assert(false && "Timeout while waiting for socket event");
}

bool CheckEvent(const char* event_type,
                SOPC_Event* event,
                int32_t expected_event,
                uint32_t expected_id,
                uintptr_t expected_aux)
{
    if (event->event == expected_event && event->auxParam == expected_aux && event->eltId == expected_id)
    {
        return true;
    }
    else
    {
        printf("Unexpected %s event received event=%" PRIi32 " (expected %" PRIi32 "), eltId=%" PRIu32
               " (expected "
               "%" PRIu32 "), auxParam=%" PRIuPTR " (expected %" PRIuPTR ")\n",
               event_type, event->event, expected_event, event->eltId, expected_id, event->auxParam, expected_aux);
        return false;
    }
}

bool CheckEventAllParams(const char* event_type,
                         SOPC_Event* event,
                         int32_t expected_event,
                         uint32_t expected_id,
                         uintptr_t expected_param,
                         uintptr_t expected_aux)
{
    if (CheckEvent(event_type, event, expected_event, expected_id, expected_aux))
    {
        if (event->params == expected_param)
        {
            return true;
        }
        else
        {
            printf("Unexpected %s event received event=%" PRIi32 ", eltId=%" PRIu32 " , param=%" PRIuPTR
                   "(expected %" PRIuPTR "), auxParam=%" PRIuPTR "\n",
                   event_type, event->event, event->eltId, event->params, expected_param, event->auxParam);
            return false;
        }
    }
    return false;
}
