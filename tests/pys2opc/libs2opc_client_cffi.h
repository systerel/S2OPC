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

typedef uint32_t SOPC_StatusCode;
typedef uint8_t bool;

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

typedef enum
{
    SOPC_TOOLKIT_LOG_LEVEL_ERROR = 0,
    SOPC_TOOLKIT_LOG_LEVEL_WARNING = 1,
    SOPC_TOOLKIT_LOG_LEVEL_INFO = 2,
    SOPC_TOOLKIT_LOG_LEVEL_DEBUG = 3
} SOPC_Toolkit_Log_Level;

typedef enum
{
    OpcUa_MessageSecurityMode_Invalid = 0,
    OpcUa_MessageSecurityMode_None = 1,
    OpcUa_MessageSecurityMode_Sign = 2,
    OpcUa_MessageSecurityMode_SignAndEncrypt = 3
} OpcUa_MessageSecurityMode;

const char* SOPC_SecurityPolicy_None_URI;
const char* SOPC_SecurityPolicy_Basic256_URI;
const char* SOPC_SecurityPolicy_Basic256Sha256_URI;

void SOPC_Sleep(unsigned int milliseconds);

/**
 * \brief Configure the toolkit log generation properties (SOPC_LibSub_Initialize required,
 *        !SOPC_LibSub_Configured required)
 *
 * \param logDirPath       Absolute or relative path of the directory to be used for logs
 *                        (full path shall exists or createDirectory flag shall be set,
 *                         path shall terminate with directory separator).
 *                         Default value is execution directory (same * as "" value provided).
 *
 * \param createDirectory  Flag indicating if the directory (last item of path regarding directory separator) shall be
 *                         created
 *
 * \warning The value of the pointer \p logDirPath is used afterwards by the Toolkit. The string is not copied. Hence,
 *          it must not be modified nor freed by the caller before SOPC_LibSub_Clear.
 */
SOPC_ReturnStatus SOPC_ToolkitConfig_SetCircularLogPath(const char* logDirPath, bool createDirectory);

/**
 * \brief Configure the toolkit log generation properties (SOPC_LibSub_Initialize required,
 *        !SOPC_LibSub_Configured required)
 *
 *
 * \param maxBytes      A maximum amount of bytes (> 100) by log file before opening a new file incrementing the integer
 *                      suffix. It is a best effort value (amount verified after each print). Default value is 1048576.
 *
 * \param maxFiles      A maximum number of files (> 0) to be used, when reached the older log file is overwritten
 *                      (starting with *_00001.log). Default value is 50.
 */
SOPC_ReturnStatus SOPC_ToolkitConfig_SetCircularLogProperties(uint32_t maxBytes, uint16_t maxFiles);

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

/*
  @description
    Structure defining the value of a node
  @field quality
    The value quality.
  @field type
    The value type. Specifies the type of '*value' amongst:
    - SOPC_LibSub_CstString (SOPC_LibSub_DataType_string / SOPC_LibSub_DataType_bytestring)
    - int64_t (SOPC_LibSub_DataType_bool / SOPC_LibSub_DataType_integer)
    - NULL (SOPC_LibSub_DataType_other)
  @field length
    The length of the value, in case it is a bytestring. 0 otherwise.
  @field raw_value
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

/*
  @description
    AttributeIds, as defined in the OPC UA Reference, Part 6 Annex A */
typedef enum
{
    SOPC_LibSub_AttributeId_NodeId = 1,
    SOPC_LibSub_AttributeId_NodeClass = 2,
    SOPC_LibSub_AttributeId_BrowseName = 3,
    SOPC_LibSub_AttributeId_DisplayName = 4,
    SOPC_LibSub_AttributeId_Description = 5,
    SOPC_LibSub_AttributeId_WriteMask = 6,
    SOPC_LibSub_AttributeId_UserWriteMask = 7,
    SOPC_LibSub_AttributeId_IsAbstract = 8,
    SOPC_LibSub_AttributeId_Symmetric = 9,
    SOPC_LibSub_AttributeId_InverseName = 10,
    SOPC_LibSub_AttributeId_ContainsNoLoops = 11,
    SOPC_LibSub_AttributeId_EventNotifier = 12,
    SOPC_LibSub_AttributeId_Value = 13,
    SOPC_LibSub_AttributeId_DataType = 14,
    SOPC_LibSub_AttributeId_ValueRank = 15,
    SOPC_LibSub_AttributeId_ArrayDimensions = 16,
    SOPC_LibSub_AttributeId_AccessLevel = 17,
    SOPC_LibSub_AttributeId_UserAccessLevel = 18,
    SOPC_LibSub_AttributeId_MinimumSamplingInterval = 19,
    SOPC_LibSub_AttributeId_Historizing = 20,
    SOPC_LibSub_AttributeId_Executable = 21,
    SOPC_LibSub_AttributeId_UserExecutable = 22
} SOPC_LibSub_AttributeId;

/*
  @description
    The event passed to the connection SOPC_LibSub_EventCbk.
    Either an error or a valid response notification.
*/
typedef enum SOPC_LibSub_ApplicativeEvent
{
    SOPC_LibSub_ApplicativeEvent_SendFailed,
    SOPC_LibSub_ApplicativeEvent_Response
} SOPC_LibSub_ApplicativeEvent;

/*
  @description
    Log callback type
  @param log_level
    The Log level (SOPC_Toolkit_Log_Level). Note: SOPC_log_error shall be non-returning.
  @param text
    The text string to log (shall not be null) */
typedef void (*SOPC_LibSub_LogCbk)(const SOPC_Toolkit_Log_Level log_level, SOPC_LibSub_CstString text);

/*
  @description
    Callback type for disconnect event
  @param c_id
    The connection id that has been disconnected */
typedef void (*SOPC_LibSub_DisconnectCbk)(const SOPC_LibSub_ConnectionId c_id);

/*
  @description
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
typedef void (*SOPC_LibSub_DataChangeCbk)(const SOPC_LibSub_ConnectionId c_id,
                                          const SOPC_LibSub_DataId d_id,
                                          const SOPC_LibSub_Value* value);

/*
  @description
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
typedef void (*SOPC_LibSub_EventCbk)(SOPC_LibSub_ConnectionId c_id,
                                     SOPC_LibSub_ApplicativeEvent event,
                                     SOPC_StatusCode status,
                                     const void* response,
                                     uintptr_t responseContext);

/*
 =====================
 STRUCTURES DEFINITION
 ===================== */

