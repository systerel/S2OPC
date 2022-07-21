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
#include "sopc_crypto_profiles.h"
#include "sopc_enum_types.h"
#include "sopc_log_manager.h"
#include "sopc_user_app_itf.h"

/**
  @brief
    Callback type for data change event (related to a subscription)
  @warning
    No blocking operation shall be done in callback

  @param connectionId
    The connection id on which the datachange happened
  @param nodeId
    The node id of changed node
  @param value
    The new value of the Attribute 'Value'. Its content is freed by the LibSub after this function has been called,
    hence the callback must copy it if it should be used outside the callback. */
typedef void SOPC_ClientHelper_DataChangeCbk(const int32_t connectionId,
                                             const char* nodeId,
                                             const SOPC_DataValue* value);

/**
 * @brief
 *   Callback type for disconnection.
 * @warning
 *   No blocking operation shall be done in callback
 *
 * @param connectionId
 *   The disconnected connection id
 */
typedef void SOPC_ClientHelper_DisconnectCbk(const uint32_t connectionId);

/**
 @struct SOPC_ClientHelper_Security
 @brief
   Connection configuration to a remote OPC server
 @var SOPC_ClientHelper_Security::security_policy
   The chosen OPC-UA security policy for the connection, one of the SOPC_SecurityPolicy_*_URI string from
 "sopc_crypto_profiles.h". zero-terminated string
 @var SOPC_ClientHelper_Security::security_mode
   The chosen OPC-UA security mode for the connection.
   The list of accepted values is one of the following OpcUa_MessageSecurityMode values:
   - OpcUa_MessageSecurityMode_None: no security mode,
   - OpcUa_MessageSecurityMode_Sign: only signature,
   - OpcUa_MessageSecurityMode_SignAndEncrypt: signature and encryption.
 @var SOPC_ClientHelper_Security::path_cert_auth
   Zero-terminated path to the root certificate authority in the DER format
 @var SOPC_ClientHelper_Security::path_crl
   Zero-terminated path to the Certificate Revocation List (CRL) of the certificate authority in the DER format
 @var SOPC_ClientHelper_Security::path_cert_srv
   Zero-terminated path to the server certificate in the DER format, signed by the root certificate authority
 @var SOPC_ClientHelper_Security::path_cert_cli
   Zero-terminated path to the client certificate in the DER format, signed by the root certificate authority
 @var SOPC_ClientHelper_Security::path_key_cli
   Zero-terminated path to the client private key which is paired to the public key signed client certificate,
   in the DER format
 @var SOPC_ClientHelper_Security::policyId
   Zero-terminated policy id. To know which policy id to use, please read a
   GetEndpointsResponse or a CreateSessionResponse. When username is NULL, the
   AnonymousIdentityToken is used and the policy id must correspond to an
   anonymous UserIdentityPolicy. Otherwise, the UserNameIdentityToken is used
   and the policy id must correspond to an username UserIdentityPolicy.
 @var SOPC_ClientHelper_Security::username
   Zero-terminated username, NULL for anonymous access, see policyId
   The password will be encrypted, or not, depending on the user token security policy associated to the policyId
   or if it is empty depending on the SecureChannel security policy.
 @var SOPC_ClientHelper_Security::password
   Zero-terminated password, ignored when username is NULL. Password is kept in memory for future reconnections.
*/
typedef struct
{
    const char* security_policy;
    OpcUa_MessageSecurityMode security_mode;
    const char* path_cert_auth;
    const char* path_crl;
    const char* path_cert_srv;
    const char* path_cert_cli;
    const char* path_key_cli;
    const char* policyId;
    const char* username;
    const char* password;
} SOPC_ClientHelper_Security;

/**
  @struct SOPC_ClientHelper_WriteValue
  @brief
    Structure defining a node and value to write.
    Value should be single value or one-dimensional array: value.value.ArrayType should be SingleValue or Array.
  @var SOPC_ClientHelper_WriteValue::nodeId
    NodeId of the Node that contains the value to write. zero-terminated string
  @var SOPC_ClientHelper_WriteValue::indexRange
    Used only if the attribute 'Value' is an array. Otherwise, it should be NULL.
    Index of a single value or range of value in the array.
    See NumericRange defined in the OPC UA Reference, Part 4 Chapter 7.
  @var SOPC_ClientHelper_WriteValue::value
    Value to write in the attribute 'Value'.
    If indexRange is specified, value should be an array (value.value.ArrayType = Array).
*/
typedef struct
{
    const char* nodeId;
    const char* indexRange;
    SOPC_DataValue* value;
} SOPC_ClientHelper_WriteValue;

