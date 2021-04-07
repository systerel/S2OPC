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
 * \brief Contains the types to be used by the user application to use the Toolkit
 *
 */

#ifndef SOPC_USER_APP_ITF_H_
#define SOPC_USER_APP_ITF_H_

#include <stdbool.h>

#include "sopc_call_method_manager.h"
#include "sopc_common_build_info.h"
#include "sopc_crypto_profiles.h"
#include "sopc_key_manager.h"
#include "sopc_types.h"
#include "sopc_user_manager.h"

/**
 *  \brief Client static configuration of a Secure Channel
 */
typedef struct SOPC_SecureChannel_Config
{
    uint8_t isClientSc;
    const char* url;
    const SOPC_SerializedCertificate* crt_cli;
    const SOPC_SerializedAsymmetricKey* key_priv_cli;
    const SOPC_SerializedCertificate* crt_srv;
    const SOPC_PKIProvider* pki;
    const char* reqSecuPolicyUri;
    uint32_t requestedLifetime;
    OpcUa_MessageSecurityMode msgSecurityMode;

    uintptr_t internalProtocolData; /**< Internal use only: used to store internal protocol data (set only during
                                       connecting phase) */
} SOPC_SecureChannel_Config;

#define SOPC_SECURITY_MODE_NONE_MASK 0x01
#define SOPC_SECURITY_MODE_SIGN_MASK 0x02
#define SOPC_SECURITY_MODE_SIGNANDENCRYPT_MASK 0x04
#define SOPC_SECURITY_MODE_ANY_MASK 0x07

/** Default anonymous user security policy supported configuration */
extern const OpcUa_UserTokenPolicy SOPC_UserTokenPolicy_Anonymous;

/** Default username security policy supported and configured with security policy None.
 * With this security policy, the password will never be encrypted and this policy
 * shall not be used on unsecured or unencrypted secure channels. */
extern const OpcUa_UserTokenPolicy SOPC_UserTokenPolicy_UserName_NoneSecurityPolicy;

#define SOPC_MAX_SECU_POLICIES_CFG 5 /* Maximum number of security policies in a configuration array */

/**
 * \brief Endpoint security policy configuration
 */
typedef struct SOPC_SecurityPolicy
{
    SOPC_String securityPolicy; /**< Security policy URI supported */
    uint16_t securityModes;     /**< Mask of security modes supported (use combination of SOPC_SECURITY_MODE_*_MASK) */
    uint8_t nbOfUserTokenPolicies; /**< The number elements in the user security policies supported array (<= 10) */
    OpcUa_UserTokenPolicy
        userTokenPolicies[SOPC_MAX_SECU_POLICIES_CFG]; /**< The array of user security policies supported,
                                                        * use the constant predefined policies provided
                                                        * (password encryption is not provided and shall be implemented
                                                        *  by authorization manager if applicable)
                                                        */
} SOPC_SecurityPolicy;

typedef struct SOPC_Server_Config SOPC_Server_Config;

/**
 * \brief Server static configuration of a Endpoint listener
 */
typedef struct SOPC_Endpoint_Config
{
    struct SOPC_Server_Config* serverConfigPtr; /**< Pointer to the server configuration containing this endpoint */
    char* endpointURL;                          /**< Endpoint URL: opc.tcp://IP-HOSTNAME:PORT(/NAME)*/
    bool hasDiscoveryEndpoint; /**< Implicit discovery endpoint with same endpoint URL is added if necessary when set */
    uint8_t nbSecuConfigs;     /**< Number of security configuration (<= SOPC_MAX_SECU_POLICIES_CFG) */
    SOPC_SecurityPolicy
        secuConfigurations[SOPC_MAX_SECU_POLICIES_CFG]; /**< Security policies defined for the current endpoint URL.
                                                         * An implicit discovery endpoint will be defined with this URL
                                                         * if SecurityPolicy None is not present. Otherwise the
                                                         * discovery endpoint and session endpoint are the same for this
                                                         * endpoint URL.
                                                         */

    /* To be instantiated by applicative code: */
    SOPC_UserAuthentication_Manager*
        authenticationManager; /**< The user authentication manager: user authentication on session activation */
    SOPC_UserAuthorization_Manager*
        authorizationManager; /**< The user authorization manager: user access level evaluation */
} SOPC_Endpoint_Config;