/*
 @description
   Static configuration of OPC client libray
 @field host_log_callback
   Host log callback
 @field disconnect_callback
   Notification event for disconnection from server */
typedef struct
{
    SOPC_LibSub_LogCbk host_log_callback;
    SOPC_LibSub_DisconnectCbk disconnect_callback;
} SOPC_LibSub_StaticCfg;

/*
 @description
   Connection configuration to a remote OPC server
 @field server_url
   Zero-terminated path to server URL
 @field security_policy
   The chosen OPC-UA security policy for the connection, one of the SOPC_SecurityPolicy_*_URI string
 @field security_mode
   The chosen OPC-UA security mode for the connection, one of the OpcUa_MessageSecurityMode constant
 @field disable_certificate_verification
   Uses a PKIProvider which does not verify the certificates against a certificate authority.
   Setting this flag to not 0 severely harms security. The certificate authority is not required in this case.
   Other certificates are still required when using an encrypted or signed secure channel.
 @field path_cert_auth
   Zero-terminated path to the root certificate authority in the DER format
 @field path_cert_srv
   Zero-terminated path to the server certificate in the DER format, signed by the root certificate authority
 @field path_cert_cli
   Zero-terminated path to the client certificate in the DER format, signed by the root certificate authority
 @field path_key_cli
   Zero-terminated path to the client private key which is paired to the public key signed server certificate,
   in the DER format
 @field path_crl
   Zero-terminated path to the certificate revocation list in the DER format
 @field policyId
   Zero-terminated policy id. To know which policy id to use, please read a
   GetEndpointsResponse or a CreateSessionResponse. When username is NULL, the
   AnonymousIdentityToken is used and the policy id must correspond to an
   anonymous UserIdentityPolicy. Otherwise, the UserNameIdentityToken is used
   and the policy id must correspond to an username UserIdentityPolicy.
 @field username
   Zero-terminated username, NULL for anonymous access, see policyId
 @field password
   Zero-terminated password, ignored when username is NULL. Password is kept in memory for future reconnections.
 @field publish_period_ms
   The requested publish period for the created subscription (in milliseconds)
 @field n_max_keepalive
   The max keep alive count for the subscription. When there is no notification to send, the subscription waits
   at most the number of publish cycle before sending a publish response (which is then empty).
 @field n_max_lifetime
   The max number of time that a subscription may timeout its publish cycle without being able to send a
   response (because there is no publish request to answer to). In this case, the subscription is killed by the
   server. A large value is recommended (e.g. 1000).
 @field data_change_callback
   The callback for data change notification
 @field timeout_ms
   Connection timeout (milliseconds)
 @field sc_lifetime
   Time before secure channel renewal (milliseconds). A parameter larger than 60000 is recommended.
 @field token_target
   Number of tokens (PublishRequest) that the client tries to maintain throughout the connection
 @field generic_response_callback
   The callback used to transmit generic responses to request passed
   through SOPC_LibSub_AsyncSendRequestOnSession.
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
    SOPC_LibSub_DataChangeCbk data_change_callback;
    int64_t timeout_ms;
    uint32_t sc_lifetime;
    uint16_t token_target;
    SOPC_LibSub_EventCbk generic_response_callback;
} SOPC_LibSub_ConnectionCfg;

/*
 ===================
 SERVICES DEFINITION
 =================== */

/*
    Return the current version of the library
*/
SOPC_LibSub_CstString SOPC_LibSub_GetVersion(void);

/*
 @description
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

/*
 @description
    Clears the connections, configurations, and clears the Toolkit.
 @warning
    As this function should be called only once, it is not threadsafe. */
void SOPC_LibSub_Clear(void);

/*
 @description
    Configure a future connection. This function shall be called once per connection before
    a call to SOPC_LibSub_Configured(). The given /p pCfgId is later used to create connections.
 @param pCfg
    Non null pointer to the connection configuration. The content of the configuration is copied
    and the object pointed by /p pCfg can be freed by the caller.
 @param pCfgId [out, not null]
    The configuration connection id. Set when the value returned is "SOPC_STATUS_OK".
 @return
    The operation status */
SOPC_ReturnStatus SOPC_LibSub_ConfigureConnection(const SOPC_LibSub_ConnectionCfg* pCfg,
                                                  SOPC_LibSub_ConfigurationId* pCfgId);

/*
 @description
    Mark the library as configured. All calls to SOPC_LibSub_ConfigureConnection() shall
    be done prior to calling this function. All calls to SOPC_LibSub_Connect() shall be done
    after calling this function.
 @warning
    As this function should be called only once, it is not threadsafe.
 @return
    The operation status */
SOPC_ReturnStatus SOPC_LibSub_Configured(void);

/*
 @description
    Creates a new connection to a remote OPC server from configuration id cfg_id.
    The connection represent the whole client and is later identified by the returned cli_id.
    A subscription is created and associated with this client.
    The function waits until the client is effectively connected and the subscription created,
    or the Toolkit times out.
 @param cfgId
    The parameters of the connection to create, return by SOPC_LibSub_ConfigureConnection().
 @param pCliId [out, not null]
    The connection id of the newly created client, set when return is SOPC_STATUS_OK.
 @return
    The operation status and SOPC_STATUS_TIMEOUT when connection hanged for more than
    connection_cfg->timeout_ms milliseconds */
SOPC_ReturnStatus SOPC_LibSub_Connect(const SOPC_LibSub_ConfigurationId cfgId, SOPC_LibSub_ConnectionId* pCliId);

/*
 @description
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
 @param lDataId [out, not null]
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

/*
 @description
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
/*
 @description
    Disconnect from a remote OPC server.
    The function waits until the client is effectively disconnected, or the Toolkit times out.
 @param c_id
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
void Helpers_Log(const SOPC_Toolkit_Log_Level log_level, const char* format, ...);

/**
 * \brief Helper logger, prints a log message to stdout, with the following format "# log_level: text\n".
 */
void Helpers_LoggerStdout(const SOPC_Toolkit_Log_Level log_level, const SOPC_LibSub_CstString text);

void* SOPC_Malloc(size_t size);
void SOPC_Free(void* ptr);
void* SOPC_Calloc(size_t nmemb, size_t size);
void* SOPC_Realloc(void* ptr, size_t old_size, size_t new_size);

typedef struct
{
    uint32_t initial_size; /**< initial size (also used as size increment step) */
    uint32_t current_size; /**< current size */
    uint32_t maximum_size; /**< maximum size */
    uint32_t position;     /**< read/write position */
    uint32_t length;       /**< data length */
    uint8_t* data;         /**< data bytes */
} SOPC_Buffer;

