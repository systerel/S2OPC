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
 * \brief Additional server configuration facilities for non-XML configuration of the server
 *        or non-essential advanced configuration.
 *
 * \note TLDR: if server configuration is done through XML configuration files, you might ignore this header.
 */

#ifndef LIBS2OPC_SERVER_CONFIG_CUSTOM_H_
#define LIBS2OPC_SERVER_CONFIG_CUSTOM_H_

#include <stdbool.h>

#include "libs2opc_server_config.h"

#include "sopc_address_space.h"

/** \brief Server configuration without XML */

/**
 * \brief Define server namespaces from an array of strings.
 *        Index in array is the namespace index starting to 1 for first element,
 *        namespace 0 is reserved for OPC UA namespace and is implicitely declared.
 *
 * \param nbNamespaces  The number of namespaces defined in the array
 * \param namespaces    The array of namespaces. Array and its content is copied by function.
 *
 * \return SOPC_STATUS_OK in case of success, otherwise SOPC_STATUS_INVALID_PARAMETERS
 *         if \p nbNamespaces == 0 or \p namespaces is invalid
 *         or SOPC_STATUS_INVALID_STATE if the configuration is not possible
 *         (toolkit not initialized, namesapces already defined, server already started).
 */
SOPC_ReturnStatus SOPC_HelperConfigServer_SetNamespaces(size_t nbNamespaces, char** namespaces);

/**
 * \brief Define server locales ids supported from an array of locale strings.
 *
 * \warning The application name shall be defined for each supported locale defined here
 *          (use ::SOPC_HelperConfigServer_AddApplicationNameLocale when more than one locale supported)
 *
 * \param nbLocales  The number of locales defined in the array. It might be 0 if no locale defined (only default exist)
 * \param localeIds  The array of locales. Array and its content is copied by function.
 *
 * \return SOPC_STATUS_OK in case of success, otherwise SOPC_STATUS_INVALID_PARAMETERS
 *         if \p localeIds is invalid when \p nbLocales \> 0
 *         or SOPC_STATUS_INVALID_STATE if the configuration is not possible
 *         (toolkit not initialized, localesIds already defined, server already started).
 */
SOPC_ReturnStatus SOPC_HelperConfigServer_SetLocaleIds(size_t nbLocales, char** localeIds);

/**
 * \brief Define server application description
 *
 * \param applicationUri        The globally unique identifier for the application instance.
 *                              This URI is used as ServerUri in Services if the application is a Server.
 * \param productUri            The globally unique identifier for the product.
 * \param defaultAppName        The name of the application using the default locale language.
 * \param defaultAppNameLocale  The default locale if any. If defined it shall exists in supported locales.
 * \param applicationType       The type of application, it shall be one of the OpcUa_ApplicationType_*Server types
 *
 * \return SOPC_STATUS_OK in case of success, otherwise SOPC_STATUS_INVALID_PARAMETERS
 *         if \p applicationUri, \p productUri or \p defaultAppName are invalid
 *         or SOPC_STATUS_INVALID_STATE if the configuration is not possible
 *         (toolkit not initialized, application description already set, server already started).
 *
 * \note Supported locales shall be defined using ::SOPC_HelperConfigServer_SetLocaleIds prior to this function call.
 * \note If several locales are supported by server, application name shall be defined for each supported locale.
 *       Use ::SOPC_HelperConfigServer_AddApplicationNameLocale to add all application name necessary.
 */
SOPC_ReturnStatus SOPC_HelperConfigServer_SetApplicationDescription(const char* applicationUri,
                                                                    const char* productUri,
                                                                    const char* defaultAppName,
                                                                    const char* defaultAppNameLocale,
                                                                    OpcUa_ApplicationType applicationType);

/**
 * \brief Define server additional application name with given locale id
 *
 * \param additionalAppName        The name of the application using the additional locale language.
 * \param additionalAppNameLocale  Locale used for the application name, it shall exists in supported locales of the
 *                                 server.
 *
 * \return SOPC_STATUS_OK in case of success, otherwise SOPC_STATUS_INVALID_PARAMETERS
 *         if \p additionalApplicationName or \p additionalApplicationNameLocale are invalid
 *         or SOPC_STATUS_INVALID_STATE if the configuration is not possible
 *         (toolkit not initialized, server already started).
 *
 * \note Supported locales shall be defined using ::SOPC_HelperConfigServer_SetLocaleIds prior to this function call.
 * \note This function shall not be called before defining default name and locale with
 * ::SOPC_HelperConfigServer_SetApplicationDescription
 */
