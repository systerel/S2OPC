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

/* A simple state machine that handles:
 * - Toolkit configuration
 * - SecureChannel and Session creation
 * Starts in the init state. Shall be configured with a call to StateMachine_ConfigureMachine(), which configures the
 * Toolkit with parameters from config.h. Then SOPC_Toolkit_Configured() shall be called  and the Async
 * processus shall be started using StateMachine_StartSession() or StateMachine_StartDiscovery() function. The
 * StateMachine_EventDispatcher() shall be called on the state machine from the callback given to
 * SOPC_Toolkit_Initialize().
 */

#ifndef STATE_MACHINE_H_
#define STATE_MACHINE_H_

#include <stdbool.h>

#include "sopc_mutexes.h"
#include "sopc_user_app_itf.h"

typedef enum
{
    stInit,
    stConfigured,
    stActivating,
    stActivated,
    stClosing,
    stDiscovering,
    stRegister,
    stError
} StateMachine_State;

typedef struct
{
    uint32_t uid;
    uintptr_t appCtx;
} StateMachine_RequestContext;

typedef struct
{
    Mutex mutex;
    StateMachine_State state;
    SOPC_SecureChannel_Config* pscConfig;
    uint32_t iscConfig;                       /* Internal scConfig ID */
    StateMachine_RequestContext* pCtxSession; /* Internal Session context */
    uint32_t iSessionID;                      /* OPC UA Session ID */
    StateMachine_RequestContext* pCtxRequest; /* Internal request context used to identify requests */
} StateMachine_Machine;

/* Machine lifecycle */

/**
 * \brief Creates a new state machine, initialized in state stInit.
 */
StateMachine_Machine* StateMachine_Create(void);

/**
 * \brief Creates a secure channel configuration depending on sign and encrypt boolean option.
 * If both sign and encrypt are false, security mode is None and policy is None.
 * If sign and/or encrypt are true, security policy is Basic256Sha256 and mode is "Sign" or "SignAndEncrypt".
 *
 * State machines shall be configured before the call to SOPC_Toolkit_Configured().
 */
SOPC_ReturnStatus StateMachine_ConfigureMachine(StateMachine_Machine* pSM, bool sign, bool encrypt);

/**
 * \brief Creates an anonymous session. See SOPC_ToolkitClient_AsyncActivateSession().
 * You shall call StateMachine_StopSession() to close the connection gracefully.
 */
SOPC_ReturnStatus StateMachine_StartSession_Anonymous(StateMachine_Machine* pSM, const char* policyId);

/**
 * \brief Creates a session with username and password. See SOPC_ToolkitClient_AsyncActivateSession().
 * You shall call StateMachine_StopSession() to close the connection gracefully.
 */
SOPC_ReturnStatus StateMachine_StartSession_UsernamePassword(StateMachine_Machine* pSM,
                                                             const char* policyId,
                                                             const char* username,
                                                             const char* password);

/**
 * \brief Send a GetEndpointsRequest.
 */
SOPC_ReturnStatus StateMachine_StartDiscovery(StateMachine_Machine* pSM);

/**
 * \brief Send a FindServersRequest.
 */
SOPC_ReturnStatus StateMachine_StartFindServers(StateMachine_Machine* pSM);

/**
 * \brief Send a RegisterServer.
 */
SOPC_ReturnStatus StateMachine_StartRegisterServer(StateMachine_Machine* pSM);

/**
 * \brief Close the session. If not StateMachine_IsConnected(), the machine is put in state stError.
 */
SOPC_ReturnStatus StateMachine_StopSession(StateMachine_Machine* pSM);

/**
 * \brief Sends a request, wraps SOPC_ToolkitClient_AsyncSendRequestOnSession().
 *
 * The machine must be activated.
 *
 * \warning Every client request should be sent with this wrapper, so that the machine
 *  can recognize the response from the server.
 * \warning Only one request can be sent at a time.
 *
 * \param requestStruct The structure of the request, see SOPC_ToolkitClient_AsyncSendRequestOnSession()
 * \param appCtx        An ID that will be given back through the call to the event handler.
                        The value 0 indicates "no ID".
 */
SOPC_ReturnStatus StateMachine_SendRequest(StateMachine_Machine* pSM, void* requestStruct, uintptr_t appCtx);

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
 * \brief Returns a bool whether the machine is discovering (GetEndpoints or FindServers) or not.
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
 * \param pAppCtx   A pointer to an uintptr_t which will contain the appCtx given to
                    StateMachine_SendRequest(). NULL is valid. 0 indicates "appID unavailable".
 *
 * \return True if the event is targeted to the machine. False otherwise, or when pSM is NULL.
 */
bool StateMachine_EventDispatcher(StateMachine_Machine* pSM,
                                  uintptr_t* pAppCtx,
                                  SOPC_App_Com_Event event,
                                  uint32_t arg,
                                  void* pParam,
                                  uintptr_t appCtx);

#endif /* STATE_MACHINE_H_ */
