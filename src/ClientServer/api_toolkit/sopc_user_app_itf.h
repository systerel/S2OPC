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
#include "sopc_event.h"
#include "sopc_event_manager.h"
#include "sopc_key_cert_pair.h"
#include "sopc_key_manager.h"
#include "sopc_pki_decl.h"
#include "sopc_service_call_context.h"
#include "sopc_toolkit_config_constants.h"
#include "sopc_types.h"
#include "sopc_user_manager.h"

/**
 * \brief OPC UA client configuration type
 */
typedef struct SOPC_Client_Config SOPC_Client_Config;

/**
 *  \brief Client configuration of a Secure Channel
 */
typedef struct SOPC_SecureChannel_Config
{
    uint8_t isClientSc; /**< Flag to indicate if this secure channel configuration is on client side.
                             It shall always be true if not created internally. */
    const SOPC_Client_Config* clientConfigPtr; /**< Pointer to the client configuration containing this secure channel.
                                                    It should be defined to provide client application information
                                                    (locales, description, etc.) for session establishment. */

    const OpcUa_GetEndpointsResponse* expectedEndpoints; /**< Response returned by prior call to GetEndpoints service
                                                             and checked to be the same during session establishment,
                                                             NULL otherwise (no verification will be done).*/
    const char* serverUri; /**< This value shall only be specified if the server is accessed through a gateway server.
                                In this case this value is the applicationUri for the underlying Server.
                                This value might be specified for reverse connection in order to be verified
                                on ReverseHello reception. */
    const char* url;       /**< The endpoint URL used for connection. It shall always be defined. */

    SOPC_CertHolder* peerAppCert;              /**< Peer application certificate:
                                                                    isClientSc => serverCertificate (configuration data)
                                                                    !isClientSc => clientCertificate (runtime data) */
    const char* reqSecuPolicyUri;              /**< Requested Security Policy URI */
    uint32_t requestedLifetime;                /**< Requested Secure channel lifetime */
    OpcUa_MessageSecurityMode msgSecurityMode; /**< Requested Security Mode */

    uintptr_t internalProtocolData; /**< Internal use only: used to store internal protocol data (set only during
                                       connecting phase) */
} SOPC_SecureChannel_Config;

#define SOPC_SECURITY_MODE_NONE_MASK 0x01
#define SOPC_SECURITY_MODE_SIGN_MASK 0x02
#define SOPC_SECURITY_MODE_SIGNANDENCRYPT_MASK 0x04
#define SOPC_SECURITY_MODE_ANY_MASK 0x07

#ifndef SOPC_MAX_SECU_POLICIES_CFG
#define SOPC_MAX_SECU_POLICIES_CFG 5 /* Maximum number of security policies in a configuration array */
#endif

/* Maximum number of client secure connections */
#ifndef SOPC_MAX_CLIENT_SECURE_CONNECTIONS_CONFIG
#define SOPC_MAX_CLIENT_SECURE_CONNECTIONS_CONFIG SOPC_MAX_SECURE_CONNECTIONS
#else
#if SOPC_MAX_CLIENT_SECURE_CONNECTIONS_CONFIG > UINT16_MAX
#error "Maximum number of secure connections configuration cannot be > UINT16_MAX"
#endif
#endif

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
                                                        * it is possible to use the predefined policies provided:
                                                        * ::SOPC_UserTokenPolicy_Anonymous,
                                                        * ::SOPC_UserTokenPolicy_UserName_DefaultSecurityPolicy or
                                                        * ::SOPC_UserTokenPolicy_UserName_NoneSecurityPolicy supported
                                                        */
} SOPC_SecurityPolicy;

/**
 * \brief OPC UA server configuration type
 */
typedef struct SOPC_Server_Config SOPC_Server_Config;

/**
 * \brief OPC UA server client to reverse connect configuration type.
 *        From specification part 6 (v1.05.01):
 *        "For each Client, the administrator shall provide
 *         an ApplicationUri and an EndpointUrl for the Client."
 *
 * \note There is no indication to validate the ApplicationUri in specification.
 *       It might be checked in the future using the CreateSessionRequest content .
 */
typedef struct SOPC_Server_ClientToConnect
{
    char* clientApplicationURI; /**< The client application URI.
                                     It might be empty since it is not checked for now. */
    char* clientEndpointURL;    /**< The client endpoint URL to connect to establish the reverse connection. */
} SOPC_Server_ClientToConnect;