typedef void(SOPC_EncodeableObject_PfnInitialize)(void* value);
typedef void(SOPC_EncodeableObject_PfnClear)(void* value);
typedef void(SOPC_EncodeableObject_PfnGetSize)(void);
typedef SOPC_ReturnStatus(SOPC_EncodeableObject_PfnEncode)(const void* value, SOPC_Buffer* msgBuffer);
typedef SOPC_ReturnStatus(SOPC_EncodeableObject_PfnDecode)(void* value, SOPC_Buffer* msgBuffer);
typedef struct SOPC_EncodeableType
{
    char* TypeName;
    uint32_t TypeId;
    uint32_t BinaryEncodingTypeId;
    uint32_t XmlEncodingTypeId;
    char* NamespaceUri;
    size_t AllocationSize;
    SOPC_EncodeableObject_PfnInitialize* Initialize;
    SOPC_EncodeableObject_PfnClear* Clear;
    SOPC_EncodeableObject_PfnGetSize* GetSize;
    SOPC_EncodeableObject_PfnEncode* Encode;
    SOPC_EncodeableObject_PfnDecode* Decode;
} SOPC_EncodeableType;

// typedef uint8_t SOPC_Byte;
//
// typedef struct SOPC_String
//{
//    int32_t Length;
//    bool DoNotClear; // flag indicating if bytes must be freed
//    SOPC_Byte* Data;
//} SOPC_String;
//
// typedef SOPC_String SOPC_ByteString;
//
// typedef struct SOPC_Guid
//{
//    uint32_t Data1;
//    uint16_t Data2;
//    uint16_t Data3;
//    SOPC_Byte Data4[8];
//} SOPC_Guid;
//
// typedef enum SOPC_IdentifierType
//{
//    SOPC_IdentifierType_Numeric = 0x00,
//    SOPC_IdentifierType_String = 0x01,
//    SOPC_IdentifierType_Guid = 0x02,
//    SOPC_IdentifierType_ByteString = 0x03,
//} SOPC_IdentifierType;
//
// typedef struct SOPC_NodeId
//{
//    SOPC_IdentifierType IdentifierType;
//    uint16_t Namespace;
//
//    union {
//        uint32_t Numeric;
//        SOPC_String String;
//        SOPC_Guid* Guid;
//        SOPC_ByteString Bstring;
//    } Data;
//} SOPC_NodeId;
//
// typedef struct SOPC_QualifiedName
//{
//    uint16_t NamespaceIndex;
//    SOPC_String Name;
//} SOPC_QualifiedName;

/********************************************************************
 * builtintypes.h
 *******************************************************************/

typedef SOPC_ReturnStatus(SOPC_EncodeableObject_PfnCopy)(void* dest, const void* src);
typedef SOPC_ReturnStatus(SOPC_EncodeableObject_PfnComp)(const void* left, const void* right, int32_t* comp);

typedef enum SOPC_BuiltinId
{
    SOPC_Null_Id = 0,
    SOPC_Boolean_Id = 1,
    SOPC_SByte_Id = 2,
    SOPC_Byte_Id = 3,
    SOPC_Int16_Id = 4,
    SOPC_UInt16_Id = 5,
    SOPC_Int32_Id = 6,
    SOPC_UInt32_Id = 7,
    SOPC_Int64_Id = 8,
    SOPC_UInt64_Id = 9,
    SOPC_Float_Id = 10,
    SOPC_Double_Id = 11,
    SOPC_String_Id = 12,
    SOPC_DateTime_Id = 13,
    SOPC_Guid_Id = 14,
    SOPC_ByteString_Id = 15,
    SOPC_XmlElement_Id = 16,
    SOPC_NodeId_Id = 17,
    SOPC_ExpandedNodeId_Id = 18,
    SOPC_StatusCode_Id = 19,
    SOPC_QualifiedName_Id = 20,
    SOPC_LocalizedText_Id = 21,
    SOPC_ExtensionObject_Id = 22,
    SOPC_DataValue_Id = 23,
    SOPC_Variant_Id = 24,
    SOPC_DiagnosticInfo_Id = 25
} SOPC_BuiltinId;

typedef uint8_t SOPC_Byte;

typedef SOPC_Byte SOPC_Boolean;

typedef int8_t SOPC_SByte;

typedef struct SOPC_String
{
    int32_t Length;
    bool DoNotClear; // flag indicating if bytes must be freed
    SOPC_Byte* Data;
} SOPC_String;

typedef SOPC_String SOPC_XmlElement;
typedef SOPC_String SOPC_ByteString;

typedef int64_t SOPC_DateTime;

typedef struct SOPC_Guid
{
    uint32_t Data1;
    uint16_t Data2;
    uint16_t Data3;
    SOPC_Byte Data4[8];
} SOPC_Guid;

typedef enum SOPC_IdentifierType
{
    SOPC_IdentifierType_Numeric = 0x00,
    SOPC_IdentifierType_String = 0x01,
    SOPC_IdentifierType_Guid = 0x02,
    SOPC_IdentifierType_ByteString = 0x03,
} SOPC_IdentifierType;

typedef struct SOPC_NodeId
{
    SOPC_IdentifierType IdentifierType;
    uint16_t Namespace;

    union {
        uint32_t Numeric;
        SOPC_String String;
        SOPC_Guid* Guid;
        SOPC_ByteString Bstring;
    } Data;
} SOPC_NodeId;

typedef struct SOPC_ExpandedNodeId
{
    SOPC_NodeId NodeId;
    SOPC_String NamespaceUri;
    uint32_t ServerIndex;
} SOPC_ExpandedNodeId;

typedef struct SOPC_DiagnosticInfo
{
    int32_t SymbolicId;
    int32_t NamespaceUri;
    int32_t Locale;
    int32_t LocalizedText;
    SOPC_String AdditionalInfo;
    SOPC_StatusCode InnerStatusCode;
    struct SOPC_DiagnosticInfo* InnerDiagnosticInfo;
} SOPC_DiagnosticInfo;

typedef struct SOPC_QualifiedName
{
    uint16_t NamespaceIndex;
    SOPC_String Name;
} SOPC_QualifiedName;

typedef struct SOPC_LocalizedText
{
    SOPC_String defaultLocale;
    SOPC_String defaultText;

    void* localizedTextList; // Unused field on client side => hide type
} SOPC_LocalizedText;

