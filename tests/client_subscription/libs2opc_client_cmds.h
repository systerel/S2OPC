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
 * \brief Interface of a library supporting the subscription management, read/write operation and browse.
 *
 * The functions of this interface are synchronous and mutually excluded: functions wait for server's response and 2
 * functions cannot be called simultaneously.
 */

#ifndef LIBS2OPC_CLIENT_CMDS_H_
#define LIBS2OPC_CLIENT_CMDS_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "sopc_builtintypes.h"
#include "sopc_user_app_itf.h"

/* Security policies, taken from "sopc_crypto_profiles.h" */
#define SOPC_SecurityPolicy_None_URI "http://opcfoundation.org/UA/SecurityPolicy#None"
#define SOPC_SecurityPolicy_Basic256_URI "http://opcfoundation.org/UA/SecurityPolicy#Basic256"
#define SOPC_SecurityPolicy_Basic256Sha256_URI "http://opcfoundation.org/UA/SecurityPolicy#Basic256Sha256"

/*
  @description
    Callback type for data change event (related to a subscription)
  @param connectionId
    The connection id on which the datachange happened
  @param nodeId
    The node id of changed node
  @param value
    The new value of the Attribute 'Value'. Its content is freed by the LibSub after this function has been called,
    hence the callback must copy it if it should be used outside the callback. */
typedef void (*SOPC_ClientHelper_DataChangeCbk)(const int32_t connectionId,
                                                const char* nodeId,
                                                const SOPC_DataValue* value);

/*
 @description
   Connection configuration to a remote OPC server
 @field security_policy
   The chosen OPC-UA security policy for the connection, one of the SOPC_SecurityPolicy_*_URI string.
   zero-terminated string
 @field security_mode
   The chosen OPC-UA security mode for the connection.
   The list of accepted values is:
   - 1: no security mode,
   - 2: only signature,
   - 3: signature and encryption.
 @field path_cert_auth
   Zero-terminated path to the root certificate authority in the DER format
 @field path_cert_srv
   Zero-terminated path to the server certificate in the DER format, signed by the root certificate authority
 @field path_cert_cli
   Zero-terminated path to the client certificate in the DER format, signed by the root certificate authority
 @field path_key_cli
   Zero-terminated path to the client private key which is paired to the public key signed server certificate,
   in the DER format
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
*/
typedef struct
{
    const char* security_policy;
    int32_t security_mode;
    const char* path_cert_auth;
    const char* path_cert_srv;
    const char* path_cert_cli;
    const char* path_key_cli;
    const char* policyId;
    const char* username;
    const char* password;
} SOPC_ClientHelper_Security;

/*
  @description
    Structure defining a node and value to write.
    Value should be single value or one-dimensional array: value.value.ArrayType should be SingleValue or Array.
  @field nodeId
    NodeId of the Node that contains the value to write. zero-terminated string
  @field indexRange
    Used only if the attribute 'Value' is an array. Otherwise, it should be NULL.
    Index of a single value or range of value in the array.
    See NumericRange defined in the OPC UA Reference, Part 4 Chapter 7.
  @field value
    Value to write in the attribute 'Value'.
    If indexRange is specified, value should be an array (value.value.ArrayType = Array).
*/
typedef struct
{
    char* nodeId;
    char* indexRange;
    SOPC_DataValue* value;
} SOPC_ClientHelper_WriteValue;

/*
  @description
    Structure defining a node, an attribute and a value.
  @field nodeId
    NodeId of the Node that contains the attribute to read. zero-terminated string
  @field attributeId
    AttributeId of the Node that contains the value to write. 0 is not valid.
    Ids of attributes are defined in Part 6.
  @field indexRange
    Used only if the attribute 'Value' is an array. Otherwise, it should be NULL.
    Index of a single value or range of value in the array.
    See NumericRange defined in the OPC UA Reference, Part 4 Chapter 7.
  @field value
    Value to write in the attribute 'Value'.
    If indexRange is specified, value should be an array (value.value.ArrayType = Array).
*/
typedef struct
{
    char* nodeId;
    uint32_t attributeId;
    char* indexRange;
} SOPC_ClientHelper_ReadValue;

/*
  @description
    Structure defining a node, an attribute and a value.
  @field nodeId
    NodeId of the Node to be browsed. zero-terminated string
  @field direction
    The direction of References to follow.
    The list of accepted values is:
    - 0: Forward references,
    - 1: Inverse references,
    - 2: Forward and inverse references.
  @field referenceTypeId
    NodeId of the ReferenceType to follow. zero-terminated string
  @field includeSubTypes
    Indicates whether subtypes of the ReferenceType should be included.
*/
typedef struct
{
    char* nodeId;
    int32_t direction;
    char* referenceTypeId;
    bool includeSubtypes;
} SOPC_ClientHelper_BrowseRequest;