/**
  @struct SOPC_ClientHelper_ReadValue
  @brief
    Structure defining a node, an attribute.
  @var SOPC_ClientHelper_ReadValue::nodeId
    NodeId of the Node that contains the attribute to read. zero-terminated string
  @var SOPC_ClientHelper_ReadValue::attributeId
    AttributeId of the Node that contains the value to read. 0 is not valid.
    Ids of attributes are defined in Part 6.
  @var SOPC_ClientHelper_ReadValue::indexRange
    Used only if the attribute 'Value' is an array. Otherwise, it should be NULL.
    Index of a single value or range of value in the array.
    See NumericRange defined in the OPC UA Reference, Part 4 Chapter 7.
*/
typedef struct
{
    const char* nodeId;
    uint32_t attributeId;
    const char* indexRange;
} SOPC_ClientHelper_ReadValue;

/**
  @struct SOPC_ClientHelper_BrowseRequest
  @brief
    Structure defining a node, an attribute and a value.
  @var SOPC_ClientHelper_BrowseRequest::nodeId
    NodeId of the Node to be browsed. zero-terminated string
  @var SOPC_ClientHelper_BrowseRequest::direction
    The direction of References to follow.
    The list of accepted values is one of the following OpcUa_BrowseDirection values:
    - OpcUa_BrowseDirection_Forward: Forward references,
    - OpcUa_BrowseDirection_Inverse: Inverse references,
    - OpcUa_BrowseDirection_Both: Forward and inverse references.
  @var SOPC_ClientHelper_BrowseRequest::referenceTypeId
    NodeId of the ReferenceType to follow. zero-terminated string
  @var SOPC_ClientHelper_BrowseRequest::includeSubtypes
    Indicates whether subtypes of the ReferenceType should be included.
*/
typedef struct
{
    const char* nodeId;
    OpcUa_BrowseDirection direction;
    const char* referenceTypeId;
    bool includeSubtypes;
} SOPC_ClientHelper_BrowseRequest;

/**
  @struct SOPC_ClientHelper_BrowseResultReference
  @brief
    Structure defining a node, an attribute and a value.
  @var SOPC_ClientHelper_BrowseResultReference::referenceTypeId
    NodeId of the ReferenceType to follow. zero-terminated string.
  @var SOPC_ClientHelper_BrowseResultReference::isForward
    If True, the server follow a forward reference. Otherwise, it follow an inverse.
  @var SOPC_ClientHelper_BrowseResultReference::nodeId
    ExpandedNodeId (see OPC Unified Architecture, Part 4) of the target node following the Reference defined by the
  returned ReferenceTypeId.
  @var SOPC_ClientHelper_BrowseResultReference::browseName
    BrowseName of the target node. zero-terminated string or NULL.
  @var SOPC_ClientHelper_BrowseResultReference::displayName
    DisplayName of the target node. zero-terminated string.
  @var SOPC_ClientHelper_BrowseResultReference::nodeClass
    NodeClass identifier of the target node.
*/
typedef struct
{
    char* referenceTypeId;
    bool isForward;
    char* nodeId;
    char* browseName;
    char* displayName;
    OpcUa_NodeClass nodeClass;
} SOPC_ClientHelper_BrowseResultReference;

/**
 * @struct SOPC_ClientHelper_BrowseResult
 * @brief
 *   structure containing the result of a browse request
 * @var SOPC_ClientHelper_BrowseResult::statusCode
 *   status code of the browse operation
 * @var SOPC_ClientHelper_BrowseResult::nbOfReferences
 *   number of references
 * @var SOPC_ClientHelper_BrowseResult::references
 *   references return by the browse request
 */
typedef struct
{
    SOPC_ReturnStatus statusCode;
    int32_t nbOfReferences;
    SOPC_ClientHelper_BrowseResultReference* references;
} SOPC_ClientHelper_BrowseResult;