typedef enum SOPC_ExtObjectBodyEncoding
{
    SOPC_ExtObjBodyEncoding_None = 0x00,
    SOPC_ExtObjBodyEncoding_ByteString = 0x01,
    SOPC_ExtObjBodyEncoding_XMLElement = 0x02,
    SOPC_ExtObjBodyEncoding_Object = 0x03
} SOPC_ExtObjectBodyEncoding;

typedef struct SOPC_ExtensionObject
{
    SOPC_ExpandedNodeId TypeId;
    SOPC_ExtObjectBodyEncoding Encoding;

    union {
        SOPC_ByteString Bstring;
        SOPC_XmlElement Xml;
        struct
        {
            void* Value;
            SOPC_EncodeableType* ObjType;
        } Object;

    } Body;

    int32_t Length;

} SOPC_ExtensionObject;

typedef enum SOPC_VariantArrayTypeFlag
{
    SOPC_VariantArrayValueFlag = 128,    // 2^7 => bit 7
    SOPC_VariantArrayDimensionsFlag = 64 // 2^6 => bit 6
} SOPC_VariantArrayTypeFlag;

// Binary compatible types
typedef enum SOPC_VariantArrayType
{
    SOPC_VariantArrayType_SingleValue = 0x0,
    SOPC_VariantArrayType_Array = 0x1,
    SOPC_VariantArrayType_Matrix = 0x2
} SOPC_VariantArrayType;

struct SOPC_DataValue;
struct SOPC_Variant;

typedef union SOPC_VariantArrayValue {
    SOPC_Boolean* BooleanArr;
    SOPC_SByte* SbyteArr;
    SOPC_Byte* ByteArr;
    int16_t* Int16Arr;
    uint16_t* Uint16Arr;
    int32_t* Int32Arr;
    uint32_t* Uint32Arr;
    int64_t* Int64Arr;
    uint64_t* Uint64Arr;
    float* FloatvArr;
    double* DoublevArr;
    SOPC_String* StringArr;
    SOPC_DateTime* DateArr;
    SOPC_Guid* GuidArr;
    SOPC_ByteString* BstringArr;
    SOPC_XmlElement* XmlEltArr;
    SOPC_NodeId* NodeIdArr;
    SOPC_ExpandedNodeId* ExpNodeIdArr;
    SOPC_StatusCode* StatusArr;
    SOPC_QualifiedName* QnameArr;
    SOPC_LocalizedText* LocalizedTextArr;
    SOPC_ExtensionObject* ExtObjectArr;
    struct SOPC_DataValue* DataValueArr;
    struct SOPC_Variant* VariantArr;
    SOPC_DiagnosticInfo* DiagInfoArr; // TODO: not present ?
} SOPC_VariantArrayValue;

typedef union SOPC_VariantValue {
    SOPC_Boolean Boolean;
    SOPC_SByte Sbyte;
    SOPC_Byte Byte;
    int16_t Int16;
    uint16_t Uint16;
    int32_t Int32;
    uint32_t Uint32;
    int64_t Int64;
    uint64_t Uint64;
    float Floatv;
    double Doublev;
    SOPC_String String;
    SOPC_DateTime Date;
    SOPC_Guid* Guid;
    SOPC_ByteString Bstring;
    SOPC_XmlElement XmlElt;
    SOPC_NodeId* NodeId;
    SOPC_ExpandedNodeId* ExpNodeId;
    SOPC_StatusCode Status;
    SOPC_QualifiedName* Qname;
    SOPC_LocalizedText* LocalizedText;
    SOPC_ExtensionObject* ExtObject;
    struct SOPC_DataValue* DataValue;
    SOPC_DiagnosticInfo* DiagInfo; // TODO: not present ?
    struct
    {
        int32_t Length;
        SOPC_VariantArrayValue Content;
    } Array;
    struct
    {
        int32_t Dimensions;
        int32_t*
            ArrayDimensions; // Product of dimensions must be <= INT32_MAX ! (binary arrayLength valid for matrix too)
        SOPC_VariantArrayValue Content;
    } Matrix;

} SOPC_VariantValue;

typedef struct SOPC_Variant
{
    bool DoNotClear; // flag indicating if variant content must be freed
    SOPC_BuiltinId BuiltInTypeId;
    SOPC_VariantArrayType ArrayType;
    SOPC_VariantValue Value;
} SOPC_Variant;

typedef struct SOPC_DataValue
{
    SOPC_Variant Value;
    SOPC_StatusCode Status;
    SOPC_DateTime SourceTimestamp;
    SOPC_DateTime ServerTimestamp;
    uint16_t SourcePicoSeconds;
    uint16_t ServerPicoSeconds;
} SOPC_DataValue;

//#define SECURITY_POLICY_NONE "http://opcfoundation.org/UA/SecurityPolicy#None"
//#define SECURITY_POLICY_BASIC128RSA15 "http://opcfoundation.org/UA/SecurityPolicy#Basic128Rsa15"
//#define SECURITY_POLICY_BASIC256 "http://opcfoundation.org/UA/SecurityPolicy#Basic256"
//#define SECURITY_POLICY_BASIC256SHA256 "http://opcfoundation.org/UA/SecurityPolicy#Basic256Sha256"

void SOPC_Boolean_Initialize(SOPC_Boolean* b);
void SOPC_Boolean_InitializeAux(void* value);
SOPC_ReturnStatus SOPC_Boolean_CopyAux(void* dest, const void* src);
SOPC_ReturnStatus SOPC_Boolean_CompareAux(const void* left, const void* right, int32_t* comparison);
void SOPC_Boolean_Clear(SOPC_Boolean* b);
void SOPC_Boolean_ClearAux(void* value);

void SOPC_SByte_Initialize(SOPC_SByte* sbyte);
void SOPC_SByte_InitializeAux(void* value);
SOPC_ReturnStatus SOPC_SByte_CopyAux(void* dest, const void* src);
SOPC_ReturnStatus SOPC_SByte_CompareAux(const void* left, const void* right, int32_t* comparison);
void SOPC_SByte_Clear(SOPC_SByte* sbyte);
void SOPC_SByte_ClearAux(void* value);

void SOPC_Byte_Initialize(SOPC_Byte* byte);
void SOPC_Byte_InitializeAux(void* value);
SOPC_ReturnStatus SOPC_Byte_CopyAux(void* dest, const void* src);
SOPC_ReturnStatus SOPC_Byte_CompareAux(const void* left, const void* right, int32_t* comparison);
void SOPC_Byte_Clear(SOPC_Byte* byte);
void SOPC_Byte_ClearAux(void* value);