/**
 * \brief Server configuration of a Endpoint connection listener
 */
typedef struct SOPC_Endpoint_Config
{
    SOPC_Server_Config* serverConfigPtr; /**< Pointer to the server configuration containing this endpoint */
    char* endpointURL;                   /**< Endpoint URL: opc.tcp://IP-HOSTNAME:PORT(/NAME)*/
    bool hasDiscoveryEndpoint; /**< Implicit discovery endpoint with same endpoint URL is added if necessary when set */
    uint8_t nbSecuConfigs;     /**< Number of security configuration (<= SOPC_MAX_SECU_POLICIES_CFG) */
    SOPC_SecurityPolicy
        secuConfigurations[SOPC_MAX_SECU_POLICIES_CFG]; /**< Security policies defined for the current endpoint URL.
                                                         * An implicit discovery endpoint will be defined with this URL
                                                         * if SecurityPolicy None is not present. Otherwise the
                                                         * discovery endpoint and session endpoint are the same for this
                                                         * endpoint URL.
                                                         */
    /* Configure reverse connection mechanism */
    bool noListening;            /**< If Flag is set, the server does not listen connection initiated by clients */
    uint16_t nbClientsToConnect; /**< Number of clients to connect using reverse connection mechanism */
    SOPC_Server_ClientToConnect
        clientsToConnect[SOPC_MAX_REVERSE_CLIENT_CONNECTIONS]; /**< Array of configuration for reverse connection
                                                                     to clients */
} SOPC_Endpoint_Config;

/**
 * \brief Client temporary configuration structure used to store user X509 configuration data from paths.
 *        Those paths are should be used to load the serialized version for the certificate / key.
 */
typedef struct SOPC_Session_UserX509_ConfigFromPaths
{
    char* userCertPath; /**< Temporary path to the user certificate (certX509 shall be instantiated by applicative code)
                         */
    char* userKeyPath;  /**< Temporary path to the user key (keyX509 shall be instantiated by applicative code) */
    bool userKeyEncrypted; /**< Boolean to indicate if the private key is encrypted */
} SOPC_Session_UserX509_ConfigFromPaths;

/**
 * \brief Client configuration structure used to store session activation data for an X509 user token type.
 */
typedef struct SOPC_Session_UserX509
{
    SOPC_SerializedCertificate* certX509;
    SOPC_SerializedAsymmetricKey* keyX509;

    bool isConfigFromPathNeeded; /**< True if the following field shall be treated to configure the X509 user for
                                    session */
    SOPC_Session_UserX509_ConfigFromPaths* configFromPaths; /**< The paths configuration to use for user certificate and
                                   key if if isConfigFromPathsNeeded is true.  NULL otherwise.
                                   (used to configure certX509 and keyX509) */
} SOPC_Session_UserX509;

/**
 * \brief Client configuration structure used to store session activation data for a user name token type.
 */
typedef struct SOPC_Session_UserName
{
    char* userName;
    char* userPwd;
} SOPC_Session_UserName;

/**
 * \brief Client configuration structure used to store session activation configuration data.
 */
typedef struct SOPC_Session_Config
{
    const char* userPolicyId;
    OpcUa_UserTokenType userTokenType;
    union
    {
        SOPC_Session_UserName userName;
        SOPC_Session_UserX509 userX509;
    } userToken;

} SOPC_Session_Config;

/**
 * \brief Structure representing a secure connection configuration (secure channel + session) which allow to establish a
 * connection to a server
 */
typedef struct SOPC_SecureConnection_Config
{
    const char* userDefinedId; // Optional user defined id

    SOPC_SecureChannel_Config scConfig;
    const char* reverseURL;

    bool isServerCertFromPathNeeded; /**< True if the following field shall be treated to configure the client */
    char* serverCertPath; /**< Path to the server certificate if isServerCertFromPathNeeded true, NULL otherwise
                               (scConfig.peerAppCert shall be instantiated by applicative code) */
    SOPC_KeyCertPairUpdateCb* serverCertUpdateCb; /**< This callback shall be set in case the server certificate held
                                                     by scConfig.peerAppCert needs to be updated at runtime.*/
    uintptr_t serverCertUpdateParam;              /**< The parameter to provide to serverCertUpdateCb callback*/

    SOPC_Session_Config sessionConfig; /**< Session activation data */

    uint16_t secureConnectionIdx;      /**< Index into ::SOPC_Client_Config secureConnections array */
    uint32_t reverseEndpointConfigIdx; /**< (Optional) Index of the Reverse Endpoint configuration to listen for server
                                          connection returned by ::SOPC_ToolkitClient_AddReverseEndpointConfig(). If not
                                          applicable it shall be 0. */
    uint32_t secureChannelConfigIdx; /**< Index of the Secure Channel configuration for endpoint connection returned by
                                        ::SOPC_ToolkitClient_AddSecureChannelConfig(). It shall not be 0. */
    bool finalized; /** < Set when the configuration of the secure connection is frozen and configuration from paths
                          has been done. */
} SOPC_SecureConnection_Config;

