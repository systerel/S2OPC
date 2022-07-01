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
 * \brief Interface of an example client library supporting the subscription management.
 *
 * The functions of this interface are threadsafe, except stated otherwise.
 */

#ifndef LIBS2OPC_CLIENT_H_
#define LIBS2OPC_CLIENT_H_
/*
  Notes:
  This header is designed so as to make it possible to generate automatically an Ada specification package (.ads)
    for any application that would have to link the staticC  library S2OPC_CLIENT.
  The Generation of .ADS file can be done using the following command:
     g++ -c -std=gnu++11 -fdump-ada-spec -C libs2opc_client.h
  gcc also works but loses parameter names in function prototypes, that is why the g++ and 'extern "C"' are used.
*/

#include <stdint.h>

/*
 =================
 TYPES DEFINITION
 ================= */

/* S2OPC types: define them except when SKIP_S2OPC_DEFINITIONS is defined.
 * To use the LibSub with other parts of the S2OPC Toolkit, you shall define SKIP_S2OPC_DEFINITIONS
 * and includes the following files before including this header:
 *  sopc_builtintypes.h
 *  sopc_user_app_itf.h
 */
#ifndef SKIP_S2OPC_DEFINITIONS

/*
  Data value quality
  TBD? Masks for BAD and UNCERTAIN quality?
  Taken from sop "sopc_builtintypes.h" */
typedef uint32_t SOPC_StatusCode;

/* Result, taken from "sopc_enums.h" */
typedef enum SOPC_ReturnStatus
{
    SOPC_STATUS_OK = 0,
    SOPC_STATUS_NOK = 1,
    SOPC_STATUS_INVALID_PARAMETERS = 2,
    SOPC_STATUS_INVALID_STATE = 3,
    SOPC_STATUS_ENCODING_ERROR = 4,
    SOPC_STATUS_WOULD_BLOCK = 5,
    SOPC_STATUS_TIMEOUT = 6,
    SOPC_STATUS_OUT_OF_MEMORY = 7,
    SOPC_STATUS_CLOSED = 8,
    SOPC_STATUS_NOT_SUPPORTED = 9
} SOPC_ReturnStatus;

/* Log levels, taken from "sopc_user_app_itf.h" */
typedef enum
{
    SOPC_LOG_LEVEL_ERROR = 0,
    SOPC_LOG_LEVEL_WARNING = 1,
    SOPC_LOG_LEVEL_INFO = 2,
    SOPC_LOG_LEVEL_DEBUG = 3
} SOPC_Log_Level;

/* SecurityMode, directly compatible with the encoded OPC-UA type,
 * taken from "sopc_types.h" */
typedef enum
{
    OpcUa_MessageSecurityMode_Invalid = 0,
    OpcUa_MessageSecurityMode_None = 1,
    OpcUa_MessageSecurityMode_Sign = 2,
    OpcUa_MessageSecurityMode_SignAndEncrypt = 3
} OpcUa_MessageSecurityMode;

/* Security policies, taken from "sopc_crypto_profiles.h" */
#define SOPC_SecurityPolicy_None_URI "http://opcfoundation.org/UA/SecurityPolicy#None"
#define SOPC_SecurityPolicy_Basic256_URI "http://opcfoundation.org/UA/SecurityPolicy#Basic256"
#define SOPC_SecurityPolicy_Basic256Sha256_URI "http://opcfoundation.org/UA/SecurityPolicy#Basic256Sha256"
#define SOPC_SecurityPolicy_Aes128Sha256RsaOaep_URI "http://opcfoundation.org/UA/SecurityPolicy#Aes128_Sha256_RsaOaep"
#define SOPC_SecurityPolicy_Aes256Sha256RsaPss_URI "http://opcfoundation.org/UA/SecurityPolicy#Aes256_Sha256_RsaPss"

typedef enum
{
    SOPC_USER_POLICY_ID_ANONYMOUS = 0,
    SOPC_USER_POLICY_ID_USERNAME = 1
} SOPC_UserPolicyId;

void SOPC_Sleep(unsigned int milliseconds);
#endif

/* C String type */
typedef char* SOPC_LibSub_String;

/* C Const String type */
typedef const char* SOPC_LibSub_CstString;

/* Connnection configuration identifier */
typedef uint32_t SOPC_LibSub_ConfigurationId;

/* Connection identifier */
typedef uint32_t SOPC_LibSub_ConnectionId;

