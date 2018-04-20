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
     g++ -c -fdump-ada-spec -C libs2opc_client.h
  gcc also works but loses parameter names in function prototypes, that is why the g++ and 'extern "C"' are used.
*/

/*
 =================
 TYPES DEFINITION
 ================= */

// C Const String type
typedef const char* cst_string_t;

// C String type
typedef char* string_t;

// 64 bits signed integer
typedef signed long long int64_t;

// Connection identifier
typedef signed long long s2opc_client_connection_id_t;

// Data identifier (used for subscription change notification)
typedef unsigned int s2opc_client_data_id_t;

/*
  Data value quality
  TBD? Masks for BAD and UNCERTAIN quality? */
typedef unsigned int s2opc_client_data_quality_t;

// Timestamp (NTP Format)
typedef unsigned long long s2opc_client_timestamp_t;

// Data value type
enum s2opc_client_data_type_t
{
    s2opc_client_type_bool = 1,
    s2opc_client_type_integer = 2,
    s2opc_client_type_string = 3,
    s2opc_client_type_bytestring = 4
};

/*
  @description
    Structure defining the value of a node
  @field quality
    The value quality.
  @field type
    The value type. Specifies the type of '*value' amongst:
    - cst_string_t (s2opc_client_type_string / s2opc_client_type_bytestring)
    - int64_t (s2opc_client_type_bool / s2opc_client_type_integer)
*/
typedef struct
{
    s2opc_client_data_quality_t quality;
    s2opc_client_data_type_t type;
    void* value;
} s2opc_client_data_value_t;

// Result
enum s2opc_client_result_t
{
    s2opc_client_no_error = 1,
    s2opc_client_timeout = 2,
    s2opc_client_failure = 3
};

/*
  Log levels */
enum s2opc_client_log_level_t
{
    s2opc_client_log_error = 1,
    s2opc_client_log_warning = 2,
    s2opc_client_log_info = 3,
    s2opc_client_log_debug = 4
};

/*
 ===================
 SERVICES DEFINITION
 =================== */
/*
  @description
    Log callback type
  @param log_level
    The Log level (s2opc_client_log_level_t). Note: s2opc_client_log_error shall be non-returning.
  @param text
    The text string to log (shall not be null) */
typedef void (*log_callback_t)(const s2opc_client_log_level_t log_level, cst_string_t text);

/*
  @description
    Callback type for disconnect event
  @param c_id
    The connection id that has been disconnected */
typedef void (*disconnect_callback_t)(const s2opc_client_connection_id_t c_id);

/*
  @description
    Callback type for data change event (related to a subscription)
  @param c_id
    The connection id that has been disconnected */
typedef void (*data_change_callback_t)(const s2opc_client_connection_id_t c_id,
                                       const s2opc_client_data_id_t d_id,
                                       const s2opc_client_data_value_t* value,
                                       const s2opc_client_timestamp_t source_timestamp,
                                       const s2opc_client_timestamp_t server_timestamp);

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
    cst_string_t username;
    cst_string_t password;
} s2opc_client_identification_cfg_t;

/*
 @description
   Static configuration of OPC client libray
 @field host_log_callback
   Host log callback
 @field disconnect_callback_t
   Notification event for disconnection from server */
typedef struct
{
    log_callback_t host_log_callback;
    disconnect_callback_t disconnect_callback;
    // TODO : configuration des jetons?
} s2opc_client_static_cfg_t;

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
    cst_string_t server_url;
    int64_t timeout_ms;
    s2opc_client_identification_cfg_t identification_cfg;
} s2opc_client_connect_cfg_t;

/*
 ===================
 SERVICES DEFINITION
 =================== */

/*
    Return the current version library
*/
cst_string_t s2opc_client_getVersion(void);

/*
 @description
    Configure the library. This function shall be called once by the host application
    before any other service can be used.
 @param pCfg
    non null pointer to the static configuration
 @return
    The operation status */
s2opc_client_result_t s2opc_client_initialize(const s2opc_client_static_cfg_t* pCfg);

/*
 @description
    Connect to a remote OPC server.
 @param c_id [out, not null]
    The connection id. Set when the value returned is "s2opc_client_no_error"
 @return
    The operation status */
s2opc_client_result_t s2opc_client_connect(const s2opc_client_connect_cfg_t* pCfg, s2opc_client_connection_id_t* c_id);

/*
 @description
    Create a subscription on a connection
 @param c_id
    The connection id.
 @param publish_period_msid
    The requestes publish period (in milliseconds)
 @param data_change_callback
    The callback for data change notification
 @return
    The operation status */
s2opc_client_result_t s2opc_client_create_subscription(const s2opc_client_connection_id_t c_id,
                                                       const int64_t publish_period_ms,
                                                       data_change_callback_t data_change_callback);

/*
 @description
    Add a variable to an existing subscription
 @param c_id
    The connection id.
 @param d_id [out, not null]
    The variable data identifier. Will be used in call to data_change_callback.
 @return
    The operation status */
s2opc_client_result_t s2opc_client_add_to_subscription(const s2opc_client_connection_id_t c_id,
                                                       s2opc_client_data_id_t* d_id);

/*
 @description
    Disconnect from a remote OPC server.
 @param c_id
    The connection id to disconnect
 @return
    The operation status */
s2opc_client_result_t s2opc_client_disconnect(const s2opc_client_connection_id_t c_id);

/*--------------------------------
    TBC??
    - delete_subscription
    - delete_from_subscription
--------------------------------*/

#endif /* LIBS2OPC_CLIENT_H_ */