/*
  @description
    Structure defining a node, an attribute and a value.
  @field referenceTypeId
    NodeId of the ReferenceType to follow. zero-terminated string.
  @field isForward
    If True, the server follow a forward reference. Otherwise, it follow an inverse.
  @field nodeId
    ExpandedNodeId (see OPC Unified Architecture, Part 4) of the target node following the Reference defined by the
  returned ReferenceTypeId.
  @field browseName
    BrowseName of the target node. zero-terminated string or NULL.
  @field DisplayName
    DisplayName of the target node. zero-terminated string.
  @field nodeClass
    NodeClass identifier of the target node.
  @field type
    Type definition of target node. Used only if NodeClass is Object or Variable. Otherwise, it is NULL.
*/
typedef struct
{
    char* referenceTypeId;
    bool isForward;
    char* nodeId;
    char* browseName;
    char* displayName;
    int32_t nodeClass;
} SOPC_ClientHelper_BrowseResultReference;

/*
 * @description
 *   structure containing the result of a browse request
 * @field statusCode
 *   status code of the browse operation
 * @field NbOfReferences
 *   number of references
 * @field references
 *   references return by the browse request
 */
typedef struct
{
    SOPC_ReturnStatus statusCode;
    int32_t nbOfReferences;
    SOPC_ClientHelper_BrowseResultReference* references;
} SOPC_ClientHelper_BrowseResult;

/*
 @description
    Configure the library. This function shall be called once by the host application
    before any other service can be used.
 @param log_path
    Absolute or relative path of the directory to be used for logs. It will contain all created log files.
    Shall be terminated by a directory separator.
    If given directory does not exist, it will be created by S2OP_ClientHelper
    If log_path is NULL, './logs/' is used by default.
    Value shall be NULL or zero-terminated string.
    The content of this String is copied the object pointed by /p log_path can be freed by the caller
 @param log_level
    Minimum level of log traces to be printed in the log files
    The list of accepted values is:
    - 0: ERROR,
    - 1: WARNING,
    - 2: INFO,
    - 3: DEBUG.
    This list is decreasing. It means ERROR level is the maximum value and DEBUG level is the minimum value.
    If level is not an accepted value, DEBUG is used by default.
 @return
   '0' if operation succeed
   '-1' if path is not valid
   '-2' if toolkit not initialized
 */
int32_t SOPC_ClientHelper_Initialize(const char* log_path, int32_t log_level);

/*
 @description
    Clears the connections, configurations, and clears the Toolkit.
 @warning
    As this function should be called only once, it is not threadsafe. */
void SOPC_ClientHelper_Finalize(void);

/*
 @description
    Creates a new connection to a remote OPC server.
    The connection represent the whole client and is later identified by the returned connectionId.
    The function waits until the client is effectively connected or the Toolkit times out.
    Return a connection id or error code.
    All parameters are copied and can be freed by the caller.
 @param endpointUrl
   Zero-terminated path to server URL
 @return
    If this operation succeeded, return a connection id > 0.
    If invalid endpointUrl detected, return -1.
    If invalid security detected, return -<10+n> with <n> field number (starting from 1).
    If connection failed, return '-100'.
 */
int32_t SOPC_ClientHelper_Connect(const char* endpointUrl, SOPC_ClientHelper_Security security);

/*
 @description
    Create a subscription associated to the given connection
    The given callback will be called on data changes.

    Only attributes "Value" are monitored.
    The function waits until the subscription is effectively created or the Toolkit times out.
    Returns 0 if succeeded, otherwise an error code < 0.
    This function can be called only once time for a valid connection id or user should call
 SOPC_ClientHelper_unsubscribe() before.
 @param connectionId
    The connection id. It should be > 0
 @param callback
   The callback for data change notification. Should not be null.
 @return
   '0' if operation succeed
   '-1' if connectionId not valid
   '-2' if the data change callback associated to connectionId is NULL
   '-100' if operation failed
*/
int32_t SOPC_ClientHelper_CreateSubscription(int32_t connectionId, SOPC_ClientHelper_DataChangeCbk callback);

/*
 @description
    Adds monitored items to the subscription associated to the given connection.
    SOPC_ClientHelper_CreateSubscription() should have been called previously.

    Only attributes "Value" are monitored.
    The function waits until the monitored items are effectively created or the Toolkit times out.
    Returns 0 if succeeded, otherwise an error code < 0.

 @param connectionId
    The connection id. It should be > 0
 @param nodeIds
    An array of zero-terminated strings describing the NodeIds to add.
    It should be not NULL and be at least \p nbNodeIds long.
    See OPC Unified Architecture, Part 3 for NodeId description.
 @param nbNodeIds
    Number of elements to subscribes. It should be between 1 and INT32_MAX
 @return
   '0' if operation succeed
   '-1' if connectionId not valid
   '-2' if nodeIds or nbNodeIds not valid
   '-<3+index>' if nodeIds[index] is not valid
   '-100' if operation failed
*/
int32_t SOPC_ClientHelper_AddMonitoredItems(int32_t connectionId, char** nodeIds, size_t nbNodeIds);

