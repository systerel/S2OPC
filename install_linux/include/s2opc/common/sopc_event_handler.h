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

#ifndef SOPC_EVENT_HANDLER_H_
#define SOPC_EVENT_HANDLER_H_

#include <stdint.h>

#include "sopc_enums.h"

/**
 * \brief Processes messages from a queue.
 *
 * An SOPC_EventHandler processes the messages from its queue with a callback
 * function. That callback function is called in the thread of the looper to
 * which the event handler is attached. Posting event is safe to do from other
 * threads.
 */
typedef struct _SOPC_EventHandler SOPC_EventHandler;

/**
 * \brief Manages the processing of events on a given thread.
 *
 * A SOPC_Looper owns a thread on which the callbacks from one or more event
 * handlers will be processed.
 */
typedef struct _SOPC_Looper SOPC_Looper;

/**
 * \brief Struct describing the various parts of an event.
 */
typedef struct
{
    int32_t event;
    uint32_t eltId;
    uintptr_t params;
    uintptr_t auxParam;
} SOPC_Event;

/**
 * \brief Function prototype for connecting an event emitter to a listener.
 */
typedef void SOPC_SetListenerFunc(SOPC_EventHandler* handler);

/**
 * \brief Function prototype for message processing callbacks.
 *
 * \p handler is a pointer to the \c SOPC_EventHandler invoking this callback,
 * the other parameters (and their lifetime) are specific to the event invoked.
 */
typedef void SOPC_EventHandler_Callback(SOPC_EventHandler* handler,
                                        int32_t event,
                                        uint32_t eltId,
                                        uintptr_t params,
                                        uintptr_t auxParam);

/**
 * \brief Creates a new event handler and attaches it to an existing looper.
 *
 * \param looper    the looper to attach the event handler to
 * \param callback  the callback to call to process incoming messages
 *
 * The returned handler belongs to the looper, and will be freed with it.
 *
 * \return The created event handler, or \c NULL on memory allocation failure.
 */
SOPC_EventHandler* SOPC_EventHandler_Create(SOPC_Looper* looper, SOPC_EventHandler_Callback* callback);

/**
 * \brief Posts an event to the back of the event handler's message queue.
 *
 * \param handler   the event handler
 * \param event     the event code
 * \param eltId     event specific data
 * \param params    event specific data
 * \param auxParam  event specific data
 *
 * \return \c SOPC_STATUS_OK on success, or an error code on failure.
 */
SOPC_ReturnStatus SOPC_EventHandler_Post(SOPC_EventHandler* handler,
                                         int32_t event,
                                         uint32_t eltId,
                                         uintptr_t params,
                                         uintptr_t auxParam);

/**
 * \brief Posts an event to the front of the event handler's message queue
 *
 * \param handler   the event handler
 * \param event     the event code
 * \param eltId     event specific data
 * \param params    event specific data
 * \param auxParam  event specific data
 *
 * \return \c SOPC_STATUS_OK on success, or an error code on failure.
 */
SOPC_ReturnStatus SOPC_EventHandler_PostAsNext(SOPC_EventHandler* handler,
                                               int32_t event,
                                               uint32_t eltId,
                                               uintptr_t params,
                                               uintptr_t auxParam);

/**
 *
 * \brief Creates a new looper and attaches it to a new thread.
 *
 *  \param threadName name of the thread
 *
 * \return The created looper, or \c NULL in case or error.
 */
SOPC_Looper* SOPC_Looper_Create(const char* threadName);

/**
 * \brief Stops a looper and releases all resources allocated to it.
 *
 * \param looper  the looper
 *
 * The queues of the attached handlers will be processed before the thread of
 * the looper is stopped.
 */
void SOPC_Looper_Delete(SOPC_Looper* looper);

#endif // SOPC_EVENT_HANDLER_H_
