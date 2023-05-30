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
 *         (wrapper not initialized, preferred localesIds already defined, client connection initiated).
 */
SOPC_ReturnStatus SOPC_ClientConfigHelper_SetPreferredLocaleIds(size_t nbLocales, const char** localeIds);

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
 *         (wrapper not initialized, application description already set, client connection initiated).
 */
SOPC_ReturnStatus SOPC_ClientConfigHelper_SetApplicationDescription(const char* applicationUri,
                                                                    const char* productUri,
                                                                    const char* defaultAppName,
                                                                    const char* defaultAppNameLocale,
                                                                    OpcUa_ApplicationType applicationType);

/**
 * \brief Defines the PKI provider that will be in charge of validating certificates received by client.
 *
 * \param pki  The PKI provider to be used.
 *             It will be automatically deallocated using ::SOPC_PKIProvider_Free on call to
 *             ::SOPC_ClientConfigHelper_Clear.
 *
 * \return SOPC_STATUS_OK in case of success, otherwise SOPC_STATUS_INVALID_PARAMETERS
 *         if \p pki is invalid
 *         or SOPC_STATUS_INVALID_STATE if the configuration is not possible
 *         (wrapper not initialized, PKI already defined, server already started).
 *
 * \note A default PKI provider compliant with OPC UA standard is provided in sopc_pki_stack.h
 */
SOPC_ReturnStatus SOPC_ClientConfigHelper_SetPKIprovider(SOPC_PKIProvider* pki);

/**
 * \brief Sets asymmetrical certificate and key of client from file paths.
 *        Certificate files shall use DER format, key file shall use DER or PEM format.
 *
 * \param clientCertPath  Path to client certificate file at DER format (copied by function)
 * \param clientKeyPath   Path to client key file at DER or PEM format (copied by function)
 * \param encrypted       Whether if the key is encrypted or not
 *
 * \return SOPC_STATUS_OK in case of success, otherwise SOPC_STATUS_INVALID_PARAMETERS
 *         if \p clientCertPath or \p clientKeyPath are invalid
 *         or SOPC_STATUS_INVALID_STATE if the configuration is not possible
 *         (wrapper not initialized, key/cert pair already set, connection initiated).
 */
SOPC_ReturnStatus SOPC_ClientConfigHelper_SetKeyCertPairFromPath(const char* clientCertPath,
                                                                 const char* clientKeyPath,
                                                                 bool encrypted);

/**
 * \brief Sets asymmetrical certificate and key of client from byte arrays.
 *        Certificate shall be in DER format, key file shall be in DER or PEM format.
 *
 * \param certificateNbBytes Number of elements in \p clientCertificate array
 * \param clientCertificate  Array of bytes containing client certificate at DER format (copied by function)
 * \param keyNbBytes         Number of elements in \p clientPrivateKey array
 * \param clientPrivateKey   Array of bytes containing client key file at DER or PEM format (copied by function)
 *
 * \return SOPC_STATUS_OK in case of success, otherwise SOPC_STATUS_INVALID_PARAMETERS
 *         if \p certificateNbBytes, \p clientCertificate, \p keyNbBytes or \p clientPrivateKey are invalid (0 or NULL)
 *         or SOPC_STATUS_INVALID_STATE if the configuration is not possible
 *         (wrapper not initialized, key/cert pair already set).
 */
SOPC_ReturnStatus SOPC_ClientConfigHelper_SetKeyCertPairFromBytes(size_t certificateNbBytes,
                                                                  const unsigned char* clientCertificate,
                                                                  size_t keyNbBytes,
                                                                  const unsigned char* clientPrivateKey);

/**
 * \brief Create a new secure channel configuration in client be completed by using the functions below
 * (::SOPC_SecureConnectionConfig_SetServerCertificateFromPath or
 *  ::SOPC_SecureConnectionConfig_SetServerCertificateFromBytes, etc.)
 *
 * \param userDefinedId  A user defined identifier to retrieve the secure connection configuration
 *                       using ::SOPC_ClientConfigHelper_GetConfigFromId.
 * \param endpointUrl    URL of the endpoint: \verbatim opc.tcp://<host>:<port>[/<name>] \endverbatim
 * \param secuMode       Security mode required for this SecureConnection: None, Sign or SignAndEncrypt.
 *                       If value different from None, SOPC_SecureConnectionConfig_AddServerCertificate* shall be
 *                       called.
 * \param secuPolicy     Security policy URI required for this SecureConnection.
 *                       If value different from None, SOPC_SecureConnectionConfig_AddServerCertificate* shall be
 *                       called.
 *
 * \return SOPC_SecureConnectionConfig pointer to configuration structure to be filled
 *         with ::SOPC_SecureConnectionConfig_AddSecurityConfig.
 *         Otherwise Returns NULL if no more configuration slots are available
 *         (see ::SOPC_MAX_CLIENT_SECURE_CONNECTIONS_CONFIG).
 */
