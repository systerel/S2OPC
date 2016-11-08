/* ========================================================================
 * Copyright (c) 2005-2016 The OPC Foundation, Inc. All rights reserved.
 *
 * OPC Foundation MIT License 1.00
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * The complete license agreement can be found here:
 * http://opcfoundation.org/License/MIT/1.00/
 * ======================================================================*/

#ifndef _UA_ClientApi_H_
#define _UA_ClientApi_H_ 1
#ifdef OPCUA_HAVE_CLIENTAPI

#include <ua_types.h>
#include <ua_channel.h>

BEGIN_EXTERN_C

#ifndef OPCUA_EXCLUDE_FindServers
/*============================================================================
 * Synchronously calls the FindServers service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_FindServers(
    UA_Channel                     hChannel,
    const OpcUa_RequestHeader*     pRequestHeader,
    const UA_String*               pEndpointUrl,
    int32_t                        nNoOfLocaleIds,
    const UA_String*               pLocaleIds,
    int32_t                        nNoOfServerUris,
    const UA_String*               pServerUris,
    OpcUa_ResponseHeader*          pResponseHeader,
    int32_t*                       pNoOfServers,
    OpcUa_ApplicationDescription** pServers);

/*============================================================================
 * Asynchronously calls the FindServers service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginFindServers(
    UA_Channel                     hChannel,
    const OpcUa_RequestHeader*     pRequestHeader,
    const UA_String*               pEndpointUrl,
    int32_t                        nNoOfLocaleIds,
    const UA_String*               pLocaleIds,
    int32_t                        nNoOfServerUris,
    const UA_String*               pServerUris,
    UA_Channel_PfnRequestComplete* pCallback,
    void*                          pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_FindServersOnNetwork
/*============================================================================
 * Synchronously calls the FindServersOnNetwork service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_FindServersOnNetwork(
    UA_Channel                 hChannel,
    const OpcUa_RequestHeader* pRequestHeader,
    uint32_t                   nStartingRecordId,
    uint32_t                   nMaxRecordsToReturn,
    int32_t                    nNoOfServerCapabilityFilter,
    const UA_String*           pServerCapabilityFilter,
    OpcUa_ResponseHeader*      pResponseHeader,
    UA_DateTime*               pLastCounterResetTime,
    int32_t*                   pNoOfServers,
    OpcUa_ServerOnNetwork**    pServers);

/*============================================================================
 * Asynchronously calls the FindServersOnNetwork service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginFindServersOnNetwork(
    UA_Channel                     hChannel,
    const OpcUa_RequestHeader*     pRequestHeader,
    uint32_t                       nStartingRecordId,
    uint32_t                       nMaxRecordsToReturn,
    int32_t                        nNoOfServerCapabilityFilter,
    const UA_String*               pServerCapabilityFilter,
    UA_Channel_PfnRequestComplete* pCallback,
    void*                          pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_GetEndpoints
/*============================================================================
 * Synchronously calls the GetEndpoints service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_GetEndpoints(
    UA_Channel                  hChannel,
    const OpcUa_RequestHeader*  pRequestHeader,
    const UA_String*            pEndpointUrl,
    int32_t                     nNoOfLocaleIds,
    const UA_String*            pLocaleIds,
    int32_t                     nNoOfProfileUris,
    const UA_String*            pProfileUris,
    OpcUa_ResponseHeader*       pResponseHeader,
    int32_t*                    pNoOfEndpoints,
    OpcUa_EndpointDescription** pEndpoints);

/*============================================================================
 * Asynchronously calls the GetEndpoints service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginGetEndpoints(
    UA_Channel                     hChannel,
    const OpcUa_RequestHeader*     pRequestHeader,
    const UA_String*               pEndpointUrl,
    int32_t                        nNoOfLocaleIds,
    const UA_String*               pLocaleIds,
    int32_t                        nNoOfProfileUris,
    const UA_String*               pProfileUris,
    UA_Channel_PfnRequestComplete* pCallback,
    void*                          pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_RegisterServer
/*============================================================================
 * Synchronously calls the RegisterServer service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_RegisterServer(
    UA_Channel                    hChannel,
    const OpcUa_RequestHeader*    pRequestHeader,
    const OpcUa_RegisteredServer* pServer,
    OpcUa_ResponseHeader*         pResponseHeader);

/*============================================================================
 * Asynchronously calls the RegisterServer service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginRegisterServer(
    UA_Channel                     hChannel,
    const OpcUa_RequestHeader*     pRequestHeader,
    const OpcUa_RegisteredServer*  pServer,
    UA_Channel_PfnRequestComplete* pCallback,
    void*                          pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_RegisterServer2
/*============================================================================
 * Synchronously calls the RegisterServer2 service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_RegisterServer2(
    UA_Channel                    hChannel,
    const OpcUa_RequestHeader*    pRequestHeader,
    const OpcUa_RegisteredServer* pServer,
    int32_t                       nNoOfDiscoveryConfiguration,
    const UA_ExtensionObject*     pDiscoveryConfiguration,
    OpcUa_ResponseHeader*         pResponseHeader,
    int32_t*                      pNoOfConfigurationResults,
    SOPC_StatusCode**                  pConfigurationResults,
    int32_t*                      pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**           pDiagnosticInfos);

/*============================================================================
 * Asynchronously calls the RegisterServer2 service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginRegisterServer2(
    UA_Channel                     hChannel,
    const OpcUa_RequestHeader*     pRequestHeader,
    const OpcUa_RegisteredServer*  pServer,
    int32_t                        nNoOfDiscoveryConfiguration,
    const UA_ExtensionObject*      pDiscoveryConfiguration,
    UA_Channel_PfnRequestComplete* pCallback,
    void*                          pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_CreateSession
/*============================================================================
 * Synchronously calls the CreateSession service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_CreateSession(
    UA_Channel                          hChannel,
    const OpcUa_RequestHeader*          pRequestHeader,
    const OpcUa_ApplicationDescription* pClientDescription,
    const UA_String*                    pServerUri,
    const UA_String*                    pEndpointUrl,
    const UA_String*                    pSessionName,
    const UA_ByteString*                pClientNonce,
    const UA_ByteString*                pClientCertificate,
    double                              nRequestedSessionTimeout,
    uint32_t                            nMaxResponseMessageSize,
    OpcUa_ResponseHeader*               pResponseHeader,
    UA_NodeId*                          pSessionId,
    UA_NodeId*                          pAuthenticationToken,
    double*                             pRevisedSessionTimeout,
    UA_ByteString*                      pServerNonce,
    UA_ByteString*                      pServerCertificate,
    int32_t*                            pNoOfServerEndpoints,
    OpcUa_EndpointDescription**         pServerEndpoints,
    int32_t*                            pNoOfServerSoftwareCertificates,
    OpcUa_SignedSoftwareCertificate**   pServerSoftwareCertificates,
    OpcUa_SignatureData*                pServerSignature,
    uint32_t*                           pMaxRequestMessageSize);

/*============================================================================
 * Asynchronously calls the CreateSession service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginCreateSession(
    UA_Channel                          hChannel,
    const OpcUa_RequestHeader*          pRequestHeader,
    const OpcUa_ApplicationDescription* pClientDescription,
    const UA_String*                    pServerUri,
    const UA_String*                    pEndpointUrl,
    const UA_String*                    pSessionName,
    const UA_ByteString*                pClientNonce,
    const UA_ByteString*                pClientCertificate,
    double                              nRequestedSessionTimeout,
    uint32_t                            nMaxResponseMessageSize,
    UA_Channel_PfnRequestComplete*      pCallback,
    void*                               pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_ActivateSession
/*============================================================================
 * Synchronously calls the ActivateSession service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_ActivateSession(
    UA_Channel                             hChannel,
    const OpcUa_RequestHeader*             pRequestHeader,
    const OpcUa_SignatureData*             pClientSignature,
    int32_t                                nNoOfClientSoftwareCertificates,
    const OpcUa_SignedSoftwareCertificate* pClientSoftwareCertificates,
    int32_t                                nNoOfLocaleIds,
    const UA_String*                       pLocaleIds,
    const UA_ExtensionObject*              pUserIdentityToken,
    const OpcUa_SignatureData*             pUserTokenSignature,
    OpcUa_ResponseHeader*                  pResponseHeader,
    UA_ByteString*                         pServerNonce,
    int32_t*                               pNoOfResults,
    SOPC_StatusCode**                           pResults,
    int32_t*                               pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**                    pDiagnosticInfos);

/*============================================================================
 * Asynchronously calls the ActivateSession service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginActivateSession(
    UA_Channel                             hChannel,
    const OpcUa_RequestHeader*             pRequestHeader,
    const OpcUa_SignatureData*             pClientSignature,
    int32_t                                nNoOfClientSoftwareCertificates,
    const OpcUa_SignedSoftwareCertificate* pClientSoftwareCertificates,
    int32_t                                nNoOfLocaleIds,
    const UA_String*                       pLocaleIds,
    const UA_ExtensionObject*              pUserIdentityToken,
    const OpcUa_SignatureData*             pUserTokenSignature,
    UA_Channel_PfnRequestComplete*         pCallback,
    void*                                  pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_CloseSession
/*============================================================================
 * Synchronously calls the CloseSession service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_CloseSession(
    UA_Channel                 hChannel,
    const OpcUa_RequestHeader* pRequestHeader,
    UA_Boolean                 bDeleteSubscriptions,
    OpcUa_ResponseHeader*      pResponseHeader);

/*============================================================================
 * Asynchronously calls the CloseSession service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginCloseSession(
    UA_Channel                     hChannel,
    const OpcUa_RequestHeader*     pRequestHeader,
    UA_Boolean                     bDeleteSubscriptions,
    UA_Channel_PfnRequestComplete* pCallback,
    void*                          pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_Cancel
/*============================================================================
 * Synchronously calls the Cancel service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_Cancel(
    UA_Channel                 hChannel,
    const OpcUa_RequestHeader* pRequestHeader,
    uint32_t                   nRequestHandle,
    OpcUa_ResponseHeader*      pResponseHeader,
    uint32_t*                  pCancelCount);

/*============================================================================
 * Asynchronously calls the Cancel service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginCancel(
    UA_Channel                     hChannel,
    const OpcUa_RequestHeader*     pRequestHeader,
    uint32_t                       nRequestHandle,
    UA_Channel_PfnRequestComplete* pCallback,
    void*                          pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_AddNodes
/*============================================================================
 * Synchronously calls the AddNodes service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_AddNodes(
    UA_Channel                 hChannel,
    const OpcUa_RequestHeader* pRequestHeader,
    int32_t                    nNoOfNodesToAdd,
    const OpcUa_AddNodesItem*  pNodesToAdd,
    OpcUa_ResponseHeader*      pResponseHeader,
    int32_t*                   pNoOfResults,
    OpcUa_AddNodesResult**     pResults,
    int32_t*                   pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**        pDiagnosticInfos);

/*============================================================================
 * Asynchronously calls the AddNodes service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginAddNodes(
    UA_Channel                     hChannel,
    const OpcUa_RequestHeader*     pRequestHeader,
    int32_t                        nNoOfNodesToAdd,
    const OpcUa_AddNodesItem*      pNodesToAdd,
    UA_Channel_PfnRequestComplete* pCallback,
    void*                          pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_AddReferences
/*============================================================================
 * Synchronously calls the AddReferences service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_AddReferences(
    UA_Channel                     hChannel,
    const OpcUa_RequestHeader*     pRequestHeader,
    int32_t                        nNoOfReferencesToAdd,
    const OpcUa_AddReferencesItem* pReferencesToAdd,
    OpcUa_ResponseHeader*          pResponseHeader,
    int32_t*                       pNoOfResults,
    SOPC_StatusCode**                   pResults,
    int32_t*                       pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**            pDiagnosticInfos);

/*============================================================================
 * Asynchronously calls the AddReferences service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginAddReferences(
    UA_Channel                     hChannel,
    const OpcUa_RequestHeader*     pRequestHeader,
    int32_t                        nNoOfReferencesToAdd,
    const OpcUa_AddReferencesItem* pReferencesToAdd,
    UA_Channel_PfnRequestComplete* pCallback,
    void*                          pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_DeleteNodes
/*============================================================================
 * Synchronously calls the DeleteNodes service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_DeleteNodes(
    UA_Channel                   hChannel,
    const OpcUa_RequestHeader*   pRequestHeader,
    int32_t                      nNoOfNodesToDelete,
    const OpcUa_DeleteNodesItem* pNodesToDelete,
    OpcUa_ResponseHeader*        pResponseHeader,
    int32_t*                     pNoOfResults,
    SOPC_StatusCode**                 pResults,
    int32_t*                     pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**          pDiagnosticInfos);

/*============================================================================
 * Asynchronously calls the DeleteNodes service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginDeleteNodes(
    UA_Channel                     hChannel,
    const OpcUa_RequestHeader*     pRequestHeader,
    int32_t                        nNoOfNodesToDelete,
    const OpcUa_DeleteNodesItem*   pNodesToDelete,
    UA_Channel_PfnRequestComplete* pCallback,
    void*                          pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_DeleteReferences
/*============================================================================
 * Synchronously calls the DeleteReferences service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_DeleteReferences(
    UA_Channel                        hChannel,
    const OpcUa_RequestHeader*        pRequestHeader,
    int32_t                           nNoOfReferencesToDelete,
    const OpcUa_DeleteReferencesItem* pReferencesToDelete,
    OpcUa_ResponseHeader*             pResponseHeader,
    int32_t*                          pNoOfResults,
    SOPC_StatusCode**                      pResults,
    int32_t*                          pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**               pDiagnosticInfos);

/*============================================================================
 * Asynchronously calls the DeleteReferences service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginDeleteReferences(
    UA_Channel                        hChannel,
    const OpcUa_RequestHeader*        pRequestHeader,
    int32_t                           nNoOfReferencesToDelete,
    const OpcUa_DeleteReferencesItem* pReferencesToDelete,
    UA_Channel_PfnRequestComplete*    pCallback,
    void*                             pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_Browse
/*============================================================================
 * Synchronously calls the Browse service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_Browse(
    UA_Channel                     hChannel,
    const OpcUa_RequestHeader*     pRequestHeader,
    const OpcUa_ViewDescription*   pView,
    uint32_t                       nRequestedMaxReferencesPerNode,
    int32_t                        nNoOfNodesToBrowse,
    const OpcUa_BrowseDescription* pNodesToBrowse,
    OpcUa_ResponseHeader*          pResponseHeader,
    int32_t*                       pNoOfResults,
    OpcUa_BrowseResult**           pResults,
    int32_t*                       pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**            pDiagnosticInfos);

/*============================================================================
 * Asynchronously calls the Browse service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginBrowse(
    UA_Channel                     hChannel,
    const OpcUa_RequestHeader*     pRequestHeader,
    const OpcUa_ViewDescription*   pView,
    uint32_t                       nRequestedMaxReferencesPerNode,
    int32_t                        nNoOfNodesToBrowse,
    const OpcUa_BrowseDescription* pNodesToBrowse,
    UA_Channel_PfnRequestComplete* pCallback,
    void*                          pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_BrowseNext
/*============================================================================
 * Synchronously calls the BrowseNext service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BrowseNext(
    UA_Channel                 hChannel,
    const OpcUa_RequestHeader* pRequestHeader,
    UA_Boolean                 bReleaseContinuationPoints,
    int32_t                    nNoOfContinuationPoints,
    const UA_ByteString*       pContinuationPoints,
    OpcUa_ResponseHeader*      pResponseHeader,
    int32_t*                   pNoOfResults,
    OpcUa_BrowseResult**       pResults,
    int32_t*                   pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**        pDiagnosticInfos);

/*============================================================================
 * Asynchronously calls the BrowseNext service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginBrowseNext(
    UA_Channel                     hChannel,
    const OpcUa_RequestHeader*     pRequestHeader,
    UA_Boolean                     bReleaseContinuationPoints,
    int32_t                        nNoOfContinuationPoints,
    const UA_ByteString*           pContinuationPoints,
    UA_Channel_PfnRequestComplete* pCallback,
    void*                          pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_TranslateBrowsePathsToNodeIds
/*============================================================================
 * Synchronously calls the TranslateBrowsePathsToNodeIds service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_TranslateBrowsePathsToNodeIds(
    UA_Channel                 hChannel,
    const OpcUa_RequestHeader* pRequestHeader,
    int32_t                    nNoOfBrowsePaths,
    const OpcUa_BrowsePath*    pBrowsePaths,
    OpcUa_ResponseHeader*      pResponseHeader,
    int32_t*                   pNoOfResults,
    OpcUa_BrowsePathResult**   pResults,
    int32_t*                   pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**        pDiagnosticInfos);

/*============================================================================
 * Asynchronously calls the TranslateBrowsePathsToNodeIds service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginTranslateBrowsePathsToNodeIds(
    UA_Channel                     hChannel,
    const OpcUa_RequestHeader*     pRequestHeader,
    int32_t                        nNoOfBrowsePaths,
    const OpcUa_BrowsePath*        pBrowsePaths,
    UA_Channel_PfnRequestComplete* pCallback,
    void*                          pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_RegisterNodes
/*============================================================================
 * Synchronously calls the RegisterNodes service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_RegisterNodes(
    UA_Channel                 hChannel,
    const OpcUa_RequestHeader* pRequestHeader,
    int32_t                    nNoOfNodesToRegister,
    const UA_NodeId*           pNodesToRegister,
    OpcUa_ResponseHeader*      pResponseHeader,
    int32_t*                   pNoOfRegisteredNodeIds,
    UA_NodeId**                pRegisteredNodeIds);

/*============================================================================
 * Asynchronously calls the RegisterNodes service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginRegisterNodes(
    UA_Channel                     hChannel,
    const OpcUa_RequestHeader*     pRequestHeader,
    int32_t                        nNoOfNodesToRegister,
    const UA_NodeId*               pNodesToRegister,
    UA_Channel_PfnRequestComplete* pCallback,
    void*                          pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_UnregisterNodes
/*============================================================================
 * Synchronously calls the UnregisterNodes service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_UnregisterNodes(
    UA_Channel                 hChannel,
    const OpcUa_RequestHeader* pRequestHeader,
    int32_t                    nNoOfNodesToUnregister,
    const UA_NodeId*           pNodesToUnregister,
    OpcUa_ResponseHeader*      pResponseHeader);

/*============================================================================
 * Asynchronously calls the UnregisterNodes service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginUnregisterNodes(
    UA_Channel                     hChannel,
    const OpcUa_RequestHeader*     pRequestHeader,
    int32_t                        nNoOfNodesToUnregister,
    const UA_NodeId*               pNodesToUnregister,
    UA_Channel_PfnRequestComplete* pCallback,
    void*                          pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_QueryFirst
/*============================================================================
 * Synchronously calls the QueryFirst service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_QueryFirst(
    UA_Channel                       hChannel,
    const OpcUa_RequestHeader*       pRequestHeader,
    const OpcUa_ViewDescription*     pView,
    int32_t                          nNoOfNodeTypes,
    const OpcUa_NodeTypeDescription* pNodeTypes,
    const OpcUa_ContentFilter*       pFilter,
    uint32_t                         nMaxDataSetsToReturn,
    uint32_t                         nMaxReferencesToReturn,
    OpcUa_ResponseHeader*            pResponseHeader,
    int32_t*                         pNoOfQueryDataSets,
    OpcUa_QueryDataSet**             pQueryDataSets,
    UA_ByteString*                   pContinuationPoint,
    int32_t*                         pNoOfParsingResults,
    OpcUa_ParsingResult**            pParsingResults,
    int32_t*                         pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**              pDiagnosticInfos,
    OpcUa_ContentFilterResult*       pFilterResult);

/*============================================================================
 * Asynchronously calls the QueryFirst service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginQueryFirst(
    UA_Channel                       hChannel,
    const OpcUa_RequestHeader*       pRequestHeader,
    const OpcUa_ViewDescription*     pView,
    int32_t                          nNoOfNodeTypes,
    const OpcUa_NodeTypeDescription* pNodeTypes,
    const OpcUa_ContentFilter*       pFilter,
    uint32_t                         nMaxDataSetsToReturn,
    uint32_t                         nMaxReferencesToReturn,
    UA_Channel_PfnRequestComplete*   pCallback,
    void*                            pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_QueryNext
/*============================================================================
 * Synchronously calls the QueryNext service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_QueryNext(
    UA_Channel                 hChannel,
    const OpcUa_RequestHeader* pRequestHeader,
    UA_Boolean                 bReleaseContinuationPoint,
    const UA_ByteString*       pContinuationPoint,
    OpcUa_ResponseHeader*      pResponseHeader,
    int32_t*                   pNoOfQueryDataSets,
    OpcUa_QueryDataSet**       pQueryDataSets,
    UA_ByteString*             pRevisedContinuationPoint);

/*============================================================================
 * Asynchronously calls the QueryNext service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginQueryNext(
    UA_Channel                     hChannel,
    const OpcUa_RequestHeader*     pRequestHeader,
    UA_Boolean                     bReleaseContinuationPoint,
    const UA_ByteString*           pContinuationPoint,
    UA_Channel_PfnRequestComplete* pCallback,
    void*                          pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_Read
/*============================================================================
 * Synchronously calls the Read service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_Read(
    UA_Channel                 hChannel,
    const OpcUa_RequestHeader* pRequestHeader,
    double                     nMaxAge,
    OpcUa_TimestampsToReturn   eTimestampsToReturn,
    int32_t                    nNoOfNodesToRead,
    const OpcUa_ReadValueId*   pNodesToRead,
    OpcUa_ResponseHeader*      pResponseHeader,
    int32_t*                   pNoOfResults,
    UA_DataValue**             pResults,
    int32_t*                   pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**        pDiagnosticInfos);

/*============================================================================
 * Asynchronously calls the Read service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginRead(
    UA_Channel                     hChannel,
    const OpcUa_RequestHeader*     pRequestHeader,
    double                         nMaxAge,
    OpcUa_TimestampsToReturn       eTimestampsToReturn,
    int32_t                        nNoOfNodesToRead,
    const OpcUa_ReadValueId*       pNodesToRead,
    UA_Channel_PfnRequestComplete* pCallback,
    void*                          pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_HistoryRead
/*============================================================================
 * Synchronously calls the HistoryRead service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_HistoryRead(
    UA_Channel                      hChannel,
    const OpcUa_RequestHeader*      pRequestHeader,
    const UA_ExtensionObject*       pHistoryReadDetails,
    OpcUa_TimestampsToReturn        eTimestampsToReturn,
    UA_Boolean                      bReleaseContinuationPoints,
    int32_t                         nNoOfNodesToRead,
    const OpcUa_HistoryReadValueId* pNodesToRead,
    OpcUa_ResponseHeader*           pResponseHeader,
    int32_t*                        pNoOfResults,
    OpcUa_HistoryReadResult**       pResults,
    int32_t*                        pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**             pDiagnosticInfos);

/*============================================================================
 * Asynchronously calls the HistoryRead service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginHistoryRead(
    UA_Channel                      hChannel,
    const OpcUa_RequestHeader*      pRequestHeader,
    const UA_ExtensionObject*       pHistoryReadDetails,
    OpcUa_TimestampsToReturn        eTimestampsToReturn,
    UA_Boolean                      bReleaseContinuationPoints,
    int32_t                         nNoOfNodesToRead,
    const OpcUa_HistoryReadValueId* pNodesToRead,
    UA_Channel_PfnRequestComplete*  pCallback,
    void*                           pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_Write
/*============================================================================
 * Synchronously calls the Write service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_Write(
    UA_Channel                 hChannel,
    const OpcUa_RequestHeader* pRequestHeader,
    int32_t                    nNoOfNodesToWrite,
    const OpcUa_WriteValue*    pNodesToWrite,
    OpcUa_ResponseHeader*      pResponseHeader,
    int32_t*                   pNoOfResults,
    SOPC_StatusCode**               pResults,
    int32_t*                   pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**        pDiagnosticInfos);

/*============================================================================
 * Asynchronously calls the Write service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginWrite(
    UA_Channel                     hChannel,
    const OpcUa_RequestHeader*     pRequestHeader,
    int32_t                        nNoOfNodesToWrite,
    const OpcUa_WriteValue*        pNodesToWrite,
    UA_Channel_PfnRequestComplete* pCallback,
    void*                          pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_HistoryUpdate
/*============================================================================
 * Synchronously calls the HistoryUpdate service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_HistoryUpdate(
    UA_Channel                  hChannel,
    const OpcUa_RequestHeader*  pRequestHeader,
    int32_t                     nNoOfHistoryUpdateDetails,
    const UA_ExtensionObject*   pHistoryUpdateDetails,
    OpcUa_ResponseHeader*       pResponseHeader,
    int32_t*                    pNoOfResults,
    OpcUa_HistoryUpdateResult** pResults,
    int32_t*                    pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**         pDiagnosticInfos);

/*============================================================================
 * Asynchronously calls the HistoryUpdate service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginHistoryUpdate(
    UA_Channel                     hChannel,
    const OpcUa_RequestHeader*     pRequestHeader,
    int32_t                        nNoOfHistoryUpdateDetails,
    const UA_ExtensionObject*      pHistoryUpdateDetails,
    UA_Channel_PfnRequestComplete* pCallback,
    void*                          pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_Call
/*============================================================================
 * Synchronously calls the Call service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_Call(
    UA_Channel                     hChannel,
    const OpcUa_RequestHeader*     pRequestHeader,
    int32_t                        nNoOfMethodsToCall,
    const OpcUa_CallMethodRequest* pMethodsToCall,
    OpcUa_ResponseHeader*          pResponseHeader,
    int32_t*                       pNoOfResults,
    OpcUa_CallMethodResult**       pResults,
    int32_t*                       pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**            pDiagnosticInfos);

/*============================================================================
 * Asynchronously calls the Call service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginCall(
    UA_Channel                     hChannel,
    const OpcUa_RequestHeader*     pRequestHeader,
    int32_t                        nNoOfMethodsToCall,
    const OpcUa_CallMethodRequest* pMethodsToCall,
    UA_Channel_PfnRequestComplete* pCallback,
    void*                          pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_CreateMonitoredItems
/*============================================================================
 * Synchronously calls the CreateMonitoredItems service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_CreateMonitoredItems(
    UA_Channel                              hChannel,
    const OpcUa_RequestHeader*              pRequestHeader,
    uint32_t                                nSubscriptionId,
    OpcUa_TimestampsToReturn                eTimestampsToReturn,
    int32_t                                 nNoOfItemsToCreate,
    const OpcUa_MonitoredItemCreateRequest* pItemsToCreate,
    OpcUa_ResponseHeader*                   pResponseHeader,
    int32_t*                                pNoOfResults,
    OpcUa_MonitoredItemCreateResult**       pResults,
    int32_t*                                pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**                     pDiagnosticInfos);

/*============================================================================
 * Asynchronously calls the CreateMonitoredItems service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginCreateMonitoredItems(
    UA_Channel                              hChannel,
    const OpcUa_RequestHeader*              pRequestHeader,
    uint32_t                                nSubscriptionId,
    OpcUa_TimestampsToReturn                eTimestampsToReturn,
    int32_t                                 nNoOfItemsToCreate,
    const OpcUa_MonitoredItemCreateRequest* pItemsToCreate,
    UA_Channel_PfnRequestComplete*          pCallback,
    void*                                   pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_ModifyMonitoredItems
/*============================================================================
 * Synchronously calls the ModifyMonitoredItems service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_ModifyMonitoredItems(
    UA_Channel                              hChannel,
    const OpcUa_RequestHeader*              pRequestHeader,
    uint32_t                                nSubscriptionId,
    OpcUa_TimestampsToReturn                eTimestampsToReturn,
    int32_t                                 nNoOfItemsToModify,
    const OpcUa_MonitoredItemModifyRequest* pItemsToModify,
    OpcUa_ResponseHeader*                   pResponseHeader,
    int32_t*                                pNoOfResults,
    OpcUa_MonitoredItemModifyResult**       pResults,
    int32_t*                                pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**                     pDiagnosticInfos);

/*============================================================================
 * Asynchronously calls the ModifyMonitoredItems service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginModifyMonitoredItems(
    UA_Channel                              hChannel,
    const OpcUa_RequestHeader*              pRequestHeader,
    uint32_t                                nSubscriptionId,
    OpcUa_TimestampsToReturn                eTimestampsToReturn,
    int32_t                                 nNoOfItemsToModify,
    const OpcUa_MonitoredItemModifyRequest* pItemsToModify,
    UA_Channel_PfnRequestComplete*          pCallback,
    void*                                   pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_SetMonitoringMode
/*============================================================================
 * Synchronously calls the SetMonitoringMode service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_SetMonitoringMode(
    UA_Channel                 hChannel,
    const OpcUa_RequestHeader* pRequestHeader,
    uint32_t                   nSubscriptionId,
    OpcUa_MonitoringMode       eMonitoringMode,
    int32_t                    nNoOfMonitoredItemIds,
    const uint32_t*            pMonitoredItemIds,
    OpcUa_ResponseHeader*      pResponseHeader,
    int32_t*                   pNoOfResults,
    SOPC_StatusCode**               pResults,
    int32_t*                   pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**        pDiagnosticInfos);

/*============================================================================
 * Asynchronously calls the SetMonitoringMode service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginSetMonitoringMode(
    UA_Channel                     hChannel,
    const OpcUa_RequestHeader*     pRequestHeader,
    uint32_t                       nSubscriptionId,
    OpcUa_MonitoringMode           eMonitoringMode,
    int32_t                        nNoOfMonitoredItemIds,
    const uint32_t*                pMonitoredItemIds,
    UA_Channel_PfnRequestComplete* pCallback,
    void*                          pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_SetTriggering
/*============================================================================
 * Synchronously calls the SetTriggering service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_SetTriggering(
    UA_Channel                 hChannel,
    const OpcUa_RequestHeader* pRequestHeader,
    uint32_t                   nSubscriptionId,
    uint32_t                   nTriggeringItemId,
    int32_t                    nNoOfLinksToAdd,
    const uint32_t*            pLinksToAdd,
    int32_t                    nNoOfLinksToRemove,
    const uint32_t*            pLinksToRemove,
    OpcUa_ResponseHeader*      pResponseHeader,
    int32_t*                   pNoOfAddResults,
    SOPC_StatusCode**               pAddResults,
    int32_t*                   pNoOfAddDiagnosticInfos,
    UA_DiagnosticInfo**        pAddDiagnosticInfos,
    int32_t*                   pNoOfRemoveResults,
    SOPC_StatusCode**               pRemoveResults,
    int32_t*                   pNoOfRemoveDiagnosticInfos,
    UA_DiagnosticInfo**        pRemoveDiagnosticInfos);

/*============================================================================
 * Asynchronously calls the SetTriggering service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginSetTriggering(
    UA_Channel                     hChannel,
    const OpcUa_RequestHeader*     pRequestHeader,
    uint32_t                       nSubscriptionId,
    uint32_t                       nTriggeringItemId,
    int32_t                        nNoOfLinksToAdd,
    const uint32_t*                pLinksToAdd,
    int32_t                        nNoOfLinksToRemove,
    const uint32_t*                pLinksToRemove,
    UA_Channel_PfnRequestComplete* pCallback,
    void*                          pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_DeleteMonitoredItems
/*============================================================================
 * Synchronously calls the DeleteMonitoredItems service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_DeleteMonitoredItems(
    UA_Channel                 hChannel,
    const OpcUa_RequestHeader* pRequestHeader,
    uint32_t                   nSubscriptionId,
    int32_t                    nNoOfMonitoredItemIds,
    const uint32_t*            pMonitoredItemIds,
    OpcUa_ResponseHeader*      pResponseHeader,
    int32_t*                   pNoOfResults,
    SOPC_StatusCode**               pResults,
    int32_t*                   pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**        pDiagnosticInfos);

/*============================================================================
 * Asynchronously calls the DeleteMonitoredItems service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginDeleteMonitoredItems(
    UA_Channel                     hChannel,
    const OpcUa_RequestHeader*     pRequestHeader,
    uint32_t                       nSubscriptionId,
    int32_t                        nNoOfMonitoredItemIds,
    const uint32_t*                pMonitoredItemIds,
    UA_Channel_PfnRequestComplete* pCallback,
    void*                          pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_CreateSubscription
/*============================================================================
 * Synchronously calls the CreateSubscription service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_CreateSubscription(
    UA_Channel                 hChannel,
    const OpcUa_RequestHeader* pRequestHeader,
    double                     nRequestedPublishingInterval,
    uint32_t                   nRequestedLifetimeCount,
    uint32_t                   nRequestedMaxKeepAliveCount,
    uint32_t                   nMaxNotificationsPerPublish,
    UA_Boolean                 bPublishingEnabled,
    UA_Byte                    nPriority,
    OpcUa_ResponseHeader*      pResponseHeader,
    uint32_t*                  pSubscriptionId,
    double*                    pRevisedPublishingInterval,
    uint32_t*                  pRevisedLifetimeCount,
    uint32_t*                  pRevisedMaxKeepAliveCount);

/*============================================================================
 * Asynchronously calls the CreateSubscription service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginCreateSubscription(
    UA_Channel                     hChannel,
    const OpcUa_RequestHeader*     pRequestHeader,
    double                         nRequestedPublishingInterval,
    uint32_t                       nRequestedLifetimeCount,
    uint32_t                       nRequestedMaxKeepAliveCount,
    uint32_t                       nMaxNotificationsPerPublish,
    UA_Boolean                     bPublishingEnabled,
    UA_Byte                        nPriority,
    UA_Channel_PfnRequestComplete* pCallback,
    void*                          pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_ModifySubscription
/*============================================================================
 * Synchronously calls the ModifySubscription service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_ModifySubscription(
    UA_Channel                 hChannel,
    const OpcUa_RequestHeader* pRequestHeader,
    uint32_t                   nSubscriptionId,
    double                     nRequestedPublishingInterval,
    uint32_t                   nRequestedLifetimeCount,
    uint32_t                   nRequestedMaxKeepAliveCount,
    uint32_t                   nMaxNotificationsPerPublish,
    UA_Byte                    nPriority,
    OpcUa_ResponseHeader*      pResponseHeader,
    double*                    pRevisedPublishingInterval,
    uint32_t*                  pRevisedLifetimeCount,
    uint32_t*                  pRevisedMaxKeepAliveCount);

/*============================================================================
 * Asynchronously calls the ModifySubscription service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginModifySubscription(
    UA_Channel                     hChannel,
    const OpcUa_RequestHeader*     pRequestHeader,
    uint32_t                       nSubscriptionId,
    double                         nRequestedPublishingInterval,
    uint32_t                       nRequestedLifetimeCount,
    uint32_t                       nRequestedMaxKeepAliveCount,
    uint32_t                       nMaxNotificationsPerPublish,
    UA_Byte                        nPriority,
    UA_Channel_PfnRequestComplete* pCallback,
    void*                          pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_SetPublishingMode
/*============================================================================
 * Synchronously calls the SetPublishingMode service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_SetPublishingMode(
    UA_Channel                 hChannel,
    const OpcUa_RequestHeader* pRequestHeader,
    UA_Boolean                 bPublishingEnabled,
    int32_t                    nNoOfSubscriptionIds,
    const uint32_t*            pSubscriptionIds,
    OpcUa_ResponseHeader*      pResponseHeader,
    int32_t*                   pNoOfResults,
    SOPC_StatusCode**               pResults,
    int32_t*                   pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**        pDiagnosticInfos);

/*============================================================================
 * Asynchronously calls the SetPublishingMode service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginSetPublishingMode(
    UA_Channel                     hChannel,
    const OpcUa_RequestHeader*     pRequestHeader,
    UA_Boolean                     bPublishingEnabled,
    int32_t                        nNoOfSubscriptionIds,
    const uint32_t*                pSubscriptionIds,
    UA_Channel_PfnRequestComplete* pCallback,
    void*                          pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_Publish
/*============================================================================
 * Synchronously calls the Publish service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_Publish(
    UA_Channel                               hChannel,
    const OpcUa_RequestHeader*               pRequestHeader,
    int32_t                                  nNoOfSubscriptionAcknowledgements,
    const OpcUa_SubscriptionAcknowledgement* pSubscriptionAcknowledgements,
    OpcUa_ResponseHeader*                    pResponseHeader,
    uint32_t*                                pSubscriptionId,
    int32_t*                                 pNoOfAvailableSequenceNumbers,
    uint32_t**                               pAvailableSequenceNumbers,
    UA_Boolean*                              pMoreNotifications,
    OpcUa_NotificationMessage*               pNotificationMessage,
    int32_t*                                 pNoOfResults,
    SOPC_StatusCode**                             pResults,
    int32_t*                                 pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**                      pDiagnosticInfos);

/*============================================================================
 * Asynchronously calls the Publish service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginPublish(
    UA_Channel                               hChannel,
    const OpcUa_RequestHeader*               pRequestHeader,
    int32_t                                  nNoOfSubscriptionAcknowledgements,
    const OpcUa_SubscriptionAcknowledgement* pSubscriptionAcknowledgements,
    UA_Channel_PfnRequestComplete*           pCallback,
    void*                                    pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_Republish
/*============================================================================
 * Synchronously calls the Republish service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_Republish(
    UA_Channel                 hChannel,
    const OpcUa_RequestHeader* pRequestHeader,
    uint32_t                   nSubscriptionId,
    uint32_t                   nRetransmitSequenceNumber,
    OpcUa_ResponseHeader*      pResponseHeader,
    OpcUa_NotificationMessage* pNotificationMessage);

/*============================================================================
 * Asynchronously calls the Republish service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginRepublish(
    UA_Channel                     hChannel,
    const OpcUa_RequestHeader*     pRequestHeader,
    uint32_t                       nSubscriptionId,
    uint32_t                       nRetransmitSequenceNumber,
    UA_Channel_PfnRequestComplete* pCallback,
    void*                          pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_TransferSubscriptions
/*============================================================================
 * Synchronously calls the TransferSubscriptions service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_TransferSubscriptions(
    UA_Channel                 hChannel,
    const OpcUa_RequestHeader* pRequestHeader,
    int32_t                    nNoOfSubscriptionIds,
    const uint32_t*            pSubscriptionIds,
    UA_Boolean                 bSendInitialValues,
    OpcUa_ResponseHeader*      pResponseHeader,
    int32_t*                   pNoOfResults,
    OpcUa_TransferResult**     pResults,
    int32_t*                   pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**        pDiagnosticInfos);

/*============================================================================
 * Asynchronously calls the TransferSubscriptions service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginTransferSubscriptions(
    UA_Channel                     hChannel,
    const OpcUa_RequestHeader*     pRequestHeader,
    int32_t                        nNoOfSubscriptionIds,
    const uint32_t*                pSubscriptionIds,
    UA_Boolean                     bSendInitialValues,
    UA_Channel_PfnRequestComplete* pCallback,
    void*                          pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_DeleteSubscriptions
/*============================================================================
 * Synchronously calls the DeleteSubscriptions service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_DeleteSubscriptions(
    UA_Channel                 hChannel,
    const OpcUa_RequestHeader* pRequestHeader,
    int32_t                    nNoOfSubscriptionIds,
    const uint32_t*            pSubscriptionIds,
    OpcUa_ResponseHeader*      pResponseHeader,
    int32_t*                   pNoOfResults,
    SOPC_StatusCode**               pResults,
    int32_t*                   pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**        pDiagnosticInfos);

/*============================================================================
 * Asynchronously calls the DeleteSubscriptions service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginDeleteSubscriptions(
    UA_Channel                     hChannel,
    const OpcUa_RequestHeader*     pRequestHeader,
    int32_t                        nNoOfSubscriptionIds,
    const uint32_t*                pSubscriptionIds,
    UA_Channel_PfnRequestComplete* pCallback,
    void*                          pCallbackData);
#endif

END_EXTERN_C

#endif /* OPCUA_HAVE_CLIENTAPI */
#endif /* _UA_ClientApi_H_ */
/* This is the last line of an autogenerated file. */
