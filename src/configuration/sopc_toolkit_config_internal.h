/*
 *  Copyright (C) 2017 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SOPC_TOOLKIT_CONFIG_INTERNAL_H_
#define SOPC_TOOLKIT_CONFIG_INTERNAL_H_

#include "sopc_user_app_itf.h"

void SOPC_Internal_ToolkitServer_SetAddressSpaceConfig(SOPC_AddressSpace* addressSpace);

typedef enum SOPC_App_EventType {
  SOPC_APP_COM_EVENT = 0x0,
  SOPC_APP_ADDRESS_SPACE_NOTIF = 0x01
} SOPC_App_EventType;

void SOPC_Internal_ApplicationEventDispatcher(int32_t  eventAndType,
                                              uint32_t id,
                                              void*    params,
                                              uint32_t auxParam);

int32_t SOPC_AppEvent_ComEvent_Create(SOPC_App_Com_Event event);
int32_t SOPC_AppEvent_AddSpaceEvent_Create(SOPC_App_AddSpace_Event event);

SOPC_App_EventType SOPC_AppEvent_AppEventType_Get(int32_t iEvent);
SOPC_App_Com_Event SOPC_AppEvent_ComEvent_Get(int32_t iEvent);
SOPC_App_AddSpace_Event SOPC_AppEvent_AddSpaceEvent_Get(int32_t iEvent);

#endif /* SOPC_TOOLKIT_CONFIG_INTERNAL_H_ */
