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

typedef enum { stInit, stConfigured, stActivating, stActivated, stClosing, stDiscovering, stError } StateMachine_State;

typedef struct
{
    StateMachine_State state;
    SOPC_SecureChannel_Config* pscConfig;
    uint32_t iscConfig;    /* Internal scConfig ID */
    uintptr_t iSessionCtx; /* Internal Session context */
    uint32_t iSessionID;   /* OPC UA Session ID */
    uintptr_t iRequestCtx; /* Internal request context used for discovery requests */
} StateMachine_Machine;

/* Machine lifecycle */

/**
 * \brief Creates a new state machine, initialized in state stInit.
 */
StateMachine_Machine* StateMachine_Create(void);

/**
 * \brief Creates a secure channel configuration and add it to the toolkit.
 *  State machines shall be configured before the call to SOPC_Toolkit_Configured().
 */
SOPC_ReturnStatus StateMachine_ConfigureMachine(StateMachine_Machine* pSM);

/**
 * \brief Creates a session. See SOPC_ToolkitClient_AsyncActivateSession().
 * You shall call StateMachine_StopSession() to close the connection gracefully.
 */
SOPC_ReturnStatus StateMachine_StartSession(StateMachine_Machine* pSM);

/**
 * \brief Send a GetEndpointRequest.
 */
SOPC_ReturnStatus StateMachine_StartDiscovery(StateMachine_Machine* pSM);

/**
 * \brief Close the session. If not StateMachine_IsConnected(), the machine is put in state stError.
 */
SOPC_ReturnStatus StateMachine_StopSession(StateMachine_Machine* pSM);

/**
 * \brief Returns a bool whether the machine is configured and ready for a new SecureChannel.
 *  For now, it is the stConfigured state.
 */
bool StateMachine_IsConnectable(StateMachine_Machine* pSM);

/**
 * \brief Returns a bool whether the machine is in a connected state or not.
 *  Connected states are: stActivating, stActivated, stClosing.
 */
bool StateMachine_IsConnected(StateMachine_Machine* pSM);

/**
 * \brief Returns a bool whether the machine is discovering or not.
 *  For now, it is the stDiscovering states.
 */
bool StateMachine_IsDiscovering(StateMachine_Machine* pSM);

/**
 * \brief Returns a bool whether the machine is idle or not.
 *  Idle states are stInit, stConfigured (not yet started or finished), and stError.
 */
bool StateMachine_IsIdle(StateMachine_Machine* pSM);

/**
 * \brief Deletes the machine and the created secure channel configuration, if any.
 */
void StateMachine_Delete(StateMachine_Machine** ppSM);

/**
 * \brief Handles the events from the Toolkit and change the state machine state.
 *  This function shall be called upon each event raised by the Toolkit.
 *
 * This function can be called even if the message is not destined to this particular machine.
 *
 * \return True if the event is targeted to the machine. False otherwise, or when pSM is NULL.
 */
bool StateMachine_EventDispatcher(StateMachine_Machine* pSM,
                                  SOPC_App_Com_Event event,
                                  uint32_t arg,
                                  void* pParam,
                                  uintptr_t appCtx);

#endif /* STATE_MACHINE_H_ */
