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

#ifndef SOPC_EVENT_RECORDER_H
#define SOPC_EVENT_RECORDER_H

#include "sopc_async_queue.h"
#include "sopc_event_handler.h"

/**
 * \brief Simple event recording facility.
 *
 * An SOPC_EventRecorder spins up a looper and sets up a standard event handler.
 * For each call to the event handler, the parameters are wrapped into an
 * \ref SOPC_Event and posted to the async queue. It is a convenient way to
 * collect emitted events back to the main thread when doing tests.
 */
typedef struct _SOPC_EventRecorder
{
    SOPC_Looper* looper;
    SOPC_EventHandler* eventHandler;
    SOPC_AsyncQueue* events;
} SOPC_EventRecorder;

/**
 * \brief Creates a new event recorder.
 * \return The event recorder, or \c NULL in case of error.
 *
 * The \c eventHandler of this recorder will for each call post an
 * \ref SOPC_Event to the \c events async queue.
 */
SOPC_EventRecorder* SOPC_EventRecorder_Create(void);

/**
 * \brief Stops an event recorder and releases all resources associated to it.
 * \param recorder  The event recorder.
 */
void SOPC_EventRecorder_Delete(SOPC_EventRecorder* recorder);

#endif
