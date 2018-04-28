/*
 *  Copyright (C) 2018 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/** \file
 *
 * \brief Interface of an example client library supporting the subscription management.
 *
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

/* Version information */
#define SOPC_LIBSUB_VERSION "0.0.1*"

/*
 =================
 TYPES DEFINITION
 ================= */

/* S2OPC types: define them except when SKIP_S2OPC_DEFINITIONS is defined.
 * To use the LibSub with other parts of the S2OPC Toolkit, you shall define SKIP_S2OPC_DEFINITIONS
 * and includes the following files before including this header:
 *  sopc_builtintypes.h
 *  sopc_toolkit_constants.h
 *  sopc_log_manager.h
 */
#ifndef SKIP_S2OPC_DEFINITIONS

/*
  Data value quality
  TBD? Masks for BAD and UNCERTAIN quality?
  Taken from sop "sopc_builtintypes.h" */
typedef uint32_t SOPC_StatusCode;

/* Result, taken from "sopc_toolkit_constants.h" */
typedef enum SOPC_ReturnStatus {
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

/* Log levels, taken from "sopc_log_manager.h" */
typedef enum {
    SOPC_LOG_LEVEL_ERROR = 0,
    SOPC_LOG_LEVEL_WARNING = 1,
    SOPC_LOG_LEVEL_INFO = 2,
    SOPC_LOG_LEVEL_DEBUG = 3
} SOPC_Log_Level;

/* SecurityMode, directly compatible with the encoded OPC-UA type,
 * taken from "sopc_types.h" */
typedef enum {
    OpcUa_MessageSecurityMode_Invalid = 0,
    OpcUa_MessageSecurityMode_None = 1,
    OpcUa_MessageSecurityMode_Sign = 2,
    OpcUa_MessageSecurityMode_SignAndEncrypt = 3
} OpcUa_MessageSecurityMode;

/* Security policies, taken from "sopc_crypto_profiles.h" */
#define SOPC_SecurityPolicy_None_URI "http://opcfoundation.org/UA/SecurityPolicy#None"
#define SOPC_SecurityPolicy_Basic256_URI "http://opcfoundation.org/UA/SecurityPolicy#Basic256"
#define SOPC_SecurityPolicy_Basic256Sha256_URI "http://opcfoundation.org/UA/SecurityPolicy#Basic256Sha256"

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
typedef enum {
    SOPC_LibSub_DataType_bool = 1,
    SOPC_LibSub_DataType_integer = 2,
    SOPC_LibSub_DataType_string = 3,
    SOPC_LibSub_DataType_bytestring = 4
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
*/
typedef struct
{
    SOPC_LibSub_DataType type;
    SOPC_StatusCode quality;
    void* value;
    SOPC_LibSub_Timestamp source_timestamp;
    SOPC_LibSub_Timestamp server_timestamp;
} SOPC_LibSub_Value;

/*
  @description
    Log callback type
  @param log_level
    The Log level (SOPC_Log_Level). Note: SOPC_log_error shall be non-returning.
  @param text
    The text string to log (shall not be null) */
typedef void (*SOPC_LibSub_LogCbk)(const SOPC_Log_Level log_level, SOPC_LibSub_CstString text);

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
    The connection id that has been disconnected */
typedef void (*SOPC_LibSub_DataChangeCbk)(const SOPC_LibSub_ConnectionId c_id,
                                          const SOPC_LibSub_DataId d_id,
                                          const SOPC_LibSub_Value* value);

/*
 =====================
 STRUCTURES DEFINITION
 ===================== */

/*
 @description
   Static configuration of OPC client libray
 @field host_log_callback
   Host log callback
 @field SOPC_LibSub_DisconnectCbk
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
   Path to server URL
 @field security_policy
   The chosen OPC-UA security policy for the connection, one of the SOPC_SecurityPolicy_*_URI string
 @field security_mode
   The chosen OPC-UA security mode for the connection, one of the OpcUa_MessageSecurityMode constant
 @field path_cert_auth
   Path to the root certificate authority in the DER format
 @field path_cert_srv
   Path to the server certificate in the DER format, signed by the root certificate authority
 @field path_cert_cli
   Path to the client certificate in the DER format, signed by the root certificate authority
 @field path_key_cli
   Path to the client private key which is paired to the public key signed server certificate, in the DER format
 @field path_crl
   Path to the certificate revocation list in the DER format
 @field username
   Username, NULL for anonymous access
 @field password
   Password, Not significant when username is NULL
 @field publish_period_ms
    The requested publish period for the created subscription (in milliseconds)
 @field data_change_callback
    The callback for data change notification
 @field timeout_ms
   Connection timeout (milliseconds)
 @field sc_lifetime
   Time before secure channel renewal (milliseconds). A parameter larger than 60000 is recommended.
 @field token_target
   Number of tokens (PublishRequest) that the client tries to maintain throughout the connection
 */
typedef struct
{
    SOPC_LibSub_CstString server_url;
    SOPC_LibSub_CstString security_policy;
    OpcUa_MessageSecurityMode security_mode;
    SOPC_LibSub_CstString path_cert_auth;
    SOPC_LibSub_CstString path_cert_srv;
    SOPC_LibSub_CstString path_cert_cli;
    SOPC_LibSub_CstString path_key_cli;
    SOPC_LibSub_CstString path_crl;
    SOPC_LibSub_CstString username;
    SOPC_LibSub_CstString password;
    int64_t publish_period_ms;
    SOPC_LibSub_DataChangeCbk data_change_callback;
    int64_t timeout_ms;
    uint32_t sc_lifetime;
    uint16_t token_target;
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
 @param pCfg
    Non null pointer to the static configuration. The content of the configuration is copied
    and the object pointed by /p pCfg can be freed by the caller.
 @return
    The operation status */
SOPC_ReturnStatus SOPC_LibSub_Initialize(const SOPC_LibSub_StaticCfg* pCfg);

/*
 @description
    Clears the connections, configurations, and clears the Toolkit. */
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
 @return
    The operation status */
SOPC_ReturnStatus SOPC_LibSub_Configured(void);

/*
 @description
    Creates a new connection to a remote OPC server from configuration id cfg_id.
    The connection represent the whole client and is later identified by the returned cli_id.
    A subscription is created and associated with this client.
 @param cfgId
    The parameters of the connection to create, return by SOPC_LibSub_ConfigureConnection().
 @param pCliId [out, not null]
    The connection id of the newly created client, set when return is SOPC_STATUS_OK.
 @return
    The operation status */
SOPC_ReturnStatus SOPC_LibSub_Connect(const SOPC_LibSub_ConfigurationId cfgId, SOPC_LibSub_ConnectionId* pCliId);

/*
 @description
    Add a variable to an existing subscription
 @param cliId
    The connection id.
 @param szNodeId
    A zero-terminated string describing the NodeId to add.
 @param pDataId [out, not null]
    The unique variable data identifier. Will be used in call to data_change_callback.
 @return
    The operation status */
SOPC_ReturnStatus SOPC_LibSub_AddToSubscription(const SOPC_LibSub_ConnectionId cliId,
                                                SOPC_LibSub_CstString* szNodeId,
                                                SOPC_LibSub_DataId* pDataId);

/*
 @description
    Disconnect from a remote OPC server.
 @param c_id
    The connection id to disconnect
 @return
    The operation status */
SOPC_ReturnStatus SOPC_LibSub_Disconnect(const SOPC_LibSub_ConnectionId c_id);

/*--------------------------------
    TBC??
    - delete_subscription
    - delete_from_subscription
--------------------------------*/

#endif /* LIBS2OPC_CLIENT_H_ */
