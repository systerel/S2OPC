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

#ifndef SOPC_USER_APP_CONFIG_H_
#define SOPC_USER_APP_CONFIG_H_

#include "sopc_event_dispatcher_manager.h"

/* Client and Server communication events to be managed by applicative code*/
typedef enum SOPC_App_Com_Event {
  /* Client application events */
  SE_SESSION_ACTIVATION_FAILURE,
  SE_ACTIVATED_SESSION,
  SE_SESSION_REACTIVATING, /* automatic new SC or manual new user on same SC */
  SE_RCV_SESSION_RESPONSE,
  SE_CLOSED_SESSION,
  //  SE_RCV_PUBLIC_RESPONSE, => discovery services
  
  /* Server application events */
  SE_CLOSED_ENDPOINT,
} SOPC_App_Com_Event;

/* Server address space access/modification notifications to applicative code */
typedef enum SOPC_App_AddSpace_Event {
  /* Server application events */
  AS_READ_EVENT,
  AS_WRITE_EVENT,
} SOPC_App_AddSpace_Event;

/* Server address space local modifications */
typedef enum SOPC_App_AddSpace_LocalService_Result {
  /* Server application local service results */
  AS_LOCAL_READ_RESULT,
  AS_WRITE_RESULT,
} SOPC_App_AddSpace_LocalService_Result;

/// INTERNAL USE ONLY ////

extern SOPC_EventDispatcherManager* applicationEventDispatcherMgr;

typedef enum SOPC_App_EventType {
  APP_COM_EVENT = 0x0,
  APP_ADDRESS_SPACE_NOTIF = 0x01
} SOPC_App_EventType;

 // TODO: define parameter for each type of event
typedef void SOPC_ComEvent_Fct(SOPC_App_Com_Event event,
                               void*              param,
                               SOPC_StatusCode    status);


// TODO: define parameter for each type of event
typedef void SOPC_AddressSpaceNotif_Fct(SOPC_App_AddSpace_Event event,
                                        void*                   param,
                                        SOPC_StatusCode         status);

// TODO: define parameter for each type of event
typedef void SOPC_AddressSpaceLocalService_Fct(SOPC_App_AddSpace_LocalService_Result resultType,
                                               void*                                 param,
                                               SOPC_StatusCode                       status);

void SOPC_ApplicationEventDispatcher(int32_t  appEvent, 
                                     uint32_t eventType, 
                                     void*    params, 
                                     int32_t  auxParam);

int32_t SOPC_AppEvent_ComEvent_Create(SOPC_App_Com_Event event);

int32_t SOPC_AppEvent_AddSpaceEvent_Create(SOPC_App_AddSpace_Event event);

SOPC_App_EventType SOPC_AppEvent_AppEventType_Get(int32_t iEvent);
SOPC_App_Com_Event SOPC_AppEvent_ComEvent_Get(int32_t iEvent);
SOPC_App_AddSpace_Event SOPC_AppEvent_AddSpaceEvent_Get(int32_t iEvent);

#endif // SOPC_USER_APP_CONFIG_H_