SOPC_SecureConnection_Config* SOPC_ClientConfigHelper_CreateSecureConnection(const char* userDefinedId,
                                                                             const char* endpointUrl,
                                                                             OpcUa_MessageSecurityMode secuMode,
                                                                             SOPC_SecurityPolicy_URI secuPolicy);

/**
 * \brief Defines the Secure Connection expected EndpointsDescription from given GetEndpointsResponse.
 *        If defined, it is used for verification of coherence during the session activation.
 *
 * \param scConfig               The secure connection configuration to modify
 * \param getEndpointsResponse   The client expected endpoint description to be returned by the server during
 *                               connection. Connection will be aborted otherwise.
 *                               The response will be copied and might be deallocated after call.
 *
 *  \return SOPC_STATUS_OK in case of success, SOPC_STATUS_INVALID_PARAMETERS in case of NULL parameters,
 *          SOPC_STATUS_INVALID_STATE if the if the configuration is not possible (wrapper not initialized)
 *          or connection config cannot be modified (already used for a connection
 *          or expected endpoint already set), SOPC_STATUS_OUT_OF_MEMORY if OOM raised.
 */
SOPC_ReturnStatus SOPC_SecureConnectionConfig_SetExpectedEndpointsDescription(
    SOPC_SecureConnection_Config* secConnConfig,
    const OpcUa_GetEndpointsResponse* getEndpointsResponse);

/**
 * \brief Sets the Secure Connection in reverse connection mode
 *
 * \param scConfig                  The secure connection configuration to set
 * \param clientReverseEndpointUri  The client reverse endpoint URI to be used to listen for reverse connection from the
 * server
 *
 *  \return SOPC_STATUS_OK in case of success, SOPC_STATUS_INVALID_PARAMETERS in case of NULL parameters,
 *          SOPC_STATUS_INVALID_STATE if the if the configuration is not possible (wrapper not initialized)
 *          or connection config cannot be modified (already used for a connection
 *          or reverse endpoint already set), SOPC_STATUS_OUT_OF_MEMORY if OOM raised.
 */
SOPC_ReturnStatus SOPC_SecureConnectionConfig_SetReverseConnection(SOPC_SecureConnection_Config* secConnConfig,
                                                                   const char* clientReverseEndpointUri);

/**
 * A default value is used if not provided.
 *
 *  \param reqLifetime Requested lifetime for the secure channel between 2 renewal.
 *
 */
SOPC_ReturnStatus SOPC_SecureConnectionConfig_SetReqLifetime(SOPC_SecureConnection_Config* secConnConfig,
                                                             uint32_t reqLifetime);

SOPC_ReturnStatus SOPC_SecureConnectionConfig_SetServerCertificateFromPath(SOPC_SecureConnection_Config* secConnConfig,
                                                                           const char* serverCertPath);

SOPC_ReturnStatus SOPC_SecureConnectionConfig_SetServerCertificateFromBytes(SOPC_SecureConnection_Config* secConnConfig,
                                                                            size_t certificateNbBytes,
                                                                            const unsigned char* serverCertificate);

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

/**
 * \brief Defines the user authentication mode as anonymous for the secure connection
 *        and set the associated user policy Id to be used in server
 *
 * \note By default, the user authentication mode is anonymous and user policy Id is ""
 *
 * \param secConnConfig  The secure connection configuration to set
 * \param userPolicyId   The user policy Id to be used in server for anonymous
 *                       (might not be verified by server for anonymous)
 *
 * \return SOPC_STATUS_OK in case of success, SOPC_STATUS_INVALID_PARAMETERS in case of NULL parameters,
 *         SOPC_STATUS_INVALID_STATE if the if the configuration is not possible (wrapper not initialized)
 *         or connection config cannot be modified (user authentication mode already set).
 */