SOPC_ReturnStatus SOPC_HelperConfigServer_AddApplicationNameLocale(const char* additionalAppName,
                                                                   const char* additionalAppNameLocale);

/**
 * \brief Define the PKI provider that will be in charge of validating certificates received by server.
 *
 * \param pki  The PKI provider to be used.
 *             It will be automatically deallocated using ::SOPC_PKIProvider_Free on call to
 *             ::SOPC_HelperConfigServer_Clear.
 *
 * \return SOPC_STATUS_OK in case of success, otherwise SOPC_STATUS_INVALID_PARAMETERS
 *         if \p pki is invalid
 *         or SOPC_STATUS_INVALID_STATE if the configuration is not possible
 *         (toolkit not initialized, PKI already defined, server already started).
 *
 * \note A default PKI provider compliant with OPC UA standard is provided in sopc_pki_stack.h
 */
SOPC_ReturnStatus SOPC_HelperConfigServer_SetPKIprovider(SOPC_PKIProvider* pki);

/**
 * \brief Set asymmetrical certificate and key of server from file paths.
 *        Certificate files shall use DER format, key file shall use DER or PEM format.
 *
 * \param serverCertPath  Path to server certificate file at DER format
 * \param serverKeyPath   Path to server key file at DER or PEM format
 *
 * \return SOPC_STATUS_OK in case of success, otherwise SOPC_STATUS_INVALID_PARAMETERS
 *         if \p serverCertPath or \p serverKeyPath are invalid
 *         or SOPC_STATUS_INVALID_STATE if the configuration is not possible
 *         (toolkit not initialized, key/cert pair already set, server already started).
 */
SOPC_ReturnStatus SOPC_HelperConfigServer_SetKeyCertPairFromPath(const char* serverCertPath, const char* serverKeyPath);

/**
 * \brief Set asymmetrical certificate and key of server from byte arrays.
 *        Certificate shall be in DER format, key file shall be in DER or PEM format.
 *
 * \param certificateNbBytes Number of elements in \p serverCertificate array
 * \param serverCertificate  Array of bytes containing server certificate at DER format
 * \param keyNbBytes         Number of elements in \p serverPrivateKey array
 * \param serverPrivateKey   Array of bytes containing server key file at DER or PEM format
 *
 * \return SOPC_STATUS_OK in case of success, otherwise SOPC_STATUS_INVALID_PARAMETERS
 *         if \p certificateNbBytes, \p serverCertificate, \p keyNbBytes or \p serverKeyPath are invalid (0 or NULL)
 *         or SOPC_STATUS_INVALID_STATE if the configuration is not possible
 *         (toolkit not initialized, key/cert pair already set, server already started).
 */
SOPC_ReturnStatus SOPC_HelperConfigServer_SetKeyCertPairFromBytes(size_t certificateNbBytes,
                                                                  const unsigned char* serverCertificate,
                                                                  size_t keyNbBytes,
                                                                  const unsigned char* serverPrivateKey);

/**
 * \brief Create a new endpoint configuration in server to be completed by using the functions below
 * (::SOPC_EndpointConfig_AddSecurityConfig, etc.)
 *
 * \param url           URL of the endpoint: \verbatim opc.tcp://<host>:<port>[/<name>] \endverbatim
 * \param hasDiscovery  If set activate discovery endpoint facility on same endpoint URL.
 *                      Discovery services are then accessible without any security even
 *                      if endpoint only allow secure connections (Sign or SignAndEncrypt)
 *
 * \return SOPC_Endpoint_Config pointer to configuration structure to be filled
 *         with ::SOPC_EndpointConfig_AddSecurityConfig.
 *         Otherwise Returns NULL if no more configuration slots are available
 *         (see SOPC_MAX_ENDPOINT_DESCRIPTION_CONFIGURATIONS).
 *
 * \note Invalid parameter or out of memory issue will result in assertion failure.
 * \note The returned pointer points to static memory and should not be freed or reused once configuration completed.
 *
 */
SOPC_Endpoint_Config* SOPC_HelperConfigServer_CreateEndpoint(const char* url, bool hasDiscovery);