/* Data identifier (used for subscription change notification) */
typedef uint32_t SOPC_LibSub_DataId;

/* Timestamp (NTP Format) */
typedef uint64_t SOPC_LibSub_Timestamp;

/* Data value type */
typedef enum
{
    SOPC_LibSub_DataType_bool = 1,
    SOPC_LibSub_DataType_integer = 2,
    SOPC_LibSub_DataType_string = 3,
    SOPC_LibSub_DataType_bytestring = 4,
    SOPC_LibSub_DataType_other = 5
} SOPC_LibSub_DataType;

/**
  @struct SOPC_LibSub_Value
  @brief
    Structure defining the value of a node
  @var SOPC_LibSub_Value::quality
    The value quality.
  @var SOPC_LibSub_Value::type
    The value type. Specifies the type of '*value' amongst:
    - SOPC_LibSub_CstString (SOPC_LibSub_DataType_string / SOPC_LibSub_DataType_bytestring)
    - int64_t (SOPC_LibSub_DataType_bool / SOPC_LibSub_DataType_integer)
    - NULL (SOPC_LibSub_DataType_other)
  @var SOPC_LibSub_Value::length
    The length of the value, in case it is a bytestring. 0 otherwise.
  @var SOPC_LibSub_Value::raw_value
    A pointer to a copy of the SOPC_Variant.
*/
typedef struct
{
    SOPC_LibSub_DataType type;
    SOPC_StatusCode quality;
    void* value;
    int32_t length;
    SOPC_LibSub_Timestamp source_timestamp;
    SOPC_LibSub_Timestamp server_timestamp;
    void* raw_value;
} SOPC_LibSub_Value;

/**
  @brief
    AttributeIds, as defined in the OPC UA Reference, Part 6 Annex A */
typedef uint32_t SOPC_LibSub_AttributeId;

#define SOPC_LibSub_AttributeId_NodeId 1
#define SOPC_LibSub_AttributeId_NodeClass 2
#define SOPC_LibSub_AttributeId_BrowseName 3
#define SOPC_LibSub_AttributeId_DisplayName 4
#define SOPC_LibSub_AttributeId_Description 5
#define SOPC_LibSub_AttributeId_WriteMask 6
#define SOPC_LibSub_AttributeId_UserWriteMask 7
#define SOPC_LibSub_AttributeId_IsAbstract 8
#define SOPC_LibSub_AttributeId_Symmetric 9
#define SOPC_LibSub_AttributeId_InverseName 10
#define SOPC_LibSub_AttributeId_ContainsNoLoops 11
#define SOPC_LibSub_AttributeId_EventNotifier 12
#define SOPC_LibSub_AttributeId_Value 13
#define SOPC_LibSub_AttributeId_DataType 14
#define SOPC_LibSub_AttributeId_ValueRank 15
#define SOPC_LibSub_AttributeId_ArrayDimensions 16
#define SOPC_LibSub_AttributeId_AccessLevel 17
#define SOPC_LibSub_AttributeId_UserAccessLevel 18
#define SOPC_LibSub_AttributeId_MinimumSamplingInterval 19
#define SOPC_LibSub_AttributeId_Historizing 20
#define SOPC_LibSub_AttributeId_Executable 21
#define SOPC_LibSub_AttributeId_UserExecutable 22

/**
  @brief
    The event passed to the connection SOPC_LibSub_EventCbk.
    Either an error or a valid response notification.
*/
typedef enum SOPC_LibSub_ApplicativeEvent
{
    SOPC_LibSub_ApplicativeEvent_SendFailed,
    SOPC_LibSub_ApplicativeEvent_Response
} SOPC_LibSub_ApplicativeEvent;

/**
  @brief
    Log callback type
  @param log_level
    The Log level (SOPC_Log_Level). Note: SOPC_log_error shall be non-returning.
  @param text
    The text string to log (shall not be null) */
typedef void SOPC_LibSub_LogCbk(const SOPC_Log_Level log_level, SOPC_LibSub_CstString text);

/**
  @brief
    Callback type for disconnect event
  @param c_id
    The connection id that has been disconnected */
typedef void SOPC_LibSub_DisconnectCbk(const SOPC_LibSub_ConnectionId c_id);

