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
 *
 * Modifications: adaptation for INGOPCS project
 * ======================================================================*/

#ifndef _SOPC_ServerApi_H_
#define _SOPC_ServerApi_H_ 1
#ifdef OPCUA_HAVE_SERVERAPI

#include "sopc_types.h"
#include "sopc_endpoint.h"

BEGIN_EXTERN_C

#ifndef OPCUA_EXCLUDE_FindServers
/*============================================================================
 * Synchronously calls the FindServers service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_FindServers(
    SOPC_Endpoint                  hEndpoint,
    struct SOPC_RequestContext*    hContext,
    const OpcUa_RequestHeader*     pRequestHeader,
    const SOPC_String*             pEndpointUrl,
    int32_t                        nNoOfLocaleIds,
    const SOPC_String*             pLocaleIds,
    int32_t                        nNoOfServerUris,
    const SOPC_String*             pServerUris,
    OpcUa_ResponseHeader*          pResponseHeader,
    int32_t*                       pNoOfServers,
    OpcUa_ApplicationDescription** pServers);

/*============================================================================
 * Begins processing of a FindServers service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginFindServers(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void *                      a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType);
#endif

#ifndef OPCUA_EXCLUDE_FindServersOnNetwork
/*============================================================================
 * Synchronously calls the FindServersOnNetwork service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_FindServersOnNetwork(
    SOPC_Endpoint               hEndpoint,
    struct SOPC_RequestContext* hContext,
    const OpcUa_RequestHeader*  pRequestHeader,
    uint32_t                    nStartingRecordId,
    uint32_t                    nMaxRecordsToReturn,
    int32_t                     nNoOfServerCapabilityFilter,
    const SOPC_String*          pServerCapabilityFilter,
    OpcUa_ResponseHeader*       pResponseHeader,
    SOPC_DateTime*              pLastCounterResetTime,
    int32_t*                    pNoOfServers,
    OpcUa_ServerOnNetwork**     pServers);

/*============================================================================
 * Begins processing of a FindServersOnNetwork service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginFindServersOnNetwork(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void *                      a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType);
#endif

#ifndef OPCUA_EXCLUDE_GetEndpoints
/*============================================================================
 * Synchronously calls the GetEndpoints service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_GetEndpoints(
    SOPC_Endpoint               hEndpoint,
    struct SOPC_RequestContext* hContext,
    const OpcUa_RequestHeader*  pRequestHeader,
    const SOPC_String*          pEndpointUrl,
    int32_t                     nNoOfLocaleIds,
    const SOPC_String*          pLocaleIds,
    int32_t                     nNoOfProfileUris,
    const SOPC_String*          pProfileUris,
    OpcUa_ResponseHeader*       pResponseHeader,
    int32_t*                    pNoOfEndpoints,
    OpcUa_EndpointDescription** pEndpoints);

/*============================================================================
 * Begins processing of a GetEndpoints service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginGetEndpoints(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void *                      a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType);
#endif

#ifndef OPCUA_EXCLUDE_RegisterServer
/*============================================================================
 * Synchronously calls the RegisterServer service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_RegisterServer(
    SOPC_Endpoint                 hEndpoint,
    struct SOPC_RequestContext*   hContext,
    const OpcUa_RequestHeader*    pRequestHeader,
    const OpcUa_RegisteredServer* pServer,
    OpcUa_ResponseHeader*         pResponseHeader);

/*============================================================================
 * Begins processing of a RegisterServer service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginRegisterServer(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void *                      a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType);
#endif

#ifndef OPCUA_EXCLUDE_RegisterServer2
/*============================================================================
 * Synchronously calls the RegisterServer2 service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_RegisterServer2(
    SOPC_Endpoint                 hEndpoint,
    struct SOPC_RequestContext*   hContext,
    const OpcUa_RequestHeader*    pRequestHeader,
    const OpcUa_RegisteredServer* pServer,
    int32_t                       nNoOfDiscoveryConfiguration,
    const SOPC_ExtensionObject*   pDiscoveryConfiguration,
    OpcUa_ResponseHeader*         pResponseHeader,
    int32_t*                      pNoOfConfigurationResults,
    SOPC_StatusCode**             pConfigurationResults,
    int32_t*                      pNoOfDiagnosticInfos,
    SOPC_DiagnosticInfo**         pDiagnosticInfos);

/*============================================================================
 * Begins processing of a RegisterServer2 service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginRegisterServer2(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void *                      a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType);
#endif

#ifndef OPCUA_EXCLUDE_CreateSession
/*============================================================================
 * Synchronously calls the CreateSession service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_CreateSession(
    SOPC_Endpoint                       hEndpoint,
    struct SOPC_RequestContext*         hContext,
    const OpcUa_RequestHeader*          pRequestHeader,
    const OpcUa_ApplicationDescription* pClientDescription,
    const SOPC_String*                  pServerUri,
    const SOPC_String*                  pEndpointUrl,
    const SOPC_String*                  pSessionName,
    const SOPC_ByteString*              pClientNonce,
    const SOPC_ByteString*              pClientCertificate,
    double                              nRequestedSessionTimeout,
    uint32_t                            nMaxResponseMessageSize,
    OpcUa_ResponseHeader*               pResponseHeader,
    SOPC_NodeId*                        pSessionId,
    SOPC_NodeId*                        pAuthenticationToken,
    double*                             pRevisedSessionTimeout,
    SOPC_ByteString*                    pServerNonce,
    SOPC_ByteString*                    pServerCertificate,
    int32_t*                            pNoOfServerEndpoints,
    OpcUa_EndpointDescription**         pServerEndpoints,
    int32_t*                            pNoOfServerSoftwareCertificates,
    OpcUa_SignedSoftwareCertificate**   pServerSoftwareCertificates,
    OpcUa_SignatureData*                pServerSignature,
    uint32_t*                           pMaxRequestMessageSize);

/*============================================================================
 * Begins processing of a CreateSession service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginCreateSession(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void *                      a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType);
#endif

#ifndef OPCUA_EXCLUDE_ActivateSession
/*============================================================================
 * Synchronously calls the ActivateSession service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_ActivateSession(
    SOPC_Endpoint                          hEndpoint,
    struct SOPC_RequestContext*            hContext,
    const OpcUa_RequestHeader*             pRequestHeader,
    const OpcUa_SignatureData*             pClientSignature,
    int32_t                                nNoOfClientSoftwareCertificates,
    const OpcUa_SignedSoftwareCertificate* pClientSoftwareCertificates,
    int32_t                                nNoOfLocaleIds,
    const SOPC_String*                     pLocaleIds,
    const SOPC_ExtensionObject*            pUserIdentityToken,
    const OpcUa_SignatureData*             pUserTokenSignature,
    OpcUa_ResponseHeader*                  pResponseHeader,
    SOPC_ByteString*                       pServerNonce,
    int32_t*                               pNoOfResults,
    SOPC_StatusCode**                      pResults,
    int32_t*                               pNoOfDiagnosticInfos,
    SOPC_DiagnosticInfo**                  pDiagnosticInfos);

/*============================================================================
 * Begins processing of a ActivateSession service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginActivateSession(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void *                      a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType);
#endif

#ifndef OPCUA_EXCLUDE_CloseSession
/*============================================================================
 * Synchronously calls the CloseSession service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_CloseSession(
    SOPC_Endpoint               hEndpoint,
    struct SOPC_RequestContext* hContext,
    const OpcUa_RequestHeader*  pRequestHeader,
    SOPC_Boolean                bDeleteSubscriptions,
    OpcUa_ResponseHeader*       pResponseHeader);

/*============================================================================
 * Begins processing of a CloseSession service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginCloseSession(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void *                      a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType);
#endif

#ifndef OPCUA_EXCLUDE_Cancel
/*============================================================================
 * Synchronously calls the Cancel service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_Cancel(
    SOPC_Endpoint               hEndpoint,
    struct SOPC_RequestContext* hContext,
    const OpcUa_RequestHeader*  pRequestHeader,
    uint32_t                    nRequestHandle,
    OpcUa_ResponseHeader*       pResponseHeader,
    uint32_t*                   pCancelCount);

/*============================================================================
 * Begins processing of a Cancel service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginCancel(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void *                      a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType);
#endif

#ifndef OPCUA_EXCLUDE_AddNodes
/*============================================================================
 * Synchronously calls the AddNodes service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_AddNodes(
    SOPC_Endpoint               hEndpoint,
    struct SOPC_RequestContext* hContext,
    const OpcUa_RequestHeader*  pRequestHeader,
    int32_t                     nNoOfNodesToAdd,
    const OpcUa_AddNodesItem*   pNodesToAdd,
    OpcUa_ResponseHeader*       pResponseHeader,
    int32_t*                    pNoOfResults,
    OpcUa_AddNodesResult**      pResults,
    int32_t*                    pNoOfDiagnosticInfos,
    SOPC_DiagnosticInfo**       pDiagnosticInfos);

/*============================================================================
 * Begins processing of a AddNodes service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginAddNodes(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void *                      a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType);
#endif

#ifndef OPCUA_EXCLUDE_AddReferences
/*============================================================================
 * Synchronously calls the AddReferences service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_AddReferences(
    SOPC_Endpoint                  hEndpoint,
    struct SOPC_RequestContext*    hContext,
    const OpcUa_RequestHeader*     pRequestHeader,
    int32_t                        nNoOfReferencesToAdd,
    const OpcUa_AddReferencesItem* pReferencesToAdd,
    OpcUa_ResponseHeader*          pResponseHeader,
    int32_t*                       pNoOfResults,
    SOPC_StatusCode**              pResults,
    int32_t*                       pNoOfDiagnosticInfos,
    SOPC_DiagnosticInfo**          pDiagnosticInfos);

/*============================================================================
 * Begins processing of a AddReferences service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginAddReferences(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void *                      a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType);
#endif

#ifndef OPCUA_EXCLUDE_DeleteNodes
/*============================================================================
 * Synchronously calls the DeleteNodes service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_DeleteNodes(
    SOPC_Endpoint                hEndpoint,
    struct SOPC_RequestContext*  hContext,
    const OpcUa_RequestHeader*   pRequestHeader,
    int32_t                      nNoOfNodesToDelete,
    const OpcUa_DeleteNodesItem* pNodesToDelete,
    OpcUa_ResponseHeader*        pResponseHeader,
    int32_t*                     pNoOfResults,
    SOPC_StatusCode**            pResults,
    int32_t*                     pNoOfDiagnosticInfos,
    SOPC_DiagnosticInfo**        pDiagnosticInfos);

/*============================================================================
 * Begins processing of a DeleteNodes service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginDeleteNodes(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void *                      a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType);
#endif

#ifndef OPCUA_EXCLUDE_DeleteReferences
/*============================================================================
 * Synchronously calls the DeleteReferences service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_DeleteReferences(
    SOPC_Endpoint                     hEndpoint,
    struct SOPC_RequestContext*       hContext,
    const OpcUa_RequestHeader*        pRequestHeader,
    int32_t                           nNoOfReferencesToDelete,
    const OpcUa_DeleteReferencesItem* pReferencesToDelete,
    OpcUa_ResponseHeader*             pResponseHeader,
    int32_t*                          pNoOfResults,
    SOPC_StatusCode**                 pResults,
    int32_t*                          pNoOfDiagnosticInfos,
    SOPC_DiagnosticInfo**             pDiagnosticInfos);

/*============================================================================
 * Begins processing of a DeleteReferences service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginDeleteReferences(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void *                      a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType);
#endif

#ifndef OPCUA_EXCLUDE_Browse
/*============================================================================
 * Synchronously calls the Browse service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_Browse(
    SOPC_Endpoint                  hEndpoint,
    struct SOPC_RequestContext*    hContext,
    const OpcUa_RequestHeader*     pRequestHeader,
    const OpcUa_ViewDescription*   pView,
    uint32_t                       nRequestedMaxReferencesPerNode,
    int32_t                        nNoOfNodesToBrowse,
    const OpcUa_BrowseDescription* pNodesToBrowse,
    OpcUa_ResponseHeader*          pResponseHeader,
    int32_t*                       pNoOfResults,
    OpcUa_BrowseResult**           pResults,
    int32_t*                       pNoOfDiagnosticInfos,
    SOPC_DiagnosticInfo**          pDiagnosticInfos);

/*============================================================================
 * Begins processing of a Browse service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginBrowse(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void *                      a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType);
#endif

#ifndef OPCUA_EXCLUDE_BrowseNext
/*============================================================================
 * Synchronously calls the BrowseNext service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_BrowseNext(
    SOPC_Endpoint               hEndpoint,
    struct SOPC_RequestContext* hContext,
    const OpcUa_RequestHeader*  pRequestHeader,
    SOPC_Boolean                bReleaseContinuationPoints,
    int32_t                     nNoOfContinuationPoints,
    const SOPC_ByteString*      pContinuationPoints,
    OpcUa_ResponseHeader*       pResponseHeader,
    int32_t*                    pNoOfResults,
    OpcUa_BrowseResult**        pResults,
    int32_t*                    pNoOfDiagnosticInfos,
    SOPC_DiagnosticInfo**       pDiagnosticInfos);

/*============================================================================
 * Begins processing of a BrowseNext service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginBrowseNext(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void *                      a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType);
#endif

#ifndef OPCUA_EXCLUDE_TranslateBrowsePathsToNodeIds
/*============================================================================
 * Synchronously calls the TranslateBrowsePathsToNodeIds service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_TranslateBrowsePathsToNodeIds(
    SOPC_Endpoint               hEndpoint,
    struct SOPC_RequestContext* hContext,
    const OpcUa_RequestHeader*  pRequestHeader,
    int32_t                     nNoOfBrowsePaths,
    const OpcUa_BrowsePath*     pBrowsePaths,
    OpcUa_ResponseHeader*       pResponseHeader,
    int32_t*                    pNoOfResults,
    OpcUa_BrowsePathResult**    pResults,
    int32_t*                    pNoOfDiagnosticInfos,
    SOPC_DiagnosticInfo**       pDiagnosticInfos);

/*============================================================================
 * Begins processing of a TranslateBrowsePathsToNodeIds service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginTranslateBrowsePathsToNodeIds(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void *                      a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType);
#endif

#ifndef OPCUA_EXCLUDE_RegisterNodes
/*============================================================================
 * Synchronously calls the RegisterNodes service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_RegisterNodes(
    SOPC_Endpoint               hEndpoint,
    struct SOPC_RequestContext* hContext,
    const OpcUa_RequestHeader*  pRequestHeader,
    int32_t                     nNoOfNodesToRegister,
    const SOPC_NodeId*          pNodesToRegister,
    OpcUa_ResponseHeader*       pResponseHeader,
    int32_t*                    pNoOfRegisteredNodeIds,
    SOPC_NodeId**               pRegisteredNodeIds);

/*============================================================================
 * Begins processing of a RegisterNodes service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginRegisterNodes(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void *                      a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType);
#endif

#ifndef OPCUA_EXCLUDE_UnregisterNodes
/*============================================================================
 * Synchronously calls the UnregisterNodes service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_UnregisterNodes(
    SOPC_Endpoint               hEndpoint,
    struct SOPC_RequestContext* hContext,
    const OpcUa_RequestHeader*  pRequestHeader,
    int32_t                     nNoOfNodesToUnregister,
    const SOPC_NodeId*          pNodesToUnregister,
    OpcUa_ResponseHeader*       pResponseHeader);

/*============================================================================
 * Begins processing of a UnregisterNodes service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginUnregisterNodes(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void *                      a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType);
#endif

#ifndef OPCUA_EXCLUDE_QueryFirst
/*============================================================================
 * Synchronously calls the QueryFirst service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_QueryFirst(
    SOPC_Endpoint                    hEndpoint,
    struct SOPC_RequestContext*      hContext,
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
    SOPC_ByteString*                 pContinuationPoint,
    int32_t*                         pNoOfParsingResults,
    OpcUa_ParsingResult**            pParsingResults,
    int32_t*                         pNoOfDiagnosticInfos,
    SOPC_DiagnosticInfo**            pDiagnosticInfos,
    OpcUa_ContentFilterResult*       pFilterResult);

/*============================================================================
 * Begins processing of a QueryFirst service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginQueryFirst(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void *                      a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType);
#endif

#ifndef OPCUA_EXCLUDE_QueryNext
/*============================================================================
 * Synchronously calls the QueryNext service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_QueryNext(
    SOPC_Endpoint               hEndpoint,
    struct SOPC_RequestContext* hContext,
    const OpcUa_RequestHeader*  pRequestHeader,
    SOPC_Boolean                bReleaseContinuationPoint,
    const SOPC_ByteString*      pContinuationPoint,
    OpcUa_ResponseHeader*       pResponseHeader,
    int32_t*                    pNoOfQueryDataSets,
    OpcUa_QueryDataSet**        pQueryDataSets,
    SOPC_ByteString*            pRevisedContinuationPoint);

/*============================================================================
 * Begins processing of a QueryNext service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginQueryNext(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void *                      a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType);
#endif

#ifndef OPCUA_EXCLUDE_Read
/*============================================================================
 * Synchronously calls the Read service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_Read(
    SOPC_Endpoint               hEndpoint,
    struct SOPC_RequestContext* hContext,
    const OpcUa_RequestHeader*  pRequestHeader,
    double                      nMaxAge,
    OpcUa_TimestampsToReturn    eTimestampsToReturn,
    int32_t                     nNoOfNodesToRead,
    const OpcUa_ReadValueId*    pNodesToRead,
    OpcUa_ResponseHeader*       pResponseHeader,
    int32_t*                    pNoOfResults,
    SOPC_DataValue**            pResults,
    int32_t*                    pNoOfDiagnosticInfos,
    SOPC_DiagnosticInfo**       pDiagnosticInfos);

/*============================================================================
 * Begins processing of a Read service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginRead(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void *                      a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType);
#endif

#ifndef OPCUA_EXCLUDE_HistoryRead
/*============================================================================
 * Synchronously calls the HistoryRead service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_HistoryRead(
    SOPC_Endpoint                   hEndpoint,
    struct SOPC_RequestContext*     hContext,
    const OpcUa_RequestHeader*      pRequestHeader,
    const SOPC_ExtensionObject*     pHistoryReadDetails,
    OpcUa_TimestampsToReturn        eTimestampsToReturn,
    SOPC_Boolean                    bReleaseContinuationPoints,
    int32_t                         nNoOfNodesToRead,
    const OpcUa_HistoryReadValueId* pNodesToRead,
    OpcUa_ResponseHeader*           pResponseHeader,
    int32_t*                        pNoOfResults,
    OpcUa_HistoryReadResult**       pResults,
    int32_t*                        pNoOfDiagnosticInfos,
    SOPC_DiagnosticInfo**           pDiagnosticInfos);

/*============================================================================
 * Begins processing of a HistoryRead service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginHistoryRead(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void *                      a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType);
#endif

#ifndef OPCUA_EXCLUDE_Write
/*============================================================================
 * Synchronously calls the Write service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_Write(
    SOPC_Endpoint               hEndpoint,
    struct SOPC_RequestContext* hContext,
    const OpcUa_RequestHeader*  pRequestHeader,
    int32_t                     nNoOfNodesToWrite,
    const OpcUa_WriteValue*     pNodesToWrite,
    OpcUa_ResponseHeader*       pResponseHeader,
    int32_t*                    pNoOfResults,
    SOPC_StatusCode**           pResults,
    int32_t*                    pNoOfDiagnosticInfos,
    SOPC_DiagnosticInfo**       pDiagnosticInfos);

/*============================================================================
 * Begins processing of a Write service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginWrite(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void *                      a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType);
#endif

#ifndef OPCUA_EXCLUDE_HistoryUpdate
/*============================================================================
 * Synchronously calls the HistoryUpdate service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_HistoryUpdate(
    SOPC_Endpoint               hEndpoint,
    struct SOPC_RequestContext* hContext,
    const OpcUa_RequestHeader*  pRequestHeader,
    int32_t                     nNoOfHistoryUpdateDetails,
    const SOPC_ExtensionObject* pHistoryUpdateDetails,
    OpcUa_ResponseHeader*       pResponseHeader,
    int32_t*                    pNoOfResults,
    OpcUa_HistoryUpdateResult** pResults,
    int32_t*                    pNoOfDiagnosticInfos,
    SOPC_DiagnosticInfo**       pDiagnosticInfos);

/*============================================================================
 * Begins processing of a HistoryUpdate service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginHistoryUpdate(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void *                      a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType);
#endif

#ifndef OPCUA_EXCLUDE_Call
/*============================================================================
 * Synchronously calls the Call service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_Call(
    SOPC_Endpoint                  hEndpoint,
    struct SOPC_RequestContext*    hContext,
    const OpcUa_RequestHeader*     pRequestHeader,
    int32_t                        nNoOfMethodsToCall,
    const OpcUa_CallMethodRequest* pMethodsToCall,
    OpcUa_ResponseHeader*          pResponseHeader,
    int32_t*                       pNoOfResults,
    OpcUa_CallMethodResult**       pResults,
    int32_t*                       pNoOfDiagnosticInfos,
    SOPC_DiagnosticInfo**          pDiagnosticInfos);

/*============================================================================
 * Begins processing of a Call service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginCall(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void *                      a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType);
#endif

#ifndef OPCUA_EXCLUDE_CreateMonitoredItems
/*============================================================================
 * Synchronously calls the CreateMonitoredItems service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_CreateMonitoredItems(
    SOPC_Endpoint                           hEndpoint,
    struct SOPC_RequestContext*             hContext,
    const OpcUa_RequestHeader*              pRequestHeader,
    uint32_t                                nSubscriptionId,
    OpcUa_TimestampsToReturn                eTimestampsToReturn,
    int32_t                                 nNoOfItemsToCreate,
    const OpcUa_MonitoredItemCreateRequest* pItemsToCreate,
    OpcUa_ResponseHeader*                   pResponseHeader,
    int32_t*                                pNoOfResults,
    OpcUa_MonitoredItemCreateResult**       pResults,
    int32_t*                                pNoOfDiagnosticInfos,
    SOPC_DiagnosticInfo**                   pDiagnosticInfos);

/*============================================================================
 * Begins processing of a CreateMonitoredItems service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginCreateMonitoredItems(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void *                      a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType);
#endif

#ifndef OPCUA_EXCLUDE_ModifyMonitoredItems
/*============================================================================
 * Synchronously calls the ModifyMonitoredItems service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_ModifyMonitoredItems(
    SOPC_Endpoint                           hEndpoint,
    struct SOPC_RequestContext*             hContext,
    const OpcUa_RequestHeader*              pRequestHeader,
    uint32_t                                nSubscriptionId,
    OpcUa_TimestampsToReturn                eTimestampsToReturn,
    int32_t                                 nNoOfItemsToModify,
    const OpcUa_MonitoredItemModifyRequest* pItemsToModify,
    OpcUa_ResponseHeader*                   pResponseHeader,
    int32_t*                                pNoOfResults,
    OpcUa_MonitoredItemModifyResult**       pResults,
    int32_t*                                pNoOfDiagnosticInfos,
    SOPC_DiagnosticInfo**                   pDiagnosticInfos);

/*============================================================================
 * Begins processing of a ModifyMonitoredItems service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginModifyMonitoredItems(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void *                      a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType);
#endif

#ifndef OPCUA_EXCLUDE_SetMonitoringMode
/*============================================================================
 * Synchronously calls the SetMonitoringMode service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_SetMonitoringMode(
    SOPC_Endpoint               hEndpoint,
    struct SOPC_RequestContext* hContext,
    const OpcUa_RequestHeader*  pRequestHeader,
    uint32_t                    nSubscriptionId,
    OpcUa_MonitoringMode        eMonitoringMode,
    int32_t                     nNoOfMonitoredItemIds,
    const uint32_t*             pMonitoredItemIds,
    OpcUa_ResponseHeader*       pResponseHeader,
    int32_t*                    pNoOfResults,
    SOPC_StatusCode**           pResults,
    int32_t*                    pNoOfDiagnosticInfos,
    SOPC_DiagnosticInfo**       pDiagnosticInfos);

/*============================================================================
 * Begins processing of a SetMonitoringMode service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginSetMonitoringMode(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void *                      a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType);
#endif

#ifndef OPCUA_EXCLUDE_SetTriggering
/*============================================================================
 * Synchronously calls the SetTriggering service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_SetTriggering(
    SOPC_Endpoint               hEndpoint,
    struct SOPC_RequestContext* hContext,
    const OpcUa_RequestHeader*  pRequestHeader,
    uint32_t                    nSubscriptionId,
    uint32_t                    nTriggeringItemId,
    int32_t                     nNoOfLinksToAdd,
    const uint32_t*             pLinksToAdd,
    int32_t                     nNoOfLinksToRemove,
    const uint32_t*             pLinksToRemove,
    OpcUa_ResponseHeader*       pResponseHeader,
    int32_t*                    pNoOfAddResults,
    SOPC_StatusCode**           pAddResults,
    int32_t*                    pNoOfAddDiagnosticInfos,
    SOPC_DiagnosticInfo**       pAddDiagnosticInfos,
    int32_t*                    pNoOfRemoveResults,
    SOPC_StatusCode**           pRemoveResults,
    int32_t*                    pNoOfRemoveDiagnosticInfos,
    SOPC_DiagnosticInfo**       pRemoveDiagnosticInfos);

/*============================================================================
 * Begins processing of a SetTriggering service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginSetTriggering(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void *                      a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType);
#endif

#ifndef OPCUA_EXCLUDE_DeleteMonitoredItems
/*============================================================================
 * Synchronously calls the DeleteMonitoredItems service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_DeleteMonitoredItems(
    SOPC_Endpoint               hEndpoint,
    struct SOPC_RequestContext* hContext,
    const OpcUa_RequestHeader*  pRequestHeader,
    uint32_t                    nSubscriptionId,
    int32_t                     nNoOfMonitoredItemIds,
    const uint32_t*             pMonitoredItemIds,
    OpcUa_ResponseHeader*       pResponseHeader,
    int32_t*                    pNoOfResults,
    SOPC_StatusCode**           pResults,
    int32_t*                    pNoOfDiagnosticInfos,
    SOPC_DiagnosticInfo**       pDiagnosticInfos);

/*============================================================================
 * Begins processing of a DeleteMonitoredItems service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginDeleteMonitoredItems(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void *                      a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType);
#endif

#ifndef OPCUA_EXCLUDE_CreateSubscription
/*============================================================================
 * Synchronously calls the CreateSubscription service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_CreateSubscription(
    SOPC_Endpoint               hEndpoint,
    struct SOPC_RequestContext* hContext,
    const OpcUa_RequestHeader*  pRequestHeader,
    double                      nRequestedPublishingInterval,
    uint32_t                    nRequestedLifetimeCount,
    uint32_t                    nRequestedMaxKeepAliveCount,
    uint32_t                    nMaxNotificationsPerPublish,
    SOPC_Boolean                bPublishingEnabled,
    SOPC_Byte                   nPriority,
    OpcUa_ResponseHeader*       pResponseHeader,
    uint32_t*                   pSubscriptionId,
    double*                     pRevisedPublishingInterval,
    uint32_t*                   pRevisedLifetimeCount,
    uint32_t*                   pRevisedMaxKeepAliveCount);

/*============================================================================
 * Begins processing of a CreateSubscription service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginCreateSubscription(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void *                      a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType);
#endif

#ifndef OPCUA_EXCLUDE_ModifySubscription
/*============================================================================
 * Synchronously calls the ModifySubscription service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_ModifySubscription(
    SOPC_Endpoint               hEndpoint,
    struct SOPC_RequestContext* hContext,
    const OpcUa_RequestHeader*  pRequestHeader,
    uint32_t                    nSubscriptionId,
    double                      nRequestedPublishingInterval,
    uint32_t                    nRequestedLifetimeCount,
    uint32_t                    nRequestedMaxKeepAliveCount,
    uint32_t                    nMaxNotificationsPerPublish,
    SOPC_Byte                   nPriority,
    OpcUa_ResponseHeader*       pResponseHeader,
    double*                     pRevisedPublishingInterval,
    uint32_t*                   pRevisedLifetimeCount,
    uint32_t*                   pRevisedMaxKeepAliveCount);

/*============================================================================
 * Begins processing of a ModifySubscription service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginModifySubscription(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void *                      a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType);
#endif

#ifndef OPCUA_EXCLUDE_SetPublishingMode
/*============================================================================
 * Synchronously calls the SetPublishingMode service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_SetPublishingMode(
    SOPC_Endpoint               hEndpoint,
    struct SOPC_RequestContext* hContext,
    const OpcUa_RequestHeader*  pRequestHeader,
    SOPC_Boolean                bPublishingEnabled,
    int32_t                     nNoOfSubscriptionIds,
    const uint32_t*             pSubscriptionIds,
    OpcUa_ResponseHeader*       pResponseHeader,
    int32_t*                    pNoOfResults,
    SOPC_StatusCode**           pResults,
    int32_t*                    pNoOfDiagnosticInfos,
    SOPC_DiagnosticInfo**       pDiagnosticInfos);

/*============================================================================
 * Begins processing of a SetPublishingMode service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginSetPublishingMode(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void *                      a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType);
#endif

#ifndef OPCUA_EXCLUDE_Publish
/*============================================================================
 * Synchronously calls the Publish service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_Publish(
    SOPC_Endpoint                            hEndpoint,
    struct SOPC_RequestContext*              hContext,
    const OpcUa_RequestHeader*               pRequestHeader,
    int32_t                                  nNoOfSubscriptionAcknowledgements,
    const OpcUa_SubscriptionAcknowledgement* pSubscriptionAcknowledgements,
    OpcUa_ResponseHeader*                    pResponseHeader,
    uint32_t*                                pSubscriptionId,
    int32_t*                                 pNoOfAvailableSequenceNumbers,
    uint32_t**                               pAvailableSequenceNumbers,
    SOPC_Boolean*                            pMoreNotifications,
    OpcUa_NotificationMessage*               pNotificationMessage,
    int32_t*                                 pNoOfResults,
    SOPC_StatusCode**                        pResults,
    int32_t*                                 pNoOfDiagnosticInfos,
    SOPC_DiagnosticInfo**                    pDiagnosticInfos);

/*============================================================================
 * Begins processing of a Publish service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginPublish(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void *                      a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType);
#endif

#ifndef OPCUA_EXCLUDE_Republish
/*============================================================================
 * Synchronously calls the Republish service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_Republish(
    SOPC_Endpoint               hEndpoint,
    struct SOPC_RequestContext* hContext,
    const OpcUa_RequestHeader*  pRequestHeader,
    uint32_t                    nSubscriptionId,
    uint32_t                    nRetransmitSequenceNumber,
    OpcUa_ResponseHeader*       pResponseHeader,
    OpcUa_NotificationMessage*  pNotificationMessage);

/*============================================================================
 * Begins processing of a Republish service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginRepublish(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void *                      a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType);
#endif

#ifndef OPCUA_EXCLUDE_TransferSubscriptions
/*============================================================================
 * Synchronously calls the TransferSubscriptions service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_TransferSubscriptions(
    SOPC_Endpoint               hEndpoint,
    struct SOPC_RequestContext* hContext,
    const OpcUa_RequestHeader*  pRequestHeader,
    int32_t                     nNoOfSubscriptionIds,
    const uint32_t*             pSubscriptionIds,
    SOPC_Boolean                bSendInitialValues,
    OpcUa_ResponseHeader*       pResponseHeader,
    int32_t*                    pNoOfResults,
    OpcUa_TransferResult**      pResults,
    int32_t*                    pNoOfDiagnosticInfos,
    SOPC_DiagnosticInfo**       pDiagnosticInfos);

/*============================================================================
 * Begins processing of a TransferSubscriptions service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginTransferSubscriptions(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void *                      a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType);
#endif

#ifndef OPCUA_EXCLUDE_DeleteSubscriptions
/*============================================================================
 * Synchronously calls the DeleteSubscriptions service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_DeleteSubscriptions(
    SOPC_Endpoint               hEndpoint,
    struct SOPC_RequestContext* hContext,
    const OpcUa_RequestHeader*  pRequestHeader,
    int32_t                     nNoOfSubscriptionIds,
    const uint32_t*             pSubscriptionIds,
    OpcUa_ResponseHeader*       pResponseHeader,
    int32_t*                    pNoOfResults,
    SOPC_StatusCode**           pResults,
    int32_t*                    pNoOfDiagnosticInfos,
    SOPC_DiagnosticInfo**       pDiagnosticInfos);

/*============================================================================
 * Begins processing of a DeleteSubscriptions service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginDeleteSubscriptions(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void *                      a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType);
#endif

SOPC_StatusCode SOPC_ServerApi_CreateFault(OpcUa_RequestHeader* requestHeader,
                                           SOPC_StatusCode      status,
                                           SOPC_DiagnosticInfo* serviceDiagnostics,
                                           int32_t*             noOfStringTable,
                                           SOPC_String**        stringTable,
                                           OpcUa_ServiceFault*  faultObj);

extern SOPC_ServiceType* SOPC_SupportedServiceTypes[];

END_EXTERN_C

#endif /* OPCUA_HAVE_SERVERAPI */
#endif /* _SOPC_ServerApi_H_ */
/* This is the last line of an autogenerated file. */
