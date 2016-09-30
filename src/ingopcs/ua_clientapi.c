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

/* core */
#include <opcua_platformdefs.h>
#include <opcua_config.h>

#ifdef OPCUA_HAVE_CLIENTAPI

/* types */
#include <ua_builtintypes.h>

#include <opcua_identifiers.h>
#include <ua_clientapi.h>

#ifndef OPCUA_EXCLUDE_FindServers
/*============================================================================
 * Synchronously calls the FindServers service.
 *===========================================================================*/
StatusCode UA_ClientApi_FindServers(
    OpcUa_Channel               a_hChannel,
    const UA_RequestHeader*     a_pRequestHeader,
    const UA_String*            a_pEndpointUrl,
    int32_t                     a_nNoOfLocaleIds,
    const UA_String*            a_pLocaleIds,
    int32_t                     a_nNoOfServerUris,
    const UA_String*            a_pServerUris,
    UA_ResponseHeader*          a_pResponseHeader,
    int32_t*                    a_pNoOfServers,
    UA_ApplicationDescription** a_pServers)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_FindServersRequest cRequest;
    UA_FindServersResponse* pResponse = UA_NULL;
    UA_EncodeableType* pResponseType = UA_NULL;
    
    UA_FindServersRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  a_pEndpointUrl != UA_NULL
       &&  (a_pLocaleIds != UA_NULL || a_nNoOfLocaleIds <= 0)
       &&  (a_pServerUris != UA_NULL || a_nNoOfServerUris <= 0)
       &&  a_pResponseHeader != UA_NULL
       &&  a_pNoOfServers != UA_NULL
       &&  a_pServers != UA_NULL)
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
            &UA_FindServersRequest_EncodeableType,
            &UA_FindServersResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->typeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((UA_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (UA_FindServersResponse_EncodeableType.typeId != pResponseType->typeId)
        {
            pResponseType->clearFunction(pResponse);
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
StatusCode UA_ClientApi_BeginFindServers(
    OpcUa_Channel                     a_hChannel,
    const UA_RequestHeader*           a_pRequestHeader,
    const UA_String*                  a_pEndpointUrl,
    int32_t                           a_nNoOfLocaleIds,
    const UA_String*                  a_pLocaleIds,
    int32_t                           a_nNoOfServerUris,
    const UA_String*                  a_pServerUris,
    UA_Channel_PfnRequestComplete*    a_pCallback,
    OpcUa_Void*                       a_pCallbackData)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_FindServersRequest cRequest;
    UA_FindServersRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  a_pEndpointUrl != UA_NULL
       &&  (a_pLocaleIds != UA_NULL || a_nNoOfLocaleIds <= 0)
       &&  (a_pServerUris != UA_NULL || a_nNoOfServerUris <= 0))
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
            (OpcUa_Void*)&cRequest,
            &UA_FindServersRequest_EncodeableType,
            &UA_FindServersResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*   )a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_FindServersOnNetwork
/*============================================================================
 * Synchronously calls the FindServersOnNetwork service.
 *===========================================================================*/
StatusCode UA_ClientApi_FindServersOnNetwork(
    OpcUa_Channel           a_hChannel,
    const UA_RequestHeader* a_pRequestHeader,
    uint32_t                a_nStartingRecordId,
    uint32_t                a_nMaxRecordsToReturn,
    int32_t                 a_nNoOfServerCapabilityFilter,
    const UA_String*        a_pServerCapabilityFilter,
    UA_ResponseHeader*      a_pResponseHeader,
    UA_DateTime*            a_pLastCounterResetTime,
    int32_t*                a_pNoOfServers,
    UA_ServerOnNetwork**    a_pServers)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_FindServersOnNetworkRequest cRequest;
    UA_FindServersOnNetworkResponse* pResponse = UA_NULL;
    UA_EncodeableType* pResponseType = UA_NULL;
    
    UA_FindServersOnNetworkRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  (a_pServerCapabilityFilter != UA_NULL || a_nNoOfServerCapabilityFilter <= 0)
       &&  a_pResponseHeader != UA_NULL
       &&  a_pLastCounterResetTime != UA_NULL
       &&  a_pNoOfServers != UA_NULL
       &&  a_pServers != UA_NULL)
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
            &UA_FindServersOnNetworkRequest_EncodeableType,
            &UA_FindServersOnNetworkResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->typeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((UA_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (UA_FindServersOnNetworkResponse_EncodeableType.typeId != pResponseType->typeId)
        {
            pResponseType->clearFunction(pResponse);
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
StatusCode UA_ClientApi_BeginFindServersOnNetwork(
    OpcUa_Channel                     a_hChannel,
    const UA_RequestHeader*           a_pRequestHeader,
    uint32_t                          a_nStartingRecordId,
    uint32_t                          a_nMaxRecordsToReturn,
    int32_t                           a_nNoOfServerCapabilityFilter,
    const UA_String*                  a_pServerCapabilityFilter,
    UA_Channel_PfnRequestComplete*    a_pCallback,
    OpcUa_Void*                       a_pCallbackData)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_FindServersOnNetworkRequest cRequest;
    UA_FindServersOnNetworkRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  (a_pServerCapabilityFilter != UA_NULL || a_nNoOfServerCapabilityFilter <= 0))
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
            (OpcUa_Void*)&cRequest,
            &UA_FindServersOnNetworkRequest_EncodeableType,
            &UA_FindServersOnNetworkResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*   )a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_GetEndpoints
/*============================================================================
 * Synchronously calls the GetEndpoints service.
 *===========================================================================*/
StatusCode UA_ClientApi_GetEndpoints(
    OpcUa_Channel            a_hChannel,
    const UA_RequestHeader*  a_pRequestHeader,
    const UA_String*         a_pEndpointUrl,
    int32_t                  a_nNoOfLocaleIds,
    const UA_String*         a_pLocaleIds,
    int32_t                  a_nNoOfProfileUris,
    const UA_String*         a_pProfileUris,
    UA_ResponseHeader*       a_pResponseHeader,
    int32_t*                 a_pNoOfEndpoints,
    UA_EndpointDescription** a_pEndpoints)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_GetEndpointsRequest cRequest;
    UA_GetEndpointsResponse* pResponse = UA_NULL;
    UA_EncodeableType* pResponseType = UA_NULL;
    
    UA_GetEndpointsRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  a_pEndpointUrl != UA_NULL
       &&  (a_pLocaleIds != UA_NULL || a_nNoOfLocaleIds <= 0)
       &&  (a_pProfileUris != UA_NULL || a_nNoOfProfileUris <= 0)
       &&  a_pResponseHeader != UA_NULL
       &&  a_pNoOfEndpoints != UA_NULL
       &&  a_pEndpoints != UA_NULL)
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
            &UA_GetEndpointsRequest_EncodeableType,
            &UA_GetEndpointsResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->typeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((UA_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (UA_GetEndpointsResponse_EncodeableType.typeId != pResponseType->typeId)
        {
            pResponseType->clearFunction(pResponse);
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
StatusCode UA_ClientApi_BeginGetEndpoints(
    OpcUa_Channel                     a_hChannel,
    const UA_RequestHeader*           a_pRequestHeader,
    const UA_String*                  a_pEndpointUrl,
    int32_t                           a_nNoOfLocaleIds,
    const UA_String*                  a_pLocaleIds,
    int32_t                           a_nNoOfProfileUris,
    const UA_String*                  a_pProfileUris,
    UA_Channel_PfnRequestComplete*    a_pCallback,
    OpcUa_Void*                       a_pCallbackData)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_GetEndpointsRequest cRequest;
    UA_GetEndpointsRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  a_pEndpointUrl != UA_NULL
       &&  (a_pLocaleIds != UA_NULL || a_nNoOfLocaleIds <= 0)
       &&  (a_pProfileUris != UA_NULL || a_nNoOfProfileUris <= 0))
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
            (OpcUa_Void*)&cRequest,
            &UA_GetEndpointsRequest_EncodeableType,
            &UA_GetEndpointsResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*   )a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_RegisterServer
/*============================================================================
 * Synchronously calls the RegisterServer service.
 *===========================================================================*/
StatusCode UA_ClientApi_RegisterServer(
    OpcUa_Channel              a_hChannel,
    const UA_RequestHeader*    a_pRequestHeader,
    const UA_RegisteredServer* a_pServer,
    UA_ResponseHeader*         a_pResponseHeader)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_RegisterServerRequest cRequest;
    UA_RegisterServerResponse* pResponse = UA_NULL;
    UA_EncodeableType* pResponseType = UA_NULL;
    
    UA_RegisterServerRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  a_pServer != UA_NULL
       &&  a_pResponseHeader != UA_NULL)
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
            &UA_RegisterServerRequest_EncodeableType,
            &UA_RegisterServerResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->typeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((UA_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (UA_RegisterServerResponse_EncodeableType.typeId != pResponseType->typeId)
        {
            pResponseType->clearFunction(pResponse);
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
StatusCode UA_ClientApi_BeginRegisterServer(
    OpcUa_Channel                     a_hChannel,
    const UA_RequestHeader*           a_pRequestHeader,
    const UA_RegisteredServer*        a_pServer,
    UA_Channel_PfnRequestComplete*    a_pCallback,
    OpcUa_Void*                       a_pCallbackData)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_RegisterServerRequest cRequest;
    UA_RegisterServerRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  a_pServer != UA_NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader = *a_pRequestHeader;
        cRequest.Server        = *a_pServer;

        /* begin invoke service */
        status = UA_Channel_BeginInvokeService(
            a_hChannel,
            "RegisterServer",
            (OpcUa_Void*)&cRequest,
            &UA_RegisterServerRequest_EncodeableType,
            &UA_RegisterServerResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*   )a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_RegisterServer2
/*============================================================================
 * Synchronously calls the RegisterServer2 service.
 *===========================================================================*/
StatusCode UA_ClientApi_RegisterServer2(
    OpcUa_Channel              a_hChannel,
    const UA_RequestHeader*    a_pRequestHeader,
    const UA_RegisteredServer* a_pServer,
    int32_t                    a_nNoOfDiscoveryConfiguration,
    const UA_ExtensionObject*  a_pDiscoveryConfiguration,
    UA_ResponseHeader*         a_pResponseHeader,
    int32_t*                   a_pNoOfConfigurationResults,
    StatusCode**               a_pConfigurationResults,
    int32_t*                   a_pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**        a_pDiagnosticInfos)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_RegisterServer2Request cRequest;
    UA_RegisterServer2Response* pResponse = UA_NULL;
    UA_EncodeableType* pResponseType = UA_NULL;
    
    UA_RegisterServer2Request_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  a_pServer != UA_NULL
       &&  (a_pDiscoveryConfiguration != UA_NULL || a_nNoOfDiscoveryConfiguration <= 0)
       &&  a_pResponseHeader != UA_NULL
       &&  a_pNoOfConfigurationResults != UA_NULL
       &&  a_pConfigurationResults != UA_NULL
       &&  a_pNoOfDiagnosticInfos != UA_NULL
       &&  a_pDiagnosticInfos != UA_NULL)
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
            &UA_RegisterServer2Request_EncodeableType,
            &UA_RegisterServer2Response_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->typeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((UA_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (UA_RegisterServer2Response_EncodeableType.typeId != pResponseType->typeId)
        {
            pResponseType->clearFunction(pResponse);
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
StatusCode UA_ClientApi_BeginRegisterServer2(
    OpcUa_Channel                     a_hChannel,
    const UA_RequestHeader*           a_pRequestHeader,
    const UA_RegisteredServer*        a_pServer,
    int32_t                           a_nNoOfDiscoveryConfiguration,
    const UA_ExtensionObject*         a_pDiscoveryConfiguration,
    UA_Channel_PfnRequestComplete*    a_pCallback,
    OpcUa_Void*                       a_pCallbackData)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_RegisterServer2Request cRequest;
    UA_RegisterServer2Request_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  a_pServer != UA_NULL
       &&  (a_pDiscoveryConfiguration != UA_NULL || a_nNoOfDiscoveryConfiguration <= 0))
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
            (OpcUa_Void*)&cRequest,
            &UA_RegisterServer2Request_EncodeableType,
            &UA_RegisterServer2Response_EncodeableType,
            (UA_Channel_PfnRequestComplete*   )a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_CreateSession
/*============================================================================
 * Synchronously calls the CreateSession service.
 *===========================================================================*/
StatusCode UA_ClientApi_CreateSession(
    OpcUa_Channel                    a_hChannel,
    const UA_RequestHeader*          a_pRequestHeader,
    const UA_ApplicationDescription* a_pClientDescription,
    const UA_String*                 a_pServerUri,
    const UA_String*                 a_pEndpointUrl,
    const UA_String*                 a_pSessionName,
    const UA_ByteString*             a_pClientNonce,
    const UA_ByteString*             a_pClientCertificate,
    double                           a_nRequestedSessionTimeout,
    uint32_t                         a_nMaxResponseMessageSize,
    UA_ResponseHeader*               a_pResponseHeader,
    UA_NodeId*                       a_pSessionId,
    UA_NodeId*                       a_pAuthenticationToken,
    double*                          a_pRevisedSessionTimeout,
    UA_ByteString*                   a_pServerNonce,
    UA_ByteString*                   a_pServerCertificate,
    int32_t*                         a_pNoOfServerEndpoints,
    UA_EndpointDescription**         a_pServerEndpoints,
    int32_t*                         a_pNoOfServerSoftwareCertificates,
    UA_SignedSoftwareCertificate**   a_pServerSoftwareCertificates,
    UA_SignatureData*                a_pServerSignature,
    uint32_t*                        a_pMaxRequestMessageSize)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_CreateSessionRequest cRequest;
    UA_CreateSessionResponse* pResponse = UA_NULL;
    UA_EncodeableType* pResponseType = UA_NULL;
    
    UA_CreateSessionRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  a_pClientDescription != UA_NULL
       &&  a_pServerUri != UA_NULL
       &&  a_pEndpointUrl != UA_NULL
       &&  a_pSessionName != UA_NULL
       &&  a_pClientNonce != UA_NULL
       &&  a_pClientCertificate != UA_NULL
       &&  a_pResponseHeader != UA_NULL
       &&  a_pSessionId != UA_NULL
       &&  a_pAuthenticationToken != UA_NULL
       &&  a_pRevisedSessionTimeout != UA_NULL
       &&  a_pServerNonce != UA_NULL
       &&  a_pServerCertificate != UA_NULL
       &&  a_pNoOfServerEndpoints != UA_NULL
       &&  a_pServerEndpoints != UA_NULL
       &&  a_pNoOfServerSoftwareCertificates != UA_NULL
       &&  a_pServerSoftwareCertificates != UA_NULL
       &&  a_pServerSignature != UA_NULL
       &&  a_pMaxRequestMessageSize != UA_NULL)
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
            &UA_CreateSessionRequest_EncodeableType,
            &UA_CreateSessionResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->typeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((UA_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (UA_CreateSessionResponse_EncodeableType.typeId != pResponseType->typeId)
        {
            pResponseType->clearFunction(pResponse);
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
StatusCode UA_ClientApi_BeginCreateSession(
    OpcUa_Channel                     a_hChannel,
    const UA_RequestHeader*           a_pRequestHeader,
    const UA_ApplicationDescription*  a_pClientDescription,
    const UA_String*                  a_pServerUri,
    const UA_String*                  a_pEndpointUrl,
    const UA_String*                  a_pSessionName,
    const UA_ByteString*              a_pClientNonce,
    const UA_ByteString*              a_pClientCertificate,
    double                            a_nRequestedSessionTimeout,
    uint32_t                          a_nMaxResponseMessageSize,
    UA_Channel_PfnRequestComplete*    a_pCallback,
    OpcUa_Void*                       a_pCallbackData)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_CreateSessionRequest cRequest;
    UA_CreateSessionRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  a_pClientDescription != UA_NULL
       &&  a_pServerUri != UA_NULL
       &&  a_pEndpointUrl != UA_NULL
       &&  a_pSessionName != UA_NULL
       &&  a_pClientNonce != UA_NULL
       &&  a_pClientCertificate != UA_NULL)
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
            (OpcUa_Void*)&cRequest,
            &UA_CreateSessionRequest_EncodeableType,
            &UA_CreateSessionResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*   )a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_ActivateSession
/*============================================================================
 * Synchronously calls the ActivateSession service.
 *===========================================================================*/
StatusCode UA_ClientApi_ActivateSession(
    OpcUa_Channel                       a_hChannel,
    const UA_RequestHeader*             a_pRequestHeader,
    const UA_SignatureData*             a_pClientSignature,
    int32_t                             a_nNoOfClientSoftwareCertificates,
    const UA_SignedSoftwareCertificate* a_pClientSoftwareCertificates,
    int32_t                             a_nNoOfLocaleIds,
    const UA_String*                    a_pLocaleIds,
    const UA_ExtensionObject*           a_pUserIdentityToken,
    const UA_SignatureData*             a_pUserTokenSignature,
    UA_ResponseHeader*                  a_pResponseHeader,
    UA_ByteString*                      a_pServerNonce,
    int32_t*                            a_pNoOfResults,
    StatusCode**                        a_pResults,
    int32_t*                            a_pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**                 a_pDiagnosticInfos)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_ActivateSessionRequest cRequest;
    UA_ActivateSessionResponse* pResponse = UA_NULL;
    UA_EncodeableType* pResponseType = UA_NULL;
    
    UA_ActivateSessionRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  a_pClientSignature != UA_NULL
       &&  (a_pClientSoftwareCertificates != UA_NULL || a_nNoOfClientSoftwareCertificates <= 0)
       &&  (a_pLocaleIds != UA_NULL || a_nNoOfLocaleIds <= 0)
       &&  a_pUserIdentityToken != UA_NULL
       &&  a_pUserTokenSignature != UA_NULL
       &&  a_pResponseHeader != UA_NULL
       &&  a_pServerNonce != UA_NULL
       &&  a_pNoOfResults != UA_NULL
       &&  a_pResults != UA_NULL
       &&  a_pNoOfDiagnosticInfos != UA_NULL
       &&  a_pDiagnosticInfos != UA_NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader                  = *a_pRequestHeader;
        cRequest.ClientSignature                = *a_pClientSignature;
        cRequest.NoOfClientSoftwareCertificates = a_nNoOfClientSoftwareCertificates;
        cRequest.ClientSoftwareCertificates     = (UA_SignedSoftwareCertificate*)a_pClientSoftwareCertificates;
        cRequest.NoOfLocaleIds                  = a_nNoOfLocaleIds;
        cRequest.LocaleIds                      = (UA_String*)a_pLocaleIds;
        cRequest.UserIdentityToken              = *a_pUserIdentityToken;
        cRequest.UserTokenSignature             = *a_pUserTokenSignature;

        /* invoke service */
        status = UA_Channel_InvokeService(
            a_hChannel,
            "ActivateSession",
            (void*)&cRequest,
            &UA_ActivateSessionRequest_EncodeableType,
            &UA_ActivateSessionResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->typeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((UA_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (UA_ActivateSessionResponse_EncodeableType.typeId != pResponseType->typeId)
        {
            pResponseType->clearFunction(pResponse);
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
StatusCode UA_ClientApi_BeginActivateSession(
    OpcUa_Channel                       a_hChannel,
    const UA_RequestHeader*             a_pRequestHeader,
    const UA_SignatureData*             a_pClientSignature,
    int32_t                             a_nNoOfClientSoftwareCertificates,
    const UA_SignedSoftwareCertificate* a_pClientSoftwareCertificates,
    int32_t                             a_nNoOfLocaleIds,
    const UA_String*                    a_pLocaleIds,
    const UA_ExtensionObject*           a_pUserIdentityToken,
    const UA_SignatureData*             a_pUserTokenSignature,
    UA_Channel_PfnRequestComplete*      a_pCallback,
    OpcUa_Void*                         a_pCallbackData)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_ActivateSessionRequest cRequest;
    UA_ActivateSessionRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  a_pClientSignature != UA_NULL
       &&  (a_pClientSoftwareCertificates != UA_NULL || a_nNoOfClientSoftwareCertificates <= 0)
       &&  (a_pLocaleIds != UA_NULL || a_nNoOfLocaleIds <= 0)
       &&  a_pUserIdentityToken != UA_NULL
       &&  a_pUserTokenSignature != UA_NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader                  = *a_pRequestHeader;
        cRequest.ClientSignature                = *a_pClientSignature;
        cRequest.NoOfClientSoftwareCertificates = a_nNoOfClientSoftwareCertificates;
        cRequest.ClientSoftwareCertificates     = (UA_SignedSoftwareCertificate*)a_pClientSoftwareCertificates;
        cRequest.NoOfLocaleIds                  = a_nNoOfLocaleIds;
        cRequest.LocaleIds                      = (UA_String*)a_pLocaleIds;
        cRequest.UserIdentityToken              = *a_pUserIdentityToken;
        cRequest.UserTokenSignature             = *a_pUserTokenSignature;

        /* begin invoke service */
        status = UA_Channel_BeginInvokeService(
            a_hChannel,
            "ActivateSession",
            (OpcUa_Void*)&cRequest,
            &UA_ActivateSessionRequest_EncodeableType,
            &UA_ActivateSessionResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*   )a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_CloseSession
/*============================================================================
 * Synchronously calls the CloseSession service.
 *===========================================================================*/
StatusCode UA_ClientApi_CloseSession(
    OpcUa_Channel           a_hChannel,
    const UA_RequestHeader* a_pRequestHeader,
    UA_Boolean              a_bDeleteSubscriptions,
    UA_ResponseHeader*      a_pResponseHeader)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_CloseSessionRequest cRequest;
    UA_CloseSessionResponse* pResponse = UA_NULL;
    UA_EncodeableType* pResponseType = UA_NULL;
    
    UA_CloseSessionRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  a_pResponseHeader != UA_NULL)
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
            &UA_CloseSessionRequest_EncodeableType,
            &UA_CloseSessionResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->typeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((UA_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (UA_CloseSessionResponse_EncodeableType.typeId != pResponseType->typeId)
        {
            pResponseType->clearFunction(pResponse);
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
StatusCode UA_ClientApi_BeginCloseSession(
    OpcUa_Channel                     a_hChannel,
    const UA_RequestHeader*           a_pRequestHeader,
    UA_Boolean                        a_bDeleteSubscriptions,
    UA_Channel_PfnRequestComplete*    a_pCallback,
    OpcUa_Void*                       a_pCallbackData)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_CloseSessionRequest cRequest;
    UA_CloseSessionRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader       = *a_pRequestHeader;
        cRequest.DeleteSubscriptions = a_bDeleteSubscriptions;

        /* begin invoke service */
        status = UA_Channel_BeginInvokeService(
            a_hChannel,
            "CloseSession",
            (OpcUa_Void*)&cRequest,
            &UA_CloseSessionRequest_EncodeableType,
            &UA_CloseSessionResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*   )a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_Cancel
/*============================================================================
 * Synchronously calls the Cancel service.
 *===========================================================================*/
StatusCode UA_ClientApi_Cancel(
    OpcUa_Channel           a_hChannel,
    const UA_RequestHeader* a_pRequestHeader,
    uint32_t                a_nRequestHandle,
    UA_ResponseHeader*      a_pResponseHeader,
    uint32_t*               a_pCancelCount)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_CancelRequest cRequest;
    UA_CancelResponse* pResponse = UA_NULL;
    UA_EncodeableType* pResponseType = UA_NULL;
    
    UA_CancelRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  a_pResponseHeader != UA_NULL
       &&  a_pCancelCount != UA_NULL)
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
            &UA_CancelRequest_EncodeableType,
            &UA_CancelResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->typeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((UA_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (UA_CancelResponse_EncodeableType.typeId != pResponseType->typeId)
        {
            pResponseType->clearFunction(pResponse);
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
StatusCode UA_ClientApi_BeginCancel(
    OpcUa_Channel                     a_hChannel,
    const UA_RequestHeader*           a_pRequestHeader,
    uint32_t                          a_nRequestHandle,
    UA_Channel_PfnRequestComplete*    a_pCallback,
    OpcUa_Void*                       a_pCallbackData)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_CancelRequest cRequest;
    UA_CancelRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader = *a_pRequestHeader;
        cRequest.RequestHandle = a_nRequestHandle;

        /* begin invoke service */
        status = UA_Channel_BeginInvokeService(
            a_hChannel,
            "Cancel",
            (OpcUa_Void*)&cRequest,
            &UA_CancelRequest_EncodeableType,
            &UA_CancelResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*   )a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_AddNodes
/*============================================================================
 * Synchronously calls the AddNodes service.
 *===========================================================================*/
StatusCode UA_ClientApi_AddNodes(
    OpcUa_Channel           a_hChannel,
    const UA_RequestHeader* a_pRequestHeader,
    int32_t                 a_nNoOfNodesToAdd,
    const UA_AddNodesItem*  a_pNodesToAdd,
    UA_ResponseHeader*      a_pResponseHeader,
    int32_t*                a_pNoOfResults,
    UA_AddNodesResult**     a_pResults,
    int32_t*                a_pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**     a_pDiagnosticInfos)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_AddNodesRequest cRequest;
    UA_AddNodesResponse* pResponse = UA_NULL;
    UA_EncodeableType* pResponseType = UA_NULL;
    
    UA_AddNodesRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  (a_pNodesToAdd != UA_NULL || a_nNoOfNodesToAdd <= 0)
       &&  a_pResponseHeader != UA_NULL
       &&  a_pNoOfResults != UA_NULL
       &&  a_pResults != UA_NULL
       &&  a_pNoOfDiagnosticInfos != UA_NULL
       &&  a_pDiagnosticInfos != UA_NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader  = *a_pRequestHeader;
        cRequest.NoOfNodesToAdd = a_nNoOfNodesToAdd;
        cRequest.NodesToAdd     = (UA_AddNodesItem*)a_pNodesToAdd;

        /* invoke service */
        status = UA_Channel_InvokeService(
            a_hChannel,
            "AddNodes",
            (void*)&cRequest,
            &UA_AddNodesRequest_EncodeableType,
            &UA_AddNodesResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->typeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((UA_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (UA_AddNodesResponse_EncodeableType.typeId != pResponseType->typeId)
        {
            pResponseType->clearFunction(pResponse);
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
StatusCode UA_ClientApi_BeginAddNodes(
    OpcUa_Channel                     a_hChannel,
    const UA_RequestHeader*           a_pRequestHeader,
    int32_t                           a_nNoOfNodesToAdd,
    const UA_AddNodesItem*            a_pNodesToAdd,
    UA_Channel_PfnRequestComplete*    a_pCallback,
    OpcUa_Void*                       a_pCallbackData)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_AddNodesRequest cRequest;
    UA_AddNodesRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  (a_pNodesToAdd != UA_NULL || a_nNoOfNodesToAdd <= 0))
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader  = *a_pRequestHeader;
        cRequest.NoOfNodesToAdd = a_nNoOfNodesToAdd;
        cRequest.NodesToAdd     = (UA_AddNodesItem*)a_pNodesToAdd;

        /* begin invoke service */
        status = UA_Channel_BeginInvokeService(
            a_hChannel,
            "AddNodes",
            (OpcUa_Void*)&cRequest,
            &UA_AddNodesRequest_EncodeableType,
            &UA_AddNodesResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*   )a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_AddReferences
/*============================================================================
 * Synchronously calls the AddReferences service.
 *===========================================================================*/
StatusCode UA_ClientApi_AddReferences(
    OpcUa_Channel               a_hChannel,
    const UA_RequestHeader*     a_pRequestHeader,
    int32_t                     a_nNoOfReferencesToAdd,
    const UA_AddReferencesItem* a_pReferencesToAdd,
    UA_ResponseHeader*          a_pResponseHeader,
    int32_t*                    a_pNoOfResults,
    StatusCode**                a_pResults,
    int32_t*                    a_pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**         a_pDiagnosticInfos)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_AddReferencesRequest cRequest;
    UA_AddReferencesResponse* pResponse = UA_NULL;
    UA_EncodeableType* pResponseType = UA_NULL;
    
    UA_AddReferencesRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  (a_pReferencesToAdd != UA_NULL || a_nNoOfReferencesToAdd <= 0)
       &&  a_pResponseHeader != UA_NULL
       &&  a_pNoOfResults != UA_NULL
       &&  a_pResults != UA_NULL
       &&  a_pNoOfDiagnosticInfos != UA_NULL
       &&  a_pDiagnosticInfos != UA_NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader       = *a_pRequestHeader;
        cRequest.NoOfReferencesToAdd = a_nNoOfReferencesToAdd;
        cRequest.ReferencesToAdd     = (UA_AddReferencesItem*)a_pReferencesToAdd;

        /* invoke service */
        status = UA_Channel_InvokeService(
            a_hChannel,
            "AddReferences",
            (void*)&cRequest,
            &UA_AddReferencesRequest_EncodeableType,
            &UA_AddReferencesResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->typeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((UA_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (UA_AddReferencesResponse_EncodeableType.typeId != pResponseType->typeId)
        {
            pResponseType->clearFunction(pResponse);
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
StatusCode UA_ClientApi_BeginAddReferences(
    OpcUa_Channel                     a_hChannel,
    const UA_RequestHeader*           a_pRequestHeader,
    int32_t                           a_nNoOfReferencesToAdd,
    const UA_AddReferencesItem*       a_pReferencesToAdd,
    UA_Channel_PfnRequestComplete*    a_pCallback,
    OpcUa_Void*                       a_pCallbackData)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_AddReferencesRequest cRequest;
    UA_AddReferencesRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  (a_pReferencesToAdd != UA_NULL || a_nNoOfReferencesToAdd <= 0))
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader       = *a_pRequestHeader;
        cRequest.NoOfReferencesToAdd = a_nNoOfReferencesToAdd;
        cRequest.ReferencesToAdd     = (UA_AddReferencesItem*)a_pReferencesToAdd;

        /* begin invoke service */
        status = UA_Channel_BeginInvokeService(
            a_hChannel,
            "AddReferences",
            (OpcUa_Void*)&cRequest,
            &UA_AddReferencesRequest_EncodeableType,
            &UA_AddReferencesResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*   )a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_DeleteNodes
/*============================================================================
 * Synchronously calls the DeleteNodes service.
 *===========================================================================*/
StatusCode UA_ClientApi_DeleteNodes(
    OpcUa_Channel             a_hChannel,
    const UA_RequestHeader*   a_pRequestHeader,
    int32_t                   a_nNoOfNodesToDelete,
    const UA_DeleteNodesItem* a_pNodesToDelete,
    UA_ResponseHeader*        a_pResponseHeader,
    int32_t*                  a_pNoOfResults,
    StatusCode**              a_pResults,
    int32_t*                  a_pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**       a_pDiagnosticInfos)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_DeleteNodesRequest cRequest;
    UA_DeleteNodesResponse* pResponse = UA_NULL;
    UA_EncodeableType* pResponseType = UA_NULL;
    
    UA_DeleteNodesRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  (a_pNodesToDelete != UA_NULL || a_nNoOfNodesToDelete <= 0)
       &&  a_pResponseHeader != UA_NULL
       &&  a_pNoOfResults != UA_NULL
       &&  a_pResults != UA_NULL
       &&  a_pNoOfDiagnosticInfos != UA_NULL
       &&  a_pDiagnosticInfos != UA_NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader     = *a_pRequestHeader;
        cRequest.NoOfNodesToDelete = a_nNoOfNodesToDelete;
        cRequest.NodesToDelete     = (UA_DeleteNodesItem*)a_pNodesToDelete;

        /* invoke service */
        status = UA_Channel_InvokeService(
            a_hChannel,
            "DeleteNodes",
            (void*)&cRequest,
            &UA_DeleteNodesRequest_EncodeableType,
            &UA_DeleteNodesResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->typeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((UA_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (UA_DeleteNodesResponse_EncodeableType.typeId != pResponseType->typeId)
        {
            pResponseType->clearFunction(pResponse);
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
StatusCode UA_ClientApi_BeginDeleteNodes(
    OpcUa_Channel                     a_hChannel,
    const UA_RequestHeader*           a_pRequestHeader,
    int32_t                           a_nNoOfNodesToDelete,
    const UA_DeleteNodesItem*         a_pNodesToDelete,
    UA_Channel_PfnRequestComplete*    a_pCallback,
    OpcUa_Void*                       a_pCallbackData)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_DeleteNodesRequest cRequest;
    UA_DeleteNodesRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  (a_pNodesToDelete != UA_NULL || a_nNoOfNodesToDelete <= 0))
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader     = *a_pRequestHeader;
        cRequest.NoOfNodesToDelete = a_nNoOfNodesToDelete;
        cRequest.NodesToDelete     = (UA_DeleteNodesItem*)a_pNodesToDelete;

        /* begin invoke service */
        status = UA_Channel_BeginInvokeService(
            a_hChannel,
            "DeleteNodes",
            (OpcUa_Void*)&cRequest,
            &UA_DeleteNodesRequest_EncodeableType,
            &UA_DeleteNodesResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*   )a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_DeleteReferences
/*============================================================================
 * Synchronously calls the DeleteReferences service.
 *===========================================================================*/
StatusCode UA_ClientApi_DeleteReferences(
    OpcUa_Channel                  a_hChannel,
    const UA_RequestHeader*        a_pRequestHeader,
    int32_t                        a_nNoOfReferencesToDelete,
    const UA_DeleteReferencesItem* a_pReferencesToDelete,
    UA_ResponseHeader*             a_pResponseHeader,
    int32_t*                       a_pNoOfResults,
    StatusCode**                   a_pResults,
    int32_t*                       a_pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**            a_pDiagnosticInfos)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_DeleteReferencesRequest cRequest;
    UA_DeleteReferencesResponse* pResponse = UA_NULL;
    UA_EncodeableType* pResponseType = UA_NULL;
    
    UA_DeleteReferencesRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  (a_pReferencesToDelete != UA_NULL || a_nNoOfReferencesToDelete <= 0)
       &&  a_pResponseHeader != UA_NULL
       &&  a_pNoOfResults != UA_NULL
       &&  a_pResults != UA_NULL
       &&  a_pNoOfDiagnosticInfos != UA_NULL
       &&  a_pDiagnosticInfos != UA_NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader          = *a_pRequestHeader;
        cRequest.NoOfReferencesToDelete = a_nNoOfReferencesToDelete;
        cRequest.ReferencesToDelete     = (UA_DeleteReferencesItem*)a_pReferencesToDelete;

        /* invoke service */
        status = UA_Channel_InvokeService(
            a_hChannel,
            "DeleteReferences",
            (void*)&cRequest,
            &UA_DeleteReferencesRequest_EncodeableType,
            &UA_DeleteReferencesResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->typeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((UA_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (UA_DeleteReferencesResponse_EncodeableType.typeId != pResponseType->typeId)
        {
            pResponseType->clearFunction(pResponse);
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
StatusCode UA_ClientApi_BeginDeleteReferences(
    OpcUa_Channel                     a_hChannel,
    const UA_RequestHeader*           a_pRequestHeader,
    int32_t                           a_nNoOfReferencesToDelete,
    const UA_DeleteReferencesItem*    a_pReferencesToDelete,
    UA_Channel_PfnRequestComplete*    a_pCallback,
    OpcUa_Void*                       a_pCallbackData)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_DeleteReferencesRequest cRequest;
    UA_DeleteReferencesRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  (a_pReferencesToDelete != UA_NULL || a_nNoOfReferencesToDelete <= 0))
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader          = *a_pRequestHeader;
        cRequest.NoOfReferencesToDelete = a_nNoOfReferencesToDelete;
        cRequest.ReferencesToDelete     = (UA_DeleteReferencesItem*)a_pReferencesToDelete;

        /* begin invoke service */
        status = UA_Channel_BeginInvokeService(
            a_hChannel,
            "DeleteReferences",
            (OpcUa_Void*)&cRequest,
            &UA_DeleteReferencesRequest_EncodeableType,
            &UA_DeleteReferencesResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*   )a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_Browse
/*============================================================================
 * Synchronously calls the Browse service.
 *===========================================================================*/
StatusCode UA_ClientApi_Browse(
    OpcUa_Channel               a_hChannel,
    const UA_RequestHeader*     a_pRequestHeader,
    const UA_ViewDescription*   a_pView,
    uint32_t                    a_nRequestedMaxReferencesPerNode,
    int32_t                     a_nNoOfNodesToBrowse,
    const UA_BrowseDescription* a_pNodesToBrowse,
    UA_ResponseHeader*          a_pResponseHeader,
    int32_t*                    a_pNoOfResults,
    UA_BrowseResult**           a_pResults,
    int32_t*                    a_pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**         a_pDiagnosticInfos)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_BrowseRequest cRequest;
    UA_BrowseResponse* pResponse = UA_NULL;
    UA_EncodeableType* pResponseType = UA_NULL;
    
    UA_BrowseRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  a_pView != UA_NULL
       &&  (a_pNodesToBrowse != UA_NULL || a_nNoOfNodesToBrowse <= 0)
       &&  a_pResponseHeader != UA_NULL
       &&  a_pNoOfResults != UA_NULL
       &&  a_pResults != UA_NULL
       &&  a_pNoOfDiagnosticInfos != UA_NULL
       &&  a_pDiagnosticInfos != UA_NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader                 = *a_pRequestHeader;
        cRequest.View                          = *a_pView;
        cRequest.RequestedMaxReferencesPerNode = a_nRequestedMaxReferencesPerNode;
        cRequest.NoOfNodesToBrowse             = a_nNoOfNodesToBrowse;
        cRequest.NodesToBrowse                 = (UA_BrowseDescription*)a_pNodesToBrowse;

        /* invoke service */
        status = UA_Channel_InvokeService(
            a_hChannel,
            "Browse",
            (void*)&cRequest,
            &UA_BrowseRequest_EncodeableType,
            &UA_BrowseResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->typeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((UA_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (UA_BrowseResponse_EncodeableType.typeId != pResponseType->typeId)
        {
            pResponseType->clearFunction(pResponse);
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
StatusCode UA_ClientApi_BeginBrowse(
    OpcUa_Channel                     a_hChannel,
    const UA_RequestHeader*           a_pRequestHeader,
    const UA_ViewDescription*         a_pView,
    uint32_t                          a_nRequestedMaxReferencesPerNode,
    int32_t                           a_nNoOfNodesToBrowse,
    const UA_BrowseDescription*       a_pNodesToBrowse,
    UA_Channel_PfnRequestComplete*    a_pCallback,
    OpcUa_Void*                       a_pCallbackData)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_BrowseRequest cRequest;
    UA_BrowseRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  a_pView != UA_NULL
       &&  (a_pNodesToBrowse != UA_NULL || a_nNoOfNodesToBrowse <= 0))
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader                 = *a_pRequestHeader;
        cRequest.View                          = *a_pView;
        cRequest.RequestedMaxReferencesPerNode = a_nRequestedMaxReferencesPerNode;
        cRequest.NoOfNodesToBrowse             = a_nNoOfNodesToBrowse;
        cRequest.NodesToBrowse                 = (UA_BrowseDescription*)a_pNodesToBrowse;

        /* begin invoke service */
        status = UA_Channel_BeginInvokeService(
            a_hChannel,
            "Browse",
            (OpcUa_Void*)&cRequest,
            &UA_BrowseRequest_EncodeableType,
            &UA_BrowseResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*   )a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_BrowseNext
/*============================================================================
 * Synchronously calls the BrowseNext service.
 *===========================================================================*/
StatusCode UA_ClientApi_BrowseNext(
    OpcUa_Channel           a_hChannel,
    const UA_RequestHeader* a_pRequestHeader,
    UA_Boolean              a_bReleaseContinuationPoints,
    int32_t                 a_nNoOfContinuationPoints,
    const UA_ByteString*    a_pContinuationPoints,
    UA_ResponseHeader*      a_pResponseHeader,
    int32_t*                a_pNoOfResults,
    UA_BrowseResult**       a_pResults,
    int32_t*                a_pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**     a_pDiagnosticInfos)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_BrowseNextRequest cRequest;
    UA_BrowseNextResponse* pResponse = UA_NULL;
    UA_EncodeableType* pResponseType = UA_NULL;
    
    UA_BrowseNextRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  (a_pContinuationPoints != UA_NULL || a_nNoOfContinuationPoints <= 0)
       &&  a_pResponseHeader != UA_NULL
       &&  a_pNoOfResults != UA_NULL
       &&  a_pResults != UA_NULL
       &&  a_pNoOfDiagnosticInfos != UA_NULL
       &&  a_pDiagnosticInfos != UA_NULL)
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
            &UA_BrowseNextRequest_EncodeableType,
            &UA_BrowseNextResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->typeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((UA_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (UA_BrowseNextResponse_EncodeableType.typeId != pResponseType->typeId)
        {
            pResponseType->clearFunction(pResponse);
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
StatusCode UA_ClientApi_BeginBrowseNext(
    OpcUa_Channel                     a_hChannel,
    const UA_RequestHeader*           a_pRequestHeader,
    UA_Boolean                        a_bReleaseContinuationPoints,
    int32_t                           a_nNoOfContinuationPoints,
    const UA_ByteString*              a_pContinuationPoints,
    UA_Channel_PfnRequestComplete*    a_pCallback,
    OpcUa_Void*                       a_pCallbackData)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_BrowseNextRequest cRequest;
    UA_BrowseNextRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  (a_pContinuationPoints != UA_NULL || a_nNoOfContinuationPoints <= 0))
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
            (OpcUa_Void*)&cRequest,
            &UA_BrowseNextRequest_EncodeableType,
            &UA_BrowseNextResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*   )a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_TranslateBrowsePathsToNodeIds
/*============================================================================
 * Synchronously calls the TranslateBrowsePathsToNodeIds service.
 *===========================================================================*/
StatusCode UA_ClientApi_TranslateBrowsePathsToNodeIds(
    OpcUa_Channel           a_hChannel,
    const UA_RequestHeader* a_pRequestHeader,
    int32_t                 a_nNoOfBrowsePaths,
    const UA_BrowsePath*    a_pBrowsePaths,
    UA_ResponseHeader*      a_pResponseHeader,
    int32_t*                a_pNoOfResults,
    UA_BrowsePathResult**   a_pResults,
    int32_t*                a_pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**     a_pDiagnosticInfos)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_TranslateBrowsePathsToNodeIdsRequest cRequest;
    UA_TranslateBrowsePathsToNodeIdsResponse* pResponse = UA_NULL;
    UA_EncodeableType* pResponseType = UA_NULL;
    
    UA_TranslateBrowsePathsToNodeIdsRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  (a_pBrowsePaths != UA_NULL || a_nNoOfBrowsePaths <= 0)
       &&  a_pResponseHeader != UA_NULL
       &&  a_pNoOfResults != UA_NULL
       &&  a_pResults != UA_NULL
       &&  a_pNoOfDiagnosticInfos != UA_NULL
       &&  a_pDiagnosticInfos != UA_NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader   = *a_pRequestHeader;
        cRequest.NoOfBrowsePaths = a_nNoOfBrowsePaths;
        cRequest.BrowsePaths     = (UA_BrowsePath*)a_pBrowsePaths;

        /* invoke service */
        status = UA_Channel_InvokeService(
            a_hChannel,
            "TranslateBrowsePathsToNodeIds",
            (void*)&cRequest,
            &UA_TranslateBrowsePathsToNodeIdsRequest_EncodeableType,
            &UA_TranslateBrowsePathsToNodeIdsResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->typeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((UA_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (UA_TranslateBrowsePathsToNodeIdsResponse_EncodeableType.typeId != pResponseType->typeId)
        {
            pResponseType->clearFunction(pResponse);
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
StatusCode UA_ClientApi_BeginTranslateBrowsePathsToNodeIds(
    OpcUa_Channel                     a_hChannel,
    const UA_RequestHeader*           a_pRequestHeader,
    int32_t                           a_nNoOfBrowsePaths,
    const UA_BrowsePath*              a_pBrowsePaths,
    UA_Channel_PfnRequestComplete*    a_pCallback,
    OpcUa_Void*                       a_pCallbackData)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_TranslateBrowsePathsToNodeIdsRequest cRequest;
    UA_TranslateBrowsePathsToNodeIdsRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  (a_pBrowsePaths != UA_NULL || a_nNoOfBrowsePaths <= 0))
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader   = *a_pRequestHeader;
        cRequest.NoOfBrowsePaths = a_nNoOfBrowsePaths;
        cRequest.BrowsePaths     = (UA_BrowsePath*)a_pBrowsePaths;

        /* begin invoke service */
        status = UA_Channel_BeginInvokeService(
            a_hChannel,
            "TranslateBrowsePathsToNodeIds",
            (OpcUa_Void*)&cRequest,
            &UA_TranslateBrowsePathsToNodeIdsRequest_EncodeableType,
            &UA_TranslateBrowsePathsToNodeIdsResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*   )a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_RegisterNodes
/*============================================================================
 * Synchronously calls the RegisterNodes service.
 *===========================================================================*/
StatusCode UA_ClientApi_RegisterNodes(
    OpcUa_Channel           a_hChannel,
    const UA_RequestHeader* a_pRequestHeader,
    int32_t                 a_nNoOfNodesToRegister,
    const UA_NodeId*        a_pNodesToRegister,
    UA_ResponseHeader*      a_pResponseHeader,
    int32_t*                a_pNoOfRegisteredNodeIds,
    UA_NodeId**             a_pRegisteredNodeIds)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_RegisterNodesRequest cRequest;
    UA_RegisterNodesResponse* pResponse = UA_NULL;
    UA_EncodeableType* pResponseType = UA_NULL;
    
    UA_RegisterNodesRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  (a_pNodesToRegister != UA_NULL || a_nNoOfNodesToRegister <= 0)
       &&  a_pResponseHeader != UA_NULL
       &&  a_pNoOfRegisteredNodeIds != UA_NULL
       &&  a_pRegisteredNodeIds != UA_NULL)
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
            &UA_RegisterNodesRequest_EncodeableType,
            &UA_RegisterNodesResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->typeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((UA_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (UA_RegisterNodesResponse_EncodeableType.typeId != pResponseType->typeId)
        {
            pResponseType->clearFunction(pResponse);
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
StatusCode UA_ClientApi_BeginRegisterNodes(
    OpcUa_Channel                     a_hChannel,
    const UA_RequestHeader*           a_pRequestHeader,
    int32_t                           a_nNoOfNodesToRegister,
    const UA_NodeId*                  a_pNodesToRegister,
    UA_Channel_PfnRequestComplete*    a_pCallback,
    OpcUa_Void*                       a_pCallbackData)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_RegisterNodesRequest cRequest;
    UA_RegisterNodesRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  (a_pNodesToRegister != UA_NULL || a_nNoOfNodesToRegister <= 0))
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
            (OpcUa_Void*)&cRequest,
            &UA_RegisterNodesRequest_EncodeableType,
            &UA_RegisterNodesResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*   )a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_UnregisterNodes
/*============================================================================
 * Synchronously calls the UnregisterNodes service.
 *===========================================================================*/
StatusCode UA_ClientApi_UnregisterNodes(
    OpcUa_Channel           a_hChannel,
    const UA_RequestHeader* a_pRequestHeader,
    int32_t                 a_nNoOfNodesToUnregister,
    const UA_NodeId*        a_pNodesToUnregister,
    UA_ResponseHeader*      a_pResponseHeader)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_UnregisterNodesRequest cRequest;
    UA_UnregisterNodesResponse* pResponse = UA_NULL;
    UA_EncodeableType* pResponseType = UA_NULL;
    
    UA_UnregisterNodesRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  (a_pNodesToUnregister != UA_NULL || a_nNoOfNodesToUnregister <= 0)
       &&  a_pResponseHeader != UA_NULL)
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
            &UA_UnregisterNodesRequest_EncodeableType,
            &UA_UnregisterNodesResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->typeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((UA_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (UA_UnregisterNodesResponse_EncodeableType.typeId != pResponseType->typeId)
        {
            pResponseType->clearFunction(pResponse);
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
StatusCode UA_ClientApi_BeginUnregisterNodes(
    OpcUa_Channel                     a_hChannel,
    const UA_RequestHeader*           a_pRequestHeader,
    int32_t                           a_nNoOfNodesToUnregister,
    const UA_NodeId*                  a_pNodesToUnregister,
    UA_Channel_PfnRequestComplete*    a_pCallback,
    OpcUa_Void*                       a_pCallbackData)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_UnregisterNodesRequest cRequest;
    UA_UnregisterNodesRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  (a_pNodesToUnregister != UA_NULL || a_nNoOfNodesToUnregister <= 0))
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
            (OpcUa_Void*)&cRequest,
            &UA_UnregisterNodesRequest_EncodeableType,
            &UA_UnregisterNodesResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*   )a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_QueryFirst
/*============================================================================
 * Synchronously calls the QueryFirst service.
 *===========================================================================*/
StatusCode UA_ClientApi_QueryFirst(
    OpcUa_Channel                 a_hChannel,
    const UA_RequestHeader*       a_pRequestHeader,
    const UA_ViewDescription*     a_pView,
    int32_t                       a_nNoOfNodeTypes,
    const UA_NodeTypeDescription* a_pNodeTypes,
    const UA_ContentFilter*       a_pFilter,
    uint32_t                      a_nMaxDataSetsToReturn,
    uint32_t                      a_nMaxReferencesToReturn,
    UA_ResponseHeader*            a_pResponseHeader,
    int32_t*                      a_pNoOfQueryDataSets,
    UA_QueryDataSet**             a_pQueryDataSets,
    UA_ByteString*                a_pContinuationPoint,
    int32_t*                      a_pNoOfParsingResults,
    UA_ParsingResult**            a_pParsingResults,
    int32_t*                      a_pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**           a_pDiagnosticInfos,
    UA_ContentFilterResult*       a_pFilterResult)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_QueryFirstRequest cRequest;
    UA_QueryFirstResponse* pResponse = UA_NULL;
    UA_EncodeableType* pResponseType = UA_NULL;
    
    UA_QueryFirstRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  a_pView != UA_NULL
       &&  (a_pNodeTypes != UA_NULL || a_nNoOfNodeTypes <= 0)
       &&  a_pFilter != UA_NULL
       &&  a_pResponseHeader != UA_NULL
       &&  a_pNoOfQueryDataSets != UA_NULL
       &&  a_pQueryDataSets != UA_NULL
       &&  a_pContinuationPoint != UA_NULL
       &&  a_pNoOfParsingResults != UA_NULL
       &&  a_pParsingResults != UA_NULL
       &&  a_pNoOfDiagnosticInfos != UA_NULL
       &&  a_pDiagnosticInfos != UA_NULL
       &&  a_pFilterResult != UA_NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader         = *a_pRequestHeader;
        cRequest.View                  = *a_pView;
        cRequest.NoOfNodeTypes         = a_nNoOfNodeTypes;
        cRequest.NodeTypes             = (UA_NodeTypeDescription*)a_pNodeTypes;
        cRequest.Filter                = *a_pFilter;
        cRequest.MaxDataSetsToReturn   = a_nMaxDataSetsToReturn;
        cRequest.MaxReferencesToReturn = a_nMaxReferencesToReturn;

        /* invoke service */
        status = UA_Channel_InvokeService(
            a_hChannel,
            "QueryFirst",
            (void*)&cRequest,
            &UA_QueryFirstRequest_EncodeableType,
            &UA_QueryFirstResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->typeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((UA_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (UA_QueryFirstResponse_EncodeableType.typeId != pResponseType->typeId)
        {
            pResponseType->clearFunction(pResponse);
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
StatusCode UA_ClientApi_BeginQueryFirst(
    OpcUa_Channel                     a_hChannel,
    const UA_RequestHeader*           a_pRequestHeader,
    const UA_ViewDescription*         a_pView,
    int32_t                           a_nNoOfNodeTypes,
    const UA_NodeTypeDescription*     a_pNodeTypes,
    const UA_ContentFilter*           a_pFilter,
    uint32_t                          a_nMaxDataSetsToReturn,
    uint32_t                          a_nMaxReferencesToReturn,
    UA_Channel_PfnRequestComplete*    a_pCallback,
    OpcUa_Void*                       a_pCallbackData)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_QueryFirstRequest cRequest;
    UA_QueryFirstRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  a_pView != UA_NULL
       &&  (a_pNodeTypes != UA_NULL || a_nNoOfNodeTypes <= 0)
       &&  a_pFilter != UA_NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader         = *a_pRequestHeader;
        cRequest.View                  = *a_pView;
        cRequest.NoOfNodeTypes         = a_nNoOfNodeTypes;
        cRequest.NodeTypes             = (UA_NodeTypeDescription*)a_pNodeTypes;
        cRequest.Filter                = *a_pFilter;
        cRequest.MaxDataSetsToReturn   = a_nMaxDataSetsToReturn;
        cRequest.MaxReferencesToReturn = a_nMaxReferencesToReturn;

        /* begin invoke service */
        status = UA_Channel_BeginInvokeService(
            a_hChannel,
            "QueryFirst",
            (OpcUa_Void*)&cRequest,
            &UA_QueryFirstRequest_EncodeableType,
            &UA_QueryFirstResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*   )a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_QueryNext
/*============================================================================
 * Synchronously calls the QueryNext service.
 *===========================================================================*/
StatusCode UA_ClientApi_QueryNext(
    OpcUa_Channel           a_hChannel,
    const UA_RequestHeader* a_pRequestHeader,
    UA_Boolean              a_bReleaseContinuationPoint,
    const UA_ByteString*    a_pContinuationPoint,
    UA_ResponseHeader*      a_pResponseHeader,
    int32_t*                a_pNoOfQueryDataSets,
    UA_QueryDataSet**       a_pQueryDataSets,
    UA_ByteString*          a_pRevisedContinuationPoint)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_QueryNextRequest cRequest;
    UA_QueryNextResponse* pResponse = UA_NULL;
    UA_EncodeableType* pResponseType = UA_NULL;
    
    UA_QueryNextRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  a_pContinuationPoint != UA_NULL
       &&  a_pResponseHeader != UA_NULL
       &&  a_pNoOfQueryDataSets != UA_NULL
       &&  a_pQueryDataSets != UA_NULL
       &&  a_pRevisedContinuationPoint != UA_NULL)
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
            &UA_QueryNextRequest_EncodeableType,
            &UA_QueryNextResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->typeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((UA_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (UA_QueryNextResponse_EncodeableType.typeId != pResponseType->typeId)
        {
            pResponseType->clearFunction(pResponse);
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
StatusCode UA_ClientApi_BeginQueryNext(
    OpcUa_Channel                     a_hChannel,
    const UA_RequestHeader*           a_pRequestHeader,
    UA_Boolean                        a_bReleaseContinuationPoint,
    const UA_ByteString*              a_pContinuationPoint,
    UA_Channel_PfnRequestComplete*    a_pCallback,
    OpcUa_Void*                       a_pCallbackData)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_QueryNextRequest cRequest;
    UA_QueryNextRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  a_pContinuationPoint != UA_NULL)
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
            (OpcUa_Void*)&cRequest,
            &UA_QueryNextRequest_EncodeableType,
            &UA_QueryNextResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*   )a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_Read
/*============================================================================
 * Synchronously calls the Read service.
 *===========================================================================*/
StatusCode UA_ClientApi_Read(
    OpcUa_Channel           a_hChannel,
    const UA_RequestHeader* a_pRequestHeader,
    double                  a_nMaxAge,
    UA_TimestampsToReturn   a_eTimestampsToReturn,
    int32_t                 a_nNoOfNodesToRead,
    const UA_ReadValueId*   a_pNodesToRead,
    UA_ResponseHeader*      a_pResponseHeader,
    int32_t*                a_pNoOfResults,
    UA_DataValue**          a_pResults,
    int32_t*                a_pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**     a_pDiagnosticInfos)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_ReadRequest cRequest;
    UA_ReadResponse* pResponse = UA_NULL;
    UA_EncodeableType* pResponseType = UA_NULL;
    
    UA_ReadRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  (a_pNodesToRead != UA_NULL || a_nNoOfNodesToRead <= 0)
       &&  a_pResponseHeader != UA_NULL
       &&  a_pNoOfResults != UA_NULL
       &&  a_pResults != UA_NULL
       &&  a_pNoOfDiagnosticInfos != UA_NULL
       &&  a_pDiagnosticInfos != UA_NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader      = *a_pRequestHeader;
        cRequest.MaxAge             = a_nMaxAge;
        cRequest.TimestampsToReturn = a_eTimestampsToReturn;
        cRequest.NoOfNodesToRead    = a_nNoOfNodesToRead;
        cRequest.NodesToRead        = (UA_ReadValueId*)a_pNodesToRead;

        /* invoke service */
        status = UA_Channel_InvokeService(
            a_hChannel,
            "Read",
            (void*)&cRequest,
            &UA_ReadRequest_EncodeableType,
            &UA_ReadResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->typeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((UA_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (UA_ReadResponse_EncodeableType.typeId != pResponseType->typeId)
        {
            pResponseType->clearFunction(pResponse);
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
StatusCode UA_ClientApi_BeginRead(
    OpcUa_Channel                     a_hChannel,
    const UA_RequestHeader*           a_pRequestHeader,
    double                            a_nMaxAge,
    UA_TimestampsToReturn             a_eTimestampsToReturn,
    int32_t                           a_nNoOfNodesToRead,
    const UA_ReadValueId*             a_pNodesToRead,
    UA_Channel_PfnRequestComplete*    a_pCallback,
    OpcUa_Void*                       a_pCallbackData)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_ReadRequest cRequest;
    UA_ReadRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  (a_pNodesToRead != UA_NULL || a_nNoOfNodesToRead <= 0))
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader      = *a_pRequestHeader;
        cRequest.MaxAge             = a_nMaxAge;
        cRequest.TimestampsToReturn = a_eTimestampsToReturn;
        cRequest.NoOfNodesToRead    = a_nNoOfNodesToRead;
        cRequest.NodesToRead        = (UA_ReadValueId*)a_pNodesToRead;

        /* begin invoke service */
        status = UA_Channel_BeginInvokeService(
            a_hChannel,
            "Read",
            (OpcUa_Void*)&cRequest,
            &UA_ReadRequest_EncodeableType,
            &UA_ReadResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*   )a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_HistoryRead
/*============================================================================
 * Synchronously calls the HistoryRead service.
 *===========================================================================*/
StatusCode UA_ClientApi_HistoryRead(
    OpcUa_Channel                a_hChannel,
    const UA_RequestHeader*      a_pRequestHeader,
    const UA_ExtensionObject*    a_pHistoryReadDetails,
    UA_TimestampsToReturn        a_eTimestampsToReturn,
    UA_Boolean                   a_bReleaseContinuationPoints,
    int32_t                      a_nNoOfNodesToRead,
    const UA_HistoryReadValueId* a_pNodesToRead,
    UA_ResponseHeader*           a_pResponseHeader,
    int32_t*                     a_pNoOfResults,
    UA_HistoryReadResult**       a_pResults,
    int32_t*                     a_pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**          a_pDiagnosticInfos)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_HistoryReadRequest cRequest;
    UA_HistoryReadResponse* pResponse = UA_NULL;
    UA_EncodeableType* pResponseType = UA_NULL;
    
    UA_HistoryReadRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  a_pHistoryReadDetails != UA_NULL
       &&  (a_pNodesToRead != UA_NULL || a_nNoOfNodesToRead <= 0)
       &&  a_pResponseHeader != UA_NULL
       &&  a_pNoOfResults != UA_NULL
       &&  a_pResults != UA_NULL
       &&  a_pNoOfDiagnosticInfos != UA_NULL
       &&  a_pDiagnosticInfos != UA_NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader             = *a_pRequestHeader;
        cRequest.HistoryReadDetails        = *a_pHistoryReadDetails;
        cRequest.TimestampsToReturn        = a_eTimestampsToReturn;
        cRequest.ReleaseContinuationPoints = a_bReleaseContinuationPoints;
        cRequest.NoOfNodesToRead           = a_nNoOfNodesToRead;
        cRequest.NodesToRead               = (UA_HistoryReadValueId*)a_pNodesToRead;

        /* invoke service */
        status = UA_Channel_InvokeService(
            a_hChannel,
            "HistoryRead",
            (void*)&cRequest,
            &UA_HistoryReadRequest_EncodeableType,
            &UA_HistoryReadResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->typeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((UA_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (UA_HistoryReadResponse_EncodeableType.typeId != pResponseType->typeId)
        {
            pResponseType->clearFunction(pResponse);
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
StatusCode UA_ClientApi_BeginHistoryRead(
    OpcUa_Channel                     a_hChannel,
    const UA_RequestHeader*           a_pRequestHeader,
    const UA_ExtensionObject*         a_pHistoryReadDetails,
    UA_TimestampsToReturn             a_eTimestampsToReturn,
    UA_Boolean                        a_bReleaseContinuationPoints,
    int32_t                           a_nNoOfNodesToRead,
    const UA_HistoryReadValueId*      a_pNodesToRead,
    UA_Channel_PfnRequestComplete*    a_pCallback,
    OpcUa_Void*                       a_pCallbackData)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_HistoryReadRequest cRequest;
    UA_HistoryReadRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  a_pHistoryReadDetails != UA_NULL
       &&  (a_pNodesToRead != UA_NULL || a_nNoOfNodesToRead <= 0))
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader             = *a_pRequestHeader;
        cRequest.HistoryReadDetails        = *a_pHistoryReadDetails;
        cRequest.TimestampsToReturn        = a_eTimestampsToReturn;
        cRequest.ReleaseContinuationPoints = a_bReleaseContinuationPoints;
        cRequest.NoOfNodesToRead           = a_nNoOfNodesToRead;
        cRequest.NodesToRead               = (UA_HistoryReadValueId*)a_pNodesToRead;

        /* begin invoke service */
        status = UA_Channel_BeginInvokeService(
            a_hChannel,
            "HistoryRead",
            (OpcUa_Void*)&cRequest,
            &UA_HistoryReadRequest_EncodeableType,
            &UA_HistoryReadResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*   )a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_Write
/*============================================================================
 * Synchronously calls the Write service.
 *===========================================================================*/
StatusCode UA_ClientApi_Write(
    OpcUa_Channel           a_hChannel,
    const UA_RequestHeader* a_pRequestHeader,
    int32_t                 a_nNoOfNodesToWrite,
    const UA_WriteValue*    a_pNodesToWrite,
    UA_ResponseHeader*      a_pResponseHeader,
    int32_t*                a_pNoOfResults,
    StatusCode**            a_pResults,
    int32_t*                a_pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**     a_pDiagnosticInfos)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_WriteRequest cRequest;
    UA_WriteResponse* pResponse = UA_NULL;
    UA_EncodeableType* pResponseType = UA_NULL;
    
    UA_WriteRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  (a_pNodesToWrite != UA_NULL || a_nNoOfNodesToWrite <= 0)
       &&  a_pResponseHeader != UA_NULL
       &&  a_pNoOfResults != UA_NULL
       &&  a_pResults != UA_NULL
       &&  a_pNoOfDiagnosticInfos != UA_NULL
       &&  a_pDiagnosticInfos != UA_NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader    = *a_pRequestHeader;
        cRequest.NoOfNodesToWrite = a_nNoOfNodesToWrite;
        cRequest.NodesToWrite     = (UA_WriteValue*)a_pNodesToWrite;

        /* invoke service */
        status = UA_Channel_InvokeService(
            a_hChannel,
            "Write",
            (void*)&cRequest,
            &UA_WriteRequest_EncodeableType,
            &UA_WriteResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->typeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((UA_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (UA_WriteResponse_EncodeableType.typeId != pResponseType->typeId)
        {
            pResponseType->clearFunction(pResponse);
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
StatusCode UA_ClientApi_BeginWrite(
    OpcUa_Channel                     a_hChannel,
    const UA_RequestHeader*           a_pRequestHeader,
    int32_t                           a_nNoOfNodesToWrite,
    const UA_WriteValue*              a_pNodesToWrite,
    UA_Channel_PfnRequestComplete*    a_pCallback,
    OpcUa_Void*                       a_pCallbackData)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_WriteRequest cRequest;
    UA_WriteRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  (a_pNodesToWrite != UA_NULL || a_nNoOfNodesToWrite <= 0))
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader    = *a_pRequestHeader;
        cRequest.NoOfNodesToWrite = a_nNoOfNodesToWrite;
        cRequest.NodesToWrite     = (UA_WriteValue*)a_pNodesToWrite;

        /* begin invoke service */
        status = UA_Channel_BeginInvokeService(
            a_hChannel,
            "Write",
            (OpcUa_Void*)&cRequest,
            &UA_WriteRequest_EncodeableType,
            &UA_WriteResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*   )a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_HistoryUpdate
/*============================================================================
 * Synchronously calls the HistoryUpdate service.
 *===========================================================================*/
StatusCode UA_ClientApi_HistoryUpdate(
    OpcUa_Channel             a_hChannel,
    const UA_RequestHeader*   a_pRequestHeader,
    int32_t                   a_nNoOfHistoryUpdateDetails,
    const UA_ExtensionObject* a_pHistoryUpdateDetails,
    UA_ResponseHeader*        a_pResponseHeader,
    int32_t*                  a_pNoOfResults,
    UA_HistoryUpdateResult**  a_pResults,
    int32_t*                  a_pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**       a_pDiagnosticInfos)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_HistoryUpdateRequest cRequest;
    UA_HistoryUpdateResponse* pResponse = UA_NULL;
    UA_EncodeableType* pResponseType = UA_NULL;
    
    UA_HistoryUpdateRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  (a_pHistoryUpdateDetails != UA_NULL || a_nNoOfHistoryUpdateDetails <= 0)
       &&  a_pResponseHeader != UA_NULL
       &&  a_pNoOfResults != UA_NULL
       &&  a_pResults != UA_NULL
       &&  a_pNoOfDiagnosticInfos != UA_NULL
       &&  a_pDiagnosticInfos != UA_NULL)
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
            &UA_HistoryUpdateRequest_EncodeableType,
            &UA_HistoryUpdateResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->typeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((UA_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (UA_HistoryUpdateResponse_EncodeableType.typeId != pResponseType->typeId)
        {
            pResponseType->clearFunction(pResponse);
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
StatusCode UA_ClientApi_BeginHistoryUpdate(
    OpcUa_Channel                     a_hChannel,
    const UA_RequestHeader*           a_pRequestHeader,
    int32_t                           a_nNoOfHistoryUpdateDetails,
    const UA_ExtensionObject*         a_pHistoryUpdateDetails,
    UA_Channel_PfnRequestComplete*    a_pCallback,
    OpcUa_Void*                       a_pCallbackData)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_HistoryUpdateRequest cRequest;
    UA_HistoryUpdateRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  (a_pHistoryUpdateDetails != UA_NULL || a_nNoOfHistoryUpdateDetails <= 0))
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
            (OpcUa_Void*)&cRequest,
            &UA_HistoryUpdateRequest_EncodeableType,
            &UA_HistoryUpdateResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*   )a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_Call
/*============================================================================
 * Synchronously calls the Call service.
 *===========================================================================*/
StatusCode UA_ClientApi_Call(
    OpcUa_Channel               a_hChannel,
    const UA_RequestHeader*     a_pRequestHeader,
    int32_t                     a_nNoOfMethodsToCall,
    const UA_CallMethodRequest* a_pMethodsToCall,
    UA_ResponseHeader*          a_pResponseHeader,
    int32_t*                    a_pNoOfResults,
    UA_CallMethodResult**       a_pResults,
    int32_t*                    a_pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**         a_pDiagnosticInfos)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_CallRequest cRequest;
    UA_CallResponse* pResponse = UA_NULL;
    UA_EncodeableType* pResponseType = UA_NULL;
    
    UA_CallRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  (a_pMethodsToCall != UA_NULL || a_nNoOfMethodsToCall <= 0)
       &&  a_pResponseHeader != UA_NULL
       &&  a_pNoOfResults != UA_NULL
       &&  a_pResults != UA_NULL
       &&  a_pNoOfDiagnosticInfos != UA_NULL
       &&  a_pDiagnosticInfos != UA_NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader     = *a_pRequestHeader;
        cRequest.NoOfMethodsToCall = a_nNoOfMethodsToCall;
        cRequest.MethodsToCall     = (UA_CallMethodRequest*)a_pMethodsToCall;

        /* invoke service */
        status = UA_Channel_InvokeService(
            a_hChannel,
            "Call",
            (void*)&cRequest,
            &UA_CallRequest_EncodeableType,
            &UA_CallResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->typeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((UA_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (UA_CallResponse_EncodeableType.typeId != pResponseType->typeId)
        {
            pResponseType->clearFunction(pResponse);
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
StatusCode UA_ClientApi_BeginCall(
    OpcUa_Channel                     a_hChannel,
    const UA_RequestHeader*           a_pRequestHeader,
    int32_t                           a_nNoOfMethodsToCall,
    const UA_CallMethodRequest*       a_pMethodsToCall,
    UA_Channel_PfnRequestComplete*    a_pCallback,
    OpcUa_Void*                       a_pCallbackData)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_CallRequest cRequest;
    UA_CallRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  (a_pMethodsToCall != UA_NULL || a_nNoOfMethodsToCall <= 0))
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader     = *a_pRequestHeader;
        cRequest.NoOfMethodsToCall = a_nNoOfMethodsToCall;
        cRequest.MethodsToCall     = (UA_CallMethodRequest*)a_pMethodsToCall;

        /* begin invoke service */
        status = UA_Channel_BeginInvokeService(
            a_hChannel,
            "Call",
            (OpcUa_Void*)&cRequest,
            &UA_CallRequest_EncodeableType,
            &UA_CallResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*   )a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_CreateMonitoredItems
/*============================================================================
 * Synchronously calls the CreateMonitoredItems service.
 *===========================================================================*/
StatusCode UA_ClientApi_CreateMonitoredItems(
    OpcUa_Channel                        a_hChannel,
    const UA_RequestHeader*              a_pRequestHeader,
    uint32_t                             a_nSubscriptionId,
    UA_TimestampsToReturn                a_eTimestampsToReturn,
    int32_t                              a_nNoOfItemsToCreate,
    const UA_MonitoredItemCreateRequest* a_pItemsToCreate,
    UA_ResponseHeader*                   a_pResponseHeader,
    int32_t*                             a_pNoOfResults,
    UA_MonitoredItemCreateResult**       a_pResults,
    int32_t*                             a_pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**                  a_pDiagnosticInfos)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_CreateMonitoredItemsRequest cRequest;
    UA_CreateMonitoredItemsResponse* pResponse = UA_NULL;
    UA_EncodeableType* pResponseType = UA_NULL;
    
    UA_CreateMonitoredItemsRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  (a_pItemsToCreate != UA_NULL || a_nNoOfItemsToCreate <= 0)
       &&  a_pResponseHeader != UA_NULL
       &&  a_pNoOfResults != UA_NULL
       &&  a_pResults != UA_NULL
       &&  a_pNoOfDiagnosticInfos != UA_NULL
       &&  a_pDiagnosticInfos != UA_NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader      = *a_pRequestHeader;
        cRequest.SubscriptionId     = a_nSubscriptionId;
        cRequest.TimestampsToReturn = a_eTimestampsToReturn;
        cRequest.NoOfItemsToCreate  = a_nNoOfItemsToCreate;
        cRequest.ItemsToCreate      = (UA_MonitoredItemCreateRequest*)a_pItemsToCreate;

        /* invoke service */
        status = UA_Channel_InvokeService(
            a_hChannel,
            "CreateMonitoredItems",
            (void*)&cRequest,
            &UA_CreateMonitoredItemsRequest_EncodeableType,
            &UA_CreateMonitoredItemsResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->typeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((UA_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (UA_CreateMonitoredItemsResponse_EncodeableType.typeId != pResponseType->typeId)
        {
            pResponseType->clearFunction(pResponse);
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
StatusCode UA_ClientApi_BeginCreateMonitoredItems(
    OpcUa_Channel                        a_hChannel,
    const UA_RequestHeader*              a_pRequestHeader,
    uint32_t                             a_nSubscriptionId,
    UA_TimestampsToReturn                a_eTimestampsToReturn,
    int32_t                              a_nNoOfItemsToCreate,
    const UA_MonitoredItemCreateRequest* a_pItemsToCreate,
    UA_Channel_PfnRequestComplete*       a_pCallback,
    OpcUa_Void*                          a_pCallbackData)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_CreateMonitoredItemsRequest cRequest;
    UA_CreateMonitoredItemsRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  (a_pItemsToCreate != UA_NULL || a_nNoOfItemsToCreate <= 0))
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader      = *a_pRequestHeader;
        cRequest.SubscriptionId     = a_nSubscriptionId;
        cRequest.TimestampsToReturn = a_eTimestampsToReturn;
        cRequest.NoOfItemsToCreate  = a_nNoOfItemsToCreate;
        cRequest.ItemsToCreate      = (UA_MonitoredItemCreateRequest*)a_pItemsToCreate;

        /* begin invoke service */
        status = UA_Channel_BeginInvokeService(
            a_hChannel,
            "CreateMonitoredItems",
            (OpcUa_Void*)&cRequest,
            &UA_CreateMonitoredItemsRequest_EncodeableType,
            &UA_CreateMonitoredItemsResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*   )a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_ModifyMonitoredItems
/*============================================================================
 * Synchronously calls the ModifyMonitoredItems service.
 *===========================================================================*/
StatusCode UA_ClientApi_ModifyMonitoredItems(
    OpcUa_Channel                        a_hChannel,
    const UA_RequestHeader*              a_pRequestHeader,
    uint32_t                             a_nSubscriptionId,
    UA_TimestampsToReturn                a_eTimestampsToReturn,
    int32_t                              a_nNoOfItemsToModify,
    const UA_MonitoredItemModifyRequest* a_pItemsToModify,
    UA_ResponseHeader*                   a_pResponseHeader,
    int32_t*                             a_pNoOfResults,
    UA_MonitoredItemModifyResult**       a_pResults,
    int32_t*                             a_pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**                  a_pDiagnosticInfos)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_ModifyMonitoredItemsRequest cRequest;
    UA_ModifyMonitoredItemsResponse* pResponse = UA_NULL;
    UA_EncodeableType* pResponseType = UA_NULL;
    
    UA_ModifyMonitoredItemsRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  (a_pItemsToModify != UA_NULL || a_nNoOfItemsToModify <= 0)
       &&  a_pResponseHeader != UA_NULL
       &&  a_pNoOfResults != UA_NULL
       &&  a_pResults != UA_NULL
       &&  a_pNoOfDiagnosticInfos != UA_NULL
       &&  a_pDiagnosticInfos != UA_NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader      = *a_pRequestHeader;
        cRequest.SubscriptionId     = a_nSubscriptionId;
        cRequest.TimestampsToReturn = a_eTimestampsToReturn;
        cRequest.NoOfItemsToModify  = a_nNoOfItemsToModify;
        cRequest.ItemsToModify      = (UA_MonitoredItemModifyRequest*)a_pItemsToModify;

        /* invoke service */
        status = UA_Channel_InvokeService(
            a_hChannel,
            "ModifyMonitoredItems",
            (void*)&cRequest,
            &UA_ModifyMonitoredItemsRequest_EncodeableType,
            &UA_ModifyMonitoredItemsResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->typeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((UA_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (UA_ModifyMonitoredItemsResponse_EncodeableType.typeId != pResponseType->typeId)
        {
            pResponseType->clearFunction(pResponse);
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
StatusCode UA_ClientApi_BeginModifyMonitoredItems(
    OpcUa_Channel                        a_hChannel,
    const UA_RequestHeader*              a_pRequestHeader,
    uint32_t                             a_nSubscriptionId,
    UA_TimestampsToReturn                a_eTimestampsToReturn,
    int32_t                              a_nNoOfItemsToModify,
    const UA_MonitoredItemModifyRequest* a_pItemsToModify,
    UA_Channel_PfnRequestComplete*       a_pCallback,
    OpcUa_Void*                          a_pCallbackData)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_ModifyMonitoredItemsRequest cRequest;
    UA_ModifyMonitoredItemsRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  (a_pItemsToModify != UA_NULL || a_nNoOfItemsToModify <= 0))
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader      = *a_pRequestHeader;
        cRequest.SubscriptionId     = a_nSubscriptionId;
        cRequest.TimestampsToReturn = a_eTimestampsToReturn;
        cRequest.NoOfItemsToModify  = a_nNoOfItemsToModify;
        cRequest.ItemsToModify      = (UA_MonitoredItemModifyRequest*)a_pItemsToModify;

        /* begin invoke service */
        status = UA_Channel_BeginInvokeService(
            a_hChannel,
            "ModifyMonitoredItems",
            (OpcUa_Void*)&cRequest,
            &UA_ModifyMonitoredItemsRequest_EncodeableType,
            &UA_ModifyMonitoredItemsResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*   )a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_SetMonitoringMode
/*============================================================================
 * Synchronously calls the SetMonitoringMode service.
 *===========================================================================*/
StatusCode UA_ClientApi_SetMonitoringMode(
    OpcUa_Channel           a_hChannel,
    const UA_RequestHeader* a_pRequestHeader,
    uint32_t                a_nSubscriptionId,
    UA_MonitoringMode       a_eMonitoringMode,
    int32_t                 a_nNoOfMonitoredItemIds,
    const uint32_t*         a_pMonitoredItemIds,
    UA_ResponseHeader*      a_pResponseHeader,
    int32_t*                a_pNoOfResults,
    StatusCode**            a_pResults,
    int32_t*                a_pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**     a_pDiagnosticInfos)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_SetMonitoringModeRequest cRequest;
    UA_SetMonitoringModeResponse* pResponse = UA_NULL;
    UA_EncodeableType* pResponseType = UA_NULL;
    
    UA_SetMonitoringModeRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  (a_pMonitoredItemIds != UA_NULL || a_nNoOfMonitoredItemIds <= 0)
       &&  a_pResponseHeader != UA_NULL
       &&  a_pNoOfResults != UA_NULL
       &&  a_pResults != UA_NULL
       &&  a_pNoOfDiagnosticInfos != UA_NULL
       &&  a_pDiagnosticInfos != UA_NULL)
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
            &UA_SetMonitoringModeRequest_EncodeableType,
            &UA_SetMonitoringModeResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->typeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((UA_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (UA_SetMonitoringModeResponse_EncodeableType.typeId != pResponseType->typeId)
        {
            pResponseType->clearFunction(pResponse);
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
StatusCode UA_ClientApi_BeginSetMonitoringMode(
    OpcUa_Channel                     a_hChannel,
    const UA_RequestHeader*           a_pRequestHeader,
    uint32_t                          a_nSubscriptionId,
    UA_MonitoringMode                 a_eMonitoringMode,
    int32_t                           a_nNoOfMonitoredItemIds,
    const uint32_t*                   a_pMonitoredItemIds,
    UA_Channel_PfnRequestComplete*    a_pCallback,
    OpcUa_Void*                       a_pCallbackData)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_SetMonitoringModeRequest cRequest;
    UA_SetMonitoringModeRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  (a_pMonitoredItemIds != UA_NULL || a_nNoOfMonitoredItemIds <= 0))
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
            (OpcUa_Void*)&cRequest,
            &UA_SetMonitoringModeRequest_EncodeableType,
            &UA_SetMonitoringModeResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*   )a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_SetTriggering
/*============================================================================
 * Synchronously calls the SetTriggering service.
 *===========================================================================*/
StatusCode UA_ClientApi_SetTriggering(
    OpcUa_Channel           a_hChannel,
    const UA_RequestHeader* a_pRequestHeader,
    uint32_t                a_nSubscriptionId,
    uint32_t                a_nTriggeringItemId,
    int32_t                 a_nNoOfLinksToAdd,
    const uint32_t*         a_pLinksToAdd,
    int32_t                 a_nNoOfLinksToRemove,
    const uint32_t*         a_pLinksToRemove,
    UA_ResponseHeader*      a_pResponseHeader,
    int32_t*                a_pNoOfAddResults,
    StatusCode**            a_pAddResults,
    int32_t*                a_pNoOfAddDiagnosticInfos,
    UA_DiagnosticInfo**     a_pAddDiagnosticInfos,
    int32_t*                a_pNoOfRemoveResults,
    StatusCode**            a_pRemoveResults,
    int32_t*                a_pNoOfRemoveDiagnosticInfos,
    UA_DiagnosticInfo**     a_pRemoveDiagnosticInfos)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_SetTriggeringRequest cRequest;
    UA_SetTriggeringResponse* pResponse = UA_NULL;
    UA_EncodeableType* pResponseType = UA_NULL;
    
    UA_SetTriggeringRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  (a_pLinksToAdd != UA_NULL || a_nNoOfLinksToAdd <= 0)
       &&  (a_pLinksToRemove != UA_NULL || a_nNoOfLinksToRemove <= 0)
       &&  a_pResponseHeader != UA_NULL
       &&  a_pNoOfAddResults != UA_NULL
       &&  a_pAddResults != UA_NULL
       &&  a_pNoOfAddDiagnosticInfos != UA_NULL
       &&  a_pAddDiagnosticInfos != UA_NULL
       &&  a_pNoOfRemoveResults != UA_NULL
       &&  a_pRemoveResults != UA_NULL
       &&  a_pNoOfRemoveDiagnosticInfos != UA_NULL
       &&  a_pRemoveDiagnosticInfos != UA_NULL)
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
            &UA_SetTriggeringRequest_EncodeableType,
            &UA_SetTriggeringResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->typeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((UA_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (UA_SetTriggeringResponse_EncodeableType.typeId != pResponseType->typeId)
        {
            pResponseType->clearFunction(pResponse);
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
StatusCode UA_ClientApi_BeginSetTriggering(
    OpcUa_Channel                     a_hChannel,
    const UA_RequestHeader*           a_pRequestHeader,
    uint32_t                          a_nSubscriptionId,
    uint32_t                          a_nTriggeringItemId,
    int32_t                           a_nNoOfLinksToAdd,
    const uint32_t*                   a_pLinksToAdd,
    int32_t                           a_nNoOfLinksToRemove,
    const uint32_t*                   a_pLinksToRemove,
    UA_Channel_PfnRequestComplete*    a_pCallback,
    OpcUa_Void*                       a_pCallbackData)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_SetTriggeringRequest cRequest;
    UA_SetTriggeringRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  (a_pLinksToAdd != UA_NULL || a_nNoOfLinksToAdd <= 0)
       &&  (a_pLinksToRemove != UA_NULL || a_nNoOfLinksToRemove <= 0))
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
            (OpcUa_Void*)&cRequest,
            &UA_SetTriggeringRequest_EncodeableType,
            &UA_SetTriggeringResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*   )a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_DeleteMonitoredItems
/*============================================================================
 * Synchronously calls the DeleteMonitoredItems service.
 *===========================================================================*/
StatusCode UA_ClientApi_DeleteMonitoredItems(
    OpcUa_Channel           a_hChannel,
    const UA_RequestHeader* a_pRequestHeader,
    uint32_t                a_nSubscriptionId,
    int32_t                 a_nNoOfMonitoredItemIds,
    const uint32_t*         a_pMonitoredItemIds,
    UA_ResponseHeader*      a_pResponseHeader,
    int32_t*                a_pNoOfResults,
    StatusCode**            a_pResults,
    int32_t*                a_pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**     a_pDiagnosticInfos)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_DeleteMonitoredItemsRequest cRequest;
    UA_DeleteMonitoredItemsResponse* pResponse = UA_NULL;
    UA_EncodeableType* pResponseType = UA_NULL;
    
    UA_DeleteMonitoredItemsRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  (a_pMonitoredItemIds != UA_NULL || a_nNoOfMonitoredItemIds <= 0)
       &&  a_pResponseHeader != UA_NULL
       &&  a_pNoOfResults != UA_NULL
       &&  a_pResults != UA_NULL
       &&  a_pNoOfDiagnosticInfos != UA_NULL
       &&  a_pDiagnosticInfos != UA_NULL)
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
            &UA_DeleteMonitoredItemsRequest_EncodeableType,
            &UA_DeleteMonitoredItemsResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->typeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((UA_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (UA_DeleteMonitoredItemsResponse_EncodeableType.typeId != pResponseType->typeId)
        {
            pResponseType->clearFunction(pResponse);
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
StatusCode UA_ClientApi_BeginDeleteMonitoredItems(
    OpcUa_Channel                     a_hChannel,
    const UA_RequestHeader*           a_pRequestHeader,
    uint32_t                          a_nSubscriptionId,
    int32_t                           a_nNoOfMonitoredItemIds,
    const uint32_t*                   a_pMonitoredItemIds,
    UA_Channel_PfnRequestComplete*    a_pCallback,
    OpcUa_Void*                       a_pCallbackData)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_DeleteMonitoredItemsRequest cRequest;
    UA_DeleteMonitoredItemsRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  (a_pMonitoredItemIds != UA_NULL || a_nNoOfMonitoredItemIds <= 0))
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
            (OpcUa_Void*)&cRequest,
            &UA_DeleteMonitoredItemsRequest_EncodeableType,
            &UA_DeleteMonitoredItemsResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*   )a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_CreateSubscription
/*============================================================================
 * Synchronously calls the CreateSubscription service.
 *===========================================================================*/
StatusCode UA_ClientApi_CreateSubscription(
    OpcUa_Channel           a_hChannel,
    const UA_RequestHeader* a_pRequestHeader,
    double                  a_nRequestedPublishingInterval,
    uint32_t                a_nRequestedLifetimeCount,
    uint32_t                a_nRequestedMaxKeepAliveCount,
    uint32_t                a_nMaxNotificationsPerPublish,
    UA_Boolean              a_bPublishingEnabled,
    UA_Byte                 a_nPriority,
    UA_ResponseHeader*      a_pResponseHeader,
    uint32_t*               a_pSubscriptionId,
    double*                 a_pRevisedPublishingInterval,
    uint32_t*               a_pRevisedLifetimeCount,
    uint32_t*               a_pRevisedMaxKeepAliveCount)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_CreateSubscriptionRequest cRequest;
    UA_CreateSubscriptionResponse* pResponse = UA_NULL;
    UA_EncodeableType* pResponseType = UA_NULL;
    
    UA_CreateSubscriptionRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  a_pResponseHeader != UA_NULL
       &&  a_pSubscriptionId != UA_NULL
       &&  a_pRevisedPublishingInterval != UA_NULL
       &&  a_pRevisedLifetimeCount != UA_NULL
       &&  a_pRevisedMaxKeepAliveCount != UA_NULL)
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
            &UA_CreateSubscriptionRequest_EncodeableType,
            &UA_CreateSubscriptionResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->typeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((UA_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (UA_CreateSubscriptionResponse_EncodeableType.typeId != pResponseType->typeId)
        {
            pResponseType->clearFunction(pResponse);
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
StatusCode UA_ClientApi_BeginCreateSubscription(
    OpcUa_Channel                     a_hChannel,
    const UA_RequestHeader*           a_pRequestHeader,
    double                            a_nRequestedPublishingInterval,
    uint32_t                          a_nRequestedLifetimeCount,
    uint32_t                          a_nRequestedMaxKeepAliveCount,
    uint32_t                          a_nMaxNotificationsPerPublish,
    UA_Boolean                        a_bPublishingEnabled,
    UA_Byte                           a_nPriority,
    UA_Channel_PfnRequestComplete*    a_pCallback,
    OpcUa_Void*                       a_pCallbackData)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_CreateSubscriptionRequest cRequest;
    UA_CreateSubscriptionRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL)
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
            (OpcUa_Void*)&cRequest,
            &UA_CreateSubscriptionRequest_EncodeableType,
            &UA_CreateSubscriptionResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*   )a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_ModifySubscription
/*============================================================================
 * Synchronously calls the ModifySubscription service.
 *===========================================================================*/
StatusCode UA_ClientApi_ModifySubscription(
    OpcUa_Channel           a_hChannel,
    const UA_RequestHeader* a_pRequestHeader,
    uint32_t                a_nSubscriptionId,
    double                  a_nRequestedPublishingInterval,
    uint32_t                a_nRequestedLifetimeCount,
    uint32_t                a_nRequestedMaxKeepAliveCount,
    uint32_t                a_nMaxNotificationsPerPublish,
    UA_Byte                 a_nPriority,
    UA_ResponseHeader*      a_pResponseHeader,
    double*                 a_pRevisedPublishingInterval,
    uint32_t*               a_pRevisedLifetimeCount,
    uint32_t*               a_pRevisedMaxKeepAliveCount)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_ModifySubscriptionRequest cRequest;
    UA_ModifySubscriptionResponse* pResponse = UA_NULL;
    UA_EncodeableType* pResponseType = UA_NULL;
    
    UA_ModifySubscriptionRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  a_pResponseHeader != UA_NULL
       &&  a_pRevisedPublishingInterval != UA_NULL
       &&  a_pRevisedLifetimeCount != UA_NULL
       &&  a_pRevisedMaxKeepAliveCount != UA_NULL)
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
            &UA_ModifySubscriptionRequest_EncodeableType,
            &UA_ModifySubscriptionResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->typeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((UA_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (UA_ModifySubscriptionResponse_EncodeableType.typeId != pResponseType->typeId)
        {
            pResponseType->clearFunction(pResponse);
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
StatusCode UA_ClientApi_BeginModifySubscription(
    OpcUa_Channel                     a_hChannel,
    const UA_RequestHeader*           a_pRequestHeader,
    uint32_t                          a_nSubscriptionId,
    double                            a_nRequestedPublishingInterval,
    uint32_t                          a_nRequestedLifetimeCount,
    uint32_t                          a_nRequestedMaxKeepAliveCount,
    uint32_t                          a_nMaxNotificationsPerPublish,
    UA_Byte                           a_nPriority,
    UA_Channel_PfnRequestComplete*    a_pCallback,
    OpcUa_Void*                       a_pCallbackData)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_ModifySubscriptionRequest cRequest;
    UA_ModifySubscriptionRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL)
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
            (OpcUa_Void*)&cRequest,
            &UA_ModifySubscriptionRequest_EncodeableType,
            &UA_ModifySubscriptionResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*   )a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_SetPublishingMode
/*============================================================================
 * Synchronously calls the SetPublishingMode service.
 *===========================================================================*/
StatusCode UA_ClientApi_SetPublishingMode(
    OpcUa_Channel           a_hChannel,
    const UA_RequestHeader* a_pRequestHeader,
    UA_Boolean              a_bPublishingEnabled,
    int32_t                 a_nNoOfSubscriptionIds,
    const uint32_t*         a_pSubscriptionIds,
    UA_ResponseHeader*      a_pResponseHeader,
    int32_t*                a_pNoOfResults,
    StatusCode**            a_pResults,
    int32_t*                a_pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**     a_pDiagnosticInfos)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_SetPublishingModeRequest cRequest;
    UA_SetPublishingModeResponse* pResponse = UA_NULL;
    UA_EncodeableType* pResponseType = UA_NULL;
    
    UA_SetPublishingModeRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  (a_pSubscriptionIds != UA_NULL || a_nNoOfSubscriptionIds <= 0)
       &&  a_pResponseHeader != UA_NULL
       &&  a_pNoOfResults != UA_NULL
       &&  a_pResults != UA_NULL
       &&  a_pNoOfDiagnosticInfos != UA_NULL
       &&  a_pDiagnosticInfos != UA_NULL)
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
            &UA_SetPublishingModeRequest_EncodeableType,
            &UA_SetPublishingModeResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->typeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((UA_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (UA_SetPublishingModeResponse_EncodeableType.typeId != pResponseType->typeId)
        {
            pResponseType->clearFunction(pResponse);
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
StatusCode UA_ClientApi_BeginSetPublishingMode(
    OpcUa_Channel                     a_hChannel,
    const UA_RequestHeader*           a_pRequestHeader,
    UA_Boolean                        a_bPublishingEnabled,
    int32_t                           a_nNoOfSubscriptionIds,
    const uint32_t*                   a_pSubscriptionIds,
    UA_Channel_PfnRequestComplete*    a_pCallback,
    OpcUa_Void*                       a_pCallbackData)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_SetPublishingModeRequest cRequest;
    UA_SetPublishingModeRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  (a_pSubscriptionIds != UA_NULL || a_nNoOfSubscriptionIds <= 0))
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
            (OpcUa_Void*)&cRequest,
            &UA_SetPublishingModeRequest_EncodeableType,
            &UA_SetPublishingModeResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*   )a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_Publish
/*============================================================================
 * Synchronously calls the Publish service.
 *===========================================================================*/
StatusCode UA_ClientApi_Publish(
    OpcUa_Channel                         a_hChannel,
    const UA_RequestHeader*               a_pRequestHeader,
    int32_t                               a_nNoOfSubscriptionAcknowledgements,
    const UA_SubscriptionAcknowledgement* a_pSubscriptionAcknowledgements,
    UA_ResponseHeader*                    a_pResponseHeader,
    uint32_t*                             a_pSubscriptionId,
    int32_t*                              a_pNoOfAvailableSequenceNumbers,
    uint32_t**                            a_pAvailableSequenceNumbers,
    UA_Boolean*                           a_pMoreNotifications,
    UA_NotificationMessage*               a_pNotificationMessage,
    int32_t*                              a_pNoOfResults,
    StatusCode**                          a_pResults,
    int32_t*                              a_pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**                   a_pDiagnosticInfos)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_PublishRequest cRequest;
    UA_PublishResponse* pResponse = UA_NULL;
    UA_EncodeableType* pResponseType = UA_NULL;
    
    UA_PublishRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  (a_pSubscriptionAcknowledgements != UA_NULL || a_nNoOfSubscriptionAcknowledgements <= 0)
       &&  a_pResponseHeader != UA_NULL
       &&  a_pSubscriptionId != UA_NULL
       &&  a_pNoOfAvailableSequenceNumbers != UA_NULL
       &&  a_pAvailableSequenceNumbers != UA_NULL
       &&  a_pMoreNotifications != UA_NULL
       &&  a_pNotificationMessage != UA_NULL
       &&  a_pNoOfResults != UA_NULL
       &&  a_pResults != UA_NULL
       &&  a_pNoOfDiagnosticInfos != UA_NULL
       &&  a_pDiagnosticInfos != UA_NULL)
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader                    = *a_pRequestHeader;
        cRequest.NoOfSubscriptionAcknowledgements = a_nNoOfSubscriptionAcknowledgements;
        cRequest.SubscriptionAcknowledgements     = (UA_SubscriptionAcknowledgement*)a_pSubscriptionAcknowledgements;

        /* invoke service */
        status = UA_Channel_InvokeService(
            a_hChannel,
            "Publish",
            (void*)&cRequest,
            &UA_PublishRequest_EncodeableType,
            &UA_PublishResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->typeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((UA_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (UA_PublishResponse_EncodeableType.typeId != pResponseType->typeId)
        {
            pResponseType->clearFunction(pResponse);
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
StatusCode UA_ClientApi_BeginPublish(
    OpcUa_Channel                         a_hChannel,
    const UA_RequestHeader*               a_pRequestHeader,
    int32_t                               a_nNoOfSubscriptionAcknowledgements,
    const UA_SubscriptionAcknowledgement* a_pSubscriptionAcknowledgements,
    UA_Channel_PfnRequestComplete*        a_pCallback,
    OpcUa_Void*                           a_pCallbackData)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_PublishRequest cRequest;
    UA_PublishRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  (a_pSubscriptionAcknowledgements != UA_NULL || a_nNoOfSubscriptionAcknowledgements <= 0))
        status = STATUS_OK;


    if(status == STATUS_OK){
        /* copy parameters into request object. */
        cRequest.RequestHeader                    = *a_pRequestHeader;
        cRequest.NoOfSubscriptionAcknowledgements = a_nNoOfSubscriptionAcknowledgements;
        cRequest.SubscriptionAcknowledgements     = (UA_SubscriptionAcknowledgement*)a_pSubscriptionAcknowledgements;

        /* begin invoke service */
        status = UA_Channel_BeginInvokeService(
            a_hChannel,
            "Publish",
            (OpcUa_Void*)&cRequest,
            &UA_PublishRequest_EncodeableType,
            &UA_PublishResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*   )a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_Republish
/*============================================================================
 * Synchronously calls the Republish service.
 *===========================================================================*/
StatusCode UA_ClientApi_Republish(
    OpcUa_Channel           a_hChannel,
    const UA_RequestHeader* a_pRequestHeader,
    uint32_t                a_nSubscriptionId,
    uint32_t                a_nRetransmitSequenceNumber,
    UA_ResponseHeader*      a_pResponseHeader,
    UA_NotificationMessage* a_pNotificationMessage)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_RepublishRequest cRequest;
    UA_RepublishResponse* pResponse = UA_NULL;
    UA_EncodeableType* pResponseType = UA_NULL;
    
    UA_RepublishRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  a_pResponseHeader != UA_NULL
       &&  a_pNotificationMessage != UA_NULL)
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
            &UA_RepublishRequest_EncodeableType,
            &UA_RepublishResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->typeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((UA_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (UA_RepublishResponse_EncodeableType.typeId != pResponseType->typeId)
        {
            pResponseType->clearFunction(pResponse);
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
StatusCode UA_ClientApi_BeginRepublish(
    OpcUa_Channel                     a_hChannel,
    const UA_RequestHeader*           a_pRequestHeader,
    uint32_t                          a_nSubscriptionId,
    uint32_t                          a_nRetransmitSequenceNumber,
    UA_Channel_PfnRequestComplete*    a_pCallback,
    OpcUa_Void*                       a_pCallbackData)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_RepublishRequest cRequest;
    UA_RepublishRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL)
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
            (OpcUa_Void*)&cRequest,
            &UA_RepublishRequest_EncodeableType,
            &UA_RepublishResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*   )a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_TransferSubscriptions
/*============================================================================
 * Synchronously calls the TransferSubscriptions service.
 *===========================================================================*/
StatusCode UA_ClientApi_TransferSubscriptions(
    OpcUa_Channel           a_hChannel,
    const UA_RequestHeader* a_pRequestHeader,
    int32_t                 a_nNoOfSubscriptionIds,
    const uint32_t*         a_pSubscriptionIds,
    UA_Boolean              a_bSendInitialValues,
    UA_ResponseHeader*      a_pResponseHeader,
    int32_t*                a_pNoOfResults,
    UA_TransferResult**     a_pResults,
    int32_t*                a_pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**     a_pDiagnosticInfos)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_TransferSubscriptionsRequest cRequest;
    UA_TransferSubscriptionsResponse* pResponse = UA_NULL;
    UA_EncodeableType* pResponseType = UA_NULL;
    
    UA_TransferSubscriptionsRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  (a_pSubscriptionIds != UA_NULL || a_nNoOfSubscriptionIds <= 0)
       &&  a_pResponseHeader != UA_NULL
       &&  a_pNoOfResults != UA_NULL
       &&  a_pResults != UA_NULL
       &&  a_pNoOfDiagnosticInfos != UA_NULL
       &&  a_pDiagnosticInfos != UA_NULL)
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
            &UA_TransferSubscriptionsRequest_EncodeableType,
            &UA_TransferSubscriptionsResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->typeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((UA_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (UA_TransferSubscriptionsResponse_EncodeableType.typeId != pResponseType->typeId)
        {
            pResponseType->clearFunction(pResponse);
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
StatusCode UA_ClientApi_BeginTransferSubscriptions(
    OpcUa_Channel                     a_hChannel,
    const UA_RequestHeader*           a_pRequestHeader,
    int32_t                           a_nNoOfSubscriptionIds,
    const uint32_t*                   a_pSubscriptionIds,
    UA_Boolean                        a_bSendInitialValues,
    UA_Channel_PfnRequestComplete*    a_pCallback,
    OpcUa_Void*                       a_pCallbackData)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_TransferSubscriptionsRequest cRequest;
    UA_TransferSubscriptionsRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  (a_pSubscriptionIds != UA_NULL || a_nNoOfSubscriptionIds <= 0))
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
            (OpcUa_Void*)&cRequest,
            &UA_TransferSubscriptionsRequest_EncodeableType,
            &UA_TransferSubscriptionsResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*   )a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#ifndef OPCUA_EXCLUDE_DeleteSubscriptions
/*============================================================================
 * Synchronously calls the DeleteSubscriptions service.
 *===========================================================================*/
StatusCode UA_ClientApi_DeleteSubscriptions(
    OpcUa_Channel           a_hChannel,
    const UA_RequestHeader* a_pRequestHeader,
    int32_t                 a_nNoOfSubscriptionIds,
    const uint32_t*         a_pSubscriptionIds,
    UA_ResponseHeader*      a_pResponseHeader,
    int32_t*                a_pNoOfResults,
    StatusCode**            a_pResults,
    int32_t*                a_pNoOfDiagnosticInfos,
    UA_DiagnosticInfo**     a_pDiagnosticInfos)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_DeleteSubscriptionsRequest cRequest;
    UA_DeleteSubscriptionsResponse* pResponse = UA_NULL;
    UA_EncodeableType* pResponseType = UA_NULL;
    
    UA_DeleteSubscriptionsRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  (a_pSubscriptionIds != UA_NULL || a_nNoOfSubscriptionIds <= 0)
       &&  a_pResponseHeader != UA_NULL
       &&  a_pNoOfResults != UA_NULL
       &&  a_pResults != UA_NULL
       &&  a_pNoOfDiagnosticInfos != UA_NULL
       &&  a_pDiagnosticInfos != UA_NULL)
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
            &UA_DeleteSubscriptionsRequest_EncodeableType,
            &UA_DeleteSubscriptionsResponse_EncodeableType,
            (void**)&pResponse,
            &pResponseType);
    }

    if(status == STATUS_OK){
        /* check for fault */
        if (pResponseType->typeId == OpcUaId_ServiceFault)
        {
            *a_pResponseHeader = ((UA_ServiceFault*)pResponse)->ResponseHeader;
            free(pResponse);
        }

        /* check response type */
        else if (UA_DeleteSubscriptionsResponse_EncodeableType.typeId != pResponseType->typeId)
        {
            pResponseType->clearFunction(pResponse);
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
StatusCode UA_ClientApi_BeginDeleteSubscriptions(
    OpcUa_Channel                     a_hChannel,
    const UA_RequestHeader*           a_pRequestHeader,
    int32_t                           a_nNoOfSubscriptionIds,
    const uint32_t*                   a_pSubscriptionIds,
    UA_Channel_PfnRequestComplete*    a_pCallback,
    OpcUa_Void*                       a_pCallbackData)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_DeleteSubscriptionsRequest cRequest;
    UA_DeleteSubscriptionsRequest_Initialize(&cRequest);

    /* validate arguments. */
    if( a_pRequestHeader != UA_NULL
       &&  (a_pSubscriptionIds != UA_NULL || a_nNoOfSubscriptionIds <= 0))
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
            (OpcUa_Void*)&cRequest,
            &UA_DeleteSubscriptionsRequest_EncodeableType,
            &UA_DeleteSubscriptionsResponse_EncodeableType,
            (UA_Channel_PfnRequestComplete*   )a_pCallback,
            a_pCallbackData);
    }

    return status;
}
#endif

#endif /* OPCUA_HAVE_CLIENTAPI */
/* This is the last line of an autogenerated file. */