void SOPC_Int16_Initialize(int16_t* intv);
void SOPC_Int16_InitializeAux(void* value);
SOPC_ReturnStatus SOPC_Int16_CopyAux(void* dest, const void* src);
SOPC_ReturnStatus SOPC_Int16_CompareAux(const void* left, const void* right, int32_t* comparison);
void SOPC_Int16_Clear(int16_t* intv);
void SOPC_Int16_ClearAux(void* value);

void SOPC_UInt16_Initialize(uint16_t* uint);
void SOPC_UInt16_InitializeAux(void* value);
SOPC_ReturnStatus SOPC_UInt16_CopyAux(void* dest, const void* src);
SOPC_ReturnStatus SOPC_UInt16_CompareAux(const void* left, const void* right, int32_t* comparison);
void SOPC_UInt16_Clear(uint16_t* uint);
void SOPC_UInt16_ClearAux(void* value);

void SOPC_Int32_Initialize(int32_t* intv);
void SOPC_Int32_InitializeAux(void* value);
SOPC_ReturnStatus SOPC_Int32_CopyAux(void* dest, const void* src);
SOPC_ReturnStatus SOPC_Int32_CompareAux(const void* left, const void* right, int32_t* comparison);
void SOPC_Int32_Clear(int32_t* intv);
void SOPC_Int32_ClearAux(void* value);

void SOPC_UInt32_Initialize(uint32_t* uint);
void SOPC_UInt32_InitializeAux(void* value);
SOPC_ReturnStatus SOPC_UInt32_CopyAux(void* dest, const void* src);
SOPC_ReturnStatus SOPC_UInt32_CompareAux(const void* left, const void* right, int32_t* comparison);
void SOPC_UInt32_Clear(uint32_t* uint);
void SOPC_UInt32_ClearAux(void* value);

void SOPC_Int64_Initialize(int64_t* intv);
void SOPC_Int64_InitializeAux(void* value);
SOPC_ReturnStatus SOPC_Int64_CopyAux(void* dest, const void* src);
SOPC_ReturnStatus SOPC_Int64_CompareAux(const void* left, const void* right, int32_t* comparison);
void SOPC_Int64_Clear(int64_t* intv);
void SOPC_Int64_ClearAux(void* value);

void SOPC_UInt64_Initialize(uint64_t* uint);
void SOPC_UInt64_InitializeAux(void* value);
SOPC_ReturnStatus SOPC_UInt64_CopyAux(void* dest, const void* src);
SOPC_ReturnStatus SOPC_UInt64_CompareAux(const void* left, const void* right, int32_t* comparison);
void SOPC_UInt64_Clear(uint64_t* uint);
void SOPC_UInt64_ClearAux(void* value);

void SOPC_Float_Initialize(float* f);
void SOPC_Float_InitializeAux(void* value);
SOPC_ReturnStatus SOPC_Float_CopyAux(void* dest, const void* src);
SOPC_ReturnStatus SOPC_Float_CompareAux(const void* left, const void* right, int32_t* comparison);
void SOPC_Float_Clear(float* f);
void SOPC_Float_ClearAux(void* value);

void SOPC_Double_Initialize(double* d);
void SOPC_Double_InitializeAux(void* value);
SOPC_ReturnStatus SOPC_Double_CopyAux(void* dest, const void* src);
SOPC_ReturnStatus SOPC_Double_CompareAux(const void* left, const void* right, int32_t* comparison);
void SOPC_Double_Clear(double* d);
void SOPC_Double_ClearAux(void* value);

void SOPC_ByteString_Initialize(SOPC_ByteString* bstring);
void SOPC_ByteString_InitializeAux(void* value);
SOPC_ByteString* SOPC_ByteString_Create(void);
SOPC_ReturnStatus SOPC_ByteString_InitializeFixedSize(SOPC_ByteString* bstring, uint32_t size);
SOPC_ReturnStatus SOPC_ByteString_CopyFromBytes(SOPC_ByteString* dest, const SOPC_Byte* bytes, int32_t length);
SOPC_ReturnStatus SOPC_ByteString_Copy(SOPC_ByteString* dest, const SOPC_ByteString* src);
SOPC_ReturnStatus SOPC_ByteString_CopyAux(void* dest, const void* src);
void SOPC_ByteString_Clear(SOPC_ByteString* bstring);
void SOPC_ByteString_ClearAux(void* value);
void SOPC_ByteString_Delete(SOPC_ByteString* bstring);

SOPC_ReturnStatus SOPC_ByteString_Compare(const SOPC_ByteString* left,
                                          const SOPC_ByteString* right,
                                          int32_t* comparison);
SOPC_ReturnStatus SOPC_ByteString_CompareAux(const void* left, const void* right, int32_t* comparison);

bool SOPC_ByteString_Equal(const SOPC_ByteString* left, const SOPC_ByteString* right);

void SOPC_String_Initialize(SOPC_String* string);
void SOPC_String_InitializeAux(void* value);
SOPC_String* SOPC_String_Create(void);
SOPC_ReturnStatus SOPC_String_CopyFromCString(SOPC_String* string, const char* cString);
SOPC_ReturnStatus SOPC_String_CopyAux(void* dest, const void* src);
SOPC_ReturnStatus SOPC_String_InitializeFromCString(SOPC_String* string, const char* cString);
char* SOPC_String_GetCString(const SOPC_String* string);          // Copy
const char* SOPC_String_GetRawCString(const SOPC_String* string); // Pointer to string

SOPC_ReturnStatus SOPC_String_AttachFrom(SOPC_String* dest, SOPC_String* src);
SOPC_ReturnStatus SOPC_String_AttachFromCstring(SOPC_String* dest, char* src);

SOPC_ReturnStatus SOPC_String_Copy(SOPC_String* dest, const SOPC_String* src);
void SOPC_String_Clear(SOPC_String* bstring);
void SOPC_String_ClearAux(void* value);
void SOPC_String_Delete(SOPC_String* bstring);

SOPC_ReturnStatus SOPC_String_Compare(const SOPC_String* left,
                                      const SOPC_String* right,
                                      bool ignoreCase,
                                      int32_t* comparison);
SOPC_ReturnStatus SOPC_String_CompareAux(const void* left, const void* right, int32_t* comparison);

bool SOPC_String_Equal(const SOPC_String* left, const SOPC_String* right);

