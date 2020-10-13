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
 * \brief High level interface to build OPC UA request and access OPC UA response
 *
 * \note All parameters provided to request builders are copied and might be deallocated after builder call.
 *
 * \note Allocated requests will be deallocated by the library after service call.
 *       Once a service called (e.g. ::SOPC_ServerHelper_LocalServiceSync),
 *       the request is transfered to library and shall not be accessed anymore.
 *
 */

#ifndef SOPC_TYPE_HELPER_H_
#define SOPC_TYPE_HELPER_H_

#include "sopc_types.h"

/**
 * \brief Create a read request
 *
 * \param nbReadValues  Number of items (node, attribute, index range) to read with this read request.
 *                      \p nbReadValue <= INT32_MAX. ::SOPC_ReadRequest_SetReadValueFromStrings
 *                      or ::SOPC_ReadRequest_SetReadValue shall be called for each read value index.
 *                      Otherwise empty read value is sent for the index not configured.
 * \param tsToReturn    The kind of Timestamps to be returned for each requested Variable Value Attribute
 *
 * \return allocated read request in case of success, NULL in case of failure (invalid timestamp kind or out of memory)
 */
OpcUa_ReadRequest* SOPC_ReadRequest_Create(size_t nbReadValues, OpcUa_TimestampsToReturn tsToReturn);

/**
 * \brief Indicates to the server of maximum age of the data it should return.
 *        Default value is 0 to indicate to the server to return a fresh data value if applicable,
 *        value >= INT32_MAX might be used to request a cached value if applicable to server data.
 *
 * \param readRequest  The read request to configure
 * \param maxAge       Maximum age of the value to be read in milliseconds
 *
 * \return SOPC_STATUS_OK in case of success, SOPC_STATUS_INVALID_PARAMETERS in case of invalid read request (NULL)
 */
SOPC_ReturnStatus SOPC_ReadRequest_SetMaxAge(OpcUa_ReadRequest* readRequest, double maxAge);

/**
 * \brief Set the value to read at given index in read request  (using C strings for node id and index range)
 *
 * \param readRequest  The read request to configure
 *
 * \param index        Index of the read value to configure in the read request.
 *                     \p index < number of read value configured in ::SOPC_ReadRequest_Create
 *
 * \param nodeId       The id of the node to read as a C string, e.g. 'ns=1;s=MyNode'.
 *                     \p nodeId shall not be NULL
 *
 * \param attribute    The attribute to read in the node.
 *                     \p attribute shall be in the range of ::SOPC_AttributeId and not ::SOPC_AttributeId_Invalid
 *
 *
 * \param indexRange   The index range used to identify a single element of an array,
 *                     or a single range of indexes for arrays.
 *                     If not used for the read value requested it should be NULL.
 *
 * \return SOPC_STATUS_OK in case of success,
 *         SOPC_STATUS_INVALID_PARAMETERS in case of invalid read request, index, nodeId or attribute.
 */
SOPC_ReturnStatus SOPC_ReadRequest_SetReadValueFromStrings(OpcUa_ReadRequest* readRequest,
                                                           size_t index,
                                                           const char* nodeId,
                                                           SOPC_AttributeId attribute,
                                                           const char* indexRange);

/**
 * \brief Set the value to read at given index in read request
 *
 * \param readRequest  The read request to configure
 *
 * \param index        Index of the read value to configure in the read request.
 *                     \p index < number of read value configured in ::SOPC_ReadRequest_Create
 *
 * \param nodeId       The id of the node to read.
 *                     \p nodeId shall not be NULL
 *
 * \param attribute    The attribute to read in the node.
 *                     \p attribute shall be in the range of ::SOPC_AttributeId and not ::SOPC_AttributeId_Invalid
 *
 * \param indexRange   The index range used to identify a single element of an array,
 *                     or a single range of indexes for arrays.
 *                     If not used for the read value requested it should be NULL.
 *
 * \return SOPC_STATUS_OK in case of success,
 *         SOPC_STATUS_INVALID_PARAMETERS in case of invalid read request, index, nodeId or attribute.
 */
SOPC_ReturnStatus SOPC_ReadRequest_SetReadValue(OpcUa_ReadRequest* readRequest,
                                                size_t index,
                                                SOPC_NodeId* nodeId,
                                                SOPC_AttributeId attribute,
                                                SOPC_String* indexRange);

/**
 * \brief Set the data encoding of the value to read.
 *
 * \param readRequest   The read request to configure
 * \param index         Index of the read value to configure in the read request.
 *                      \p index < number of read value configured in ::SOPC_ReadRequest_Create
 * \param dataEncoding  The data encoding to use
 */