/**
 @brief
    Move the content of the SOPC_ClientHelper_BrowseResultReference from source to destination.
    Note: after use of move function, clearing source has no effect on destination structure content
          since source does not point to allocated data anymore.

 @param dest  A pointer to the SOPC_ClientHelper_BrowseResultReference structure to use as destination
 @param src   A pointer to the SOPC_ClientHelper_BrowseResultReference structure to use as source
 */
void SOPC_ClientHelper_BrowseResultReference_Move(SOPC_ClientHelper_BrowseResultReference* dest,
                                                  SOPC_ClientHelper_BrowseResultReference* src);
/**
 @brief
    Clears the content of the SOPC_ClientHelper_BrowseResultReference structure pointed by \p brr

 @param brr  A pointer to the SOPC_ClientHelper_BrowseResultReference structure to clear.
 */
void SOPC_ClientHelper_BrowseResultReference_Clear(SOPC_ClientHelper_BrowseResultReference* brr);

/**
 @brief
    Clears the content of the SOPC_ClientHelper_BrowseResult[] \p results.
    The array is not deallocated since it is managed by the caller of SOPC_ClientHelper_Browse.

    SOPC_ClientHelper_BrowseResultReference_Move might be use to keep partial result before calling this function.

 @param nbElements  The number of elements in the browse result array
 @param results     The SOPC_ClientHelper_BrowseResult array to clear.
 */
void SOPC_ClientHelper_BrowseResults_Clear(size_t nbElements, SOPC_ClientHelper_BrowseResult* results);

/**
 * @struct SOPC_ClientHelper_CallMethodRequest
 * @brief
 *   structure containing the requested method to call and input parameters
 * @var SOPC_ClientHelper_CallMethodRequest::objectNodeId
 *   nodeId of the object on which method is called
 * @var SOPC_ClientHelper_CallMethodRequest::methodNodeId
 *   nodeId of the called method
 * @var SOPC_ClientHelper_CallMethodRequest::nbOfInputParams
 *   number of input parameters provided in \p SOPC_ClientHelper_CallMethodRequest::inputParams
 * @var SOPC_ClientHelper_CallMethodRequest::inputParams
 *   ordered input parameters values for the the method called (array of SOPC_Variant)
 */
typedef struct
{
    char* objectNodeId;
    char* methodNodeId;
    int32_t nbOfInputParams;
    SOPC_Variant* inputParams;
} SOPC_ClientHelper_CallMethodRequest;

/**
 * @struct SOPC_ClientHelper_CallMethodResult
 * @brief
 *   structure containing the result of a method call
 * @var SOPC_ClientHelper_CallMethodResult::status
 *   status of the method call
 * @var SOPC_ClientHelper_CallMethodResult::nbOfOutputParams
 *   number of output parameters provided in \p SOPC_ClientHelper_CallMethodResult::outputParams
 * @var SOPC_ClientHelper_CallMethodResult::outputParams
 *   ordered output parameters values as result of method call (SOPC_Variant array)
 */
typedef struct
{
    SOPC_StatusCode status;
    int32_t nbOfOutputParams;
    SOPC_Variant* outputParams;
} SOPC_ClientHelper_CallMethodResult;

/**
 * @struct SOPC_ClientHelper_UserIdentityToken
 * @brief
 *   structure containing a user identity token
 * @var SOPC_ClientHelper_UserIdentityToken::policyId
 *   policy id
 * @var SOPC_ClientHelper_UserIdentityToken::tokenType
 *   The list of accepted values is one of the following OpcUa_UserTokenType values:
 *    - OpcUa_UserTokenType_Anonymous: anonymous
 *    - OpcUa_UserTokenType_UserName: username
 *    - OpcUa_UserTokenType_Certificate: certificate
 *    - OpcUa_UserTokenType_IssuedToken: IssuedToken
 *    - OpcUa_UserTokenType_Kerberos: Kerberos
 * @var SOPC_ClientHelper_UserIdentityToken::issuedTokenType
 *   name of the token type
 * @var SOPC_ClientHelper_UserIdentityToken::issuerEndpointUrl
 *   endpoint Url of the issuer
 * @var SOPC_ClientHelper_UserIdentityToken::securityPolicyUri
 *   Uri of the security policy
 */
