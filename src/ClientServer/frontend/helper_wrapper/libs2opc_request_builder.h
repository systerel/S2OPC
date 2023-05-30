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
 * \note All parameters provided to request builders are copied and might be deallocated after builder call
 *       (unless specified otherwise).
 *
 * \note Allocated requests will be deallocated by the library after service call.
 *       Once a service called (e.g. ::SOPC_ServerHelper_LocalServiceSync),
 *       the request is transfered to library and shall not be accessed anymore.
 *
 */

#ifndef LIBS2OPC_REQUEST_BUILDER_H_
#define LIBS2OPC_REQUEST_BUILDER_H_

#include "sopc_types.h"

/**
 * \brief Creates a read request
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
 *
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
 * \brief Sets the value to read at given index in read request  (using C strings for node id and index range)
 *
 * \param readRequest  The read request to configure
 *
 * \param index        Index of the read value to configure in the read request.
 *                     \p index < number of read value configured in ::SOPC_ReadRequest_Create
 *
 * \param nodeId       The id of the node to read as a C string, e.g. 'ns=1;s=MyNode'.
 *                     \p nodeId shall not be NULL.
 *                     Format is described in
 *                     <a href=https://reference.opcfoundation.org/v104/Core/docs/Part6/5.3.1/#5.3.1.10>
 *                     OPC UA specification</a>.
 *
 * \param attribute    The attribute to read in the node.
 *                     \p attribute shall be in the range of ::SOPC_AttributeId and not ::SOPC_AttributeId_Invalid
 *
 * \param indexRange   The index range used to identify a single element of an array,
 *                     or a single range of indexes for arrays.
 *                     If not used for the read value requested it should be NULL.
 *                     Format is described in
 *                     <a href=https://reference.opcfoundation.org/v104/Core/docs/Part4/7.22>
 *                     OPC UA specification</a>.
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
 * \brief Sets the value to read at given index in read request
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
 *                     Format is described in
 *                     <a href=https://reference.opcfoundation.org/v104/Core/docs/Part4/7.22>
 *                     OPC UA specification</a>.
 *
 * \return SOPC_STATUS_OK in case of success,
 *         SOPC_STATUS_INVALID_PARAMETERS in case of invalid read request, index, nodeId or attribute.
 */
SOPC_ReturnStatus SOPC_ReadRequest_SetReadValue(OpcUa_ReadRequest* readRequest,
                                                size_t index,
                                                const SOPC_NodeId* nodeId,
                                                SOPC_AttributeId attribute,
                                                const SOPC_String* indexRange);

/**
 * \brief Sets the data encoding of the value to read.
 *
 * \param readRequest   The read request to configure
 * \param index         Index of the read value to configure in the read request.
 *                      \p index < number of read value configured in ::SOPC_ReadRequest_Create
 * \param dataEncoding  The data encoding to use
 */
SOPC_ReturnStatus SOPC_ReadRequest_SetReadValueDataEncoding(OpcUa_ReadRequest* readRequest,
                                                            size_t index,
                                                            const SOPC_QualifiedName* dataEncoding);

/**
 * \brief Creates a write request
 *
 * \param nbWriteValues  Number of items (node, attribute, index range) to write with this write request.
 *                       \p nbWriteValue <= INT32_MAX. ::SOPC_WriteRequest_SetWriteValueFromStrings
 *                       or ::SOPC_WriteRequest_SetWriteValue shall be called for each write value index.
 *                       Otherwise empty write value is sent for the index not configured.
 *
 * \return allocated write request in case of success, NULL in case of failure (out of memory)
 */
OpcUa_WriteRequest* SOPC_WriteRequest_Create(size_t nbWriteValues);

