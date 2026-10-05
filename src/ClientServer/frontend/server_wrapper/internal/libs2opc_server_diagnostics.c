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

#include "libs2opc_server_diagnostics.h"

#include "libs2opc_server_internal.h"
#include "libs2opc_server_runtime_variables.h"

#include "opcua_identifiers.h"
#include "opcua_statuscodes.h"
#include "sopc_assert.h"
#include "sopc_event_handler.h"
#include "sopc_logger.h"
#include "sopc_macros.h"

/* ServerDiagnostics event handler: created before endpoints are opened when diagnostics are supported,
 * its callback is executed in the application looper thread */
static SOPC_EventHandler* serverDiagnosticsEventHandler = NULL;

/* ServerDiagnostics EnabledFlag state: set during configuration, then only accessed from the application looper
 * thread (write behavior callback posts an event to the diagnostics event handler).
 * Copied into the runtime variables diagnostics on server start (see set_enabled_flag). */
static bool diagEnabledFlag = false;

/* ServerDiagnostics support state: set during configuration and read-only afterwards (services thread and
 * application looper thread). Server messages are only treated once endpoints are opened, which occurs after
 * configuration through the services event queue. */
static bool diagSupported = false;

static const SOPC_NodeId enabledFlagNodeId = SOPC_NODEID_NS0_NUMERIC(OpcUaId_Server_ServerDiagnostics_EnabledFlag);

// Keeps the module EnabledFlag state and its copy in the runtime variables diagnostics consistent
static void set_enabled_flag(bool value)
{
    diagEnabledFlag = value;
    sopc_server_helper_config.runtimeVariables.diagnostics.enabledFlag = value;
}

static void write_server_diagnostics(const SOPC_Server_RuntimeVariablesDiagnostics* diagnostics)
{
    OpcUa_WriteRequest* writeRequest = SOPC_RuntimeVariables_BuildUpdateServerDiagnosticsWriteRequest(diagnostics);

    if (NULL != writeRequest)
    {
        bool res = SOPC_ServerInternal_LocalServiceAsync(
            SOPC_HelperInternal_RuntimeVariableSetResponseCb, writeRequest, (uintptr_t) NULL,
            "Updating server diagnostics runtime variables of server information nodes failed."
            " Please check address space content includes necessary diagnostic information nodes.");

        SOPC_UNUSED_RESULT(res);
    }
}

// Called from the application looper thread on EnabledFlag write
static void update_enabled_flag(bool newValue)
{
    if (newValue == diagEnabledFlag)
    {
        return;
    }
    set_enabled_flag(newValue);
    /* Runtime variables are kept up to date while diagnostics are disabled:
       up to date values are written when enabled, 0 with OpcUa_BadNotReadable when disabled */
    write_server_diagnostics(&sopc_server_helper_config.runtimeVariables.diagnostics);
}

static bool get_enabled_flag_value(const SOPC_Variant* value, bool* enabled)
{
    if (NULL == value || SOPC_Boolean_Id != value->BuiltInTypeId ||
        SOPC_VariantArrayType_SingleValue != value->ArrayType)
    {
        return false; // failed to extract enabledFlag value
    }
    *enabled = value->Value.Boolean;
    return true; // enabled value extraction succeeded
}

// Called from the services thread: postpone the EnabledFlag update in the application looper thread
static void enabled_flag_write_behavior_cb(const SOPC_NodeId* nodeId,
                                           const SOPC_DataValue* prevValue,
                                           const SOPC_DataValue* newValue,
                                           uintptr_t auxParam)
{
    SOPC_UNUSED_ARG(nodeId);
    SOPC_UNUSED_ARG(prevValue);
    SOPC_UNUSED_ARG(auxParam);

    // Diagnostics inhibited after configuration (event handler creation failure): nothing to notify
    if (!diagSupported)
    {
        return;
    }

    bool enabled = false;
    if (!get_enabled_flag_value(&newValue->Value, &enabled))
    {
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                 "ServerDiagnostics EnabledFlag written with an invalid value: ignored.");
        return;
    }

    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_STATE;
    if (NULL != serverDiagnosticsEventHandler)
    {
        status = SOPC_EventHandler_Post(serverDiagnosticsEventHandler, OpcUaId_Server_ServerDiagnostics_EnabledFlag,
                                        (uint32_t) enabled, (uintptr_t) 0, (uintptr_t) 0);
    }
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "Failed to notify ServerDiagnostics EnabledFlag change (status=%d).", (int) status);
    }
}

