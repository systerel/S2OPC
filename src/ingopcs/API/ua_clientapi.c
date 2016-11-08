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

#include <stdlib.h>
#include <stddef.h>

#include <ua_stack_csts.h>

#ifdef OPCUA_HAVE_CLIENTAPI

/* types */
#include <ua_builtintypes.h>

#include <opcua_identifiers.h>
#include <opcua_statuscodes.h>
#include <ua_clientapi.h>

#ifndef OPCUA_EXCLUDE_FindServers
/*============================================================================
 * Synchronously calls the FindServers service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_FindServers(
    UA_Channel                     a_hChannel,
    const OpcUa_RequestHeader*     a_pRequestHeader,
    const UA_String*               a_pEndpointUrl,
    int32_t                        a_nNoOfLocaleIds,
    const UA_String*               a_pLocaleIds,
    int32_t                        a_nNoOfServerUris,
    const UA_String*               a_pServerUris,
    OpcUa_ResponseHeader*          a_pResponseHeader,
    int32_t*                       a_pNoOfServers,
    OpcUa_ApplicationDescription** a_pServers)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_FindServersRequest cRequest;
    OpcUa_FindServersResponse* pResponse = NULL;
    UA_EncodeableType* pResponseType = NULL;
    
    OpcUa_FindServersRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  a_pEndpointUrl != NULL
       &&  (a_pLocaleIds != NULL || a_nNoOfLocaleIds <= 0)
       &&  (a_pServerUris != NULL || a_nNoOfServerUris <= 0)
       &&  a_pResponseHeader != NULL
       &&  a_pNoOfServers != NULL
       &&  a_pServers != NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader  = *a_pRequestHeader;
        cRequest.EndpointUrl    = *a_pEndpointUrl;
        cRequest.NoOfLocaleIds  = a_nNoOfLocaleIds;
        cRequest.LocaleIds      = (UA_String*)a_pLocaleIds;
        cRequest.NoOfServerUris = a_nNoOfServerUris;
        cRequest.ServerUris     = (UA_String*)a_pServerUris;

        /* invoke service */
        status = UA_Channel_InvokeService(
            a_hChannel,
            "FindServers",
            (void*)&cRequest,
            &OpcUa_FindServersRequest_EncodeableType,
            &OpcUa_FindServersResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->TypeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((OpcUa_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (OpcUa_FindServersResponse_EncodeableType.TypeId != pResponseType->TypeId)
        {
            pResponseType->Clear(pResponse);
            status = OpcUa_BadUnknownResponse;
        }

        /* copy parameters from response object into return parameters. */
        else
        {
            *a_pResponseHeader = pResponse->ResponseHeader;
            *a_pNoOfServers    = pResponse->NoOfServers;
            *a_pServers        = pResponse->Servers;
        }
    }else{
        //TODO: check if status invalid response could have been already deallocated in InvokeService
        /* memory contained in the reponse objects is owned by the caller */
        free(pResponse);
    }

    return status;
}

/*============================================================================
 * Asynchronously calls the FindServers service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginFindServers(
    UA_Channel                     a_hChannel,
    const OpcUa_RequestHeader*     a_pRequestHeader,
    const UA_String*               a_pEndpointUrl,
    int32_t                        a_nNoOfLocaleIds,
    const UA_String*               a_pLocaleIds,
    int32_t                        a_nNoOfServerUris,
    const UA_String*               a_pServerUris,
    UA_Channel_PfnRequestComplete* a_pCallback,
    void*                          a_pCallbackData)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_FindServersRequest cRequest;
    OpcUa_FindServersRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  a_pEndpointUrl != NULL
       &&  (a_pLocaleIds != NULL || a_nNoOfLocaleIds <= 0)
       &&  (a_pServerUris != NULL || a_nNoOfServerUris <= 0))
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader  = *a_pRequestHeader;
        cRequest.EndpointUrl    = *a_pEndpointUrl;
        cRequest.NoOfLocaleIds  = a_nNoOfLocaleIds;
        cRequest.LocaleIds      = (UA_String*)a_pLocaleIds;
        cRequest.NoOfServerUris = a_nNoOfServerUris;
        cRequest.ServerUris     = (UA_String*)a_pServerUris;

        /* begin invoke service */
        status = UA_Channel_BeginInvokeService(
            a_hChannel,
            "FindServers",
            (void*)&cRequest,
            &OpcUa_FindServersRequest_EncodeableType,
            &OpcUa_FindServersResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*)a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_FindServersOnNetwork
/*============================================================================
 * Synchronously calls the FindServersOnNetwork service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_FindServersOnNetwork(
    UA_Channel                 a_hChannel,
    const OpcUa_RequestHeader* a_pRequestHeader,
    uint32_t                   a_nStartingRecordId,
    uint32_t                   a_nMaxRecordsToReturn,
    int32_t                    a_nNoOfServerCapabilityFilter,
    const UA_String*           a_pServerCapabilityFilter,
    OpcUa_ResponseHeader*      a_pResponseHeader,
    UA_DateTime*               a_pLastCounterResetTime,
    int32_t*                   a_pNoOfServers,
    OpcUa_ServerOnNetwork**    a_pServers)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_FindServersOnNetworkRequest cRequest;
    OpcUa_FindServersOnNetworkResponse* pResponse = NULL;
    UA_EncodeableType* pResponseType = NULL;
    
    OpcUa_FindServersOnNetworkRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  (a_pServerCapabilityFilter != NULL || a_nNoOfServerCapabilityFilter <= 0)
       &&  a_pResponseHeader != NULL
       &&  a_pLastCounterResetTime != NULL
       &&  a_pNoOfServers != NULL
       &&  a_pServers != NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader              = *a_pRequestHeader;
        cRequest.StartingRecordId           = a_nStartingRecordId;
        cRequest.MaxRecordsToReturn         = a_nMaxRecordsToReturn;
        cRequest.NoOfServerCapabilityFilter = a_nNoOfServerCapabilityFilter;
        cRequest.ServerCapabilityFilter     = (UA_String*)a_pServerCapabilityFilter;

        /* invoke service */
        status = UA_Channel_InvokeService(
            a_hChannel,
            "FindServersOnNetwork",
            (void*)&cRequest,
            &OpcUa_FindServersOnNetworkRequest_EncodeableType,
            &OpcUa_FindServersOnNetworkResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->TypeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((OpcUa_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (OpcUa_FindServersOnNetworkResponse_EncodeableType.TypeId != pResponseType->TypeId)
        {
            pResponseType->Clear(pResponse);
            status = OpcUa_BadUnknownResponse;
        }

        /* copy parameters from response object into return parameters. */
        else
        {
            *a_pResponseHeader       = pResponse->ResponseHeader;
            *a_pLastCounterResetTime = pResponse->LastCounterResetTime;
            *a_pNoOfServers          = pResponse->NoOfServers;
            *a_pServers              = pResponse->Servers;
        }
    }else{
        //TODO: check if status invalid response could have been already deallocated in InvokeService
        /* memory contained in the reponse objects is owned by the caller */
        free(pResponse);
    }

    return status;
}

/*============================================================================
 * Asynchronously calls the FindServersOnNetwork service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginFindServersOnNetwork(
    UA_Channel                     a_hChannel,
    const OpcUa_RequestHeader*     a_pRequestHeader,
    uint32_t                       a_nStartingRecordId,
    uint32_t                       a_nMaxRecordsToReturn,
    int32_t                        a_nNoOfServerCapabilityFilter,
    const UA_String*               a_pServerCapabilityFilter,
    UA_Channel_PfnRequestComplete* a_pCallback,
    void*                          a_pCallbackData)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_FindServersOnNetworkRequest cRequest;
    OpcUa_FindServersOnNetworkRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  (a_pServerCapabilityFilter != NULL || a_nNoOfServerCapabilityFilter <= 0))
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader              = *a_pRequestHeader;
        cRequest.StartingRecordId           = a_nStartingRecordId;
        cRequest.MaxRecordsToReturn         = a_nMaxRecordsToReturn;
        cRequest.NoOfServerCapabilityFilter = a_nNoOfServerCapabilityFilter;
        cRequest.ServerCapabilityFilter     = (UA_String*)a_pServerCapabilityFilter;

        /* begin invoke service */
        status = UA_Channel_BeginInvokeService(
            a_hChannel,
            "FindServersOnNetwork",
            (void*)&cRequest,
            &OpcUa_FindServersOnNetworkRequest_EncodeableType,
            &OpcUa_FindServersOnNetworkResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*)a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_GetEndpoints
/*============================================================================
 * Synchronously calls the GetEndpoints service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_GetEndpoints(
    UA_Channel                  a_hChannel,
    const OpcUa_RequestHeader*  a_pRequestHeader,
    const UA_String*            a_pEndpointUrl,
    int32_t                     a_nNoOfLocaleIds,
    const UA_String*            a_pLocaleIds,
    int32_t                     a_nNoOfProfileUris,
    const UA_String*            a_pProfileUris,
    OpcUa_ResponseHeader*       a_pResponseHeader,
    int32_t*                    a_pNoOfEndpoints,
    OpcUa_EndpointDescription** a_pEndpoints)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_GetEndpointsRequest cRequest;
    OpcUa_GetEndpointsResponse* pResponse = NULL;
    UA_EncodeableType* pResponseType = NULL;
    
    OpcUa_GetEndpointsRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  a_pEndpointUrl != NULL
       &&  (a_pLocaleIds != NULL || a_nNoOfLocaleIds <= 0)
       &&  (a_pProfileUris != NULL || a_nNoOfProfileUris <= 0)
       &&  a_pResponseHeader != NULL
       &&  a_pNoOfEndpoints != NULL
       &&  a_pEndpoints != NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader   = *a_pRequestHeader;
        cRequest.EndpointUrl     = *a_pEndpointUrl;
        cRequest.NoOfLocaleIds   = a_nNoOfLocaleIds;
        cRequest.LocaleIds       = (UA_String*)a_pLocaleIds;
        cRequest.NoOfProfileUris = a_nNoOfProfileUris;
        cRequest.ProfileUris     = (UA_String*)a_pProfileUris;

        /* invoke service */
        status = UA_Channel_InvokeService(
            a_hChannel,
            "GetEndpoints",
            (void*)&cRequest,
            &OpcUa_GetEndpointsRequest_EncodeableType,
            &OpcUa_GetEndpointsResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->TypeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((OpcUa_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (OpcUa_GetEndpointsResponse_EncodeableType.TypeId != pResponseType->TypeId)
        {
            pResponseType->Clear(pResponse);
            status = OpcUa_BadUnknownResponse;
        }

        /* copy parameters from response object into return parameters. */
        else
        {
            *a_pResponseHeader = pResponse->ResponseHeader;
            *a_pNoOfEndpoints  = pResponse->NoOfEndpoints;
            *a_pEndpoints      = pResponse->Endpoints;
        }
    }else{
        //TODO: check if status invalid response could have been already deallocated in InvokeService
        /* memory contained in the reponse objects is owned by the caller */
        free(pResponse);
    }

    return status;
}

/*============================================================================
 * Asynchronously calls the GetEndpoints service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginGetEndpoints(
    UA_Channel                     a_hChannel,
    const OpcUa_RequestHeader*     a_pRequestHeader,
    const UA_String*               a_pEndpointUrl,
    int32_t                        a_nNoOfLocaleIds,
    const UA_String*               a_pLocaleIds,
    int32_t                        a_nNoOfProfileUris,
    const UA_String*               a_pProfileUris,
    UA_Channel_PfnRequestComplete* a_pCallback,
    void*                          a_pCallbackData)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_GetEndpointsRequest cRequest;
    OpcUa_GetEndpointsRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  a_pEndpointUrl != NULL
       &&  (a_pLocaleIds != NULL || a_nNoOfLocaleIds <= 0)
       &&  (a_pProfileUris != NULL || a_nNoOfProfileUris <= 0))
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader   = *a_pRequestHeader;
        cRequest.EndpointUrl     = *a_pEndpointUrl;
        cRequest.NoOfLocaleIds   = a_nNoOfLocaleIds;
        cRequest.LocaleIds       = (UA_String*)a_pLocaleIds;
        cRequest.NoOfProfileUris = a_nNoOfProfileUris;
        cRequest.ProfileUris     = (UA_String*)a_pProfileUris;

        /* begin invoke service */
        status = UA_Channel_BeginInvokeService(
            a_hChannel,
            "GetEndpoints",
            (void*)&cRequest,
            &OpcUa_GetEndpointsRequest_EncodeableType,
            &OpcUa_GetEndpointsResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*)a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_RegisterServer
/*============================================================================
 * Synchronously calls the RegisterServer service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_RegisterServer(
    UA_Channel                    a_hChannel,
    const OpcUa_RequestHeader*    a_pRequestHeader,
    const OpcUa_RegisteredServer* a_pServer,
    OpcUa_ResponseHeader*         a_pResponseHeader)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_RegisterServerRequest cRequest;
    OpcUa_RegisterServerResponse* pResponse = NULL;
    UA_EncodeableType* pResponseType = NULL;
    
    OpcUa_RegisterServerRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  a_pServer != NULL
       &&  a_pResponseHeader != NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader = *a_pRequestHeader;
        cRequest.Server        = *a_pServer;

        /* invoke service */
        status = UA_Channel_InvokeService(
            a_hChannel,
            "RegisterServer",
            (void*)&cRequest,
            &OpcUa_RegisterServerRequest_EncodeableType,
            &OpcUa_RegisterServerResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->TypeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((OpcUa_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (OpcUa_RegisterServerResponse_EncodeableType.TypeId != pResponseType->TypeId)
        {
            pResponseType->Clear(pResponse);
            status = OpcUa_BadUnknownResponse;
        }

        /* copy parameters from response object into return parameters. */
        else
        {
            *a_pResponseHeader = pResponse->ResponseHeader;
        }
    }else{
        //TODO: check if status invalid response could have been already deallocated in InvokeService
        /* memory contained in the reponse objects is owned by the caller */
        free(pResponse);
    }

    return status;
}

/*============================================================================
 * Asynchronously calls the RegisterServer service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginRegisterServer(
    UA_Channel                     a_hChannel,
    const OpcUa_RequestHeader*     a_pRequestHeader,
    const OpcUa_RegisteredServer*  a_pServer,
    UA_Channel_PfnRequestComplete* a_pCallback,
    void*                          a_pCallbackData)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_RegisterServerRequest cRequest;
    OpcUa_RegisterServerRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  a_pServer != NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader = *a_pRequestHeader;
        cRequest.Server        = *a_pServer;

        /* begin invoke service */
        status = UA_Channel_BeginInvokeService(
            a_hChannel,
            "RegisterServer",
            (void*)&cRequest,
            &OpcUa_RegisterServerRequest_EncodeableType,
            &OpcUa_RegisterServerResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*)a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_RegisterServer2
/*============================================================================
 * Synchronously calls the RegisterServer2 service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_RegisterServer2(
    UA_Channel                    a_hChannel,
    const OpcUa_RequestHeader*    a_pRequestHeader,
    const OpcUa_RegisteredServer* a_pServer,
    int32_t                       a_nNoOfDiscoveryConfiguration,
    const UA_ExtensionObject*     a_pDiscoveryConfiguration,
    OpcUa_ResponseHeader*         a_pResponseHeader,
    int32_t*                      a_pNoOfConfigurationResults,
    SOPC_StatusCode**                  a_pConfigurationResults,
    int32_t*                      a_pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**           a_pDiagnosticInfos)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_RegisterServer2Request cRequest;
    OpcUa_RegisterServer2Response* pResponse = NULL;
    UA_EncodeableType* pResponseType = NULL;
    
    OpcUa_RegisterServer2Request_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  a_pServer != NULL
       &&  (a_pDiscoveryConfiguration != NULL || a_nNoOfDiscoveryConfiguration <= 0)
       &&  a_pResponseHeader != NULL
       &&  a_pNoOfConfigurationResults != NULL
       &&  a_pConfigurationResults != NULL
       &&  a_pNoOfDiagnosticInfos != NULL
       &&  a_pDiagnosticInfos != NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader              = *a_pRequestHeader;
        cRequest.Server                     = *a_pServer;
        cRequest.NoOfDiscoveryConfiguration = a_nNoOfDiscoveryConfiguration;
        cRequest.DiscoveryConfiguration     = (UA_ExtensionObject*)a_pDiscoveryConfiguration;

        /* invoke service */
        status = UA_Channel_InvokeService(
            a_hChannel,
            "RegisterServer2",
            (void*)&cRequest,
            &OpcUa_RegisterServer2Request_EncodeableType,
            &OpcUa_RegisterServer2Response_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->TypeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((OpcUa_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (OpcUa_RegisterServer2Response_EncodeableType.TypeId != pResponseType->TypeId)
        {
            pResponseType->Clear(pResponse);
            status = OpcUa_BadUnknownResponse;
        }

        /* copy parameters from response object into return parameters. */
        else
        {
            *a_pResponseHeader           = pResponse->ResponseHeader;
            *a_pNoOfConfigurationResults = pResponse->NoOfConfigurationResults;
            *a_pConfigurationResults     = pResponse->ConfigurationResults;
            *a_pNoOfDiagnosticInfos      = pResponse->NoOfDiagnosticInfos;
            *a_pDiagnosticInfos          = pResponse->DiagnosticInfos;
        }
    }else{
        //TODO: check if status invalid response could have been already deallocated in InvokeService
        /* memory contained in the reponse objects is owned by the caller */
        free(pResponse);
    }

    return status;
}

/*============================================================================
 * Asynchronously calls the RegisterServer2 service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginRegisterServer2(
    UA_Channel                     a_hChannel,
    const OpcUa_RequestHeader*     a_pRequestHeader,
    const OpcUa_RegisteredServer*  a_pServer,
    int32_t                        a_nNoOfDiscoveryConfiguration,
    const UA_ExtensionObject*      a_pDiscoveryConfiguration,
    UA_Channel_PfnRequestComplete* a_pCallback,
    void*                          a_pCallbackData)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_RegisterServer2Request cRequest;
    OpcUa_RegisterServer2Request_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  a_pServer != NULL
       &&  (a_pDiscoveryConfiguration != NULL || a_nNoOfDiscoveryConfiguration <= 0))
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader              = *a_pRequestHeader;
        cRequest.Server                     = *a_pServer;
        cRequest.NoOfDiscoveryConfiguration = a_nNoOfDiscoveryConfiguration;
        cRequest.DiscoveryConfiguration     = (UA_ExtensionObject*)a_pDiscoveryConfiguration;

        /* begin invoke service */
        status = UA_Channel_BeginInvokeService(
            a_hChannel,
            "RegisterServer2",
            (void*)&cRequest,
            &OpcUa_RegisterServer2Request_EncodeableType,
            &OpcUa_RegisterServer2Response_EncodeableType,
            (UA_Channel_PfnRequestComplete*)a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_CreateSession
/*============================================================================
 * Synchronously calls the CreateSession service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_CreateSession(
    UA_Channel                          a_hChannel,
    const OpcUa_RequestHeader*          a_pRequestHeader,
    const OpcUa_ApplicationDescription* a_pClientDescription,
    const UA_String*                    a_pServerUri,
    const UA_String*                    a_pEndpointUrl,
    const UA_String*                    a_pSessionName,
    const UA_ByteString*                a_pClientNonce,
    const UA_ByteString*                a_pClientCertificate,
    double                              a_nRequestedSessionTimeout,
    uint32_t                            a_nMaxResponseMessageSize,
    OpcUa_ResponseHeader*               a_pResponseHeader,
    UA_NodeId*                          a_pSessionId,
    UA_NodeId*                          a_pAuthenticationToken,
    double*                             a_pRevisedSessionTimeout,
    UA_ByteString*                      a_pServerNonce,
    UA_ByteString*                      a_pServerCertificate,
    int32_t*                            a_pNoOfServerEndpoints,
    OpcUa_EndpointDescription**         a_pServerEndpoints,
    int32_t*                            a_pNoOfServerSoftwareCertificates,
    OpcUa_SignedSoftwareCertificate**   a_pServerSoftwareCertificates,
    OpcUa_SignatureData*                a_pServerSignature,
    uint32_t*                           a_pMaxRequestMessageSize)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_CreateSessionRequest cRequest;
    OpcUa_CreateSessionResponse* pResponse = NULL;
    UA_EncodeableType* pResponseType = NULL;
    
    OpcUa_CreateSessionRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  a_pClientDescription != NULL
       &&  a_pServerUri != NULL
       &&  a_pEndpointUrl != NULL
       &&  a_pSessionName != NULL
       &&  a_pClientNonce != NULL
       &&  a_pClientCertificate != NULL
       &&  a_pResponseHeader != NULL
       &&  a_pSessionId != NULL
       &&  a_pAuthenticationToken != NULL
       &&  a_pRevisedSessionTimeout != NULL
       &&  a_pServerNonce != NULL
       &&  a_pServerCertificate != NULL
       &&  a_pNoOfServerEndpoints != NULL
       &&  a_pServerEndpoints != NULL
       &&  a_pNoOfServerSoftwareCertificates != NULL
       &&  a_pServerSoftwareCertificates != NULL
       &&  a_pServerSignature != NULL
       &&  a_pMaxRequestMessageSize != NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader           = *a_pRequestHeader;
        cRequest.ClientDescription       = *a_pClientDescription;
        cRequest.ServerUri               = *a_pServerUri;
        cRequest.EndpointUrl             = *a_pEndpointUrl;
        cRequest.SessionName             = *a_pSessionName;
        cRequest.ClientNonce             = *a_pClientNonce;
        cRequest.ClientCertificate       = *a_pClientCertificate;
        cRequest.RequestedSessionTimeout = a_nRequestedSessionTimeout;
        cRequest.MaxResponseMessageSize  = a_nMaxResponseMessageSize;

        /* invoke service */
        status = UA_Channel_InvokeService(
            a_hChannel,
            "CreateSession",
            (void*)&cRequest,
            &OpcUa_CreateSessionRequest_EncodeableType,
            &OpcUa_CreateSessionResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->TypeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((OpcUa_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (OpcUa_CreateSessionResponse_EncodeableType.TypeId != pResponseType->TypeId)
        {
            pResponseType->Clear(pResponse);
            status = OpcUa_BadUnknownResponse;
        }

        /* copy parameters from response object into return parameters. */
        else
        {
            *a_pResponseHeader                 = pResponse->ResponseHeader;
            *a_pSessionId                      = pResponse->SessionId;
            *a_pAuthenticationToken            = pResponse->AuthenticationToken;
            *a_pRevisedSessionTimeout          = pResponse->RevisedSessionTimeout;
            *a_pServerNonce                    = pResponse->ServerNonce;
            *a_pServerCertificate              = pResponse->ServerCertificate;
            *a_pNoOfServerEndpoints            = pResponse->NoOfServerEndpoints;
            *a_pServerEndpoints                = pResponse->ServerEndpoints;
            *a_pNoOfServerSoftwareCertificates = pResponse->NoOfServerSoftwareCertificates;
            *a_pServerSoftwareCertificates     = pResponse->ServerSoftwareCertificates;
            *a_pServerSignature                = pResponse->ServerSignature;
            *a_pMaxRequestMessageSize          = pResponse->MaxRequestMessageSize;
        }
    }else{
        //TODO: check if status invalid response could have been already deallocated in InvokeService
        /* memory contained in the reponse objects is owned by the caller */
        free(pResponse);
    }

    return status;
}

/*============================================================================
 * Asynchronously calls the CreateSession service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginCreateSession(
    UA_Channel                          a_hChannel,
    const OpcUa_RequestHeader*          a_pRequestHeader,
    const OpcUa_ApplicationDescription* a_pClientDescription,
    const UA_String*                    a_pServerUri,
    const UA_String*                    a_pEndpointUrl,
    const UA_String*                    a_pSessionName,
    const UA_ByteString*                a_pClientNonce,
    const UA_ByteString*                a_pClientCertificate,
    double                              a_nRequestedSessionTimeout,
    uint32_t                            a_nMaxResponseMessageSize,
    UA_Channel_PfnRequestComplete*      a_pCallback,
    void*                               a_pCallbackData)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_CreateSessionRequest cRequest;
    OpcUa_CreateSessionRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  a_pClientDescription != NULL
       &&  a_pServerUri != NULL
       &&  a_pEndpointUrl != NULL
       &&  a_pSessionName != NULL
       &&  a_pClientNonce != NULL
       &&  a_pClientCertificate != NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader           = *a_pRequestHeader;
        cRequest.ClientDescription       = *a_pClientDescription;
        cRequest.ServerUri               = *a_pServerUri;
        cRequest.EndpointUrl             = *a_pEndpointUrl;
        cRequest.SessionName             = *a_pSessionName;
        cRequest.ClientNonce             = *a_pClientNonce;
        cRequest.ClientCertificate       = *a_pClientCertificate;
        cRequest.RequestedSessionTimeout = a_nRequestedSessionTimeout;
        cRequest.MaxResponseMessageSize  = a_nMaxResponseMessageSize;

        /* begin invoke service */
        status = UA_Channel_BeginInvokeService(
            a_hChannel,
            "CreateSession",
            (void*)&cRequest,
            &OpcUa_CreateSessionRequest_EncodeableType,
            &OpcUa_CreateSessionResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*)a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_ActivateSession
/*============================================================================
 * Synchronously calls the ActivateSession service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_ActivateSession(
    UA_Channel                             a_hChannel,
    const OpcUa_RequestHeader*             a_pRequestHeader,
    const OpcUa_SignatureData*             a_pClientSignature,
    int32_t                                a_nNoOfClientSoftwareCertificates,
    const OpcUa_SignedSoftwareCertificate* a_pClientSoftwareCertificates,
    int32_t                                a_nNoOfLocaleIds,
    const UA_String*                       a_pLocaleIds,
    const UA_ExtensionObject*              a_pUserIdentityToken,
    const OpcUa_SignatureData*             a_pUserTokenSignature,
    OpcUa_ResponseHeader*                  a_pResponseHeader,
    UA_ByteString*                         a_pServerNonce,
    int32_t*                               a_pNoOfResults,
    SOPC_StatusCode**                           a_pResults,
    int32_t*                               a_pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**                    a_pDiagnosticInfos)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_ActivateSessionRequest cRequest;
    OpcUa_ActivateSessionResponse* pResponse = NULL;
    UA_EncodeableType* pResponseType = NULL;
    
    OpcUa_ActivateSessionRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  a_pClientSignature != NULL
       &&  (a_pClientSoftwareCertificates != NULL || a_nNoOfClientSoftwareCertificates <= 0)
       &&  (a_pLocaleIds != NULL || a_nNoOfLocaleIds <= 0)
       &&  a_pUserIdentityToken != NULL
       &&  a_pUserTokenSignature != NULL
       &&  a_pResponseHeader != NULL
       &&  a_pServerNonce != NULL
       &&  a_pNoOfResults != NULL
       &&  a_pResults != NULL
       &&  a_pNoOfDiagnosticInfos != NULL
       &&  a_pDiagnosticInfos != NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader                  = *a_pRequestHeader;
        cRequest.ClientSignature                = *a_pClientSignature;
        cRequest.NoOfClientSoftwareCertificates = a_nNoOfClientSoftwareCertificates;
        cRequest.ClientSoftwareCertificates     = (OpcUa_SignedSoftwareCertificate*)a_pClientSoftwareCertificates;
        cRequest.NoOfLocaleIds                  = a_nNoOfLocaleIds;
        cRequest.LocaleIds                      = (UA_String*)a_pLocaleIds;
        cRequest.UserIdentityToken              = *a_pUserIdentityToken;
        cRequest.UserTokenSignature             = *a_pUserTokenSignature;

        /* invoke service */
        status = UA_Channel_InvokeService(
            a_hChannel,
            "ActivateSession",
            (void*)&cRequest,
            &OpcUa_ActivateSessionRequest_EncodeableType,
            &OpcUa_ActivateSessionResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->TypeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((OpcUa_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (OpcUa_ActivateSessionResponse_EncodeableType.TypeId != pResponseType->TypeId)
        {
            pResponseType->Clear(pResponse);
            status = OpcUa_BadUnknownResponse;
        }

        /* copy parameters from response object into return parameters. */
        else
        {
            *a_pResponseHeader      = pResponse->ResponseHeader;
            *a_pServerNonce         = pResponse->ServerNonce;
            *a_pNoOfResults         = pResponse->NoOfResults;
            *a_pResults             = pResponse->Results;
            *a_pNoOfDiagnosticInfos = pResponse->NoOfDiagnosticInfos;
            *a_pDiagnosticInfos     = pResponse->DiagnosticInfos;
        }
    }else{
        //TODO: check if status invalid response could have been already deallocated in InvokeService
        /* memory contained in the reponse objects is owned by the caller */
        free(pResponse);
    }

    return status;
}

/*============================================================================
 * Asynchronously calls the ActivateSession service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginActivateSession(
    UA_Channel                             a_hChannel,
    const OpcUa_RequestHeader*             a_pRequestHeader,
    const OpcUa_SignatureData*             a_pClientSignature,
    int32_t                                a_nNoOfClientSoftwareCertificates,
    const OpcUa_SignedSoftwareCertificate* a_pClientSoftwareCertificates,
    int32_t                                a_nNoOfLocaleIds,
    const UA_String*                       a_pLocaleIds,
    const UA_ExtensionObject*              a_pUserIdentityToken,
    const OpcUa_SignatureData*             a_pUserTokenSignature,
    UA_Channel_PfnRequestComplete*         a_pCallback,
    void*                                  a_pCallbackData)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_ActivateSessionRequest cRequest;
    OpcUa_ActivateSessionRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  a_pClientSignature != NULL
       &&  (a_pClientSoftwareCertificates != NULL || a_nNoOfClientSoftwareCertificates <= 0)
       &&  (a_pLocaleIds != NULL || a_nNoOfLocaleIds <= 0)
       &&  a_pUserIdentityToken != NULL
       &&  a_pUserTokenSignature != NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader                  = *a_pRequestHeader;
        cRequest.ClientSignature                = *a_pClientSignature;
        cRequest.NoOfClientSoftwareCertificates = a_nNoOfClientSoftwareCertificates;
        cRequest.ClientSoftwareCertificates     = (OpcUa_SignedSoftwareCertificate*)a_pClientSoftwareCertificates;
        cRequest.NoOfLocaleIds                  = a_nNoOfLocaleIds;
        cRequest.LocaleIds                      = (UA_String*)a_pLocaleIds;
        cRequest.UserIdentityToken              = *a_pUserIdentityToken;
        cRequest.UserTokenSignature             = *a_pUserTokenSignature;

        /* begin invoke service */
        status = UA_Channel_BeginInvokeService(
            a_hChannel,
            "ActivateSession",
            (void*)&cRequest,
            &OpcUa_ActivateSessionRequest_EncodeableType,
            &OpcUa_ActivateSessionResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*)a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_CloseSession
/*============================================================================
 * Synchronously calls the CloseSession service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_CloseSession(
    UA_Channel                 a_hChannel,
    const OpcUa_RequestHeader* a_pRequestHeader,
    UA_Boolean                 a_bDeleteSubscriptions,
    OpcUa_ResponseHeader*      a_pResponseHeader)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_CloseSessionRequest cRequest;
    OpcUa_CloseSessionResponse* pResponse = NULL;
    UA_EncodeableType* pResponseType = NULL;
    
    OpcUa_CloseSessionRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  a_pResponseHeader != NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader       = *a_pRequestHeader;
        cRequest.DeleteSubscriptions = a_bDeleteSubscriptions;

        /* invoke service */
        status = UA_Channel_InvokeService(
            a_hChannel,
            "CloseSession",
            (void*)&cRequest,
            &OpcUa_CloseSessionRequest_EncodeableType,
            &OpcUa_CloseSessionResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->TypeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((OpcUa_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (OpcUa_CloseSessionResponse_EncodeableType.TypeId != pResponseType->TypeId)
        {
            pResponseType->Clear(pResponse);
            status = OpcUa_BadUnknownResponse;
        }

        /* copy parameters from response object into return parameters. */
        else
        {
            *a_pResponseHeader = pResponse->ResponseHeader;
        }
    }else{
        //TODO: check if status invalid response could have been already deallocated in InvokeService
        /* memory contained in the reponse objects is owned by the caller */
        free(pResponse);
    }

    return status;
}

/*============================================================================
 * Asynchronously calls the CloseSession service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginCloseSession(
    UA_Channel                     a_hChannel,
    const OpcUa_RequestHeader*     a_pRequestHeader,
    UA_Boolean                     a_bDeleteSubscriptions,
    UA_Channel_PfnRequestComplete* a_pCallback,
    void*                          a_pCallbackData)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_CloseSessionRequest cRequest;
    OpcUa_CloseSessionRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader       = *a_pRequestHeader;
        cRequest.DeleteSubscriptions = a_bDeleteSubscriptions;

        /* begin invoke service */
        status = UA_Channel_BeginInvokeService(
            a_hChannel,
            "CloseSession",
            (void*)&cRequest,
            &OpcUa_CloseSessionRequest_EncodeableType,
            &OpcUa_CloseSessionResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*)a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_Cancel
/*============================================================================
 * Synchronously calls the Cancel service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_Cancel(
    UA_Channel                 a_hChannel,
    const OpcUa_RequestHeader* a_pRequestHeader,
    uint32_t                   a_nRequestHandle,
    OpcUa_ResponseHeader*      a_pResponseHeader,
    uint32_t*                  a_pCancelCount)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_CancelRequest cRequest;
    OpcUa_CancelResponse* pResponse = NULL;
    UA_EncodeableType* pResponseType = NULL;
    
    OpcUa_CancelRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  a_pResponseHeader != NULL
       &&  a_pCancelCount != NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader = *a_pRequestHeader;
        cRequest.RequestHandle = a_nRequestHandle;

        /* invoke service */
        status = UA_Channel_InvokeService(
            a_hChannel,
            "Cancel",
            (void*)&cRequest,
            &OpcUa_CancelRequest_EncodeableType,
            &OpcUa_CancelResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->TypeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((OpcUa_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (OpcUa_CancelResponse_EncodeableType.TypeId != pResponseType->TypeId)
        {
            pResponseType->Clear(pResponse);
            status = OpcUa_BadUnknownResponse;
        }

        /* copy parameters from response object into return parameters. */
        else
        {
            *a_pResponseHeader = pResponse->ResponseHeader;
            *a_pCancelCount    = pResponse->CancelCount;
        }
    }else{
        //TODO: check if status invalid response could have been already deallocated in InvokeService
        /* memory contained in the reponse objects is owned by the caller */
        free(pResponse);
    }

    return status;
}

/*============================================================================
 * Asynchronously calls the Cancel service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginCancel(
    UA_Channel                     a_hChannel,
    const OpcUa_RequestHeader*     a_pRequestHeader,
    uint32_t                       a_nRequestHandle,
    UA_Channel_PfnRequestComplete* a_pCallback,
    void*                          a_pCallbackData)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_CancelRequest cRequest;
    OpcUa_CancelRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader = *a_pRequestHeader;
        cRequest.RequestHandle = a_nRequestHandle;

        /* begin invoke service */
        status = UA_Channel_BeginInvokeService(
            a_hChannel,
            "Cancel",
            (void*)&cRequest,
            &OpcUa_CancelRequest_EncodeableType,
            &OpcUa_CancelResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*)a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_AddNodes
/*============================================================================
 * Synchronously calls the AddNodes service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_AddNodes(
    UA_Channel                 a_hChannel,
    const OpcUa_RequestHeader* a_pRequestHeader,
    int32_t                    a_nNoOfNodesToAdd,
    const OpcUa_AddNodesItem*  a_pNodesToAdd,
    OpcUa_ResponseHeader*      a_pResponseHeader,
    int32_t*                   a_pNoOfResults,
    OpcUa_AddNodesResult**     a_pResults,
    int32_t*                   a_pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**        a_pDiagnosticInfos)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_AddNodesRequest cRequest;
    OpcUa_AddNodesResponse* pResponse = NULL;
    UA_EncodeableType* pResponseType = NULL;
    
    OpcUa_AddNodesRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  (a_pNodesToAdd != NULL || a_nNoOfNodesToAdd <= 0)
       &&  a_pResponseHeader != NULL
       &&  a_pNoOfResults != NULL
       &&  a_pResults != NULL
       &&  a_pNoOfDiagnosticInfos != NULL
       &&  a_pDiagnosticInfos != NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader  = *a_pRequestHeader;
        cRequest.NoOfNodesToAdd = a_nNoOfNodesToAdd;
        cRequest.NodesToAdd     = (OpcUa_AddNodesItem*)a_pNodesToAdd;

        /* invoke service */
        status = UA_Channel_InvokeService(
            a_hChannel,
            "AddNodes",
            (void*)&cRequest,
            &OpcUa_AddNodesRequest_EncodeableType,
            &OpcUa_AddNodesResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->TypeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((OpcUa_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (OpcUa_AddNodesResponse_EncodeableType.TypeId != pResponseType->TypeId)
        {
            pResponseType->Clear(pResponse);
            status = OpcUa_BadUnknownResponse;
        }

        /* copy parameters from response object into return parameters. */
        else
        {
            *a_pResponseHeader      = pResponse->ResponseHeader;
            *a_pNoOfResults         = pResponse->NoOfResults;
            *a_pResults             = pResponse->Results;
            *a_pNoOfDiagnosticInfos = pResponse->NoOfDiagnosticInfos;
            *a_pDiagnosticInfos     = pResponse->DiagnosticInfos;
        }
    }else{
        //TODO: check if status invalid response could have been already deallocated in InvokeService
        /* memory contained in the reponse objects is owned by the caller */
        free(pResponse);
    }

    return status;
}

/*============================================================================
 * Asynchronously calls the AddNodes service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginAddNodes(
    UA_Channel                     a_hChannel,
    const OpcUa_RequestHeader*     a_pRequestHeader,
    int32_t                        a_nNoOfNodesToAdd,
    const OpcUa_AddNodesItem*      a_pNodesToAdd,
    UA_Channel_PfnRequestComplete* a_pCallback,
    void*                          a_pCallbackData)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_AddNodesRequest cRequest;
    OpcUa_AddNodesRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  (a_pNodesToAdd != NULL || a_nNoOfNodesToAdd <= 0))
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader  = *a_pRequestHeader;
        cRequest.NoOfNodesToAdd = a_nNoOfNodesToAdd;
        cRequest.NodesToAdd     = (OpcUa_AddNodesItem*)a_pNodesToAdd;

        /* begin invoke service */
        status = UA_Channel_BeginInvokeService(
            a_hChannel,
            "AddNodes",
            (void*)&cRequest,
            &OpcUa_AddNodesRequest_EncodeableType,
            &OpcUa_AddNodesResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*)a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_AddReferences
/*============================================================================
 * Synchronously calls the AddReferences service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_AddReferences(
    UA_Channel                     a_hChannel,
    const OpcUa_RequestHeader*     a_pRequestHeader,
    int32_t                        a_nNoOfReferencesToAdd,
    const OpcUa_AddReferencesItem* a_pReferencesToAdd,
    OpcUa_ResponseHeader*          a_pResponseHeader,
    int32_t*                       a_pNoOfResults,
    SOPC_StatusCode**                   a_pResults,
    int32_t*                       a_pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**            a_pDiagnosticInfos)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_AddReferencesRequest cRequest;
    OpcUa_AddReferencesResponse* pResponse = NULL;
    UA_EncodeableType* pResponseType = NULL;
    
    OpcUa_AddReferencesRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  (a_pReferencesToAdd != NULL || a_nNoOfReferencesToAdd <= 0)
       &&  a_pResponseHeader != NULL
       &&  a_pNoOfResults != NULL
       &&  a_pResults != NULL
       &&  a_pNoOfDiagnosticInfos != NULL
       &&  a_pDiagnosticInfos != NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader       = *a_pRequestHeader;
        cRequest.NoOfReferencesToAdd = a_nNoOfReferencesToAdd;
        cRequest.ReferencesToAdd     = (OpcUa_AddReferencesItem*)a_pReferencesToAdd;

        /* invoke service */
        status = UA_Channel_InvokeService(
            a_hChannel,
            "AddReferences",
            (void*)&cRequest,
            &OpcUa_AddReferencesRequest_EncodeableType,
            &OpcUa_AddReferencesResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->TypeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((OpcUa_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (OpcUa_AddReferencesResponse_EncodeableType.TypeId != pResponseType->TypeId)
        {
            pResponseType->Clear(pResponse);
            status = OpcUa_BadUnknownResponse;
        }

        /* copy parameters from response object into return parameters. */
        else
        {
            *a_pResponseHeader      = pResponse->ResponseHeader;
            *a_pNoOfResults         = pResponse->NoOfResults;
            *a_pResults             = pResponse->Results;
            *a_pNoOfDiagnosticInfos = pResponse->NoOfDiagnosticInfos;
            *a_pDiagnosticInfos     = pResponse->DiagnosticInfos;
        }
    }else{
        //TODO: check if status invalid response could have been already deallocated in InvokeService
        /* memory contained in the reponse objects is owned by the caller */
        free(pResponse);
    }

    return status;
}

/*============================================================================
 * Asynchronously calls the AddReferences service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginAddReferences(
    UA_Channel                     a_hChannel,
    const OpcUa_RequestHeader*     a_pRequestHeader,
    int32_t                        a_nNoOfReferencesToAdd,
    const OpcUa_AddReferencesItem* a_pReferencesToAdd,
    UA_Channel_PfnRequestComplete* a_pCallback,
    void*                          a_pCallbackData)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_AddReferencesRequest cRequest;
    OpcUa_AddReferencesRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  (a_pReferencesToAdd != NULL || a_nNoOfReferencesToAdd <= 0))
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader       = *a_pRequestHeader;
        cRequest.NoOfReferencesToAdd = a_nNoOfReferencesToAdd;
        cRequest.ReferencesToAdd     = (OpcUa_AddReferencesItem*)a_pReferencesToAdd;

        /* begin invoke service */
        status = UA_Channel_BeginInvokeService(
            a_hChannel,
            "AddReferences",
            (void*)&cRequest,
            &OpcUa_AddReferencesRequest_EncodeableType,
            &OpcUa_AddReferencesResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*)a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_DeleteNodes
/*============================================================================
 * Synchronously calls the DeleteNodes service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_DeleteNodes(
    UA_Channel                   a_hChannel,
    const OpcUa_RequestHeader*   a_pRequestHeader,
    int32_t                      a_nNoOfNodesToDelete,
    const OpcUa_DeleteNodesItem* a_pNodesToDelete,
    OpcUa_ResponseHeader*        a_pResponseHeader,
    int32_t*                     a_pNoOfResults,
    SOPC_StatusCode**                 a_pResults,
    int32_t*                     a_pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**          a_pDiagnosticInfos)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_DeleteNodesRequest cRequest;
    OpcUa_DeleteNodesResponse* pResponse = NULL;
    UA_EncodeableType* pResponseType = NULL;
    
    OpcUa_DeleteNodesRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  (a_pNodesToDelete != NULL || a_nNoOfNodesToDelete <= 0)
       &&  a_pResponseHeader != NULL
       &&  a_pNoOfResults != NULL
       &&  a_pResults != NULL
       &&  a_pNoOfDiagnosticInfos != NULL
       &&  a_pDiagnosticInfos != NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader     = *a_pRequestHeader;
        cRequest.NoOfNodesToDelete = a_nNoOfNodesToDelete;
        cRequest.NodesToDelete     = (OpcUa_DeleteNodesItem*)a_pNodesToDelete;

        /* invoke service */
        status = UA_Channel_InvokeService(
            a_hChannel,
            "DeleteNodes",
            (void*)&cRequest,
            &OpcUa_DeleteNodesRequest_EncodeableType,
            &OpcUa_DeleteNodesResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->TypeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((OpcUa_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (OpcUa_DeleteNodesResponse_EncodeableType.TypeId != pResponseType->TypeId)
        {
            pResponseType->Clear(pResponse);
            status = OpcUa_BadUnknownResponse;
        }

        /* copy parameters from response object into return parameters. */
        else
        {
            *a_pResponseHeader      = pResponse->ResponseHeader;
            *a_pNoOfResults         = pResponse->NoOfResults;
            *a_pResults             = pResponse->Results;
            *a_pNoOfDiagnosticInfos = pResponse->NoOfDiagnosticInfos;
            *a_pDiagnosticInfos     = pResponse->DiagnosticInfos;
        }
    }else{
        //TODO: check if status invalid response could have been already deallocated in InvokeService
        /* memory contained in the reponse objects is owned by the caller */
        free(pResponse);
    }

    return status;
}

/*============================================================================
 * Asynchronously calls the DeleteNodes service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginDeleteNodes(
    UA_Channel                     a_hChannel,
    const OpcUa_RequestHeader*     a_pRequestHeader,
    int32_t                        a_nNoOfNodesToDelete,
    const OpcUa_DeleteNodesItem*   a_pNodesToDelete,
    UA_Channel_PfnRequestComplete* a_pCallback,
    void*                          a_pCallbackData)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_DeleteNodesRequest cRequest;
    OpcUa_DeleteNodesRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  (a_pNodesToDelete != NULL || a_nNoOfNodesToDelete <= 0))
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader     = *a_pRequestHeader;
        cRequest.NoOfNodesToDelete = a_nNoOfNodesToDelete;
        cRequest.NodesToDelete     = (OpcUa_DeleteNodesItem*)a_pNodesToDelete;

        /* begin invoke service */
        status = UA_Channel_BeginInvokeService(
            a_hChannel,
            "DeleteNodes",
            (void*)&cRequest,
            &OpcUa_DeleteNodesRequest_EncodeableType,
            &OpcUa_DeleteNodesResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*)a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_DeleteReferences
/*============================================================================
 * Synchronously calls the DeleteReferences service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_DeleteReferences(
    UA_Channel                        a_hChannel,
    const OpcUa_RequestHeader*        a_pRequestHeader,
    int32_t                           a_nNoOfReferencesToDelete,
    const OpcUa_DeleteReferencesItem* a_pReferencesToDelete,
    OpcUa_ResponseHeader*             a_pResponseHeader,
    int32_t*                          a_pNoOfResults,
    SOPC_StatusCode**                      a_pResults,
    int32_t*                          a_pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**               a_pDiagnosticInfos)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_DeleteReferencesRequest cRequest;
    OpcUa_DeleteReferencesResponse* pResponse = NULL;
    UA_EncodeableType* pResponseType = NULL;
    
    OpcUa_DeleteReferencesRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  (a_pReferencesToDelete != NULL || a_nNoOfReferencesToDelete <= 0)
       &&  a_pResponseHeader != NULL
       &&  a_pNoOfResults != NULL
       &&  a_pResults != NULL
       &&  a_pNoOfDiagnosticInfos != NULL
       &&  a_pDiagnosticInfos != NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader          = *a_pRequestHeader;
        cRequest.NoOfReferencesToDelete = a_nNoOfReferencesToDelete;
        cRequest.ReferencesToDelete     = (OpcUa_DeleteReferencesItem*)a_pReferencesToDelete;

        /* invoke service */
        status = UA_Channel_InvokeService(
            a_hChannel,
            "DeleteReferences",
            (void*)&cRequest,
            &OpcUa_DeleteReferencesRequest_EncodeableType,
            &OpcUa_DeleteReferencesResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->TypeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((OpcUa_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (OpcUa_DeleteReferencesResponse_EncodeableType.TypeId != pResponseType->TypeId)
        {
            pResponseType->Clear(pResponse);
            status = OpcUa_BadUnknownResponse;
        }

        /* copy parameters from response object into return parameters. */
        else
        {
            *a_pResponseHeader      = pResponse->ResponseHeader;
            *a_pNoOfResults         = pResponse->NoOfResults;
            *a_pResults             = pResponse->Results;
            *a_pNoOfDiagnosticInfos = pResponse->NoOfDiagnosticInfos;
            *a_pDiagnosticInfos     = pResponse->DiagnosticInfos;
        }
    }else{
        //TODO: check if status invalid response could have been already deallocated in InvokeService
        /* memory contained in the reponse objects is owned by the caller */
        free(pResponse);
    }

    return status;
}

/*============================================================================
 * Asynchronously calls the DeleteReferences service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginDeleteReferences(
    UA_Channel                        a_hChannel,
    const OpcUa_RequestHeader*        a_pRequestHeader,
    int32_t                           a_nNoOfReferencesToDelete,
    const OpcUa_DeleteReferencesItem* a_pReferencesToDelete,
    UA_Channel_PfnRequestComplete*    a_pCallback,
    void*                             a_pCallbackData)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_DeleteReferencesRequest cRequest;
    OpcUa_DeleteReferencesRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  (a_pReferencesToDelete != NULL || a_nNoOfReferencesToDelete <= 0))
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader          = *a_pRequestHeader;
        cRequest.NoOfReferencesToDelete = a_nNoOfReferencesToDelete;
        cRequest.ReferencesToDelete     = (OpcUa_DeleteReferencesItem*)a_pReferencesToDelete;

        /* begin invoke service */
        status = UA_Channel_BeginInvokeService(
            a_hChannel,
            "DeleteReferences",
            (void*)&cRequest,
            &OpcUa_DeleteReferencesRequest_EncodeableType,
            &OpcUa_DeleteReferencesResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*)a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_Browse
/*============================================================================
 * Synchronously calls the Browse service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_Browse(
    UA_Channel                     a_hChannel,
    const OpcUa_RequestHeader*     a_pRequestHeader,
    const OpcUa_ViewDescription*   a_pView,
    uint32_t                       a_nRequestedMaxReferencesPerNode,
    int32_t                        a_nNoOfNodesToBrowse,
    const OpcUa_BrowseDescription* a_pNodesToBrowse,
    OpcUa_ResponseHeader*          a_pResponseHeader,
    int32_t*                       a_pNoOfResults,
    OpcUa_BrowseResult**           a_pResults,
    int32_t*                       a_pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**            a_pDiagnosticInfos)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_BrowseRequest cRequest;
    OpcUa_BrowseResponse* pResponse = NULL;
    UA_EncodeableType* pResponseType = NULL;
    
    OpcUa_BrowseRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  a_pView != NULL
       &&  (a_pNodesToBrowse != NULL || a_nNoOfNodesToBrowse <= 0)
       &&  a_pResponseHeader != NULL
       &&  a_pNoOfResults != NULL
       &&  a_pResults != NULL
       &&  a_pNoOfDiagnosticInfos != NULL
       &&  a_pDiagnosticInfos != NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader                 = *a_pRequestHeader;
        cRequest.View                          = *a_pView;
        cRequest.RequestedMaxReferencesPerNode = a_nRequestedMaxReferencesPerNode;
        cRequest.NoOfNodesToBrowse             = a_nNoOfNodesToBrowse;
        cRequest.NodesToBrowse                 = (OpcUa_BrowseDescription*)a_pNodesToBrowse;

        /* invoke service */
        status = UA_Channel_InvokeService(
            a_hChannel,
            "Browse",
            (void*)&cRequest,
            &OpcUa_BrowseRequest_EncodeableType,
            &OpcUa_BrowseResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->TypeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((OpcUa_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (OpcUa_BrowseResponse_EncodeableType.TypeId != pResponseType->TypeId)
        {
            pResponseType->Clear(pResponse);
            status = OpcUa_BadUnknownResponse;
        }

        /* copy parameters from response object into return parameters. */
        else
        {
            *a_pResponseHeader      = pResponse->ResponseHeader;
            *a_pNoOfResults         = pResponse->NoOfResults;
            *a_pResults             = pResponse->Results;
            *a_pNoOfDiagnosticInfos = pResponse->NoOfDiagnosticInfos;
            *a_pDiagnosticInfos     = pResponse->DiagnosticInfos;
        }
    }else{
        //TODO: check if status invalid response could have been already deallocated in InvokeService
        /* memory contained in the reponse objects is owned by the caller */
        free(pResponse);
    }

    return status;
}

/*============================================================================
 * Asynchronously calls the Browse service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginBrowse(
    UA_Channel                     a_hChannel,
    const OpcUa_RequestHeader*     a_pRequestHeader,
    const OpcUa_ViewDescription*   a_pView,
    uint32_t                       a_nRequestedMaxReferencesPerNode,
    int32_t                        a_nNoOfNodesToBrowse,
    const OpcUa_BrowseDescription* a_pNodesToBrowse,
    UA_Channel_PfnRequestComplete* a_pCallback,
    void*                          a_pCallbackData)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_BrowseRequest cRequest;
    OpcUa_BrowseRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  a_pView != NULL
       &&  (a_pNodesToBrowse != NULL || a_nNoOfNodesToBrowse <= 0))
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader                 = *a_pRequestHeader;
        cRequest.View                          = *a_pView;
        cRequest.RequestedMaxReferencesPerNode = a_nRequestedMaxReferencesPerNode;
        cRequest.NoOfNodesToBrowse             = a_nNoOfNodesToBrowse;
        cRequest.NodesToBrowse                 = (OpcUa_BrowseDescription*)a_pNodesToBrowse;

        /* begin invoke service */
        status = UA_Channel_BeginInvokeService(
            a_hChannel,
            "Browse",
            (void*)&cRequest,
            &OpcUa_BrowseRequest_EncodeableType,
            &OpcUa_BrowseResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*)a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_BrowseNext
/*============================================================================
 * Synchronously calls the BrowseNext service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BrowseNext(
    UA_Channel                 a_hChannel,
    const OpcUa_RequestHeader* a_pRequestHeader,
    UA_Boolean                 a_bReleaseContinuationPoints,
    int32_t                    a_nNoOfContinuationPoints,
    const UA_ByteString*       a_pContinuationPoints,
    OpcUa_ResponseHeader*      a_pResponseHeader,
    int32_t*                   a_pNoOfResults,
    OpcUa_BrowseResult**       a_pResults,
    int32_t*                   a_pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**        a_pDiagnosticInfos)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_BrowseNextRequest cRequest;
    OpcUa_BrowseNextResponse* pResponse = NULL;
    UA_EncodeableType* pResponseType = NULL;
    
    OpcUa_BrowseNextRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  (a_pContinuationPoints != NULL || a_nNoOfContinuationPoints <= 0)
       &&  a_pResponseHeader != NULL
       &&  a_pNoOfResults != NULL
       &&  a_pResults != NULL
       &&  a_pNoOfDiagnosticInfos != NULL
       &&  a_pDiagnosticInfos != NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader             = *a_pRequestHeader;
        cRequest.ReleaseContinuationPoints = a_bReleaseContinuationPoints;
        cRequest.NoOfContinuationPoints    = a_nNoOfContinuationPoints;
        cRequest.ContinuationPoints        = (UA_ByteString*)a_pContinuationPoints;

        /* invoke service */
        status = UA_Channel_InvokeService(
            a_hChannel,
            "BrowseNext",
            (void*)&cRequest,
            &OpcUa_BrowseNextRequest_EncodeableType,
            &OpcUa_BrowseNextResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->TypeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((OpcUa_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (OpcUa_BrowseNextResponse_EncodeableType.TypeId != pResponseType->TypeId)
        {
            pResponseType->Clear(pResponse);
            status = OpcUa_BadUnknownResponse;
        }

        /* copy parameters from response object into return parameters. */
        else
        {
            *a_pResponseHeader      = pResponse->ResponseHeader;
            *a_pNoOfResults         = pResponse->NoOfResults;
            *a_pResults             = pResponse->Results;
            *a_pNoOfDiagnosticInfos = pResponse->NoOfDiagnosticInfos;
            *a_pDiagnosticInfos     = pResponse->DiagnosticInfos;
        }
    }else{
        //TODO: check if status invalid response could have been already deallocated in InvokeService
        /* memory contained in the reponse objects is owned by the caller */
        free(pResponse);
    }

    return status;
}

/*============================================================================
 * Asynchronously calls the BrowseNext service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginBrowseNext(
    UA_Channel                     a_hChannel,
    const OpcUa_RequestHeader*     a_pRequestHeader,
    UA_Boolean                     a_bReleaseContinuationPoints,
    int32_t                        a_nNoOfContinuationPoints,
    const UA_ByteString*           a_pContinuationPoints,
    UA_Channel_PfnRequestComplete* a_pCallback,
    void*                          a_pCallbackData)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_BrowseNextRequest cRequest;
    OpcUa_BrowseNextRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  (a_pContinuationPoints != NULL || a_nNoOfContinuationPoints <= 0))
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader             = *a_pRequestHeader;
        cRequest.ReleaseContinuationPoints = a_bReleaseContinuationPoints;
        cRequest.NoOfContinuationPoints    = a_nNoOfContinuationPoints;
        cRequest.ContinuationPoints        = (UA_ByteString*)a_pContinuationPoints;

        /* begin invoke service */
        status = UA_Channel_BeginInvokeService(
            a_hChannel,
            "BrowseNext",
            (void*)&cRequest,
            &OpcUa_BrowseNextRequest_EncodeableType,
            &OpcUa_BrowseNextResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*)a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_TranslateBrowsePathsToNodeIds
/*============================================================================
 * Synchronously calls the TranslateBrowsePathsToNodeIds service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_TranslateBrowsePathsToNodeIds(
    UA_Channel                 a_hChannel,
    const OpcUa_RequestHeader* a_pRequestHeader,
    int32_t                    a_nNoOfBrowsePaths,
    const OpcUa_BrowsePath*    a_pBrowsePaths,
    OpcUa_ResponseHeader*      a_pResponseHeader,
    int32_t*                   a_pNoOfResults,
    OpcUa_BrowsePathResult**   a_pResults,
    int32_t*                   a_pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**        a_pDiagnosticInfos)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_TranslateBrowsePathsToNodeIdsRequest cRequest;
    OpcUa_TranslateBrowsePathsToNodeIdsResponse* pResponse = NULL;
    UA_EncodeableType* pResponseType = NULL;
    
    OpcUa_TranslateBrowsePathsToNodeIdsRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  (a_pBrowsePaths != NULL || a_nNoOfBrowsePaths <= 0)
       &&  a_pResponseHeader != NULL
       &&  a_pNoOfResults != NULL
       &&  a_pResults != NULL
       &&  a_pNoOfDiagnosticInfos != NULL
       &&  a_pDiagnosticInfos != NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader   = *a_pRequestHeader;
        cRequest.NoOfBrowsePaths = a_nNoOfBrowsePaths;
        cRequest.BrowsePaths     = (OpcUa_BrowsePath*)a_pBrowsePaths;

        /* invoke service */
        status = UA_Channel_InvokeService(
            a_hChannel,
            "TranslateBrowsePathsToNodeIds",
            (void*)&cRequest,
            &OpcUa_TranslateBrowsePathsToNodeIdsRequest_EncodeableType,
            &OpcUa_TranslateBrowsePathsToNodeIdsResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->TypeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((OpcUa_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (OpcUa_TranslateBrowsePathsToNodeIdsResponse_EncodeableType.TypeId != pResponseType->TypeId)
        {
            pResponseType->Clear(pResponse);
            status = OpcUa_BadUnknownResponse;
        }

        /* copy parameters from response object into return parameters. */
        else
        {
            *a_pResponseHeader      = pResponse->ResponseHeader;
            *a_pNoOfResults         = pResponse->NoOfResults;
            *a_pResults             = pResponse->Results;
            *a_pNoOfDiagnosticInfos = pResponse->NoOfDiagnosticInfos;
            *a_pDiagnosticInfos     = pResponse->DiagnosticInfos;
        }
    }else{
        //TODO: check if status invalid response could have been already deallocated in InvokeService
        /* memory contained in the reponse objects is owned by the caller */
        free(pResponse);
    }

    return status;
}

/*============================================================================
 * Asynchronously calls the TranslateBrowsePathsToNodeIds service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginTranslateBrowsePathsToNodeIds(
    UA_Channel                     a_hChannel,
    const OpcUa_RequestHeader*     a_pRequestHeader,
    int32_t                        a_nNoOfBrowsePaths,
    const OpcUa_BrowsePath*        a_pBrowsePaths,
    UA_Channel_PfnRequestComplete* a_pCallback,
    void*                          a_pCallbackData)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_TranslateBrowsePathsToNodeIdsRequest cRequest;
    OpcUa_TranslateBrowsePathsToNodeIdsRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  (a_pBrowsePaths != NULL || a_nNoOfBrowsePaths <= 0))
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader   = *a_pRequestHeader;
        cRequest.NoOfBrowsePaths = a_nNoOfBrowsePaths;
        cRequest.BrowsePaths     = (OpcUa_BrowsePath*)a_pBrowsePaths;

        /* begin invoke service */
        status = UA_Channel_BeginInvokeService(
            a_hChannel,
            "TranslateBrowsePathsToNodeIds",
            (void*)&cRequest,
            &OpcUa_TranslateBrowsePathsToNodeIdsRequest_EncodeableType,
            &OpcUa_TranslateBrowsePathsToNodeIdsResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*)a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_RegisterNodes
/*============================================================================
 * Synchronously calls the RegisterNodes service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_RegisterNodes(
    UA_Channel                 a_hChannel,
    const OpcUa_RequestHeader* a_pRequestHeader,
    int32_t                    a_nNoOfNodesToRegister,
    const UA_NodeId*           a_pNodesToRegister,
    OpcUa_ResponseHeader*      a_pResponseHeader,
    int32_t*                   a_pNoOfRegisteredNodeIds,
    UA_NodeId**                a_pRegisteredNodeIds)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_RegisterNodesRequest cRequest;
    OpcUa_RegisterNodesResponse* pResponse = NULL;
    UA_EncodeableType* pResponseType = NULL;
    
    OpcUa_RegisterNodesRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  (a_pNodesToRegister != NULL || a_nNoOfNodesToRegister <= 0)
       &&  a_pResponseHeader != NULL
       &&  a_pNoOfRegisteredNodeIds != NULL
       &&  a_pRegisteredNodeIds != NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader       = *a_pRequestHeader;
        cRequest.NoOfNodesToRegister = a_nNoOfNodesToRegister;
        cRequest.NodesToRegister     = (UA_NodeId*)a_pNodesToRegister;

        /* invoke service */
        status = UA_Channel_InvokeService(
            a_hChannel,
            "RegisterNodes",
            (void*)&cRequest,
            &OpcUa_RegisterNodesRequest_EncodeableType,
            &OpcUa_RegisterNodesResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->TypeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((OpcUa_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (OpcUa_RegisterNodesResponse_EncodeableType.TypeId != pResponseType->TypeId)
        {
            pResponseType->Clear(pResponse);
            status = OpcUa_BadUnknownResponse;
        }

        /* copy parameters from response object into return parameters. */
        else
        {
            *a_pResponseHeader        = pResponse->ResponseHeader;
            *a_pNoOfRegisteredNodeIds = pResponse->NoOfRegisteredNodeIds;
            *a_pRegisteredNodeIds     = pResponse->RegisteredNodeIds;
        }
    }else{
        //TODO: check if status invalid response could have been already deallocated in InvokeService
        /* memory contained in the reponse objects is owned by the caller */
        free(pResponse);
    }

    return status;
}

/*============================================================================
 * Asynchronously calls the RegisterNodes service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginRegisterNodes(
    UA_Channel                     a_hChannel,
    const OpcUa_RequestHeader*     a_pRequestHeader,
    int32_t                        a_nNoOfNodesToRegister,
    const UA_NodeId*               a_pNodesToRegister,
    UA_Channel_PfnRequestComplete* a_pCallback,
    void*                          a_pCallbackData)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_RegisterNodesRequest cRequest;
    OpcUa_RegisterNodesRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  (a_pNodesToRegister != NULL || a_nNoOfNodesToRegister <= 0))
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader       = *a_pRequestHeader;
        cRequest.NoOfNodesToRegister = a_nNoOfNodesToRegister;
        cRequest.NodesToRegister     = (UA_NodeId*)a_pNodesToRegister;

        /* begin invoke service */
        status = UA_Channel_BeginInvokeService(
            a_hChannel,
            "RegisterNodes",
            (void*)&cRequest,
            &OpcUa_RegisterNodesRequest_EncodeableType,
            &OpcUa_RegisterNodesResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*)a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_UnregisterNodes
/*============================================================================
 * Synchronously calls the UnregisterNodes service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_UnregisterNodes(
    UA_Channel                 a_hChannel,
    const OpcUa_RequestHeader* a_pRequestHeader,
    int32_t                    a_nNoOfNodesToUnregister,
    const UA_NodeId*           a_pNodesToUnregister,
    OpcUa_ResponseHeader*      a_pResponseHeader)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_UnregisterNodesRequest cRequest;
    OpcUa_UnregisterNodesResponse* pResponse = NULL;
    UA_EncodeableType* pResponseType = NULL;
    
    OpcUa_UnregisterNodesRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  (a_pNodesToUnregister != NULL || a_nNoOfNodesToUnregister <= 0)
       &&  a_pResponseHeader != NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader         = *a_pRequestHeader;
        cRequest.NoOfNodesToUnregister = a_nNoOfNodesToUnregister;
        cRequest.NodesToUnregister     = (UA_NodeId*)a_pNodesToUnregister;

        /* invoke service */
        status = UA_Channel_InvokeService(
            a_hChannel,
            "UnregisterNodes",
            (void*)&cRequest,
            &OpcUa_UnregisterNodesRequest_EncodeableType,
            &OpcUa_UnregisterNodesResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->TypeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((OpcUa_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (OpcUa_UnregisterNodesResponse_EncodeableType.TypeId != pResponseType->TypeId)
        {
            pResponseType->Clear(pResponse);
            status = OpcUa_BadUnknownResponse;
        }

        /* copy parameters from response object into return parameters. */
        else
        {
            *a_pResponseHeader = pResponse->ResponseHeader;
        }
    }else{
        //TODO: check if status invalid response could have been already deallocated in InvokeService
        /* memory contained in the reponse objects is owned by the caller */
        free(pResponse);
    }

    return status;
}

/*============================================================================
 * Asynchronously calls the UnregisterNodes service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginUnregisterNodes(
    UA_Channel                     a_hChannel,
    const OpcUa_RequestHeader*     a_pRequestHeader,
    int32_t                        a_nNoOfNodesToUnregister,
    const UA_NodeId*               a_pNodesToUnregister,
    UA_Channel_PfnRequestComplete* a_pCallback,
    void*                          a_pCallbackData)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_UnregisterNodesRequest cRequest;
    OpcUa_UnregisterNodesRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  (a_pNodesToUnregister != NULL || a_nNoOfNodesToUnregister <= 0))
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader         = *a_pRequestHeader;
        cRequest.NoOfNodesToUnregister = a_nNoOfNodesToUnregister;
        cRequest.NodesToUnregister     = (UA_NodeId*)a_pNodesToUnregister;

        /* begin invoke service */
        status = UA_Channel_BeginInvokeService(
            a_hChannel,
            "UnregisterNodes",
            (void*)&cRequest,
            &OpcUa_UnregisterNodesRequest_EncodeableType,
            &OpcUa_UnregisterNodesResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*)a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_QueryFirst
/*============================================================================
 * Synchronously calls the QueryFirst service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_QueryFirst(
    UA_Channel                       a_hChannel,
    const OpcUa_RequestHeader*       a_pRequestHeader,
    const OpcUa_ViewDescription*     a_pView,
    int32_t                          a_nNoOfNodeTypes,
    const OpcUa_NodeTypeDescription* a_pNodeTypes,
    const OpcUa_ContentFilter*       a_pFilter,
    uint32_t                         a_nMaxDataSetsToReturn,
    uint32_t                         a_nMaxReferencesToReturn,
    OpcUa_ResponseHeader*            a_pResponseHeader,
    int32_t*                         a_pNoOfQueryDataSets,
    OpcUa_QueryDataSet**             a_pQueryDataSets,
    UA_ByteString*                   a_pContinuationPoint,
    int32_t*                         a_pNoOfParsingResults,
    OpcUa_ParsingResult**            a_pParsingResults,
    int32_t*                         a_pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**              a_pDiagnosticInfos,
    OpcUa_ContentFilterResult*       a_pFilterResult)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_QueryFirstRequest cRequest;
    OpcUa_QueryFirstResponse* pResponse = NULL;
    UA_EncodeableType* pResponseType = NULL;
    
    OpcUa_QueryFirstRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  a_pView != NULL
       &&  (a_pNodeTypes != NULL || a_nNoOfNodeTypes <= 0)
       &&  a_pFilter != NULL
       &&  a_pResponseHeader != NULL
       &&  a_pNoOfQueryDataSets != NULL
       &&  a_pQueryDataSets != NULL
       &&  a_pContinuationPoint != NULL
       &&  a_pNoOfParsingResults != NULL
       &&  a_pParsingResults != NULL
       &&  a_pNoOfDiagnosticInfos != NULL
       &&  a_pDiagnosticInfos != NULL
       &&  a_pFilterResult != NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader         = *a_pRequestHeader;
        cRequest.View                  = *a_pView;
        cRequest.NoOfNodeTypes         = a_nNoOfNodeTypes;
        cRequest.NodeTypes             = (OpcUa_NodeTypeDescription*)a_pNodeTypes;
        cRequest.Filter                = *a_pFilter;
        cRequest.MaxDataSetsToReturn   = a_nMaxDataSetsToReturn;
        cRequest.MaxReferencesToReturn = a_nMaxReferencesToReturn;

        /* invoke service */
        status = UA_Channel_InvokeService(
            a_hChannel,
            "QueryFirst",
            (void*)&cRequest,
            &OpcUa_QueryFirstRequest_EncodeableType,
            &OpcUa_QueryFirstResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->TypeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((OpcUa_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (OpcUa_QueryFirstResponse_EncodeableType.TypeId != pResponseType->TypeId)
        {
            pResponseType->Clear(pResponse);
            status = OpcUa_BadUnknownResponse;
        }

        /* copy parameters from response object into return parameters. */
        else
        {
            *a_pResponseHeader      = pResponse->ResponseHeader;
            *a_pNoOfQueryDataSets   = pResponse->NoOfQueryDataSets;
            *a_pQueryDataSets       = pResponse->QueryDataSets;
            *a_pContinuationPoint   = pResponse->ContinuationPoint;
            *a_pNoOfParsingResults  = pResponse->NoOfParsingResults;
            *a_pParsingResults      = pResponse->ParsingResults;
            *a_pNoOfDiagnosticInfos = pResponse->NoOfDiagnosticInfos;
            *a_pDiagnosticInfos     = pResponse->DiagnosticInfos;
            *a_pFilterResult        = pResponse->FilterResult;
        }
    }else{
        //TODO: check if status invalid response could have been already deallocated in InvokeService
        /* memory contained in the reponse objects is owned by the caller */
        free(pResponse);
    }

    return status;
}

/*============================================================================
 * Asynchronously calls the QueryFirst service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginQueryFirst(
    UA_Channel                       a_hChannel,
    const OpcUa_RequestHeader*       a_pRequestHeader,
    const OpcUa_ViewDescription*     a_pView,
    int32_t                          a_nNoOfNodeTypes,
    const OpcUa_NodeTypeDescription* a_pNodeTypes,
    const OpcUa_ContentFilter*       a_pFilter,
    uint32_t                         a_nMaxDataSetsToReturn,
    uint32_t                         a_nMaxReferencesToReturn,
    UA_Channel_PfnRequestComplete*   a_pCallback,
    void*                            a_pCallbackData)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_QueryFirstRequest cRequest;
    OpcUa_QueryFirstRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  a_pView != NULL
       &&  (a_pNodeTypes != NULL || a_nNoOfNodeTypes <= 0)
       &&  a_pFilter != NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader         = *a_pRequestHeader;
        cRequest.View                  = *a_pView;
        cRequest.NoOfNodeTypes         = a_nNoOfNodeTypes;
        cRequest.NodeTypes             = (OpcUa_NodeTypeDescription*)a_pNodeTypes;
        cRequest.Filter                = *a_pFilter;
        cRequest.MaxDataSetsToReturn   = a_nMaxDataSetsToReturn;
        cRequest.MaxReferencesToReturn = a_nMaxReferencesToReturn;

        /* begin invoke service */
        status = UA_Channel_BeginInvokeService(
            a_hChannel,
            "QueryFirst",
            (void*)&cRequest,
            &OpcUa_QueryFirstRequest_EncodeableType,
            &OpcUa_QueryFirstResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*)a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_QueryNext
/*============================================================================
 * Synchronously calls the QueryNext service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_QueryNext(
    UA_Channel                 a_hChannel,
    const OpcUa_RequestHeader* a_pRequestHeader,
    UA_Boolean                 a_bReleaseContinuationPoint,
    const UA_ByteString*       a_pContinuationPoint,
    OpcUa_ResponseHeader*      a_pResponseHeader,
    int32_t*                   a_pNoOfQueryDataSets,
    OpcUa_QueryDataSet**       a_pQueryDataSets,
    UA_ByteString*             a_pRevisedContinuationPoint)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_QueryNextRequest cRequest;
    OpcUa_QueryNextResponse* pResponse = NULL;
    UA_EncodeableType* pResponseType = NULL;
    
    OpcUa_QueryNextRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  a_pContinuationPoint != NULL
       &&  a_pResponseHeader != NULL
       &&  a_pNoOfQueryDataSets != NULL
       &&  a_pQueryDataSets != NULL
       &&  a_pRevisedContinuationPoint != NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader            = *a_pRequestHeader;
        cRequest.ReleaseContinuationPoint = a_bReleaseContinuationPoint;
        cRequest.ContinuationPoint        = *a_pContinuationPoint;

        /* invoke service */
        status = UA_Channel_InvokeService(
            a_hChannel,
            "QueryNext",
            (void*)&cRequest,
            &OpcUa_QueryNextRequest_EncodeableType,
            &OpcUa_QueryNextResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->TypeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((OpcUa_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (OpcUa_QueryNextResponse_EncodeableType.TypeId != pResponseType->TypeId)
        {
            pResponseType->Clear(pResponse);
            status = OpcUa_BadUnknownResponse;
        }

        /* copy parameters from response object into return parameters. */
        else
        {
            *a_pResponseHeader           = pResponse->ResponseHeader;
            *a_pNoOfQueryDataSets        = pResponse->NoOfQueryDataSets;
            *a_pQueryDataSets            = pResponse->QueryDataSets;
            *a_pRevisedContinuationPoint = pResponse->RevisedContinuationPoint;
        }
    }else{
        //TODO: check if status invalid response could have been already deallocated in InvokeService
        /* memory contained in the reponse objects is owned by the caller */
        free(pResponse);
    }

    return status;
}

/*============================================================================
 * Asynchronously calls the QueryNext service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginQueryNext(
    UA_Channel                     a_hChannel,
    const OpcUa_RequestHeader*     a_pRequestHeader,
    UA_Boolean                     a_bReleaseContinuationPoint,
    const UA_ByteString*           a_pContinuationPoint,
    UA_Channel_PfnRequestComplete* a_pCallback,
    void*                          a_pCallbackData)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_QueryNextRequest cRequest;
    OpcUa_QueryNextRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  a_pContinuationPoint != NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader            = *a_pRequestHeader;
        cRequest.ReleaseContinuationPoint = a_bReleaseContinuationPoint;
        cRequest.ContinuationPoint        = *a_pContinuationPoint;

        /* begin invoke service */
        status = UA_Channel_BeginInvokeService(
            a_hChannel,
            "QueryNext",
            (void*)&cRequest,
            &OpcUa_QueryNextRequest_EncodeableType,
            &OpcUa_QueryNextResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*)a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_Read
/*============================================================================
 * Synchronously calls the Read service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_Read(
    UA_Channel                 a_hChannel,
    const OpcUa_RequestHeader* a_pRequestHeader,
    double                     a_nMaxAge,
    OpcUa_TimestampsToReturn   a_eTimestampsToReturn,
    int32_t                    a_nNoOfNodesToRead,
    const OpcUa_ReadValueId*   a_pNodesToRead,
    OpcUa_ResponseHeader*      a_pResponseHeader,
    int32_t*                   a_pNoOfResults,
    UA_DataValue**             a_pResults,
    int32_t*                   a_pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**        a_pDiagnosticInfos)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_ReadRequest cRequest;
    OpcUa_ReadResponse* pResponse = NULL;
    UA_EncodeableType* pResponseType = NULL;
    
    OpcUa_ReadRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  (a_pNodesToRead != NULL || a_nNoOfNodesToRead <= 0)
       &&  a_pResponseHeader != NULL
       &&  a_pNoOfResults != NULL
       &&  a_pResults != NULL
       &&  a_pNoOfDiagnosticInfos != NULL
       &&  a_pDiagnosticInfos != NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader      = *a_pRequestHeader;
        cRequest.MaxAge             = a_nMaxAge;
        cRequest.TimestampsToReturn = a_eTimestampsToReturn;
        cRequest.NoOfNodesToRead    = a_nNoOfNodesToRead;
        cRequest.NodesToRead        = (OpcUa_ReadValueId*)a_pNodesToRead;

        /* invoke service */
        status = UA_Channel_InvokeService(
            a_hChannel,
            "Read",
            (void*)&cRequest,
            &OpcUa_ReadRequest_EncodeableType,
            &OpcUa_ReadResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->TypeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((OpcUa_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (OpcUa_ReadResponse_EncodeableType.TypeId != pResponseType->TypeId)
        {
            pResponseType->Clear(pResponse);
            status = OpcUa_BadUnknownResponse;
        }

        /* copy parameters from response object into return parameters. */
        else
        {
            *a_pResponseHeader      = pResponse->ResponseHeader;
            *a_pNoOfResults         = pResponse->NoOfResults;
            *a_pResults             = pResponse->Results;
            *a_pNoOfDiagnosticInfos = pResponse->NoOfDiagnosticInfos;
            *a_pDiagnosticInfos     = pResponse->DiagnosticInfos;
        }
    }else{
        //TODO: check if status invalid response could have been already deallocated in InvokeService
        /* memory contained in the reponse objects is owned by the caller */
        free(pResponse);
    }

    return status;
}

/*============================================================================
 * Asynchronously calls the Read service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginRead(
    UA_Channel                     a_hChannel,
    const OpcUa_RequestHeader*     a_pRequestHeader,
    double                         a_nMaxAge,
    OpcUa_TimestampsToReturn       a_eTimestampsToReturn,
    int32_t                        a_nNoOfNodesToRead,
    const OpcUa_ReadValueId*       a_pNodesToRead,
    UA_Channel_PfnRequestComplete* a_pCallback,
    void*                          a_pCallbackData)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_ReadRequest cRequest;
    OpcUa_ReadRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  (a_pNodesToRead != NULL || a_nNoOfNodesToRead <= 0))
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader      = *a_pRequestHeader;
        cRequest.MaxAge             = a_nMaxAge;
        cRequest.TimestampsToReturn = a_eTimestampsToReturn;
        cRequest.NoOfNodesToRead    = a_nNoOfNodesToRead;
        cRequest.NodesToRead        = (OpcUa_ReadValueId*)a_pNodesToRead;

        /* begin invoke service */
        status = UA_Channel_BeginInvokeService(
            a_hChannel,
            "Read",
            (void*)&cRequest,
            &OpcUa_ReadRequest_EncodeableType,
            &OpcUa_ReadResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*)a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_HistoryRead
/*============================================================================
 * Synchronously calls the HistoryRead service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_HistoryRead(
    UA_Channel                      a_hChannel,
    const OpcUa_RequestHeader*      a_pRequestHeader,
    const UA_ExtensionObject*       a_pHistoryReadDetails,
    OpcUa_TimestampsToReturn        a_eTimestampsToReturn,
    UA_Boolean                      a_bReleaseContinuationPoints,
    int32_t                         a_nNoOfNodesToRead,
    const OpcUa_HistoryReadValueId* a_pNodesToRead,
    OpcUa_ResponseHeader*           a_pResponseHeader,
    int32_t*                        a_pNoOfResults,
    OpcUa_HistoryReadResult**       a_pResults,
    int32_t*                        a_pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**             a_pDiagnosticInfos)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_HistoryReadRequest cRequest;
    OpcUa_HistoryReadResponse* pResponse = NULL;
    UA_EncodeableType* pResponseType = NULL;
    
    OpcUa_HistoryReadRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  a_pHistoryReadDetails != NULL
       &&  (a_pNodesToRead != NULL || a_nNoOfNodesToRead <= 0)
       &&  a_pResponseHeader != NULL
       &&  a_pNoOfResults != NULL
       &&  a_pResults != NULL
       &&  a_pNoOfDiagnosticInfos != NULL
       &&  a_pDiagnosticInfos != NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader             = *a_pRequestHeader;
        cRequest.HistoryReadDetails        = *a_pHistoryReadDetails;
        cRequest.TimestampsToReturn        = a_eTimestampsToReturn;
        cRequest.ReleaseContinuationPoints = a_bReleaseContinuationPoints;
        cRequest.NoOfNodesToRead           = a_nNoOfNodesToRead;
        cRequest.NodesToRead               = (OpcUa_HistoryReadValueId*)a_pNodesToRead;

        /* invoke service */
        status = UA_Channel_InvokeService(
            a_hChannel,
            "HistoryRead",
            (void*)&cRequest,
            &OpcUa_HistoryReadRequest_EncodeableType,
            &OpcUa_HistoryReadResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->TypeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((OpcUa_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (OpcUa_HistoryReadResponse_EncodeableType.TypeId != pResponseType->TypeId)
        {
            pResponseType->Clear(pResponse);
            status = OpcUa_BadUnknownResponse;
        }

        /* copy parameters from response object into return parameters. */
        else
        {
            *a_pResponseHeader      = pResponse->ResponseHeader;
            *a_pNoOfResults         = pResponse->NoOfResults;
            *a_pResults             = pResponse->Results;
            *a_pNoOfDiagnosticInfos = pResponse->NoOfDiagnosticInfos;
            *a_pDiagnosticInfos     = pResponse->DiagnosticInfos;
        }
    }else{
        //TODO: check if status invalid response could have been already deallocated in InvokeService
        /* memory contained in the reponse objects is owned by the caller */
        free(pResponse);
    }

    return status;
}

/*============================================================================
 * Asynchronously calls the HistoryRead service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginHistoryRead(
    UA_Channel                      a_hChannel,
    const OpcUa_RequestHeader*      a_pRequestHeader,
    const UA_ExtensionObject*       a_pHistoryReadDetails,
    OpcUa_TimestampsToReturn        a_eTimestampsToReturn,
    UA_Boolean                      a_bReleaseContinuationPoints,
    int32_t                         a_nNoOfNodesToRead,
    const OpcUa_HistoryReadValueId* a_pNodesToRead,
    UA_Channel_PfnRequestComplete*  a_pCallback,
    void*                           a_pCallbackData)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_HistoryReadRequest cRequest;
    OpcUa_HistoryReadRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  a_pHistoryReadDetails != NULL
       &&  (a_pNodesToRead != NULL || a_nNoOfNodesToRead <= 0))
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader             = *a_pRequestHeader;
        cRequest.HistoryReadDetails        = *a_pHistoryReadDetails;
        cRequest.TimestampsToReturn        = a_eTimestampsToReturn;
        cRequest.ReleaseContinuationPoints = a_bReleaseContinuationPoints;
        cRequest.NoOfNodesToRead           = a_nNoOfNodesToRead;
        cRequest.NodesToRead               = (OpcUa_HistoryReadValueId*)a_pNodesToRead;

        /* begin invoke service */
        status = UA_Channel_BeginInvokeService(
            a_hChannel,
            "HistoryRead",
            (void*)&cRequest,
            &OpcUa_HistoryReadRequest_EncodeableType,
            &OpcUa_HistoryReadResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*)a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_Write
/*============================================================================
 * Synchronously calls the Write service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_Write(
    UA_Channel                 a_hChannel,
    const OpcUa_RequestHeader* a_pRequestHeader,
    int32_t                    a_nNoOfNodesToWrite,
    const OpcUa_WriteValue*    a_pNodesToWrite,
    OpcUa_ResponseHeader*      a_pResponseHeader,
    int32_t*                   a_pNoOfResults,
    SOPC_StatusCode**               a_pResults,
    int32_t*                   a_pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**        a_pDiagnosticInfos)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_WriteRequest cRequest;
    OpcUa_WriteResponse* pResponse = NULL;
    UA_EncodeableType* pResponseType = NULL;
    
    OpcUa_WriteRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  (a_pNodesToWrite != NULL || a_nNoOfNodesToWrite <= 0)
       &&  a_pResponseHeader != NULL
       &&  a_pNoOfResults != NULL
       &&  a_pResults != NULL
       &&  a_pNoOfDiagnosticInfos != NULL
       &&  a_pDiagnosticInfos != NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader    = *a_pRequestHeader;
        cRequest.NoOfNodesToWrite = a_nNoOfNodesToWrite;
        cRequest.NodesToWrite     = (OpcUa_WriteValue*)a_pNodesToWrite;

        /* invoke service */
        status = UA_Channel_InvokeService(
            a_hChannel,
            "Write",
            (void*)&cRequest,
            &OpcUa_WriteRequest_EncodeableType,
            &OpcUa_WriteResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->TypeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((OpcUa_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (OpcUa_WriteResponse_EncodeableType.TypeId != pResponseType->TypeId)
        {
            pResponseType->Clear(pResponse);
            status = OpcUa_BadUnknownResponse;
        }

        /* copy parameters from response object into return parameters. */
        else
        {
            *a_pResponseHeader      = pResponse->ResponseHeader;
            *a_pNoOfResults         = pResponse->NoOfResults;
            *a_pResults             = pResponse->Results;
            *a_pNoOfDiagnosticInfos = pResponse->NoOfDiagnosticInfos;
            *a_pDiagnosticInfos     = pResponse->DiagnosticInfos;
        }
    }else{
        //TODO: check if status invalid response could have been already deallocated in InvokeService
        /* memory contained in the reponse objects is owned by the caller */
        free(pResponse);
    }

    return status;
}

/*============================================================================
 * Asynchronously calls the Write service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginWrite(
    UA_Channel                     a_hChannel,
    const OpcUa_RequestHeader*     a_pRequestHeader,
    int32_t                        a_nNoOfNodesToWrite,
    const OpcUa_WriteValue*        a_pNodesToWrite,
    UA_Channel_PfnRequestComplete* a_pCallback,
    void*                          a_pCallbackData)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_WriteRequest cRequest;
    OpcUa_WriteRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  (a_pNodesToWrite != NULL || a_nNoOfNodesToWrite <= 0))
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader    = *a_pRequestHeader;
        cRequest.NoOfNodesToWrite = a_nNoOfNodesToWrite;
        cRequest.NodesToWrite     = (OpcUa_WriteValue*)a_pNodesToWrite;

        /* begin invoke service */
        status = UA_Channel_BeginInvokeService(
            a_hChannel,
            "Write",
            (void*)&cRequest,
            &OpcUa_WriteRequest_EncodeableType,
            &OpcUa_WriteResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*)a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_HistoryUpdate
/*============================================================================
 * Synchronously calls the HistoryUpdate service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_HistoryUpdate(
    UA_Channel                  a_hChannel,
    const OpcUa_RequestHeader*  a_pRequestHeader,
    int32_t                     a_nNoOfHistoryUpdateDetails,
    const UA_ExtensionObject*   a_pHistoryUpdateDetails,
    OpcUa_ResponseHeader*       a_pResponseHeader,
    int32_t*                    a_pNoOfResults,
    OpcUa_HistoryUpdateResult** a_pResults,
    int32_t*                    a_pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**         a_pDiagnosticInfos)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_HistoryUpdateRequest cRequest;
    OpcUa_HistoryUpdateResponse* pResponse = NULL;
    UA_EncodeableType* pResponseType = NULL;
    
    OpcUa_HistoryUpdateRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  (a_pHistoryUpdateDetails != NULL || a_nNoOfHistoryUpdateDetails <= 0)
       &&  a_pResponseHeader != NULL
       &&  a_pNoOfResults != NULL
       &&  a_pResults != NULL
       &&  a_pNoOfDiagnosticInfos != NULL
       &&  a_pDiagnosticInfos != NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader            = *a_pRequestHeader;
        cRequest.NoOfHistoryUpdateDetails = a_nNoOfHistoryUpdateDetails;
        cRequest.HistoryUpdateDetails     = (UA_ExtensionObject*)a_pHistoryUpdateDetails;

        /* invoke service */
        status = UA_Channel_InvokeService(
            a_hChannel,
            "HistoryUpdate",
            (void*)&cRequest,
            &OpcUa_HistoryUpdateRequest_EncodeableType,
            &OpcUa_HistoryUpdateResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->TypeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((OpcUa_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (OpcUa_HistoryUpdateResponse_EncodeableType.TypeId != pResponseType->TypeId)
        {
            pResponseType->Clear(pResponse);
            status = OpcUa_BadUnknownResponse;
        }

        /* copy parameters from response object into return parameters. */
        else
        {
            *a_pResponseHeader      = pResponse->ResponseHeader;
            *a_pNoOfResults         = pResponse->NoOfResults;
            *a_pResults             = pResponse->Results;
            *a_pNoOfDiagnosticInfos = pResponse->NoOfDiagnosticInfos;
            *a_pDiagnosticInfos     = pResponse->DiagnosticInfos;
        }
    }else{
        //TODO: check if status invalid response could have been already deallocated in InvokeService
        /* memory contained in the reponse objects is owned by the caller */
        free(pResponse);
    }

    return status;
}

/*============================================================================
 * Asynchronously calls the HistoryUpdate service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginHistoryUpdate(
    UA_Channel                     a_hChannel,
    const OpcUa_RequestHeader*     a_pRequestHeader,
    int32_t                        a_nNoOfHistoryUpdateDetails,
    const UA_ExtensionObject*      a_pHistoryUpdateDetails,
    UA_Channel_PfnRequestComplete* a_pCallback,
    void*                          a_pCallbackData)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_HistoryUpdateRequest cRequest;
    OpcUa_HistoryUpdateRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  (a_pHistoryUpdateDetails != NULL || a_nNoOfHistoryUpdateDetails <= 0))
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader            = *a_pRequestHeader;
        cRequest.NoOfHistoryUpdateDetails = a_nNoOfHistoryUpdateDetails;
        cRequest.HistoryUpdateDetails     = (UA_ExtensionObject*)a_pHistoryUpdateDetails;

        /* begin invoke service */
        status = UA_Channel_BeginInvokeService(
            a_hChannel,
            "HistoryUpdate",
            (void*)&cRequest,
            &OpcUa_HistoryUpdateRequest_EncodeableType,
            &OpcUa_HistoryUpdateResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*)a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_Call
/*============================================================================
 * Synchronously calls the Call service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_Call(
    UA_Channel                     a_hChannel,
    const OpcUa_RequestHeader*     a_pRequestHeader,
    int32_t                        a_nNoOfMethodsToCall,
    const OpcUa_CallMethodRequest* a_pMethodsToCall,
    OpcUa_ResponseHeader*          a_pResponseHeader,
    int32_t*                       a_pNoOfResults,
    OpcUa_CallMethodResult**       a_pResults,
    int32_t*                       a_pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**            a_pDiagnosticInfos)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_CallRequest cRequest;
    OpcUa_CallResponse* pResponse = NULL;
    UA_EncodeableType* pResponseType = NULL;
    
    OpcUa_CallRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  (a_pMethodsToCall != NULL || a_nNoOfMethodsToCall <= 0)
       &&  a_pResponseHeader != NULL
       &&  a_pNoOfResults != NULL
       &&  a_pResults != NULL
       &&  a_pNoOfDiagnosticInfos != NULL
       &&  a_pDiagnosticInfos != NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader     = *a_pRequestHeader;
        cRequest.NoOfMethodsToCall = a_nNoOfMethodsToCall;
        cRequest.MethodsToCall     = (OpcUa_CallMethodRequest*)a_pMethodsToCall;

        /* invoke service */
        status = UA_Channel_InvokeService(
            a_hChannel,
            "Call",
            (void*)&cRequest,
            &OpcUa_CallRequest_EncodeableType,
            &OpcUa_CallResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->TypeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((OpcUa_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (OpcUa_CallResponse_EncodeableType.TypeId != pResponseType->TypeId)
        {
            pResponseType->Clear(pResponse);
            status = OpcUa_BadUnknownResponse;
        }

        /* copy parameters from response object into return parameters. */
        else
        {
            *a_pResponseHeader      = pResponse->ResponseHeader;
            *a_pNoOfResults         = pResponse->NoOfResults;
            *a_pResults             = pResponse->Results;
            *a_pNoOfDiagnosticInfos = pResponse->NoOfDiagnosticInfos;
            *a_pDiagnosticInfos     = pResponse->DiagnosticInfos;
        }
    }else{
        //TODO: check if status invalid response could have been already deallocated in InvokeService
        /* memory contained in the reponse objects is owned by the caller */
        free(pResponse);
    }

    return status;
}

/*============================================================================
 * Asynchronously calls the Call service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginCall(
    UA_Channel                     a_hChannel,
    const OpcUa_RequestHeader*     a_pRequestHeader,
    int32_t                        a_nNoOfMethodsToCall,
    const OpcUa_CallMethodRequest* a_pMethodsToCall,
    UA_Channel_PfnRequestComplete* a_pCallback,
    void*                          a_pCallbackData)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_CallRequest cRequest;
    OpcUa_CallRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  (a_pMethodsToCall != NULL || a_nNoOfMethodsToCall <= 0))
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader     = *a_pRequestHeader;
        cRequest.NoOfMethodsToCall = a_nNoOfMethodsToCall;
        cRequest.MethodsToCall     = (OpcUa_CallMethodRequest*)a_pMethodsToCall;

        /* begin invoke service */
        status = UA_Channel_BeginInvokeService(
            a_hChannel,
            "Call",
            (void*)&cRequest,
            &OpcUa_CallRequest_EncodeableType,
            &OpcUa_CallResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*)a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_CreateMonitoredItems
/*============================================================================
 * Synchronously calls the CreateMonitoredItems service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_CreateMonitoredItems(
    UA_Channel                              a_hChannel,
    const OpcUa_RequestHeader*              a_pRequestHeader,
    uint32_t                                a_nSubscriptionId,
    OpcUa_TimestampsToReturn                a_eTimestampsToReturn,
    int32_t                                 a_nNoOfItemsToCreate,
    const OpcUa_MonitoredItemCreateRequest* a_pItemsToCreate,
    OpcUa_ResponseHeader*                   a_pResponseHeader,
    int32_t*                                a_pNoOfResults,
    OpcUa_MonitoredItemCreateResult**       a_pResults,
    int32_t*                                a_pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**                     a_pDiagnosticInfos)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_CreateMonitoredItemsRequest cRequest;
    OpcUa_CreateMonitoredItemsResponse* pResponse = NULL;
    UA_EncodeableType* pResponseType = NULL;
    
    OpcUa_CreateMonitoredItemsRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  (a_pItemsToCreate != NULL || a_nNoOfItemsToCreate <= 0)
       &&  a_pResponseHeader != NULL
       &&  a_pNoOfResults != NULL
       &&  a_pResults != NULL
       &&  a_pNoOfDiagnosticInfos != NULL
       &&  a_pDiagnosticInfos != NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader      = *a_pRequestHeader;
        cRequest.SubscriptionId     = a_nSubscriptionId;
        cRequest.TimestampsToReturn = a_eTimestampsToReturn;
        cRequest.NoOfItemsToCreate  = a_nNoOfItemsToCreate;
        cRequest.ItemsToCreate      = (OpcUa_MonitoredItemCreateRequest*)a_pItemsToCreate;

        /* invoke service */
        status = UA_Channel_InvokeService(
            a_hChannel,
            "CreateMonitoredItems",
            (void*)&cRequest,
            &OpcUa_CreateMonitoredItemsRequest_EncodeableType,
            &OpcUa_CreateMonitoredItemsResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->TypeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((OpcUa_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (OpcUa_CreateMonitoredItemsResponse_EncodeableType.TypeId != pResponseType->TypeId)
        {
            pResponseType->Clear(pResponse);
            status = OpcUa_BadUnknownResponse;
        }

        /* copy parameters from response object into return parameters. */
        else
        {
            *a_pResponseHeader      = pResponse->ResponseHeader;
            *a_pNoOfResults         = pResponse->NoOfResults;
            *a_pResults             = pResponse->Results;
            *a_pNoOfDiagnosticInfos = pResponse->NoOfDiagnosticInfos;
            *a_pDiagnosticInfos     = pResponse->DiagnosticInfos;
        }
    }else{
        //TODO: check if status invalid response could have been already deallocated in InvokeService
        /* memory contained in the reponse objects is owned by the caller */
        free(pResponse);
    }

    return status;
}

/*============================================================================
 * Asynchronously calls the CreateMonitoredItems service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginCreateMonitoredItems(
    UA_Channel                              a_hChannel,
    const OpcUa_RequestHeader*              a_pRequestHeader,
    uint32_t                                a_nSubscriptionId,
    OpcUa_TimestampsToReturn                a_eTimestampsToReturn,
    int32_t                                 a_nNoOfItemsToCreate,
    const OpcUa_MonitoredItemCreateRequest* a_pItemsToCreate,
    UA_Channel_PfnRequestComplete*          a_pCallback,
    void*                                   a_pCallbackData)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_CreateMonitoredItemsRequest cRequest;
    OpcUa_CreateMonitoredItemsRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  (a_pItemsToCreate != NULL || a_nNoOfItemsToCreate <= 0))
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader      = *a_pRequestHeader;
        cRequest.SubscriptionId     = a_nSubscriptionId;
        cRequest.TimestampsToReturn = a_eTimestampsToReturn;
        cRequest.NoOfItemsToCreate  = a_nNoOfItemsToCreate;
        cRequest.ItemsToCreate      = (OpcUa_MonitoredItemCreateRequest*)a_pItemsToCreate;

        /* begin invoke service */
        status = UA_Channel_BeginInvokeService(
            a_hChannel,
            "CreateMonitoredItems",
            (void*)&cRequest,
            &OpcUa_CreateMonitoredItemsRequest_EncodeableType,
            &OpcUa_CreateMonitoredItemsResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*)a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_ModifyMonitoredItems
/*============================================================================
 * Synchronously calls the ModifyMonitoredItems service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_ModifyMonitoredItems(
    UA_Channel                              a_hChannel,
    const OpcUa_RequestHeader*              a_pRequestHeader,
    uint32_t                                a_nSubscriptionId,
    OpcUa_TimestampsToReturn                a_eTimestampsToReturn,
    int32_t                                 a_nNoOfItemsToModify,
    const OpcUa_MonitoredItemModifyRequest* a_pItemsToModify,
    OpcUa_ResponseHeader*                   a_pResponseHeader,
    int32_t*                                a_pNoOfResults,
    OpcUa_MonitoredItemModifyResult**       a_pResults,
    int32_t*                                a_pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**                     a_pDiagnosticInfos)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_ModifyMonitoredItemsRequest cRequest;
    OpcUa_ModifyMonitoredItemsResponse* pResponse = NULL;
    UA_EncodeableType* pResponseType = NULL;
    
    OpcUa_ModifyMonitoredItemsRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  (a_pItemsToModify != NULL || a_nNoOfItemsToModify <= 0)
       &&  a_pResponseHeader != NULL
       &&  a_pNoOfResults != NULL
       &&  a_pResults != NULL
       &&  a_pNoOfDiagnosticInfos != NULL
       &&  a_pDiagnosticInfos != NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader      = *a_pRequestHeader;
        cRequest.SubscriptionId     = a_nSubscriptionId;
        cRequest.TimestampsToReturn = a_eTimestampsToReturn;
        cRequest.NoOfItemsToModify  = a_nNoOfItemsToModify;
        cRequest.ItemsToModify      = (OpcUa_MonitoredItemModifyRequest*)a_pItemsToModify;

        /* invoke service */
        status = UA_Channel_InvokeService(
            a_hChannel,
            "ModifyMonitoredItems",
            (void*)&cRequest,
            &OpcUa_ModifyMonitoredItemsRequest_EncodeableType,
            &OpcUa_ModifyMonitoredItemsResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->TypeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((OpcUa_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (OpcUa_ModifyMonitoredItemsResponse_EncodeableType.TypeId != pResponseType->TypeId)
        {
            pResponseType->Clear(pResponse);
            status = OpcUa_BadUnknownResponse;
        }

        /* copy parameters from response object into return parameters. */
        else
        {
            *a_pResponseHeader      = pResponse->ResponseHeader;
            *a_pNoOfResults         = pResponse->NoOfResults;
            *a_pResults             = pResponse->Results;
            *a_pNoOfDiagnosticInfos = pResponse->NoOfDiagnosticInfos;
            *a_pDiagnosticInfos     = pResponse->DiagnosticInfos;
        }
    }else{
        //TODO: check if status invalid response could have been already deallocated in InvokeService
        /* memory contained in the reponse objects is owned by the caller */
        free(pResponse);
    }

    return status;
}

/*============================================================================
 * Asynchronously calls the ModifyMonitoredItems service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginModifyMonitoredItems(
    UA_Channel                              a_hChannel,
    const OpcUa_RequestHeader*              a_pRequestHeader,
    uint32_t                                a_nSubscriptionId,
    OpcUa_TimestampsToReturn                a_eTimestampsToReturn,
    int32_t                                 a_nNoOfItemsToModify,
    const OpcUa_MonitoredItemModifyRequest* a_pItemsToModify,
    UA_Channel_PfnRequestComplete*          a_pCallback,
    void*                                   a_pCallbackData)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_ModifyMonitoredItemsRequest cRequest;
    OpcUa_ModifyMonitoredItemsRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  (a_pItemsToModify != NULL || a_nNoOfItemsToModify <= 0))
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader      = *a_pRequestHeader;
        cRequest.SubscriptionId     = a_nSubscriptionId;
        cRequest.TimestampsToReturn = a_eTimestampsToReturn;
        cRequest.NoOfItemsToModify  = a_nNoOfItemsToModify;
        cRequest.ItemsToModify      = (OpcUa_MonitoredItemModifyRequest*)a_pItemsToModify;

        /* begin invoke service */
        status = UA_Channel_BeginInvokeService(
            a_hChannel,
            "ModifyMonitoredItems",
            (void*)&cRequest,
            &OpcUa_ModifyMonitoredItemsRequest_EncodeableType,
            &OpcUa_ModifyMonitoredItemsResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*)a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_SetMonitoringMode
/*============================================================================
 * Synchronously calls the SetMonitoringMode service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_SetMonitoringMode(
    UA_Channel                 a_hChannel,
    const OpcUa_RequestHeader* a_pRequestHeader,
    uint32_t                   a_nSubscriptionId,
    OpcUa_MonitoringMode       a_eMonitoringMode,
    int32_t                    a_nNoOfMonitoredItemIds,
    const uint32_t*            a_pMonitoredItemIds,
    OpcUa_ResponseHeader*      a_pResponseHeader,
    int32_t*                   a_pNoOfResults,
    SOPC_StatusCode**               a_pResults,
    int32_t*                   a_pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**        a_pDiagnosticInfos)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_SetMonitoringModeRequest cRequest;
    OpcUa_SetMonitoringModeResponse* pResponse = NULL;
    UA_EncodeableType* pResponseType = NULL;
    
    OpcUa_SetMonitoringModeRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  (a_pMonitoredItemIds != NULL || a_nNoOfMonitoredItemIds <= 0)
       &&  a_pResponseHeader != NULL
       &&  a_pNoOfResults != NULL
       &&  a_pResults != NULL
       &&  a_pNoOfDiagnosticInfos != NULL
       &&  a_pDiagnosticInfos != NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader        = *a_pRequestHeader;
        cRequest.SubscriptionId       = a_nSubscriptionId;
        cRequest.MonitoringMode       = a_eMonitoringMode;
        cRequest.NoOfMonitoredItemIds = a_nNoOfMonitoredItemIds;
        cRequest.MonitoredItemIds     = (uint32_t*)a_pMonitoredItemIds;

        /* invoke service */
        status = UA_Channel_InvokeService(
            a_hChannel,
            "SetMonitoringMode",
            (void*)&cRequest,
            &OpcUa_SetMonitoringModeRequest_EncodeableType,
            &OpcUa_SetMonitoringModeResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->TypeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((OpcUa_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (OpcUa_SetMonitoringModeResponse_EncodeableType.TypeId != pResponseType->TypeId)
        {
            pResponseType->Clear(pResponse);
            status = OpcUa_BadUnknownResponse;
        }

        /* copy parameters from response object into return parameters. */
        else
        {
            *a_pResponseHeader      = pResponse->ResponseHeader;
            *a_pNoOfResults         = pResponse->NoOfResults;
            *a_pResults             = pResponse->Results;
            *a_pNoOfDiagnosticInfos = pResponse->NoOfDiagnosticInfos;
            *a_pDiagnosticInfos     = pResponse->DiagnosticInfos;
        }
    }else{
        //TODO: check if status invalid response could have been already deallocated in InvokeService
        /* memory contained in the reponse objects is owned by the caller */
        free(pResponse);
    }

    return status;
}

/*============================================================================
 * Asynchronously calls the SetMonitoringMode service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginSetMonitoringMode(
    UA_Channel                     a_hChannel,
    const OpcUa_RequestHeader*     a_pRequestHeader,
    uint32_t                       a_nSubscriptionId,
    OpcUa_MonitoringMode           a_eMonitoringMode,
    int32_t                        a_nNoOfMonitoredItemIds,
    const uint32_t*                a_pMonitoredItemIds,
    UA_Channel_PfnRequestComplete* a_pCallback,
    void*                          a_pCallbackData)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_SetMonitoringModeRequest cRequest;
    OpcUa_SetMonitoringModeRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  (a_pMonitoredItemIds != NULL || a_nNoOfMonitoredItemIds <= 0))
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader        = *a_pRequestHeader;
        cRequest.SubscriptionId       = a_nSubscriptionId;
        cRequest.MonitoringMode       = a_eMonitoringMode;
        cRequest.NoOfMonitoredItemIds = a_nNoOfMonitoredItemIds;
        cRequest.MonitoredItemIds     = (uint32_t*)a_pMonitoredItemIds;

        /* begin invoke service */
        status = UA_Channel_BeginInvokeService(
            a_hChannel,
            "SetMonitoringMode",
            (void*)&cRequest,
            &OpcUa_SetMonitoringModeRequest_EncodeableType,
            &OpcUa_SetMonitoringModeResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*)a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_SetTriggering
/*============================================================================
 * Synchronously calls the SetTriggering service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_SetTriggering(
    UA_Channel                 a_hChannel,
    const OpcUa_RequestHeader* a_pRequestHeader,
    uint32_t                   a_nSubscriptionId,
    uint32_t                   a_nTriggeringItemId,
    int32_t                    a_nNoOfLinksToAdd,
    const uint32_t*            a_pLinksToAdd,
    int32_t                    a_nNoOfLinksToRemove,
    const uint32_t*            a_pLinksToRemove,
    OpcUa_ResponseHeader*      a_pResponseHeader,
    int32_t*                   a_pNoOfAddResults,
    SOPC_StatusCode**               a_pAddResults,
    int32_t*                   a_pNoOfAddDiagnosticInfos,
    UA_DiagnosticInfo**        a_pAddDiagnosticInfos,
    int32_t*                   a_pNoOfRemoveResults,
    SOPC_StatusCode**               a_pRemoveResults,
    int32_t*                   a_pNoOfRemoveDiagnosticInfos,
    UA_DiagnosticInfo**        a_pRemoveDiagnosticInfos)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_SetTriggeringRequest cRequest;
    OpcUa_SetTriggeringResponse* pResponse = NULL;
    UA_EncodeableType* pResponseType = NULL;
    
    OpcUa_SetTriggeringRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  (a_pLinksToAdd != NULL || a_nNoOfLinksToAdd <= 0)
       &&  (a_pLinksToRemove != NULL || a_nNoOfLinksToRemove <= 0)
       &&  a_pResponseHeader != NULL
       &&  a_pNoOfAddResults != NULL
       &&  a_pAddResults != NULL
       &&  a_pNoOfAddDiagnosticInfos != NULL
       &&  a_pAddDiagnosticInfos != NULL
       &&  a_pNoOfRemoveResults != NULL
       &&  a_pRemoveResults != NULL
       &&  a_pNoOfRemoveDiagnosticInfos != NULL
       &&  a_pRemoveDiagnosticInfos != NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader     = *a_pRequestHeader;
        cRequest.SubscriptionId    = a_nSubscriptionId;
        cRequest.TriggeringItemId  = a_nTriggeringItemId;
        cRequest.NoOfLinksToAdd    = a_nNoOfLinksToAdd;
        cRequest.LinksToAdd        = (uint32_t*)a_pLinksToAdd;
        cRequest.NoOfLinksToRemove = a_nNoOfLinksToRemove;
        cRequest.LinksToRemove     = (uint32_t*)a_pLinksToRemove;

        /* invoke service */
        status = UA_Channel_InvokeService(
            a_hChannel,
            "SetTriggering",
            (void*)&cRequest,
            &OpcUa_SetTriggeringRequest_EncodeableType,
            &OpcUa_SetTriggeringResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->TypeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((OpcUa_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (OpcUa_SetTriggeringResponse_EncodeableType.TypeId != pResponseType->TypeId)
        {
            pResponseType->Clear(pResponse);
            status = OpcUa_BadUnknownResponse;
        }

        /* copy parameters from response object into return parameters. */
        else
        {
            *a_pResponseHeader            = pResponse->ResponseHeader;
            *a_pNoOfAddResults            = pResponse->NoOfAddResults;
            *a_pAddResults                = pResponse->AddResults;
            *a_pNoOfAddDiagnosticInfos    = pResponse->NoOfAddDiagnosticInfos;
            *a_pAddDiagnosticInfos        = pResponse->AddDiagnosticInfos;
            *a_pNoOfRemoveResults         = pResponse->NoOfRemoveResults;
            *a_pRemoveResults             = pResponse->RemoveResults;
            *a_pNoOfRemoveDiagnosticInfos = pResponse->NoOfRemoveDiagnosticInfos;
            *a_pRemoveDiagnosticInfos     = pResponse->RemoveDiagnosticInfos;
        }
    }else{
        //TODO: check if status invalid response could have been already deallocated in InvokeService
        /* memory contained in the reponse objects is owned by the caller */
        free(pResponse);
    }

    return status;
}

/*============================================================================
 * Asynchronously calls the SetTriggering service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginSetTriggering(
    UA_Channel                     a_hChannel,
    const OpcUa_RequestHeader*     a_pRequestHeader,
    uint32_t                       a_nSubscriptionId,
    uint32_t                       a_nTriggeringItemId,
    int32_t                        a_nNoOfLinksToAdd,
    const uint32_t*                a_pLinksToAdd,
    int32_t                        a_nNoOfLinksToRemove,
    const uint32_t*                a_pLinksToRemove,
    UA_Channel_PfnRequestComplete* a_pCallback,
    void*                          a_pCallbackData)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_SetTriggeringRequest cRequest;
    OpcUa_SetTriggeringRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  (a_pLinksToAdd != NULL || a_nNoOfLinksToAdd <= 0)
       &&  (a_pLinksToRemove != NULL || a_nNoOfLinksToRemove <= 0))
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader     = *a_pRequestHeader;
        cRequest.SubscriptionId    = a_nSubscriptionId;
        cRequest.TriggeringItemId  = a_nTriggeringItemId;
        cRequest.NoOfLinksToAdd    = a_nNoOfLinksToAdd;
        cRequest.LinksToAdd        = (uint32_t*)a_pLinksToAdd;
        cRequest.NoOfLinksToRemove = a_nNoOfLinksToRemove;
        cRequest.LinksToRemove     = (uint32_t*)a_pLinksToRemove;

        /* begin invoke service */
        status = UA_Channel_BeginInvokeService(
            a_hChannel,
            "SetTriggering",
            (void*)&cRequest,
            &OpcUa_SetTriggeringRequest_EncodeableType,
            &OpcUa_SetTriggeringResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*)a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_DeleteMonitoredItems
/*============================================================================
 * Synchronously calls the DeleteMonitoredItems service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_DeleteMonitoredItems(
    UA_Channel                 a_hChannel,
    const OpcUa_RequestHeader* a_pRequestHeader,
    uint32_t                   a_nSubscriptionId,
    int32_t                    a_nNoOfMonitoredItemIds,
    const uint32_t*            a_pMonitoredItemIds,
    OpcUa_ResponseHeader*      a_pResponseHeader,
    int32_t*                   a_pNoOfResults,
    SOPC_StatusCode**               a_pResults,
    int32_t*                   a_pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**        a_pDiagnosticInfos)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_DeleteMonitoredItemsRequest cRequest;
    OpcUa_DeleteMonitoredItemsResponse* pResponse = NULL;
    UA_EncodeableType* pResponseType = NULL;
    
    OpcUa_DeleteMonitoredItemsRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  (a_pMonitoredItemIds != NULL || a_nNoOfMonitoredItemIds <= 0)
       &&  a_pResponseHeader != NULL
       &&  a_pNoOfResults != NULL
       &&  a_pResults != NULL
       &&  a_pNoOfDiagnosticInfos != NULL
       &&  a_pDiagnosticInfos != NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader        = *a_pRequestHeader;
        cRequest.SubscriptionId       = a_nSubscriptionId;
        cRequest.NoOfMonitoredItemIds = a_nNoOfMonitoredItemIds;
        cRequest.MonitoredItemIds     = (uint32_t*)a_pMonitoredItemIds;

        /* invoke service */
        status = UA_Channel_InvokeService(
            a_hChannel,
            "DeleteMonitoredItems",
            (void*)&cRequest,
            &OpcUa_DeleteMonitoredItemsRequest_EncodeableType,
            &OpcUa_DeleteMonitoredItemsResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->TypeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((OpcUa_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (OpcUa_DeleteMonitoredItemsResponse_EncodeableType.TypeId != pResponseType->TypeId)
        {
            pResponseType->Clear(pResponse);
            status = OpcUa_BadUnknownResponse;
        }

        /* copy parameters from response object into return parameters. */
        else
        {
            *a_pResponseHeader      = pResponse->ResponseHeader;
            *a_pNoOfResults         = pResponse->NoOfResults;
            *a_pResults             = pResponse->Results;
            *a_pNoOfDiagnosticInfos = pResponse->NoOfDiagnosticInfos;
            *a_pDiagnosticInfos     = pResponse->DiagnosticInfos;
        }
    }else{
        //TODO: check if status invalid response could have been already deallocated in InvokeService
        /* memory contained in the reponse objects is owned by the caller */
        free(pResponse);
    }

    return status;
}

/*============================================================================
 * Asynchronously calls the DeleteMonitoredItems service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginDeleteMonitoredItems(
    UA_Channel                     a_hChannel,
    const OpcUa_RequestHeader*     a_pRequestHeader,
    uint32_t                       a_nSubscriptionId,
    int32_t                        a_nNoOfMonitoredItemIds,
    const uint32_t*                a_pMonitoredItemIds,
    UA_Channel_PfnRequestComplete* a_pCallback,
    void*                          a_pCallbackData)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_DeleteMonitoredItemsRequest cRequest;
    OpcUa_DeleteMonitoredItemsRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  (a_pMonitoredItemIds != NULL || a_nNoOfMonitoredItemIds <= 0))
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader        = *a_pRequestHeader;
        cRequest.SubscriptionId       = a_nSubscriptionId;
        cRequest.NoOfMonitoredItemIds = a_nNoOfMonitoredItemIds;
        cRequest.MonitoredItemIds     = (uint32_t*)a_pMonitoredItemIds;

        /* begin invoke service */
        status = UA_Channel_BeginInvokeService(
            a_hChannel,
            "DeleteMonitoredItems",
            (void*)&cRequest,
            &OpcUa_DeleteMonitoredItemsRequest_EncodeableType,
            &OpcUa_DeleteMonitoredItemsResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*)a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_CreateSubscription
/*============================================================================
 * Synchronously calls the CreateSubscription service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_CreateSubscription(
    UA_Channel                 a_hChannel,
    const OpcUa_RequestHeader* a_pRequestHeader,
    double                     a_nRequestedPublishingInterval,
    uint32_t                   a_nRequestedLifetimeCount,
    uint32_t                   a_nRequestedMaxKeepAliveCount,
    uint32_t                   a_nMaxNotificationsPerPublish,
    UA_Boolean                 a_bPublishingEnabled,
    UA_Byte                    a_nPriority,
    OpcUa_ResponseHeader*      a_pResponseHeader,
    uint32_t*                  a_pSubscriptionId,
    double*                    a_pRevisedPublishingInterval,
    uint32_t*                  a_pRevisedLifetimeCount,
    uint32_t*                  a_pRevisedMaxKeepAliveCount)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_CreateSubscriptionRequest cRequest;
    OpcUa_CreateSubscriptionResponse* pResponse = NULL;
    UA_EncodeableType* pResponseType = NULL;
    
    OpcUa_CreateSubscriptionRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  a_pResponseHeader != NULL
       &&  a_pSubscriptionId != NULL
       &&  a_pRevisedPublishingInterval != NULL
       &&  a_pRevisedLifetimeCount != NULL
       &&  a_pRevisedMaxKeepAliveCount != NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader               = *a_pRequestHeader;
        cRequest.RequestedPublishingInterval = a_nRequestedPublishingInterval;
        cRequest.RequestedLifetimeCount      = a_nRequestedLifetimeCount;
        cRequest.RequestedMaxKeepAliveCount  = a_nRequestedMaxKeepAliveCount;
        cRequest.MaxNotificationsPerPublish  = a_nMaxNotificationsPerPublish;
        cRequest.PublishingEnabled           = a_bPublishingEnabled;
        cRequest.Priority                    = a_nPriority;

        /* invoke service */
        status = UA_Channel_InvokeService(
            a_hChannel,
            "CreateSubscription",
            (void*)&cRequest,
            &OpcUa_CreateSubscriptionRequest_EncodeableType,
            &OpcUa_CreateSubscriptionResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->TypeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((OpcUa_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (OpcUa_CreateSubscriptionResponse_EncodeableType.TypeId != pResponseType->TypeId)
        {
            pResponseType->Clear(pResponse);
            status = OpcUa_BadUnknownResponse;
        }

        /* copy parameters from response object into return parameters. */
        else
        {
            *a_pResponseHeader            = pResponse->ResponseHeader;
            *a_pSubscriptionId            = pResponse->SubscriptionId;
            *a_pRevisedPublishingInterval = pResponse->RevisedPublishingInterval;
            *a_pRevisedLifetimeCount      = pResponse->RevisedLifetimeCount;
            *a_pRevisedMaxKeepAliveCount  = pResponse->RevisedMaxKeepAliveCount;
        }
    }else{
        //TODO: check if status invalid response could have been already deallocated in InvokeService
        /* memory contained in the reponse objects is owned by the caller */
        free(pResponse);
    }

    return status;
}

/*============================================================================
 * Asynchronously calls the CreateSubscription service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginCreateSubscription(
    UA_Channel                     a_hChannel,
    const OpcUa_RequestHeader*     a_pRequestHeader,
    double                         a_nRequestedPublishingInterval,
    uint32_t                       a_nRequestedLifetimeCount,
    uint32_t                       a_nRequestedMaxKeepAliveCount,
    uint32_t                       a_nMaxNotificationsPerPublish,
    UA_Boolean                     a_bPublishingEnabled,
    UA_Byte                        a_nPriority,
    UA_Channel_PfnRequestComplete* a_pCallback,
    void*                          a_pCallbackData)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_CreateSubscriptionRequest cRequest;
    OpcUa_CreateSubscriptionRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader               = *a_pRequestHeader;
        cRequest.RequestedPublishingInterval = a_nRequestedPublishingInterval;
        cRequest.RequestedLifetimeCount      = a_nRequestedLifetimeCount;
        cRequest.RequestedMaxKeepAliveCount  = a_nRequestedMaxKeepAliveCount;
        cRequest.MaxNotificationsPerPublish  = a_nMaxNotificationsPerPublish;
        cRequest.PublishingEnabled           = a_bPublishingEnabled;
        cRequest.Priority                    = a_nPriority;

        /* begin invoke service */
        status = UA_Channel_BeginInvokeService(
            a_hChannel,
            "CreateSubscription",
            (void*)&cRequest,
            &OpcUa_CreateSubscriptionRequest_EncodeableType,
            &OpcUa_CreateSubscriptionResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*)a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_ModifySubscription
/*============================================================================
 * Synchronously calls the ModifySubscription service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_ModifySubscription(
    UA_Channel                 a_hChannel,
    const OpcUa_RequestHeader* a_pRequestHeader,
    uint32_t                   a_nSubscriptionId,
    double                     a_nRequestedPublishingInterval,
    uint32_t                   a_nRequestedLifetimeCount,
    uint32_t                   a_nRequestedMaxKeepAliveCount,
    uint32_t                   a_nMaxNotificationsPerPublish,
    UA_Byte                    a_nPriority,
    OpcUa_ResponseHeader*      a_pResponseHeader,
    double*                    a_pRevisedPublishingInterval,
    uint32_t*                  a_pRevisedLifetimeCount,
    uint32_t*                  a_pRevisedMaxKeepAliveCount)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_ModifySubscriptionRequest cRequest;
    OpcUa_ModifySubscriptionResponse* pResponse = NULL;
    UA_EncodeableType* pResponseType = NULL;
    
    OpcUa_ModifySubscriptionRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  a_pResponseHeader != NULL
       &&  a_pRevisedPublishingInterval != NULL
       &&  a_pRevisedLifetimeCount != NULL
       &&  a_pRevisedMaxKeepAliveCount != NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader               = *a_pRequestHeader;
        cRequest.SubscriptionId              = a_nSubscriptionId;
        cRequest.RequestedPublishingInterval = a_nRequestedPublishingInterval;
        cRequest.RequestedLifetimeCount      = a_nRequestedLifetimeCount;
        cRequest.RequestedMaxKeepAliveCount  = a_nRequestedMaxKeepAliveCount;
        cRequest.MaxNotificationsPerPublish  = a_nMaxNotificationsPerPublish;
        cRequest.Priority                    = a_nPriority;

        /* invoke service */
        status = UA_Channel_InvokeService(
            a_hChannel,
            "ModifySubscription",
            (void*)&cRequest,
            &OpcUa_ModifySubscriptionRequest_EncodeableType,
            &OpcUa_ModifySubscriptionResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->TypeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((OpcUa_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (OpcUa_ModifySubscriptionResponse_EncodeableType.TypeId != pResponseType->TypeId)
        {
            pResponseType->Clear(pResponse);
            status = OpcUa_BadUnknownResponse;
        }

        /* copy parameters from response object into return parameters. */
        else
        {
            *a_pResponseHeader            = pResponse->ResponseHeader;
            *a_pRevisedPublishingInterval = pResponse->RevisedPublishingInterval;
            *a_pRevisedLifetimeCount      = pResponse->RevisedLifetimeCount;
            *a_pRevisedMaxKeepAliveCount  = pResponse->RevisedMaxKeepAliveCount;
        }
    }else{
        //TODO: check if status invalid response could have been already deallocated in InvokeService
        /* memory contained in the reponse objects is owned by the caller */
        free(pResponse);
    }

    return status;
}

/*============================================================================
 * Asynchronously calls the ModifySubscription service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginModifySubscription(
    UA_Channel                     a_hChannel,
    const OpcUa_RequestHeader*     a_pRequestHeader,
    uint32_t                       a_nSubscriptionId,
    double                         a_nRequestedPublishingInterval,
    uint32_t                       a_nRequestedLifetimeCount,
    uint32_t                       a_nRequestedMaxKeepAliveCount,
    uint32_t                       a_nMaxNotificationsPerPublish,
    UA_Byte                        a_nPriority,
    UA_Channel_PfnRequestComplete* a_pCallback,
    void*                          a_pCallbackData)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_ModifySubscriptionRequest cRequest;
    OpcUa_ModifySubscriptionRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader               = *a_pRequestHeader;
        cRequest.SubscriptionId              = a_nSubscriptionId;
        cRequest.RequestedPublishingInterval = a_nRequestedPublishingInterval;
        cRequest.RequestedLifetimeCount      = a_nRequestedLifetimeCount;
        cRequest.RequestedMaxKeepAliveCount  = a_nRequestedMaxKeepAliveCount;
        cRequest.MaxNotificationsPerPublish  = a_nMaxNotificationsPerPublish;
        cRequest.Priority                    = a_nPriority;

        /* begin invoke service */
        status = UA_Channel_BeginInvokeService(
            a_hChannel,
            "ModifySubscription",
            (void*)&cRequest,
            &OpcUa_ModifySubscriptionRequest_EncodeableType,
            &OpcUa_ModifySubscriptionResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*)a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_SetPublishingMode
/*============================================================================
 * Synchronously calls the SetPublishingMode service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_SetPublishingMode(
    UA_Channel                 a_hChannel,
    const OpcUa_RequestHeader* a_pRequestHeader,
    UA_Boolean                 a_bPublishingEnabled,
    int32_t                    a_nNoOfSubscriptionIds,
    const uint32_t*            a_pSubscriptionIds,
    OpcUa_ResponseHeader*      a_pResponseHeader,
    int32_t*                   a_pNoOfResults,
    SOPC_StatusCode**               a_pResults,
    int32_t*                   a_pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**        a_pDiagnosticInfos)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_SetPublishingModeRequest cRequest;
    OpcUa_SetPublishingModeResponse* pResponse = NULL;
    UA_EncodeableType* pResponseType = NULL;
    
    OpcUa_SetPublishingModeRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  (a_pSubscriptionIds != NULL || a_nNoOfSubscriptionIds <= 0)
       &&  a_pResponseHeader != NULL
       &&  a_pNoOfResults != NULL
       &&  a_pResults != NULL
       &&  a_pNoOfDiagnosticInfos != NULL
       &&  a_pDiagnosticInfos != NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader       = *a_pRequestHeader;
        cRequest.PublishingEnabled   = a_bPublishingEnabled;
        cRequest.NoOfSubscriptionIds = a_nNoOfSubscriptionIds;
        cRequest.SubscriptionIds     = (uint32_t*)a_pSubscriptionIds;

        /* invoke service */
        status = UA_Channel_InvokeService(
            a_hChannel,
            "SetPublishingMode",
            (void*)&cRequest,
            &OpcUa_SetPublishingModeRequest_EncodeableType,
            &OpcUa_SetPublishingModeResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->TypeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((OpcUa_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (OpcUa_SetPublishingModeResponse_EncodeableType.TypeId != pResponseType->TypeId)
        {
            pResponseType->Clear(pResponse);
            status = OpcUa_BadUnknownResponse;
        }

        /* copy parameters from response object into return parameters. */
        else
        {
            *a_pResponseHeader      = pResponse->ResponseHeader;
            *a_pNoOfResults         = pResponse->NoOfResults;
            *a_pResults             = pResponse->Results;
            *a_pNoOfDiagnosticInfos = pResponse->NoOfDiagnosticInfos;
            *a_pDiagnosticInfos     = pResponse->DiagnosticInfos;
        }
    }else{
        //TODO: check if status invalid response could have been already deallocated in InvokeService
        /* memory contained in the reponse objects is owned by the caller */
        free(pResponse);
    }

    return status;
}

/*============================================================================
 * Asynchronously calls the SetPublishingMode service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginSetPublishingMode(
    UA_Channel                     a_hChannel,
    const OpcUa_RequestHeader*     a_pRequestHeader,
    UA_Boolean                     a_bPublishingEnabled,
    int32_t                        a_nNoOfSubscriptionIds,
    const uint32_t*                a_pSubscriptionIds,
    UA_Channel_PfnRequestComplete* a_pCallback,
    void*                          a_pCallbackData)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_SetPublishingModeRequest cRequest;
    OpcUa_SetPublishingModeRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  (a_pSubscriptionIds != NULL || a_nNoOfSubscriptionIds <= 0))
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader       = *a_pRequestHeader;
        cRequest.PublishingEnabled   = a_bPublishingEnabled;
        cRequest.NoOfSubscriptionIds = a_nNoOfSubscriptionIds;
        cRequest.SubscriptionIds     = (uint32_t*)a_pSubscriptionIds;

        /* begin invoke service */
        status = UA_Channel_BeginInvokeService(
            a_hChannel,
            "SetPublishingMode",
            (void*)&cRequest,
            &OpcUa_SetPublishingModeRequest_EncodeableType,
            &OpcUa_SetPublishingModeResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*)a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_Publish
/*============================================================================
 * Synchronously calls the Publish service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_Publish(
    UA_Channel                               a_hChannel,
    const OpcUa_RequestHeader*               a_pRequestHeader,
    int32_t                                  a_nNoOfSubscriptionAcknowledgements,
    const OpcUa_SubscriptionAcknowledgement* a_pSubscriptionAcknowledgements,
    OpcUa_ResponseHeader*                    a_pResponseHeader,
    uint32_t*                                a_pSubscriptionId,
    int32_t*                                 a_pNoOfAvailableSequenceNumbers,
    uint32_t**                               a_pAvailableSequenceNumbers,
    UA_Boolean*                              a_pMoreNotifications,
    OpcUa_NotificationMessage*               a_pNotificationMessage,
    int32_t*                                 a_pNoOfResults,
    SOPC_StatusCode**                             a_pResults,
    int32_t*                                 a_pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**                      a_pDiagnosticInfos)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_PublishRequest cRequest;
    OpcUa_PublishResponse* pResponse = NULL;
    UA_EncodeableType* pResponseType = NULL;
    
    OpcUa_PublishRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  (a_pSubscriptionAcknowledgements != NULL || a_nNoOfSubscriptionAcknowledgements <= 0)
       &&  a_pResponseHeader != NULL
       &&  a_pSubscriptionId != NULL
       &&  a_pNoOfAvailableSequenceNumbers != NULL
       &&  a_pAvailableSequenceNumbers != NULL
       &&  a_pMoreNotifications != NULL
       &&  a_pNotificationMessage != NULL
       &&  a_pNoOfResults != NULL
       &&  a_pResults != NULL
       &&  a_pNoOfDiagnosticInfos != NULL
       &&  a_pDiagnosticInfos != NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader                    = *a_pRequestHeader;
        cRequest.NoOfSubscriptionAcknowledgements = a_nNoOfSubscriptionAcknowledgements;
        cRequest.SubscriptionAcknowledgements     = (OpcUa_SubscriptionAcknowledgement*)a_pSubscriptionAcknowledgements;

        /* invoke service */
        status = UA_Channel_InvokeService(
            a_hChannel,
            "Publish",
            (void*)&cRequest,
            &OpcUa_PublishRequest_EncodeableType,
            &OpcUa_PublishResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->TypeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((OpcUa_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (OpcUa_PublishResponse_EncodeableType.TypeId != pResponseType->TypeId)
        {
            pResponseType->Clear(pResponse);
            status = OpcUa_BadUnknownResponse;
        }

        /* copy parameters from response object into return parameters. */
        else
        {
            *a_pResponseHeader               = pResponse->ResponseHeader;
            *a_pSubscriptionId               = pResponse->SubscriptionId;
            *a_pNoOfAvailableSequenceNumbers = pResponse->NoOfAvailableSequenceNumbers;
            *a_pAvailableSequenceNumbers     = pResponse->AvailableSequenceNumbers;
            *a_pMoreNotifications            = pResponse->MoreNotifications;
            *a_pNotificationMessage          = pResponse->NotificationMessage;
            *a_pNoOfResults                  = pResponse->NoOfResults;
            *a_pResults                      = pResponse->Results;
            *a_pNoOfDiagnosticInfos          = pResponse->NoOfDiagnosticInfos;
            *a_pDiagnosticInfos              = pResponse->DiagnosticInfos;
        }
    }else{
        //TODO: check if status invalid response could have been already deallocated in InvokeService
        /* memory contained in the reponse objects is owned by the caller */
        free(pResponse);
    }

    return status;
}

/*============================================================================
 * Asynchronously calls the Publish service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginPublish(
    UA_Channel                               a_hChannel,
    const OpcUa_RequestHeader*               a_pRequestHeader,
    int32_t                                  a_nNoOfSubscriptionAcknowledgements,
    const OpcUa_SubscriptionAcknowledgement* a_pSubscriptionAcknowledgements,
    UA_Channel_PfnRequestComplete*           a_pCallback,
    void*                                    a_pCallbackData)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_PublishRequest cRequest;
    OpcUa_PublishRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  (a_pSubscriptionAcknowledgements != NULL || a_nNoOfSubscriptionAcknowledgements <= 0))
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader                    = *a_pRequestHeader;
        cRequest.NoOfSubscriptionAcknowledgements = a_nNoOfSubscriptionAcknowledgements;
        cRequest.SubscriptionAcknowledgements     = (OpcUa_SubscriptionAcknowledgement*)a_pSubscriptionAcknowledgements;

        /* begin invoke service */
        status = UA_Channel_BeginInvokeService(
            a_hChannel,
            "Publish",
            (void*)&cRequest,
            &OpcUa_PublishRequest_EncodeableType,
            &OpcUa_PublishResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*)a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_Republish
/*============================================================================
 * Synchronously calls the Republish service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_Republish(
    UA_Channel                 a_hChannel,
    const OpcUa_RequestHeader* a_pRequestHeader,
    uint32_t                   a_nSubscriptionId,
    uint32_t                   a_nRetransmitSequenceNumber,
    OpcUa_ResponseHeader*      a_pResponseHeader,
    OpcUa_NotificationMessage* a_pNotificationMessage)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_RepublishRequest cRequest;
    OpcUa_RepublishResponse* pResponse = NULL;
    UA_EncodeableType* pResponseType = NULL;
    
    OpcUa_RepublishRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  a_pResponseHeader != NULL
       &&  a_pNotificationMessage != NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader            = *a_pRequestHeader;
        cRequest.SubscriptionId           = a_nSubscriptionId;
        cRequest.RetransmitSequenceNumber = a_nRetransmitSequenceNumber;

        /* invoke service */
        status = UA_Channel_InvokeService(
            a_hChannel,
            "Republish",
            (void*)&cRequest,
            &OpcUa_RepublishRequest_EncodeableType,
            &OpcUa_RepublishResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->TypeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((OpcUa_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (OpcUa_RepublishResponse_EncodeableType.TypeId != pResponseType->TypeId)
        {
            pResponseType->Clear(pResponse);
            status = OpcUa_BadUnknownResponse;
        }

        /* copy parameters from response object into return parameters. */
        else
        {
            *a_pResponseHeader      = pResponse->ResponseHeader;
            *a_pNotificationMessage = pResponse->NotificationMessage;
        }
    }else{
        //TODO: check if status invalid response could have been already deallocated in InvokeService
        /* memory contained in the reponse objects is owned by the caller */
        free(pResponse);
    }

    return status;
}

/*============================================================================
 * Asynchronously calls the Republish service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginRepublish(
    UA_Channel                     a_hChannel,
    const OpcUa_RequestHeader*     a_pRequestHeader,
    uint32_t                       a_nSubscriptionId,
    uint32_t                       a_nRetransmitSequenceNumber,
    UA_Channel_PfnRequestComplete* a_pCallback,
    void*                          a_pCallbackData)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_RepublishRequest cRequest;
    OpcUa_RepublishRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader            = *a_pRequestHeader;
        cRequest.SubscriptionId           = a_nSubscriptionId;
        cRequest.RetransmitSequenceNumber = a_nRetransmitSequenceNumber;

        /* begin invoke service */
        status = UA_Channel_BeginInvokeService(
            a_hChannel,
            "Republish",
            (void*)&cRequest,
            &OpcUa_RepublishRequest_EncodeableType,
            &OpcUa_RepublishResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*)a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_TransferSubscriptions
/*============================================================================
 * Synchronously calls the TransferSubscriptions service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_TransferSubscriptions(
    UA_Channel                 a_hChannel,
    const OpcUa_RequestHeader* a_pRequestHeader,
    int32_t                    a_nNoOfSubscriptionIds,
    const uint32_t*            a_pSubscriptionIds,
    UA_Boolean                 a_bSendInitialValues,
    OpcUa_ResponseHeader*      a_pResponseHeader,
    int32_t*                   a_pNoOfResults,
    OpcUa_TransferResult**     a_pResults,
    int32_t*                   a_pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**        a_pDiagnosticInfos)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_TransferSubscriptionsRequest cRequest;
    OpcUa_TransferSubscriptionsResponse* pResponse = NULL;
    UA_EncodeableType* pResponseType = NULL;
    
    OpcUa_TransferSubscriptionsRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  (a_pSubscriptionIds != NULL || a_nNoOfSubscriptionIds <= 0)
       &&  a_pResponseHeader != NULL
       &&  a_pNoOfResults != NULL
       &&  a_pResults != NULL
       &&  a_pNoOfDiagnosticInfos != NULL
       &&  a_pDiagnosticInfos != NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader       = *a_pRequestHeader;
        cRequest.NoOfSubscriptionIds = a_nNoOfSubscriptionIds;
        cRequest.SubscriptionIds     = (uint32_t*)a_pSubscriptionIds;
        cRequest.SendInitialValues   = a_bSendInitialValues;

        /* invoke service */
        status = UA_Channel_InvokeService(
            a_hChannel,
            "TransferSubscriptions",
            (void*)&cRequest,
            &OpcUa_TransferSubscriptionsRequest_EncodeableType,
            &OpcUa_TransferSubscriptionsResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->TypeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((OpcUa_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (OpcUa_TransferSubscriptionsResponse_EncodeableType.TypeId != pResponseType->TypeId)
        {
            pResponseType->Clear(pResponse);
            status = OpcUa_BadUnknownResponse;
        }

        /* copy parameters from response object into return parameters. */
        else
        {
            *a_pResponseHeader      = pResponse->ResponseHeader;
            *a_pNoOfResults         = pResponse->NoOfResults;
            *a_pResults             = pResponse->Results;
            *a_pNoOfDiagnosticInfos = pResponse->NoOfDiagnosticInfos;
            *a_pDiagnosticInfos     = pResponse->DiagnosticInfos;
        }
    }else{
        //TODO: check if status invalid response could have been already deallocated in InvokeService
        /* memory contained in the reponse objects is owned by the caller */
        free(pResponse);
    }

    return status;
}

/*============================================================================
 * Asynchronously calls the TransferSubscriptions service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginTransferSubscriptions(
    UA_Channel                     a_hChannel,
    const OpcUa_RequestHeader*     a_pRequestHeader,
    int32_t                        a_nNoOfSubscriptionIds,
    const uint32_t*                a_pSubscriptionIds,
    UA_Boolean                     a_bSendInitialValues,
    UA_Channel_PfnRequestComplete* a_pCallback,
    void*                          a_pCallbackData)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_TransferSubscriptionsRequest cRequest;
    OpcUa_TransferSubscriptionsRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  (a_pSubscriptionIds != NULL || a_nNoOfSubscriptionIds <= 0))
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader       = *a_pRequestHeader;
        cRequest.NoOfSubscriptionIds = a_nNoOfSubscriptionIds;
        cRequest.SubscriptionIds     = (uint32_t*)a_pSubscriptionIds;
        cRequest.SendInitialValues   = a_bSendInitialValues;

        /* begin invoke service */
        status = UA_Channel_BeginInvokeService(
            a_hChannel,
            "TransferSubscriptions",
            (void*)&cRequest,
            &OpcUa_TransferSubscriptionsRequest_EncodeableType,
            &OpcUa_TransferSubscriptionsResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*)a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_DeleteSubscriptions
/*============================================================================
 * Synchronously calls the DeleteSubscriptions service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_DeleteSubscriptions(
    UA_Channel                 a_hChannel,
    const OpcUa_RequestHeader* a_pRequestHeader,
    int32_t                    a_nNoOfSubscriptionIds,
    const uint32_t*            a_pSubscriptionIds,
    OpcUa_ResponseHeader*      a_pResponseHeader,
    int32_t*                   a_pNoOfResults,
    SOPC_StatusCode**               a_pResults,
    int32_t*                   a_pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**        a_pDiagnosticInfos)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_DeleteSubscriptionsRequest cRequest;
    OpcUa_DeleteSubscriptionsResponse* pResponse = NULL;
    UA_EncodeableType* pResponseType = NULL;
    
    OpcUa_DeleteSubscriptionsRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  (a_pSubscriptionIds != NULL || a_nNoOfSubscriptionIds <= 0)
       &&  a_pResponseHeader != NULL
       &&  a_pNoOfResults != NULL
       &&  a_pResults != NULL
       &&  a_pNoOfDiagnosticInfos != NULL
       &&  a_pDiagnosticInfos != NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader       = *a_pRequestHeader;
        cRequest.NoOfSubscriptionIds = a_nNoOfSubscriptionIds;
        cRequest.SubscriptionIds     = (uint32_t*)a_pSubscriptionIds;

        /* invoke service */
        status = UA_Channel_InvokeService(
            a_hChannel,
            "DeleteSubscriptions",
            (void*)&cRequest,
            &OpcUa_DeleteSubscriptionsRequest_EncodeableType,
            &OpcUa_DeleteSubscriptionsResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->TypeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((OpcUa_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (OpcUa_DeleteSubscriptionsResponse_EncodeableType.TypeId != pResponseType->TypeId)
        {
            pResponseType->Clear(pResponse);
            status = OpcUa_BadUnknownResponse;
        }

        /* copy parameters from response object into return parameters. */
        else
        {
            *a_pResponseHeader      = pResponse->ResponseHeader;
            *a_pNoOfResults         = pResponse->NoOfResults;
            *a_pResults             = pResponse->Results;
            *a_pNoOfDiagnosticInfos = pResponse->NoOfDiagnosticInfos;
            *a_pDiagnosticInfos     = pResponse->DiagnosticInfos;
        }
    }else{
        //TODO: check if status invalid response could have been already deallocated in InvokeService
        /* memory contained in the reponse objects is owned by the caller */
        free(pResponse);
    }

    return status;
}

/*============================================================================
 * Asynchronously calls the DeleteSubscriptions service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ClientApi_BeginDeleteSubscriptions(
    UA_Channel                     a_hChannel,
    const OpcUa_RequestHeader*     a_pRequestHeader,
    int32_t                        a_nNoOfSubscriptionIds,
    const uint32_t*                a_pSubscriptionIds,
    UA_Channel_PfnRequestComplete* a_pCallback,
    void*                          a_pCallbackData)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_DeleteSubscriptionsRequest cRequest;
    OpcUa_DeleteSubscriptionsRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != NULL
       &&  (a_pSubscriptionIds != NULL || a_nNoOfSubscriptionIds <= 0))
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader       = *a_pRequestHeader;
        cRequest.NoOfSubscriptionIds = a_nNoOfSubscriptionIds;
        cRequest.SubscriptionIds     = (uint32_t*)a_pSubscriptionIds;

        /* begin invoke service */
        status = UA_Channel_BeginInvokeService(
            a_hChannel,
            "DeleteSubscriptions",
            (void*)&cRequest,
            &OpcUa_DeleteSubscriptionsRequest_EncodeableType,
            &OpcUa_DeleteSubscriptionsResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*)a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#endif /* OPCUA_HAVE_CLIENTAPI */
/* This is the last line of an autogenerated file. */
