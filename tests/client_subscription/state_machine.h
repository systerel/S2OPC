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
 */

#ifndef STATE_MACHINE_H_
#define STATE_MACHINE_H_

#include <stdbool.h>

/* The following includes are required to fetch the SOPC_LibSub_DataChangeCbk type */
#include "sopc_builtintypes.h"
#include "sopc_crypto_profiles.h"
#include "sopc_log_manager.h"
#include "sopc_toolkit_constants.h"
#include "sopc_types.h"
#define SKIP_S2OPC_DEFINITIONS
#include "libs2opc_client.h"

#include "sopc_user_app_itf.h"

/* Machine content is private to the implementation */
typedef enum SOPC_StaMac_State SOPC_StaMac_State;
typedef struct SOPC_StaMac_ReqCtx SOPC_StaMac_ReqCtx;
typedef struct SOPC_StaMac_Machine SOPC_StaMac_Machine;

/* Machine lifecycle */

/**
 * \brief Creates a new state machine, initialized in state stInit.
 *
 * \param iscConfig
 * \param cbkDataChanged    The callback to trigger when a PublishResponse is received
 * \param fPublishInterval  Subscription publish interval, in milliseconds
 * \param ppSM              The returned machine, when successful
 */
SOPC_ReturnStatus SOPC_StaMac_Create(uint32_t iscConfig,
                                     SOPC_LibSub_DataChangeCbk cbkDataChanged,
                                     double fPublishInterval,
                                     SOPC_StaMac_Machine** ppSM);

/**
 * \brief Deletes and deallocate the machine.
 */
void SOPC_StaMac_Delete(SOPC_StaMac_Machine** ppSM);

/**
 * \brief Creates a session.  The toolkit shall be configured with SOPC_Toolkit_Configured() beforehand.
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
 * \brief Returns a bool whether the machine is configured and ready for a new SecureChannel.
 *
 * Note: for now, it is the stInit state.
 */
bool SOPC_StaMac_IsConnectable(SOPC_StaMac_Machine* pSM);

/**
 * \brief Returns a bool whether the machine is in a connected state or not.
 *
 * Connected states are: stActivating, stActivated, stCreateing*, stClosing.
 */
bool SOPC_StaMac_IsConnected(SOPC_StaMac_Machine* pSM);

/**
 * \brief Returns a bool whether the machine is in stError or not.
 */
bool SOPC_StaMac_IsError(SOPC_StaMac_Machine* pSM);

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
