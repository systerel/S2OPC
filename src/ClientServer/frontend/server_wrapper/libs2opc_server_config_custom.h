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
 *         (toolkit not initialized, server already started).
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
 *         (toolkit not initialized, server already started).
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
 *         (toolkit not initialized, server already started).
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
 * server.
 *
 * \return SOPC_STATUS_OK in case of success, otherwise SOPC_STATUS_INVALID_PARAMETERS
 *         if \p additionalApplicationName or \p additionalApplicationNameLocale are invalid
 *         or SOPC_STATUS_INVALID_STATE if the configuration is not possible
 *         (toolkit not initialized, server already started).
 *
 * \note Supported locales shall be defined using ::SOPC_HelperConfigServer_SetLocaleIds prior to this function call.
 */
SOPC_ReturnStatus SOPC_HelperConfigServer_AddApplicationNameLocale(const char* additionalAppName,
                                                                   const char* additionalAppNameLocale);

/**
 * \brief Define the PKI provider that will be in charge of validating certificates received by server.
 *
 * \param pki  The PKI provider to be used.
 *             It will be automatically deallocated using ::SOPC_PKIProvider_Free on call to ::SOPC_Helper_Clear.
 *
 * \return SOPC_STATUS_OK in case of success, otherwise SOPC_STATUS_INVALID_PARAMETERS
 *         if \p pki is invalid
 *         or SOPC_STATUS_INVALID_STATE if the configuration is not possible
 *         (toolkit not initialized, server already started).
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
 *         (toolkit not initialized, server already started).
 */
SOPC_ReturnStatus SOPC_HelperConfigServer_SetCertificateFromPath(const char* serverCertPath, const char* serverKeyPath);

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
 *         (toolkit not initialized, server already started).
 */
SOPC_ReturnStatus SOPC_HelperConfigServer_SetCertificateFromBytes(size_t certificateNbBytes,
                                                                  const unsigned char* serverCertificate,
                                                                  size_t keyNbBytes,
                                                                  const unsigned char* serverPrivateKey);

/**
 * \brief Attach a new endpoint configuration to server configuration.
 *        Use functions below to create the endpoint configuration
 *        (see ::SOPC_HelperConfigServer_AddEndpoint, etc.)
 *
 * \param endpointConfig  The endpoint configuration to add to server configuration
 *
 * \return SOPC_STATUS_OK in case of success, otherwise SOPC_STATUS_INVALID_PARAMETERS
 *         if \p endpointConfig is invalid (NULL)
 *         or SOPC_STATUS_INVALID_STATE if the configuration is not possible
 *         (toolkit not initialized, server already started).
 * \note \p endpointConfig shall have been built with below constructor and not forged manually.
 *       Allocation issues and restrictions managed by constructors.
 * \note \p endpointConfig shall not be accessed nor used after call to this function
 *       and structure does not need to be kept in memory scope after this call.
 */
SOPC_ReturnStatus SOPC_HelperConfigServer_AddEndpoint(SOPC_Endpoint_Config* endpointConfig);

/**
 * \brief Create a new endpoint configuration structure to configure with the functions below
 * (::SOPC_EndpointConfig_AddSecurityPolicy, etc.)
 *
 * \param url           URL of the endpoint: \verbatim opc.tcp://<host>:<port>[/<name>] \endverbatim
 * \param hasDiscovery  If set activate discovery endpoint facility on same endpoint URL.
 *                      Discovery services are then accessible without any security even
 *                      if endpoint only allow secure connections (Sign or SignAndEncrypt)
 *
 * \return SOPC_Endpoint_Config structure to be filled with ::SOPC_EndpointConfig_AddSecurityPolicy
 *         and added to server configuration once filled using ::SOPC_HelperConfigServer_AddEndpoint
 *
 * \note Invalid parameter or out of memory issue will result in assertion failure.
 */
SOPC_Endpoint_Config SOPC_EndpointConfig_Create(const char* url, bool hasDiscovery);

/**
 * \brief Enumerated values authorized for use with ::SOPC_EndpointConfig_AddSecurityPolicy.
 *        Values are limited to the security policies supported by server.
 */
typedef enum
{
    SOPC_SecurityPolicy_None,           /*!< http://opcfoundation.org/UA/SecurityPolicy#None */
    SOPC_SecurityPolicy_Basic256,       /*!< http://opcfoundation.org/UA/SecurityPolicy#Basic256 */
    SOPC_SecurityPolicy_Basic256Sha256, /*!< http://opcfoundation.org/UA/SecurityPolicy#Basic256Sha256 */
} SOPC_SecurityPolicy_URI;

/**
 * \brief Add a security policy to the endpoint configuration
 *
 * \param destEndpoint Pointer to endpoint created with ::SOPC_EndpointConfig_Create
 * \param uri          Security policy ::SOPC_SecurityPolicy_URI supported by \p destEndpoint
 *
 * \return A pointer to the new security policy supported or NULL if ::SOPC_MAX_SECU_POLICIES_CFG are already defined.
 *         The new security policy shall be completed using ::SOPC_SecurityPolicy_AddSecurityModes and
 *         ::SOPC_SecurityPolicy_AddUserTokenPolicy
 */
