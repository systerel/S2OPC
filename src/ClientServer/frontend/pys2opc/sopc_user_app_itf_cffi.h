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

/**
 * This file is an excerpt from sopc_user_app_itf.h.
 * It should not be included in a generic project.
 * See s2opc_headers.h
 */

//#include "sopc_call_method_manager.h"
#include "sopc_common_build_info.h"
//#include "sopc_crypto_profiles.h"
#include "sopc_crypto_decl.h"
//#include "sopc_key_manager.h"
//#include "sopc_types.h"
//#include "sopc_user_manager.h"

typedef struct SOPC_Client_Config SOPC_Client_Config;

typedef struct SOPC_SecureChannel_Config
{
    uint8_t isClientSc;
    const SOPC_Client_Config*
        clientConfigPtr; /**< Pointer to the client configuration containing this secure channel. */

    const OpcUa_GetEndpointsResponse* expectedEndpoints; /**< Response returned by prior call to GetEndpoints service
                                                             and checked to be the same during session establishment,
                                                             NULL otherwise (no verification will be done).*/
    const char* serverUri; /**< This value shall only be specified if the server is accessed through a gateway server.
                                In this case this value is the applicationUri for the underlying Server. */
    const char* url;       /**< The endpoint URL used for connection. */
    const SOPC_SerializedCertificate* crt_cli;
    const SOPC_SerializedAsymmetricKey* key_priv_cli;
    const SOPC_SerializedCertificate* crt_srv;
    const SOPC_PKIProvider*
        pki; /**< PKI shall not be shared between several configurations except if it is thread-safe */
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

/* Maximum number of security policies in a configuration array */
#ifndef SOPC_MAX_SECU_POLICIES_CFG
#define SOPC_MAX_SECU_POLICIES_CFG 5
#endif

/* Maximum number of configured reverse connection from a server endpoint to clients */
#ifndef SOPC_MAX_REVERSE_CLIENT_CONNECTIONS
#define SOPC_MAX_REVERSE_CLIENT_CONNECTIONS 5
#endif

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

typedef struct SOPC_Server_Config SOPC_Server_Config;

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

    /* To be instantiated by applicative code: */
    SOPC_UserAuthentication_Manager*
        authenticationManager; /**< The user authentication manager: user authentication on session activation */
    SOPC_UserAuthorization_Manager*
        authorizationManager; /**< The user authorization manager: user access level evaluation */

    /* Configure reverse connection mechanism */
    bool noListening;            /**< If Flag is set, the server does not listen connection initiated by clients */
    uint16_t nbClientsToConnect; /**< Number of clients to connect using reverse connection mechanism */
    SOPC_Server_ClientToConnect
        clientsToConnect[SOPC_MAX_REVERSE_CLIENT_CONNECTIONS]; /**< Array of configuration for reverse connection
                                                                     to clients */
} SOPC_Endpoint_Config;

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
 * \param      nodeId       NodeId that is not part of the server address space yet
 *                          and which is requested in a MonitoredItemCreateRequest.
 *                          It might be added by AddNode service later.
 * \param[out] outNodeClass The NodeClass of the known node when it will be available.
 *                          It shall always be the same for the same NodeId.
 * \param[out] outBadStatus The appropriate Bad StatusCode to return in the Publish response.
 *                          OpcUa_UnavailableData or OpcUa_WouldBlock are recommended.
 *
 * \return                  true when CreateMonitoredItem for this \p nodeId shall succeed, false otherwise.
 */
typedef bool(SOPC_CreateMI_NodeAvailFunc)(const SOPC_NodeId* nodeId,
                                          OpcUa_NodeClass* outNodeClass,
                                          SOPC_StatusCode* outUnavailabilityStatus);

/* OPC UA server configuration structure */
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
    SOPC_SerializedCertificate* serverCertificate;
    SOPC_SerializedAsymmetricKey* serverKey;
    SOPC_PKIProvider* pki;
    SOPC_MethodCallManager* mcm;                /**< Method Call service configuration.
                                                     Can be instantiated with SOPC_MethodCallManager_Create()
                                                     or specific code by applicative code.
                                                     Can be NULL if Method Call service is not used. */
    SOPC_CreateMI_NodeAvailFunc* nodeAvailFunc; /**< If defined, the callback is called by CreateMonitoredItem service
                                                     when NodeId is not already part of server AddressSpace.
                                                     The callback indicates if  it should be considered known by server
                                                     (and might exist later).
                                                     See ::SOPC_CreateMonitoredItem_NodeAvailability for details. */
};

