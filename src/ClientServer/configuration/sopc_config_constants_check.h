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

/* It is expected by specification that server supports one more SecureChannel than Sessions*/
#if SOPC_MAX_SECURE_CONNECTIONS <= SOPC_MAX_SESSIONS
#error "Maximum number of secure channel shall be greater than maximum number of session"
#endif

/* Maximum value accepted in B model */
#if SOPC_MAX_SESSIONS > INT32_MAX
#error "Max number of sessions cannot be more than INT32_MAX"
#endif

/* Maximum number of session per secure connection */
#if SOPC_MAX_SESSIONS_PER_SECURE_CONNECTION < 0
#error "Maximum number of session per secure connection cannot be < 0"
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

/* Maximum number of session authentification attempts */
#if SOPC_MAX_SESSION_AUTH_ATTEMPTS < 0
#error "Maximum number of session authentification attempts cannot be < 0"
#endif

/* Create session lock delay secs */
#if SOPC_CREATE_SESSION_LOCK_DELAY_SECS < 0
#error "Number of create session lock delay secs cannot be < 0"
#endif

/* Number of session activation min delay secs */
#if SOPC_SESSION_ACTIVATION_MIN_DELAY_SECS < 0
#error "Number of session activation min delay secs cannot be < 0"
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

/* Maximum number of operations representable */
#if SOPC_MAX_OPERATIONS_PER_MSG > INT32_MAX
#error "Maximum number of operations per message cannot be > INT32_MAX"
#endif

/* Minimum number of operations representable */
#if SOPC_MAX_OPERATIONS_PER_MSG < 0
#error "Minimum number of operations per message cannot be < 0"
#endif

/* Maximum number of heavy operations per messages*/
#if SOPC_MAX_HEAVY_OPERATIONS_PER_MSG < 0
#error "Maximum number of heavy operations per messages cannot be < 0"
#endif

/* Maximum number of event notification queue size */
#if SOPC_MAX_NOTIFICATION_QUEUE_SIZE <= 0
#error "Maximum number of event notification queue size cannot be <= 0"
#endif

/* Minimum number of event notification queue size */
#if SOPC_MIN_EVENT_NOTIFICATION_QUEUE_SIZE <= 0
#error "Minimum number of event notification queue size cannot be <= 0"
#endif

/* Default number of event notification queue size */
#if SOPC_DEFAULT_EVENT_NOTIFICATION_QUEUE_SIZE <= 0
#error "Default number of event notification queue size cannot be <= 0"
#endif

/* Maximum number of tranlate browse path matches */
#if SOPC_MAX_TRANSLATE_BROWSE_PATH_MATCHES < 0
#error "Maximum number of tranlate browse path matches cannot be < 0"
#endif

/** Check Node management services activation for clients is set only in case of nano extended services */
#if S2OPC_NODE_MANAGEMENT && S2OPC_NANO_PROFILE != false
#error "Node management services cannot be activated for clients with S2OPC_NANO_PROFILE variable set"
#endif

#endif