/**
 * \brief Sets the value to write at given index in write request (using C strings for node id and index range)
 *
 * \param writeRequest  The write request to configure
 *
 * \param index         Index of the write value to configure in the write request.
 *                      \p index < number of write value configured in ::SOPC_WriteRequest_Create
 *
 * \param nodeId       The id of the node to write as a C string, e.g. 'ns=1;s=MyNode'.
 *                     \p nodeId shall not be NULL.
 *                     Format is described in
 *                     <a href=https://reference.opcfoundation.org/v104/Core/docs/Part6/5.3.1/#5.3.1.10>
 *                     OPC UA specification</a>.
 *
 * \param attribute    The attribute to write in the node.
 *                     \p attribute shall be in the range of ::SOPC_AttributeId and not ::SOPC_AttributeId_Invalid.
 *
 *
 * \param indexRange   The index range used to identify a single element of an array,
 *                     or a single range of indexes for arrays.
 *                     If not used for the write value requested it should be NULL.
 *                     Format is described in
 *                     <a href=https://reference.opcfoundation.org/v104/Core/docs/Part4/7.22>
 *                     OPC UA specification</a>.
 *                     If the indexRange parameter is specified then the Value in \p value shall be an array even if
 *                     only one element is being written.
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
                                                             const SOPC_DataValue* value);

/**
 * \brief Sets the value to write at given index in write request
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
 *                     Format is described in
 *                     <a href=https://reference.opcfoundation.org/v104/Core/docs/Part4/7.22>
 *                     OPC UA specification</a>.
 *                     If the indexRange parameter is specified then the Value in \p value shall be an array even if
 *                     only one element is being written.
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
                                                  const SOPC_NodeId* nodeId,
                                                  SOPC_AttributeId attribute,
                                                  const SOPC_String* indexRange,
                                                  const SOPC_DataValue* value);

/**
 * \brief Enumerated result fields mask values authorized for use with ::SOPC_BrowseRequest_SetBrowseDescription.
 *        Those values are masks which means they might be used with OR bitwise operation to
 *        select several result fields.
 */

/**
 * \brief Creates a browse request
 *
 * \param nbNodesToBrowse       Number of nodes to browse with this browse request.
 *                              \p nbNodesToBrowse <= INT32_MAX. ::SOPC_BrowseRequest_SetBrowseDescriptionFromStrings
 *                              or ::SOPC_BrowseRequest_SetBrowseDescription shall be called for each browse description
 *                              index. Otherwise empty browse description is sent for the index not configured.
 *
 * \param maxReferencesPerNode  Indicates the maximum number of references to return for each starting node
 *                              specified in the request (0 means no limitation).
 *
 * \param optView               (Optional) Description of the View to browse. If no view used, it should be NULL.
 *
 * \return allocated browse request in case of success, NULL in case of failure (invalid parameters or out of memory)
 */
OpcUa_BrowseRequest* SOPC_BrowseRequest_Create(size_t nbNodesToBrowse,
                                               size_t maxReferencesPerNode,
                                               const OpcUa_ViewDescription* optView);

/**
 * \brief Sets the node to browse at given index in browse request (using C strings for node id and reference type id)
 *
 * \param browseRequest  The browse request to configure
 *
 * \param index            Index of the browse description to configure in the browse request.
 *                         \p index < number of nodes to browse configured in ::SOPC_BrowseRequest_Create
 *
 * \param nodeId           The id of the node to browse as a C string, e.g. 'ns=1;s=MyNode'.
 *                         \p nodeId shall not be NULL.
 *                         Format is described in
 *                         <a href=https://reference.opcfoundation.org/v104/Core/docs/Part6/5.3.1/#5.3.1.10>
 *                         OPC UA specification</a>.
 *
 * \param browseDirection  The browse direction to use
 *                         \p browseDirection shall be in the range of ::OpcUa_BrowseDirection.
 *
 *
 * \param referenceTypeId  The node id of the reference type to browse as a C string,  e.g. 'ns=0;i=35' or 'i=35'.
 *                         If not specified then all References are returned and \p includeSubtypes is ignored.
 *                         If not used for to browse this node it should be NULL.
 *                         Format is described in
 *                         <a href=https://reference.opcfoundation.org/v104/Core/docs/Part6/5.3.1/#5.3.1.10>
 *                         OPC UA specification</a>.
 *
 * \param includeSubtypes  Indicates whether subtypes of the ReferenceType should be included in the browse.
 *                         If TRUE, then instances of referenceTypeId and all of its subtypes are returned.
 *
 * \param nodeClassMask    Mask specifying the node classes of the target nodes.
 *                         Only TargetNodes with the selected node classes are returned.
 *                         If set to zero, then all NodeClasses are returned.
 *                         Value shall be a bitwise OR of ::OpcUa_NodeClass
 *
 * \param resultMask       Mask specifying the fields in the ::OpcUa_ReferenceDescription structure
 *                         that should be returned.
 *                         Value shall be a bitwise OR of ::OpcUa_BrowseResultMask
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
                                                                     OpcUa_NodeClass nodeClassMask,
                                                                     OpcUa_BrowseResultMask resultMask);

/**
 * \brief Sets the node to browse at given index in browse request
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
 *                         Value shall be a bitwise OR of ::OpcUa_NodeClass
 *
 * \param resultMask       Mask specifying the fields in the ::OpcUa_ReferenceDescription structure
 *                         that should be returned.
 *                         Value shall be a bitwise OR of ::OpcUa_BrowseResultMask
 *
 * \return SOPC_STATUS_OK in case of success,
 *         SOPC_STATUS_INVALID_PARAMETERS in case of invalid browse request, index, nodeId or browseDirection.
 */