/**
 * \brief Client temporary configuration structure used to store client certificate / key / PKI configuration data from
 * paths. Those paths are should be used to load the serialized version for the certificate / key and instantiate PKI.
 */
typedef struct SOPC_Client_ConfigFromPaths
{
    char* clientCertPath;    /**< Temporary path to the client certificate (clientCertificate shall be instantiated by
                                applicative code) */
    char* clientKeyPath;     /**< Temporary path to the client key (key_priv_cli shall be instantiated by applicative
                                code) */
    bool clientKeyEncrypted; /**< Boolean to indicate if the private key is encrypted */
    char* clientPkiPath;     /**< Temporary path to the client public key infrastructure */
} SOPC_Client_ConfigFromPaths;

/**
 * \brief OPC UA client configuration structure
 */
struct SOPC_Client_Config
{
    OpcUa_ApplicationDescription
        clientDescription; /**< Application description of the client.
                            *   \warning: The ApplicationURI is automatically extracted from certificate when the client
                            *             certificate is used. If no certificate used or URI extraction failed,
                            *             the ApplicationURI of clientDescription is used.
                            */

    bool freeCstringsFlag;  /**< A flag to indicate if the following C strings contained in the client configuration
                                 shall be freed */
    char** clientLocaleIds; /**< An array of locale ids preferred by the client terminated by a NULL pointer.
                                 It might be NULL if there are no preferred locale ids.
                                 The array of locale ids indicates priority order for localized strings.
                                 The first LocaleId in the array has the highest priority. */

    bool isConfigFromPathsNeeded; /**< True if the following field shall be treated to configure the client */
    SOPC_Client_ConfigFromPaths* configFromPaths; /**< The paths configuration to use for PKI and client certificate and
                                                     key if if isConfigFromPathsNeeded is true. NULL otherwise.
                                                     (used to configure clientCertificate, clientKey and clientPKI) */

    SOPC_KeyCertPair* clientKeyCertPair; /**< Key and certificate might be set from paths or bytes arrays */
    SOPC_PKIProvider* clientPKI;         /**< PKI might be set from paths or bytes arrays */

    uint16_t nbSecureConnections; /**< Number of secure connections defined by the client */
    SOPC_SecureConnection_Config*
        secureConnections[SOPC_MAX_CLIENT_SECURE_CONNECTIONS_CONFIG]; /**< Secure connection configuration array.
                                                                           Indexes [0;nbSecureConnections[ shall contain
                                                                           non-null configuration. */
    uint16_t nbReverseEndpointURLs;
    char* reverseEndpointURLs[SOPC_MAX_CLIENT_SECURE_CONNECTIONS_CONFIG]; /**< Reverse endpoint URLs array. Maximum 1
                                                                             per secure connection config. */
};

/**
 * \brief Type of the callback called by CreateMonitoredItem service when a NodeId is not already part of server
 *        address space, the callback result indicates if it shall be considered known by server
 *        (and might exist later using AddNode service).
 *
 *        It returns true when CreateMonitoredItem for this \p nodeId shall succeed,
 *        in this case \p outNodeClass shall be set to the expected NodeClass
 *        and \p outBadStatus shall be set with an appropriate Bad StatusCode
 *        returned in the Publish response as first value notification.
 *        It returns false otherwise, in this case server will return Bad_NodeIdUnknown
 *        in the CreateMonitoredItem response.
 *
 * \warning This callback shall not block the thread that calls it, and shall return immediately.
 *
 *
 * \param      nodeId                   NodeId that is not part of the server address space yet
 *                                      and which is requested in a MonitoredItemCreateRequest.
 *                                      It might be added by AddNode service later.
 * \param[out] outNodeClass             The NodeClass of the known node when it will be available.
 *                                      It shall always be the same for the same NodeId.
 * \param[out] outUnavailabilityStatus  The appropriate Bad StatusCode to return in the Publish response.
 *                                      ::OpcUa_BadDataUnavailable or ::OpcUa_BadWouldBlock are recommended.
 *
 * \return                              true when CreateMonitoredItem for this \p nodeId shall succeed, false otherwise.
 */
