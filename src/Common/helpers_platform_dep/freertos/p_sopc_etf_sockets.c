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

#include "sopc_enums.h"
#include "sopc_etf_sockets.h"
#include "sopc_macros.h"

SOPC_ReturnStatus SOPC_ETF_Socket_Option(Socket* sock, bool deadlineMode, uint32_t soPriority, bool errorQueueEnable)
{
    SOPC_UNUSED_ARG(sock);
    SOPC_UNUSED_ARG(deadlineMode);
    SOPC_UNUSED_ARG(soPriority);
    SOPC_UNUSED_ARG(errorQueueEnable);
    return SOPC_STATUS_NOT_SUPPORTED;
}

SOPC_ReturnStatus SOPC_ETF_Socket_send(Socket sock, SOPC_Buffer* buffer, uint64_t txtime, SOPC_Socket_Address* sockAddr)
{
    SOPC_UNUSED_ARG(sock);
    SOPC_UNUSED_ARG(buffer);
    SOPC_UNUSED_ARG(txtime);
    SOPC_UNUSED_ARG(sockAddr);
    return SOPC_STATUS_NOT_SUPPORTED;
}

SOPC_ReturnStatus SOPC_ETF_Socket_Error_Queue(Socket sock)
{
    SOPC_UNUSED_ARG(sock);
    return SOPC_STATUS_NOT_SUPPORTED;
}