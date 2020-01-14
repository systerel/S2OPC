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

#ifndef SOPC_CHECK_SC_RCV_HELPERS_H
#define SOPC_CHECK_SC_RCV_HELPERS_H

#include "event_recorder.h"
#include "sopc_builtintypes.h"
#include "sopc_event_handler.h"
#include "sopc_secure_channels_api.h"
#include "sopc_sockets_api.h"

extern SOPC_EventRecorder* servicesEvents;

void Check_SC_Init(void);
void Check_SC_Clear(void);
SOPC_Event* Check_Service_Event_Received(SOPC_SecureChannels_OutputEvent event, uint32_t eltId, uintptr_t auxParam);
SOPC_Event* Check_Service_Event_Received_AllParams(SOPC_SecureChannels_OutputEvent event,
                                                   uint32_t eltId,
                                                   uintptr_t param,
                                                   uintptr_t auxParam);
SOPC_ReturnStatus Check_Client_Closed_SC(uint32_t scIdx,
                                         uint32_t socketIdx,
                                         uint32_t scConfigIdx,
                                         uint32_t pendingRequestHandle,
                                         SOPC_StatusCode errorStatus);

#endif
