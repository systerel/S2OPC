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
 * \brief High level interface to configure an OPC UA client and/or server
 *
 */

#ifndef LIBS2OPC_COMMON_CONFIG_H_
#define LIBS2OPC_COMMON_CONFIG_H_

#include <stdbool.h>

#include "sopc_common.h"
#include "sopc_encodeabletype.h"
#include "sopc_types.h"
#include "sopc_user_app_itf.h"

/**
 * \brief Enumerated values authorized for use with ::SOPC_EndpointConfig_AddSecurityConfig or
 * ::SOPC_ClientConfigHelper_CreateSecureConnection.
 * Values are limited to the security policies supported by client/server.
 */
typedef enum
{
    SOPC_SecurityPolicy_None,                /*!< http://opcfoundation.org/UA/SecurityPolicy#None */
    SOPC_SecurityPolicy_Basic256,            /*!< http://opcfoundation.org/UA/SecurityPolicy#Basic256 */
    SOPC_SecurityPolicy_Basic256Sha256,      /*!< http://opcfoundation.org/UA/SecurityPolicy#Basic256Sha256 */
    SOPC_SecurityPolicy_Aes128Sha256RsaOaep, /*!< http://opcfoundation.org/UA/SecurityPolicy#Aes128_Sha256_RsaOaep */
    SOPC_SecurityPolicy_Aes256Sha256RsaPss   /*!< http://opcfoundation.org/UA/SecurityPolicy#Aes256_Sha256_RsaPss */
} SOPC_SecurityPolicy_URI;

/**
 * \brief Initializes the S2OPC Client/Server frontend library (start threads, initialize configuration, etc.)
 *        and define a custom log configuration.
 *        Call to ::SOPC_CommonHelper_Initialize is required before any other operation.
 *
 * \note This function and ::SOPC_CommonHelper_Clear function are not thread-safe and shall be called in the same
 * thread.
 *
 * \param optLogConfig the custom log configuration or NULL to keep default configuration
 * \param optAuditConfig the custom audit configuration or NULL for non-auditing applications.
 *        The content of optAuditConfig may be freed/modified after call. If S2OPC_HAS_AUDITING is not defined, this
 *        parameter shall be NULL.
 *
 * \result SOPC_STATUS_OK in case of success, otherwise SOPC_STATUS_INVALID_STATE in case of double initialization.
 */
SOPC_ReturnStatus SOPC_CommonHelper_Initialize(const SOPC_Log_Configuration* optLogConfig,
                                               const SOPC_Audit_Configuration* optAuditConfig);

/**
 * \brief Clears the S2OPC Client/Server frontend library (stop threads, clear common configuration, etc.)
 *        Call to ::SOPC_CommonHelper_Clear shall be done after any Client/Server wrapper Clear operations.
 *
 * \note This function and ::SOPC_CommonHelper_Initialize function are not thread-safe and shall be called in the same
 * thread.
 */
void SOPC_CommonHelper_Clear(void);

/**
 * \brief Retrieves the S2OPC Client/Server frontend library build info (version, date, etc.).
 *        Shortcut to ::SOPC_ToolkitConfig_GetBuildInfo.
 *
 * \return Toolkit build information
 *
 */
SOPC_Toolkit_Build_Info SOPC_CommonHelper_GetBuildInfo(void);

/**
 * \brief Retrieves the S2OPC Client/Server configuration.
 *        It should be called after call to ::SOPC_CommonHelper_Initialize and before call to ::SOPC_CommonHelper_Clear
 *
 * \return The returned value is ensured to be a non-NULL pointer to Helper configuration
 *
 * \warning Applications are generally not allowed to modify any data in the configuration since it may lead
 *          to undetermined behavior. The only exceptions apply to server applications (see push server sample),
 *          as follows:
 *          - serverConfig.pki: might be modified during server configuration to implement TOFU mode,
 *          - serverConfig.pki and serverConfig.authenticationManager->pUsrPKI: might be accessed at runtime to update
 *            PKIs (server PKI and server user PKI, thread-safe API),
 *          - serverConfig.serverKeyCertPair: might be accessed at runtime to update server Key and Certificate
 *            (thread-safe API).
 *
 *          Modifying some other fields is reserved for internal libs2opc use.
 */
SOPC_S2OPC_Config* SOPC_CommonHelper_GetConfiguration(void);

/**
 * \brief Retrieves the S2OPC Client/Server frontend "initialized" state
 *
 * \return True if the Helper is initialized (when ::SOPC_CommonHelper_Initialize is called)
 *
 */
bool SOPC_CommonHelper_GetInitialized(void);

/**
 * \brief Returns the C string matching the security policy URI enum value
 *
 * \param secuUri The security policy URI enum value
 * \return The C string matching the security policy URI enum value or NULL if value invalid
 *
 */
const char* SOPC_SecurityPolicyUriToCstring(SOPC_SecurityPolicy_URI secuUri);

#endif