// Called from the application looper thread: events posted by the diagnostics sources
static void diagnostics_event_handler_cb(SOPC_EventHandler* handler,
                                         int32_t event,
                                         uint32_t eltId,
                                         uintptr_t params,
                                         uintptr_t auxParam)
{
    SOPC_UNUSED_ARG(handler);

    SOPC_Server_RuntimeVariablesDiagnostics* diagnostics = &sopc_server_helper_config.runtimeVariables.diagnostics;

    uint32_t currentSubscriptionCount = 0;
    uint32_t cumulatedSubscriptionCount = 0;
    uint32_t publishingIntervalCount = 0;
    uint32_t rejectedRequestsCount = 0;
    uint32_t securityRejectedRequestsCount = 0;

    bool updateServerDiagnostics = true;

    // Runtime variables diagnostics are updated even if diagnostics are disabled: only AddressSpace update is inhibited
    switch (event)
    {
    case OpcUaId_Server_ServerDiagnostics_ServerDiagnosticsSummary_CurrentSessionCount:
        // Only used to reset the session counters
        // (values are actually updated in ::SOPC_ServerInternal_DiagnosticsOnSessionEvent)
        diagnostics->currentSessionCount = 0;
        diagnostics->cumulatedSessionCount = 0;
        diagnostics->securityRejectedSessionCount = 0;
        diagnostics->rejectedSessionCount = 0;
        diagnostics->sessionTimeoutCount = 0;
        diagnostics->sessionAbortCount = 0;
        break;

    case OpcUaId_Server_ServerDiagnostics_EnabledFlag:
        update_enabled_flag((bool) eltId);
        updateServerDiagnostics = false; // already managed in ::update_enabled_flag
        break;

    case OpcUaId_Server_ServerDiagnostics_ServerDiagnosticsSummary_CurrentSubscriptionCount:
        SOPC_ASSERT(params <= UINT32_MAX);
        SOPC_ASSERT(auxParam <= UINT32_MAX);

        currentSubscriptionCount = eltId;
        cumulatedSubscriptionCount = (uint32_t) params;
        publishingIntervalCount = (uint32_t) auxParam;

        diagnostics->currentSubscriptionCount = currentSubscriptionCount;
        diagnostics->cumulatedSubscriptionCount = cumulatedSubscriptionCount;
        diagnostics->publishingIntervalCount = publishingIntervalCount;
        break;

    case OpcUaId_Server_ServerDiagnostics_ServerDiagnosticsSummary_RejectedRequestsCount:
        SOPC_ASSERT(params <= UINT32_MAX);

        rejectedRequestsCount = (uint32_t) eltId;
        securityRejectedRequestsCount = (uint32_t) params;

        diagnostics->rejectedRequestsCount = rejectedRequestsCount;
        diagnostics->securityRejectedRequestsCount = securityRejectedRequestsCount;
        break;

    default:
        SOPC_ASSERT(false);
        updateServerDiagnostics = false;
        break;
    }

    if (updateServerDiagnostics)
    {
        SOPC_ServerInternal_UpdateServerDiagnostics(diagnostics);
    }
}

static bool is_security_rejected_session_status(SOPC_StatusCode status)
{
    /* UACTT Base Info Diagnostics/023 explicitly checks that BadIdentityTokenInvalid, BadIdentityTokenRejected and
     * BadUserAccessDenied increment SecurityRejectedSessionCount.
     * Mantis #6406 confirms specification should evolve to indicate it.
     * The other StatusCodes below are security-related ActivateSession Service results
     * defined in OPC UA Part 4, Section 5.7.3.3
     */
    switch (status)
    {
    case OpcUa_BadIdentityTokenInvalid:
    case OpcUa_BadIdentityTokenRejected:
    case OpcUa_BadUserAccessDenied:
    case OpcUa_BadApplicationSignatureInvalid:
    case OpcUa_BadUserSignatureInvalid:
    case OpcUa_BadNoValidCertificates:
    case OpcUa_BadSecurityPolicyRejected:
        return true;

    default:
        return false;
    }
}

SOPC_ReturnStatus SOPC_ServerInternal_DiagnosticsConfigure(SOPC_AddressSpace* addSpace)
{
    // Initialize and computes those 2 internal variable values
    diagSupported = false;
    diagEnabledFlag = false;

    // Retrieve the node in address space configured
    bool found = false;
    SOPC_AddressSpace_Node* node = SOPC_AddressSpace_Get_Node(addSpace, &enabledFlagNodeId, &found);
    if (!found || NULL == node || OpcUa_NodeClass_Variable != *SOPC_AddressSpace_Get_NodeClass(addSpace, node))
    {
        SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_CLIENTSERVER,
                              "ServerDiagnostics EnabledFlag variable not found: diagnostics are inhibited.");
        return SOPC_STATUS_OK;
    }

    // A node without value (Null) is considered as FALSE
    const SOPC_Variant* value = SOPC_AddressSpace_Get_Value(addSpace, node);
    if (NULL != value && SOPC_Null_Id != value->BuiltInTypeId && !get_enabled_flag_value(value, &diagEnabledFlag))
    {
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                 "ServerDiagnostics EnabledFlag node has an invalid value: diagnostics are disabled.");
    }

    // Retrieve the AccessLevel writable property
    const bool writable = 0 != (SOPC_AddressSpace_Get_AccessLevel(addSpace, node) & OpcUa_AccessLevelType_CurrentWrite);
    // Either it is writable or it is enabled by configuration. Otherwise it is unsupported for whole server lifecycle.
    diagSupported = diagEnabledFlag || writable;

    if (!diagSupported)
    {
        /* Diagnostics are inhibited: no write behavior callback is registered.
         * Note: local services bypass the AccessLevel check (see has_access_level_write in
         * address_space_authorization_i.imp), a local write of EnabledFlag remains possible
         * but has no effect on diagnostics. */
        SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_CLIENTSERVER,
                              "ServerDiagnostics EnabledFlag is FALSE and not writable: diagnostics are inhibited.");
        return SOPC_STATUS_OK;
    }
    // Note: if enabled but not writable, keep possibility for the application to disable it at runtime (local write)
    return SOPC_ServerInternal_RegisterWriteBehaviorCb(&enabledFlagNodeId, enabled_flag_write_behavior_cb, 0);
}

