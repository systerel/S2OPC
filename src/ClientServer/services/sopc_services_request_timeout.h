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

/**
 *  \file sopc_services_request_timeout.h
 *
 *  \brief Services layer internal helpers to evaluate the request header TimeoutHint on server side.
 *
 *  The reception time of the message currently processed by the Services layer is the time at which the
 *  Secure Channels layer posted it to the Services layer (::SC_SERVICE_RCV_MSG event post time).
 *  It is set by the Services layer before the message is provided to the B model and reset after,
 *  thus it is only available in the Services thread during the treatment of a received message.
 */

#ifndef SOPC_SERVICES_REQUEST_TIMEOUT_H_
#define SOPC_SERVICES_REQUEST_TIMEOUT_H_

#include <stdbool.h>
#include <stdint.h>

#include "sopc_time_reference.h"

/**
 * \brief Sets the reception time of the message currently treated by the Services layer.
 *
 * \param receptionTime  The monotonic reception time of the message
 *
 * \note Shall only be called from the Services thread.
 */
void SOPC_Services_SetCurrentMsgReceptionTime(SOPC_TimeReference receptionTime);

/**
 * \brief Resets the reception time of the message currently treated by the Services layer (no message treated).
 *
 * \note Shall only be called from the Services thread.
 */
void SOPC_Services_ResetCurrentMsgReceptionTime(void);

/**
 * \brief Gets the reception time of the message currently treated by the Services layer.
 *
 * \param[out] receptionTime  The monotonic reception time of the message currently treated
 *
 * \return true if a message is currently treated and \p receptionTime has been set, false otherwise.
 *
 * \note Shall only be called from the Services thread.
 */
bool SOPC_Services_GetCurrentMsgReceptionTime(SOPC_TimeReference* receptionTime);

/**
 * \brief Evaluates if a request TimeoutHint is elapsed.
 *
 * A request is considered expired when its TimeoutHint is not 0 (0 means no timeout) and
 * (\p currentTime - \p receptionTime) + \p marginMs > \p timeoutHintMs.
 *
 * \param receptionTime  The monotonic reception time of the request
 * \param currentTime    The monotonic current time
 * \param timeoutHintMs  The request header TimeoutHint in milliseconds (0 means no timeout)
 * \param marginMs       The margin in milliseconds used to anticipate the expiration
 *
 * \return true if the request is expired, false otherwise
 *          (also false if \p currentTime is before \p receptionTime).
 */
bool SOPC_Services_IsRequestTimeoutHintExpired(SOPC_TimeReference receptionTime,
                                               SOPC_TimeReference currentTime,
                                               uint32_t timeoutHintMs,
                                               uint32_t marginMs);

#endif /* SOPC_SERVICES_REQUEST_TIMEOUT_H_ */
