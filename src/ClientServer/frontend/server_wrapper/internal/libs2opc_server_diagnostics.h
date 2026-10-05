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
 * \privatesection
 *
 * \brief Internal module managing the server diagnostics (ServerDiagnostics node and its EnabledFlag).
 *        It should not be used outside of the server wrapper and services implementation.
 *
 * Threading model:
 * - the diagnostics support and EnabledFlag states are initialized during server configuration
 *   (::SOPC_ServerInternal_DiagnosticsConfigure),
 * - the diagnostics event handler is created and the EnabledFlag state copied into the server runtime variables
 *   before endpoints are opened (::SOPC_ServerInternal_DiagnosticsStart),
 * - then the diagnostics support state is read-only (services thread and application looper thread),
 *   the EnabledFlag state and the runtime variables diagnostics are only accessed from the application looper thread.
 */

#ifndef LIBS2OPC_SERVER_DIAGNOSTICS_H_
#define LIBS2OPC_SERVER_DIAGNOSTICS_H_

#include <stdbool.h>

#include "sopc_address_space.h"
#include "sopc_event_handler.h"

#include "libs2opc_server_config.h"
#include "libs2opc_server_runtime_variables.h"

/**
 * \brief Initializes the ServerDiagnostics state from the EnabledFlag node of the address space.
 *
 * Diagnostics are supported if the EnabledFlag initial value is TRUE or if its AccessLevel allows to write it
 * (CurrentWrite bit). If the EnabledFlag variable is absent, the diagnostics are not supported.
 * The internal write behavior callback on the EnabledFlag node is only registered when diagnostics are supported.
 * When EnabledFlag value changes through a write:
 * - to TRUE: the ServerDiagnostics nodes are updated from the current server runtime variables,
 * - to FALSE: the ServerDiagnostics nodes are set to 0 with the OpcUa_BadNotReadable status.
 *
 * \note Local services bypass the AccessLevel check: when diagnostics are not supported,
 *       a local write of EnabledFlag remains possible but has no effect on diagnostics.
 * \note Shall be called in server configuring state.
 *
 * \param addSpace  The server address space
 *
 * \return SOPC_STATUS_OK in case of success, the ::SOPC_ServerInternal_RegisterWriteBehaviorCb status otherwise.
 */
SOPC_ReturnStatus SOPC_ServerInternal_DiagnosticsConfigure(SOPC_AddressSpace* addSpace);

/**
 * \brief Starts the ServerDiagnostics management on server start:
 *        - creates the event handler used to update ServerDiagnostics in the application looper thread,
 *          if diagnostics are supported and the handler does not exist yet.
 *          On creation failure, the diagnostics are inhibited (not supported and disabled),
 *        - copies the EnabledFlag state into the server runtime variables diagnostics.
 *
 * \note Shall be called after the server runtime variables are built, before they are written and before endpoints
 *       are opened: the EnabledFlag write notifies the event handler and the diagnostics support state is read-only
 *       afterwards.
 *
 * \param looper  The application looper on which the event handler callback is executed
 */
void SOPC_ServerInternal_DiagnosticsStart(SOPC_Looper* looper);

/**
 * \brief Returns the event handler used to update ServerDiagnostics.
 *
 * \return The ServerDiagnostics event handler, or NULL if it has not been initialized.
 */
SOPC_EventHandler* SOPC_ServerInternal_GetDiagnosticsEventHandler(void);

/**
 * \brief Returns the current ServerDiagnostics EnabledFlag state.
 *
 * \note Only exposed for test purposes
 *
 * \return true if the ServerDiagnostics are enabled, false otherwise.
 */
bool SOPC_ServerInternal_IsDiagnosticsEnabled(void);

/**
 * \brief Returns whether the ServerDiagnostics are supported by the server.
 *        When not supported, all the diagnostics code is inhibited (no counting, no event handler, no callback).
 *
 * \note Set during server configuration by ::SOPC_ServerInternal_DiagnosticsConfigure and read-only afterwards.
 *
 * \return true if the ServerDiagnostics are supported, false otherwise.
 */
bool SOPC_ServerInternal_IsDiagnosticsSupported(void);

/**
 * \brief Updates the ServerDiagnostics nodes in the AddressSpace from the current server runtime variables.
 *        Nothing is written when the ServerDiagnostics EnabledFlag is FALSE.
 *
 * \note Diagnostic values shall be kept up to date by the caller even when diagnostics are disabled,
 *       only the AddressSpace update is inhibited.
 * \note Shall be called from the application looper thread.
 *
 * \param diagnostics Pointer to the diagnostic values to write.
 */
void SOPC_ServerInternal_UpdateServerDiagnostics(const SOPC_Server_RuntimeVariablesDiagnostics* diagnostics);

/**
 * \brief Updates the session diagnostics on a session event and the ServerDiagnostics nodes in the AddressSpace.
 *        Session diagnostics are counted even if diagnostics are disabled (only the AddressSpace update is
 *        inhibited) and are not counted if diagnostics are not supported.
 *
 * \note Shall be called from the application looper thread.
 *
 * \param event      The session event
 * \param sessionId  The session identifier
 * \param status     The status associated to the session event
 */
void SOPC_ServerInternal_DiagnosticsOnSessionEvent(SOPC_ServerSessionEvent event,
                                                   SOPC_SessionId sessionId,
                                                   SOPC_StatusCode status);

/**
 * \brief Enqueue an event in application looper to reset the session related counters
 *        in the context of a server shutdown.
 */
void SOPC_ServerInternal_DiagnosticsSessionCountersReset(void);

/**
 * \brief Resets the ServerDiagnostics state: diagnostics not supported, disabled and event handler reference cleared.
 *        The event handler itself is owned (and freed) by the application looper.
 *
 * \note Shall be called on server configuration clear.
 */
void SOPC_ServerInternal_DiagnosticsClear(void);

#endif /* LIBS2OPC_SERVER_DIAGNOSTICS_H_ */
