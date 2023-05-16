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
 * \brief Additional client configuration facilities for non-XML configuration of the client
 *        or non-essential advanced configuration.
 *
 * \note TLDR: if client configuration is done through XML configuration files, you might ignore this header.
 */

#ifndef LIBS2OPC_CLIENT_CONFIG_CUSTOM_H_
#define LIBS2OPC_CLIENT_CONFIG_CUSTOM_H_

#include <stdbool.h>

#include "libs2opc_client_config.h"
#include "libs2opc_common_config.h"

/** \brief Client configuration without XML */

/**
 * \brief Defines client preferred locales ids sorted by preference from an array of locale strings.
 *
 * \param nbLocales  The number of locales defined in the array.
 *                   It might be 0 if no locale defined (only default exist).
 * \param localeIds  The array of locales sorted by preference order.
 *                   Array and its content is copied by function.
 *
 * \return SOPC_STATUS_OK in case of success, otherwise SOPC_STATUS_INVALID_PARAMETERS
 *         if \p localeIds is invalid when \p nbLocales \> 0
 *         or SOPC_STATUS_INVALID_STATE if the configuration is not possible
 *         (toolkit not initialized, preferred localesIds already defined, client connection initiated).
 */
SOPC_ReturnStatus SOPC_HelperConfigClient_SetPreferredLocaleIds(size_t nbLocales, const char** localeIds);

/**
 * \brief Defines client application description
 *
 * \param applicationUri        The globally unique identifier for the application instance.
 *                              This URI is used as ServerUri in Services if the application is a Server.
 * \param productUri            The globally unique identifier for the product.
 * \param defaultAppName        The name of the application using the default locale language.
 * \param defaultAppNameLocale  The default locale if any. If defined it shall exists in supported locales.
 * \param applicationType       The type of application, it shall be one of the OpcUa_ApplicationType_Client* types
 *
 * \return SOPC_STATUS_OK in case of success, otherwise SOPC_STATUS_INVALID_PARAMETERS
 *         if \p applicationUri, \p productUri or \p defaultAppName are invalid
 *         or SOPC_STATUS_INVALID_STATE if the configuration is not possible
 *         (toolkit not initialized, application description already set, client connection initiated).
 */
SOPC_ReturnStatus SOPC_HelperConfigClient_SetApplicationDescription(const char* applicationUri,
                                                                    const char* productUri,
                                                                    const char* defaultAppName,
                                                                    const char* defaultAppNameLocale,
                                                                    OpcUa_ApplicationType applicationType);

/**
 * \brief Defines the PKI provider that will be in charge of validating certificates received by client.
 *
 * \param pki  The PKI provider to be used.
 *             It will be automatically deallocated using ::SOPC_PKIProvider_Free on call to
 *             ::SOPC_HelperConfigClient_Clear.
 *
 * \return SOPC_STATUS_OK in case of success, otherwise SOPC_STATUS_INVALID_PARAMETERS
 *         if \p pki is invalid
 *         or SOPC_STATUS_INVALID_STATE if the configuration is not possible
 *         (toolkit not initialized, PKI already defined, server already started).
 *
 * \note A default PKI provider compliant with OPC UA standard is provided in sopc_pki_stack.h
 */
SOPC_ReturnStatus SOPC_HelperConfigClient_SetPKIprovider(SOPC_PKIProvider* pki);

/**
 * \brief Sets asymmetrical certificate and key of client from file paths.
 *        Certificate files shall use DER format, key file shall use DER or PEM format.
 *
 * \param clientCertPath  Path to client certificate file at DER format
 * \param clientKeyPath   Path to client key file at DER or PEM format
 * \param encrypted       Whether if the key is encrypted or not
 *
 * \return SOPC_STATUS_OK in case of success, otherwise SOPC_STATUS_INVALID_PARAMETERS
 *         if \p clientCertPath or \p clientKeyPath are invalid
 *         or SOPC_STATUS_INVALID_STATE if the configuration is not possible
 *         (toolkit not initialized, key/cert pair already set, connection initiated).
 */
SOPC_ReturnStatus SOPC_HelperConfigClient_SetKeyCertPairFromPath(const char* clientCertPath,
                                                                 const char* clientKeyPath,
                                                                 bool encrypted);

/**
 * \brief Sets asymmetrical certificate and key of client from byte arrays.
 *        Certificate shall be in DER format, key file shall be in DER or PEM format.
 *
 * \param certificateNbBytes Number of elements in \p clientCertificate array
 * \param clientCertificate  Array of bytes containing client certificate at DER format
 * \param keyNbBytes         Number of elements in \p clientPrivateKey array
 * \param clientPrivateKey   Array of bytes containing client key file at DER or PEM format
 *
 * \return SOPC_STATUS_OK in case of success, otherwise SOPC_STATUS_INVALID_PARAMETERS
 *         if \p certificateNbBytes, \p clientCertificate, \p keyNbBytes or \p clientKeyPath are invalid (0 or NULL)
 *         or SOPC_STATUS_INVALID_STATE if the configuration is not possible
 *         (toolkit not initialized, key/cert pair already set, connection initiated).
 */