SOPC_ReturnStatus SOPC_SecureConnectionConfig_SetAnonymous(SOPC_SecureConnection_Config* secConnConfig,
                                                           const char* userPolicyId);

/**
 * \brief Defines the user authentication mode as username/password for the secure connection,
 *        sets the associated user policy Id to be used in server
 *        and sets the username/password to be used for authentication.
 *
 * \param secConnConfig  The secure connection configuration to set
 * \param userPolicyId   The user policy Id to be used in server for username/password
 * \param userName       The username to be used for authentication
 * \param password       The password to be used for authentication or NULL to be retrieved from the callback defined
 *                       with ::SOPC_ClientConfigHelper_SetUserNamePasswordCallback.
 *                       Note: the password should not be hardcoded string in the code.
 *
 * \return SOPC_STATUS_OK in case of success, SOPC_STATUS_INVALID_PARAMETERS in case of NULL parameters,
 *         SOPC_STATUS_INVALID_STATE if the if the configuration is not possible (wrapper not initialized)
 *         or connection config cannot be modified (user authentication mode already set).
 */
SOPC_ReturnStatus SOPC_SecureConnectionConfig_SetUserName(SOPC_SecureConnection_Config* secConnConfig,
                                                          const char* userPolicyId,
                                                          const char* userName,
                                                          const char* password);

/**
 * \brief Defines the user authentication mode as X509 certificate for the secure connection,
 *       sets the associated user policy Id to be used in server
 *       and sets the certificate/key paths to be used for authentication.
 *
 * \param secConnConfig  The secure connection configuration to set
 * \param userPolicyId   The user policy Id to be used in server for X509 certificate
 * \param userCertPath   The path to the user certificate file at DER format (copied by function)
 * \param userKeyPath    The path to the user key file at DER or PEM format (copied by function)
 * \param encrypted      Whether if the key is encrypted or not
 *
 * \return SOPC_STATUS_OK in case of success, SOPC_STATUS_INVALID_PARAMETERS in case of NULL parameters,
 *         SOPC_STATUS_INVALID_STATE if the if the configuration is not possible (wrapper not initialized)
 *         or connection config cannot be modified (user authentication mode already set).
 */
SOPC_ReturnStatus SOPC_SecureConnectionConfig_SetUserX509FromPaths(SOPC_SecureConnection_Config* secConnConfig,
                                                                   const char* userPolicyId,
                                                                   const char* userCertPath,
                                                                   const char* userKeyPath,
                                                                   bool encrypted);

/**
 * \brief Sets asymmetrical certificate and key of user from byte arrays.
 *        Certificate shall be in DER format, key file shall be in DER or PEM format.
 *
 * \param secConnConfig  The secure connection configuration to set
 * \param userPolicyId   The user policy Id to be used in server for X509 certificate
 * \param certificateNbBytes Number of elements in \p clientCertificate array
 * \param userCertificate    Array of bytes containing user certificate at DER format (copied by function)
 * \param keyNbBytes         Number of elements in \p clientPrivateKey array
 * \param userPrivateKey     Array of bytes containing user key file at DER or PEM format (copied by function)
 *
 * \return SOPC_STATUS_OK in case of success, otherwise SOPC_STATUS_INVALID_PARAMETERS
 *         if \p certificateNbBytes, \p userCertificate, \p keyNbBytes or \p userPrivateKey are invalid (0 or NULL)
 *         or SOPC_STATUS_INVALID_STATE if the configuration is not possible
 *         (wrapper not initialized, key/cert pair already set).
 */
SOPC_ReturnStatus SOPC_SecureConnectionConfig_SetUserX509FromBytes(SOPC_SecureConnection_Config* secConnConfig,
                                                                   const char* userPolicyId,
                                                                   size_t certificateNbBytes,
                                                                   const unsigned char* userCertificate,
                                                                   size_t keyNbBytes,
                                                                   const unsigned char* userPrivateKey);

SOPC_ReturnStatus SOPC_ClientConfigHelper_GetSecureConnectionConfigs(size_t* nbScConfigs,
                                                                     SOPC_SecureConnection_Config*** scConfigArray);

#endif