void SOPC_XmlElement_Initialize(SOPC_XmlElement* xmlElt);
void SOPC_XmlElement_InitializeAux(void* value);
SOPC_ReturnStatus SOPC_XmlElement_Copy(SOPC_XmlElement* dest, const SOPC_XmlElement* src);
SOPC_ReturnStatus SOPC_XmlElement_CopyAux(void* dest, const void* src);
SOPC_ReturnStatus SOPC_XmlElement_Compare(const SOPC_XmlElement* left,
                                          const SOPC_XmlElement* right,
                                          int32_t* comparison);
SOPC_ReturnStatus SOPC_XmlElement_CompareAux(const void* left, const void* right, int32_t* comparison);
void SOPC_XmlElement_Clear(SOPC_XmlElement* xmlElt);
void SOPC_XmlElement_ClearAux(void* value);

void SOPC_DateTime_Initialize(SOPC_DateTime* dateTime);
void SOPC_DateTime_InitializeAux(void* value);
SOPC_ReturnStatus SOPC_DateTime_CopyAux(void* dest, const void* src);
SOPC_ReturnStatus SOPC_DateTime_Compare(const SOPC_DateTime* left, const SOPC_DateTime* right, int32_t* comparison);
SOPC_ReturnStatus SOPC_DateTime_CompareAux(const void* left, const void* right, int32_t* comparison);
void SOPC_DateTime_Clear(SOPC_DateTime* dateTime);
void SOPC_DateTime_ClearAux(void* value);

void SOPC_Guid_Initialize(SOPC_Guid* guid);
void SOPC_Guid_InitializeAux(void* value);
SOPC_ReturnStatus SOPC_Guid_FromCString(SOPC_Guid* guid, const char* str, size_t len);
SOPC_ReturnStatus SOPC_Guid_Copy(SOPC_Guid* dest, const SOPC_Guid* src);
SOPC_ReturnStatus SOPC_Guid_CompareAux(const void* left, const void* right, int32_t* comparison);
SOPC_ReturnStatus SOPC_Guid_CopyAux(void* dest, const void* src);
void SOPC_Guid_Clear(SOPC_Guid* guid);
void SOPC_Guid_ClearAux(void* value);

void SOPC_NodeId_Initialize(SOPC_NodeId* nodeId);
void SOPC_NodeId_InitializeAux(void* value);
SOPC_ReturnStatus SOPC_NodeId_Copy(SOPC_NodeId* dest, const SOPC_NodeId* src);
SOPC_ReturnStatus SOPC_NodeId_CopyAux(void* dest, const void* src);
void SOPC_NodeId_Clear(SOPC_NodeId* nodeId);
void SOPC_NodeId_ClearAux(void* value);

SOPC_ReturnStatus SOPC_NodeId_Compare(const SOPC_NodeId* left, const SOPC_NodeId* right, int32_t* comparison);
SOPC_ReturnStatus SOPC_NodeId_CompareAux(const void* left, const void* right, int32_t* comparison);

void SOPC_NodeId_Hash(const SOPC_NodeId* nodeId, uint64_t* hash);

char* SOPC_NodeId_ToCString(SOPC_NodeId* nodeId);
SOPC_NodeId* SOPC_NodeId_FromCString(const char* cString, int32_t len);

// SOPC_Dict* SOPC_NodeId_Dict_Create(bool free_keys, SOPC_Dict_Free_Fct value_free);

void SOPC_ExpandedNodeId_Initialize(SOPC_ExpandedNodeId* expNodeId);
void SOPC_ExpandedNodeId_InitializeAux(void* value);
SOPC_ReturnStatus SOPC_ExpandedNodeId_Copy(SOPC_ExpandedNodeId* dest, const SOPC_ExpandedNodeId* src);
SOPC_ReturnStatus SOPC_ExpandedNodeId_CopyAux(void* dest, const void* src);
SOPC_ReturnStatus SOPC_ExpandedNodeId_Compare(const SOPC_ExpandedNodeId* left,
                                              const SOPC_ExpandedNodeId* right,
                                              int32_t* comparison);
SOPC_ReturnStatus SOPC_ExpandedNodeId_CompareAux(const void* left, const void* right, int32_t* comparison);
void SOPC_ExpandedNodeId_Clear(SOPC_ExpandedNodeId* expNodeId);
void SOPC_ExpandedNodeId_ClearAux(void* value);

void SOPC_StatusCode_Initialize(SOPC_StatusCode* status);
void SOPC_StatusCode_InitializeAux(void* value);
SOPC_ReturnStatus SOPC_StatusCode_CopyAux(void* dest, const void* src);
SOPC_ReturnStatus SOPC_StatusCode_CompareAux(const void* left, const void* right, int32_t* comparison);
void SOPC_StatusCode_Clear(SOPC_StatusCode* status);
void SOPC_StatusCode_ClearAux(void* value);

void SOPC_DiagnosticInfo_Initialize(SOPC_DiagnosticInfo* diagInfo);
void SOPC_DiagnosticInfo_InitializeAux(void* value);
SOPC_ReturnStatus SOPC_DiagnosticInfo_Copy(SOPC_DiagnosticInfo* dest, const SOPC_DiagnosticInfo* src);
SOPC_ReturnStatus SOPC_DiagnosticInfo_CopyAux(void* dest, const void* src);
SOPC_ReturnStatus SOPC_DiagnosticInfo_Compare(const SOPC_DiagnosticInfo* left,
                                              const SOPC_DiagnosticInfo* right,
                                              int32_t* comparison);
SOPC_ReturnStatus SOPC_DiagnosticInfo_CompareAux(const void* left, const void* right, int32_t* comparison);
void SOPC_DiagnosticInfo_Clear(SOPC_DiagnosticInfo* diagInfo);
void SOPC_DiagnosticInfo_ClearAux(void* value);

void SOPC_QualifiedName_Initialize(SOPC_QualifiedName* qname);
void SOPC_QualifiedName_InitializeAux(void* value);
SOPC_ReturnStatus SOPC_QualifiedName_Copy(SOPC_QualifiedName* dest, const SOPC_QualifiedName* src);
SOPC_ReturnStatus SOPC_QualifiedName_CopyAux(void* dest, const void* src);
SOPC_ReturnStatus SOPC_QualifiedName_Compare(const SOPC_QualifiedName* left,
                                             const SOPC_QualifiedName* right,
                                             int32_t* comparison);
SOPC_ReturnStatus SOPC_QualifiedName_CompareAux(const void* left, const void* right, int32_t* comparison);
void SOPC_QualifiedName_Clear(SOPC_QualifiedName* qname);
void SOPC_QualifiedName_ClearAux(void* value);
SOPC_ReturnStatus SOPC_QualifiedName_ParseCString(SOPC_QualifiedName* qname, const char* str);