typedef bool SOPC_CreateMI_NodeAvailFunc(const SOPC_NodeId* nodeId,
                                         OpcUa_NodeClass* outNodeClass,
                                         SOPC_StatusCode* outUnavailabilityStatus);

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
    bool serverKeyEncrypted;         /**< Boolean to indicate if the private key is encrypted */
    char* serverPkiPath;             /**< Temporary path to the server public key infrastructure */
    uint8_t nbEndpoints;             /**< Number of endpoints defined by the server */
    SOPC_Endpoint_Config* endpoints; /**< Endpoint configuration array */

    /* To be instantiated by applicative code: */
    SOPC_KeyCertPair* serverKeyCertPair;        /**< Server key and certificate to be instantiated from path or bytes */
    SOPC_PKIProvider* pki;                      /**< PKI provider to be instantiated. Possible use of
                                                   ::SOPC_PKIProvider_CreateFromStore or ::SOPC_PKIProvider_CreateFromList. */
    SOPC_MethodCallManager* mcm;                /**< Method Call service configuration.
                                                     Can be instantiated with SOPC_MethodCallManager_Create()
                                                     or specific code by applicative code.
                                                     Can be NULL if Method Call service is not used. */
    SOPC_CreateMI_NodeAvailFunc* nodeAvailFunc; /**< If defined, the callback is called by CreateMonitoredItem service
                                                     when NodeId is not already part of server AddressSpace.
                                                     The callback indicates if it should be considered known by server
                                                     (and might exist later).
                                                     See ::SOPC_CreateMI_NodeAvailFunc for details. */
    SOPC_UserAuthentication_Manager*
        authenticationManager; /**< The user authentication manager: user authentication on session activation */
    SOPC_UserAuthorization_Manager*
        authorizationManager; /**< The user authorization manager: user access level evaluation */

    SOPC_Server_Event_Types* eventTypes; /**< The server events types configuration based on address space content
                                            (S2OPC_NODE_MANAGEMENT needed to be set) */
};

/**
 * \brief S2OPC configuration
 */
typedef struct SOPC_S2OPC_Config
{
    SOPC_Server_Config serverConfig; /**< server configuration */
    SOPC_Client_Config clientConfig; /**< client configuration */
} SOPC_S2OPC_Config;

/**
 * \brief Session identifier type, instances are generated by SE_ACTIVATED_SESSION
 */
typedef uint32_t SOPC_SessionId;

/**
 *  \brief Client and Server communication events to be managed by applicative code
 */
