/*
 *  Copyright (C) 2018 Systerel and others.
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

/* A simple state machine that handles:
 * - Toolkit configuration
 * - SecureChannel and Session creation
 * Starts in the init state. Shall be configured with a call to StateMachine_ConfigureToolkit(),
 *  which configures the Toolkit with parameters from config.h and calls Toolkit_Configured(),
 *  and starts the Async processus.
 * The StateMachine_EventDispatcher shall be called on the state machine from the callback
 *  given to Toolkit_Initialize().
 */

#ifndef STATE_MACHINE_H_
#define STATE_MACHINE_H_

#include <stdbool.h>

#include "sopc_user_app_itf.h"

typedef enum { stInit, stWaitActivation, stWaitResponse, stWaitFinished, stAbort } StateMachine_State;

typedef struct
{
    StateMachine_State state;
    SOPC_SecureChannel_Config* pscConfig;
    uint32_t iscConfig;  /* Internal scConfig ID */
    uintptr_t iSession;  /* Internal ID */
    uint32_t iSessionID; /* OPC UA Session ID */
} StateMachine_Machine;

/* Machine lifecycle */
StateMachine_Machine* StateMachine_Create(void);
SOPC_ReturnStatus StateMachine_ConfigureToolkit(StateMachine_Machine* pSM);
bool StateMachine_IsOver(StateMachine_Machine* pSM);
void StateMachine_Delete(StateMachine_Machine** ppSM);

/* Handles machine events */
void StateMachine_EventDispatcher(StateMachine_Machine* pSM,
                                  SOPC_App_Com_Event event,
                                  uint32_t arg,
                                  void* pParam,
                                  uintptr_t appCtx);

#endif /* STATE_MACHINE_H_ */