SOPC_ReturnStatus SOPC_ReadRequest_SetReadValueDataEncoding(OpcUa_ReadRequest* readRequest,
                                                            size_t index,
                                                            SOPC_QualifiedName* dataEncoding);

/**
 * \brief Create a write request
 *
 * \param nbWriteValues  Number of items (node, attribute, index range) to write with this write request.
 *                       \p nbWriteValue <= INT32_MAX
 *
 * \return allocated write request in case of success, NULL in case of failure (out of memory)
 */
OpcUa_WriteRequest* SOPC_WriteRequest_Create(size_t nbWriteValues);

/**
 * \brief Set the value to write at given index in write request (using C strings for node id and index range)
 *
 * \param writeRequest  The write request to configure
 *
 * \param index         Index of the write value to configure in the write request.
 *                      \p index < number of write value configured in ::SOPC_WriteRequest_Create
 *
 * \param nodeId       The id of the node to write as a C string, e.g. 'ns=1;s=MyNode'.
 *                     \p nodeId shall not be NULL
 *
 * \param attribute    The attribute to write in the node.
 *                     \p attribute shall be in the range of ::SOPC_AttributeId and not ::SOPC_AttributeId_Invalid.
 *
 *
 * \param indexRange   The index range used to identify a single element of an array,
 *                     or a single range of indexes for arrays.
 *                     If not used for the write value requested it should be NULL.
 *
 * \param value        The value to write for given item (node, attribute, index range).
 *                     \p value shall not be NULL
 *
 * \return SOPC_STATUS_OK in case of success,
 *         SOPC_STATUS_INVALID_PARAMETERS in case of invalid write request, index, nodeId, attribute or value.
 *
 * \note  Only ::SOPC_AttributeId_Value attribute write is supported by s2opc server
 */
SOPC_ReturnStatus SOPC_WriteRequest_SetWriteValueFromStrings(OpcUa_WriteRequest* writeRequest,
                                                             size_t index,
                                                             const char* nodeId,
                                                             SOPC_AttributeId attribute,
                                                             const char* indexRange,
                                                             SOPC_DataValue* value);

/**
 * \brief Set the value to write at given index in write request
 *
 * \param writeRequest  The write request to configure
 *
 * \param index         Index of the write value to configure in the write request.
 *                      \p index < number of write value configured in ::SOPC_WriteRequest_Create
 *
 * \param nodeId       The id of the node to write.
 *                     \p nodeId shall not be NULL
 *
 * \param attribute    The attribute to write in the node.
 *                     \p attribute shall be in the range of ::SOPC_AttributeId and not ::SOPC_AttributeId_Invalid.
 *
 *
 * \param indexRange   The index range used to identify a single element of an array,
 *                     or a single range of indexes for arrays.
 *                     If not used for the write value requested it should be NULL.
 *
 * \param value        The value to write for given item (node, attribute, index range).
 *                     \p value shall not be NULL
 *
 * \return SOPC_STATUS_OK in case of success,
 *         SOPC_STATUS_INVALID_PARAMETERS in case of invalid write request, index, nodeId, attribute or value.
 *
 * \note  Only ::SOPC_AttributeId_Value attribute write is supported by s2opc server
 */
SOPC_ReturnStatus SOPC_WriteRequest_SetWriteValue(OpcUa_WriteRequest* writeRequest,
                                                  size_t index,
                                                  SOPC_NodeId* nodeId,
                                                  SOPC_AttributeId attribute,
                                                  SOPC_String* indexRange,
                                                  SOPC_DataValue* value);

/**
 * \brief Enumerated node class mask values authorized for use with ::SOPC_BrowseRequest_SetBrowseDescription.
 *        Those values are masks which means they might be used with OR bitwise operation to
 *        select several node classes.
 */
typedef enum
{
    SOPC_NodeClassMask_Object = 0x01,
    SOPC_NodeClassMask_Variable = 0x02,
    SOPC_NodeClassMask_Method = 0x04,
    SOPC_NodeClassMask_ObjectType = 0x08,
    SOPC_NodeClassMask_VariableType = 0x10,
    SOPC_NodeClassMask_ReferenceType = 0x20,
    SOPC_NodeClassMask_DataType = 0x40,
    SOPC_NodeClassMask_View = 0x80,
} SOPC_BrowseRequest_NodeClassMask;

/**
 * \brief Enumerated result fields mask values authorized for use with ::SOPC_BrowseRequest_SetBrowseDescription.
 *        Those values are masks which means they might be used with OR bitwise operation to
 *        select several result fields.
 */