/* NOT IMPLEMENTED
 *
 * SOPC_ReturnStatus SOPC_HelperConfigClient_SetKeyCertPairFromBytes(size_t certificateNbBytes,
 *                                                                  const unsigned char* clientCertificate,
 *                                                                  size_t keyNbBytes,
 *                                                                  const unsigned char* clientPrivateKey);
 */

/**
 * \brief Create a new secure channel configuration in client be completed by using the functions below
 * (::SOPC_SecureConnectionConfig_AddSecurityConfig, etc.)
 *
 * \param endpointUrl    URL of the endpoint: \verbatim opc.tcp://<host>:<port>[/<name>] \endverbatim
 * \param secuMode       Security mode required for this SecureConnection: None, Sign or SignAndEncrypt.
 *                       If value different from None, ::SOPC_SecureConnectionConfig_AddSecurityConfig shall be called.
 *
 * \return SOPC_SecureConnectionConfig pointer to configuration structure to be filled
 *         with ::SOPC_SecureConnectionConfig_AddSecurityConfig.
 *         Otherwise Returns NULL if no more configuration slots are available
 *         (see ::SOPC_MAX_SECURE_CONNECTIONS).
 */
SOPC_SecureConnection_Config* SOPC_HelperConfigClient_CreateSecureConnection(const char* userDefinedId,
                                                                             const char* endpointUrl,
                                                                             OpcUa_MessageSecurityMode secuMode,
                                                                             SOPC_SecurityPolicy_URI secuPolicy);

/**
 * \brief Defines the Secure Connection expected EndpointsDescription from given GetEndpointsResponse.
 *        If defined, it is used for verification of coherence during the session activation.
 *
 * \param scConfig               The secure connection configuration to set
 * \param getEndpointsResponse   The client reverse endpoint to be used for reverse connection with the server
 */
SOPC_ReturnStatus SOPC_SecureConnectionConfig_SetExpectedEndpointsDescription(
    SOPC_SecureConnection_Config* secConnConfig,
    OpcUa_GetEndpointsResponse* getEndpointsResponse);

/**
 * \brief Sets the Secure Connection in reverse connection mode
 *
 * \param scConfig               The secure connection configuration to set
 * \param clientReverseEndpoint  The client reverse endpoint to be used for reverse connection with the server
 */
SOPC_ReturnStatus SOPC_SecureConnectionConfig_SetReverseConnection(SOPC_SecureConnection_Config* secConnConfig,
                                                                   const char* clientReverseEndpoint);

/**
 * A default value is used if not provided.
 *
 *  \param reqLifetime Requested lifetime for the secure channel between 2 renewal.
 *
 */
SOPC_ReturnStatus SOPC_SecureConnectionConfig_AddReqLifetime(SOPC_SecureConnection_Config* secConnConfig,
                                                             uint32_t reqLifetime);

/* Well known server: secu policy + certificate */

SOPC_ReturnStatus SOPC_SecureConnectionConfig_AddSecurityConfigFromPath(SOPC_SecureConnection_Config* secConnConfig,
                                                                        const char* serverCertPath);

/* NOT IMPLEMENTED
 *
 * SOPC_ReturnStatus SOPC_SecureConnectionConfig_AddSecurityConfigFromBytes(SOPC_SecureConnection_Config* secConnConfig,
 *                                                                         size_t certificateNbBytes,
 *                                                                        const unsigned char* serverCertificate);
 */
/* NOT IMPLEMENTED
 *
 * Unknown server certificate / policy:
 * - retrieve server certificate through getEndpoints
 * - might require a minimum security policy or not
 * SOPC_ReturnStatus SOPC_SecureConnectionConfig_AddAutoSecurityConfig(SOPC_SecureConnectionConfig* scConfig,
 *                                                                  SOPC_SecurityPolicy_URI* reqSecuPolicy);
 */

/* NOT IMPLEMENTED
 * SOPC_ReturnStatus SOPC_SecureConnectionConfig_AddSessionConfig(SOPC_SecureConnectionConfig* scConfig,
 *                                                             uint32_t reqSessionTimeout)
 */

/* User configuration: not configured == anonymous, only 1 configuration possible */

SOPC_ReturnStatus SOPC_SecureConnectionConfig_AddUserName(SOPC_SecureConnection_Config* secConnConfig,
                                                          const char* userPolicyId,
                                                          const char* userName);

SOPC_ReturnStatus SOPC_SecureConnectionConfig_AddUserX509FromPath(SOPC_SecureConnection_Config* secConnConfig,
                                                                  const char* userCertPath,
                                                                  const char* userKeyPath,
                                                                  bool encrypted);

// Note: any interest not to provide password through those functions and use callbacks ? And in server ?

SOPC_ReturnStatus SOPC_SecureConnectionConfig_AddUserX509FromBytes(SOPC_SecureConnection_Config* secConnConfig,
                                                                   size_t certificateNbBytes,
                                                                   const unsigned char* userCertificate,
                                                                   size_t keyNbBytes,
                                                                   const unsigned char* userPrivateKey);

SOPC_ReturnStatus SOPC_HelperConfigClient_GetSecureConnectionConfigs(size_t* nbScConfigs,
                                                                     SOPC_SecureConnection_Config** scConfigArray);

#endif
