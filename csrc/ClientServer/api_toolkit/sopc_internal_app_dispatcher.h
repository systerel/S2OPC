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

#ifndef SOPC_INTERNAL_APP_DISPATCHER_H
#define SOPC_INTERNAL_APP_DISPATCHER_H

#include "sopc_event_handler.h"
#include "sopc_user_app_itf.h"

extern SOPC_ComEvent_Fct* appEventCallback;
extern SOPC_AddressSpaceNotif_Fct* appAddressSpaceNotificationCallback;

void SOPC_App_Initialize(void);
void SOPC_App_Clear(void);

SOPC_ReturnStatus SOPC_App_EnqueueComEvent(SOPC_App_Com_Event event, uint32_t id, uintptr_t params, uintptr_t auxParam);
SOPC_ReturnStatus SOPC_App_EnqueueAddressSpaceNotification(SOPC_App_AddSpace_Event event,
                                                           uint32_t id,
                                                           uintptr_t params,
                                                           uintptr_t auxParam);

#endif // SOPC_INTERNAL_APP_DISPATCHER_H