SOPC_ReturnStatus SOPC_BrowseRequest_SetBrowseDescription(OpcUa_BrowseRequest* browseRequest,
                                                          size_t index,
                                                          const SOPC_NodeId* nodeId,
                                                          OpcUa_BrowseDirection browseDirection,
                                                          const SOPC_NodeId* referenceTypeId,
                                                          bool includeSubtypes,
                                                          OpcUa_NodeClass nodeClassMask,
                                                          OpcUa_BrowseResultMask resultMask);

/**
 * \brief Creates a browse next request
 *
 *   BrowseNext are used to continue a Browse that had too much browse results (more than \p maxReferencesPerNode).
 *   The continuation points are found in the browse response.
 *   A continuation point shall be used to iterate over all the browse results.
 *   When no more browse results are available, the BrowseNext should be sent once more with the continuation points to
 *   free (set \p releaseContinuationPoints to true for these continuation points).
 *
 * \param releaseContinuationPoints  If set to true passed continuationPoints shall be reset to free resources
 *                                   in the Server. Otherwise the passed continuationPoints shall be used to get
 *                                   the next set of browse information.
 * \param nbContinuationPoints       Number of continuation points to browse with this browse next request.
 *                                   \p nbContinuationPoints <= INT32_MAX
 *
 * \return allocated browse next request in case of success, NULL in case of failure
 *         (invalid parameters or out of memory)
 */
OpcUa_BrowseNextRequest* SOPC_BrowseNextRequest_Create(bool releaseContinuationPoints, size_t nbContinuationPoints);

/**
 * \brief Create a translate browse paths request
 *
 * \param nbTranslateBrowsePaths   Number of nodes to browse with this browse request.
 *                                 \p nbTranslateBrowsePaths <= INT32_MAX.
 *                                 ::SOPC_TranslateBrowsePathRequest_SetPathFromString or
 *                                 ::SOPC_TranslateBrowsePathRequest_SetPath shall be called
 *                                 for each path description index.
 *                                 Otherwise empty path description is sent for the index not configured.
 *
 * \return allocated translate browse request in case of success, NULL in case of failure
 *         (invalid parameters or out of memory)
 */
OpcUa_TranslateBrowsePathsToNodeIdsRequest* SOPC_TranslateBrowsePathsRequest_Create(size_t nbTranslateBrowsePaths);

/**
 * \brief Sets the browse path to translate at given index in translate browse paths request
 *        (using C strings for node id)
 *
 * \param tbpRequest       The translate browse paths request to configure
 *
 * \param index            Index of the browse path description to configure in the translate browse path request.
 *                         \p index < number of browse paths configured in ::SOPC_TranslateBrowsePathsRequest_Create
 *
 * \param startingNodeId   The id of the node from which translate browse path start as a C string.
 *                         E.g. 'ns=1;s=MyNode'.  \p nodeId shall not be NULL.
 *                         Format is described in
 *                         <a href=https://reference.opcfoundation.org/v104/Core/docs/Part6/5.3.1/#5.3.1.10>
 *                         OPC UA specification</a>.
 *
 * \param nbPathElements   The number of elements in the path (>0).
 *
 * \param pathElements     The array of path elements of length \p nbPathElements.
 *                         It might be built using ::SOPC_RelativePathElements_Create and shall not be NULL.
 *                         The array is assigned in the translate browse path request
 *                         and its memory managed with request.
 *                         i.e. \p pathElements shall not be unallocated or used anymore.
 *
 * \return SOPC_STATUS_OK in case of success,
 *         SOPC_STATUS_INVALID_PARAMETERS in case of invalid request, index, nodeId or path elements.
 */
SOPC_ReturnStatus SOPC_TranslateBrowsePathRequest_SetPathFromString(
    OpcUa_TranslateBrowsePathsToNodeIdsRequest* tbpRequest,
    size_t index,
    const char* startingNodeId,
    size_t nbPathElements,
    OpcUa_RelativePathElement* pathElements);

/**
 * \brief Sets the browse path to translate at given index in translate browse paths request
 *
 * \param tbpRequest       The translate browse paths request to configure
 *
 * \param index            Index of the browse path description to configure in the translate browse path request.
 *                         \p index < number of browse paths configured in ::SOPC_TranslateBrowsePathsRequest_Create
 *
 * \param startingNodeId   The id of the node from which translate browse path start, \p nodeId shall not be NULL.
 *
 * \param nbPathElements   The number of elements in the path (>0).
 *
 * \param pathElements     The array of path elements of length \p nbPathElements.
 *                         It might be built using ::SOPC_RelativePathElements_Create and shall not be NULL.
 *                         The array is assigned in the translate browse path request
 *                         and its memory managed with request.
 *                         i.e. \p pathElements shall not be unallocated or used anymore.
 *
 * \return SOPC_STATUS_OK in case of success,
 *         SOPC_STATUS_INVALID_PARAMETERS in case of invalid request, index, nodeId or path elements.
 */