/**
 * \brief Enumerated values authorized for use with ::SOPC_EndpointConfig_AddSecurityConfig.
 *        Values are limited to the security policies supported by server.
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
 * \brief The structure containing an endpoint security configuration.
 */
typedef SOPC_SecurityPolicy SOPC_SecurityConfig;

/**
 * \brief Add a security policy to the endpoint configuration
 *
 * \param destEndpoint Pointer to endpoint created with ::SOPC_HelperConfigServer_CreateEndpoint
 * \param uri          Security policy ::SOPC_SecurityPolicy_URI supported by \p destEndpoint
 *
 * \return A pointer to the new security configuration supported
 *         or NULL if ::SOPC_MAX_SECU_POLICIES_CFG are already defined.
 *         The new security policy shall be completed using ::SOPC_SecurityConfig_SetSecurityModes and
 *         ::SOPC_SecurityConfig_AddUserTokenPolicy.
 *
 * \note   The returned pointer points to static memory and should not be freed or reused once configuration completed.
 */
SOPC_SecurityConfig* SOPC_EndpointConfig_AddSecurityConfig(SOPC_Endpoint_Config* destEndpoint,
                                                           SOPC_SecurityPolicy_URI uri);

/**
 * \brief Enumerated mask values authorized for use with ::SOPC_SecurityConfig_SetSecurityModes.
 *        Those values are masks which means they might be used with OR bitwise operation to
 *        activate several security modes.
 */
typedef enum
{
    SOPC_SecurityModeMask_None = 0x01, /*!< Mask to activate mode with no security applied on exchanges */
    SOPC_SecurityModeMask_Sign =
        0x02, /*!< Mask to activate mode with signature of exchanges (and encryption during Secure Channel opening) */
    SOPC_SecurityModeMask_SignAndEncrypt = 0x04 /*!< Mask to activate mode with signature and encryption of exchanges */
} SOPC_SecurityModeMask;

/**
 * \brief Set a security mode mask to the security configuration
 *
 * \param destSecuConfig Pointer to security configuration added with ::SOPC_EndpointConfig_AddSecurityConfig
 * \param modes          Mask of security modes to be supported using a bitwise OR of ::SOPC_SecurityModeMask
 *
 * \return SOPC_STATUS_OK in case of success, otherwise SOPC_STATUS_INVALID_PARAMETERS
 *         if \p destSecuConfig or \p modes are invalid (0 or NULL)
 *
 * \note Security configuration with the None security policy does not support any mode except None
 */
SOPC_ReturnStatus SOPC_SecurityConfig_SetSecurityModes(SOPC_SecurityConfig* destSecuConfig,
                                                       SOPC_SecurityModeMask modes);

/**
 * \brief User token policy type to be used with ::SOPC_SecurityConfig_AddUserTokenPolicy
 */
typedef OpcUa_UserTokenPolicy SOPC_UserTokenPolicy;

/**
 * \brief Add a user token policy to the security policy
 *
 * \param destSecuConfig   Pointer to security policy added with ::SOPC_EndpointConfig_AddSecurityConfig
 * \param userTokenPolicy  User token policy to use for this security policy.
 *                         ::SOPC_UserTokenPolicy_Anonymous, ::SOPC_UserTokenPolicy_UserName_DefaultSecurityPolicy or
 *                         ::SOPC_UserTokenPolicy_UserName_NoneSecurityPolicy supported
 *
 * \return SOPC_STATUS_OK in case of success, otherwise SOPC_STATUS_INVALID_PARAMETERS
 *         if \p destSecuConfig or \p modes are invalid (NULL),
 *         or SOPC_STATUS_OUT_OF_MEMORY if already ::SOPC_MAX_SECU_POLICIES_CFG
 *         user token policies defined in this security policy
 *
 * \note ::SOPC_UserTokenPolicy_UserName_NoneSecurityPolicy shall never be used
 *       in conjunction with None or Sign only security mode to avoid possible user credential leaks.
 */
SOPC_ReturnStatus SOPC_SecurityConfig_AddUserTokenPolicy(SOPC_SecurityConfig* destSecuConfig,
                                                         const SOPC_UserTokenPolicy* userTokenPolicy);