/**
  @brief
    Callback type for data change event (related to a subscription)
  @param c_id
    The connection id on which the datachange happened
  @param d_id
    The data id of the monitored item (see SOPC_LibSub_AddToSubscription())
  @param value
    The new value. Its content is freed by the LibSub after this function has been called,
    hence the callback must copy it if it should be used outside the callback.
    The NULL pointer is given to the callback when the SOPC_DataValue could not be converted
    to a SOPC_LibSub_Value, or the malloc failed. */
typedef void SOPC_LibSub_DataChangeCbk(const SOPC_LibSub_ConnectionId c_id,
                                       const SOPC_LibSub_DataId d_id,
                                       const SOPC_LibSub_Value* value);

/**
  @brief
    Callback for generic responses to a call to SOPC_LibSub_AsyncSendRequestOnSession().
  @param c_id
    The connection id on which the event happened
  @param event
    The type of the event:
    - SOPC_LibSub_ApplicativeEvent_SendFailed: the request was not sent and \p status holds the reason.
      In this case, response shall be NULL but the responseContext is valid.
      The underlying connection is about to be closed.
    - SOPC_LibSub_ApplicativeEvent_Response: the response pointer and the response context are valid
      and the \p status is Good.
  @param status
    The status code for the event
  @param response
    An (OpcUa_<MessageStruct>*) pointing to the OPC-UA response structure.
    This message is freed by the caller and should not be modified by the callback function.
  @param responseContext
    The requestContext given in SOPC_LibSub_AsyncSendRequestOnSession().
*/
typedef void SOPC_LibSub_EventCbk(SOPC_LibSub_ConnectionId c_id,
                                  SOPC_LibSub_ApplicativeEvent event,
                                  SOPC_StatusCode status,
                                  const void* response,
                                  uintptr_t responseContext);
// TODO: the const constraint on response should be released since it is not necessary and by-passed

/*
 =====================
 STRUCTURES DEFINITION
 ===================== */

/**
 @struct SOPC_LibSub_StaticCfg
 @brief
   Static configuration of OPC client library
 @var SOPC_LibSub_StaticCfg::host_log_callback
   Host log callback
 @var SOPC_LibSub_StaticCfg::disconnect_callback
   Notification event for disconnection from server */
typedef struct
{
    SOPC_LibSub_LogCbk* host_log_callback;
    SOPC_LibSub_DisconnectCbk* disconnect_callback;
    struct
    {
        SOPC_Log_Level level;
        const char* log_path;
        uint32_t maxBytes;
        uint16_t maxFiles;
    } toolkit_logger;
} SOPC_LibSub_StaticCfg;

/**
 @struct SOPC_LibSub_ConnectionCfg
 @brief
   Connection configuration to a remote OPC server
 @var SOPC_LibSub_ConnectionCfg::server_url
   Zero-terminated path to server URL
 @var SOPC_LibSub_ConnectionCfg::security_policy
   The chosen OPC-UA security policy for the connection, one of the SOPC_SecurityPolicy_*_URI string
 @var SOPC_LibSub_ConnectionCfg::security_mode
   The chosen OPC-UA security mode for the connection, one of the OpcUa_MessageSecurityMode constant
 @var SOPC_LibSub_ConnectionCfg::disable_certificate_verification
   Uses a PKIProvider which does not verify the certificates against a certificate authority.
   Setting this flag to not 0 severely harms security. The certificate authority is not required in this case.
   Other certificates are still required when using an encrypted or signed secure channel.
 @var SOPC_LibSub_ConnectionCfg::path_cert_auth
   Zero-terminated path to the root certificate authority in the DER format
 @var SOPC_LibSub_ConnectionCfg::path_cert_srv
   Zero-terminated path to the server certificate in the DER format, signed by the root certificate authority
 @var SOPC_LibSub_ConnectionCfg::path_cert_cli
   Zero-terminated path to the client certificate in the DER format, signed by the root certificate authority
 @var SOPC_LibSub_ConnectionCfg::path_key_cli
   Zero-terminated path to the client private key which is paired to the public key signed server certificate,
   in the DER format
 @var SOPC_LibSub_ConnectionCfg::path_crl
   Zero-terminated path to the certificate revocation list in the DER format
 @var SOPC_LibSub_ConnectionCfg::policyId
   Zero-terminated policy id. To know which policy id to use, please read a
   GetEndpointsResponse or a CreateSessionResponse. When username is NULL, the
   AnonymousIdentityToken is used and the policy id must correspond to an
   anonymous UserIdentityPolicy. Otherwise, the UserNameIdentityToken is used
   and the policy id must correspond to an username UserIdentityPolicy.
 @var SOPC_LibSub_ConnectionCfg::username
   Zero-terminated username, NULL for anonymous access, see policyId
 @var SOPC_LibSub_ConnectionCfg::password
   Zero-terminated password, ignored when username is NULL. Password is kept in memory for future reconnections.
 @var SOPC_LibSub_ConnectionCfg::publish_period_ms
   The requested publish period for the created subscription (in milliseconds)
 @var SOPC_LibSub_ConnectionCfg::n_max_keepalive
   The max keep alive count for the subscription. When there is no notification to send, the subscription waits
   at most the number of publish cycle before sending a publish response (which is then empty).
 @var SOPC_LibSub_ConnectionCfg::n_max_lifetime
   The max number of time that a subscription may timeout its publish cycle without being able to send a
   response (because there is no publish request to answer to). In this case, the subscription is killed by the
   server. A large value is recommended (e.g. 1000).
 @var SOPC_LibSub_ConnectionCfg::data_change_callback
   The callback for data change notification
 @var SOPC_LibSub_ConnectionCfg::timeout_ms
   Connection timeout (milliseconds)
 @var SOPC_LibSub_ConnectionCfg::sc_lifetime
   Time before secure channel renewal (milliseconds). A parameter larger than 60000 is recommended.
 @var SOPC_LibSub_ConnectionCfg::token_target
   Number of tokens (PublishRequest) that the client tries to maintain throughout the connection
 @var SOPC_LibSub_ConnectionCfg::generic_response_callback
   The callback used to transmit generic responses to request passed
   through SOPC_LibSub_AsyncSendRequestOnSession.
 @var SOPC_LibSub_ConnectionCfg::expected_endpoints
      Response returned by prior call to GetEndpoints service
      and checked to be the same during session establishment,
      NULL otherwise (no verification will be done).
      Its type shall be a pointer of ::OpcUa_GetEndpointsResponse.
 */