typedef struct
{
    char* policyId;
    OpcUa_UserTokenType tokenType;
    char* issuedTokenType;
    char* issuerEndpointUrl;
    char* securityPolicyUri;
} SOPC_ClientHelper_UserIdentityToken;

/**
 * @struct SOPC_ClientHelper_EndpointDescription
 * @brief
 *   structure containing an endpoint description
 * @var SOPC_ClientHelper_EndpointDescription::endpointUrl
 *   url of the endpoint
 * @var SOPC_ClientHelper_EndpointDescription::security_mode
 *   the security mode of the endpoint:
 *   The list of accepted values is one of the following OpcUa_MessageSecurityMode values:
 *    - OpcUa_MessageSecurityMode_None: no security mode,
 *    - OpcUa_MessageSecurityMode_Sign: only signature,
 *    - OpcUa_MessageSecurityMode_SignAndEncrypt: signature and encryption.
 * @var SOPC_ClientHelper_EndpointDescription::security_policyUri
 *   Uri of the security policy
 * @var SOPC_ClientHelper_EndpointDescription::nbOfUserIdentityTokens
 *   The number of user identity tokens in userIdentityTokens array
 * @var SOPC_ClientHelper_EndpointDescription::userIdentityTokens
 *   The array containing user identity tokens
 * @var SOPC_ClientHelper_EndpointDescription::transportProfileUri
 *   Uri of the transport profile
 * @var SOPC_ClientHelper_EndpointDescription::securityLevel
 *   the security level of the endpoint relative to other available endpoints
 *   on the server (the higher the better)
 * @var SOPC_ClientHelper_EndpointDescription::serverCertificateNbBytes
 *   the number of bytes in the server certificate
 * @var SOPC_ClientHelper_EndpointDescription::serverCertificate
 *   the server certificate bytes
 */
typedef struct
{
    char* endpointUrl;
    int32_t security_mode;
    char* security_policyUri;
    int32_t nbOfUserIdentityTokens;
    SOPC_ClientHelper_UserIdentityToken* userIdentityTokens;
    char* transportProfileUri;
    int32_t securityLevel;
    int32_t serverCertificateNbBytes;
    uint8_t* serverCertificate;
} SOPC_ClientHelper_EndpointDescription;

/**
 * @struct SOPC_ClientHelper_GetEndpointsResult
 * @brief
 *   structure containing the result of a GetEndpoints request
 *
 * @var SOPC_ClientHelper_GetEndpointsResult::nbOfEndpoints
 *   the number of endpoints in the endpoints array
 * @var SOPC_ClientHelper_GetEndpointsResult::endpoints
 *   array of endpoints
 */
typedef struct
{
    int32_t nbOfEndpoints;
    SOPC_ClientHelper_EndpointDescription* endpoints;
} SOPC_ClientHelper_GetEndpointsResult;

/**
 @brief
    Configure the library. This function shall be called once by the host application
    before any other service can be used.
    It shall be done after a call to ::SOPC_CommonHelper_Initialize
 @param disconnect_callback
    Optional, can be NULL. Callback called when a connection is disconnected.
 @return
   '0' if operation succeed
   '-1' if allocation / local init failure
   '-2' if toolkit not initialized
   '-100' if toolkit already initialized
 */
int32_t SOPC_ClientHelper_Initialize(SOPC_ClientHelper_DisconnectCbk* const disconnect_callback);

/**
 @brief
    Clears the connections and configurations.
    It shall be done before a call to ::SOPC_CommonHelper_Clear
 @warning
    As this function should be called only once, it is not threadsafe. */
void SOPC_ClientHelper_Finalize(void);

/**
 * @brief
 *   Sends a GetEndpoints request to the endpointUrl and provide the results
 * @param endpointUrl
 *   Url of the endpoint
 * @param result
 *   result of the request, shall not be used if function result is not 0.
 * @return
 *   multiple error codes:
 *    - (0): everything is OK.
 *    - (-1): endpointUrl is NULL.
 *    - (-2): result is NULL.
 *    - (-100): the request failed.
 * @note
 *   results content is dynamically allocated. It is up to the caller to free
 *   this memory.
 */
