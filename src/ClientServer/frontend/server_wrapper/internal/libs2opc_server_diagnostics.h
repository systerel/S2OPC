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
 * - the EnabledFlag state is copied into the server runtime variables before endpoints are opened
 *   (::SOPC_ServerInternal_DiagnosticsStart),
 * - then the diagnostics support state is read-only, the EnabledFlag state and the runtime variables diagnostics are
 *   only accessed from the services thread.
 *
 * The ServerDiagnostics nodes are updated by the services thread: diagnostics updates only record that an update is
 * needed during a services event treatment, then a single priority local WriteRequest is enqueued at the end of the
 * event treatment (::SOPC_ServerInternal_DiagnosticsMayUpdateVariables services hook). It is treated before any other
 * pending event, including already received client requests.
 */

#ifndef LIBS2OPC_SERVER_DIAGNOSTICS_H_
#define LIBS2OPC_SERVER_DIAGNOSTICS_H_

#include <stdbool.h>
#include <stdint.h>

#include "sopc_address_space.h"

#include "libs2opc_server_config.h"

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
 * \note The EnabledFlag value is part of the runtime variables written on server start: its write behavior callback
 *       is then called and enqueues the ServerDiagnostics nodes update.
 * \note Shall be called in server configuring state.
 *
 * \param addSpace  The server address space
 *
 * \return SOPC_STATUS_OK in case of success, the ::SOPC_ServerInternal_RegisterWriteBehaviorCb status otherwise.
 */
SOPC_ReturnStatus SOPC_ServerInternal_DiagnosticsConfigure(SOPC_AddressSpace* addSpace);

/**
 * \brief Starts the ServerDiagnostics management on server start:
 *        copies the EnabledFlag state into the server runtime variables diagnostics and, if diagnostics are supported,
 *        sets ::SOPC_ServerInternal_DiagnosticsMayUpdateVariables as services event treated hook.
 *
 * \note Shall be called after the server runtime variables are built, before they are written and before endpoints
 *       are opened.
 */
void SOPC_ServerInternal_DiagnosticsStart(void);

/**
 * \brief Writes the ServerDiagnostics nodes in the AddressSpace with a priority local WriteRequest if an update was
 *        requested during the services event treatment (diagnostics values or EnabledFlag changed).
 *
 * \note Services event treated hook (see ::SOPC_Services_SetEventTreatedHook): called from the services thread at the
 *       end of each event treatment.
 */
void SOPC_ServerInternal_DiagnosticsMayUpdateVariables(void);

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
 *        When not supported, all the diagnostics code is inhibited (no counting, no AddressSpace update, no callback).
 *
 * \note Set during server configuration by ::SOPC_ServerInternal_DiagnosticsConfigure and read-only afterwards.
 *
 * \return true if the ServerDiagnostics are supported, false otherwise.
 */
bool SOPC_ServerInternal_IsDiagnosticsSupported(void);

/**
 * \brief Updates the session diagnostics on a session event and requests the ServerDiagnostics nodes update
 *        at the end of the services event treatment.
 *        Session diagnostics are counted even if diagnostics are disabled (only the AddressSpace update is
 *        inhibited) and are not counted if diagnostics are not supported.
 *
 * \note Shall be called from the services thread.
 *
 * \param event   The session event
 * \param status  The status associated to the session event
 */
void SOPC_ServerInternal_DiagnosticsUpdateSessionCounts(SOPC_ServerSessionEvent event, SOPC_StatusCode status);

/**
 * \brief Updates the subscription diagnostics and requests the ServerDiagnostics nodes update
 *        at the end of the services event treatment.
 *        Subscription diagnostics are updated even if diagnostics are disabled (only the AddressSpace update is
 *        inhibited) and are not updated if diagnostics are not supported.
 *
 * \note Shall be called from the services thread.
 *
 * \param currentSubscriptionCount    The number of subscriptions currently existing
 * \param cumulatedSubscriptionCount  The number of subscriptions created since server start
 * \param publishingIntervalCount     The number of distinct publishing intervals currently used
 */
void SOPC_ServerInternal_DiagnosticsUpdateSubscriptionCounts(uint32_t currentSubscriptionCount,
                                                             uint32_t cumulatedSubscriptionCount,
                                                             uint32_t publishingIntervalCount);

/**
 * \brief Updates the rejected requests diagnostics and requests the ServerDiagnostics nodes update
 *        at the end of the services event treatment.
 *        Rejected requests diagnostics are updated even if diagnostics are disabled (only the AddressSpace update is
 *        inhibited) and are not updated if diagnostics are not supported.
 *
 * \note Shall be called from the services thread.
 *
 * \param rejectedRequestsCount          The number of requests rejected since server start
 * \param securityRejectedRequestsCount  The number of requests rejected for security reasons since server start
 */
void SOPC_ServerInternal_DiagnosticsUpdateRequestCounts(uint32_t rejectedRequestsCount,
                                                        uint32_t securityRejectedRequestsCount);

/**
 * \brief Resets the ServerDiagnostics state: diagnostics not supported and disabled, services event treated hook reset.
 *
 * \note Shall be called on server configuration clear.
 */
void SOPC_ServerInternal_DiagnosticsClear(void);

#endif /* LIBS2OPC_SERVER_DIAGNOSTICS_H_ */