SOPC_ReturnStatus SOPC_TranslateBrowsePathRequest_SetPath(OpcUa_TranslateBrowsePathsToNodeIdsRequest* tbpRequest,
                                                          size_t index,
                                                          SOPC_NodeId* startingNodeId,
                                                          size_t nbPathElements,
                                                          OpcUa_RelativePathElement* pathElements);

/**
 * \brief Creates an array of relative path element to be used in translate browse path.
 *
 * \param nbPathElements   Number of relative path elements in the array
 *                                 \p nbPathElements <= INT32_MAX.
 *                                 ::SOPC_RelativePathElements_SetPathElement shall be called
 *                                 for each path element description index.
 *                                 Otherwise empty path element description is sent for the index not configured.
 *
 * \return allocated relative path element array in case of success, NULL in case of failure
 *         (invalid parameters or out of memory)
 */
OpcUa_RelativePathElement* SOPC_RelativePathElements_Create(size_t nbPathElements);

/**
 * \brief Sets the path element at given index in relative path element array
 *
 * \param pathElementsArray  The array of path elements to initialize
 *
 * \param index            Index of the path element description to configure in the array.
 *                         \p index < number of path elements configured in ::SOPC_RelativePathElements_Create
 *
 * \param referenceTypeId  The id of a node defining the kind of reference to follow from the current node
 *                         or NULL if all References are included (in this case parameter includeSubtypes is ignored).
 *
 * \param isInverse        Only inverse references shall be followed if this value is TRUE.
 *                         Only forward references shall be followed if this value is FALSE.
 *
 * \param includeSubtypes  Indicates whether subtypes of \p referenceTypeId shall be followed.
 *                         Subtypes are included if this value is TRUE.
 *
 * \param targetNsIndex    Index of the namespace of the expected target node
 *
 * \param targetName       BrowseName of the expected target node
 *
 * \return SOPC_STATUS_OK in case of success,
 *         SOPC_STATUS_INVALID_PARAMETERS in case of invalid parameters.
 */
SOPC_ReturnStatus SOPC_RelativePathElements_SetPathElement(OpcUa_RelativePathElement* pathElementsArray,
                                                           size_t index,
                                                           const SOPC_NodeId* referenceTypeId,
                                                           bool isInverse,
                                                           bool includeSubtypes,
                                                           uint16_t targetNsIndex,
                                                           const char* targetName);

/**
 * \brief Sets the continuation point to browse at given index in browse next request
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
                                                              const SOPC_ByteString* continuationPoint);

/**
 * \brief Creates a GetEndpoint request for the given endpoint URL
 *
 * \param endpointURL  The endpoint URL as C string: "opc.tcp://<hostname>:<port>[/<name>]"
 *
 * \return allocated get endpoints request in case of success, NULL in case of failure
 *         (invalid parameters or out of memory)
 */
OpcUa_GetEndpointsRequest* SOPC_GetEndpointsRequest_Create(const char* endpointURL);

/**
 * \brief Requests preferred locales for the endpoints to be returned by the get endpoints service (Optional)
 *        Preferred locale order is the order of \p localesIds array
 *
 * \param getEndpointsReq  The get endpoints request to configure
 * \param nbLocales        Number of locales in array. It shall be > 0.
 * \param localeIds        Array of preferred locale ids by order of preference.
 *                         Values are copied. They shall not be NULL.
 *
 * \return SOPC_STATUS_OK in case of success,
 *         SOPC_STATUS_INVALID_PARAMETERS in case of invalid get endpoints request or locale ids,
 *         SOPC_STATUS_INVALID_STATE in case preferred locales already set.
 */
SOPC_ReturnStatus SOPC_GetEndpointsRequest_SetPreferredLocales(OpcUa_GetEndpointsRequest* getEndpointsReq,
                                                               size_t nbLocales,
                                                               char** localeIds);

