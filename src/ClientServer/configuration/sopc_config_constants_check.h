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

#ifndef SOPC_CONFIG_CONSTANTS_CHECK_H_
#define SOPC_CONFIG_CONSTANTS_CHECK_H_

#include <stdint.h>

#include "sopc_event_timer_manager.h"
#include "sopc_toolkit_config_constants.h"

/* Maximum value accepted in B model */
#if SOPC_MAX_ENDPOINT_DESCRIPTION_CONFIGURATIONS > INT32_MAX
#error "Max number of endpoint descriptions cannot be more than INT32_MAX"
#endif

#if SOPC_MAX_SECURE_CONNECTIONS_PLUS_BUFFERED > SOPC_MAX_SOCKETS
#error "Max number of secure connections cannot be greater than max number of sockets"
#endif

/* Maximum value accepted in B model */
#if SOPC_MAX_SECURE_CONNECTIONS_PLUS_BUFFERED > INT32_MAX
#error "Max number of secure connections cannot be more than INT32_MAX"
#endif

/* Maximum value accepted in B model */
#if SOPC_MAX_SESSIONS > INT32_MAX
#error "Max number of sessions cannot be more than INT32_MAX"
#endif

/* Maximum session timeout accepted */
#if SOPC_MAX_SESSION_TIMEOUT > UINT32_MAX
#error "Maximum requested session timeout is > UINT32_MAX"
#endif
#if SOPC_MAX_SESSION_TIMEOUT < SOPC_MIN_SESSION_TIMEOUT
#error "Maximum requested session timeout is < MIN"
#endif

/* Minimum session timeout accepted */
#if SOPC_MIN_SESSION_TIMEOUT < 10000
#error "Minimum requested session timeout is < 10000"
#endif
#if SOPC_MIN_SESSION_TIMEOUT > SOPC_MAX_SESSION_TIMEOUT
#error "Minimum requested session timeout is > MAX"
#endif

/** Minimum number of publish requests per subscription shall be > 0 for B model */
#if SOPC_MAX_SUBSCRIPTION_PUBLISH_REQUESTS <= 0
#error "Minimum subscription publish requests <= 0"
#endif

/** Maximum number of publish requests per subscription shall be in NAT for B model
 * Note: max republish notifs == 2 * max publish requests */
#if SOPC_MAX_SUBSCRIPTION_PUBLISH_REQUESTS > INT32_MAX / 2
#error "Maximum subscription publish requests > INT32_MAX / 2"
#endif

/** Minimum publish interval shall be greater to the event timer resolution */
#if 2 * SOPC_TIMER_RESOLUTION_MS > SOPC_MIN_SUBSCRIPTION_INTERVAL_DURATION
#error "Minimum publish interval < 2 * SOPC_TIMER_RESOLUTION_MS"
#endif

/** Minimum number of publish intervals before a keep alive is sent (server to client) */
#if SOPC_MIN_KEEPALIVE_PUBLISH_INTERVALS < 1
#error "Minimum subscription keep alive intervals < 1"
#endif

/** Maximum number of publish intervals before a keep alive is sent (server to client) */
#if SOPC_MAX_KEEPALIVE_PUBLISH_INTERVALS > INT32_MAX
#error "Maximum subscription keep alive intervals > INT32_MAX"
#endif

/** Maximum number of publish intervals before lifetime expired (client to server communication) */
#if SOPC_MAX_LIFETIME_PUBLISH_INTERVALS > INT32_MAX
#error "Maximum subscription lifetime intervals > INT32_MAX"
#endif

/** Minimum number of publish intervals before lifetime expired shall be 3 times bigger than keep alive max value */
#if SOPC_MIN_LIFETIME_PUBLISH_INTERVALS < 3 * SOPC_MIN_KEEPALIVE_PUBLISH_INTERVALS
#error "Minimum subscription lifetime intervals < 3 * SOPC_MIN_KEEPALIVE_PUBLISH_INTERVALS"
#endif

/** Maximum number of publish intervals before lifetime expired shall be 3 times bigger than keep alive max value */
#if SOPC_MAX_LIFETIME_PUBLISH_INTERVALS < 3 * SOPC_MAX_KEEPALIVE_PUBLISH_INTERVALS
#error "Maximum subscription lifetime intervals < 3 * SOPC_MAX_KEEPALIVE_PUBLISH_INTERVALS"
#endif

/** @brief Maximum number of monitored items per subscription */
#if SOPC_MAX_MONITORED_ITEM > INT32_MAX
#error "Maximum number of monitored items> INT32_MAX"
#endif

/* Maximum number of operations representable */
#if SOPC_MAX_OPERATIONS_PER_MSG > INT32_MAX
#error "Maximum number of operations per message cannot be > INT32_MAX"
#endif

#endif