int32_t SOPC_ClientHelper_GetEndpoints(const char* endpointUrl, SOPC_ClientHelper_GetEndpointsResult** result);

/**
 @brief Free the get endpoints result and its content.
        Make copy or move data to be kept prior to call this function.

 @param getEpResult  The ::SOPC_ClientHelper_GetEndpointsResult structure to clear
*/
void SOPC_ClientHelper_GetEndpointsResult_Free(SOPC_ClientHelper_GetEndpointsResult** getEpResult);

/**
 @brief
    Creates a new configuration to connect to a remote OPC server.
    Return a configuration id or error code.
    All parameters are copied and can be freed by the caller.
 @param endpointUrl
   Zero-terminated path to server URL
 @param security
    security configuration to use (policy, mode, certificates, ...)
 @param expectedEndpoints
    Response returned by prior call to GetEndpoints service
    and checked to be the same during session establishment,
    NULL otherwise (no verification will be done)
 @return
    If this operation succeeded, return a configuration id \verbatim>\endverbatim 0.
    If invalid endpointUrl detected, return -1.
    If invalid security detected, return \verbatim-<10+n>\endverbatim with \verbatim<n>\endverbatim field number
    (starting from 1). If configuration failed, return '-100'.
 */
int32_t SOPC_ClientHelper_CreateConfiguration(const char* endpointUrl,
                                              SOPC_ClientHelper_Security* security,
                                              OpcUa_GetEndpointsResponse* expectedEndpoints);

/**
 @brief
    Creates a new connection to a remote OPC server.
    The connection represent the whole client and is later identified by the returned connectionId.
    The function waits until the client is effectively connected or the Toolkit times out.
    Return a connection id or error code.
 @param cfg_id
   Configuration Id created by SOPC_ClientHelper_CreateConfiguration()
 @return
    If this operation succeeded, return a connection id > 0.
    If invalid configuration detected, return -1.
    If connection failed, return '-100'.
 @warning
    The disconnect callback might be called before the function returns if connection succeeds and then fails
    immediately (in this case the connection id used in callback is not yet returned)
 */
int32_t SOPC_ClientHelper_CreateConnection(int32_t cfg_id);

/**
 @brief
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
int32_t SOPC_ClientHelper_CreateSubscription(int32_t connectionId, SOPC_ClientHelper_DataChangeCbk* callback);

/**
 @brief
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
 @param[out] results
    Pre-allocated array of status codes indicating the add MonitoredItem operation result.
    It shall have the same size and order than in \p nodeIds array.
    It might be NULL if the caller does not need this information.
    Otherwise it is relevant only if return value is >= 0.
 @return
   '0' if operation succeed
   'N > 0' if N MonitoredItem failed to be created in server (see results)
   '-1' if connectionId not valid
   '-2' if nodeIds or nbNodeIds not valid
   '-<3+index>' if nodeIds[index] is not valid
   '-100' if operation failed
*/
int32_t SOPC_ClientHelper_AddMonitoredItems(int32_t connectionId,
                                            char** nodeIds,
                                            size_t nbNodeIds,
                                            SOPC_StatusCode* results);

/**
 @brief
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

/**
 @brief
    Disconnect from a remote OPC server.
    The function waits until the client is effectively disconnected, or the Toolkit times out.
 @param connectionId
    The connection id to disconnect
 @return
   '0' if operation succeed
   '-1' if connectionId not valid
   '-2' if toolkit uninitialized
   '-3' if already closed connection
   '-100' if operation failed
*/
int32_t SOPC_ClientHelper_Disconnect(int32_t connectionId);

/**
 @brief
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

/**
 @brief
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
    Number of elements to read. It should be between 1 and INT32_MAX
 @param[out] values
    A pre-allocated array of SOPC_DataValue, filled with the result of each read value.
    It should not be NULL and be at least \p nbElements long.
    When return, the order of this list matches the order of \p readValues.

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
                               SOPC_DataValue* values);
/**
 @brief
    Free the result provided by SOPC_ClientHelper_Read.

    If some data of the result shall be kept, either:
    - make a shallow copy of the SOPC_DataValue structure and reset data in array item with SOPC_DataValue_Initialize
    - make a deep copy of it using SOPC_DataValue_Copy

 @param nbElements
    Number of elements in result. It should be between 1 and INT32_MAX
 @param[out] values
    Clear content of the SOPC_DataValue[] array. The array is not freed.
    It shall be \p nElements long.
*/
void SOPC_ClientHelper_ReadResults_Free(size_t nbElements, SOPC_DataValue* values);