/**
 * \brief Requests profile URIs for the endpoints to be returned by the get endpoints service (Optional)
 *        Endpoints of all transport profile types available are returned
 *        if ::SOPC_GetEndpointsRequest_SetProfileURIs unused.
 *
 * \param getEndpointsReq  The get endpoints request to configure
 * \param nbProfiles        Number of profile URIs in array. It shall be > 0.
 * \param profileURIs       Array of transport profile URIs to be returned by GetEndpoints service.
 *                          Values shall not be NULL.
 *                          E.g. "http://opcfoundation.org/UA-Profile/Transport/uatcp-uasc-uabinary",
 *                          other possible values are described in Transport category of
 *                          <a href=https://reference.opcfoundation.org/v104/Core/docs/Part7/6.2/>
 *                          OPC UA specification</a>.

 *
 * \return SOPC_STATUS_OK in case of success,
 *         SOPC_STATUS_INVALID_PARAMETERS in case of invalid get endpoints request or profiles URI
 *         (invalid number or NULL value), SOPC_STATUS_INVALID_STATE in case profile URIs already set.
 */
SOPC_ReturnStatus SOPC_GetEndpointsRequest_SetProfileURIs(OpcUa_GetEndpointsRequest* getEndpointsReq,
                                                          size_t nbProfiles,
                                                          char** profileURIs);

/**
 * \brief Creates a complete RegisterServer2 request from the current server configuration.
 *        It shall be used to register the current server for FindServer and FindServerOnNetwork services.
 *
 * \return allocated register server 2 request in case of success, NULL in case of failure
 *         (invalid parameters or out of memory)
 */
OpcUa_RegisterServer2Request* SOPC_RegisterServer2Request_CreateFromServerConfiguration(void);

/**
 * \brief Creates an add nodes request
 *
 * \param nbAddNodes  Number of nodes to add with this request.
 *                      \p nbAddNodes <= INT32_MAX.
 *                      ::SOPC_AddNodeRequest_SetVariableAttributes shall be called for each add node item index.
 *                      Otherwise empty add node item is sent for the index not configured.
 *
 * \return allocated read request in case of success, NULL in case of failure (out of memory)
 */
OpcUa_AddNodesRequest* SOPC_AddNodesRequest_Create(size_t nbAddNodes);

/**
 * \brief Sets the attributes values requested for the Variable node to add.
 *        Optional parameters are prefixed by "opt" and shall be NULL if not defined.
 *        If optional parameters are not defined the server will choose values for this attributes.
 *
 * \param addNodesRequest             The add nodes request to configure.
 * \param index                       Index of the add nodes items to configure in the request.
 *                                    \p index < number of add nodes configured in ::SOPC_AddNodesRequest_Create.
 * \param parentNodeId                Parent NodeId of the node to add, it should be an Object or Variable node.
 * \param referenceTypeId             Reference type of the relation between parent node and added node.
 * \param optRequestedNodeId          Requested NodeId for the node to add (optional).
 * \param browseName                  BrowseName for the node to add, it should be unique in the parent node context.
 * \param typeDefinition              TypeDefinition for the Variable node to add
 *                                    (BaseDataVariableType, PropertyType, etc.).
 * \param optDisplayName              DisplayName for the node to add (optional).
 * \param optDescription              Description for the node to add (optional).
 * \param optWriteMask                WriteMask for the node to add (optional).
 * \param optUserWriteMask            UserWriteMask for the node to add (optional).
 *                                    It should not be defined since it depends on the user.
 * \param optValue                    Value for the Variable node to add (optional).
 * \param optDataType                 DataType for the Variable node to add (optional).
 * \param optValueRank                ValueRank for the Variable node to add (optional).
 * \param noOfArrayDimensions         Number of array dimensions for the Value of Variable node to add, if
 *                                    \p optArrayDimensions not defined it shall be 0.
 * \param optArrayDimensions          Array of dimensions for the Value of Variable node to add (optional).
 *                                    If defined \p noOfArrayDimensions shall be greater than 0.
 * \param optAccessLevel              AccessLevel for the Value of Variable node to add (optional).
 * \param optUserAccessLevel          UserAccessLevel for the Value of Variable node to add (optional).
 *                                    It should not be defined since it depends on the user.
 * \param optMinimumSamplingInterval  MinimumSamplingInterval (ms) for the Value of Variable node
 *                                    in a subscription (optional)
 * \param optHistorizing              Historizing flag for the Value of Variable node (optional).
 *
 * \return SOPC_STATUS_OK in case of success,
 *         SOPC_STATUS_INVALID_PARAMETERS in case of invalid read request, index, nodeId or attribute.
 */