typedef enum _SOPC_App_Com_Event
{
    /* Client application events */
    SE_REVERSE_ENDPOINT_CLOSED = 0x700, /**< Client side only:<br/>
                                         *  Notifies the reverse connection is closed<br/>
                                         *  id = reverse endpoint configuration index<br/>
                                         *  auxParams = (SOPC_ReturnStatus) status reason
                                         */
    SE_SESSION_ACTIVATION_FAILURE,      /**< Client side only:<br/>
                                         *   Notifies the session activation (or re-activation) failed<br/>
                                         *   id = (SOPC_SessionId) internal session id (or 0 if not yet defined)<br/>
                                         *   params = (SOPC_StatusCode)(uintptr_t) status code reason<br/>
                                         *   auxParam = user application session context
                                         */
    SE_ACTIVATED_SESSION,               /**< Client side only:<br/>
                                         *   Notifies the session is active (newly created session or re-activated session)<br/>
                                         *   id = (SOPC_SessionId) internal session id<br/>
                                         *   auxParam = user application session context
                                         */
    SE_SESSION_REACTIVATING,            /**< Client side only:<br/>
                                         *  Notifies the session is currently re-activating due to connection issue.<br/>
                                         *   id = (SOPC_SessionId) internal session id<br/>
                                         *   auxParam = user application session context
                                         */
    SE_RCV_SESSION_RESPONSE,            /**< Client side only:<br/>
                                         *   Notifies the service response to a previously (non-discovery) service request sent
                                         * over a session<br/>
                                         * id = (SOPC_SessionId) internal session id<br/>
                                         * params =(OpcUa_<MessageStruct>*) OPC UA message header + payload structure<br/>
                                         * (deallocated by toolkit after callback call ends)<br/>
                                         * auxParam = user application request context
                                         */
    SE_CLOSED_SESSION,                  /**< Client side only:<br/>
                                         *   Notifies the session is closed after requesting a session close.<br/>
                                         *   id = (SOPC_SessionId) internal session id<br/>
                                         *   params = (SOPC_StatusCode)(uintptr_t) status code reason<br/>
                                         *   auxParam = user application session context
                                         */
    SE_RCV_DISCOVERY_RESPONSE,          /**< Client side only:<br/>
                                         *   Notifies the service response to a previously discovery service request sent<br/>
                                         *   params = (OpcUa_<MessageStruct>*) OPC UA discovery message header + payload<br/>
                                         *   structure (deallocated by toolkit after callback call ends) <br/>
                                         *   auxParam = user application request context
                                         */
    SE_SND_REQUEST_FAILED,              /**< Client side only:<br/>
                                         *   Notifies the service request sending failed (connection issue or timeout for response to
                                         * be received)<br/>
                                         * idOrStatus = (SOPC_ReturnStatus) status,<br/>
                                         * params = (SOPC_EncodeableType*) request type (shall not be deallocated)<br/>
                                         * auxParam = user application request context
                                         */

    /* Server application events */
    SE_CLOSED_ENDPOINT,       /**< Server side only:<br/>
                               *   Notifies the endpoint listening for connections is closed or failed to be opened.<br/>
                               *   id = endpoint configuration index,<br/>
                               *   auxParam = SOPC_ReturnStatus
                               */
    SE_LOCAL_SERVICE_RESPONSE /**< Server side only:<br/>
                               *   Notifies the service response for the local service request previously
                               * requested.<br/>
                               * id = endpoint configuration index,<br/>
                               * params =(OpcUa_<MessageStruct>*) OPC UA message header + payload structure<br/>
                               * (deallocated by toolkit after callback call ends)<br/>
                               * auxParam = user application request context
                               */
} SOPC_App_Com_Event;

/* Server only interfaces */

/**
 * \brief Server address space access/modification notifications to applicative code
 */
typedef enum _SOPC_App_AddSpace_Event
{
    /* Server application events */
    AS_WRITE_EVENT = 0x800, /**< Server side only:<BR/>
                             *   Notifies a write operation on the server address space.<br/>
                             *   opParam = (OpcUa_WriteValue*) single write value operation<br/>
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
typedef void SOPC_AddressSpaceNotif_Fct(const SOPC_CallContext* callCtxPtr,
                                        SOPC_App_AddSpace_Event event,
                                        void* opParam,
                                        SOPC_StatusCode opStatus);

/**
 * \brief Toolkit build information
 */
typedef struct
{
    SOPC_Build_Info commonBuildInfo;
    SOPC_Build_Info clientServerBuildInfo;
} SOPC_Toolkit_Build_Info;

/**
 * \brief Initializes the content of the SOPC_S2OPC_Config.
 *        Calls both ::SOPC_ServerConfig_Initialize and ::SOPC_ClientConfig_Initialize
 *
 * \param config  The s2opc client/server configuration to initialize
 */
void SOPC_S2OPC_Config_Initialize(SOPC_S2OPC_Config* config);

/**
 * \brief Clears the content of the SOPC_S2OPC_Config
 *        Calls both ::SOPC_ServerConfig_Clear and ::SOPC_ClientConfig_Clear
 *
 * \param config  The s2opc client/server configuration to clear
 */
void SOPC_S2OPC_Config_Clear(SOPC_S2OPC_Config* config);

/**
 * \brief Initializes the content of the SOPC_Server_Config
 *
 * \param config  The s2opc server configuration to initialize
 */
void SOPC_ServerConfig_Initialize(SOPC_Server_Config* config);

/**
 * \brief Clears the content of the SOPC_Server_Config
 *
 * \param config  The s2opc server configuration to clear
 */
void SOPC_ServerConfig_Clear(SOPC_Server_Config* config);

/**
 * \brief Initializes the content of the SOPC_Client_Config
 *
 * \param config  The s2opc client configuration to initialize
 */
void SOPC_ClientConfig_Initialize(SOPC_Client_Config* config);

/**
 * \brief Clears the content of the SOPC_Client_Config
 *
 * \param config  The s2opc client configuration to clear
 */
void SOPC_ClientConfig_Clear(SOPC_Client_Config* config);

#endif // SOPC_USER_APP_ITF_H_