/**
 @brief
    Discover the references of a Node using Browse and browseNext services.
    If Browse Response returns ContinuationPoint, a BrowseNext Request is sent until no ContinuationPoint is returned.

    Restrictions:
    - Views are not managed
    - requestedMaxReferencesPerNode is set to 0
    - nodeClassMask is set to 0 (all NodeClasses)
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
 @param[out] browseResults
    A pre-allocated array to the output list of BrowseResult.
    It should be at least \p nElements long.
    When return, the order of this list matches the order of \p browseRequests.
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

/**
 @brief
    Call a method

 @param connectionId
    The connection id. It shall be > 0
 @param callRequests
    An array of SOPC_ClientHelper_CallMethodRequest describing the method called with input parameters
    Note: inputParams are moved using SOPC_Variant_Move and should not be used after this function call
 @param nbOfElements
    number of elements in the \p callRequests and \p callResults arrays. It should be between 1 and INT32_MAX.
 @param[out] callResults
    A pre-allocated array of SOPC_ClientHelper_CallMethodeResult structures

 @return
   '0' if operation succeed
   '-1' if connectionId not valid
   '-2' if input or output parameters not valid
   '-3' if out of memory or internal error issue
   '-100' if operation failed
*/
int32_t SOPC_ClientHelper_CallMethod(int32_t connectionId,
                                     SOPC_ClientHelper_CallMethodRequest* callRequests,
                                     size_t nbOfElements,
                                     SOPC_ClientHelper_CallMethodResult* callResults);

/**
 @brief
    Clears the content of the SOPC_ClientHelper_CallMethodResult[] \p results.
    The array is not deallocated since it is managed by the caller of SOPC_ClientHelper_CallMethod.

    Note: the variant values to keep after this call shall have been copied or moved using corresponding SOPC_Variant_*
          function.

 @param nbElements  The number of elements in the call method result array
 @param results     The SOPC_ClientHelper_CallMethodResult array to clear.
 */
void SOPC_ClientHelper_CallMethodResults_Clear(size_t nbElements, SOPC_ClientHelper_CallMethodResult* results);

/**
 * \brief Define client preferred locales ids from an array of locale strings.
 *
 * \param nbLocales  The number of locales defined in the array. It might be 0 if no locale defined (only default exist)
 * \param localeIds  The array of locales in priority order for localized strings to be returned by server.
 *                   Array and its content is copied by function.
 *
 * \return SOPC_STATUS_OK in case of success, otherwise SOPC_STATUS_INVALID_PARAMETERS
 *         if \p localeIds is invalid when \p nbLocales \> 0
 *         or SOPC_STATUS_INVALID_STATE if the configuration is not possible.
 *
 * \warning It shall not be called once a connection is established
 */
SOPC_ReturnStatus SOPC_ClientHelper_SetLocaleIds(size_t nbLocales, char** localeIds);

/**
 * \brief Define client application description
 *
 * \param applicationUri        The globally unique identifier for the application instance.
 * \param productUri            The globally unique identifier for the product.
 * \param defaultAppName        The name of the application using the default locale language.
 * \param defaultAppNameLocale  The default locale if any. If defined it shall exists in preferred locales
 * \param applicationType       The type of application, it shall be one of the OpcUa_ApplicationType_Client* types
 *
 * \return SOPC_STATUS_OK in case of success, otherwise SOPC_STATUS_INVALID_PARAMETERS
 *         if \p applicationUri, \p productUri or \p defaultAppName are invalid
 *         or SOPC_STATUS_INVALID_STATE if the configuration is not possible.
 *
 * \warning It shall not be called once a connection is established
 */
SOPC_ReturnStatus SOPC_ClientHelper_SetApplicationDescription(const char* applicationUri,
                                                              const char* productUri,
                                                              const char* defaultAppName,
                                                              const char* defaultAppNameLocale,
                                                              OpcUa_ApplicationType applicationType);
#endif /* LIBS2OPC_CLIENT_CMDS_H_ */