void SOPC_ServerInternal_DiagnosticsStart(SOPC_Looper* looper)
{
    if (diagSupported && NULL == serverDiagnosticsEventHandler)
    {
        serverDiagnosticsEventHandler = SOPC_EventHandler_Create(looper, diagnostics_event_handler_cb);
        if (NULL == serverDiagnosticsEventHandler)
        {
            // Diagnostics cannot be managed without event handler: inhibit them all consistently
            diagSupported = false;
            diagEnabledFlag = false;
            SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                     "Event handler creation to update server diagnostics failed:"
                                     " server diagnostics are inhibited.");
        }
    }
    // Runtime variables have been (re)built: copy the current EnabledFlag state
    set_enabled_flag(diagEnabledFlag);
}

SOPC_EventHandler* SOPC_ServerInternal_GetDiagnosticsEventHandler(void)
{
    return serverDiagnosticsEventHandler;
}

bool SOPC_ServerInternal_IsDiagnosticsEnabled(void)
{
    return diagEnabledFlag;
}

bool SOPC_ServerInternal_IsDiagnosticsSupported(void)
{
    return diagSupported;
}

void SOPC_ServerInternal_UpdateServerDiagnostics(const SOPC_Server_RuntimeVariablesDiagnostics* diagnostics)
{
    if (diagEnabledFlag)
    {
        write_server_diagnostics(diagnostics);
    }
}

void SOPC_ServerInternal_DiagnosticsOnSessionEvent(SOPC_ServerSessionEvent event,
                                                   SOPC_SessionId sessionId,
                                                   SOPC_StatusCode status)
{
    SOPC_UNUSED_ARG(sessionId);

    if (!SOPC_ServerInternal_IsDiagnosticsSupported())
    {
        return;
    }

    SOPC_Server_RuntimeVariablesDiagnostics* diagnostics = &sopc_server_helper_config.runtimeVariables.diagnostics;

    // Session diagnostics are counted even if disabled: AddressSpace update is inhibited when diagnostics are disabled
    bool diagnosticsChanged = true;

    switch (event)
    {
    case SESSION_CREATION:
        diagnostics->currentSessionCount++;
        diagnostics->cumulatedSessionCount++;
        break;

    case SESSION_CLOSURE:
        /* if status = OpcUa_BadSessionIdInvalid the session can't be closed but common_server_notify_session_closed is
         * called and generates AS_SESSION_CLOSURE.
         * (see session_audit_bs__server_notify_session_closed in session_audit_bs.c)
         */
        if (OpcUa_BadSessionIdInvalid != status)
        {
            if (diagnostics->currentSessionCount > 0)
            {
                diagnostics->currentSessionCount--;
            }

            if (OpcUa_BadTimeout == status)
            {
                diagnostics->sessionTimeoutCount++;
            }
            else if (SOPC_IsBadStatus(status))
            {
                diagnostics->sessionAbortCount++;
            }
        }

        break;

    case SESSION_ACTIVATION:
        if (SOPC_IsBadStatus(status))
        {
            diagnostics->rejectedSessionCount++;
            if (is_security_rejected_session_status(status))
            {
                diagnostics->securityRejectedSessionCount++;
            }
        }
        break;

    case SESSION_INACTIVE:
    default:
        diagnosticsChanged = false;
        break;
    }

    if (diagnosticsChanged)
    {
        SOPC_ServerInternal_UpdateServerDiagnostics(diagnostics);
    }
}

void SOPC_ServerInternal_DiagnosticsSessionCountersReset(void)
{
    if (!diagSupported)
    {
        return;
    }

    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_STATE;
    if (NULL != serverDiagnosticsEventHandler)
    {
        status = SOPC_EventHandler_Post(serverDiagnosticsEventHandler,
                                        OpcUaId_Server_ServerDiagnostics_ServerDiagnosticsSummary_CurrentSessionCount,
                                        0, 0, 0);
    }
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "Failed to notify ServerDiagnostics Session counters reset (status=%d).", (int) status);
    }
}

void SOPC_ServerInternal_DiagnosticsClear(void)
{
    diagSupported = false;
    diagEnabledFlag = false;
    serverDiagnosticsEventHandler = NULL;
}
