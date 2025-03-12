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

/** \file
 *
 * \brief High level interface that allows S2OPC introspective monitoring
 *
 */

#ifndef LIBS2OPC_COMMON_MONITORING_H_
#define LIBS2OPC_COMMON_MONITORING_H_

#include <stdbool.h>
#include <stdint.h>

/**
 * \brief List of internal S2OPC queues that can be monitored
 */
typedef enum
{
    SOPC_CommonMonitoring_QueueType_Services,        /**< The "Services" processing Queue*/
    SOPC_CommonMonitoring_QueueType_SecuredChannels, /**< The "SecuredChannels" processing Queue*/
    SOPC_CommonMonitoring_QueueType_Sockets          /**< The "Sockets" processing Queue*/
} SOPC_CommonMonitoring_QueueType;

/** Number of pending items in a processing queue */
typedef uint32_t SOPC_CommonMonitoring_QueueSize;

/**
 * \brief Requests the current queue pending size. This allows the user application to monitor the pending events in
 * different queues. This is an estimation of the size queue. The actual value may have changed when the function
 * returns, since the queues are processed in independent threads.
 * Shall be called between ::SOPC_CommonHelper_Initialize and ::SOPC_CommonHelper_Clear.
 *
 * \note See also ::SOPC_MAX_NB_ELEMENTS_ASYNC_QUEUE and ::SOPC_MAX_NB_ELEMENTS_ASYNC_QUEUE_WARNING_ONLY.
 *
 * \param queueType the queue to fetch.
 *
 * \return The number of pending elements in the queue identified by \p queueType. Returns 0 if S2OPC has not been
 * initialized or if the input is invalid.
 */
SOPC_CommonMonitoring_QueueSize SOPC_CommonMonitoring_GetQueueSize(SOPC_CommonMonitoring_QueueType queueType);

#endif