SOPC_SecurityPolicy* SOPC_EndpointConfig_AddSecurityPolicy(SOPC_Endpoint_Config* destEndpoint,
                                                           SOPC_SecurityPolicy_URI uri);

/**
 * \brief Enumerated mask values authorized for use with ::SOPC_SecurityPolicy_AddSecurityModes.
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
 * \brief Add a security mode mask to the security policy
 *
 * \param destSecuPolicy Pointer to security policy added with ::SOPC_EndpointConfig_AddSecurityPolicy
 * \param modes          Mask of security modes to be supported using a bitwise disjunction of ::SOPC_SecurityModeMask
 *
 * \return SOPC_STATUS_OK in case of success, otherwise SOPC_STATUS_INVALID_PARAMETERS
 *         if \p destSecuPolicy or \p modes are invalid (0 or NULL)
 *
 * \note the None security policy does not support any mode except None
 */
SOPC_ReturnStatus SOPC_SecurityPolicy_SetSecurityModes(SOPC_SecurityPolicy* destSecuPolicy,
                                                       SOPC_SecurityModeMask modes);

/**
 * \brief User token policy type to be used with ::SOPC_SecurityPolicy_AddUserTokenPolicy
 */
typedef OpcUa_UserTokenPolicy SOPC_UserTokenPolicy;

/**
 * \brief Predefined user token policy for anonymous users
 */
extern const SOPC_UserTokenPolicy SOPC_UserTokenPolicy_Anonymous;

/**
 * \brief Predefined user token policy for users with username and password credentials.
 *        It uses None security policy which means only the security policy
 *        of the endpoint will ensure confidentiality of credentials.
 *        Thus endpoint security policy shall not allow None security mode when using it.
 *
 */
extern const SOPC_UserTokenPolicy SOPC_UserTokenPolicy_UserName_NoneSecurityPolicy;

/**
 * \brief Add a user token policy to the security policy
 *
 * \param destSecuPolicy   Pointer to security policy added with ::SOPC_EndpointConfig_AddSecurityPolicy
 * \param userTokenPolicy  User token policy to use for this security policy.
 *                         By default, only ::SOPC_UserTokenPolicy_Anonymous and
 *                         ::SOPC_UserTokenPolicy_UserName_NoneSecurityPolicy supported
 *
 * \return SOPC_STATUS_OK in case of success, otherwise SOPC_STATUS_INVALID_PARAMETERS
 *         if \p destSecuPolicy or \p modes are invalid (NULL),
 *         or SOPC_STATUS_OUT_OF_MEMORY if already ::SOPC_MAX_SECU_POLICIES_CFG
 *         user token policies defined in this security policy
 *
 * \note ::SOPC_UserTokenPolicy_UserName_NoneSecurityPolicy shall never be used
 *       in conjunction with None security mode to avoid possible user credential leaks.
 */
SOPC_ReturnStatus SOPC_SecurityPolicy_AddUserTokenPolicy(SOPC_SecurityPolicy* destSecuPolicy,
                                                         const SOPC_UserTokenPolicy* userTokenPolicy);

/* Address space configuration without XML */

/**
 * \brief Configure the server with the given address space
 *
 * \param addressSpaceConfig  the address space definition, in case of successful operation
 *                            it is then deallocated on call to ::SOPC_Helper_Clear
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
 * \brief Configure the server user authentication manager and user authorization manager.
 *
 * \param authenticationMgr  Pointer to the user authentication manager in charge to check user credentials
 * \param authorizationMgr   Pointer to the user authorization manager in charge to check user access rights
 *
 * \return SOPC_STATUS_OK in case of success, otherwise SOPC_STATUS_INVALID_PARAMETERS
 *         if \p certificateNbBytes, \p serverCertificate, \p keyNbBytes or \p serverKeyPath are invalid (0 or NULL)
 *         or SOPC_STATUS_INVALID_STATE if the configuration is not possible
 *         (toolkit not initialized, server already started).
 *
 *  \note if not defined default user managers allowing any user and any access will be instantiated
 */
SOPC_ReturnStatus SOPC_HelperConfigServer_SetUserManagers(SOPC_UserAuthentication_Manager* authenticationMgr,
                                                          SOPC_UserAuthorization_Manager* authorizationMgr);

/* Change software manufacturer */

/**
 * \brief Change the software manufacturer name in the server build info node
 *
 * \param manufacturerName  the manufacturer name to display in server build info node
 *
 * \return SOPC_STATUS_OK in case of success, otherwise SOPC_STATUS_INVALID_PARAMETERS
 *         if \p manufacturerName is invalid (0 or NULL)
 *         or SOPC_STATUS_INVALID_STATE if the configuration is not possible
 *         (toolkit not initialized, server already started).
 */
SOPC_ReturnStatus SOPC_HelperConfigServer_SetSwManufacturer(const char* manufacturerName);

#endif