typedef struct
{
    SOPC_LibSub_CstString server_url;
    SOPC_LibSub_CstString security_policy;
    OpcUa_MessageSecurityMode security_mode;
    uint8_t disable_certificate_verification;
    SOPC_LibSub_CstString path_cert_auth;
    SOPC_LibSub_CstString path_crl;
    SOPC_LibSub_CstString path_cert_srv;
    SOPC_LibSub_CstString path_cert_cli;
    SOPC_LibSub_CstString path_key_cli;
    SOPC_LibSub_CstString policyId;
    SOPC_LibSub_CstString username;
    SOPC_LibSub_CstString password;
    int64_t publish_period_ms;
    uint32_t n_max_keepalive;
    uint32_t n_max_lifetime;
    SOPC_LibSub_DataChangeCbk* data_change_callback;
    int64_t timeout_ms;
    uint32_t sc_lifetime;
    uint16_t token_target;
    SOPC_LibSub_EventCbk* generic_response_callback;
    const void* expected_endpoints;
} SOPC_LibSub_ConnectionCfg;

/*
 ===================
 SERVICES DEFINITION
 =================== */

/**
    \brief Return the current version of the library
*/
SOPC_LibSub_CstString SOPC_LibSub_GetVersion(void);

/**
 @brief
    Configure the library. This function shall be called once by the host application
    before any other service can be used.
 @warning
    This function is not threadsafe.
 @param pCfg
    Non null pointer to the static configuration. The content of the configuration is copied
    and the object pointed by /p pCfg can be freed by the caller.
 @return
    The operation status */
SOPC_ReturnStatus SOPC_LibSub_Initialize(const SOPC_LibSub_StaticCfg* pCfg);

/**
 @brief
    Clears the connections, configurations, and clears the Toolkit.
 @warning
    As this function should be called only once, it is not threadsafe. */
void SOPC_LibSub_Clear(void);

/**
 @brief
    Configure a future connection. This function shall be called once per connection before
    a call to SOPC_LibSub_Configured(). The given /p pCfgId is later used to create connections.
 @param pCfg
    Non null pointer to the connection configuration. The content of the configuration is copied
    and the object pointed by /p pCfg can be freed by the caller.
 @param[out] pCfgId
    The configuration connection id. Set when the value returned is "SOPC_STATUS_OK".
 @return
    The operation status */
SOPC_ReturnStatus SOPC_LibSub_ConfigureConnection(const SOPC_LibSub_ConnectionCfg* pCfg,
                                                  SOPC_LibSub_ConfigurationId* pCfgId);