SOPC_ReturnStatus SOPC_AddNodeRequest_SetVariableAttributes(OpcUa_AddNodesRequest* addNodesRequest,
                                                            size_t index,
                                                            const SOPC_ExpandedNodeId* parentNodeId,
                                                            const SOPC_NodeId* referenceTypeId,
                                                            const SOPC_ExpandedNodeId* optRequestedNodeId,
                                                            const SOPC_QualifiedName* browseName,
                                                            const SOPC_ExpandedNodeId* typeDefinition,
                                                            const SOPC_LocalizedText* optDisplayName,
                                                            const SOPC_LocalizedText* optDescription,
                                                            const uint32_t* optWriteMask,
                                                            const uint32_t* optUserWriteMask,
                                                            const SOPC_Variant* optValue,
                                                            const SOPC_NodeId* optDataType,
                                                            const int32_t* optValueRank,
                                                            int32_t noOfArrayDimensions,
                                                            const uint32_t* optArrayDimensions,
                                                            const SOPC_Byte* optAccessLevel,
                                                            const SOPC_Byte* optUserAccessLevel,
                                                            const double* optMinimumSamplingInterval,
                                                            SOPC_Boolean* optHistorizing);

/**
 * \brief Creates an CreateMonitoredItems request
 *
 * \param subscriptionId        The identifier of the subscription for which monitored items will be created
 *                              (obtained with ::SOPC_ClientHelperNew_Subscription_GetSubscriptionId
 *                               or in a ::OpcUa_CreateSubscriptionResponse if subscription is managed manually)
 * \param nbMonitoredItems      Number of MonitoredItem to create with the request.
 *                              \p nbMonitoredItems <= INT32_MAX.
 *                              ::SOPC_CreateMonitoredItemsRequest_SetMonitoredItemParams shall be called
 *                              for each monitored item index.
 * \param ts                    Set the timestamps (source, server) to be returned for any monitored item.
 *
 * \return allocated CreateMonitoredItems request in case of success, NULL in case of failure
 *         (invalid parameters or out of memory)
 */
OpcUa_CreateMonitoredItemsRequest* SOPC_CreateMonitoredItemsRequest_Create(uint32_t subscriptionId,
                                                                           size_t nbMonitoredItems,
                                                                           OpcUa_TimestampsToReturn ts);

/**
 * \brief Sets the monitored item identification parameters.
 *        It shall be completed by a call to ::SOPC_CreateMonitoredItemsRequest_SetMonitoredItemParams
 *        for the same index to configure monitoring parameters.
 *
 * \param createMIrequest  The create monitored item request to configure
 *
 * \param index         Index of the create monitored item to configure in the request.
 *                      \p index < number of monitored items configured in ::SOPC_CreateMonitoredItemsRequest_Create
 *
 * \param nodeId       The id of the node to monitor.
 *                     \p nodeId shall not be NULL
 *
 * \param attribute    The attribute to write in the node.
 *                     \p attribute shall be in the range of ::SOPC_AttributeId and not ::SOPC_AttributeId_Invalid.
 *
 *
 * \param indexRange   The index range used to identify a single element of an array,
 *                     or a single range of indexes for arrays.
 *                     If not used for the monitored item it should be NULL.
 *                     Format is described in
 *                     <a href=https://reference.opcfoundation.org/v104/Core/docs/Part4/7.22>
 *                     OPC UA specification</a>.
 *
 * \return SOPC_STATUS_OK in case of success,
 *         SOPC_STATUS_INVALID_PARAMETERS in case of invalid request, index, nodeId or attribute.
 */
SOPC_ReturnStatus SOPC_CreateMonitoredItemsRequest_SetMonitoredItemId(
    OpcUa_CreateMonitoredItemsRequest* createMIrequest,
    size_t index,
    const SOPC_NodeId* nodeId,
    SOPC_AttributeId attribute,
    const SOPC_String* indexRange);

/**
 * \brief Sets the monitored item identification parameters using C string parameters.
 *        It shall be completed by a call to ::SOPC_CreateMonitoredItemsRequest_SetMonitoredItemParams
 *        for the same index to configure monitoring parameters.
 *
 * \param createMIrequest  The create monitored item request to configure
 *
 * \param index         Index of the create monitored item to configure in the request.
 *                      \p index < number of monitored items configured in ::SOPC_CreateMonitoredItemsRequest_Create
 *
 * \param nodeId       The id of the node to monitor as a C string, e.g. 'ns=1;s=MyNode'.
 *                     \p nodeId shall not be NULL.
 *                     Format is described in
 *                     <a href=https://reference.opcfoundation.org/v104/Core/docs/Part6/5.3.1/#5.3.1.10>
 *                     OPC UA specification</a>.
 *
 * \param attribute    The attribute to write in the node.
 *                     \p attribute shall be in the range of ::SOPC_AttributeId and not ::SOPC_AttributeId_Invalid.
 *
 * \param indexRange   The index range used to identify a single element of an array,
 *                     or a single range of indexes for arrays.
 *                     If not used for the monitored item requested it should be NULL.
 *                     Format is described in
 *                     <a href=https://reference.opcfoundation.org/v104/Core/docs/Part4/7.22>
 *                     OPC UA specification</a>.
 *
 * \return SOPC_STATUS_OK in case of success,
 *         SOPC_STATUS_INVALID_PARAMETERS in case of invalid request, index, nodeId or attribute.
 */