/**
 * \brief Add a reverse connection configuration for a client to the server endpoint configuration
 *
 * \param destEndpoint       Pointer to endpoint created with ::SOPC_HelperConfigServer_CreateEndpoint
 * \param clientAppUri       The client ApplicationUri. It might be empty since it is not checked for now.
 * \param clientEndpointUrl  The client endpoint URL to connect to establish the reverse connection.
 *
 * \return True if the client to connect configuration was added successfully
 *         or false if ::SOPC_MAX_REVERSE_CLIENT_CONNECTIONS are already defined or invalid parameters are provided.
 */
bool SOPC_EndpointConfig_AddClientToConnect(SOPC_Endpoint_Config* destEndpoint,
                                            const char* clientAppUri,
                                            const char* clientEndpointUrl);
/**
 * \brief Configure the endpoint to not listen to incoming connections
 *        in case only reverse connections shall be possible.
 *        It will fail if no reverse connection is configured.
 *
 * \param destEndpoint  Pointer to endpoint created with ::SOPC_HelperConfigServer_CreateEndpoint
 *
 * \return True if the server endpoint is correctly configured to not listen for incoming connections from client.
 *         False in case no reverse connection configuration was defined with ::SOPC_EndpointConfig_AddClientToConnect
 *         or invalid parameter is provided.
 */
bool SOPC_EndpointConfig_StopListening(SOPC_Endpoint_Config* destEndpoint);

/* Address space configuration without XML */

/**
 * \brief Configure the server with the given address space
 *
 * \param addressSpaceConfig  the address space definition, in case of successful operation
 *                            it is then deallocated on call to ::SOPC_HelperConfigServer_Clear
 *
 *  \return SOPC_STATUS_OK if configuration succeeded,
 *          SOPC_STATUS_INVALID_STATE if the configuration is not possible
 *          (toolkit not initialized, server already started, address space is already set),
 *          SOPC_STATUS_NOK otherwise
 *
 *  \note only one address space can be set, further call will be refused
 */
SOPC_ReturnStatus SOPC_HelperConfigServer_SetAddressSpace(SOPC_AddressSpace* addressSpaceConfig);

/* User authentication and authorization managers configuration without XML */

/**
 * \brief Configure the server user authentication manager in charge to check user credentials
 *
 * \param authenticationMgr  Pointer to the user authentication manager in charge to check user credentials
 *
 * \return SOPC_STATUS_OK in case of success, otherwise SOPC_STATUS_INVALID_PARAMETERS
 *         if \p authenticationMgr is invalid (NULL)
 *         or SOPC_STATUS_INVALID_STATE if the configuration is not possible
 *         (toolkit not initialized, authentication manager already set, server already started).
 *
 *  \note if not called default user managers allowing any user will be instantiated
 */
SOPC_ReturnStatus SOPC_HelperConfigServer_SetUserAuthenticationManager(
    SOPC_UserAuthentication_Manager* authenticationMgr);

/**
 * \brief Configure the server user authorization manager to check user access rights
 *
 * \param authorizationMgr   Pointer to the user authorization manager in charge to check user access rights
 *
 * \return SOPC_STATUS_OK in case of success, otherwise SOPC_STATUS_INVALID_PARAMETERS
 *         if \p certificateNbBytes, \p serverCertificate, \p keyNbBytes or \p serverKeyPath are invalid (0 or NULL)
 *         or SOPC_STATUS_INVALID_STATE if the configuration is not possible
 *         (toolkit not initialized, authorization manager already set, server already started).
 *
 *  \note if not called default user manager allowing any access will be instantiated
 */
SOPC_ReturnStatus SOPC_HelperConfigServer_SetUserAuthorizationManager(SOPC_UserAuthorization_Manager* authorizationMgr);

/**
 * \brief Change the software build information to be displayed in the server build info node
 *
 * \param buildInfo  the build information name to display in server build info node.
 *                   Content is copied and can be cleared after call.
 *
 * \return SOPC_STATUS_OK in case of success, otherwise SOPC_STATUS_INVALID_PARAMETERS
 *         if \p buildInfo is invalid (NULL)
 *         or SOPC_STATUS_INVALID_STATE if the configuration is not possible
 *         (toolkit not initialized, build info already set, server already started).
 */
SOPC_ReturnStatus SOPC_HelperConfigServer_SetSoftwareBuildInfo(OpcUa_BuildInfo* buildInfo);

#endif
