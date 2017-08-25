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

#include "sopc_secure_channels_api.h"

#include <assert.h>

#include "sopc_event_dispatcher_manager.h"

static SOPC_EventDispatcherManager* secureChannelsEventDispatcherMgr = NULL;


SOPC_EventDispatcherManager* scEventDispatcherMgr = NULL;

void SOPC_SecureChannels_EnqueueEvent(SOPC_SecureChannels_InputEvent scEvent,
                                      uint32_t                       id,
                                      void*                          params,
                                      int32_t                        auxParam){
    if(NULL != secureChannelsEventDispatcherMgr){
        SOPC_EventDispatcherManager_AddEvent(secureChannelsEventDispatcherMgr,
                                             scEvent,
                                             id,
                                             params,
                                             auxParam,
                                             NULL);
    }
}

void SOPC_SecureChannels_Inititalize(){
    secureChannelsEventDispatcherMgr =
            SOPC_EventDispatcherManager_CreateAndStart(NULL, //SOPC_SecureChannelsEventMgr_Dispatcher,
                                                       "Sockets event manager dispatcher");
}

void SOPC_SecureChannels_Clear(){
    if(NULL != secureChannelsEventDispatcherMgr){
        SOPC_EventDispatcherManager_StopAndDelete(&secureChannelsEventDispatcherMgr);
    }
}