/* S2OPC server configuration */
typedef struct SOPC_S2OPC_Config
{
    SOPC_Server_Config serverConfig;
    SOPC_Client_Config clientConfig;
} SOPC_S2OPC_Config;

/* Client and Server communication events to be managed by applicative code*/
typedef enum SOPC_App_Com_Event
{
    /* Client application events */
    SE_SESSION_ACTIVATION_FAILURE = 0x700, /* id = internal session id (or 0 if not yet defined)
                                      params = (SOPC_StatusCode)(uintptr_t) status code reason
                                      auxParam = user application session context
                                   */
    SE_ACTIVATED_SESSION,                  /* id = internal session id
                                              auxParam = user application session context
                                           */
    SE_SESSION_REACTIVATING,               /* automatic new SC or manual new user on same SC */
                                           /* id = internal session id
                                              auxParam = user application session context
                                           */
    SE_RCV_SESSION_RESPONSE,               /* id = internal session id
                                              params = (OpcUa_<MessageStruct>*) OPC UA message header + payload structure
                                              (deallocated by toolkit after callback call ends)
                                              auxParam = user application request context
                                           */
    SE_CLOSED_SESSION,                     /* id = internal session id
                                              params = (SOPC_StatusCode)(uintptr_t) status code reason
                                              auxParam = user application session context
                                            */
    SE_RCV_DISCOVERY_RESPONSE, /* params = (OpcUa_<MessageStruct>*) OPC UA discovery message header + payload structure
                                  (deallocated by toolkit after callback call ends)
                                  auxParam = user application request context
                                */

    SE_SND_REQUEST_FAILED, /* idOrStatus = (SOPC_ReturnStatus) status,
                              params = (SOPC_EncodeableType*) request type (shall not be deallocated)
                              auxParam = user application request context
                            */

    /* Server application events */
    SE_CLOSED_ENDPOINT,       /* id = endpoint configuration index,
                                 auxParam = SOPC_ReturnStatus
                              */
    SE_LOCAL_SERVICE_RESPONSE /* id = endpoint configuration index,
                                 params = (OpcUa_<MessageStruct>*) OPC UA message header + payload structure
                                 (deallocated by toolkit after callback call ends)
                                 auxParam = user application request context
                               */
} SOPC_App_Com_Event;

/* Server address space access/modification notifications to applicative code */
typedef enum SOPC_App_AddSpace_Event
{
    /* Server application events */
    AS_WRITE_EVENT = 0x800, /* opParam = (OpcUa_WriteValue*) single write value operation
                       opStatus = status of the write operation
                     */
} SOPC_App_AddSpace_Event;

/**
 * \brief Toolkit communication events application callback type
 */
typedef void SOPC_ComEvent_Fct(SOPC_App_Com_Event event, uint32_t IdOrStatus, void* param, uintptr_t appContext);

/**
 * \brief Toolkit context provided when a callback is called, see getters available
 */
typedef struct SOPC_CallContext SOPC_CallContext;

/* The context object and properties can only be used during the callback call */
const SOPC_User* SOPC_CallContext_GetUser(const SOPC_CallContext* callContextPtr); // only valid for server event
OpcUa_MessageSecurityMode SOPC_CallContext_GetSecurityMode(const SOPC_CallContext* callContextPtr);
const char* SOPC_CallContext_GetSecurityPolicy(const SOPC_CallContext* callContextPtr);
uint32_t SOPC_CallContext_GetEndpointConfigIdx(const SOPC_CallContext* callContextPtr);

/**
 * \brief Toolkit address space notification events callback type
 */
typedef void SOPC_AddressSpaceNotif_Fct(const SOPC_CallContext* callCtxPtr,
                                        SOPC_App_AddSpace_Event event,
                                        void* opParam,
                                        SOPC_StatusCode opStatus);

/* Toolkit build information */
typedef struct
{
    SOPC_Build_Info commonBuildInfo;
    SOPC_Build_Info clientServerBuildInfo;
} SOPC_Toolkit_Build_Info;

void SOPC_S2OPC_Config_Initialize(SOPC_S2OPC_Config* config);
void SOPC_S2OPC_Config_Clear(SOPC_S2OPC_Config* config);