SOPC_ReturnStatus SOPC_CreateMonitoredItemsRequest_SetMonitoredItemIdFromStrings(
    OpcUa_CreateMonitoredItemsRequest* createMIrequest,
    size_t index,
    const char* nodeId,
    SOPC_AttributeId attribute,
    const char* indexRange);

/**
 * \brief Creates and allocates a DataChangeFilter filter parameter to be provided to
 *        ::SOPC_CreateMonitoredItemsRequest_SetMonitoredItemParams.
 *        See part 4 §7.17.2 for detailed DataChangeFilter documentation.
 *
 * \param trigger         The condition on which a data change notification is triggered:
 *                        Status, Status & Value or Status, Value and Timestamp.
 * \param deadbandType    The deadband type Absolute, Percent or None if no deadband shall be used.
 * \param deadbandValue  The deadband value to apply on Value change and for the deadband type (Absolute, Percent)
 *
 * \return SOPC_STATUS_OK in case of success,
 *         SOPC_STATUS_INVALID_PARAMETERS in case of invalid parameters.
 */
SOPC_ExtensionObject* SOPC_MonitoredItem_DataChangeFilter(OpcUa_DataChangeTrigger trigger,
                                                          OpcUa_DeadbandType deadbandType,
                                                          double deadbandValue);

/**
 * \brief Sets the monitored item monitoring parameters.
 *        It shall be completed by a call to ::SOPC_CreateMonitoredItemsRequest_SetMonitoredItemId
 *        for the same index to configure monitored item identification.
 *        See part 4 §7.16 for detailed MonitoringParameters documentation.
 *
 * \param createMIrequest  The create monitored item request to configure
 *
 * \param index            Index of the create monitored item to configure in the request.
 *                         \p index < number of monitored items configured in ::SOPC_CreateMonitoredItemsRequest_Create
 *
 * \param monitoringMode   The monitoring mode to use: disabled, sampling, reporting (default mode to be used)
 *
 * \param clientHandle     Client-supplied id of the MonitoredItem, it will be provided in Notifications of the
 *                         ::OpcUa_PublishResponse messages received.
 * \param samplingInterval The interval defines the sampling interval for the monitored item.
 *                         The value 0 indicates that the Server should use the fastest practical rate
 *                         or is based on an exception-based model.
 *                         The value -1 indicates that the default sampling interval defined by the publishing
 *                         interval of the subscription is requested.
 * \param filter           (optional) A filter used by the Server to determine if the MonitoredItem should generate a
 *                         notification.
 *                         ::SOPC_MonitoredItem_DataChangeFilter should be used to create a DataChangeFilter.
 *                         If not used, this parameter is null. If not null the filter memory is managed by the
 *                         function after call.
 * \param queueSize        The requested size of the monitored item queue.
 * \param discardOldest    If set to true the oldest notification is discarded when the queue is full,
 *                         otherwise the last notification is discarded when the queue is full.
 *
 * \return SOPC_STATUS_OK in case of success,
 *         SOPC_STATUS_INVALID_PARAMETERS in case of invalid parameters.
 *
 * \note  Only exception-based model (see \p samplingInterval) and DataChangeFilter filter are supported by s2opc server
 */
SOPC_ReturnStatus SOPC_CreateMonitoredItemsRequest_SetMonitoredItemParams(
    OpcUa_CreateMonitoredItemsRequest* createMIrequest,
    size_t index,
    OpcUa_MonitoringMode monitoringMode,
    uint32_t clientHandle,
    double samplingInterval,
    SOPC_ExtensionObject* filter,
    uint32_t queueSize,
    SOPC_Boolean discardOldest);

/**
 * \brief Creates an ModifyMonitoredItems request
 *
 * \param subscriptionId        The identifier of the subscription for which monitored items will be created
 *                              (obtained with ::SOPC_ClientHelperNew_Subscription_GetSubscriptionId
 *                               or in a ::OpcUa_CreateSubscriptionResponse if subscription is managed manually)
 * \param nbMonitoredItems      Number of MonitoredItem to modify with the request.
 *                              \p nbMonitoredItems <= INT32_MAX.
 *                              ::SOPC_CreateMonitoredItemsRequest_SetMonitoredItemParams shall be called
 *                              for each monitored item index.
 * \param ts                    Set the timestamps (source, server) to be returned for any monitored item.
 *
 * \return allocated ModifyMonitoredItems request in case of success, NULL in case of failure
 *         (invalid parameters or out of memory)
 */
