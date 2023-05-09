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

#ifndef SOPC_ETF_SOCKETS_H_
#define SOPC_ETF_SOCKETS_H_

#include "sopc_enums.h"
#include "sopc_udp_sockets.h"

#include "p_sopc_sockets.h"
#include "sopc_buffer.h"

#ifndef SO_TXTIME
#define SO_TXTIME 61
#define SCM_TXTIME SO_TXTIME
#endif

#define CLOCKID CLOCK_TAI

/**
 *  \brief Function to add new etf socket option
 *
 *  \param sock         Value pointed is set with the newly created socket
 *  \param deadlineMode Set to true to use socket in deadline mode and false for strict mode
 *  \param soPriority   Socket priority, should match corresponding configuration of traffic class (tc)
 *  \param errorQueueEnable enable packet drop report
 *
 *  \return SOPC_STATUS_OK if operation succeeded, SOPC_STATUS_INVALID_PARAMETERS
 *  or SOPC_STATUS_NOK otherwise.
 */
SOPC_ReturnStatus SOPC_ETF_Socket_Option(Socket* sock, bool deadlineMode, uint32_t soPriority, bool errorQueueEnable);

/**
 *  \brief Send data through the UDP socket to given IP address and port
 *
 *  \param sock        The socket used for sending
 *  \param buffer      The buffer containing data to be sent. Buffer considered with buffer->position 0 and containing
 * buffer->length bytes.
 *  \param txtime      The frame transmission timestamp
 *  \param sockAddr    The destination address and port
 *
 *  \return            SOPC_STATUS_OK if operation succeeded,
 * SOPC_STATUS_INVALID_PARAMETERS
 *  or SOPC_STATUS_NOK otherwise.
 */
SOPC_ReturnStatus SOPC_ETF_Socket_send(Socket sock,
                                       SOPC_Buffer* buffer,
                                       uint64_t txtime,
                                       SOPC_Socket_Address* sockAddr);

/**
 *  \brief Check if there is a sending error and log the origin of this one.
 *
 *  \param sock        Socket used for sending
 *
 *  \return            SOPC_STATUS_OK if there is no error SOPC_STATUS_NOK otherwise.
 */
SOPC_ReturnStatus SOPC_ETF_Socket_Error_Queue(Socket sock);

#endif /* SOPC_ETF_SOCKETS_H_ */
