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
#include "sopc_time_reference.h"

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
} SOPC_LooperEvent;

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

/** \brief Number of categories of pending events counted by a looper, see ::SOPC_EventHandler_CreateWithCategory */
#define SOPC_EVENT_HANDLER_NB_CATEGORIES 4

/** \brief Category of an event which is not counted, see ::SOPC_EventHandler_CategoryFct */
#define SOPC_EVENT_HANDLER_NO_CATEGORY (-1)

/**
 * \brief Function prototype returning the category of an event posted to an event handler: the looper counts the
 *        pending events of each category (see ::SOPC_EventHandler_Get_PendingCountByCategory).
 *
 * Called by the posting thread with the parameters of the posted event: it shall be short and shall not call any
 * event handler function.
 *
 * \return The category of the event in [0, ::SOPC_EVENT_HANDLER_NB_CATEGORIES[, or ::SOPC_EVENT_HANDLER_NO_CATEGORY
 *         if the event is not counted.
 */
typedef int32_t SOPC_EventHandler_CategoryFct(int32_t event, uint32_t eltId, uintptr_t params, uintptr_t auxParam);

/**
 * \brief Function prototype freeing the parameters of an event posted to an event handler and never treated
 *        (see ::SOPC_Looper_Delete).
 */
typedef void SOPC_EventHandler_FreeEventFct(int32_t event, uint32_t eltId, uintptr_t params, uintptr_t auxParam);

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
 * \brief Creates a new event handler whose posted events are counted by category while pending, and attaches it to
 *        an existing looper.
 *
 * \param looper       the looper to attach the event handler to
 * \param callback     the callback to call to process incoming messages
 * \param categoryFct  the function returning the category of a posted event
 *
 * The returned handler belongs to the looper, and will be freed with it.
 *
 * \return The created event handler, or \c NULL on invalid parameter or memory allocation failure.
 */
SOPC_EventHandler* SOPC_EventHandler_CreateWithCategory(SOPC_Looper* looper,
                                                        SOPC_EventHandler_Callback* callback,
                                                        SOPC_EventHandler_CategoryFct* categoryFct);

/**
 * \brief Returns the number of pending events of a category in the queue of the looper of \p handler: posted (by
 *        any handler of the looper) and not dequeued yet, an event being dequeued before its treatment.
 *
 * \param handler   an event handler of the looper
 * \param category  the category in [0, ::SOPC_EVENT_HANDLER_NB_CATEGORIES[
 *
 * \return The number of pending events of \p category, 0 on invalid parameter.
 */
uint32_t SOPC_EventHandler_Get_PendingCountByCategory(SOPC_EventHandler* handler, int32_t category);

/**
 * \brief Sets the function freeing the parameters of the events posted to \p handler and never treated.
 *
 * \param handler  the event handler
 * \param freeFct  the function called by ::SOPC_Looper_Delete for each untreated event of \p handler
 *
 * \note Shall be called before any event is posted to \p handler.
 */
void SOPC_EventHandler_SetFreeEventFct(SOPC_EventHandler* handler, SOPC_EventHandler_FreeEventFct* freeFct);

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
 *
 * \note An event posted after ::SOPC_Looper_Stop is accepted but never treated: its parameters are freed by
 *       ::SOPC_Looper_Delete if a free function is set (see ::SOPC_EventHandler_SetFreeEventFct).
 */
SOPC_ReturnStatus SOPC_EventHandler_Post(SOPC_EventHandler* handler,
                                         int32_t event,
                                         uint32_t eltId,
                                         uintptr_t params,
                                         uintptr_t auxParam);

/** \brief Returns the current pending number of events in queue
 * \param handler the event handler
 *
 * \return the current pending number of events in queue
 */
uint32_t SOPC_EventHandler_Get_QueueSize(SOPC_EventHandler* handler);

/**
 * \brief Returns the time reference at which the event currently processed by \p handler was posted.
 *
 * The post time is recorded (monotonic ::SOPC_TimeReference) when the event is posted with
 * ::SOPC_EventHandler_Post or ::SOPC_EventHandler_PostAsNext. It allows the callback to know
 * how long the event waited in the queue before being processed.
 *
 * \param handler  the event handler
 *
 * \return the post time of the event being processed by \p handler, or 0 if no event is being processed.
 *
 * \note This function shall only be called from the callback of \p handler (looper thread),
 *       the value is only valid during the callback execution.
 */
SOPC_TimeReference SOPC_EventHandler_GetCurrentEventPostTime(const SOPC_EventHandler* handler);

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
 * \return The created looper, or \c NULL in case of error.
 */
SOPC_Looper* SOPC_Looper_Create(const char* threadName);

/**
 * \brief Creates a new looper and attaches it to a new thread with optional priority and CPU affinity.
 *
 * \param threadName      name of the thread
 * \param threadPriority  thread priority (0 means default priority, see ::SOPC_Thread_CreatePrioritized)
 * \param cpuAffinity     CPU affinity (-1 means unset, see ::SOPC_Thread_CreatePrioritized)
 *
 * \return The created looper, or \c NULL in case of error.
 */
SOPC_Looper* SOPC_Looper_CreatePrioritized(const char* threadName, int threadPriority, int cpuAffinity);

/**
 * \brief Stops the thread of a looper without releasing its resources.
 *
 * \param looper  the looper
 *
 * The events posted before this call are treated, then the thread of the looper is joined.
 * Its event handlers remain valid: an event posted afterwards is accepted but never treated (see
 * ::SOPC_Looper_Delete).
 * Calling it on a stopped looper has no effect.
 *
 * \note Not thread-safe with ::SOPC_Looper_Stop and ::SOPC_Looper_Delete on the same looper.
 */
void SOPC_Looper_Stop(SOPC_Looper* looper);

/**
 * \brief Stops a looper if not already done (see ::SOPC_Looper_Stop) and releases all resources allocated to it.
 *
 * \param looper  the looper
 *
 * The events posted after the stop are freed without being treated, with their parameters when the handler has a free
 * function (see ::SOPC_EventHandler_SetFreeEventFct). Their number is logged as a warning.
 *
 * \warning All event handlers are deallocated by this function: no thread shall use them anymore.
 */
void SOPC_Looper_Delete(SOPC_Looper* looper);

#endif // SOPC_EVENT_HANDLER_H_