/**
 * \brief OPC UA server configuration structure
 */
struct SOPC_Server_Config
{
    bool freeCstringsFlag; /**< A flag to indicate if the C strings contained in the server configuration
                                (and endpoints) shall be freed */
    char** namespaces;     /**< An array of namespaces terminated by a NULL pointer.
                                Index in array is the namespace index. */

    char** localeIds; /**< An array of locale ids supported by the server terminated by a NULL pointer.
                           The OpcUa_ApplicationDescription ApplicationName shall contains a definition for each
                           supported locale (use SOPC_LocalizedText_AddOrSetLocalizedText to add them manually) */
    OpcUa_ApplicationDescription serverDescription; /**< Application description of the server.
                                                         Limitations: the gateway and discovery properties are ignored
                                                         (each endpoint returns its URL as discovery URL)*/
    char* serverCertPath; /**< Temporary path to the server certificate (serverCertificate shall be instantiated by
                             applicative code) */
    char* serverKeyPath;  /**< Temporary path to the server key (serverCertificate shall be instantiated by applicative
                             code) */
    char** trustedRootIssuersList; /**< A pointer to an array of paths to each trusted root CA issuer to use in the
                                  validation chain. The array must contain a NULL pointer to indicate its end. (PKI
                                  provider shall be instantiated using it by applicative code) */
    char** trustedIntermediateIssuersList; /**< A pointer to an array of paths to each trusted intermediate CA issuer to
                                  use in the validation chain. The array must contain a NULL pointer to indicate its
                                  end. (PKI provider shall be instantiated using it by applicative code) */
    char** issuedCertificatesList;         /**< A pointer to an array of paths to each issued certificate to use in the
                                              validation chain. The array must contain a NULL pointer to indicate its end. (PKI
                                              provider shall be instantiated using it by applicative code) */
    char** untrustedRootIssuersList; /**< A pointer to an array of paths to each untrusted root CA issuer to use in the
                                        validation chain. Each issued certificate must have its signing certificate
                                        chain in the untrusted issuers list. (PKI provider shall be instantiated using
                                        it by applicative code) */
    char** untrustedIntermediateIssuersList; /**< A pointer to an array of paths to each untrusted intermediate CA
                                                issuer to use in the validation chain.   Each issued certificate must
                                                have its signing certificate chain in the untrusted issuers list. (PKI
                                                provider shall be instantiated using it by applicative code) */
    char** certificateRevocationPathList;    /**<  A pointer to an array of paths to each certificate revocation list to
                                                use.    Each CA of the trusted issuers list and the untrusted issuers list
                                                must have a    CRL in the list. (PKI provider shall be instantiated using
                                                it    by applicative code)*/
    uint8_t nbEndpoints;                     /**< Number of endpoints defined by the server */
    SOPC_Endpoint_Config* endpoints;         /**< Endpoint configuration array */

    /* To be instantiated by applicative code: */
    SOPC_SerializedCertificate* serverCertificate; /**< Server certificate to be instantiated from path or bytes */
    SOPC_SerializedAsymmetricKey* serverKey;       /**< Server key to be instantiated from path or bytes */
    SOPC_PKIProvider* pki; /**< PKI provider to be instantiated. Possible use of ::SOPC_PKIProviderStack_CreateFromPaths
                              or ::SOPC_PKIProviderStack_Create. */
    SOPC_MethodCallManager* mcm; /**< Method Call service configuration.
                                      Can be instantiated with SOPC_MethodCallManager_Create()
                                      or specific code by applicative code.
                                      Can be NULL if Method Call service is not used. */
};

/**
 * \brief S2OPC server configuration
 */
typedef struct SOPC_S2OPC_Config
{
    SOPC_Server_Config serverConfig; /**< server configuration */
} SOPC_S2OPC_Config;

/**
 *  \brief Client and Server communication events to be managed by applicative code
 */