/*
 @description
    Delete subscription associated to the given connection.
    If this function succeed, no more data changes notification is received about this connection until
    SOPC_ClientHelper_CreateSubscription() and SOPC_ClientHelper_AddMonitorItems() are called.

 @param connectionId
    The connection id. It should be > 0
 @return
   '0' if operation succeed
   '-1' if connectionId not valid
   '-100' if operation failed
*/
int32_t SOPC_ClientHelper_Unsubscribe(int32_t connectionId);

/*
 @description
    Disconnect from a remote OPC server.
    The function waits until the client is effectively disconnected, or the Toolkit times out.
 @param c_id
    The connection id to disconnect
 @return
   '0' if operation succeed
   '-1' if connectionId not valid
   '-2' if toolkit uninitialized
   '-3' if already closed connection
   '-100' if operation failed
*/
int32_t SOPC_ClientHelper_Disconnect(int32_t connectionId);

/*
 @description
    Write values to attributes "Value" of one or more Nodes.
    This function waits for the server response, or the Toolkit times out.

    Restrictions:
    - Only single value or one-dimensional array can be written with this function.
 @param connectionId
    The connection id. It should be > 0
 @param writeValues
    An array of Writevalue describing the NodeIds and the Values to write.
    Its content can be freed after the function returned.
    It should be not NULL and be at least \p nbElements long.
 @param nbElements
    Number of elements to write. It should be between 1 and INT32_MAX
 @param writeResults
    A pre-allocated array of StatusCode, filled with result for each write value
    It should not be NULL and be at least \p nbElements long.
 @return
   '0' if operation succeed
   '-1' if connectionId not valid
   '-2' if writeValues or nbElements not valid
   '-3' if writeResults is not valid
   '-100' if operation failed
*/
int32_t SOPC_ClientHelper_Write(int32_t connectionId,
                                SOPC_ClientHelper_WriteValue* writeValues,
                                size_t nbElements,
                                SOPC_StatusCode* writeResults);

/*
 @description
    Read one or more attributes of one or more Nodes.
    Return both the source and Server timestamps for each requested Variable Value Attribute.

    Restrictions:
    - The service parameter 'maxAge' is not managed.
    - If DataEncoding should apply, Binary encoding is used.
 @param connectionId
    The connection id. It should be > 0
 @param readValues
    An array of ReadValue describing the NodeIds and AttributesId to read.
    Its content can be freed after the function returned.
    It should be not NULL and be at least \p nbElements long.
 @param nbElements
    Number of elements to write. It should be between 1 and INT32_MAX
 @param values [out, not null]
    A pre-allocated array to the output list of pointers of Attribute values.
    It should be at least \p nElements long.
    When return, the order of this list matches the order of \p readValues.
    The ownership of the data moved to caller which should freed the content of this array,
    i.e. user should free all attribute values
 @return
   '0' if operation succeed
   '-1' if connectionId not valid
   '-2' if readValues, values or nbElements not valid
   '-3' if values is not valid
   '-<4+index>' if readValues[index].nodeId is invalid
   '-100' if operation failed
*/
int32_t SOPC_ClientHelper_Read(int32_t connectionId,
                               SOPC_ClientHelper_ReadValue* readValues,
                               size_t nbElements,
                               SOPC_DataValue** values);

/*
 @description
    Discover the references of a Node using Browse and browseNext services.
    If Browse Request returns ContinuationPoint, a BrowseNext Request is sent until no ContinuationPoint is returned.

    Restrictions:
    - Views are not managed
    - requestedMaxReferencesPerNode is set to 0
    - nodeClassMask is set to 0 (object)
    - resultMask specifies all fields are returned
    - browse cannot be called several times simultaneously

 @param connectionId
    The connection id. It should be > 0
 @param browseRequests
    An array of BrowseRequest describing the NodeIds to browse and ReferenceType to follow.
    Its content can be freed after the function returned.
    It should be not NULL and be at least \p nbElements long.
 @param nbElements
    Number of elements to browse. It should be between 1 and INT32_MAX
 @param browseResults [out, not null]
    A pre-allocated array to the output list of BrowseResult.
    It should be at least \p nElements long.
    When return, the order of this list matches the order of \p readValues.
    The ownership of the data moved to caller which should free the content of this array.

 @return
   '0' if operation succeed
   '-1' if connectionId not valid
   '-2' if browseRequest or nbElements not valid
   '-3' if browseResults  not valid
   '-4' too many calls to BrowseNext without finishing the request
   '-100' if operation failed
*/
int32_t SOPC_ClientHelper_Browse(int32_t connectionId,
                                 SOPC_ClientHelper_BrowseRequest* browseRequests,
                                 size_t nbElements,
                                 SOPC_ClientHelper_BrowseResult* browseResults);

#endif /* LIBS2OPC_CLIENT_CMDS_H_ */
