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

#ifndef SOPC_AUDIT_H_
#define SOPC_AUDIT_H_

/** \brief
 * Provides the interface for user-configuration Auditing.
 * Only non-auditing application can be defined if S2OPC_HAS_AUDITING is not defined.
 */
#include <inttypes.h>

#include "sopc_builtintypes.h"
#include "sopc_enums.h"
#include "sopc_logger.h"

/**
 * Configuration options for Auditing. Any combination of following values is possible.
 *
 * \note For now, only security-related options are supported (see ::SOPC_Audit_SupportedSecuOptions).
 */
typedef enum SOPC_Audit_OptionMask
{
    SOPC_Audit_Options_NoOptions = 0x0000,
    SOPC_Audit_Options_ClientDiscoveryFailures = 0x0001,
    SOPC_Audit_Options_AuditRead = 0x0002,
    SOPC_Audit_Options_AuditWrite = 0x0004,
    SOPC_Audit_Options_AuditNodeMgt = 0x0008,
    SOPC_Audit_Options_AuditMethodCall = 0x0010,
    SOPC_Audit_Options_AuditSubscription = 0x0020,
    SOPC_Audit_Options_AuditSession = 0x0040,
    SOPC_Audit_Options_AuditSecureChannel = 0x0080,
    SOPC_Audit_Options_AuditCertificate = 0x0100,
    /* Unsupported options:
    SOPC_Audit_Options_LocalTime = 0x8000,
    */
} SOPC_Audit_OptionMask;

/** Supported options */
#define SOPC_Audit_SupportedSecuOptions                                                                \
    ((SOPC_Audit_OptionMask)(SOPC_Audit_Options_AuditSession | SOPC_Audit_Options_AuditSecureChannel | \
                             SOPC_Audit_Options_AuditCertificate))

/** Default options for auditing */
#define SOPC_Audit_DefaultSecuOptions SOPC_Audit_SupportedSecuOptions

/** \brief Defines the optional parameters of auditing entries and events. */
typedef struct SOPC_Audit_Configuration
{
    /** Path to the audit entry log file. Can be NULL (no log entry for audits) */
    const char* auditEntryPath;
    /** Maximum audit file size (if auditEntryPath is not NULL) */
    uint32_t logMaxBytes;
    /** Maximum number of audit files (if auditEntryPath is not NULL) */
    uint16_t logMaxFiles;
    /** Auditing options (can be a set of SOPC_Audit_Options) */
    SOPC_Audit_OptionMask options;
} SOPC_Audit_Configuration;

/**
 * \brief return Audit module initialization status.
 *
 * \param[in] optAuditConfig  audit configuration. If set to NULL, no auditing is performed.
 * \note If S2OPC_HAS_AUDITING is not defined, only non-auditing applications are supported.
 *
 * \return initialization status. Return False if the requested option does not match ::SOPC_Audit_SupportedSecuOptions
 */
bool SOPC_Audit_Initialize(const SOPC_Audit_Configuration* optAuditConfig);

/**
 * \return True if the application is currently auditing.
 */
bool SOPC_Audit_IsAuditing(void);

/**
 * \return True if the application is currently auditing and has given option activated.
 */
bool SOPC_Audit_HasOption(SOPC_Audit_OptionMask opt);

/**
 * \return The Audit log instance, or NULL if Audit are not activated, or Entries are not activated.
 */
SOPC_Log_Instance* SOPC_Audit_LogEntry(void);

/**
 * \brief clear audit library.
 */
void SOPC_Audit_Clear(void);

#endif /* SOPC_AUDIT_H_ */