typedef enum SOPC_App_Com_Event
{
    /* Client application events */
    SE_SESSION_ACTIVATION_FAILURE = 0x700, /**< (Client)<br/>
                                            *   id = internal session id (or 0 if not yet defined)<br/>
                                            *   params = (SOPC_StatusCode)(uintptr_t) status code reason<br/>
                                            *   auxParam = user application session context
                                            */
    SE_ACTIVATED_SESSION,                  /**< (Client)<br/>
                                            *   id = internal session id<br/>
                                            *   auxParam = user application session context
                                            */
    SE_SESSION_REACTIVATING,               /**< (Client) automatic new SC or manual new user on same SC */
                                           /**< (Client)<br/>
                                            *   id = internal session id<br/>
                                            *   auxParam = user application session context
                                            */
    SE_RCV_SESSION_RESPONSE,               /**< (Client)<br/>
                                            *   id = internal session id<br/>
                                            *   params = (OpcUa_<MessageStruct>*) OPC UA message header + payload structure<br/>
                                            *   (deallocated by toolkit after callback call ends)<br/>
                                            *   auxParam = user application request context
                                            */
    SE_CLOSED_SESSION,                     /**< (Client)<br/>
                                            *   id = internal session id<br/>
                                            *   params = (SOPC_StatusCode)(uintptr_t) status code reason<br/>
                                            *   auxParam = user application session context
                                            */
    SE_RCV_DISCOVERY_RESPONSE,             /**< (Client)<br/>
                                            *   params = (OpcUa_<MessageStruct>*) OPC UA discovery message header + payload<br/>
                                            *   structure (deallocated by toolkit after callback call ends) auxParam = user<br/>
                                            *   application request context
                                            */

    SE_SND_REQUEST_FAILED, /**< (Client)<br/>
                            *   idOrStatus = (SOPC_ReturnStatus) status,<br/>
                            *   params = (SOPC_EncodeableType*) request type (shall not be deallocated)<br/>
                            *   auxParam = user application request context
                            */

    /* Server application events */
    SE_CLOSED_ENDPOINT,       /**< (Server)<br/>
                               *   id = endpoint configuration index,<br/>
                               *   auxParam = SOPC_ReturnStatus
                               */
    SE_LOCAL_SERVICE_RESPONSE /**< (Server)<br/>
                               *   id = endpoint configuration index,<br/>
                               *   params = (OpcUa_<MessageStruct>*) OPC UA message header + payload structure<br/>
                               *   (deallocated by toolkit after callback call ends)<br/>
                               *   auxParam = user application request context
                               */
} SOPC_App_Com_Event;

/* Server only interfaces */

/**
 * \brief Server address space access/modification notifications to applicative code
 */
typedef enum SOPC_App_AddSpace_Event
{
    /* Server application events */
    AS_WRITE_EVENT = 0x800, /**< opParam = (OpcUa_WriteValue*) single write value operation<br/>
                             *   opStatus = status of the write operation
                             */
} SOPC_App_AddSpace_Event;

/**
 * \brief Toolkit communication events application callback type
 */
typedef void SOPC_ComEvent_Fct(SOPC_App_Com_Event event, uint32_t IdOrStatus, void* param, uintptr_t appContext);

/**
 * \brief Toolkit address space notification events callback type
 */
typedef void SOPC_AddressSpaceNotif_Fct(SOPC_App_AddSpace_Event event, void* opParam, SOPC_StatusCode opStatus);

/**
 * \brief Toolkit build information
 */
typedef struct
{
    SOPC_Build_Info commonBuildInfo;
    SOPC_Build_Info clientServerBuildInfo;
} SOPC_Toolkit_Build_Info;

/**
 * \brief Initalize the content of the SOPC_S2OPC_Config
 *
 * \param config  The s2opc server configuration to initialize
 */
void SOPC_S2OPC_Config_Initialize(SOPC_S2OPC_Config* config);

/**
 * \brief Clear the content of the SOPC_S2OPC_Config
 *
 * \param config  The s2opc server configuration to clear
 */
void SOPC_S2OPC_Config_Clear(SOPC_S2OPC_Config* config);

#endif // SOPC_USER_APP_ITF_H_