/**
 @brief
    Mark the library as configured. All calls to SOPC_LibSub_ConfigureConnection() shall
    be done prior to calling this function. All calls to SOPC_LibSub_Connect() shall be done
    after calling this function.
 @warning
    As this function should be called only once, it is not threadsafe.
 @return
    The operation status */
SOPC_ReturnStatus SOPC_LibSub_Configured(void);

/**
 @brief
    Creates a new connection to a remote OPC server from configuration id cfg_id.
    The connection represent the whole client and is later identified by the returned cli_id.
    A subscription is created and associated with this client.
    The function waits until the client is effectively connected and the subscription created,
    or the Toolkit times out.
 @param cfgId
    The parameters of the connection to create, return by SOPC_LibSub_ConfigureConnection().
 @param[out] pCliId
    The connection id of the newly created client, set when return is SOPC_STATUS_OK.
 @return
    The operation status and SOPC_STATUS_TIMEOUT when connection hanged for more than
    connection_cfg->timeout_ms milliseconds
 @warning
    The disconnect callback might be called before the end of the function
 */
SOPC_ReturnStatus SOPC_LibSub_Connect(const SOPC_LibSub_ConfigurationId cfgId, SOPC_LibSub_ConnectionId* pCliId);

/**
 @brief
    Add variables to the subscription of the connection.
    This call is synchroneous: it waits for the server response, or the Toolkit times out.
    The connection timeout is also used for this function.
 @param cliId
    The connection id.
 @param lszNodeId
    An array of zero-terminated strings describing the NodeIds to add.
    It should be at least \p nElements long.
 @param lattrId
    An array of attributes id. The subscription is created for the attribute lAttrId[i]
    for the node id lszNodeId[i].
    It should be at least \p nElements long.
 @param nElements
    The number of elements in previous arrays.
 @param[out] lDataId
    A pre-allocated array to the output unique variable data identifiers.
    It should be at least \p nElements long.
    The values will be used in call to data_change_callback.
 @return
    The operation status. lDataId is only valid when the return status is SOPC_STATUS_OK.
    SOPC_STATUS_TIMEOUT is returned when the timeout expires before receiving a response. */
SOPC_ReturnStatus SOPC_LibSub_AddToSubscription(const SOPC_LibSub_ConnectionId cliId,
                                                const SOPC_LibSub_CstString* lszNodeId,
                                                const SOPC_LibSub_AttributeId* lattrId,
                                                int32_t nElements,
                                                SOPC_LibSub_DataId* lDataId);

/**
 @brief
    Sends a generic request on the connection. The request must be accepted by the SOPC encoders
 (OpcUa_<MessageStruct>*) which are defined in "sopc_types.h". Upon response, the SOPC_LibSub_EventCbk callback
 configured with this connection is called with the OpcUa response.
 @param cliId
    The connection id.
 @param requestStruct
    OPC UA message payload structure pointer (OpcUa_<MessageStruct>*). Deallocated by toolkit.
 @param requestContext
    A context value, it will be provided in the callback alongside the corresponding response.
 */
SOPC_ReturnStatus SOPC_LibSub_AsyncSendRequestOnSession(SOPC_LibSub_ConnectionId cliId,
                                                        void* requestStruct,
                                                        uintptr_t requestContext);
/**
 @brief
    Disconnect from a remote OPC server.
    The function waits until the client is effectively disconnected, or the Toolkit times out.
 @param cliId
    The connection id to disconnect
 @return
    The operation status. Erroneous case are:
    - unitialized or unconfigured toolkit (SOPC_STATUS_INVALID_STATE),
    - inexisting connection (SOPC_STATUS_INVALID_PARAMETERS),
    - already closed connection (SOPC_STATUS_NOK). */
SOPC_ReturnStatus SOPC_LibSub_Disconnect(const SOPC_LibSub_ConnectionId cliId);

/*--------------------------------
    TBC??
    - delete_subscription
    - delete_from_subscription
--------------------------------*/

/**
 * \brief Buffers a log message, then calls the callback configured with the LibSub.
 *
 */
void Helpers_Log(const SOPC_Log_Level log_level, const char* format, ...);

/**
 * \brief Helper logger, prints a log message to stdout, with the following format "# log_level: text\n".
 */
void Helpers_LoggerStdout(const SOPC_Log_Level log_level, const SOPC_LibSub_CstString text);

#endif /* LIBS2OPC_CLIENT_H_ */