OpcUa_ModifyMonitoredItemsRequest* SOPC_ModifyMonitoredItemsRequest_Create(uint32_t subscriptionId,
                                                                           size_t nbMonitoredItems,
                                                                           OpcUa_TimestampsToReturn ts);

/**
 * \brief Sets the monitored item monitoring parameters to modify.
 *        See part 4 §7.16 for detailed MonitoringParameters documentation.
 *
 * \param modifyMIrequest  The modify monitored item request to configure
 * \param index            Index of the create monitored item to configure in the request.
 *                         \p index < number of monitored items configured in ::SOPC_ModifyMonitoredItemsRequest_Create
 * \param monitoredItemId  The monitored item identifier returned in the ::OpcUa_CreateMonitoredItemsResponse.
 * \param clientHandle     Client-supplied id of the MonitoredItem, it will be provided in Notifications of the
 *                         ::OpcUa_PublishResponse messages received.
 * \param samplingInterval The interval defines the sampling interval for the monitored item.
 *                         The value 0 indicates that the Server should use the fastest practical rate
 *                         or is based on an exception-based model.
 *                         The value -1 indicates that the default sampling interval defined by the publishing
 *                         interval of the subscription is requested.
 * \param filter           (optional) A filter used by the Server to determine if the MonitoredItem should generate a
 *                         notification.
 *                         ::SOPC_MonitoredItem_DataChangeFilter should be used to create a DataChangeFilter.
 *                         If not used, this parameter is null. If not null the filter memory is managed by the
 *                         function after call.
 * \param queueSize        The requested size of the monitored item queue.
 * \param discardOldest    If set to true the oldest notification is discarded when the queue is full,
 *                         otherwise the last notification is discarded when the queue is full.
 *
 * \return SOPC_STATUS_OK in case of success,
 *         SOPC_STATUS_INVALID_PARAMETERS in case of invalid parameters.
 *
 * \note  Only exception-based model (see \p samplingInterval) and DataChangeFilter filter are supported by s2opc server
 */
SOPC_ReturnStatus SOPC_ModifyMonitoredItemsRequest_SetMonitoredItemParams(
    OpcUa_ModifyMonitoredItemsRequest* modifyMIrequest,
    size_t index,
    uint32_t monitoredItemId,
    uint32_t clientHandle,
    double samplingInterval,
    SOPC_ExtensionObject* filter,
    uint32_t queueSize,
    SOPC_Boolean discardOldest);

/**
 * \brief Creates an DeleteMonitoredItems request
 *
 * \param subscriptionId        The identifier of the subscription for which monitored items will be deleted
 *                              (obtained with ::SOPC_ClientHelperNew_Subscription_GetSubscriptionId
 *                               or in a ::OpcUa_CreateSubscriptionResponse if subscription is managed manually)
 * \param nbMonitoredItems      Number of MonitoredItem to delete with the request.
 *                              \p nbMonitoredItems <= INT32_MAX.
 *                              ::SOPC_CreateMonitoredItemsRequest_SetMonitoredItemParams shall be called
 *                              for each monitored item index.
 * \param optMonitoredItemIds   (optional) Pointer to the array of monitored item ids to delete.
 *                              If it is NULL, the id of each monitored item shall be provided using
 *                              ::SOPC_DeleteMonitoredItemsRequest_SetMonitoredItemId.
 *
 * \return allocated DeleteMonitoredItems request in case of success, NULL in case of failure
 *         (invalid parameters or out of memory)
 */
OpcUa_DeleteMonitoredItemsRequest* SOPC_DeleteMonitoredItemsRequest_Create(uint32_t subscriptionId,
                                                                           size_t nbMonitoredItems,
                                                                           const uint32_t** optMonitoredItemIds);

/**
 * \brief Sets the monitored item identifier to delete.
 *
 * \param deleteMIrequest  The delete monitored item request to configure
 * \param index            Index of the create monitored item to configure in the request.
 *                         \p index < number of monitored items configured in ::SOPC_DeleteMonitoredItemsRequest_Create
 * \param monitoredItemId  The MonitoredItem id to delete
 *
 * \return SOPC_STATUS_OK in case of success, SOPC_STATUS_INVALID_PARAMETERS in case of invalid parameters.
 */
SOPC_ReturnStatus SOPC_DeleteMonitoredItemsRequest_SetMonitoredItemId(
    OpcUa_DeleteMonitoredItemsRequest* deleteMIrequest,
    size_t index,
    uint32_t monitoredItemId);

#endif /* LIBS2OPC_REQUEST_BUILDER_H_ */