void SOPC_LocalizedText_Initialize(SOPC_LocalizedText* localizedText);
void SOPC_LocalizedText_InitializeAux(void* value);
SOPC_ReturnStatus SOPC_LocalizedText_Copy(SOPC_LocalizedText* dest, const SOPC_LocalizedText* src);
SOPC_ReturnStatus SOPC_LocalizedText_CopyAux(void* dest, const void* src);
SOPC_ReturnStatus SOPC_LocalizedText_Compare(const SOPC_LocalizedText* left,
                                             const SOPC_LocalizedText* right,
                                             int32_t* comparison);
SOPC_ReturnStatus SOPC_LocalizedText_CompareAux(const void* left, const void* right, int32_t* comparison);
void SOPC_LocalizedText_Clear(SOPC_LocalizedText* localizedText);
void SOPC_LocalizedText_ClearAux(void* value);

void SOPC_ExtensionObject_Initialize(SOPC_ExtensionObject* extObj);
void SOPC_ExtensionObject_InitializeAux(void* value);
SOPC_ReturnStatus SOPC_ExtensionObject_Copy(SOPC_ExtensionObject* dest, const SOPC_ExtensionObject* src);
SOPC_ReturnStatus SOPC_ExtensionObject_CopyAux(void* dest, const void* src);
SOPC_ReturnStatus SOPC_ExtensionObject_Compare(const SOPC_ExtensionObject* left,
                                               const SOPC_ExtensionObject* right,
                                               int32_t* comparison);
SOPC_ReturnStatus SOPC_ExtensionObject_CompareAux(const void* left, const void* right, int32_t* comparison);
void SOPC_ExtensionObject_Clear(SOPC_ExtensionObject* extObj);
void SOPC_ExtensionObject_ClearAux(void* value);

SOPC_Variant* SOPC_Variant_Create(void);
void SOPC_Variant_Initialize(SOPC_Variant* variant);
void SOPC_Variant_InitializeAux(void* value);
SOPC_ReturnStatus SOPC_Variant_Copy(SOPC_Variant* dest, const SOPC_Variant* src);
SOPC_ReturnStatus SOPC_Variant_CopyAux(void* dest, const void* src);
// SOPC_ReturnStatus SOPC_Variant_HasRange(const SOPC_Variant* variant, const SOPC_NumericRange* range, bool*
// has_range); SOPC_ReturnStatus SOPC_Variant_GetRange(SOPC_Variant* dst, const SOPC_Variant* src, const
// SOPC_NumericRange* range); SOPC_ReturnStatus SOPC_Variant_SetRange(SOPC_Variant* variant, const SOPC_Variant* src,
// const SOPC_NumericRange* range);

SOPC_ReturnStatus SOPC_Variant_ShallowCopy(SOPC_Variant* dst, const SOPC_Variant* src);
void SOPC_Variant_Move(SOPC_Variant* dest, SOPC_Variant* src);

SOPC_ReturnStatus SOPC_Variant_Compare(const SOPC_Variant* left, const SOPC_Variant* right, int32_t* comparison);
SOPC_ReturnStatus SOPC_Variant_CompareAux(const void* left, const void* right, int32_t* comparison);
void SOPC_Variant_Clear(SOPC_Variant* variant);
void SOPC_Variant_ClearAux(void* value);
void SOPC_Variant_Delete(SOPC_Variant* variant);

void SOPC_DataValue_Initialize(SOPC_DataValue* dataValue);
void SOPC_DataValue_InitializeAux(void* value);
SOPC_ReturnStatus SOPC_DataValue_Copy(SOPC_DataValue* dest, const SOPC_DataValue* src);
SOPC_ReturnStatus SOPC_DataValue_CopyAux(void* dest, const void* src);
SOPC_ReturnStatus SOPC_DataValue_Compare(const SOPC_DataValue* left, const SOPC_DataValue* right, int32_t* comparison);
SOPC_ReturnStatus SOPC_DataValue_CompareAux(const void* left, const void* right, int32_t* comparison);
void SOPC_DataValue_Clear(SOPC_DataValue* dataValue);
void SOPC_DataValue_ClearAux(void* value);

void SOPC_Initialize_Array(int32_t* noOfElts,
                           void** eltsArray,
                           size_t sizeOfElt,
                           SOPC_EncodeableObject_PfnInitialize* initFct);
SOPC_ReturnStatus SOPC_Op_Array(int32_t noOfElts,
                                void* eltsArrayLeft,
                                void* eltsArrayRight,
                                size_t sizeOfElt,
                                SOPC_EncodeableObject_PfnCopy* opFct);
SOPC_ReturnStatus SOPC_Comp_Array(int32_t noOfElts,
                                  void* eltsArrayLeft,
                                  void* eltsArrayRight,
                                  size_t sizeOfElt,
                                  SOPC_EncodeableObject_PfnComp* compFct,
                                  int32_t* comparison);
void SOPC_Clear_Array(int32_t* noOfElts, void** eltsArray, size_t sizeOfElt, SOPC_EncodeableObject_PfnClear* clearFct);

/********************************************************************
 * end of builtintypes.h
 *******************************************************************/
SOPC_DateTime SOPC_Time_GetCurrentTimeUTC(void);

extern struct SOPC_EncodeableType OpcUa_ResponseHeader_EncodeableType;
typedef struct _OpcUa_ResponseHeader
{
    SOPC_EncodeableType* encodeableType;
    SOPC_DateTime Timestamp;
    uint32_t RequestHandle;
    SOPC_StatusCode ServiceResult;
    SOPC_DiagnosticInfo ServiceDiagnostics;
    int32_t NoOfStringTable;
    SOPC_String* StringTable;
    SOPC_ExtensionObject AdditionalHeader;
} OpcUa_ResponseHeader;

typedef enum _OpcUa_TimestampsToReturn
{
    OpcUa_TimestampsToReturn_Source = 0,
    OpcUa_TimestampsToReturn_Server = 1,
    OpcUa_TimestampsToReturn_Both = 2,
    OpcUa_TimestampsToReturn_Neither = 3
} OpcUa_TimestampsToReturn;

typedef enum _OpcUa_NodeClass
{
    OpcUa_NodeClass_Unspecified = 0,
    OpcUa_NodeClass_Object = 1,
    OpcUa_NodeClass_Variable = 2,
    OpcUa_NodeClass_Method = 4,
    OpcUa_NodeClass_ObjectType = 8,
    OpcUa_NodeClass_VariableType = 16,
    OpcUa_NodeClass_ReferenceType = 32,
    OpcUa_NodeClass_DataType = 64,
    OpcUa_NodeClass_View = 128
} OpcUa_NodeClass;