typedef enum
{
    SOPC_ResultMask_ReferenceType = 0x01,
    SOPC_ResultMask_IsForward = 0x02,
    SOPC_ResultMask_NodeClass = 0x04,
    SOPC_ResultMask_BrowseName = 0x08,
    SOPC_ResultMask_DisplayName = 0x10,
    SOPC_ResultMask_TypeDefinition = 0x20,
} SOPC_BrowseRequest_ResultMask;

/**
 * \brief Create a browse request
 *
 * \param nbNodesToBrowse       Number of nodes to browse with this browse request.
 *                              \p nbNodesToBrowse <= INT32_MAX
 *
 * \param maxReferencesPerNode  Indicates the maximum number of references to return for each starting node
 *                              specified in the request.
 *
 * \param optView               (Optional) Description of the View to browse. If no view used, it should be NULL.
 *
 * \return allocated browse request in case of success, NULL in case of failure (invalid parameters or out of memory)
 */
OpcUa_BrowseRequest* SOPC_BrowseRequest_Create(size_t nbNodesToBrowse,
                                               size_t maxReferencesPerNode,
                                               OpcUa_ViewDescription* optView);

/**
 * \brief Set the node to browse at given index in browse request (using C strings for node id and reference type id)
 *
 * \param browseRequest  The browse request to configure
 *
 * \param index            Index of the browse description to configure in the browse request.
 *                         \p index < number of nodes to browse configured in ::SOPC_BrowseRequest_Create
 *
 * \param nodeId           The id of the node to browse as a C string, e.g. 'ns=1;s=MyNode'.
 *                         \p nodeId shall not be NULL
 *
 * \param browseDirection  The browse direction to use
 *                         \p browseDirection shall be in the range of ::OpcUa_BrowseDirection.
 *
 *
 * \param referenceTypeId  The node id of the reference type to browse as a C string,  e.g. 'ns=0;i=35' or 'i=35'.
 *                         If not specified then all References are returned and \p includeSubtypes is ignored.
 *                         If not used for to browse this node it should be NULL.
 *
 * \param includeSubtypes  Indicates whether subtypes of the ReferenceType should be included in the browse.
 *                         If TRUE, then instances of referenceTypeId and all of its subtypes are returned.
 *
 * \param nodeClassMask    Mask specifying the node classes of the target nodes.
 *                         Only TargetNodes with the selected node classes are returned.
 *                         If set to zero, then all NodeClasses are returned.
 *                         Value shall be a bitwise disjunction of ::SOPC_BrowseRequest_NodeClassMask
 *
 * \param resultMask       Mask specifying the fields in the ::OpcUa_ReferenceDescription structure
 *                         that should be returned.
 *                         Value shall be a bitwise disjunction of ::SOPC_BrowseRequest_ResultMask
 *
 * \return SOPC_STATUS_OK in case of success,
 *         SOPC_STATUS_INVALID_PARAMETERS in case of invalid browse request, index, nodeId or browseDirection.
 */
SOPC_ReturnStatus SOPC_BrowseRequest_SetBrowseDescriptionFromStrings(OpcUa_BrowseRequest* browseRequest,
                                                                     size_t index,
                                                                     const char* nodeId,
                                                                     OpcUa_BrowseDirection browseDirection,
                                                                     const char* referenceTypeId,
                                                                     bool includeSubtypes,
                                                                     SOPC_BrowseRequest_NodeClassMask nodeClassMask,
                                                                     SOPC_BrowseRequest_ResultMask resultMask);

/**
 * \brief Set the node to browse at given index in browse request
 *
 * \param browseRequest  The browse request to configure
 *
 * \param index            Index of the browse description to configure in the browse request.
 *                         \p index < number of nodes to browse configured in ::SOPC_BrowseRequest_Create
 *
 * \param nodeId           The id of the node to browse.
 *                         \p nodeId shall not be NULL
 *
 * \param browseDirection  The browse direction to use
 *                         \p browseDirection shall be in the range of ::OpcUa_BrowseDirection.
 *
 *
 * \param referenceTypeId  The node id of the reference type to browse.
 *                         If not specified then all References are returned and \p includeSubtypes is ignored.
 *                         If not used for to browse this node it should be NULL.
 *
 * \param includeSubtypes  Indicates whether subtypes of the ReferenceType should be included in the browse.
 *                         If set to true, then instances of referenceTypeId and all of its subtypes are returned.
 *
 * \param nodeClassMask    Mask specifying the node classes of the target nodes.
 *                         Only TargetNodes with the selected node classes are returned.
 *                         If set to zero, then all NodeClasses are returned.
 *                         Value shall be a bitwise disjunction of ::SOPC_BrowseRequest_NodeClassMask
 *
 * \param resultMask       Mask specifying the fields in the ::OpcUa_ReferenceDescription structure
 *                         that should be returned.
 *                         Value shall be a bitwise disjunction of ::SOPC_BrowseRequest_ResultMask
 *
 * \return SOPC_STATUS_OK in case of success,
 *         SOPC_STATUS_INVALID_PARAMETERS in case of invalid browse request, index, nodeId or browseDirection.
 */
