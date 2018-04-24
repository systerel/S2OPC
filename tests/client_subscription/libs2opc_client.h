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

/*
 =================
 TYPES DEFINITION
 ================= */

// C Const String type
typedef const char* SOPC_LibSub_CstString;

// C String type
typedef char* SOPC_LibSub_String;

// Fixed size integers
#include <stdint.h>
#if 0
typedef signed long long int int64_t;
typedef unsigned long long int uint64_t;
typedef unsigned int uint32_t;
#if __cplusplus
static_assert(sizeof(int64_t) == 8, "Invalid int64_t definition");
static_assert(sizeof(uint64_t) == 8, "Invalid uint64_t definition");
static_assert(sizeof(uint32_t) == 4, "Invalid uint32_t definition");
#endif
#endif

// Connection identifier
typedef int64_t SOPC_LibSub_ConnectionId;

// Data identifier (used for subscription change notification)
typedef uint32_t SOPC_LibSub_DataId;

/*
  Data value quality
  TBD? Masks for BAD and UNCERTAIN quality? */
typedef uint32_t SOPC_StatusCode;

// Timestamp (NTP Format)
typedef uint64_t SOPC_LibSub_Timestamp;

// Data value type
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

typedef struct
{
    SOPC_LibSub_DataType type;
    SOPC_StatusCode quality;
    void* value;
    SOPC_LibSub_Timestamp source_timestamp;
    SOPC_LibSub_Timestamp server_timestamp;
} SOPC_LibSub_Value;

/*
 ===================
 SERVICES DEFINITION
 =================== */
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
 @description
   Identification / encryption configuration
 @field username
   Username. NULL for anonymous access
 @field password
   Password. No significant when username is NULL*/
// TBC...
typedef struct
{
    SOPC_LibSub_CstString username;
    SOPC_LibSub_CstString password;
} SOPC_LibSub_UserIdCfg;

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
    // TODO : configuration des jetons?
} SOPC_LibSub_StaticCfg;

/*
 @description
   Connection configuration to a remote OPC server
 @field server_url
   Path to server URL
 @field timeout_ms
   Connection timeout (milliseconds)
 @field identification_cfg
   Encryption, identification configuration */
typedef struct
{
    SOPC_LibSub_CstString server_url;
    int64_t timeout_ms;
    SOPC_LibSub_UserIdCfg identification_cfg;
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
    non null pointer to the static configuration
 @return
    The operation status */
SOPC_ReturnStatus SOPC_LibSub_Initialize(const SOPC_LibSub_StaticCfg* pCfg);

/*
 @description
    Configure a future connection. This function shall be called once per connection
    before a call to SOPC_LibSub_Configured().
 @param pCfg
    non null pointer to the static configuration.
 @param c_id [out, not null]
    The connection id. Set when the value returned is "SOPC_no_error".
 @return
    The operation status */
SOPC_ReturnStatus SOPC_LibSub_ConfigureConnection(const SOPC_LibSub_ConnectionCfg* pCfg,
                                                  SOPC_LibSub_ConnectionId* c_id);

/*
 @description
    Mark the library as configured. All calls to s2opc_clilent_configure_connection() shall
    be done prior to calling this function. All calls to SOPC_LibSub_Connect shall be done
    after calling this function.
 @return
    the operation status */
SOPC_ReturnStatus SOPC_LibSub_Configured(void);

/*
 @description
    Connect the client with id c_id to a remote OPC server, and create a subscription.
 @param c_id
    A connection id return by a call to SOPC_.
 @param publish_period_ms
    The requestes publish period (in milliseconds)
 @param data_change_callback
    The callback for data change notification
 @return
    The operation status */
SOPC_ReturnStatus SOPC_LibSub_Connect(const SOPC_LibSub_ConnectionId c_id,
                                      const int64_t publish_period_ms,
                                      SOPC_LibSub_DataChangeCbk data_change_callback);

/*
 @description
    Add a variable to an existing subscription
 @param c_id
    The connection id.
 @param d_id [out, not null]
    The unique variable data identifier. Will be used in call to data_change_callback.
 @return
    The operation status */
SOPC_ReturnStatus SOPC_LibSub_AddToSubscription(const SOPC_LibSub_ConnectionId c_id, SOPC_LibSub_DataId* d_id);

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
