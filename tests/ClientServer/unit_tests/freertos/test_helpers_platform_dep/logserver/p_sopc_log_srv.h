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

#ifndef P_SOPC_LOG_SRV_H
#define P_SOPC_LOG_SRV_H

// ************Public API**************

// Wait a client connexion.
SOPC_ReturnStatus SOPC_LogSrv_WaitClient(uint32_t timeoutMs);

// Stop log server
SOPC_ReturnStatus SOPC_LogSrv_Stop(void);

// Start log server
SOPC_ReturnStatus SOPC_LogSrv_Start(
    uint16_t portSrvTCP,  // Server listen port
    uint16_t portCltUDP); // Destination UDP port where server announce its @IP and listen port

SOPC_ReturnStatus SOPC_LogSrv_Print(const uint8_t* buffer, uint16_t length);

#endif