SOPC_ReturnStatus SOPC_BrowseRequest_SetBrowseDescription(OpcUa_BrowseRequest* browseRequest,
                                                          size_t index,
                                                          SOPC_NodeId* nodeId,
                                                          OpcUa_BrowseDirection browseDirection,
                                                          SOPC_NodeId* referenceTypeId,
                                                          bool includeSubtypes,
                                                          SOPC_BrowseRequest_NodeClassMask nodeClassMask,
                                                          SOPC_BrowseRequest_ResultMask resultMask);

/**
 * \brief Create a browse next request
 *
 * \param releaseContinuationPoints  If set to true passed continuationPoints shall be reset to free resources
 *                                   in the Server. Otherwise the passed continuationPoints shall be used to get
 *                                   the next set of browse information.
 * \param nbContinuationPoints       Number of continuation points to browse with this browse next request.
 *                                   \p nbContinuationPoints <= INT32_MAX
 *
 * \return allocated browse next request in case of success, NULL in case of failure
 *         (invalid parameters or out of memory)
 *
 * \note Only 1 continuation point (per session) is supported by s2opc server.
 */
OpcUa_BrowseNextRequest* SOPC_BrowseNextRequest_Create(bool releaseContinuationPoints, size_t nbContinuationPoints);

/**
 * \brief Set the continuation point to browse at given index in browse next request
 *
 * \param browseNextRequest  The browse next request to configure
 *
 * \param index              Index of the continuation point to configure in the browse next request.
 *                           \p index < number of continuation points configured in ::SOPC_BrowseNextRequest_Create
 *
 * \param continuationPoint  The continuation point to browse.
 *                           It shall not be NULL and shall be the one returned by a received ::OpcUa_BrowseResult.
 *
 * \return SOPC_STATUS_OK in case of success,
 *         SOPC_STATUS_INVALID_PARAMETERS in case of invalid browse next request, index or continuationPoint.
 */
SOPC_ReturnStatus SOPC_BrowseNextRequest_SetContinuationPoint(OpcUa_BrowseNextRequest* browseNextRequest,
                                                              size_t index,
                                                              SOPC_ByteString* continuationPoint);

/**
 * \brief Create a GetEndpoint request for the given endpoint URL
 *
 * \param endpointURL  The endpoint URL as C string: "opc.tcp://<hostname>:<port>[/<name>]"
 *
 * \return allocated get endpoints request in case of success, NULL in case of failure
 *         (invalid parameters or out of memory)
 */
OpcUa_GetEndpointsRequest* SOPC_GetEndpointsRequest_Create(const char* endpointURL);

/**
 * \brief Add a preferred locale to the get endpoints request (Optional)
 *        Preferred locale order is from first call to last call to this function.
 *
 * \param getEndpointsReq  The get endpoints request to configure
 *
 * \param localeId         The locale to add as preferred for application description.
 *                         It shall not be NULL.
 *
 * \return SOPC_STATUS_OK in case of success,
 *         SOPC_STATUS_INVALID_PARAMETERS in case of invalid get endpoints request or localId.
 */
SOPC_ReturnStatus SOPC_GetEndpointsRequest_AddPreferredLocale(OpcUa_GetEndpointsRequest* getEndpointsReq,
                                                              const char* localeId);

/**
 * \brief Add a profile URI to the get endpoints request (Optional)
 *        All endpoints are returned if ::SOPC_GetEndpointsRequest_AddProfileURI unused.
 *
 * \param getEndpointsReq  The get endpoints request to configure
 *
 * \param profileURI       Transport profile to be returned by GetEndpoints service.
 *                         It shall not be NULL.
 *
 * \return SOPC_STATUS_OK in case of success,
 *         SOPC_STATUS_INVALID_PARAMETERS in case of invalid get endpoints request or profileURI.
 */
SOPC_ReturnStatus SOPC_GetEndpointsRequest_AddProfileURI(OpcUa_GetEndpointsRequest* getEndpointsReq,
                                                         const char* profileURI);

/**
 * \brief Create a complete RegisterServer2 request from the current server configuration.
 *        It shall be used to register the current server for FindServer and FindServerOnNetwork services.
 *
 * \return allocated register server 2 request in case of success, NULL in case of failure
 *         (invalid parameters or out of memory)
 */
OpcUa_RegisterServer2Request* SOPC_RegisterServer2Request_CreateFromServerConfiguration(void);

#endif /* SOPC_TYPE_HELPER_H_ */
