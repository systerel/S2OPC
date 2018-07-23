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

/** \file
 *
 * \brief The state machine of the subscribing client.
 *
 * The machine handles:
 * - Toolkit configuration,
 * - SecureChannel and Session creation,
 * - Creation of the Subscription,
 * - Creation of the MonitoredItems,
 * - A number of PublishRequest.
 *
 * The machine starts in the init state. It shall be configured with a call to SOPC_StaMac_ConfigureMachine(),
 * which configures the Toolkit. Still, th Then SOPC_Toolkit_Configured(). The
 * SOPC_StaMac_EventDispatcher() shall be called on the state machine from the callback given to
 * SOPC_Toolkit_Initialize().
 *
 * The machine API is thread safe.
 *
 */

#ifndef STATE_MACHINE_H_
#define STATE_MACHINE_H_

/* Machine static parameters, MonitoredItem parameters */
#define MONIT_TIMESTAMPS_TO_RETURN OpcUa_TimestampsToReturn_Both
#define MONIT_QSIZE 10

#include <stdbool.h>

/* The following includes are required to fetch the SOPC_LibSub_DataChangeCbk type */
#include "sopc_builtintypes.h"
#include "sopc_crypto_profiles.h"
#include "sopc_toolkit_constants.h"
#include "sopc_types.h"
#include "sopc_user_app_itf.h"
#define SKIP_S2OPC_DEFINITIONS
#include "libs2opc_client.h"

/* Machine states */
typedef enum
{
    stError, /* stError is both error and closed state */
    stInit,
    stActivating,
    stActivated,
    stCreatingSubscr,
    stCreatingMonIt,
    stCreatingPubReq,
    stClosing
} SOPC_StaMac_State;

/* Machine content is private to the implementation */
typedef struct SOPC_StaMac_ReqCtx SOPC_StaMac_ReqCtx;
typedef struct SOPC_StaMac_Machine SOPC_StaMac_Machine;

/* Machine lifecycle */

/**
 * \brief Creates a new state machine, initialized in state stInit.
 *
 * \param iscConfig         The configuration identifier to use with this machine
 * \param iCliId            The client id of the machine
 * \param szPolicyId        Zero-terminated user identity policy id, see SOPC_LibSub_ConnectionCfg
 * \param szUsername        Zero-terminated username, see SOPC_LibSub_ConnectionCfg
 * \param szPassword        Zero-terminated password, see SOPC_LibSub_ConnectionCfg
 * \param cbkDataChanged    The callback to trigger when a PublishResponse is received
 * \param fPublishInterval  Subscription publish interval, in milliseconds
 * \param iCntMaxKeepAlive  The number of times an empty PublishResponse is not sent
 * \param iCntLifetime      The number of times a PublishResponse cannot be sent before killing the subscription
 * \param iTokenTarget      Number of subscription tokens the server should always have
 * \param ppSM              The returned machine, when successful
 */
SOPC_ReturnStatus SOPC_StaMac_Create(uint32_t iscConfig,
                                     uint32_t iCliId,
                                     const char* szPolicyId,
                                     const char* szUsername,
                                     const char* szPassword,
                                     SOPC_LibSub_DataChangeCbk cbkDataChanged,
                                     double fPublishInterval,
                                     uint32_t iCntMaxKeepAlive,
                                     uint32_t iCntLifetime,
                                     uint32_t iTokenTarget,
                                     SOPC_StaMac_Machine** ppSM);

/**
 * \brief Deletes and deallocate the machine.
 */
void SOPC_StaMac_Delete(SOPC_StaMac_Machine** ppSM);

/**
 * \brief Creates a session asynchronously. The toolkit shall be configured with
 *        SOPC_Toolkit_Configured() beforehand.
 *
 * The state machine will also create a subscription. See SOPC_StaMac_HasSubscription().
 *
 * See SOPC_ToolkitClient_AsyncActivateSession().
 * You shall call SOPC_StaMac_StopSession() to close the connection gracefully.
 */
SOPC_ReturnStatus SOPC_StaMac_StartSession(SOPC_StaMac_Machine* pSM);

/**
 * \brief Closes the session.
 *
 * If not SOPC_StaMac_IsConnected(), the machine is put in state stError.
 */
SOPC_ReturnStatus SOPC_StaMac_StopSession(SOPC_StaMac_Machine* pSM);

/**
 * \brief Sends a request, wraps SOPC_ToolkitClient_AsyncSendRequestOnSession().
 *
 * The machine must be activated.
 *
 * \warning Every client request should be sent with this wrapper, so that the machine
 *          can recognize the response from the server.
 *
 * \param requestStruct The structure of the request, see SOPC_ToolkitClient_AsyncSendRequestOnSession()
 * \param appCtx        An ID that will be given back through the call to the event handler.
                        The value 0 indicates "no ID".
 */
SOPC_ReturnStatus SOPC_StaMac_SendRequest(SOPC_StaMac_Machine* pSM, void* requestStruct, uintptr_t appCtx);

/**
 * \brief Creates a MonitoredItem asynchronously.
 *
 * The optional \p pAppCtx may be used to test the effective creation of the MonitoredItem with
 * SOPC_StaMac_HasMonitoredItem().
 *
 * \warning The szNodeId must be \0-terminated.
 */
SOPC_ReturnStatus SOPC_StaMac_CreateMonitoredItem(SOPC_StaMac_Machine* pSM,
                                                  const char* szNodeId,
                                                  uint32_t iAttrId,
                                                  uintptr_t* pAppCtx,
                                                  uint32_t* pCliHndl);

/**
 * \brief Returns a bool whether the machine is configured and ready for a new SecureChannel.
 *
 * Note: for now, it is the stInit state.
 */
bool SOPC_StaMac_IsConnectable(SOPC_StaMac_Machine* pSM);

/**
 * \brief Returns a bool whether the machine is in a connected state or not.
 *
 * Connected states are: stActivating, stActivated, stCreating*, stClosing.
 */
bool SOPC_StaMac_IsConnected(SOPC_StaMac_Machine* pSM);

/**
 * \brief Returns a bool whether the machine is in stError or not.
 */
bool SOPC_StaMac_IsError(SOPC_StaMac_Machine* pSM);

/**
 * \brief Returns a bool whether the machine has an active subscription or not.
 */
bool SOPC_StaMac_HasSubscription(SOPC_StaMac_Machine* pSM);

/**
 * \brief Returns whether the machine has created the MonitoredItem with the given \p appCtx or not.
 */
bool SOPC_StaMac_HasMonItByAppCtx(SOPC_StaMac_Machine* pSM, uintptr_t appCtx);

/**
 * \brief Handles the events from the Toolkit and changes the state machine state.
 *
 * This function can be called even if the message is not destined to this particular machine.
 *
 * \param pAppCtx   A pointer to an uintptr_t which will contain the appCtx given to
                    SOPC_StaMac_SendRequest(). NULL is valid. 0 indicates "appID unavailable".
 * \param appCtx    The appCtx given by the Toolkit.
 *
 * \warning This function shall be called upon each event raised by the Toolkit.
 *
 * \return True if the event is targeted to the machine. False otherwise, or when pSM is NULL.
 */
bool SOPC_StaMac_EventDispatcher(SOPC_StaMac_Machine* pSM,
                                 uintptr_t* pAppCtx,
                                 SOPC_App_Com_Event event,
                                 uint32_t arg,
                                 void* pParam,
                                 uintptr_t appCtx);

#endif /* STATE_MACHINE_H_ */