/* Read */

extern struct SOPC_EncodeableType OpcUa_ReadValueId_EncodeableType;
typedef struct _OpcUa_ReadValueId
{
    SOPC_EncodeableType* encodeableType;
    SOPC_NodeId NodeId;
    uint32_t AttributeId;
    SOPC_String IndexRange;
    SOPC_QualifiedName DataEncoding;
} OpcUa_ReadValueId;

extern struct SOPC_EncodeableType OpcUa_ReadRequest_EncodeableType;
typedef struct _OpcUa_ReadRequest
{
    SOPC_EncodeableType* encodeableType;
    double MaxAge;
    OpcUa_TimestampsToReturn TimestampsToReturn;
    int32_t NoOfNodesToRead;
    OpcUa_ReadValueId* NodesToRead;
} OpcUa_ReadRequest;

extern struct SOPC_EncodeableType OpcUa_ReadResponse_EncodeableType;
typedef struct _OpcUa_ReadResponse
{
    SOPC_EncodeableType* encodeableType;
    OpcUa_ResponseHeader ResponseHeader;
    int32_t NoOfResults;
    SOPC_DataValue* Results;
    int32_t NoOfDiagnosticInfos;
    SOPC_DiagnosticInfo* DiagnosticInfos;
} OpcUa_ReadResponse;

/* Write */

extern struct SOPC_EncodeableType OpcUa_WriteValue_EncodeableType;
typedef struct _OpcUa_WriteValue
{
    SOPC_EncodeableType* encodeableType;
    SOPC_NodeId NodeId;
    uint32_t AttributeId;
    SOPC_String IndexRange;
    SOPC_DataValue Value;
} OpcUa_WriteValue;

extern struct SOPC_EncodeableType OpcUa_WriteRequest_EncodeableType;
typedef struct _OpcUa_WriteRequest
{
    SOPC_EncodeableType* encodeableType;
    int32_t NoOfNodesToWrite;
    OpcUa_WriteValue* NodesToWrite;
} OpcUa_WriteRequest;

extern struct SOPC_EncodeableType OpcUa_WriteResponse_EncodeableType;
typedef struct _OpcUa_WriteResponse
{
    SOPC_EncodeableType* encodeableType;
    OpcUa_ResponseHeader ResponseHeader;
    int32_t NoOfResults;
    SOPC_StatusCode* Results;
    int32_t NoOfDiagnosticInfos;
    SOPC_DiagnosticInfo* DiagnosticInfos;
} OpcUa_WriteResponse;

/* Browse */

typedef enum _OpcUa_BrowseDirection
{
    OpcUa_BrowseDirection_Forward = 0,
    OpcUa_BrowseDirection_Inverse = 1,
    OpcUa_BrowseDirection_Both = 2
} OpcUa_BrowseDirection;

extern struct SOPC_EncodeableType OpcUa_ViewDescription_EncodeableType;
typedef struct _OpcUa_ViewDescription
{
    SOPC_EncodeableType* encodeableType;
    SOPC_NodeId ViewId;
    SOPC_DateTime Timestamp;
    uint32_t ViewVersion;
} OpcUa_ViewDescription;

typedef enum _OpcUa_BrowseResultMask
{
    OpcUa_BrowseResultMask_None = 0,
    OpcUa_BrowseResultMask_ReferenceTypeId = 1,
    OpcUa_BrowseResultMask_IsForward = 2,
    OpcUa_BrowseResultMask_NodeClass = 4,
    OpcUa_BrowseResultMask_BrowseName = 8,
    OpcUa_BrowseResultMask_DisplayName = 16,
    OpcUa_BrowseResultMask_TypeDefinition = 32,
    OpcUa_BrowseResultMask_All = 63,
    OpcUa_BrowseResultMask_ReferenceTypeInfo = 3,
    OpcUa_BrowseResultMask_TargetInfo = 60
} OpcUa_BrowseResultMask;

extern struct SOPC_EncodeableType OpcUa_BrowseDescription_EncodeableType;
typedef struct _OpcUa_BrowseDescription
{
    SOPC_EncodeableType* encodeableType;
    SOPC_NodeId NodeId;
    OpcUa_BrowseDirection BrowseDirection;
    SOPC_NodeId ReferenceTypeId;
    SOPC_Boolean IncludeSubtypes;
    uint32_t NodeClassMask;
    uint32_t ResultMask;
} OpcUa_BrowseDescription;

typedef struct _OpcUa_BrowseRequest
{
    SOPC_EncodeableType* encodeableType;
    OpcUa_ViewDescription View;
    uint32_t RequestedMaxReferencesPerNode;
    int32_t NoOfNodesToBrowse;
    OpcUa_BrowseDescription* NodesToBrowse;
} OpcUa_BrowseRequest;
extern struct SOPC_EncodeableType OpcUa_BrowseResponse_EncodeableType;

extern struct SOPC_EncodeableType OpcUa_ReferenceDescription_EncodeableType;
typedef struct _OpcUa_ReferenceDescription
{
    SOPC_EncodeableType* encodeableType;
    SOPC_NodeId ReferenceTypeId;
    SOPC_Boolean IsForward;
    SOPC_ExpandedNodeId NodeId;
    SOPC_QualifiedName BrowseName;
    SOPC_LocalizedText DisplayName;
    OpcUa_NodeClass NodeClass;
    SOPC_ExpandedNodeId TypeDefinition;
} OpcUa_ReferenceDescription;

extern struct SOPC_EncodeableType OpcUa_BrowseResult_EncodeableType;
typedef struct _OpcUa_BrowseResult
{
    SOPC_EncodeableType* encodeableType;
    SOPC_StatusCode StatusCode;
    SOPC_ByteString ContinuationPoint;
    int32_t NoOfReferences;
    OpcUa_ReferenceDescription* References;
} OpcUa_BrowseResult;

extern struct SOPC_EncodeableType OpcUa_BrowseRequest_EncodeableType;
typedef struct _OpcUa_BrowseResponse
{
    SOPC_EncodeableType* encodeableType;
    OpcUa_ResponseHeader ResponseHeader;
    int32_t NoOfResults;
    OpcUa_BrowseResult* Results;
    int32_t NoOfDiagnosticInfos;
    SOPC_DiagnosticInfo* DiagnosticInfos;
} OpcUa_BrowseResponse;
