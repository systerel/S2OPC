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

OPCUA_BEGIN_EXTERN_C

#ifndef OPCUA_EXCLUDE_FindServers
/*============================================================================
 * Synchronously calls the FindServers service.
 *===========================================================================*/
StatusCode UA_ClientApi_FindServers(
    OpcUa_Channel               hChannel,
    const UA_RequestHeader*     pRequestHeader,
    const UA_String*            pEndpointUrl,
    int32_t                     nNoOfLocaleIds,
    const UA_String*            pLocaleIds,
    int32_t                     nNoOfServerUris,
    const UA_String*            pServerUris,
    UA_ResponseHeader*          pResponseHeader,
    int32_t*                    pNoOfServers,
    UA_ApplicationDescription** pServers);

/*============================================================================
 * Asynchronously calls the FindServers service.
 *===========================================================================*/
StatusCode UA_ClientApi_BeginFindServers(
    OpcUa_Channel                     hChannel,
    const UA_RequestHeader*           pRequestHeader,
    const UA_String*                  pEndpointUrl,
    int32_t                           nNoOfLocaleIds,
    const UA_String*                  pLocaleIds,
    int32_t                           nNoOfServerUris,
    const UA_String*                  pServerUris,
    UA_Channel_PfnRequestComplete*    pCallback,
    OpcUa_Void*                       pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_FindServersOnNetwork
/*============================================================================
 * Synchronously calls the FindServersOnNetwork service.
 *===========================================================================*/
StatusCode UA_ClientApi_FindServersOnNetwork(
    OpcUa_Channel           hChannel,
    const UA_RequestHeader* pRequestHeader,
    uint32_t                nStartingRecordId,
    uint32_t                nMaxRecordsToReturn,
    int32_t                 nNoOfServerCapabilityFilter,
    const UA_String*        pServerCapabilityFilter,
    UA_ResponseHeader*      pResponseHeader,
    UA_DateTime*            pLastCounterResetTime,
    int32_t*                pNoOfServers,
    UA_ServerOnNetwork**    pServers);

/*============================================================================
 * Asynchronously calls the FindServersOnNetwork service.
 *===========================================================================*/
StatusCode UA_ClientApi_BeginFindServersOnNetwork(
    OpcUa_Channel                     hChannel,
    const UA_RequestHeader*           pRequestHeader,
    uint32_t                          nStartingRecordId,
    uint32_t                          nMaxRecordsToReturn,
    int32_t                           nNoOfServerCapabilityFilter,
    const UA_String*                  pServerCapabilityFilter,
    UA_Channel_PfnRequestComplete*    pCallback,
    OpcUa_Void*                       pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_GetEndpoints
/*============================================================================
 * Synchronously calls the GetEndpoints service.
 *===========================================================================*/
StatusCode UA_ClientApi_GetEndpoints(
    OpcUa_Channel            hChannel,
    const UA_RequestHeader*  pRequestHeader,
    const UA_String*         pEndpointUrl,
    int32_t                  nNoOfLocaleIds,
    const UA_String*         pLocaleIds,
    int32_t                  nNoOfProfileUris,
    const UA_String*         pProfileUris,
    UA_ResponseHeader*       pResponseHeader,
    int32_t*                 pNoOfEndpoints,
    UA_EndpointDescription** pEndpoints);

/*============================================================================
 * Asynchronously calls the GetEndpoints service.
 *===========================================================================*/
StatusCode UA_ClientApi_BeginGetEndpoints(
    OpcUa_Channel                     hChannel,
    const UA_RequestHeader*           pRequestHeader,
    const UA_String*                  pEndpointUrl,
    int32_t                           nNoOfLocaleIds,
    const UA_String*                  pLocaleIds,
    int32_t                           nNoOfProfileUris,
    const UA_String*                  pProfileUris,
    UA_Channel_PfnRequestComplete*    pCallback,
    OpcUa_Void*                       pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_RegisterServer
/*============================================================================
 * Synchronously calls the RegisterServer service.
 *===========================================================================*/
StatusCode UA_ClientApi_RegisterServer(
    OpcUa_Channel              hChannel,
    const UA_RequestHeader*    pRequestHeader,
    const UA_RegisteredServer* pServer,
    UA_ResponseHeader*         pResponseHeader);

/*============================================================================
 * Asynchronously calls the RegisterServer service.
 *===========================================================================*/
StatusCode UA_ClientApi_BeginRegisterServer(
    OpcUa_Channel                     hChannel,
    const UA_RequestHeader*           pRequestHeader,
    const UA_RegisteredServer*        pServer,
    UA_Channel_PfnRequestComplete*    pCallback,
    OpcUa_Void*                       pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_RegisterServer2
/*============================================================================
 * Synchronously calls the RegisterServer2 service.
 *===========================================================================*/
StatusCode UA_ClientApi_RegisterServer2(
    OpcUa_Channel              hChannel,
    const UA_RequestHeader*    pRequestHeader,
    const UA_RegisteredServer* pServer,
    int32_t                    nNoOfDiscoveryConfiguration,
    const UA_ExtensionObject*  pDiscoveryConfiguration,
    UA_ResponseHeader*         pResponseHeader,
    int32_t*                   pNoOfConfigurationResults,
    StatusCode**               pConfigurationResults,
    int32_t*                   pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**        pDiagnosticInfos);

/*============================================================================
 * Asynchronously calls the RegisterServer2 service.
 *===========================================================================*/
StatusCode UA_ClientApi_BeginRegisterServer2(
    OpcUa_Channel                     hChannel,
    const UA_RequestHeader*           pRequestHeader,
    const UA_RegisteredServer*        pServer,
    int32_t                           nNoOfDiscoveryConfiguration,
    const UA_ExtensionObject*         pDiscoveryConfiguration,
    UA_Channel_PfnRequestComplete*    pCallback,
    OpcUa_Void*                       pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_CreateSession
/*============================================================================
 * Synchronously calls the CreateSession service.
 *===========================================================================*/
StatusCode UA_ClientApi_CreateSession(
    OpcUa_Channel                    hChannel,
    const UA_RequestHeader*          pRequestHeader,
    const UA_ApplicationDescription* pClientDescription,
    const UA_String*                 pServerUri,
    const UA_String*                 pEndpointUrl,
    const UA_String*                 pSessionName,
    const UA_ByteString*             pClientNonce,
    const UA_ByteString*             pClientCertificate,
    double                           nRequestedSessionTimeout,
    uint32_t                         nMaxResponseMessageSize,
    UA_ResponseHeader*               pResponseHeader,
    UA_NodeId*                       pSessionId,
    UA_NodeId*                       pAuthenticationToken,
    double*                          pRevisedSessionTimeout,
    UA_ByteString*                   pServerNonce,
    UA_ByteString*                   pServerCertificate,
    int32_t*                         pNoOfServerEndpoints,
    UA_EndpointDescription**         pServerEndpoints,
    int32_t*                         pNoOfServerSoftwareCertificates,
    UA_SignedSoftwareCertificate**   pServerSoftwareCertificates,
    UA_SignatureData*                pServerSignature,
    uint32_t*                        pMaxRequestMessageSize);

/*============================================================================
 * Asynchronously calls the CreateSession service.
 *===========================================================================*/
StatusCode UA_ClientApi_BeginCreateSession(
    OpcUa_Channel                     hChannel,
    const UA_RequestHeader*           pRequestHeader,
    const UA_ApplicationDescription*  pClientDescription,
    const UA_String*                  pServerUri,
    const UA_String*                  pEndpointUrl,
    const UA_String*                  pSessionName,
    const UA_ByteString*              pClientNonce,
    const UA_ByteString*              pClientCertificate,
    double                            nRequestedSessionTimeout,
    uint32_t                          nMaxResponseMessageSize,
    UA_Channel_PfnRequestComplete*    pCallback,
    OpcUa_Void*                       pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_ActivateSession
/*============================================================================
 * Synchronously calls the ActivateSession service.
 *===========================================================================*/
StatusCode UA_ClientApi_ActivateSession(
    OpcUa_Channel                       hChannel,
    const UA_RequestHeader*             pRequestHeader,
    const UA_SignatureData*             pClientSignature,
    int32_t                             nNoOfClientSoftwareCertificates,
    const UA_SignedSoftwareCertificate* pClientSoftwareCertificates,
    int32_t                             nNoOfLocaleIds,
    const UA_String*                    pLocaleIds,
    const UA_ExtensionObject*           pUserIdentityToken,
    const UA_SignatureData*             pUserTokenSignature,
    UA_ResponseHeader*                  pResponseHeader,
    UA_ByteString*                      pServerNonce,
    int32_t*                            pNoOfResults,
    StatusCode**                        pResults,
    int32_t*                            pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**                 pDiagnosticInfos);

/*============================================================================
 * Asynchronously calls the ActivateSession service.
 *===========================================================================*/
StatusCode UA_ClientApi_BeginActivateSession(
    OpcUa_Channel                       hChannel,
    const UA_RequestHeader*             pRequestHeader,
    const UA_SignatureData*             pClientSignature,
    int32_t                             nNoOfClientSoftwareCertificates,
    const UA_SignedSoftwareCertificate* pClientSoftwareCertificates,
    int32_t                             nNoOfLocaleIds,
    const UA_String*                    pLocaleIds,
    const UA_ExtensionObject*           pUserIdentityToken,
    const UA_SignatureData*             pUserTokenSignature,
    UA_Channel_PfnRequestComplete*      pCallback,
    OpcUa_Void*                         pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_CloseSession
/*============================================================================
 * Synchronously calls the CloseSession service.
 *===========================================================================*/
StatusCode UA_ClientApi_CloseSession(
    OpcUa_Channel           hChannel,
    const UA_RequestHeader* pRequestHeader,
    UA_Boolean              bDeleteSubscriptions,
    UA_ResponseHeader*      pResponseHeader);

/*============================================================================
 * Asynchronously calls the CloseSession service.
 *===========================================================================*/
StatusCode UA_ClientApi_BeginCloseSession(
    OpcUa_Channel                     hChannel,
    const UA_RequestHeader*           pRequestHeader,
    UA_Boolean                        bDeleteSubscriptions,
    UA_Channel_PfnRequestComplete*    pCallback,
    OpcUa_Void*                       pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_Cancel
/*============================================================================
 * Synchronously calls the Cancel service.
 *===========================================================================*/
StatusCode UA_ClientApi_Cancel(
    OpcUa_Channel           hChannel,
    const UA_RequestHeader* pRequestHeader,
    uint32_t                nRequestHandle,
    UA_ResponseHeader*      pResponseHeader,
    uint32_t*               pCancelCount);

/*============================================================================
 * Asynchronously calls the Cancel service.
 *===========================================================================*/
StatusCode UA_ClientApi_BeginCancel(
    OpcUa_Channel                     hChannel,
    const UA_RequestHeader*           pRequestHeader,
    uint32_t                          nRequestHandle,
    UA_Channel_PfnRequestComplete*    pCallback,
    OpcUa_Void*                       pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_AddNodes
/*============================================================================
 * Synchronously calls the AddNodes service.
 *===========================================================================*/
StatusCode UA_ClientApi_AddNodes(
    OpcUa_Channel           hChannel,
    const UA_RequestHeader* pRequestHeader,
    int32_t                 nNoOfNodesToAdd,
    const UA_AddNodesItem*  pNodesToAdd,
    UA_ResponseHeader*      pResponseHeader,
    int32_t*                pNoOfResults,
    UA_AddNodesResult**     pResults,
    int32_t*                pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**     pDiagnosticInfos);

/*============================================================================
 * Asynchronously calls the AddNodes service.
 *===========================================================================*/
StatusCode UA_ClientApi_BeginAddNodes(
    OpcUa_Channel                     hChannel,
    const UA_RequestHeader*           pRequestHeader,
    int32_t                           nNoOfNodesToAdd,
    const UA_AddNodesItem*            pNodesToAdd,
    UA_Channel_PfnRequestComplete*    pCallback,
    OpcUa_Void*                       pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_AddReferences
/*============================================================================
 * Synchronously calls the AddReferences service.
 *===========================================================================*/
StatusCode UA_ClientApi_AddReferences(
    OpcUa_Channel               hChannel,
    const UA_RequestHeader*     pRequestHeader,
    int32_t                     nNoOfReferencesToAdd,
    const UA_AddReferencesItem* pReferencesToAdd,
    UA_ResponseHeader*          pResponseHeader,
    int32_t*                    pNoOfResults,
    StatusCode**                pResults,
    int32_t*                    pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**         pDiagnosticInfos);

/*============================================================================
 * Asynchronously calls the AddReferences service.
 *===========================================================================*/
StatusCode UA_ClientApi_BeginAddReferences(
    OpcUa_Channel                     hChannel,
    const UA_RequestHeader*           pRequestHeader,
    int32_t                           nNoOfReferencesToAdd,
    const UA_AddReferencesItem*       pReferencesToAdd,
    UA_Channel_PfnRequestComplete*    pCallback,
    OpcUa_Void*                       pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_DeleteNodes
/*============================================================================
 * Synchronously calls the DeleteNodes service.
 *===========================================================================*/
StatusCode UA_ClientApi_DeleteNodes(
    OpcUa_Channel             hChannel,
    const UA_RequestHeader*   pRequestHeader,
    int32_t                   nNoOfNodesToDelete,
    const UA_DeleteNodesItem* pNodesToDelete,
    UA_ResponseHeader*        pResponseHeader,
    int32_t*                  pNoOfResults,
    StatusCode**              pResults,
    int32_t*                  pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**       pDiagnosticInfos);

/*============================================================================
 * Asynchronously calls the DeleteNodes service.
 *===========================================================================*/
StatusCode UA_ClientApi_BeginDeleteNodes(
    OpcUa_Channel                     hChannel,
    const UA_RequestHeader*           pRequestHeader,
    int32_t                           nNoOfNodesToDelete,
    const UA_DeleteNodesItem*         pNodesToDelete,
    UA_Channel_PfnRequestComplete*    pCallback,
    OpcUa_Void*                       pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_DeleteReferences
/*============================================================================
 * Synchronously calls the DeleteReferences service.
 *===========================================================================*/
StatusCode UA_ClientApi_DeleteReferences(
    OpcUa_Channel                  hChannel,
    const UA_RequestHeader*        pRequestHeader,
    int32_t                        nNoOfReferencesToDelete,
    const UA_DeleteReferencesItem* pReferencesToDelete,
    UA_ResponseHeader*             pResponseHeader,
    int32_t*                       pNoOfResults,
    StatusCode**                   pResults,
    int32_t*                       pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**            pDiagnosticInfos);

/*============================================================================
 * Asynchronously calls the DeleteReferences service.
 *===========================================================================*/
StatusCode UA_ClientApi_BeginDeleteReferences(
    OpcUa_Channel                     hChannel,
    const UA_RequestHeader*           pRequestHeader,
    int32_t                           nNoOfReferencesToDelete,
    const UA_DeleteReferencesItem*    pReferencesToDelete,
    UA_Channel_PfnRequestComplete*    pCallback,
    OpcUa_Void*                       pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_Browse
/*============================================================================
 * Synchronously calls the Browse service.
 *===========================================================================*/
StatusCode UA_ClientApi_Browse(
    OpcUa_Channel               hChannel,
    const UA_RequestHeader*     pRequestHeader,
    const UA_ViewDescription*   pView,
    uint32_t                    nRequestedMaxReferencesPerNode,
    int32_t                     nNoOfNodesToBrowse,
    const UA_BrowseDescription* pNodesToBrowse,
    UA_ResponseHeader*          pResponseHeader,
    int32_t*                    pNoOfResults,
    UA_BrowseResult**           pResults,
    int32_t*                    pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**         pDiagnosticInfos);

/*============================================================================
 * Asynchronously calls the Browse service.
 *===========================================================================*/
StatusCode UA_ClientApi_BeginBrowse(
    OpcUa_Channel                     hChannel,
    const UA_RequestHeader*           pRequestHeader,
    const UA_ViewDescription*         pView,
    uint32_t                          nRequestedMaxReferencesPerNode,
    int32_t                           nNoOfNodesToBrowse,
    const UA_BrowseDescription*       pNodesToBrowse,
    UA_Channel_PfnRequestComplete*    pCallback,
    OpcUa_Void*                       pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_BrowseNext
/*============================================================================
 * Synchronously calls the BrowseNext service.
 *===========================================================================*/
StatusCode UA_ClientApi_BrowseNext(
    OpcUa_Channel           hChannel,
    const UA_RequestHeader* pRequestHeader,
    UA_Boolean              bReleaseContinuationPoints,
    int32_t                 nNoOfContinuationPoints,
    const UA_ByteString*    pContinuationPoints,
    UA_ResponseHeader*      pResponseHeader,
    int32_t*                pNoOfResults,
    UA_BrowseResult**       pResults,
    int32_t*                pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**     pDiagnosticInfos);

/*============================================================================
 * Asynchronously calls the BrowseNext service.
 *===========================================================================*/
StatusCode UA_ClientApi_BeginBrowseNext(
    OpcUa_Channel                     hChannel,
    const UA_RequestHeader*           pRequestHeader,
    UA_Boolean                        bReleaseContinuationPoints,
    int32_t                           nNoOfContinuationPoints,
    const UA_ByteString*              pContinuationPoints,
    UA_Channel_PfnRequestComplete*    pCallback,
    OpcUa_Void*                       pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_TranslateBrowsePathsToNodeIds
/*============================================================================
 * Synchronously calls the TranslateBrowsePathsToNodeIds service.
 *===========================================================================*/
StatusCode UA_ClientApi_TranslateBrowsePathsToNodeIds(
    OpcUa_Channel           hChannel,
    const UA_RequestHeader* pRequestHeader,
    int32_t                 nNoOfBrowsePaths,
    const UA_BrowsePath*    pBrowsePaths,
    UA_ResponseHeader*      pResponseHeader,
    int32_t*                pNoOfResults,
    UA_BrowsePathResult**   pResults,
    int32_t*                pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**     pDiagnosticInfos);

/*============================================================================
 * Asynchronously calls the TranslateBrowsePathsToNodeIds service.
 *===========================================================================*/
StatusCode UA_ClientApi_BeginTranslateBrowsePathsToNodeIds(
    OpcUa_Channel                     hChannel,
    const UA_RequestHeader*           pRequestHeader,
    int32_t                           nNoOfBrowsePaths,
    const UA_BrowsePath*              pBrowsePaths,
    UA_Channel_PfnRequestComplete*    pCallback,
    OpcUa_Void*                       pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_RegisterNodes
/*============================================================================
 * Synchronously calls the RegisterNodes service.
 *===========================================================================*/
StatusCode UA_ClientApi_RegisterNodes(
    OpcUa_Channel           hChannel,
    const UA_RequestHeader* pRequestHeader,
    int32_t                 nNoOfNodesToRegister,
    const UA_NodeId*        pNodesToRegister,
    UA_ResponseHeader*      pResponseHeader,
    int32_t*                pNoOfRegisteredNodeIds,
    UA_NodeId**             pRegisteredNodeIds);

/*============================================================================
 * Asynchronously calls the RegisterNodes service.
 *===========================================================================*/
StatusCode UA_ClientApi_BeginRegisterNodes(
    OpcUa_Channel                     hChannel,
    const UA_RequestHeader*           pRequestHeader,
    int32_t                           nNoOfNodesToRegister,
    const UA_NodeId*                  pNodesToRegister,
    UA_Channel_PfnRequestComplete*    pCallback,
    OpcUa_Void*                       pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_UnregisterNodes
/*============================================================================
 * Synchronously calls the UnregisterNodes service.
 *===========================================================================*/
StatusCode UA_ClientApi_UnregisterNodes(
    OpcUa_Channel           hChannel,
    const UA_RequestHeader* pRequestHeader,
    int32_t                 nNoOfNodesToUnregister,
    const UA_NodeId*        pNodesToUnregister,
    UA_ResponseHeader*      pResponseHeader);

/*============================================================================
 * Asynchronously calls the UnregisterNodes service.
 *===========================================================================*/
StatusCode UA_ClientApi_BeginUnregisterNodes(
    OpcUa_Channel                     hChannel,
    const UA_RequestHeader*           pRequestHeader,
    int32_t                           nNoOfNodesToUnregister,
    const UA_NodeId*                  pNodesToUnregister,
    UA_Channel_PfnRequestComplete*    pCallback,
    OpcUa_Void*                       pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_QueryFirst
/*============================================================================
 * Synchronously calls the QueryFirst service.
 *===========================================================================*/
StatusCode UA_ClientApi_QueryFirst(
    OpcUa_Channel                 hChannel,
    const UA_RequestHeader*       pRequestHeader,
    const UA_ViewDescription*     pView,
    int32_t                       nNoOfNodeTypes,
    const UA_NodeTypeDescription* pNodeTypes,
    const UA_ContentFilter*       pFilter,
    uint32_t                      nMaxDataSetsToReturn,
    uint32_t                      nMaxReferencesToReturn,
    UA_ResponseHeader*            pResponseHeader,
    int32_t*                      pNoOfQueryDataSets,
    UA_QueryDataSet**             pQueryDataSets,
    UA_ByteString*                pContinuationPoint,
    int32_t*                      pNoOfParsingResults,
    UA_ParsingResult**            pParsingResults,
    int32_t*                      pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**           pDiagnosticInfos,
    UA_ContentFilterResult*       pFilterResult);

/*============================================================================
 * Asynchronously calls the QueryFirst service.
 *===========================================================================*/
StatusCode UA_ClientApi_BeginQueryFirst(
    OpcUa_Channel                     hChannel,
    const UA_RequestHeader*           pRequestHeader,
    const UA_ViewDescription*         pView,
    int32_t                           nNoOfNodeTypes,
    const UA_NodeTypeDescription*     pNodeTypes,
    const UA_ContentFilter*           pFilter,
    uint32_t                          nMaxDataSetsToReturn,
    uint32_t                          nMaxReferencesToReturn,
    UA_Channel_PfnRequestComplete*    pCallback,
    OpcUa_Void*                       pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_QueryNext
/*============================================================================
 * Synchronously calls the QueryNext service.
 *===========================================================================*/
StatusCode UA_ClientApi_QueryNext(
    OpcUa_Channel           hChannel,
    const UA_RequestHeader* pRequestHeader,
    UA_Boolean              bReleaseContinuationPoint,
    const UA_ByteString*    pContinuationPoint,
    UA_ResponseHeader*      pResponseHeader,
    int32_t*                pNoOfQueryDataSets,
    UA_QueryDataSet**       pQueryDataSets,
    UA_ByteString*          pRevisedContinuationPoint);

/*============================================================================
 * Asynchronously calls the QueryNext service.
 *===========================================================================*/
StatusCode UA_ClientApi_BeginQueryNext(
    OpcUa_Channel                     hChannel,
    const UA_RequestHeader*           pRequestHeader,
    UA_Boolean                        bReleaseContinuationPoint,
    const UA_ByteString*              pContinuationPoint,
    UA_Channel_PfnRequestComplete*    pCallback,
    OpcUa_Void*                       pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_Read
/*============================================================================
 * Synchronously calls the Read service.
 *===========================================================================*/
StatusCode UA_ClientApi_Read(
    OpcUa_Channel           hChannel,
    const UA_RequestHeader* pRequestHeader,
    double                  nMaxAge,
    UA_TimestampsToReturn   eTimestampsToReturn,
    int32_t                 nNoOfNodesToRead,
    const UA_ReadValueId*   pNodesToRead,
    UA_ResponseHeader*      pResponseHeader,
    int32_t*                pNoOfResults,
    UA_DataValue**          pResults,
    int32_t*                pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**     pDiagnosticInfos);

/*============================================================================
 * Asynchronously calls the Read service.
 *===========================================================================*/
StatusCode UA_ClientApi_BeginRead(
    OpcUa_Channel                     hChannel,
    const UA_RequestHeader*           pRequestHeader,
    double                            nMaxAge,
    UA_TimestampsToReturn             eTimestampsToReturn,
    int32_t                           nNoOfNodesToRead,
    const UA_ReadValueId*             pNodesToRead,
    UA_Channel_PfnRequestComplete*    pCallback,
    OpcUa_Void*                       pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_HistoryRead
/*============================================================================
 * Synchronously calls the HistoryRead service.
 *===========================================================================*/
StatusCode UA_ClientApi_HistoryRead(
    OpcUa_Channel                hChannel,
    const UA_RequestHeader*      pRequestHeader,
    const UA_ExtensionObject*    pHistoryReadDetails,
    UA_TimestampsToReturn        eTimestampsToReturn,
    UA_Boolean                   bReleaseContinuationPoints,
    int32_t                      nNoOfNodesToRead,
    const UA_HistoryReadValueId* pNodesToRead,
    UA_ResponseHeader*           pResponseHeader,
    int32_t*                     pNoOfResults,
    UA_HistoryReadResult**       pResults,
    int32_t*                     pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**          pDiagnosticInfos);

/*============================================================================
 * Asynchronously calls the HistoryRead service.
 *===========================================================================*/
StatusCode UA_ClientApi_BeginHistoryRead(
    OpcUa_Channel                     hChannel,
    const UA_RequestHeader*           pRequestHeader,
    const UA_ExtensionObject*         pHistoryReadDetails,
    UA_TimestampsToReturn             eTimestampsToReturn,
    UA_Boolean                        bReleaseContinuationPoints,
    int32_t                           nNoOfNodesToRead,
    const UA_HistoryReadValueId*      pNodesToRead,
    UA_Channel_PfnRequestComplete*    pCallback,
    OpcUa_Void*                       pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_Write
/*============================================================================
 * Synchronously calls the Write service.
 *===========================================================================*/
StatusCode UA_ClientApi_Write(
    OpcUa_Channel           hChannel,
    const UA_RequestHeader* pRequestHeader,
    int32_t                 nNoOfNodesToWrite,
    const UA_WriteValue*    pNodesToWrite,
    UA_ResponseHeader*      pResponseHeader,
    int32_t*                pNoOfResults,
    StatusCode**            pResults,
    int32_t*                pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**     pDiagnosticInfos);

/*============================================================================
 * Asynchronously calls the Write service.
 *===========================================================================*/
StatusCode UA_ClientApi_BeginWrite(
    OpcUa_Channel                     hChannel,
    const UA_RequestHeader*           pRequestHeader,
    int32_t                           nNoOfNodesToWrite,
    const UA_WriteValue*              pNodesToWrite,
    UA_Channel_PfnRequestComplete*    pCallback,
    OpcUa_Void*                       pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_HistoryUpdate
/*============================================================================
 * Synchronously calls the HistoryUpdate service.
 *===========================================================================*/
StatusCode UA_ClientApi_HistoryUpdate(
    OpcUa_Channel             hChannel,
    const UA_RequestHeader*   pRequestHeader,
    int32_t                   nNoOfHistoryUpdateDetails,
    const UA_ExtensionObject* pHistoryUpdateDetails,
    UA_ResponseHeader*        pResponseHeader,
    int32_t*                  pNoOfResults,
    UA_HistoryUpdateResult**  pResults,
    int32_t*                  pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**       pDiagnosticInfos);

/*============================================================================
 * Asynchronously calls the HistoryUpdate service.
 *===========================================================================*/
StatusCode UA_ClientApi_BeginHistoryUpdate(
    OpcUa_Channel                     hChannel,
    const UA_RequestHeader*           pRequestHeader,
    int32_t                           nNoOfHistoryUpdateDetails,
    const UA_ExtensionObject*         pHistoryUpdateDetails,
    UA_Channel_PfnRequestComplete*    pCallback,
    OpcUa_Void*                       pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_Call
/*============================================================================
 * Synchronously calls the Call service.
 *===========================================================================*/
StatusCode UA_ClientApi_Call(
    OpcUa_Channel               hChannel,
    const UA_RequestHeader*     pRequestHeader,
    int32_t                     nNoOfMethodsToCall,
    const UA_CallMethodRequest* pMethodsToCall,
    UA_ResponseHeader*          pResponseHeader,
    int32_t*                    pNoOfResults,
    UA_CallMethodResult**       pResults,
    int32_t*                    pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**         pDiagnosticInfos);

/*============================================================================
 * Asynchronously calls the Call service.
 *===========================================================================*/
StatusCode UA_ClientApi_BeginCall(
    OpcUa_Channel                     hChannel,
    const UA_RequestHeader*           pRequestHeader,
    int32_t                           nNoOfMethodsToCall,
    const UA_CallMethodRequest*       pMethodsToCall,
    UA_Channel_PfnRequestComplete*    pCallback,
    OpcUa_Void*                       pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_CreateMonitoredItems
/*============================================================================
 * Synchronously calls the CreateMonitoredItems service.
 *===========================================================================*/
StatusCode UA_ClientApi_CreateMonitoredItems(
    OpcUa_Channel                        hChannel,
    const UA_RequestHeader*              pRequestHeader,
    uint32_t                             nSubscriptionId,
    UA_TimestampsToReturn                eTimestampsToReturn,
    int32_t                              nNoOfItemsToCreate,
    const UA_MonitoredItemCreateRequest* pItemsToCreate,
    UA_ResponseHeader*                   pResponseHeader,
    int32_t*                             pNoOfResults,
    UA_MonitoredItemCreateResult**       pResults,
    int32_t*                             pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**                  pDiagnosticInfos);

/*============================================================================
 * Asynchronously calls the CreateMonitoredItems service.
 *===========================================================================*/
StatusCode UA_ClientApi_BeginCreateMonitoredItems(
    OpcUa_Channel                        hChannel,
    const UA_RequestHeader*              pRequestHeader,
    uint32_t                             nSubscriptionId,
    UA_TimestampsToReturn                eTimestampsToReturn,
    int32_t                              nNoOfItemsToCreate,
    const UA_MonitoredItemCreateRequest* pItemsToCreate,
    UA_Channel_PfnRequestComplete*       pCallback,
    OpcUa_Void*                          pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_ModifyMonitoredItems
/*============================================================================
 * Synchronously calls the ModifyMonitoredItems service.
 *===========================================================================*/
StatusCode UA_ClientApi_ModifyMonitoredItems(
    OpcUa_Channel                        hChannel,
    const UA_RequestHeader*              pRequestHeader,
    uint32_t                             nSubscriptionId,
    UA_TimestampsToReturn                eTimestampsToReturn,
    int32_t                              nNoOfItemsToModify,
    const UA_MonitoredItemModifyRequest* pItemsToModify,
    UA_ResponseHeader*                   pResponseHeader,
    int32_t*                             pNoOfResults,
    UA_MonitoredItemModifyResult**       pResults,
    int32_t*                             pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**                  pDiagnosticInfos);

/*============================================================================
 * Asynchronously calls the ModifyMonitoredItems service.
 *===========================================================================*/
StatusCode UA_ClientApi_BeginModifyMonitoredItems(
    OpcUa_Channel                        hChannel,
    const UA_RequestHeader*              pRequestHeader,
    uint32_t                             nSubscriptionId,
    UA_TimestampsToReturn                eTimestampsToReturn,
    int32_t                              nNoOfItemsToModify,
    const UA_MonitoredItemModifyRequest* pItemsToModify,
    UA_Channel_PfnRequestComplete*       pCallback,
    OpcUa_Void*                          pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_SetMonitoringMode
/*============================================================================
 * Synchronously calls the SetMonitoringMode service.
 *===========================================================================*/
StatusCode UA_ClientApi_SetMonitoringMode(
    OpcUa_Channel           hChannel,
    const UA_RequestHeader* pRequestHeader,
    uint32_t                nSubscriptionId,
    UA_MonitoringMode       eMonitoringMode,
    int32_t                 nNoOfMonitoredItemIds,
    const uint32_t*         pMonitoredItemIds,
    UA_ResponseHeader*      pResponseHeader,
    int32_t*                pNoOfResults,
    StatusCode**            pResults,
    int32_t*                pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**     pDiagnosticInfos);

/*============================================================================
 * Asynchronously calls the SetMonitoringMode service.
 *===========================================================================*/
StatusCode UA_ClientApi_BeginSetMonitoringMode(
    OpcUa_Channel                     hChannel,
    const UA_RequestHeader*           pRequestHeader,
    uint32_t                          nSubscriptionId,
    UA_MonitoringMode                 eMonitoringMode,
    int32_t                           nNoOfMonitoredItemIds,
    const uint32_t*                   pMonitoredItemIds,
    UA_Channel_PfnRequestComplete*    pCallback,
    OpcUa_Void*                       pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_SetTriggering
/*============================================================================
 * Synchronously calls the SetTriggering service.
 *===========================================================================*/
StatusCode UA_ClientApi_SetTriggering(
    OpcUa_Channel           hChannel,
    const UA_RequestHeader* pRequestHeader,
    uint32_t                nSubscriptionId,
    uint32_t                nTriggeringItemId,
    int32_t                 nNoOfLinksToAdd,
    const uint32_t*         pLinksToAdd,
    int32_t                 nNoOfLinksToRemove,
    const uint32_t*         pLinksToRemove,
    UA_ResponseHeader*      pResponseHeader,
    int32_t*                pNoOfAddResults,
    StatusCode**            pAddResults,
    int32_t*                pNoOfAddDiagnosticInfos,
    UA_DiagnosticInfo**     pAddDiagnosticInfos,
    int32_t*                pNoOfRemoveResults,
    StatusCode**            pRemoveResults,
    int32_t*                pNoOfRemoveDiagnosticInfos,
    UA_DiagnosticInfo**     pRemoveDiagnosticInfos);

/*============================================================================
 * Asynchronously calls the SetTriggering service.
 *===========================================================================*/
StatusCode UA_ClientApi_BeginSetTriggering(
    OpcUa_Channel                     hChannel,
    const UA_RequestHeader*           pRequestHeader,
    uint32_t                          nSubscriptionId,
    uint32_t                          nTriggeringItemId,
    int32_t                           nNoOfLinksToAdd,
    const uint32_t*                   pLinksToAdd,
    int32_t                           nNoOfLinksToRemove,
    const uint32_t*                   pLinksToRemove,
    UA_Channel_PfnRequestComplete*    pCallback,
    OpcUa_Void*                       pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_DeleteMonitoredItems
/*============================================================================
 * Synchronously calls the DeleteMonitoredItems service.
 *===========================================================================*/
StatusCode UA_ClientApi_DeleteMonitoredItems(
    OpcUa_Channel           hChannel,
    const UA_RequestHeader* pRequestHeader,
    uint32_t                nSubscriptionId,
    int32_t                 nNoOfMonitoredItemIds,
    const uint32_t*         pMonitoredItemIds,
    UA_ResponseHeader*      pResponseHeader,
    int32_t*                pNoOfResults,
    StatusCode**            pResults,
    int32_t*                pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**     pDiagnosticInfos);

/*============================================================================
 * Asynchronously calls the DeleteMonitoredItems service.
 *===========================================================================*/
StatusCode UA_ClientApi_BeginDeleteMonitoredItems(
    OpcUa_Channel                     hChannel,
    const UA_RequestHeader*           pRequestHeader,
    uint32_t                          nSubscriptionId,
    int32_t                           nNoOfMonitoredItemIds,
    const uint32_t*                   pMonitoredItemIds,
    UA_Channel_PfnRequestComplete*    pCallback,
    OpcUa_Void*                       pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_CreateSubscription
/*============================================================================
 * Synchronously calls the CreateSubscription service.
 *===========================================================================*/
StatusCode UA_ClientApi_CreateSubscription(
    OpcUa_Channel           hChannel,
    const UA_RequestHeader* pRequestHeader,
    double                  nRequestedPublishingInterval,
    uint32_t                nRequestedLifetimeCount,
    uint32_t                nRequestedMaxKeepAliveCount,
    uint32_t                nMaxNotificationsPerPublish,
    UA_Boolean              bPublishingEnabled,
    UA_Byte                 nPriority,
    UA_ResponseHeader*      pResponseHeader,
    uint32_t*               pSubscriptionId,
    double*                 pRevisedPublishingInterval,
    uint32_t*               pRevisedLifetimeCount,
    uint32_t*               pRevisedMaxKeepAliveCount);

/*============================================================================
 * Asynchronously calls the CreateSubscription service.
 *===========================================================================*/
StatusCode UA_ClientApi_BeginCreateSubscription(
    OpcUa_Channel                     hChannel,
    const UA_RequestHeader*           pRequestHeader,
    double                            nRequestedPublishingInterval,
    uint32_t                          nRequestedLifetimeCount,
    uint32_t                          nRequestedMaxKeepAliveCount,
    uint32_t                          nMaxNotificationsPerPublish,
    UA_Boolean                        bPublishingEnabled,
    UA_Byte                           nPriority,
    UA_Channel_PfnRequestComplete*    pCallback,
    OpcUa_Void*                       pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_ModifySubscription
/*============================================================================
 * Synchronously calls the ModifySubscription service.
 *===========================================================================*/
StatusCode UA_ClientApi_ModifySubscription(
    OpcUa_Channel           hChannel,
    const UA_RequestHeader* pRequestHeader,
    uint32_t                nSubscriptionId,
    double                  nRequestedPublishingInterval,
    uint32_t                nRequestedLifetimeCount,
    uint32_t                nRequestedMaxKeepAliveCount,
    uint32_t                nMaxNotificationsPerPublish,
    UA_Byte                 nPriority,
    UA_ResponseHeader*      pResponseHeader,
    double*                 pRevisedPublishingInterval,
    uint32_t*               pRevisedLifetimeCount,
    uint32_t*               pRevisedMaxKeepAliveCount);

/*============================================================================
 * Asynchronously calls the ModifySubscription service.
 *===========================================================================*/
StatusCode UA_ClientApi_BeginModifySubscription(
    OpcUa_Channel                     hChannel,
    const UA_RequestHeader*           pRequestHeader,
    uint32_t                          nSubscriptionId,
    double                            nRequestedPublishingInterval,
    uint32_t                          nRequestedLifetimeCount,
    uint32_t                          nRequestedMaxKeepAliveCount,
    uint32_t                          nMaxNotificationsPerPublish,
    UA_Byte                           nPriority,
    UA_Channel_PfnRequestComplete*    pCallback,
    OpcUa_Void*                       pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_SetPublishingMode
/*============================================================================
 * Synchronously calls the SetPublishingMode service.
 *===========================================================================*/
StatusCode UA_ClientApi_SetPublishingMode(
    OpcUa_Channel           hChannel,
    const UA_RequestHeader* pRequestHeader,
    UA_Boolean              bPublishingEnabled,
    int32_t                 nNoOfSubscriptionIds,
    const uint32_t*         pSubscriptionIds,
    UA_ResponseHeader*      pResponseHeader,
    int32_t*                pNoOfResults,
    StatusCode**            pResults,
    int32_t*                pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**     pDiagnosticInfos);

/*============================================================================
 * Asynchronously calls the SetPublishingMode service.
 *===========================================================================*/
StatusCode UA_ClientApi_BeginSetPublishingMode(
    OpcUa_Channel                     hChannel,
    const UA_RequestHeader*           pRequestHeader,
    UA_Boolean                        bPublishingEnabled,
    int32_t                           nNoOfSubscriptionIds,
    const uint32_t*                   pSubscriptionIds,
    UA_Channel_PfnRequestComplete*    pCallback,
    OpcUa_Void*                       pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_Publish
/*============================================================================
 * Synchronously calls the Publish service.
 *===========================================================================*/
StatusCode UA_ClientApi_Publish(
    OpcUa_Channel                         hChannel,
    const UA_RequestHeader*               pRequestHeader,
    int32_t                               nNoOfSubscriptionAcknowledgements,
    const UA_SubscriptionAcknowledgement* pSubscriptionAcknowledgements,
    UA_ResponseHeader*                    pResponseHeader,
    uint32_t*                             pSubscriptionId,
    int32_t*                              pNoOfAvailableSequenceNumbers,
    uint32_t**                            pAvailableSequenceNumbers,
    UA_Boolean*                           pMoreNotifications,
    UA_NotificationMessage*               pNotificationMessage,
    int32_t*                              pNoOfResults,
    StatusCode**                          pResults,
    int32_t*                              pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**                   pDiagnosticInfos);

/*============================================================================
 * Asynchronously calls the Publish service.
 *===========================================================================*/
StatusCode UA_ClientApi_BeginPublish(
    OpcUa_Channel                         hChannel,
    const UA_RequestHeader*               pRequestHeader,
    int32_t                               nNoOfSubscriptionAcknowledgements,
    const UA_SubscriptionAcknowledgement* pSubscriptionAcknowledgements,
    UA_Channel_PfnRequestComplete*        pCallback,
    OpcUa_Void*                           pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_Republish
/*============================================================================
 * Synchronously calls the Republish service.
 *===========================================================================*/
StatusCode UA_ClientApi_Republish(
    OpcUa_Channel           hChannel,
    const UA_RequestHeader* pRequestHeader,
    uint32_t                nSubscriptionId,
    uint32_t                nRetransmitSequenceNumber,
    UA_ResponseHeader*      pResponseHeader,
    UA_NotificationMessage* pNotificationMessage);

/*============================================================================
 * Asynchronously calls the Republish service.
 *===========================================================================*/
StatusCode UA_ClientApi_BeginRepublish(
    OpcUa_Channel                     hChannel,
    const UA_RequestHeader*           pRequestHeader,
    uint32_t                          nSubscriptionId,
    uint32_t                          nRetransmitSequenceNumber,
    UA_Channel_PfnRequestComplete*    pCallback,
    OpcUa_Void*                       pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_TransferSubscriptions
/*============================================================================
 * Synchronously calls the TransferSubscriptions service.
 *===========================================================================*/
StatusCode UA_ClientApi_TransferSubscriptions(
    OpcUa_Channel           hChannel,
    const UA_RequestHeader* pRequestHeader,
    int32_t                 nNoOfSubscriptionIds,
    const uint32_t*         pSubscriptionIds,
    UA_Boolean              bSendInitialValues,
    UA_ResponseHeader*      pResponseHeader,
    int32_t*                pNoOfResults,
    UA_TransferResult**     pResults,
    int32_t*                pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**     pDiagnosticInfos);

/*============================================================================
 * Asynchronously calls the TransferSubscriptions service.
 *===========================================================================*/
StatusCode UA_ClientApi_BeginTransferSubscriptions(
    OpcUa_Channel                     hChannel,
    const UA_RequestHeader*           pRequestHeader,
    int32_t                           nNoOfSubscriptionIds,
    const uint32_t*                   pSubscriptionIds,
    UA_Boolean                        bSendInitialValues,
    UA_Channel_PfnRequestComplete*    pCallback,
    OpcUa_Void*                       pCallbackData);
#endif

#ifndef OPCUA_EXCLUDE_DeleteSubscriptions
/*============================================================================
 * Synchronously calls the DeleteSubscriptions service.
 *===========================================================================*/
StatusCode UA_ClientApi_DeleteSubscriptions(
    OpcUa_Channel           hChannel,
    const UA_RequestHeader* pRequestHeader,
    int32_t                 nNoOfSubscriptionIds,
    const uint32_t*         pSubscriptionIds,
    UA_ResponseHeader*      pResponseHeader,
    int32_t*                pNoOfResults,
    StatusCode**            pResults,
    int32_t*                pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**     pDiagnosticInfos);

/*============================================================================
 * Asynchronously calls the DeleteSubscriptions service.
 *===========================================================================*/
StatusCode UA_ClientApi_BeginDeleteSubscriptions(
    OpcUa_Channel                     hChannel,
    const UA_RequestHeader*           pRequestHeader,
    int32_t                           nNoOfSubscriptionIds,
    const uint32_t*                   pSubscriptionIds,
    UA_Channel_PfnRequestComplete*    pCallback,
    OpcUa_Void*                       pCallbackData);
#endif

OPCUA_END_EXTERN_C

#endif /* OPCUA_HAVE_CLIENTAPI */
#endif /* _UA_ClientApi_H_ */
/* This is the last line of an autogenerated file. */
