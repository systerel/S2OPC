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

/* base */
#include "sopc_stack_csts.h"

#ifdef OPCUA_HAVE_SERVERAPI

#include <string.h>

/* types */
#include "sopc_types.h"
#include "sopc_builtintypes.h"
#include "sopc_encodeabletype.h"
#include "opcua_identifiers.h"
#include "opcua_statuscodes.h"

/* server related */
#include "sopc_endpoint.h"
#include "sopc_serverapi.h"

#ifndef OPCUA_EXCLUDE_FindServers
/*============================================================================
 * A pointer to a function that implements the FindServers service.
 *===========================================================================*/
typedef SOPC_StatusCode (OpcUa_ServerApi_PfnFindServers)(
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
 * A stub method which implements the FindServers service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_FindServers(
    SOPC_Endpoint                  a_hEndpoint,
    struct SOPC_RequestContext*    a_hContext,
    const OpcUa_RequestHeader*     a_pRequestHeader,
    const SOPC_String*             a_pEndpointUrl,
    int32_t                        a_nNoOfLocaleIds,
    const SOPC_String*             a_pLocaleIds,
    int32_t                        a_nNoOfServerUris,
    const SOPC_String*             a_pServerUris,
    OpcUa_ResponseHeader*          a_pResponseHeader,
    int32_t*                       a_pNoOfServers,
    OpcUa_ApplicationDescription** a_pServers)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;

    /* validate arguments. */
    if(a_hEndpoint != NULL && a_hContext != NULL){
        if( a_pRequestHeader != NULL
           &&  a_pEndpointUrl != NULL
           &&  (a_pLocaleIds != NULL || a_nNoOfLocaleIds <= 0)
           &&  (a_pServerUris != NULL || a_nNoOfServerUris <= 0)
           &&  a_pResponseHeader != NULL
           &&  a_pNoOfServers != NULL
           &&  a_pServers != NULL)
            status = STATUS_OK;

    }

    status = OpcUa_BadNotImplemented;

    return status;
}

/*============================================================================
 * Begins processing of a FindServers service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginFindServers(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void*                       a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_FindServersRequest* pRequest = NULL;
    OpcUa_FindServersResponse pResponse;
    OpcUa_FindServersResponse_Initialize(&pResponse);
    OpcUa_ServerApi_PfnFindServers* pfnInvoke = NULL;

    if(a_hEndpoint != NULL && a_hContext != NULL
       && a_ppRequest != NULL
       && a_pRequestType != NULL){
        status = STATUS_OK;
    }
    
    if(a_pRequestType->TypeId != OpcUaId_FindServersRequest){
        status = OpcUa_BadInvalidArgument;
    }

    if(status == STATUS_OK){
        pRequest = (OpcUa_FindServersRequest*)a_ppRequest;
    }

    if(status == STATUS_OK){
        /* get the function that implements the service call. */
        status = SOPC_Endpoint_GetServiceFunction(a_hEndpoint, a_hContext,
                                                  (SOPC_InvokeService**)&pfnInvoke);
    }

    if(status == STATUS_OK){
    /* invoke the service */
        status = pfnInvoke(
            a_hEndpoint,
            a_hContext,
            &pRequest->RequestHeader,
            &pRequest->EndpointUrl,
            pRequest->NoOfLocaleIds,
            pRequest->LocaleIds,
            pRequest->NoOfServerUris,
            pRequest->ServerUris,
            &pResponse.ResponseHeader,
            &pResponse.NoOfServers,
            &pResponse.Servers);
    }
    
    if (status != STATUS_OK)
    {
        SOPC_StatusCode faultStatus;
        OpcUa_ServiceFault faultObj;
        OpcUa_ServiceFault_Initialize(&faultObj);
        /* create a fault */
        faultStatus = SOPC_ServerApi_CreateFault(&pRequest->RequestHeader,
                                                 status,
                                                 &pResponse.ResponseHeader.ServiceDiagnostics,
                                                 &pResponse.ResponseHeader.NoOfStringTable,
                                                 &pResponse.ResponseHeader.StringTable,
                                                 &faultObj);
        
        if(faultStatus == STATUS_OK){
            /* send the response */
            faultStatus = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                                     &OpcUa_ServiceFault_EncodeableType, &pResponse,
                                                     &a_hContext);
        }
    }

    if(status == STATUS_OK){
        /* send the response */
        status = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                            &OpcUa_FindServersResponse_EncodeableType, &pResponse,
                                            &a_hContext);
    }
    return status;
}

/*============================================================================
 * The service dispatch information FindServers service.
 *===========================================================================*/
struct SOPC_ServiceType OpcUa_FindServers_ServiceType =
{
    OpcUaId_FindServersRequest,
    &OpcUa_FindServersResponse_EncodeableType,
    OpcUa_Server_BeginFindServers,
    (SOPC_InvokeService*)OpcUa_ServerApi_FindServers
};
#endif

#ifndef OPCUA_EXCLUDE_FindServersOnNetwork
/*============================================================================
 * A pointer to a function that implements the FindServersOnNetwork service.
 *===========================================================================*/
typedef SOPC_StatusCode (OpcUa_ServerApi_PfnFindServersOnNetwork)(
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
 * A stub method which implements the FindServersOnNetwork service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_FindServersOnNetwork(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    const OpcUa_RequestHeader*  a_pRequestHeader,
    uint32_t                    a_nStartingRecordId,
    uint32_t                    a_nMaxRecordsToReturn,
    int32_t                     a_nNoOfServerCapabilityFilter,
    const SOPC_String*          a_pServerCapabilityFilter,
    OpcUa_ResponseHeader*       a_pResponseHeader,
    SOPC_DateTime*              a_pLastCounterResetTime,
    int32_t*                    a_pNoOfServers,
    OpcUa_ServerOnNetwork**     a_pServers)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;

    /* validate arguments. */
    if(a_hEndpoint != NULL && a_hContext != NULL){
        if( a_pRequestHeader != NULL
           &&  (a_pServerCapabilityFilter != NULL || a_nNoOfServerCapabilityFilter <= 0)
           &&  a_pResponseHeader != NULL
           &&  a_pLastCounterResetTime != NULL
           &&  a_pNoOfServers != NULL
           &&  a_pServers != NULL)
            status = STATUS_OK;
        (void) a_nStartingRecordId;
        (void) a_nMaxRecordsToReturn;

    }

    status = OpcUa_BadNotImplemented;

    return status;
}

/*============================================================================
 * Begins processing of a FindServersOnNetwork service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginFindServersOnNetwork(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void*                       a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_FindServersOnNetworkRequest* pRequest = NULL;
    OpcUa_FindServersOnNetworkResponse pResponse;
    OpcUa_FindServersOnNetworkResponse_Initialize(&pResponse);
    OpcUa_ServerApi_PfnFindServersOnNetwork* pfnInvoke = NULL;

    if(a_hEndpoint != NULL && a_hContext != NULL
       && a_ppRequest != NULL
       && a_pRequestType != NULL){
        status = STATUS_OK;
    }
    
    if(a_pRequestType->TypeId != OpcUaId_FindServersOnNetworkRequest){
        status = OpcUa_BadInvalidArgument;
    }

    if(status == STATUS_OK){
        pRequest = (OpcUa_FindServersOnNetworkRequest*)a_ppRequest;
    }

    if(status == STATUS_OK){
        /* get the function that implements the service call. */
        status = SOPC_Endpoint_GetServiceFunction(a_hEndpoint, a_hContext,
                                                  (SOPC_InvokeService**)&pfnInvoke);
    }

    if(status == STATUS_OK){
    /* invoke the service */
        status = pfnInvoke(
            a_hEndpoint,
            a_hContext,
            &pRequest->RequestHeader,
            pRequest->StartingRecordId,
            pRequest->MaxRecordsToReturn,
            pRequest->NoOfServerCapabilityFilter,
            pRequest->ServerCapabilityFilter,
            &pResponse.ResponseHeader,
            &pResponse.LastCounterResetTime,
            &pResponse.NoOfServers,
            &pResponse.Servers);
    }
    
    if (status != STATUS_OK)
    {
        SOPC_StatusCode faultStatus;
        OpcUa_ServiceFault faultObj;
        OpcUa_ServiceFault_Initialize(&faultObj);
        /* create a fault */
        faultStatus = SOPC_ServerApi_CreateFault(&pRequest->RequestHeader,
                                                 status,
                                                 &pResponse.ResponseHeader.ServiceDiagnostics,
                                                 &pResponse.ResponseHeader.NoOfStringTable,
                                                 &pResponse.ResponseHeader.StringTable,
                                                 &faultObj);
        
        if(faultStatus == STATUS_OK){
            /* send the response */
            faultStatus = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                                     &OpcUa_ServiceFault_EncodeableType, &pResponse,
                                                     &a_hContext);
        }
    }

    if(status == STATUS_OK){
        /* send the response */
        status = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                            &OpcUa_FindServersOnNetworkResponse_EncodeableType, &pResponse,
                                            &a_hContext);
    }
    return status;
}

/*============================================================================
 * The service dispatch information FindServersOnNetwork service.
 *===========================================================================*/
struct SOPC_ServiceType OpcUa_FindServersOnNetwork_ServiceType =
{
    OpcUaId_FindServersOnNetworkRequest,
    &OpcUa_FindServersOnNetworkResponse_EncodeableType,
    OpcUa_Server_BeginFindServersOnNetwork,
    (SOPC_InvokeService*)OpcUa_ServerApi_FindServersOnNetwork
};
#endif

#ifndef OPCUA_EXCLUDE_GetEndpoints
/*============================================================================
 * A pointer to a function that implements the GetEndpoints service.
 *===========================================================================*/
typedef SOPC_StatusCode (OpcUa_ServerApi_PfnGetEndpoints)(
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
 * A stub method which implements the GetEndpoints service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_GetEndpoints(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    const OpcUa_RequestHeader*  a_pRequestHeader,
    const SOPC_String*          a_pEndpointUrl,
    int32_t                     a_nNoOfLocaleIds,
    const SOPC_String*          a_pLocaleIds,
    int32_t                     a_nNoOfProfileUris,
    const SOPC_String*          a_pProfileUris,
    OpcUa_ResponseHeader*       a_pResponseHeader,
    int32_t*                    a_pNoOfEndpoints,
    OpcUa_EndpointDescription** a_pEndpoints)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;

    /* validate arguments. */
    if(a_hEndpoint != NULL && a_hContext != NULL){
        if( a_pRequestHeader != NULL
           &&  a_pEndpointUrl != NULL
           &&  (a_pLocaleIds != NULL || a_nNoOfLocaleIds <= 0)
           &&  (a_pProfileUris != NULL || a_nNoOfProfileUris <= 0)
           &&  a_pResponseHeader != NULL
           &&  a_pNoOfEndpoints != NULL
           &&  a_pEndpoints != NULL)
            status = STATUS_OK;

    }

    status = OpcUa_BadNotImplemented;

    return status;
}

/*============================================================================
 * Begins processing of a GetEndpoints service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginGetEndpoints(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void*                       a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_GetEndpointsRequest* pRequest = NULL;
    OpcUa_GetEndpointsResponse pResponse;
    OpcUa_GetEndpointsResponse_Initialize(&pResponse);
    OpcUa_ServerApi_PfnGetEndpoints* pfnInvoke = NULL;

    if(a_hEndpoint != NULL && a_hContext != NULL
       && a_ppRequest != NULL
       && a_pRequestType != NULL){
        status = STATUS_OK;
    }
    
    if(a_pRequestType->TypeId != OpcUaId_GetEndpointsRequest){
        status = OpcUa_BadInvalidArgument;
    }

    if(status == STATUS_OK){
        pRequest = (OpcUa_GetEndpointsRequest*)a_ppRequest;
    }

    if(status == STATUS_OK){
        /* get the function that implements the service call. */
        status = SOPC_Endpoint_GetServiceFunction(a_hEndpoint, a_hContext,
                                                  (SOPC_InvokeService**)&pfnInvoke);
    }

    if(status == STATUS_OK){
    /* invoke the service */
        status = pfnInvoke(
            a_hEndpoint,
            a_hContext,
            &pRequest->RequestHeader,
            &pRequest->EndpointUrl,
            pRequest->NoOfLocaleIds,
            pRequest->LocaleIds,
            pRequest->NoOfProfileUris,
            pRequest->ProfileUris,
            &pResponse.ResponseHeader,
            &pResponse.NoOfEndpoints,
            &pResponse.Endpoints);
    }
    
    if (status != STATUS_OK)
    {
        SOPC_StatusCode faultStatus;
        OpcUa_ServiceFault faultObj;
        OpcUa_ServiceFault_Initialize(&faultObj);
        /* create a fault */
        faultStatus = SOPC_ServerApi_CreateFault(&pRequest->RequestHeader,
                                                 status,
                                                 &pResponse.ResponseHeader.ServiceDiagnostics,
                                                 &pResponse.ResponseHeader.NoOfStringTable,
                                                 &pResponse.ResponseHeader.StringTable,
                                                 &faultObj);
        
        if(faultStatus == STATUS_OK){
            /* send the response */
            faultStatus = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                                     &OpcUa_ServiceFault_EncodeableType, &pResponse,
                                                     &a_hContext);
        }
    }

    if(status == STATUS_OK){
        /* send the response */
        status = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                            &OpcUa_GetEndpointsResponse_EncodeableType, &pResponse,
                                            &a_hContext);
    }
    return status;
}

/*============================================================================
 * The service dispatch information GetEndpoints service.
 *===========================================================================*/
struct SOPC_ServiceType OpcUa_GetEndpoints_ServiceType =
{
    OpcUaId_GetEndpointsRequest,
    &OpcUa_GetEndpointsResponse_EncodeableType,
    OpcUa_Server_BeginGetEndpoints,
    (SOPC_InvokeService*)OpcUa_ServerApi_GetEndpoints
};
#endif

#ifndef OPCUA_EXCLUDE_RegisterServer
/*============================================================================
 * A pointer to a function that implements the RegisterServer service.
 *===========================================================================*/
typedef SOPC_StatusCode (OpcUa_ServerApi_PfnRegisterServer)(
    SOPC_Endpoint                 hEndpoint,
    struct SOPC_RequestContext*   hContext,
    const OpcUa_RequestHeader*    pRequestHeader,
    const OpcUa_RegisteredServer* pServer,
    OpcUa_ResponseHeader*         pResponseHeader);

/*============================================================================
 * A stub method which implements the RegisterServer service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_RegisterServer(
    SOPC_Endpoint                 a_hEndpoint,
    struct SOPC_RequestContext*   a_hContext,
    const OpcUa_RequestHeader*    a_pRequestHeader,
    const OpcUa_RegisteredServer* a_pServer,
    OpcUa_ResponseHeader*         a_pResponseHeader)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;

    /* validate arguments. */
    if(a_hEndpoint != NULL && a_hContext != NULL){
        if( a_pRequestHeader != NULL
           &&  a_pServer != NULL
           &&  a_pResponseHeader != NULL)
            status = STATUS_OK;

    }

    status = OpcUa_BadNotImplemented;

    return status;
}

/*============================================================================
 * Begins processing of a RegisterServer service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginRegisterServer(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void*                       a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_RegisterServerRequest* pRequest = NULL;
    OpcUa_RegisterServerResponse pResponse;
    OpcUa_RegisterServerResponse_Initialize(&pResponse);
    OpcUa_ServerApi_PfnRegisterServer* pfnInvoke = NULL;

    if(a_hEndpoint != NULL && a_hContext != NULL
       && a_ppRequest != NULL
       && a_pRequestType != NULL){
        status = STATUS_OK;
    }
    
    if(a_pRequestType->TypeId != OpcUaId_RegisterServerRequest){
        status = OpcUa_BadInvalidArgument;
    }

    if(status == STATUS_OK){
        pRequest = (OpcUa_RegisterServerRequest*)a_ppRequest;
    }

    if(status == STATUS_OK){
        /* get the function that implements the service call. */
        status = SOPC_Endpoint_GetServiceFunction(a_hEndpoint, a_hContext,
                                                  (SOPC_InvokeService**)&pfnInvoke);
    }

    if(status == STATUS_OK){
    /* invoke the service */
        status = pfnInvoke(
            a_hEndpoint,
            a_hContext,
            &pRequest->RequestHeader,
            &pRequest->Server,
            &pResponse.ResponseHeader);
    }
    
    if (status != STATUS_OK)
    {
        SOPC_StatusCode faultStatus;
        OpcUa_ServiceFault faultObj;
        OpcUa_ServiceFault_Initialize(&faultObj);
        /* create a fault */
        faultStatus = SOPC_ServerApi_CreateFault(&pRequest->RequestHeader,
                                                 status,
                                                 &pResponse.ResponseHeader.ServiceDiagnostics,
                                                 &pResponse.ResponseHeader.NoOfStringTable,
                                                 &pResponse.ResponseHeader.StringTable,
                                                 &faultObj);
        
        if(faultStatus == STATUS_OK){
            /* send the response */
            faultStatus = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                                     &OpcUa_ServiceFault_EncodeableType, &pResponse,
                                                     &a_hContext);
        }
    }

    if(status == STATUS_OK){
        /* send the response */
        status = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                            &OpcUa_RegisterServerResponse_EncodeableType, &pResponse,
                                            &a_hContext);
    }
    return status;
}

/*============================================================================
 * The service dispatch information RegisterServer service.
 *===========================================================================*/
struct SOPC_ServiceType OpcUa_RegisterServer_ServiceType =
{
    OpcUaId_RegisterServerRequest,
    &OpcUa_RegisterServerResponse_EncodeableType,
    OpcUa_Server_BeginRegisterServer,
    (SOPC_InvokeService*)OpcUa_ServerApi_RegisterServer
};
#endif

#ifndef OPCUA_EXCLUDE_RegisterServer2
/*============================================================================
 * A pointer to a function that implements the RegisterServer2 service.
 *===========================================================================*/
typedef SOPC_StatusCode (OpcUa_ServerApi_PfnRegisterServer2)(
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
 * A stub method which implements the RegisterServer2 service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_RegisterServer2(
    SOPC_Endpoint                 a_hEndpoint,
    struct SOPC_RequestContext*   a_hContext,
    const OpcUa_RequestHeader*    a_pRequestHeader,
    const OpcUa_RegisteredServer* a_pServer,
    int32_t                       a_nNoOfDiscoveryConfiguration,
    const SOPC_ExtensionObject*   a_pDiscoveryConfiguration,
    OpcUa_ResponseHeader*         a_pResponseHeader,
    int32_t*                      a_pNoOfConfigurationResults,
    SOPC_StatusCode**             a_pConfigurationResults,
    int32_t*                      a_pNoOfDiagnosticInfos,
    SOPC_DiagnosticInfo**         a_pDiagnosticInfos)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;

    /* validate arguments. */
    if(a_hEndpoint != NULL && a_hContext != NULL){
        if( a_pRequestHeader != NULL
           &&  a_pServer != NULL
           &&  (a_pDiscoveryConfiguration != NULL || a_nNoOfDiscoveryConfiguration <= 0)
           &&  a_pResponseHeader != NULL
           &&  a_pNoOfConfigurationResults != NULL
           &&  a_pConfigurationResults != NULL
           &&  a_pNoOfDiagnosticInfos != NULL
           &&  a_pDiagnosticInfos != NULL)
            status = STATUS_OK;

    }

    status = OpcUa_BadNotImplemented;

    return status;
}

/*============================================================================
 * Begins processing of a RegisterServer2 service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginRegisterServer2(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void*                       a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_RegisterServer2Request* pRequest = NULL;
    OpcUa_RegisterServer2Response pResponse;
    OpcUa_RegisterServer2Response_Initialize(&pResponse);
    OpcUa_ServerApi_PfnRegisterServer2* pfnInvoke = NULL;

    if(a_hEndpoint != NULL && a_hContext != NULL
       && a_ppRequest != NULL
       && a_pRequestType != NULL){
        status = STATUS_OK;
    }
    
    if(a_pRequestType->TypeId != OpcUaId_RegisterServer2Request){
        status = OpcUa_BadInvalidArgument;
    }

    if(status == STATUS_OK){
        pRequest = (OpcUa_RegisterServer2Request*)a_ppRequest;
    }

    if(status == STATUS_OK){
        /* get the function that implements the service call. */
        status = SOPC_Endpoint_GetServiceFunction(a_hEndpoint, a_hContext,
                                                  (SOPC_InvokeService**)&pfnInvoke);
    }

    if(status == STATUS_OK){
    /* invoke the service */
        status = pfnInvoke(
            a_hEndpoint,
            a_hContext,
            &pRequest->RequestHeader,
            &pRequest->Server,
            pRequest->NoOfDiscoveryConfiguration,
            pRequest->DiscoveryConfiguration,
            &pResponse.ResponseHeader,
            &pResponse.NoOfConfigurationResults,
            &pResponse.ConfigurationResults,
            &pResponse.NoOfDiagnosticInfos,
            &pResponse.DiagnosticInfos);
    }
    
    if (status != STATUS_OK)
    {
        SOPC_StatusCode faultStatus;
        OpcUa_ServiceFault faultObj;
        OpcUa_ServiceFault_Initialize(&faultObj);
        /* create a fault */
        faultStatus = SOPC_ServerApi_CreateFault(&pRequest->RequestHeader,
                                                 status,
                                                 &pResponse.ResponseHeader.ServiceDiagnostics,
                                                 &pResponse.ResponseHeader.NoOfStringTable,
                                                 &pResponse.ResponseHeader.StringTable,
                                                 &faultObj);
        
        if(faultStatus == STATUS_OK){
            /* send the response */
            faultStatus = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                                     &OpcUa_ServiceFault_EncodeableType, &pResponse,
                                                     &a_hContext);
        }
    }

    if(status == STATUS_OK){
        /* send the response */
        status = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                            &OpcUa_RegisterServer2Response_EncodeableType, &pResponse,
                                            &a_hContext);
    }
    return status;
}

/*============================================================================
 * The service dispatch information RegisterServer2 service.
 *===========================================================================*/
struct SOPC_ServiceType OpcUa_RegisterServer2_ServiceType =
{
    OpcUaId_RegisterServer2Request,
    &OpcUa_RegisterServer2Response_EncodeableType,
    OpcUa_Server_BeginRegisterServer2,
    (SOPC_InvokeService*)OpcUa_ServerApi_RegisterServer2
};
#endif

#ifndef OPCUA_EXCLUDE_CreateSession
/*============================================================================
 * A pointer to a function that implements the CreateSession service.
 *===========================================================================*/
typedef SOPC_StatusCode (OpcUa_ServerApi_PfnCreateSession)(
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
 * A stub method which implements the CreateSession service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_CreateSession(
    SOPC_Endpoint                       a_hEndpoint,
    struct SOPC_RequestContext*         a_hContext,
    const OpcUa_RequestHeader*          a_pRequestHeader,
    const OpcUa_ApplicationDescription* a_pClientDescription,
    const SOPC_String*                  a_pServerUri,
    const SOPC_String*                  a_pEndpointUrl,
    const SOPC_String*                  a_pSessionName,
    const SOPC_ByteString*              a_pClientNonce,
    const SOPC_ByteString*              a_pClientCertificate,
    double                              a_nRequestedSessionTimeout,
    uint32_t                            a_nMaxResponseMessageSize,
    OpcUa_ResponseHeader*               a_pResponseHeader,
    SOPC_NodeId*                        a_pSessionId,
    SOPC_NodeId*                        a_pAuthenticationToken,
    double*                             a_pRevisedSessionTimeout,
    SOPC_ByteString*                    a_pServerNonce,
    SOPC_ByteString*                    a_pServerCertificate,
    int32_t*                            a_pNoOfServerEndpoints,
    OpcUa_EndpointDescription**         a_pServerEndpoints,
    int32_t*                            a_pNoOfServerSoftwareCertificates,
    OpcUa_SignedSoftwareCertificate**   a_pServerSoftwareCertificates,
    OpcUa_SignatureData*                a_pServerSignature,
    uint32_t*                           a_pMaxRequestMessageSize)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;

    /* validate arguments. */
    if(a_hEndpoint != NULL && a_hContext != NULL){
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
        (void) a_nRequestedSessionTimeout;
        (void) a_nMaxResponseMessageSize;

    }

    status = OpcUa_BadNotImplemented;

    return status;
}

/*============================================================================
 * Begins processing of a CreateSession service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginCreateSession(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void*                       a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_CreateSessionRequest* pRequest = NULL;
    OpcUa_CreateSessionResponse pResponse;
    OpcUa_CreateSessionResponse_Initialize(&pResponse);
    OpcUa_ServerApi_PfnCreateSession* pfnInvoke = NULL;

    if(a_hEndpoint != NULL && a_hContext != NULL
       && a_ppRequest != NULL
       && a_pRequestType != NULL){
        status = STATUS_OK;
    }
    
    if(a_pRequestType->TypeId != OpcUaId_CreateSessionRequest){
        status = OpcUa_BadInvalidArgument;
    }

    if(status == STATUS_OK){
        pRequest = (OpcUa_CreateSessionRequest*)a_ppRequest;
    }

    if(status == STATUS_OK){
        /* get the function that implements the service call. */
        status = SOPC_Endpoint_GetServiceFunction(a_hEndpoint, a_hContext,
                                                  (SOPC_InvokeService**)&pfnInvoke);
    }

    if(status == STATUS_OK){
    /* invoke the service */
        status = pfnInvoke(
            a_hEndpoint,
            a_hContext,
            &pRequest->RequestHeader,
            &pRequest->ClientDescription,
            &pRequest->ServerUri,
            &pRequest->EndpointUrl,
            &pRequest->SessionName,
            &pRequest->ClientNonce,
            &pRequest->ClientCertificate,
            pRequest->RequestedSessionTimeout,
            pRequest->MaxResponseMessageSize,
            &pResponse.ResponseHeader,
            &pResponse.SessionId,
            &pResponse.AuthenticationToken,
            &pResponse.RevisedSessionTimeout,
            &pResponse.ServerNonce,
            &pResponse.ServerCertificate,
            &pResponse.NoOfServerEndpoints,
            &pResponse.ServerEndpoints,
            &pResponse.NoOfServerSoftwareCertificates,
            &pResponse.ServerSoftwareCertificates,
            &pResponse.ServerSignature,
            &pResponse.MaxRequestMessageSize);
    }
    
    if (status != STATUS_OK)
    {
        SOPC_StatusCode faultStatus;
        OpcUa_ServiceFault faultObj;
        OpcUa_ServiceFault_Initialize(&faultObj);
        /* create a fault */
        faultStatus = SOPC_ServerApi_CreateFault(&pRequest->RequestHeader,
                                                 status,
                                                 &pResponse.ResponseHeader.ServiceDiagnostics,
                                                 &pResponse.ResponseHeader.NoOfStringTable,
                                                 &pResponse.ResponseHeader.StringTable,
                                                 &faultObj);
        
        if(faultStatus == STATUS_OK){
            /* send the response */
            faultStatus = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                                     &OpcUa_ServiceFault_EncodeableType, &pResponse,
                                                     &a_hContext);
        }
    }

    if(status == STATUS_OK){
        /* send the response */
        status = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                            &OpcUa_CreateSessionResponse_EncodeableType, &pResponse,
                                            &a_hContext);
    }
    return status;
}

/*============================================================================
 * The service dispatch information CreateSession service.
 *===========================================================================*/
struct SOPC_ServiceType OpcUa_CreateSession_ServiceType =
{
    OpcUaId_CreateSessionRequest,
    &OpcUa_CreateSessionResponse_EncodeableType,
    OpcUa_Server_BeginCreateSession,
    (SOPC_InvokeService*)OpcUa_ServerApi_CreateSession
};
#endif

#ifndef OPCUA_EXCLUDE_ActivateSession
/*============================================================================
 * A pointer to a function that implements the ActivateSession service.
 *===========================================================================*/
typedef SOPC_StatusCode (OpcUa_ServerApi_PfnActivateSession)(
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
 * A stub method which implements the ActivateSession service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_ActivateSession(
    SOPC_Endpoint                          a_hEndpoint,
    struct SOPC_RequestContext*            a_hContext,
    const OpcUa_RequestHeader*             a_pRequestHeader,
    const OpcUa_SignatureData*             a_pClientSignature,
    int32_t                                a_nNoOfClientSoftwareCertificates,
    const OpcUa_SignedSoftwareCertificate* a_pClientSoftwareCertificates,
    int32_t                                a_nNoOfLocaleIds,
    const SOPC_String*                     a_pLocaleIds,
    const SOPC_ExtensionObject*            a_pUserIdentityToken,
    const OpcUa_SignatureData*             a_pUserTokenSignature,
    OpcUa_ResponseHeader*                  a_pResponseHeader,
    SOPC_ByteString*                       a_pServerNonce,
    int32_t*                               a_pNoOfResults,
    SOPC_StatusCode**                      a_pResults,
    int32_t*                               a_pNoOfDiagnosticInfos,
    SOPC_DiagnosticInfo**                  a_pDiagnosticInfos)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;

    /* validate arguments. */
    if(a_hEndpoint != NULL && a_hContext != NULL){
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

    }

    status = OpcUa_BadNotImplemented;

    return status;
}

/*============================================================================
 * Begins processing of a ActivateSession service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginActivateSession(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void*                       a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_ActivateSessionRequest* pRequest = NULL;
    OpcUa_ActivateSessionResponse pResponse;
    OpcUa_ActivateSessionResponse_Initialize(&pResponse);
    OpcUa_ServerApi_PfnActivateSession* pfnInvoke = NULL;

    if(a_hEndpoint != NULL && a_hContext != NULL
       && a_ppRequest != NULL
       && a_pRequestType != NULL){
        status = STATUS_OK;
    }
    
    if(a_pRequestType->TypeId != OpcUaId_ActivateSessionRequest){
        status = OpcUa_BadInvalidArgument;
    }

    if(status == STATUS_OK){
        pRequest = (OpcUa_ActivateSessionRequest*)a_ppRequest;
    }

    if(status == STATUS_OK){
        /* get the function that implements the service call. */
        status = SOPC_Endpoint_GetServiceFunction(a_hEndpoint, a_hContext,
                                                  (SOPC_InvokeService**)&pfnInvoke);
    }

    if(status == STATUS_OK){
    /* invoke the service */
        status = pfnInvoke(
            a_hEndpoint,
            a_hContext,
            &pRequest->RequestHeader,
            &pRequest->ClientSignature,
            pRequest->NoOfClientSoftwareCertificates,
            pRequest->ClientSoftwareCertificates,
            pRequest->NoOfLocaleIds,
            pRequest->LocaleIds,
            &pRequest->UserIdentityToken,
            &pRequest->UserTokenSignature,
            &pResponse.ResponseHeader,
            &pResponse.ServerNonce,
            &pResponse.NoOfResults,
            &pResponse.Results,
            &pResponse.NoOfDiagnosticInfos,
            &pResponse.DiagnosticInfos);
    }
    
    if (status != STATUS_OK)
    {
        SOPC_StatusCode faultStatus;
        OpcUa_ServiceFault faultObj;
        OpcUa_ServiceFault_Initialize(&faultObj);
        /* create a fault */
        faultStatus = SOPC_ServerApi_CreateFault(&pRequest->RequestHeader,
                                                 status,
                                                 &pResponse.ResponseHeader.ServiceDiagnostics,
                                                 &pResponse.ResponseHeader.NoOfStringTable,
                                                 &pResponse.ResponseHeader.StringTable,
                                                 &faultObj);
        
        if(faultStatus == STATUS_OK){
            /* send the response */
            faultStatus = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                                     &OpcUa_ServiceFault_EncodeableType, &pResponse,
                                                     &a_hContext);
        }
    }

    if(status == STATUS_OK){
        /* send the response */
        status = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                            &OpcUa_ActivateSessionResponse_EncodeableType, &pResponse,
                                            &a_hContext);
    }
    return status;
}

/*============================================================================
 * The service dispatch information ActivateSession service.
 *===========================================================================*/
struct SOPC_ServiceType OpcUa_ActivateSession_ServiceType =
{
    OpcUaId_ActivateSessionRequest,
    &OpcUa_ActivateSessionResponse_EncodeableType,
    OpcUa_Server_BeginActivateSession,
    (SOPC_InvokeService*)OpcUa_ServerApi_ActivateSession
};
#endif

#ifndef OPCUA_EXCLUDE_CloseSession
/*============================================================================
 * A pointer to a function that implements the CloseSession service.
 *===========================================================================*/
typedef SOPC_StatusCode (OpcUa_ServerApi_PfnCloseSession)(
    SOPC_Endpoint               hEndpoint,
    struct SOPC_RequestContext* hContext,
    const OpcUa_RequestHeader*  pRequestHeader,
    SOPC_Boolean                bDeleteSubscriptions,
    OpcUa_ResponseHeader*       pResponseHeader);

/*============================================================================
 * A stub method which implements the CloseSession service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_CloseSession(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    const OpcUa_RequestHeader*  a_pRequestHeader,
    SOPC_Boolean                a_bDeleteSubscriptions,
    OpcUa_ResponseHeader*       a_pResponseHeader)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;

    /* validate arguments. */
    if(a_hEndpoint != NULL && a_hContext != NULL){
        if( a_pRequestHeader != NULL
           &&  a_pResponseHeader != NULL)
            status = STATUS_OK;
        (void) a_bDeleteSubscriptions;

    }

    status = OpcUa_BadNotImplemented;

    return status;
}

/*============================================================================
 * Begins processing of a CloseSession service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginCloseSession(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void*                       a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_CloseSessionRequest* pRequest = NULL;
    OpcUa_CloseSessionResponse pResponse;
    OpcUa_CloseSessionResponse_Initialize(&pResponse);
    OpcUa_ServerApi_PfnCloseSession* pfnInvoke = NULL;

    if(a_hEndpoint != NULL && a_hContext != NULL
       && a_ppRequest != NULL
       && a_pRequestType != NULL){
        status = STATUS_OK;
    }
    
    if(a_pRequestType->TypeId != OpcUaId_CloseSessionRequest){
        status = OpcUa_BadInvalidArgument;
    }

    if(status == STATUS_OK){
        pRequest = (OpcUa_CloseSessionRequest*)a_ppRequest;
    }

    if(status == STATUS_OK){
        /* get the function that implements the service call. */
        status = SOPC_Endpoint_GetServiceFunction(a_hEndpoint, a_hContext,
                                                  (SOPC_InvokeService**)&pfnInvoke);
    }

    if(status == STATUS_OK){
    /* invoke the service */
        status = pfnInvoke(
            a_hEndpoint,
            a_hContext,
            &pRequest->RequestHeader,
            pRequest->DeleteSubscriptions,
            &pResponse.ResponseHeader);
    }
    
    if (status != STATUS_OK)
    {
        SOPC_StatusCode faultStatus;
        OpcUa_ServiceFault faultObj;
        OpcUa_ServiceFault_Initialize(&faultObj);
        /* create a fault */
        faultStatus = SOPC_ServerApi_CreateFault(&pRequest->RequestHeader,
                                                 status,
                                                 &pResponse.ResponseHeader.ServiceDiagnostics,
                                                 &pResponse.ResponseHeader.NoOfStringTable,
                                                 &pResponse.ResponseHeader.StringTable,
                                                 &faultObj);
        
        if(faultStatus == STATUS_OK){
            /* send the response */
            faultStatus = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                                     &OpcUa_ServiceFault_EncodeableType, &pResponse,
                                                     &a_hContext);
        }
    }

    if(status == STATUS_OK){
        /* send the response */
        status = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                            &OpcUa_CloseSessionResponse_EncodeableType, &pResponse,
                                            &a_hContext);
    }
    return status;
}

/*============================================================================
 * The service dispatch information CloseSession service.
 *===========================================================================*/
struct SOPC_ServiceType OpcUa_CloseSession_ServiceType =
{
    OpcUaId_CloseSessionRequest,
    &OpcUa_CloseSessionResponse_EncodeableType,
    OpcUa_Server_BeginCloseSession,
    (SOPC_InvokeService*)OpcUa_ServerApi_CloseSession
};
#endif

#ifndef OPCUA_EXCLUDE_Cancel
/*============================================================================
 * A pointer to a function that implements the Cancel service.
 *===========================================================================*/
typedef SOPC_StatusCode (OpcUa_ServerApi_PfnCancel)(
    SOPC_Endpoint               hEndpoint,
    struct SOPC_RequestContext* hContext,
    const OpcUa_RequestHeader*  pRequestHeader,
    uint32_t                    nRequestHandle,
    OpcUa_ResponseHeader*       pResponseHeader,
    uint32_t*                   pCancelCount);

/*============================================================================
 * A stub method which implements the Cancel service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_Cancel(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    const OpcUa_RequestHeader*  a_pRequestHeader,
    uint32_t                    a_nRequestHandle,
    OpcUa_ResponseHeader*       a_pResponseHeader,
    uint32_t*                   a_pCancelCount)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;

    /* validate arguments. */
    if(a_hEndpoint != NULL && a_hContext != NULL){
        if( a_pRequestHeader != NULL
           &&  a_pResponseHeader != NULL
           &&  a_pCancelCount != NULL)
            status = STATUS_OK;
        (void) a_nRequestHandle;

    }

    status = OpcUa_BadNotImplemented;

    return status;
}

/*============================================================================
 * Begins processing of a Cancel service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginCancel(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void*                       a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_CancelRequest* pRequest = NULL;
    OpcUa_CancelResponse pResponse;
    OpcUa_CancelResponse_Initialize(&pResponse);
    OpcUa_ServerApi_PfnCancel* pfnInvoke = NULL;

    if(a_hEndpoint != NULL && a_hContext != NULL
       && a_ppRequest != NULL
       && a_pRequestType != NULL){
        status = STATUS_OK;
    }
    
    if(a_pRequestType->TypeId != OpcUaId_CancelRequest){
        status = OpcUa_BadInvalidArgument;
    }

    if(status == STATUS_OK){
        pRequest = (OpcUa_CancelRequest*)a_ppRequest;
    }

    if(status == STATUS_OK){
        /* get the function that implements the service call. */
        status = SOPC_Endpoint_GetServiceFunction(a_hEndpoint, a_hContext,
                                                  (SOPC_InvokeService**)&pfnInvoke);
    }

    if(status == STATUS_OK){
    /* invoke the service */
        status = pfnInvoke(
            a_hEndpoint,
            a_hContext,
            &pRequest->RequestHeader,
            pRequest->RequestHandle,
            &pResponse.ResponseHeader,
            &pResponse.CancelCount);
    }
    
    if (status != STATUS_OK)
    {
        SOPC_StatusCode faultStatus;
        OpcUa_ServiceFault faultObj;
        OpcUa_ServiceFault_Initialize(&faultObj);
        /* create a fault */
        faultStatus = SOPC_ServerApi_CreateFault(&pRequest->RequestHeader,
                                                 status,
                                                 &pResponse.ResponseHeader.ServiceDiagnostics,
                                                 &pResponse.ResponseHeader.NoOfStringTable,
                                                 &pResponse.ResponseHeader.StringTable,
                                                 &faultObj);
        
        if(faultStatus == STATUS_OK){
            /* send the response */
            faultStatus = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                                     &OpcUa_ServiceFault_EncodeableType, &pResponse,
                                                     &a_hContext);
        }
    }

    if(status == STATUS_OK){
        /* send the response */
        status = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                            &OpcUa_CancelResponse_EncodeableType, &pResponse,
                                            &a_hContext);
    }
    return status;
}

/*============================================================================
 * The service dispatch information Cancel service.
 *===========================================================================*/
struct SOPC_ServiceType OpcUa_Cancel_ServiceType =
{
    OpcUaId_CancelRequest,
    &OpcUa_CancelResponse_EncodeableType,
    OpcUa_Server_BeginCancel,
    (SOPC_InvokeService*)OpcUa_ServerApi_Cancel
};
#endif

#ifndef OPCUA_EXCLUDE_AddNodes
/*============================================================================
 * A pointer to a function that implements the AddNodes service.
 *===========================================================================*/
typedef SOPC_StatusCode (OpcUa_ServerApi_PfnAddNodes)(
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
 * A stub method which implements the AddNodes service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_AddNodes(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    const OpcUa_RequestHeader*  a_pRequestHeader,
    int32_t                     a_nNoOfNodesToAdd,
    const OpcUa_AddNodesItem*   a_pNodesToAdd,
    OpcUa_ResponseHeader*       a_pResponseHeader,
    int32_t*                    a_pNoOfResults,
    OpcUa_AddNodesResult**      a_pResults,
    int32_t*                    a_pNoOfDiagnosticInfos,
    SOPC_DiagnosticInfo**       a_pDiagnosticInfos)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;

    /* validate arguments. */
    if(a_hEndpoint != NULL && a_hContext != NULL){
        if( a_pRequestHeader != NULL
           &&  (a_pNodesToAdd != NULL || a_nNoOfNodesToAdd <= 0)
           &&  a_pResponseHeader != NULL
           &&  a_pNoOfResults != NULL
           &&  a_pResults != NULL
           &&  a_pNoOfDiagnosticInfos != NULL
           &&  a_pDiagnosticInfos != NULL)
            status = STATUS_OK;

    }

    status = OpcUa_BadNotImplemented;

    return status;
}

/*============================================================================
 * Begins processing of a AddNodes service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginAddNodes(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void*                       a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_AddNodesRequest* pRequest = NULL;
    OpcUa_AddNodesResponse pResponse;
    OpcUa_AddNodesResponse_Initialize(&pResponse);
    OpcUa_ServerApi_PfnAddNodes* pfnInvoke = NULL;

    if(a_hEndpoint != NULL && a_hContext != NULL
       && a_ppRequest != NULL
       && a_pRequestType != NULL){
        status = STATUS_OK;
    }
    
    if(a_pRequestType->TypeId != OpcUaId_AddNodesRequest){
        status = OpcUa_BadInvalidArgument;
    }

    if(status == STATUS_OK){
        pRequest = (OpcUa_AddNodesRequest*)a_ppRequest;
    }

    if(status == STATUS_OK){
        /* get the function that implements the service call. */
        status = SOPC_Endpoint_GetServiceFunction(a_hEndpoint, a_hContext,
                                                  (SOPC_InvokeService**)&pfnInvoke);
    }

    if(status == STATUS_OK){
    /* invoke the service */
        status = pfnInvoke(
            a_hEndpoint,
            a_hContext,
            &pRequest->RequestHeader,
            pRequest->NoOfNodesToAdd,
            pRequest->NodesToAdd,
            &pResponse.ResponseHeader,
            &pResponse.NoOfResults,
            &pResponse.Results,
            &pResponse.NoOfDiagnosticInfos,
            &pResponse.DiagnosticInfos);
    }
    
    if (status != STATUS_OK)
    {
        SOPC_StatusCode faultStatus;
        OpcUa_ServiceFault faultObj;
        OpcUa_ServiceFault_Initialize(&faultObj);
        /* create a fault */
        faultStatus = SOPC_ServerApi_CreateFault(&pRequest->RequestHeader,
                                                 status,
                                                 &pResponse.ResponseHeader.ServiceDiagnostics,
                                                 &pResponse.ResponseHeader.NoOfStringTable,
                                                 &pResponse.ResponseHeader.StringTable,
                                                 &faultObj);
        
        if(faultStatus == STATUS_OK){
            /* send the response */
            faultStatus = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                                     &OpcUa_ServiceFault_EncodeableType, &pResponse,
                                                     &a_hContext);
        }
    }

    if(status == STATUS_OK){
        /* send the response */
        status = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                            &OpcUa_AddNodesResponse_EncodeableType, &pResponse,
                                            &a_hContext);
    }
    return status;
}

/*============================================================================
 * The service dispatch information AddNodes service.
 *===========================================================================*/
struct SOPC_ServiceType OpcUa_AddNodes_ServiceType =
{
    OpcUaId_AddNodesRequest,
    &OpcUa_AddNodesResponse_EncodeableType,
    OpcUa_Server_BeginAddNodes,
    (SOPC_InvokeService*)OpcUa_ServerApi_AddNodes
};
#endif

#ifndef OPCUA_EXCLUDE_AddReferences
/*============================================================================
 * A pointer to a function that implements the AddReferences service.
 *===========================================================================*/
typedef SOPC_StatusCode (OpcUa_ServerApi_PfnAddReferences)(
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
 * A stub method which implements the AddReferences service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_AddReferences(
    SOPC_Endpoint                  a_hEndpoint,
    struct SOPC_RequestContext*    a_hContext,
    const OpcUa_RequestHeader*     a_pRequestHeader,
    int32_t                        a_nNoOfReferencesToAdd,
    const OpcUa_AddReferencesItem* a_pReferencesToAdd,
    OpcUa_ResponseHeader*          a_pResponseHeader,
    int32_t*                       a_pNoOfResults,
    SOPC_StatusCode**              a_pResults,
    int32_t*                       a_pNoOfDiagnosticInfos,
    SOPC_DiagnosticInfo**          a_pDiagnosticInfos)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;

    /* validate arguments. */
    if(a_hEndpoint != NULL && a_hContext != NULL){
        if( a_pRequestHeader != NULL
           &&  (a_pReferencesToAdd != NULL || a_nNoOfReferencesToAdd <= 0)
           &&  a_pResponseHeader != NULL
           &&  a_pNoOfResults != NULL
           &&  a_pResults != NULL
           &&  a_pNoOfDiagnosticInfos != NULL
           &&  a_pDiagnosticInfos != NULL)
            status = STATUS_OK;

    }

    status = OpcUa_BadNotImplemented;

    return status;
}

/*============================================================================
 * Begins processing of a AddReferences service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginAddReferences(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void*                       a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_AddReferencesRequest* pRequest = NULL;
    OpcUa_AddReferencesResponse pResponse;
    OpcUa_AddReferencesResponse_Initialize(&pResponse);
    OpcUa_ServerApi_PfnAddReferences* pfnInvoke = NULL;

    if(a_hEndpoint != NULL && a_hContext != NULL
       && a_ppRequest != NULL
       && a_pRequestType != NULL){
        status = STATUS_OK;
    }
    
    if(a_pRequestType->TypeId != OpcUaId_AddReferencesRequest){
        status = OpcUa_BadInvalidArgument;
    }

    if(status == STATUS_OK){
        pRequest = (OpcUa_AddReferencesRequest*)a_ppRequest;
    }

    if(status == STATUS_OK){
        /* get the function that implements the service call. */
        status = SOPC_Endpoint_GetServiceFunction(a_hEndpoint, a_hContext,
                                                  (SOPC_InvokeService**)&pfnInvoke);
    }

    if(status == STATUS_OK){
    /* invoke the service */
        status = pfnInvoke(
            a_hEndpoint,
            a_hContext,
            &pRequest->RequestHeader,
            pRequest->NoOfReferencesToAdd,
            pRequest->ReferencesToAdd,
            &pResponse.ResponseHeader,
            &pResponse.NoOfResults,
            &pResponse.Results,
            &pResponse.NoOfDiagnosticInfos,
            &pResponse.DiagnosticInfos);
    }
    
    if (status != STATUS_OK)
    {
        SOPC_StatusCode faultStatus;
        OpcUa_ServiceFault faultObj;
        OpcUa_ServiceFault_Initialize(&faultObj);
        /* create a fault */
        faultStatus = SOPC_ServerApi_CreateFault(&pRequest->RequestHeader,
                                                 status,
                                                 &pResponse.ResponseHeader.ServiceDiagnostics,
                                                 &pResponse.ResponseHeader.NoOfStringTable,
                                                 &pResponse.ResponseHeader.StringTable,
                                                 &faultObj);
        
        if(faultStatus == STATUS_OK){
            /* send the response */
            faultStatus = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                                     &OpcUa_ServiceFault_EncodeableType, &pResponse,
                                                     &a_hContext);
        }
    }

    if(status == STATUS_OK){
        /* send the response */
        status = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                            &OpcUa_AddReferencesResponse_EncodeableType, &pResponse,
                                            &a_hContext);
    }
    return status;
}

/*============================================================================
 * The service dispatch information AddReferences service.
 *===========================================================================*/
struct SOPC_ServiceType OpcUa_AddReferences_ServiceType =
{
    OpcUaId_AddReferencesRequest,
    &OpcUa_AddReferencesResponse_EncodeableType,
    OpcUa_Server_BeginAddReferences,
    (SOPC_InvokeService*)OpcUa_ServerApi_AddReferences
};
#endif

#ifndef OPCUA_EXCLUDE_DeleteNodes
/*============================================================================
 * A pointer to a function that implements the DeleteNodes service.
 *===========================================================================*/
typedef SOPC_StatusCode (OpcUa_ServerApi_PfnDeleteNodes)(
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
 * A stub method which implements the DeleteNodes service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_DeleteNodes(
    SOPC_Endpoint                a_hEndpoint,
    struct SOPC_RequestContext*  a_hContext,
    const OpcUa_RequestHeader*   a_pRequestHeader,
    int32_t                      a_nNoOfNodesToDelete,
    const OpcUa_DeleteNodesItem* a_pNodesToDelete,
    OpcUa_ResponseHeader*        a_pResponseHeader,
    int32_t*                     a_pNoOfResults,
    SOPC_StatusCode**            a_pResults,
    int32_t*                     a_pNoOfDiagnosticInfos,
    SOPC_DiagnosticInfo**        a_pDiagnosticInfos)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;

    /* validate arguments. */
    if(a_hEndpoint != NULL && a_hContext != NULL){
        if( a_pRequestHeader != NULL
           &&  (a_pNodesToDelete != NULL || a_nNoOfNodesToDelete <= 0)
           &&  a_pResponseHeader != NULL
           &&  a_pNoOfResults != NULL
           &&  a_pResults != NULL
           &&  a_pNoOfDiagnosticInfos != NULL
           &&  a_pDiagnosticInfos != NULL)
            status = STATUS_OK;

    }

    status = OpcUa_BadNotImplemented;

    return status;
}

/*============================================================================
 * Begins processing of a DeleteNodes service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginDeleteNodes(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void*                       a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_DeleteNodesRequest* pRequest = NULL;
    OpcUa_DeleteNodesResponse pResponse;
    OpcUa_DeleteNodesResponse_Initialize(&pResponse);
    OpcUa_ServerApi_PfnDeleteNodes* pfnInvoke = NULL;

    if(a_hEndpoint != NULL && a_hContext != NULL
       && a_ppRequest != NULL
       && a_pRequestType != NULL){
        status = STATUS_OK;
    }
    
    if(a_pRequestType->TypeId != OpcUaId_DeleteNodesRequest){
        status = OpcUa_BadInvalidArgument;
    }

    if(status == STATUS_OK){
        pRequest = (OpcUa_DeleteNodesRequest*)a_ppRequest;
    }

    if(status == STATUS_OK){
        /* get the function that implements the service call. */
        status = SOPC_Endpoint_GetServiceFunction(a_hEndpoint, a_hContext,
                                                  (SOPC_InvokeService**)&pfnInvoke);
    }

    if(status == STATUS_OK){
    /* invoke the service */
        status = pfnInvoke(
            a_hEndpoint,
            a_hContext,
            &pRequest->RequestHeader,
            pRequest->NoOfNodesToDelete,
            pRequest->NodesToDelete,
            &pResponse.ResponseHeader,
            &pResponse.NoOfResults,
            &pResponse.Results,
            &pResponse.NoOfDiagnosticInfos,
            &pResponse.DiagnosticInfos);
    }
    
    if (status != STATUS_OK)
    {
        SOPC_StatusCode faultStatus;
        OpcUa_ServiceFault faultObj;
        OpcUa_ServiceFault_Initialize(&faultObj);
        /* create a fault */
        faultStatus = SOPC_ServerApi_CreateFault(&pRequest->RequestHeader,
                                                 status,
                                                 &pResponse.ResponseHeader.ServiceDiagnostics,
                                                 &pResponse.ResponseHeader.NoOfStringTable,
                                                 &pResponse.ResponseHeader.StringTable,
                                                 &faultObj);
        
        if(faultStatus == STATUS_OK){
            /* send the response */
            faultStatus = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                                     &OpcUa_ServiceFault_EncodeableType, &pResponse,
                                                     &a_hContext);
        }
    }

    if(status == STATUS_OK){
        /* send the response */
        status = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                            &OpcUa_DeleteNodesResponse_EncodeableType, &pResponse,
                                            &a_hContext);
    }
    return status;
}

/*============================================================================
 * The service dispatch information DeleteNodes service.
 *===========================================================================*/
struct SOPC_ServiceType OpcUa_DeleteNodes_ServiceType =
{
    OpcUaId_DeleteNodesRequest,
    &OpcUa_DeleteNodesResponse_EncodeableType,
    OpcUa_Server_BeginDeleteNodes,
    (SOPC_InvokeService*)OpcUa_ServerApi_DeleteNodes
};
#endif

#ifndef OPCUA_EXCLUDE_DeleteReferences
/*============================================================================
 * A pointer to a function that implements the DeleteReferences service.
 *===========================================================================*/
typedef SOPC_StatusCode (OpcUa_ServerApi_PfnDeleteReferences)(
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
 * A stub method which implements the DeleteReferences service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_DeleteReferences(
    SOPC_Endpoint                     a_hEndpoint,
    struct SOPC_RequestContext*       a_hContext,
    const OpcUa_RequestHeader*        a_pRequestHeader,
    int32_t                           a_nNoOfReferencesToDelete,
    const OpcUa_DeleteReferencesItem* a_pReferencesToDelete,
    OpcUa_ResponseHeader*             a_pResponseHeader,
    int32_t*                          a_pNoOfResults,
    SOPC_StatusCode**                 a_pResults,
    int32_t*                          a_pNoOfDiagnosticInfos,
    SOPC_DiagnosticInfo**             a_pDiagnosticInfos)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;

    /* validate arguments. */
    if(a_hEndpoint != NULL && a_hContext != NULL){
        if( a_pRequestHeader != NULL
           &&  (a_pReferencesToDelete != NULL || a_nNoOfReferencesToDelete <= 0)
           &&  a_pResponseHeader != NULL
           &&  a_pNoOfResults != NULL
           &&  a_pResults != NULL
           &&  a_pNoOfDiagnosticInfos != NULL
           &&  a_pDiagnosticInfos != NULL)
            status = STATUS_OK;

    }

    status = OpcUa_BadNotImplemented;

    return status;
}

/*============================================================================
 * Begins processing of a DeleteReferences service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginDeleteReferences(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void*                       a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_DeleteReferencesRequest* pRequest = NULL;
    OpcUa_DeleteReferencesResponse pResponse;
    OpcUa_DeleteReferencesResponse_Initialize(&pResponse);
    OpcUa_ServerApi_PfnDeleteReferences* pfnInvoke = NULL;

    if(a_hEndpoint != NULL && a_hContext != NULL
       && a_ppRequest != NULL
       && a_pRequestType != NULL){
        status = STATUS_OK;
    }
    
    if(a_pRequestType->TypeId != OpcUaId_DeleteReferencesRequest){
        status = OpcUa_BadInvalidArgument;
    }

    if(status == STATUS_OK){
        pRequest = (OpcUa_DeleteReferencesRequest*)a_ppRequest;
    }

    if(status == STATUS_OK){
        /* get the function that implements the service call. */
        status = SOPC_Endpoint_GetServiceFunction(a_hEndpoint, a_hContext,
                                                  (SOPC_InvokeService**)&pfnInvoke);
    }

    if(status == STATUS_OK){
    /* invoke the service */
        status = pfnInvoke(
            a_hEndpoint,
            a_hContext,
            &pRequest->RequestHeader,
            pRequest->NoOfReferencesToDelete,
            pRequest->ReferencesToDelete,
            &pResponse.ResponseHeader,
            &pResponse.NoOfResults,
            &pResponse.Results,
            &pResponse.NoOfDiagnosticInfos,
            &pResponse.DiagnosticInfos);
    }
    
    if (status != STATUS_OK)
    {
        SOPC_StatusCode faultStatus;
        OpcUa_ServiceFault faultObj;
        OpcUa_ServiceFault_Initialize(&faultObj);
        /* create a fault */
        faultStatus = SOPC_ServerApi_CreateFault(&pRequest->RequestHeader,
                                                 status,
                                                 &pResponse.ResponseHeader.ServiceDiagnostics,
                                                 &pResponse.ResponseHeader.NoOfStringTable,
                                                 &pResponse.ResponseHeader.StringTable,
                                                 &faultObj);
        
        if(faultStatus == STATUS_OK){
            /* send the response */
            faultStatus = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                                     &OpcUa_ServiceFault_EncodeableType, &pResponse,
                                                     &a_hContext);
        }
    }

    if(status == STATUS_OK){
        /* send the response */
        status = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                            &OpcUa_DeleteReferencesResponse_EncodeableType, &pResponse,
                                            &a_hContext);
    }
    return status;
}

/*============================================================================
 * The service dispatch information DeleteReferences service.
 *===========================================================================*/
struct SOPC_ServiceType OpcUa_DeleteReferences_ServiceType =
{
    OpcUaId_DeleteReferencesRequest,
    &OpcUa_DeleteReferencesResponse_EncodeableType,
    OpcUa_Server_BeginDeleteReferences,
    (SOPC_InvokeService*)OpcUa_ServerApi_DeleteReferences
};
#endif

#ifndef OPCUA_EXCLUDE_Browse
/*============================================================================
 * A pointer to a function that implements the Browse service.
 *===========================================================================*/
typedef SOPC_StatusCode (OpcUa_ServerApi_PfnBrowse)(
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
 * A stub method which implements the Browse service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_Browse(
    SOPC_Endpoint                  a_hEndpoint,
    struct SOPC_RequestContext*    a_hContext,
    const OpcUa_RequestHeader*     a_pRequestHeader,
    const OpcUa_ViewDescription*   a_pView,
    uint32_t                       a_nRequestedMaxReferencesPerNode,
    int32_t                        a_nNoOfNodesToBrowse,
    const OpcUa_BrowseDescription* a_pNodesToBrowse,
    OpcUa_ResponseHeader*          a_pResponseHeader,
    int32_t*                       a_pNoOfResults,
    OpcUa_BrowseResult**           a_pResults,
    int32_t*                       a_pNoOfDiagnosticInfos,
    SOPC_DiagnosticInfo**          a_pDiagnosticInfos)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;

    /* validate arguments. */
    if(a_hEndpoint != NULL && a_hContext != NULL){
        if( a_pRequestHeader != NULL
           &&  a_pView != NULL
           &&  (a_pNodesToBrowse != NULL || a_nNoOfNodesToBrowse <= 0)
           &&  a_pResponseHeader != NULL
           &&  a_pNoOfResults != NULL
           &&  a_pResults != NULL
           &&  a_pNoOfDiagnosticInfos != NULL
           &&  a_pDiagnosticInfos != NULL)
            status = STATUS_OK;
        (void) a_nRequestedMaxReferencesPerNode;

    }

    status = OpcUa_BadNotImplemented;

    return status;
}

/*============================================================================
 * Begins processing of a Browse service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginBrowse(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void*                       a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_BrowseRequest* pRequest = NULL;
    OpcUa_BrowseResponse pResponse;
    OpcUa_BrowseResponse_Initialize(&pResponse);
    OpcUa_ServerApi_PfnBrowse* pfnInvoke = NULL;

    if(a_hEndpoint != NULL && a_hContext != NULL
       && a_ppRequest != NULL
       && a_pRequestType != NULL){
        status = STATUS_OK;
    }
    
    if(a_pRequestType->TypeId != OpcUaId_BrowseRequest){
        status = OpcUa_BadInvalidArgument;
    }

    if(status == STATUS_OK){
        pRequest = (OpcUa_BrowseRequest*)a_ppRequest;
    }

    if(status == STATUS_OK){
        /* get the function that implements the service call. */
        status = SOPC_Endpoint_GetServiceFunction(a_hEndpoint, a_hContext,
                                                  (SOPC_InvokeService**)&pfnInvoke);
    }

    if(status == STATUS_OK){
    /* invoke the service */
        status = pfnInvoke(
            a_hEndpoint,
            a_hContext,
            &pRequest->RequestHeader,
            &pRequest->View,
            pRequest->RequestedMaxReferencesPerNode,
            pRequest->NoOfNodesToBrowse,
            pRequest->NodesToBrowse,
            &pResponse.ResponseHeader,
            &pResponse.NoOfResults,
            &pResponse.Results,
            &pResponse.NoOfDiagnosticInfos,
            &pResponse.DiagnosticInfos);
    }
    
    if (status != STATUS_OK)
    {
        SOPC_StatusCode faultStatus;
        OpcUa_ServiceFault faultObj;
        OpcUa_ServiceFault_Initialize(&faultObj);
        /* create a fault */
        faultStatus = SOPC_ServerApi_CreateFault(&pRequest->RequestHeader,
                                                 status,
                                                 &pResponse.ResponseHeader.ServiceDiagnostics,
                                                 &pResponse.ResponseHeader.NoOfStringTable,
                                                 &pResponse.ResponseHeader.StringTable,
                                                 &faultObj);
        
        if(faultStatus == STATUS_OK){
            /* send the response */
            faultStatus = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                                     &OpcUa_ServiceFault_EncodeableType, &pResponse,
                                                     &a_hContext);
        }
    }

    if(status == STATUS_OK){
        /* send the response */
        status = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                            &OpcUa_BrowseResponse_EncodeableType, &pResponse,
                                            &a_hContext);
    }
    return status;
}

/*============================================================================
 * The service dispatch information Browse service.
 *===========================================================================*/
struct SOPC_ServiceType OpcUa_Browse_ServiceType =
{
    OpcUaId_BrowseRequest,
    &OpcUa_BrowseResponse_EncodeableType,
    OpcUa_Server_BeginBrowse,
    (SOPC_InvokeService*)OpcUa_ServerApi_Browse
};
#endif

#ifndef OPCUA_EXCLUDE_BrowseNext
/*============================================================================
 * A pointer to a function that implements the BrowseNext service.
 *===========================================================================*/
typedef SOPC_StatusCode (OpcUa_ServerApi_PfnBrowseNext)(
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
 * A stub method which implements the BrowseNext service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_BrowseNext(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    const OpcUa_RequestHeader*  a_pRequestHeader,
    SOPC_Boolean                a_bReleaseContinuationPoints,
    int32_t                     a_nNoOfContinuationPoints,
    const SOPC_ByteString*      a_pContinuationPoints,
    OpcUa_ResponseHeader*       a_pResponseHeader,
    int32_t*                    a_pNoOfResults,
    OpcUa_BrowseResult**        a_pResults,
    int32_t*                    a_pNoOfDiagnosticInfos,
    SOPC_DiagnosticInfo**       a_pDiagnosticInfos)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;

    /* validate arguments. */
    if(a_hEndpoint != NULL && a_hContext != NULL){
        if( a_pRequestHeader != NULL
           &&  (a_pContinuationPoints != NULL || a_nNoOfContinuationPoints <= 0)
           &&  a_pResponseHeader != NULL
           &&  a_pNoOfResults != NULL
           &&  a_pResults != NULL
           &&  a_pNoOfDiagnosticInfos != NULL
           &&  a_pDiagnosticInfos != NULL)
            status = STATUS_OK;
        (void) a_bReleaseContinuationPoints;

    }

    status = OpcUa_BadNotImplemented;

    return status;
}

/*============================================================================
 * Begins processing of a BrowseNext service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginBrowseNext(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void*                       a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_BrowseNextRequest* pRequest = NULL;
    OpcUa_BrowseNextResponse pResponse;
    OpcUa_BrowseNextResponse_Initialize(&pResponse);
    OpcUa_ServerApi_PfnBrowseNext* pfnInvoke = NULL;

    if(a_hEndpoint != NULL && a_hContext != NULL
       && a_ppRequest != NULL
       && a_pRequestType != NULL){
        status = STATUS_OK;
    }
    
    if(a_pRequestType->TypeId != OpcUaId_BrowseNextRequest){
        status = OpcUa_BadInvalidArgument;
    }

    if(status == STATUS_OK){
        pRequest = (OpcUa_BrowseNextRequest*)a_ppRequest;
    }

    if(status == STATUS_OK){
        /* get the function that implements the service call. */
        status = SOPC_Endpoint_GetServiceFunction(a_hEndpoint, a_hContext,
                                                  (SOPC_InvokeService**)&pfnInvoke);
    }

    if(status == STATUS_OK){
    /* invoke the service */
        status = pfnInvoke(
            a_hEndpoint,
            a_hContext,
            &pRequest->RequestHeader,
            pRequest->ReleaseContinuationPoints,
            pRequest->NoOfContinuationPoints,
            pRequest->ContinuationPoints,
            &pResponse.ResponseHeader,
            &pResponse.NoOfResults,
            &pResponse.Results,
            &pResponse.NoOfDiagnosticInfos,
            &pResponse.DiagnosticInfos);
    }
    
    if (status != STATUS_OK)
    {
        SOPC_StatusCode faultStatus;
        OpcUa_ServiceFault faultObj;
        OpcUa_ServiceFault_Initialize(&faultObj);
        /* create a fault */
        faultStatus = SOPC_ServerApi_CreateFault(&pRequest->RequestHeader,
                                                 status,
                                                 &pResponse.ResponseHeader.ServiceDiagnostics,
                                                 &pResponse.ResponseHeader.NoOfStringTable,
                                                 &pResponse.ResponseHeader.StringTable,
                                                 &faultObj);
        
        if(faultStatus == STATUS_OK){
            /* send the response */
            faultStatus = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                                     &OpcUa_ServiceFault_EncodeableType, &pResponse,
                                                     &a_hContext);
        }
    }

    if(status == STATUS_OK){
        /* send the response */
        status = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                            &OpcUa_BrowseNextResponse_EncodeableType, &pResponse,
                                            &a_hContext);
    }
    return status;
}

/*============================================================================
 * The service dispatch information BrowseNext service.
 *===========================================================================*/
struct SOPC_ServiceType OpcUa_BrowseNext_ServiceType =
{
    OpcUaId_BrowseNextRequest,
    &OpcUa_BrowseNextResponse_EncodeableType,
    OpcUa_Server_BeginBrowseNext,
    (SOPC_InvokeService*)OpcUa_ServerApi_BrowseNext
};
#endif

#ifndef OPCUA_EXCLUDE_TranslateBrowsePathsToNodeIds
/*============================================================================
 * A pointer to a function that implements the TranslateBrowsePathsToNodeIds service.
 *===========================================================================*/
typedef SOPC_StatusCode (OpcUa_ServerApi_PfnTranslateBrowsePathsToNodeIds)(
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
 * A stub method which implements the TranslateBrowsePathsToNodeIds service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_TranslateBrowsePathsToNodeIds(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    const OpcUa_RequestHeader*  a_pRequestHeader,
    int32_t                     a_nNoOfBrowsePaths,
    const OpcUa_BrowsePath*     a_pBrowsePaths,
    OpcUa_ResponseHeader*       a_pResponseHeader,
    int32_t*                    a_pNoOfResults,
    OpcUa_BrowsePathResult**    a_pResults,
    int32_t*                    a_pNoOfDiagnosticInfos,
    SOPC_DiagnosticInfo**       a_pDiagnosticInfos)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;

    /* validate arguments. */
    if(a_hEndpoint != NULL && a_hContext != NULL){
        if( a_pRequestHeader != NULL
           &&  (a_pBrowsePaths != NULL || a_nNoOfBrowsePaths <= 0)
           &&  a_pResponseHeader != NULL
           &&  a_pNoOfResults != NULL
           &&  a_pResults != NULL
           &&  a_pNoOfDiagnosticInfos != NULL
           &&  a_pDiagnosticInfos != NULL)
            status = STATUS_OK;

    }

    status = OpcUa_BadNotImplemented;

    return status;
}

/*============================================================================
 * Begins processing of a TranslateBrowsePathsToNodeIds service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginTranslateBrowsePathsToNodeIds(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void*                       a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_TranslateBrowsePathsToNodeIdsRequest* pRequest = NULL;
    OpcUa_TranslateBrowsePathsToNodeIdsResponse pResponse;
    OpcUa_TranslateBrowsePathsToNodeIdsResponse_Initialize(&pResponse);
    OpcUa_ServerApi_PfnTranslateBrowsePathsToNodeIds* pfnInvoke = NULL;

    if(a_hEndpoint != NULL && a_hContext != NULL
       && a_ppRequest != NULL
       && a_pRequestType != NULL){
        status = STATUS_OK;
    }
    
    if(a_pRequestType->TypeId != OpcUaId_TranslateBrowsePathsToNodeIdsRequest){
        status = OpcUa_BadInvalidArgument;
    }

    if(status == STATUS_OK){
        pRequest = (OpcUa_TranslateBrowsePathsToNodeIdsRequest*)a_ppRequest;
    }

    if(status == STATUS_OK){
        /* get the function that implements the service call. */
        status = SOPC_Endpoint_GetServiceFunction(a_hEndpoint, a_hContext,
                                                  (SOPC_InvokeService**)&pfnInvoke);
    }

    if(status == STATUS_OK){
    /* invoke the service */
        status = pfnInvoke(
            a_hEndpoint,
            a_hContext,
            &pRequest->RequestHeader,
            pRequest->NoOfBrowsePaths,
            pRequest->BrowsePaths,
            &pResponse.ResponseHeader,
            &pResponse.NoOfResults,
            &pResponse.Results,
            &pResponse.NoOfDiagnosticInfos,
            &pResponse.DiagnosticInfos);
    }
    
    if (status != STATUS_OK)
    {
        SOPC_StatusCode faultStatus;
        OpcUa_ServiceFault faultObj;
        OpcUa_ServiceFault_Initialize(&faultObj);
        /* create a fault */
        faultStatus = SOPC_ServerApi_CreateFault(&pRequest->RequestHeader,
                                                 status,
                                                 &pResponse.ResponseHeader.ServiceDiagnostics,
                                                 &pResponse.ResponseHeader.NoOfStringTable,
                                                 &pResponse.ResponseHeader.StringTable,
                                                 &faultObj);
        
        if(faultStatus == STATUS_OK){
            /* send the response */
            faultStatus = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                                     &OpcUa_ServiceFault_EncodeableType, &pResponse,
                                                     &a_hContext);
        }
    }

    if(status == STATUS_OK){
        /* send the response */
        status = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                            &OpcUa_TranslateBrowsePathsToNodeIdsResponse_EncodeableType, &pResponse,
                                            &a_hContext);
    }
    return status;
}

/*============================================================================
 * The service dispatch information TranslateBrowsePathsToNodeIds service.
 *===========================================================================*/
struct SOPC_ServiceType OpcUa_TranslateBrowsePathsToNodeIds_ServiceType =
{
    OpcUaId_TranslateBrowsePathsToNodeIdsRequest,
    &OpcUa_TranslateBrowsePathsToNodeIdsResponse_EncodeableType,
    OpcUa_Server_BeginTranslateBrowsePathsToNodeIds,
    (SOPC_InvokeService*)OpcUa_ServerApi_TranslateBrowsePathsToNodeIds
};
#endif

#ifndef OPCUA_EXCLUDE_RegisterNodes
/*============================================================================
 * A pointer to a function that implements the RegisterNodes service.
 *===========================================================================*/
typedef SOPC_StatusCode (OpcUa_ServerApi_PfnRegisterNodes)(
    SOPC_Endpoint               hEndpoint,
    struct SOPC_RequestContext* hContext,
    const OpcUa_RequestHeader*  pRequestHeader,
    int32_t                     nNoOfNodesToRegister,
    const SOPC_NodeId*          pNodesToRegister,
    OpcUa_ResponseHeader*       pResponseHeader,
    int32_t*                    pNoOfRegisteredNodeIds,
    SOPC_NodeId**               pRegisteredNodeIds);

/*============================================================================
 * A stub method which implements the RegisterNodes service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_RegisterNodes(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    const OpcUa_RequestHeader*  a_pRequestHeader,
    int32_t                     a_nNoOfNodesToRegister,
    const SOPC_NodeId*          a_pNodesToRegister,
    OpcUa_ResponseHeader*       a_pResponseHeader,
    int32_t*                    a_pNoOfRegisteredNodeIds,
    SOPC_NodeId**               a_pRegisteredNodeIds)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;

    /* validate arguments. */
    if(a_hEndpoint != NULL && a_hContext != NULL){
        if( a_pRequestHeader != NULL
           &&  (a_pNodesToRegister != NULL || a_nNoOfNodesToRegister <= 0)
           &&  a_pResponseHeader != NULL
           &&  a_pNoOfRegisteredNodeIds != NULL
           &&  a_pRegisteredNodeIds != NULL)
            status = STATUS_OK;

    }

    status = OpcUa_BadNotImplemented;

    return status;
}

/*============================================================================
 * Begins processing of a RegisterNodes service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginRegisterNodes(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void*                       a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_RegisterNodesRequest* pRequest = NULL;
    OpcUa_RegisterNodesResponse pResponse;
    OpcUa_RegisterNodesResponse_Initialize(&pResponse);
    OpcUa_ServerApi_PfnRegisterNodes* pfnInvoke = NULL;

    if(a_hEndpoint != NULL && a_hContext != NULL
       && a_ppRequest != NULL
       && a_pRequestType != NULL){
        status = STATUS_OK;
    }
    
    if(a_pRequestType->TypeId != OpcUaId_RegisterNodesRequest){
        status = OpcUa_BadInvalidArgument;
    }

    if(status == STATUS_OK){
        pRequest = (OpcUa_RegisterNodesRequest*)a_ppRequest;
    }

    if(status == STATUS_OK){
        /* get the function that implements the service call. */
        status = SOPC_Endpoint_GetServiceFunction(a_hEndpoint, a_hContext,
                                                  (SOPC_InvokeService**)&pfnInvoke);
    }

    if(status == STATUS_OK){
    /* invoke the service */
        status = pfnInvoke(
            a_hEndpoint,
            a_hContext,
            &pRequest->RequestHeader,
            pRequest->NoOfNodesToRegister,
            pRequest->NodesToRegister,
            &pResponse.ResponseHeader,
            &pResponse.NoOfRegisteredNodeIds,
            &pResponse.RegisteredNodeIds);
    }
    
    if (status != STATUS_OK)
    {
        SOPC_StatusCode faultStatus;
        OpcUa_ServiceFault faultObj;
        OpcUa_ServiceFault_Initialize(&faultObj);
        /* create a fault */
        faultStatus = SOPC_ServerApi_CreateFault(&pRequest->RequestHeader,
                                                 status,
                                                 &pResponse.ResponseHeader.ServiceDiagnostics,
                                                 &pResponse.ResponseHeader.NoOfStringTable,
                                                 &pResponse.ResponseHeader.StringTable,
                                                 &faultObj);
        
        if(faultStatus == STATUS_OK){
            /* send the response */
            faultStatus = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                                     &OpcUa_ServiceFault_EncodeableType, &pResponse,
                                                     &a_hContext);
        }
    }

    if(status == STATUS_OK){
        /* send the response */
        status = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                            &OpcUa_RegisterNodesResponse_EncodeableType, &pResponse,
                                            &a_hContext);
    }
    return status;
}

/*============================================================================
 * The service dispatch information RegisterNodes service.
 *===========================================================================*/
struct SOPC_ServiceType OpcUa_RegisterNodes_ServiceType =
{
    OpcUaId_RegisterNodesRequest,
    &OpcUa_RegisterNodesResponse_EncodeableType,
    OpcUa_Server_BeginRegisterNodes,
    (SOPC_InvokeService*)OpcUa_ServerApi_RegisterNodes
};
#endif

#ifndef OPCUA_EXCLUDE_UnregisterNodes
/*============================================================================
 * A pointer to a function that implements the UnregisterNodes service.
 *===========================================================================*/
typedef SOPC_StatusCode (OpcUa_ServerApi_PfnUnregisterNodes)(
    SOPC_Endpoint               hEndpoint,
    struct SOPC_RequestContext* hContext,
    const OpcUa_RequestHeader*  pRequestHeader,
    int32_t                     nNoOfNodesToUnregister,
    const SOPC_NodeId*          pNodesToUnregister,
    OpcUa_ResponseHeader*       pResponseHeader);

/*============================================================================
 * A stub method which implements the UnregisterNodes service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_UnregisterNodes(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    const OpcUa_RequestHeader*  a_pRequestHeader,
    int32_t                     a_nNoOfNodesToUnregister,
    const SOPC_NodeId*          a_pNodesToUnregister,
    OpcUa_ResponseHeader*       a_pResponseHeader)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;

    /* validate arguments. */
    if(a_hEndpoint != NULL && a_hContext != NULL){
        if( a_pRequestHeader != NULL
           &&  (a_pNodesToUnregister != NULL || a_nNoOfNodesToUnregister <= 0)
           &&  a_pResponseHeader != NULL)
            status = STATUS_OK;

    }

    status = OpcUa_BadNotImplemented;

    return status;
}

/*============================================================================
 * Begins processing of a UnregisterNodes service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginUnregisterNodes(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void*                       a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_UnregisterNodesRequest* pRequest = NULL;
    OpcUa_UnregisterNodesResponse pResponse;
    OpcUa_UnregisterNodesResponse_Initialize(&pResponse);
    OpcUa_ServerApi_PfnUnregisterNodes* pfnInvoke = NULL;

    if(a_hEndpoint != NULL && a_hContext != NULL
       && a_ppRequest != NULL
       && a_pRequestType != NULL){
        status = STATUS_OK;
    }
    
    if(a_pRequestType->TypeId != OpcUaId_UnregisterNodesRequest){
        status = OpcUa_BadInvalidArgument;
    }

    if(status == STATUS_OK){
        pRequest = (OpcUa_UnregisterNodesRequest*)a_ppRequest;
    }

    if(status == STATUS_OK){
        /* get the function that implements the service call. */
        status = SOPC_Endpoint_GetServiceFunction(a_hEndpoint, a_hContext,
                                                  (SOPC_InvokeService**)&pfnInvoke);
    }

    if(status == STATUS_OK){
    /* invoke the service */
        status = pfnInvoke(
            a_hEndpoint,
            a_hContext,
            &pRequest->RequestHeader,
            pRequest->NoOfNodesToUnregister,
            pRequest->NodesToUnregister,
            &pResponse.ResponseHeader);
    }
    
    if (status != STATUS_OK)
    {
        SOPC_StatusCode faultStatus;
        OpcUa_ServiceFault faultObj;
        OpcUa_ServiceFault_Initialize(&faultObj);
        /* create a fault */
        faultStatus = SOPC_ServerApi_CreateFault(&pRequest->RequestHeader,
                                                 status,
                                                 &pResponse.ResponseHeader.ServiceDiagnostics,
                                                 &pResponse.ResponseHeader.NoOfStringTable,
                                                 &pResponse.ResponseHeader.StringTable,
                                                 &faultObj);
        
        if(faultStatus == STATUS_OK){
            /* send the response */
            faultStatus = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                                     &OpcUa_ServiceFault_EncodeableType, &pResponse,
                                                     &a_hContext);
        }
    }

    if(status == STATUS_OK){
        /* send the response */
        status = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                            &OpcUa_UnregisterNodesResponse_EncodeableType, &pResponse,
                                            &a_hContext);
    }
    return status;
}

/*============================================================================
 * The service dispatch information UnregisterNodes service.
 *===========================================================================*/
struct SOPC_ServiceType OpcUa_UnregisterNodes_ServiceType =
{
    OpcUaId_UnregisterNodesRequest,
    &OpcUa_UnregisterNodesResponse_EncodeableType,
    OpcUa_Server_BeginUnregisterNodes,
    (SOPC_InvokeService*)OpcUa_ServerApi_UnregisterNodes
};
#endif

#ifndef OPCUA_EXCLUDE_QueryFirst
/*============================================================================
 * A pointer to a function that implements the QueryFirst service.
 *===========================================================================*/
typedef SOPC_StatusCode (OpcUa_ServerApi_PfnQueryFirst)(
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
 * A stub method which implements the QueryFirst service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_QueryFirst(
    SOPC_Endpoint                    a_hEndpoint,
    struct SOPC_RequestContext*      a_hContext,
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
    SOPC_ByteString*                 a_pContinuationPoint,
    int32_t*                         a_pNoOfParsingResults,
    OpcUa_ParsingResult**            a_pParsingResults,
    int32_t*                         a_pNoOfDiagnosticInfos,
    SOPC_DiagnosticInfo**            a_pDiagnosticInfos,
    OpcUa_ContentFilterResult*       a_pFilterResult)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;

    /* validate arguments. */
    if(a_hEndpoint != NULL && a_hContext != NULL){
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
        (void) a_nMaxDataSetsToReturn;
        (void) a_nMaxReferencesToReturn;

    }

    status = OpcUa_BadNotImplemented;

    return status;
}

/*============================================================================
 * Begins processing of a QueryFirst service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginQueryFirst(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void*                       a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_QueryFirstRequest* pRequest = NULL;
    OpcUa_QueryFirstResponse pResponse;
    OpcUa_QueryFirstResponse_Initialize(&pResponse);
    OpcUa_ServerApi_PfnQueryFirst* pfnInvoke = NULL;

    if(a_hEndpoint != NULL && a_hContext != NULL
       && a_ppRequest != NULL
       && a_pRequestType != NULL){
        status = STATUS_OK;
    }
    
    if(a_pRequestType->TypeId != OpcUaId_QueryFirstRequest){
        status = OpcUa_BadInvalidArgument;
    }

    if(status == STATUS_OK){
        pRequest = (OpcUa_QueryFirstRequest*)a_ppRequest;
    }

    if(status == STATUS_OK){
        /* get the function that implements the service call. */
        status = SOPC_Endpoint_GetServiceFunction(a_hEndpoint, a_hContext,
                                                  (SOPC_InvokeService**)&pfnInvoke);
    }

    if(status == STATUS_OK){
    /* invoke the service */
        status = pfnInvoke(
            a_hEndpoint,
            a_hContext,
            &pRequest->RequestHeader,
            &pRequest->View,
            pRequest->NoOfNodeTypes,
            pRequest->NodeTypes,
            &pRequest->Filter,
            pRequest->MaxDataSetsToReturn,
            pRequest->MaxReferencesToReturn,
            &pResponse.ResponseHeader,
            &pResponse.NoOfQueryDataSets,
            &pResponse.QueryDataSets,
            &pResponse.ContinuationPoint,
            &pResponse.NoOfParsingResults,
            &pResponse.ParsingResults,
            &pResponse.NoOfDiagnosticInfos,
            &pResponse.DiagnosticInfos,
            &pResponse.FilterResult);
    }
    
    if (status != STATUS_OK)
    {
        SOPC_StatusCode faultStatus;
        OpcUa_ServiceFault faultObj;
        OpcUa_ServiceFault_Initialize(&faultObj);
        /* create a fault */
        faultStatus = SOPC_ServerApi_CreateFault(&pRequest->RequestHeader,
                                                 status,
                                                 &pResponse.ResponseHeader.ServiceDiagnostics,
                                                 &pResponse.ResponseHeader.NoOfStringTable,
                                                 &pResponse.ResponseHeader.StringTable,
                                                 &faultObj);
        
        if(faultStatus == STATUS_OK){
            /* send the response */
            faultStatus = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                                     &OpcUa_ServiceFault_EncodeableType, &pResponse,
                                                     &a_hContext);
        }
    }

    if(status == STATUS_OK){
        /* send the response */
        status = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                            &OpcUa_QueryFirstResponse_EncodeableType, &pResponse,
                                            &a_hContext);
    }
    return status;
}

/*============================================================================
 * The service dispatch information QueryFirst service.
 *===========================================================================*/
struct SOPC_ServiceType OpcUa_QueryFirst_ServiceType =
{
    OpcUaId_QueryFirstRequest,
    &OpcUa_QueryFirstResponse_EncodeableType,
    OpcUa_Server_BeginQueryFirst,
    (SOPC_InvokeService*)OpcUa_ServerApi_QueryFirst
};
#endif

#ifndef OPCUA_EXCLUDE_QueryNext
/*============================================================================
 * A pointer to a function that implements the QueryNext service.
 *===========================================================================*/
typedef SOPC_StatusCode (OpcUa_ServerApi_PfnQueryNext)(
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
 * A stub method which implements the QueryNext service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_QueryNext(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    const OpcUa_RequestHeader*  a_pRequestHeader,
    SOPC_Boolean                a_bReleaseContinuationPoint,
    const SOPC_ByteString*      a_pContinuationPoint,
    OpcUa_ResponseHeader*       a_pResponseHeader,
    int32_t*                    a_pNoOfQueryDataSets,
    OpcUa_QueryDataSet**        a_pQueryDataSets,
    SOPC_ByteString*            a_pRevisedContinuationPoint)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;

    /* validate arguments. */
    if(a_hEndpoint != NULL && a_hContext != NULL){
        if( a_pRequestHeader != NULL
           &&  a_pContinuationPoint != NULL
           &&  a_pResponseHeader != NULL
           &&  a_pNoOfQueryDataSets != NULL
           &&  a_pQueryDataSets != NULL
           &&  a_pRevisedContinuationPoint != NULL)
            status = STATUS_OK;
        (void) a_bReleaseContinuationPoint;

    }

    status = OpcUa_BadNotImplemented;

    return status;
}

/*============================================================================
 * Begins processing of a QueryNext service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginQueryNext(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void*                       a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_QueryNextRequest* pRequest = NULL;
    OpcUa_QueryNextResponse pResponse;
    OpcUa_QueryNextResponse_Initialize(&pResponse);
    OpcUa_ServerApi_PfnQueryNext* pfnInvoke = NULL;

    if(a_hEndpoint != NULL && a_hContext != NULL
       && a_ppRequest != NULL
       && a_pRequestType != NULL){
        status = STATUS_OK;
    }
    
    if(a_pRequestType->TypeId != OpcUaId_QueryNextRequest){
        status = OpcUa_BadInvalidArgument;
    }

    if(status == STATUS_OK){
        pRequest = (OpcUa_QueryNextRequest*)a_ppRequest;
    }

    if(status == STATUS_OK){
        /* get the function that implements the service call. */
        status = SOPC_Endpoint_GetServiceFunction(a_hEndpoint, a_hContext,
                                                  (SOPC_InvokeService**)&pfnInvoke);
    }

    if(status == STATUS_OK){
    /* invoke the service */
        status = pfnInvoke(
            a_hEndpoint,
            a_hContext,
            &pRequest->RequestHeader,
            pRequest->ReleaseContinuationPoint,
            &pRequest->ContinuationPoint,
            &pResponse.ResponseHeader,
            &pResponse.NoOfQueryDataSets,
            &pResponse.QueryDataSets,
            &pResponse.RevisedContinuationPoint);
    }
    
    if (status != STATUS_OK)
    {
        SOPC_StatusCode faultStatus;
        OpcUa_ServiceFault faultObj;
        OpcUa_ServiceFault_Initialize(&faultObj);
        /* create a fault */
        faultStatus = SOPC_ServerApi_CreateFault(&pRequest->RequestHeader,
                                                 status,
                                                 &pResponse.ResponseHeader.ServiceDiagnostics,
                                                 &pResponse.ResponseHeader.NoOfStringTable,
                                                 &pResponse.ResponseHeader.StringTable,
                                                 &faultObj);
        
        if(faultStatus == STATUS_OK){
            /* send the response */
            faultStatus = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                                     &OpcUa_ServiceFault_EncodeableType, &pResponse,
                                                     &a_hContext);
        }
    }

    if(status == STATUS_OK){
        /* send the response */
        status = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                            &OpcUa_QueryNextResponse_EncodeableType, &pResponse,
                                            &a_hContext);
    }
    return status;
}

/*============================================================================
 * The service dispatch information QueryNext service.
 *===========================================================================*/
struct SOPC_ServiceType OpcUa_QueryNext_ServiceType =
{
    OpcUaId_QueryNextRequest,
    &OpcUa_QueryNextResponse_EncodeableType,
    OpcUa_Server_BeginQueryNext,
    (SOPC_InvokeService*)OpcUa_ServerApi_QueryNext
};
#endif

#ifndef OPCUA_EXCLUDE_Read
/*============================================================================
 * A pointer to a function that implements the Read service.
 *===========================================================================*/
typedef SOPC_StatusCode (OpcUa_ServerApi_PfnRead)(
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
 * A stub method which implements the Read service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_Read(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    const OpcUa_RequestHeader*  a_pRequestHeader,
    double                      a_nMaxAge,
    OpcUa_TimestampsToReturn    a_eTimestampsToReturn,
    int32_t                     a_nNoOfNodesToRead,
    const OpcUa_ReadValueId*    a_pNodesToRead,
    OpcUa_ResponseHeader*       a_pResponseHeader,
    int32_t*                    a_pNoOfResults,
    SOPC_DataValue**            a_pResults,
    int32_t*                    a_pNoOfDiagnosticInfos,
    SOPC_DiagnosticInfo**       a_pDiagnosticInfos)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;

    /* validate arguments. */
    if(a_hEndpoint != NULL && a_hContext != NULL){
        if( a_pRequestHeader != NULL
           &&  (a_pNodesToRead != NULL || a_nNoOfNodesToRead <= 0)
           &&  a_pResponseHeader != NULL
           &&  a_pNoOfResults != NULL
           &&  a_pResults != NULL
           &&  a_pNoOfDiagnosticInfos != NULL
           &&  a_pDiagnosticInfos != NULL)
            status = STATUS_OK;
        (void) a_nMaxAge;
        (void) a_eTimestampsToReturn;

    }

    status = OpcUa_BadNotImplemented;

    return status;
}

/*============================================================================
 * Begins processing of a Read service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginRead(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void*                       a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_ReadRequest* pRequest = NULL;
    OpcUa_ReadResponse pResponse;
    OpcUa_ReadResponse_Initialize(&pResponse);
    OpcUa_ServerApi_PfnRead* pfnInvoke = NULL;

    if(a_hEndpoint != NULL && a_hContext != NULL
       && a_ppRequest != NULL
       && a_pRequestType != NULL){
        status = STATUS_OK;
    }
    
    if(a_pRequestType->TypeId != OpcUaId_ReadRequest){
        status = OpcUa_BadInvalidArgument;
    }

    if(status == STATUS_OK){
        pRequest = (OpcUa_ReadRequest*)a_ppRequest;
    }

    if(status == STATUS_OK){
        /* get the function that implements the service call. */
        status = SOPC_Endpoint_GetServiceFunction(a_hEndpoint, a_hContext,
                                                  (SOPC_InvokeService**)&pfnInvoke);
    }

    if(status == STATUS_OK){
    /* invoke the service */
        status = pfnInvoke(
            a_hEndpoint,
            a_hContext,
            &pRequest->RequestHeader,
            pRequest->MaxAge,
            pRequest->TimestampsToReturn,
            pRequest->NoOfNodesToRead,
            pRequest->NodesToRead,
            &pResponse.ResponseHeader,
            &pResponse.NoOfResults,
            &pResponse.Results,
            &pResponse.NoOfDiagnosticInfos,
            &pResponse.DiagnosticInfos);
    }
    
    if (status != STATUS_OK)
    {
        SOPC_StatusCode faultStatus;
        OpcUa_ServiceFault faultObj;
        OpcUa_ServiceFault_Initialize(&faultObj);
        /* create a fault */
        faultStatus = SOPC_ServerApi_CreateFault(&pRequest->RequestHeader,
                                                 status,
                                                 &pResponse.ResponseHeader.ServiceDiagnostics,
                                                 &pResponse.ResponseHeader.NoOfStringTable,
                                                 &pResponse.ResponseHeader.StringTable,
                                                 &faultObj);
        
        if(faultStatus == STATUS_OK){
            /* send the response */
            faultStatus = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                                     &OpcUa_ServiceFault_EncodeableType, &pResponse,
                                                     &a_hContext);
        }
    }

    if(status == STATUS_OK){
        /* send the response */
        status = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                            &OpcUa_ReadResponse_EncodeableType, &pResponse,
                                            &a_hContext);
    }
    return status;
}

/*============================================================================
 * The service dispatch information Read service.
 *===========================================================================*/
struct SOPC_ServiceType OpcUa_Read_ServiceType =
{
    OpcUaId_ReadRequest,
    &OpcUa_ReadResponse_EncodeableType,
    OpcUa_Server_BeginRead,
    (SOPC_InvokeService*)OpcUa_ServerApi_Read
};
#endif

#ifndef OPCUA_EXCLUDE_HistoryRead
/*============================================================================
 * A pointer to a function that implements the HistoryRead service.
 *===========================================================================*/
typedef SOPC_StatusCode (OpcUa_ServerApi_PfnHistoryRead)(
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
 * A stub method which implements the HistoryRead service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_HistoryRead(
    SOPC_Endpoint                   a_hEndpoint,
    struct SOPC_RequestContext*     a_hContext,
    const OpcUa_RequestHeader*      a_pRequestHeader,
    const SOPC_ExtensionObject*     a_pHistoryReadDetails,
    OpcUa_TimestampsToReturn        a_eTimestampsToReturn,
    SOPC_Boolean                    a_bReleaseContinuationPoints,
    int32_t                         a_nNoOfNodesToRead,
    const OpcUa_HistoryReadValueId* a_pNodesToRead,
    OpcUa_ResponseHeader*           a_pResponseHeader,
    int32_t*                        a_pNoOfResults,
    OpcUa_HistoryReadResult**       a_pResults,
    int32_t*                        a_pNoOfDiagnosticInfos,
    SOPC_DiagnosticInfo**           a_pDiagnosticInfos)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;

    /* validate arguments. */
    if(a_hEndpoint != NULL && a_hContext != NULL){
        if( a_pRequestHeader != NULL
           &&  a_pHistoryReadDetails != NULL
           &&  (a_pNodesToRead != NULL || a_nNoOfNodesToRead <= 0)
           &&  a_pResponseHeader != NULL
           &&  a_pNoOfResults != NULL
           &&  a_pResults != NULL
           &&  a_pNoOfDiagnosticInfos != NULL
           &&  a_pDiagnosticInfos != NULL)
            status = STATUS_OK;
        (void) a_eTimestampsToReturn;
        (void) a_bReleaseContinuationPoints;

    }

    status = OpcUa_BadNotImplemented;

    return status;
}

/*============================================================================
 * Begins processing of a HistoryRead service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginHistoryRead(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void*                       a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_HistoryReadRequest* pRequest = NULL;
    OpcUa_HistoryReadResponse pResponse;
    OpcUa_HistoryReadResponse_Initialize(&pResponse);
    OpcUa_ServerApi_PfnHistoryRead* pfnInvoke = NULL;

    if(a_hEndpoint != NULL && a_hContext != NULL
       && a_ppRequest != NULL
       && a_pRequestType != NULL){
        status = STATUS_OK;
    }
    
    if(a_pRequestType->TypeId != OpcUaId_HistoryReadRequest){
        status = OpcUa_BadInvalidArgument;
    }

    if(status == STATUS_OK){
        pRequest = (OpcUa_HistoryReadRequest*)a_ppRequest;
    }

    if(status == STATUS_OK){
        /* get the function that implements the service call. */
        status = SOPC_Endpoint_GetServiceFunction(a_hEndpoint, a_hContext,
                                                  (SOPC_InvokeService**)&pfnInvoke);
    }

    if(status == STATUS_OK){
    /* invoke the service */
        status = pfnInvoke(
            a_hEndpoint,
            a_hContext,
            &pRequest->RequestHeader,
            &pRequest->HistoryReadDetails,
            pRequest->TimestampsToReturn,
            pRequest->ReleaseContinuationPoints,
            pRequest->NoOfNodesToRead,
            pRequest->NodesToRead,
            &pResponse.ResponseHeader,
            &pResponse.NoOfResults,
            &pResponse.Results,
            &pResponse.NoOfDiagnosticInfos,
            &pResponse.DiagnosticInfos);
    }
    
    if (status != STATUS_OK)
    {
        SOPC_StatusCode faultStatus;
        OpcUa_ServiceFault faultObj;
        OpcUa_ServiceFault_Initialize(&faultObj);
        /* create a fault */
        faultStatus = SOPC_ServerApi_CreateFault(&pRequest->RequestHeader,
                                                 status,
                                                 &pResponse.ResponseHeader.ServiceDiagnostics,
                                                 &pResponse.ResponseHeader.NoOfStringTable,
                                                 &pResponse.ResponseHeader.StringTable,
                                                 &faultObj);
        
        if(faultStatus == STATUS_OK){
            /* send the response */
            faultStatus = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                                     &OpcUa_ServiceFault_EncodeableType, &pResponse,
                                                     &a_hContext);
        }
    }

    if(status == STATUS_OK){
        /* send the response */
        status = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                            &OpcUa_HistoryReadResponse_EncodeableType, &pResponse,
                                            &a_hContext);
    }
    return status;
}

/*============================================================================
 * The service dispatch information HistoryRead service.
 *===========================================================================*/
struct SOPC_ServiceType OpcUa_HistoryRead_ServiceType =
{
    OpcUaId_HistoryReadRequest,
    &OpcUa_HistoryReadResponse_EncodeableType,
    OpcUa_Server_BeginHistoryRead,
    (SOPC_InvokeService*)OpcUa_ServerApi_HistoryRead
};
#endif

#ifndef OPCUA_EXCLUDE_Write
/*============================================================================
 * A pointer to a function that implements the Write service.
 *===========================================================================*/
typedef SOPC_StatusCode (OpcUa_ServerApi_PfnWrite)(
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
 * A stub method which implements the Write service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_Write(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    const OpcUa_RequestHeader*  a_pRequestHeader,
    int32_t                     a_nNoOfNodesToWrite,
    const OpcUa_WriteValue*     a_pNodesToWrite,
    OpcUa_ResponseHeader*       a_pResponseHeader,
    int32_t*                    a_pNoOfResults,
    SOPC_StatusCode**           a_pResults,
    int32_t*                    a_pNoOfDiagnosticInfos,
    SOPC_DiagnosticInfo**       a_pDiagnosticInfos)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;

    /* validate arguments. */
    if(a_hEndpoint != NULL && a_hContext != NULL){
        if( a_pRequestHeader != NULL
           &&  (a_pNodesToWrite != NULL || a_nNoOfNodesToWrite <= 0)
           &&  a_pResponseHeader != NULL
           &&  a_pNoOfResults != NULL
           &&  a_pResults != NULL
           &&  a_pNoOfDiagnosticInfos != NULL
           &&  a_pDiagnosticInfos != NULL)
            status = STATUS_OK;

    }

    status = OpcUa_BadNotImplemented;

    return status;
}

/*============================================================================
 * Begins processing of a Write service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginWrite(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void*                       a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_WriteRequest* pRequest = NULL;
    OpcUa_WriteResponse pResponse;
    OpcUa_WriteResponse_Initialize(&pResponse);
    OpcUa_ServerApi_PfnWrite* pfnInvoke = NULL;

    if(a_hEndpoint != NULL && a_hContext != NULL
       && a_ppRequest != NULL
       && a_pRequestType != NULL){
        status = STATUS_OK;
    }
    
    if(a_pRequestType->TypeId != OpcUaId_WriteRequest){
        status = OpcUa_BadInvalidArgument;
    }

    if(status == STATUS_OK){
        pRequest = (OpcUa_WriteRequest*)a_ppRequest;
    }

    if(status == STATUS_OK){
        /* get the function that implements the service call. */
        status = SOPC_Endpoint_GetServiceFunction(a_hEndpoint, a_hContext,
                                                  (SOPC_InvokeService**)&pfnInvoke);
    }

    if(status == STATUS_OK){
    /* invoke the service */
        status = pfnInvoke(
            a_hEndpoint,
            a_hContext,
            &pRequest->RequestHeader,
            pRequest->NoOfNodesToWrite,
            pRequest->NodesToWrite,
            &pResponse.ResponseHeader,
            &pResponse.NoOfResults,
            &pResponse.Results,
            &pResponse.NoOfDiagnosticInfos,
            &pResponse.DiagnosticInfos);
    }
    
    if (status != STATUS_OK)
    {
        SOPC_StatusCode faultStatus;
        OpcUa_ServiceFault faultObj;
        OpcUa_ServiceFault_Initialize(&faultObj);
        /* create a fault */
        faultStatus = SOPC_ServerApi_CreateFault(&pRequest->RequestHeader,
                                                 status,
                                                 &pResponse.ResponseHeader.ServiceDiagnostics,
                                                 &pResponse.ResponseHeader.NoOfStringTable,
                                                 &pResponse.ResponseHeader.StringTable,
                                                 &faultObj);
        
        if(faultStatus == STATUS_OK){
            /* send the response */
            faultStatus = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                                     &OpcUa_ServiceFault_EncodeableType, &pResponse,
                                                     &a_hContext);
        }
    }

    if(status == STATUS_OK){
        /* send the response */
        status = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                            &OpcUa_WriteResponse_EncodeableType, &pResponse,
                                            &a_hContext);
    }
    return status;
}

/*============================================================================
 * The service dispatch information Write service.
 *===========================================================================*/
struct SOPC_ServiceType OpcUa_Write_ServiceType =
{
    OpcUaId_WriteRequest,
    &OpcUa_WriteResponse_EncodeableType,
    OpcUa_Server_BeginWrite,
    (SOPC_InvokeService*)OpcUa_ServerApi_Write
};
#endif

#ifndef OPCUA_EXCLUDE_HistoryUpdate
/*============================================================================
 * A pointer to a function that implements the HistoryUpdate service.
 *===========================================================================*/
typedef SOPC_StatusCode (OpcUa_ServerApi_PfnHistoryUpdate)(
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
 * A stub method which implements the HistoryUpdate service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_HistoryUpdate(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    const OpcUa_RequestHeader*  a_pRequestHeader,
    int32_t                     a_nNoOfHistoryUpdateDetails,
    const SOPC_ExtensionObject* a_pHistoryUpdateDetails,
    OpcUa_ResponseHeader*       a_pResponseHeader,
    int32_t*                    a_pNoOfResults,
    OpcUa_HistoryUpdateResult** a_pResults,
    int32_t*                    a_pNoOfDiagnosticInfos,
    SOPC_DiagnosticInfo**       a_pDiagnosticInfos)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;

    /* validate arguments. */
    if(a_hEndpoint != NULL && a_hContext != NULL){
        if( a_pRequestHeader != NULL
           &&  (a_pHistoryUpdateDetails != NULL || a_nNoOfHistoryUpdateDetails <= 0)
           &&  a_pResponseHeader != NULL
           &&  a_pNoOfResults != NULL
           &&  a_pResults != NULL
           &&  a_pNoOfDiagnosticInfos != NULL
           &&  a_pDiagnosticInfos != NULL)
            status = STATUS_OK;

    }

    status = OpcUa_BadNotImplemented;

    return status;
}

/*============================================================================
 * Begins processing of a HistoryUpdate service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginHistoryUpdate(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void*                       a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_HistoryUpdateRequest* pRequest = NULL;
    OpcUa_HistoryUpdateResponse pResponse;
    OpcUa_HistoryUpdateResponse_Initialize(&pResponse);
    OpcUa_ServerApi_PfnHistoryUpdate* pfnInvoke = NULL;

    if(a_hEndpoint != NULL && a_hContext != NULL
       && a_ppRequest != NULL
       && a_pRequestType != NULL){
        status = STATUS_OK;
    }
    
    if(a_pRequestType->TypeId != OpcUaId_HistoryUpdateRequest){
        status = OpcUa_BadInvalidArgument;
    }

    if(status == STATUS_OK){
        pRequest = (OpcUa_HistoryUpdateRequest*)a_ppRequest;
    }

    if(status == STATUS_OK){
        /* get the function that implements the service call. */
        status = SOPC_Endpoint_GetServiceFunction(a_hEndpoint, a_hContext,
                                                  (SOPC_InvokeService**)&pfnInvoke);
    }

    if(status == STATUS_OK){
    /* invoke the service */
        status = pfnInvoke(
            a_hEndpoint,
            a_hContext,
            &pRequest->RequestHeader,
            pRequest->NoOfHistoryUpdateDetails,
            pRequest->HistoryUpdateDetails,
            &pResponse.ResponseHeader,
            &pResponse.NoOfResults,
            &pResponse.Results,
            &pResponse.NoOfDiagnosticInfos,
            &pResponse.DiagnosticInfos);
    }
    
    if (status != STATUS_OK)
    {
        SOPC_StatusCode faultStatus;
        OpcUa_ServiceFault faultObj;
        OpcUa_ServiceFault_Initialize(&faultObj);
        /* create a fault */
        faultStatus = SOPC_ServerApi_CreateFault(&pRequest->RequestHeader,
                                                 status,
                                                 &pResponse.ResponseHeader.ServiceDiagnostics,
                                                 &pResponse.ResponseHeader.NoOfStringTable,
                                                 &pResponse.ResponseHeader.StringTable,
                                                 &faultObj);
        
        if(faultStatus == STATUS_OK){
            /* send the response */
            faultStatus = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                                     &OpcUa_ServiceFault_EncodeableType, &pResponse,
                                                     &a_hContext);
        }
    }

    if(status == STATUS_OK){
        /* send the response */
        status = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                            &OpcUa_HistoryUpdateResponse_EncodeableType, &pResponse,
                                            &a_hContext);
    }
    return status;
}

/*============================================================================
 * The service dispatch information HistoryUpdate service.
 *===========================================================================*/
struct SOPC_ServiceType OpcUa_HistoryUpdate_ServiceType =
{
    OpcUaId_HistoryUpdateRequest,
    &OpcUa_HistoryUpdateResponse_EncodeableType,
    OpcUa_Server_BeginHistoryUpdate,
    (SOPC_InvokeService*)OpcUa_ServerApi_HistoryUpdate
};
#endif

#ifndef OPCUA_EXCLUDE_Call
/*============================================================================
 * A pointer to a function that implements the Call service.
 *===========================================================================*/
typedef SOPC_StatusCode (OpcUa_ServerApi_PfnCall)(
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
 * A stub method which implements the Call service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_Call(
    SOPC_Endpoint                  a_hEndpoint,
    struct SOPC_RequestContext*    a_hContext,
    const OpcUa_RequestHeader*     a_pRequestHeader,
    int32_t                        a_nNoOfMethodsToCall,
    const OpcUa_CallMethodRequest* a_pMethodsToCall,
    OpcUa_ResponseHeader*          a_pResponseHeader,
    int32_t*                       a_pNoOfResults,
    OpcUa_CallMethodResult**       a_pResults,
    int32_t*                       a_pNoOfDiagnosticInfos,
    SOPC_DiagnosticInfo**          a_pDiagnosticInfos)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;

    /* validate arguments. */
    if(a_hEndpoint != NULL && a_hContext != NULL){
        if( a_pRequestHeader != NULL
           &&  (a_pMethodsToCall != NULL || a_nNoOfMethodsToCall <= 0)
           &&  a_pResponseHeader != NULL
           &&  a_pNoOfResults != NULL
           &&  a_pResults != NULL
           &&  a_pNoOfDiagnosticInfos != NULL
           &&  a_pDiagnosticInfos != NULL)
            status = STATUS_OK;

    }

    status = OpcUa_BadNotImplemented;

    return status;
}

/*============================================================================
 * Begins processing of a Call service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginCall(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void*                       a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_CallRequest* pRequest = NULL;
    OpcUa_CallResponse pResponse;
    OpcUa_CallResponse_Initialize(&pResponse);
    OpcUa_ServerApi_PfnCall* pfnInvoke = NULL;

    if(a_hEndpoint != NULL && a_hContext != NULL
       && a_ppRequest != NULL
       && a_pRequestType != NULL){
        status = STATUS_OK;
    }
    
    if(a_pRequestType->TypeId != OpcUaId_CallRequest){
        status = OpcUa_BadInvalidArgument;
    }

    if(status == STATUS_OK){
        pRequest = (OpcUa_CallRequest*)a_ppRequest;
    }

    if(status == STATUS_OK){
        /* get the function that implements the service call. */
        status = SOPC_Endpoint_GetServiceFunction(a_hEndpoint, a_hContext,
                                                  (SOPC_InvokeService**)&pfnInvoke);
    }

    if(status == STATUS_OK){
    /* invoke the service */
        status = pfnInvoke(
            a_hEndpoint,
            a_hContext,
            &pRequest->RequestHeader,
            pRequest->NoOfMethodsToCall,
            pRequest->MethodsToCall,
            &pResponse.ResponseHeader,
            &pResponse.NoOfResults,
            &pResponse.Results,
            &pResponse.NoOfDiagnosticInfos,
            &pResponse.DiagnosticInfos);
    }
    
    if (status != STATUS_OK)
    {
        SOPC_StatusCode faultStatus;
        OpcUa_ServiceFault faultObj;
        OpcUa_ServiceFault_Initialize(&faultObj);
        /* create a fault */
        faultStatus = SOPC_ServerApi_CreateFault(&pRequest->RequestHeader,
                                                 status,
                                                 &pResponse.ResponseHeader.ServiceDiagnostics,
                                                 &pResponse.ResponseHeader.NoOfStringTable,
                                                 &pResponse.ResponseHeader.StringTable,
                                                 &faultObj);
        
        if(faultStatus == STATUS_OK){
            /* send the response */
            faultStatus = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                                     &OpcUa_ServiceFault_EncodeableType, &pResponse,
                                                     &a_hContext);
        }
    }

    if(status == STATUS_OK){
        /* send the response */
        status = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                            &OpcUa_CallResponse_EncodeableType, &pResponse,
                                            &a_hContext);
    }
    return status;
}

/*============================================================================
 * The service dispatch information Call service.
 *===========================================================================*/
struct SOPC_ServiceType OpcUa_Call_ServiceType =
{
    OpcUaId_CallRequest,
    &OpcUa_CallResponse_EncodeableType,
    OpcUa_Server_BeginCall,
    (SOPC_InvokeService*)OpcUa_ServerApi_Call
};
#endif

#ifndef OPCUA_EXCLUDE_CreateMonitoredItems
/*============================================================================
 * A pointer to a function that implements the CreateMonitoredItems service.
 *===========================================================================*/
typedef SOPC_StatusCode (OpcUa_ServerApi_PfnCreateMonitoredItems)(
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
 * A stub method which implements the CreateMonitoredItems service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_CreateMonitoredItems(
    SOPC_Endpoint                           a_hEndpoint,
    struct SOPC_RequestContext*             a_hContext,
    const OpcUa_RequestHeader*              a_pRequestHeader,
    uint32_t                                a_nSubscriptionId,
    OpcUa_TimestampsToReturn                a_eTimestampsToReturn,
    int32_t                                 a_nNoOfItemsToCreate,
    const OpcUa_MonitoredItemCreateRequest* a_pItemsToCreate,
    OpcUa_ResponseHeader*                   a_pResponseHeader,
    int32_t*                                a_pNoOfResults,
    OpcUa_MonitoredItemCreateResult**       a_pResults,
    int32_t*                                a_pNoOfDiagnosticInfos,
    SOPC_DiagnosticInfo**                   a_pDiagnosticInfos)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;

    /* validate arguments. */
    if(a_hEndpoint != NULL && a_hContext != NULL){
        if( a_pRequestHeader != NULL
           &&  (a_pItemsToCreate != NULL || a_nNoOfItemsToCreate <= 0)
           &&  a_pResponseHeader != NULL
           &&  a_pNoOfResults != NULL
           &&  a_pResults != NULL
           &&  a_pNoOfDiagnosticInfos != NULL
           &&  a_pDiagnosticInfos != NULL)
            status = STATUS_OK;
        (void) a_nSubscriptionId;
        (void) a_eTimestampsToReturn;

    }

    status = OpcUa_BadNotImplemented;

    return status;
}

/*============================================================================
 * Begins processing of a CreateMonitoredItems service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginCreateMonitoredItems(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void*                       a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_CreateMonitoredItemsRequest* pRequest = NULL;
    OpcUa_CreateMonitoredItemsResponse pResponse;
    OpcUa_CreateMonitoredItemsResponse_Initialize(&pResponse);
    OpcUa_ServerApi_PfnCreateMonitoredItems* pfnInvoke = NULL;

    if(a_hEndpoint != NULL && a_hContext != NULL
       && a_ppRequest != NULL
       && a_pRequestType != NULL){
        status = STATUS_OK;
    }
    
    if(a_pRequestType->TypeId != OpcUaId_CreateMonitoredItemsRequest){
        status = OpcUa_BadInvalidArgument;
    }

    if(status == STATUS_OK){
        pRequest = (OpcUa_CreateMonitoredItemsRequest*)a_ppRequest;
    }

    if(status == STATUS_OK){
        /* get the function that implements the service call. */
        status = SOPC_Endpoint_GetServiceFunction(a_hEndpoint, a_hContext,
                                                  (SOPC_InvokeService**)&pfnInvoke);
    }

    if(status == STATUS_OK){
    /* invoke the service */
        status = pfnInvoke(
            a_hEndpoint,
            a_hContext,
            &pRequest->RequestHeader,
            pRequest->SubscriptionId,
            pRequest->TimestampsToReturn,
            pRequest->NoOfItemsToCreate,
            pRequest->ItemsToCreate,
            &pResponse.ResponseHeader,
            &pResponse.NoOfResults,
            &pResponse.Results,
            &pResponse.NoOfDiagnosticInfos,
            &pResponse.DiagnosticInfos);
    }
    
    if (status != STATUS_OK)
    {
        SOPC_StatusCode faultStatus;
        OpcUa_ServiceFault faultObj;
        OpcUa_ServiceFault_Initialize(&faultObj);
        /* create a fault */
        faultStatus = SOPC_ServerApi_CreateFault(&pRequest->RequestHeader,
                                                 status,
                                                 &pResponse.ResponseHeader.ServiceDiagnostics,
                                                 &pResponse.ResponseHeader.NoOfStringTable,
                                                 &pResponse.ResponseHeader.StringTable,
                                                 &faultObj);
        
        if(faultStatus == STATUS_OK){
            /* send the response */
            faultStatus = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                                     &OpcUa_ServiceFault_EncodeableType, &pResponse,
                                                     &a_hContext);
        }
    }

    if(status == STATUS_OK){
        /* send the response */
        status = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                            &OpcUa_CreateMonitoredItemsResponse_EncodeableType, &pResponse,
                                            &a_hContext);
    }
    return status;
}

/*============================================================================
 * The service dispatch information CreateMonitoredItems service.
 *===========================================================================*/
struct SOPC_ServiceType OpcUa_CreateMonitoredItems_ServiceType =
{
    OpcUaId_CreateMonitoredItemsRequest,
    &OpcUa_CreateMonitoredItemsResponse_EncodeableType,
    OpcUa_Server_BeginCreateMonitoredItems,
    (SOPC_InvokeService*)OpcUa_ServerApi_CreateMonitoredItems
};
#endif

#ifndef OPCUA_EXCLUDE_ModifyMonitoredItems
/*============================================================================
 * A pointer to a function that implements the ModifyMonitoredItems service.
 *===========================================================================*/
typedef SOPC_StatusCode (OpcUa_ServerApi_PfnModifyMonitoredItems)(
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
 * A stub method which implements the ModifyMonitoredItems service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_ModifyMonitoredItems(
    SOPC_Endpoint                           a_hEndpoint,
    struct SOPC_RequestContext*             a_hContext,
    const OpcUa_RequestHeader*              a_pRequestHeader,
    uint32_t                                a_nSubscriptionId,
    OpcUa_TimestampsToReturn                a_eTimestampsToReturn,
    int32_t                                 a_nNoOfItemsToModify,
    const OpcUa_MonitoredItemModifyRequest* a_pItemsToModify,
    OpcUa_ResponseHeader*                   a_pResponseHeader,
    int32_t*                                a_pNoOfResults,
    OpcUa_MonitoredItemModifyResult**       a_pResults,
    int32_t*                                a_pNoOfDiagnosticInfos,
    SOPC_DiagnosticInfo**                   a_pDiagnosticInfos)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;

    /* validate arguments. */
    if(a_hEndpoint != NULL && a_hContext != NULL){
        if( a_pRequestHeader != NULL
           &&  (a_pItemsToModify != NULL || a_nNoOfItemsToModify <= 0)
           &&  a_pResponseHeader != NULL
           &&  a_pNoOfResults != NULL
           &&  a_pResults != NULL
           &&  a_pNoOfDiagnosticInfos != NULL
           &&  a_pDiagnosticInfos != NULL)
            status = STATUS_OK;
        (void) a_nSubscriptionId;
        (void) a_eTimestampsToReturn;

    }

    status = OpcUa_BadNotImplemented;

    return status;
}

/*============================================================================
 * Begins processing of a ModifyMonitoredItems service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginModifyMonitoredItems(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void*                       a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_ModifyMonitoredItemsRequest* pRequest = NULL;
    OpcUa_ModifyMonitoredItemsResponse pResponse;
    OpcUa_ModifyMonitoredItemsResponse_Initialize(&pResponse);
    OpcUa_ServerApi_PfnModifyMonitoredItems* pfnInvoke = NULL;

    if(a_hEndpoint != NULL && a_hContext != NULL
       && a_ppRequest != NULL
       && a_pRequestType != NULL){
        status = STATUS_OK;
    }
    
    if(a_pRequestType->TypeId != OpcUaId_ModifyMonitoredItemsRequest){
        status = OpcUa_BadInvalidArgument;
    }

    if(status == STATUS_OK){
        pRequest = (OpcUa_ModifyMonitoredItemsRequest*)a_ppRequest;
    }

    if(status == STATUS_OK){
        /* get the function that implements the service call. */
        status = SOPC_Endpoint_GetServiceFunction(a_hEndpoint, a_hContext,
                                                  (SOPC_InvokeService**)&pfnInvoke);
    }

    if(status == STATUS_OK){
    /* invoke the service */
        status = pfnInvoke(
            a_hEndpoint,
            a_hContext,
            &pRequest->RequestHeader,
            pRequest->SubscriptionId,
            pRequest->TimestampsToReturn,
            pRequest->NoOfItemsToModify,
            pRequest->ItemsToModify,
            &pResponse.ResponseHeader,
            &pResponse.NoOfResults,
            &pResponse.Results,
            &pResponse.NoOfDiagnosticInfos,
            &pResponse.DiagnosticInfos);
    }
    
    if (status != STATUS_OK)
    {
        SOPC_StatusCode faultStatus;
        OpcUa_ServiceFault faultObj;
        OpcUa_ServiceFault_Initialize(&faultObj);
        /* create a fault */
        faultStatus = SOPC_ServerApi_CreateFault(&pRequest->RequestHeader,
                                                 status,
                                                 &pResponse.ResponseHeader.ServiceDiagnostics,
                                                 &pResponse.ResponseHeader.NoOfStringTable,
                                                 &pResponse.ResponseHeader.StringTable,
                                                 &faultObj);
        
        if(faultStatus == STATUS_OK){
            /* send the response */
            faultStatus = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                                     &OpcUa_ServiceFault_EncodeableType, &pResponse,
                                                     &a_hContext);
        }
    }

    if(status == STATUS_OK){
        /* send the response */
        status = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                            &OpcUa_ModifyMonitoredItemsResponse_EncodeableType, &pResponse,
                                            &a_hContext);
    }
    return status;
}

/*============================================================================
 * The service dispatch information ModifyMonitoredItems service.
 *===========================================================================*/
struct SOPC_ServiceType OpcUa_ModifyMonitoredItems_ServiceType =
{
    OpcUaId_ModifyMonitoredItemsRequest,
    &OpcUa_ModifyMonitoredItemsResponse_EncodeableType,
    OpcUa_Server_BeginModifyMonitoredItems,
    (SOPC_InvokeService*)OpcUa_ServerApi_ModifyMonitoredItems
};
#endif

#ifndef OPCUA_EXCLUDE_SetMonitoringMode
/*============================================================================
 * A pointer to a function that implements the SetMonitoringMode service.
 *===========================================================================*/
typedef SOPC_StatusCode (OpcUa_ServerApi_PfnSetMonitoringMode)(
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
 * A stub method which implements the SetMonitoringMode service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_SetMonitoringMode(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    const OpcUa_RequestHeader*  a_pRequestHeader,
    uint32_t                    a_nSubscriptionId,
    OpcUa_MonitoringMode        a_eMonitoringMode,
    int32_t                     a_nNoOfMonitoredItemIds,
    const uint32_t*             a_pMonitoredItemIds,
    OpcUa_ResponseHeader*       a_pResponseHeader,
    int32_t*                    a_pNoOfResults,
    SOPC_StatusCode**           a_pResults,
    int32_t*                    a_pNoOfDiagnosticInfos,
    SOPC_DiagnosticInfo**       a_pDiagnosticInfos)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;

    /* validate arguments. */
    if(a_hEndpoint != NULL && a_hContext != NULL){
        if( a_pRequestHeader != NULL
           &&  (a_pMonitoredItemIds != NULL || a_nNoOfMonitoredItemIds <= 0)
           &&  a_pResponseHeader != NULL
           &&  a_pNoOfResults != NULL
           &&  a_pResults != NULL
           &&  a_pNoOfDiagnosticInfos != NULL
           &&  a_pDiagnosticInfos != NULL)
            status = STATUS_OK;
        (void) a_nSubscriptionId;
        (void) a_eMonitoringMode;

    }

    status = OpcUa_BadNotImplemented;

    return status;
}

/*============================================================================
 * Begins processing of a SetMonitoringMode service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginSetMonitoringMode(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void*                       a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_SetMonitoringModeRequest* pRequest = NULL;
    OpcUa_SetMonitoringModeResponse pResponse;
    OpcUa_SetMonitoringModeResponse_Initialize(&pResponse);
    OpcUa_ServerApi_PfnSetMonitoringMode* pfnInvoke = NULL;

    if(a_hEndpoint != NULL && a_hContext != NULL
       && a_ppRequest != NULL
       && a_pRequestType != NULL){
        status = STATUS_OK;
    }
    
    if(a_pRequestType->TypeId != OpcUaId_SetMonitoringModeRequest){
        status = OpcUa_BadInvalidArgument;
    }

    if(status == STATUS_OK){
        pRequest = (OpcUa_SetMonitoringModeRequest*)a_ppRequest;
    }

    if(status == STATUS_OK){
        /* get the function that implements the service call. */
        status = SOPC_Endpoint_GetServiceFunction(a_hEndpoint, a_hContext,
                                                  (SOPC_InvokeService**)&pfnInvoke);
    }

    if(status == STATUS_OK){
    /* invoke the service */
        status = pfnInvoke(
            a_hEndpoint,
            a_hContext,
            &pRequest->RequestHeader,
            pRequest->SubscriptionId,
            pRequest->MonitoringMode,
            pRequest->NoOfMonitoredItemIds,
            pRequest->MonitoredItemIds,
            &pResponse.ResponseHeader,
            &pResponse.NoOfResults,
            &pResponse.Results,
            &pResponse.NoOfDiagnosticInfos,
            &pResponse.DiagnosticInfos);
    }
    
    if (status != STATUS_OK)
    {
        SOPC_StatusCode faultStatus;
        OpcUa_ServiceFault faultObj;
        OpcUa_ServiceFault_Initialize(&faultObj);
        /* create a fault */
        faultStatus = SOPC_ServerApi_CreateFault(&pRequest->RequestHeader,
                                                 status,
                                                 &pResponse.ResponseHeader.ServiceDiagnostics,
                                                 &pResponse.ResponseHeader.NoOfStringTable,
                                                 &pResponse.ResponseHeader.StringTable,
                                                 &faultObj);
        
        if(faultStatus == STATUS_OK){
            /* send the response */
            faultStatus = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                                     &OpcUa_ServiceFault_EncodeableType, &pResponse,
                                                     &a_hContext);
        }
    }

    if(status == STATUS_OK){
        /* send the response */
        status = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                            &OpcUa_SetMonitoringModeResponse_EncodeableType, &pResponse,
                                            &a_hContext);
    }
    return status;
}

/*============================================================================
 * The service dispatch information SetMonitoringMode service.
 *===========================================================================*/
struct SOPC_ServiceType OpcUa_SetMonitoringMode_ServiceType =
{
    OpcUaId_SetMonitoringModeRequest,
    &OpcUa_SetMonitoringModeResponse_EncodeableType,
    OpcUa_Server_BeginSetMonitoringMode,
    (SOPC_InvokeService*)OpcUa_ServerApi_SetMonitoringMode
};
#endif

#ifndef OPCUA_EXCLUDE_SetTriggering
/*============================================================================
 * A pointer to a function that implements the SetTriggering service.
 *===========================================================================*/
typedef SOPC_StatusCode (OpcUa_ServerApi_PfnSetTriggering)(
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
 * A stub method which implements the SetTriggering service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_SetTriggering(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    const OpcUa_RequestHeader*  a_pRequestHeader,
    uint32_t                    a_nSubscriptionId,
    uint32_t                    a_nTriggeringItemId,
    int32_t                     a_nNoOfLinksToAdd,
    const uint32_t*             a_pLinksToAdd,
    int32_t                     a_nNoOfLinksToRemove,
    const uint32_t*             a_pLinksToRemove,
    OpcUa_ResponseHeader*       a_pResponseHeader,
    int32_t*                    a_pNoOfAddResults,
    SOPC_StatusCode**           a_pAddResults,
    int32_t*                    a_pNoOfAddDiagnosticInfos,
    SOPC_DiagnosticInfo**       a_pAddDiagnosticInfos,
    int32_t*                    a_pNoOfRemoveResults,
    SOPC_StatusCode**           a_pRemoveResults,
    int32_t*                    a_pNoOfRemoveDiagnosticInfos,
    SOPC_DiagnosticInfo**       a_pRemoveDiagnosticInfos)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;

    /* validate arguments. */
    if(a_hEndpoint != NULL && a_hContext != NULL){
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
        (void) a_nSubscriptionId;
        (void) a_nTriggeringItemId;

    }

    status = OpcUa_BadNotImplemented;

    return status;
}

/*============================================================================
 * Begins processing of a SetTriggering service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginSetTriggering(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void*                       a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_SetTriggeringRequest* pRequest = NULL;
    OpcUa_SetTriggeringResponse pResponse;
    OpcUa_SetTriggeringResponse_Initialize(&pResponse);
    OpcUa_ServerApi_PfnSetTriggering* pfnInvoke = NULL;

    if(a_hEndpoint != NULL && a_hContext != NULL
       && a_ppRequest != NULL
       && a_pRequestType != NULL){
        status = STATUS_OK;
    }
    
    if(a_pRequestType->TypeId != OpcUaId_SetTriggeringRequest){
        status = OpcUa_BadInvalidArgument;
    }

    if(status == STATUS_OK){
        pRequest = (OpcUa_SetTriggeringRequest*)a_ppRequest;
    }

    if(status == STATUS_OK){
        /* get the function that implements the service call. */
        status = SOPC_Endpoint_GetServiceFunction(a_hEndpoint, a_hContext,
                                                  (SOPC_InvokeService**)&pfnInvoke);
    }

    if(status == STATUS_OK){
    /* invoke the service */
        status = pfnInvoke(
            a_hEndpoint,
            a_hContext,
            &pRequest->RequestHeader,
            pRequest->SubscriptionId,
            pRequest->TriggeringItemId,
            pRequest->NoOfLinksToAdd,
            pRequest->LinksToAdd,
            pRequest->NoOfLinksToRemove,
            pRequest->LinksToRemove,
            &pResponse.ResponseHeader,
            &pResponse.NoOfAddResults,
            &pResponse.AddResults,
            &pResponse.NoOfAddDiagnosticInfos,
            &pResponse.AddDiagnosticInfos,
            &pResponse.NoOfRemoveResults,
            &pResponse.RemoveResults,
            &pResponse.NoOfRemoveDiagnosticInfos,
            &pResponse.RemoveDiagnosticInfos);
    }
    
    if (status != STATUS_OK)
    {
        SOPC_StatusCode faultStatus;
        OpcUa_ServiceFault faultObj;
        OpcUa_ServiceFault_Initialize(&faultObj);
        /* create a fault */
        faultStatus = SOPC_ServerApi_CreateFault(&pRequest->RequestHeader,
                                                 status,
                                                 &pResponse.ResponseHeader.ServiceDiagnostics,
                                                 &pResponse.ResponseHeader.NoOfStringTable,
                                                 &pResponse.ResponseHeader.StringTable,
                                                 &faultObj);
        
        if(faultStatus == STATUS_OK){
            /* send the response */
            faultStatus = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                                     &OpcUa_ServiceFault_EncodeableType, &pResponse,
                                                     &a_hContext);
        }
    }

    if(status == STATUS_OK){
        /* send the response */
        status = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                            &OpcUa_SetTriggeringResponse_EncodeableType, &pResponse,
                                            &a_hContext);
    }
    return status;
}

/*============================================================================
 * The service dispatch information SetTriggering service.
 *===========================================================================*/
struct SOPC_ServiceType OpcUa_SetTriggering_ServiceType =
{
    OpcUaId_SetTriggeringRequest,
    &OpcUa_SetTriggeringResponse_EncodeableType,
    OpcUa_Server_BeginSetTriggering,
    (SOPC_InvokeService*)OpcUa_ServerApi_SetTriggering
};
#endif

#ifndef OPCUA_EXCLUDE_DeleteMonitoredItems
/*============================================================================
 * A pointer to a function that implements the DeleteMonitoredItems service.
 *===========================================================================*/
typedef SOPC_StatusCode (OpcUa_ServerApi_PfnDeleteMonitoredItems)(
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
 * A stub method which implements the DeleteMonitoredItems service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_DeleteMonitoredItems(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    const OpcUa_RequestHeader*  a_pRequestHeader,
    uint32_t                    a_nSubscriptionId,
    int32_t                     a_nNoOfMonitoredItemIds,
    const uint32_t*             a_pMonitoredItemIds,
    OpcUa_ResponseHeader*       a_pResponseHeader,
    int32_t*                    a_pNoOfResults,
    SOPC_StatusCode**           a_pResults,
    int32_t*                    a_pNoOfDiagnosticInfos,
    SOPC_DiagnosticInfo**       a_pDiagnosticInfos)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;

    /* validate arguments. */
    if(a_hEndpoint != NULL && a_hContext != NULL){
        if( a_pRequestHeader != NULL
           &&  (a_pMonitoredItemIds != NULL || a_nNoOfMonitoredItemIds <= 0)
           &&  a_pResponseHeader != NULL
           &&  a_pNoOfResults != NULL
           &&  a_pResults != NULL
           &&  a_pNoOfDiagnosticInfos != NULL
           &&  a_pDiagnosticInfos != NULL)
            status = STATUS_OK;
        (void) a_nSubscriptionId;

    }

    status = OpcUa_BadNotImplemented;

    return status;
}

/*============================================================================
 * Begins processing of a DeleteMonitoredItems service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginDeleteMonitoredItems(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void*                       a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_DeleteMonitoredItemsRequest* pRequest = NULL;
    OpcUa_DeleteMonitoredItemsResponse pResponse;
    OpcUa_DeleteMonitoredItemsResponse_Initialize(&pResponse);
    OpcUa_ServerApi_PfnDeleteMonitoredItems* pfnInvoke = NULL;

    if(a_hEndpoint != NULL && a_hContext != NULL
       && a_ppRequest != NULL
       && a_pRequestType != NULL){
        status = STATUS_OK;
    }
    
    if(a_pRequestType->TypeId != OpcUaId_DeleteMonitoredItemsRequest){
        status = OpcUa_BadInvalidArgument;
    }

    if(status == STATUS_OK){
        pRequest = (OpcUa_DeleteMonitoredItemsRequest*)a_ppRequest;
    }

    if(status == STATUS_OK){
        /* get the function that implements the service call. */
        status = SOPC_Endpoint_GetServiceFunction(a_hEndpoint, a_hContext,
                                                  (SOPC_InvokeService**)&pfnInvoke);
    }

    if(status == STATUS_OK){
    /* invoke the service */
        status = pfnInvoke(
            a_hEndpoint,
            a_hContext,
            &pRequest->RequestHeader,
            pRequest->SubscriptionId,
            pRequest->NoOfMonitoredItemIds,
            pRequest->MonitoredItemIds,
            &pResponse.ResponseHeader,
            &pResponse.NoOfResults,
            &pResponse.Results,
            &pResponse.NoOfDiagnosticInfos,
            &pResponse.DiagnosticInfos);
    }
    
    if (status != STATUS_OK)
    {
        SOPC_StatusCode faultStatus;
        OpcUa_ServiceFault faultObj;
        OpcUa_ServiceFault_Initialize(&faultObj);
        /* create a fault */
        faultStatus = SOPC_ServerApi_CreateFault(&pRequest->RequestHeader,
                                                 status,
                                                 &pResponse.ResponseHeader.ServiceDiagnostics,
                                                 &pResponse.ResponseHeader.NoOfStringTable,
                                                 &pResponse.ResponseHeader.StringTable,
                                                 &faultObj);
        
        if(faultStatus == STATUS_OK){
            /* send the response */
            faultStatus = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                                     &OpcUa_ServiceFault_EncodeableType, &pResponse,
                                                     &a_hContext);
        }
    }

    if(status == STATUS_OK){
        /* send the response */
        status = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                            &OpcUa_DeleteMonitoredItemsResponse_EncodeableType, &pResponse,
                                            &a_hContext);
    }
    return status;
}

/*============================================================================
 * The service dispatch information DeleteMonitoredItems service.
 *===========================================================================*/
struct SOPC_ServiceType OpcUa_DeleteMonitoredItems_ServiceType =
{
    OpcUaId_DeleteMonitoredItemsRequest,
    &OpcUa_DeleteMonitoredItemsResponse_EncodeableType,
    OpcUa_Server_BeginDeleteMonitoredItems,
    (SOPC_InvokeService*)OpcUa_ServerApi_DeleteMonitoredItems
};
#endif

#ifndef OPCUA_EXCLUDE_CreateSubscription
/*============================================================================
 * A pointer to a function that implements the CreateSubscription service.
 *===========================================================================*/
typedef SOPC_StatusCode (OpcUa_ServerApi_PfnCreateSubscription)(
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
 * A stub method which implements the CreateSubscription service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_CreateSubscription(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    const OpcUa_RequestHeader*  a_pRequestHeader,
    double                      a_nRequestedPublishingInterval,
    uint32_t                    a_nRequestedLifetimeCount,
    uint32_t                    a_nRequestedMaxKeepAliveCount,
    uint32_t                    a_nMaxNotificationsPerPublish,
    SOPC_Boolean                a_bPublishingEnabled,
    SOPC_Byte                   a_nPriority,
    OpcUa_ResponseHeader*       a_pResponseHeader,
    uint32_t*                   a_pSubscriptionId,
    double*                     a_pRevisedPublishingInterval,
    uint32_t*                   a_pRevisedLifetimeCount,
    uint32_t*                   a_pRevisedMaxKeepAliveCount)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;

    /* validate arguments. */
    if(a_hEndpoint != NULL && a_hContext != NULL){
        if( a_pRequestHeader != NULL
           &&  a_pResponseHeader != NULL
           &&  a_pSubscriptionId != NULL
           &&  a_pRevisedPublishingInterval != NULL
           &&  a_pRevisedLifetimeCount != NULL
           &&  a_pRevisedMaxKeepAliveCount != NULL)
            status = STATUS_OK;
        (void) a_nRequestedPublishingInterval;
        (void) a_nRequestedLifetimeCount;
        (void) a_nRequestedMaxKeepAliveCount;
        (void) a_nMaxNotificationsPerPublish;
        (void) a_bPublishingEnabled;
        (void) a_nPriority;

    }

    status = OpcUa_BadNotImplemented;

    return status;
}

/*============================================================================
 * Begins processing of a CreateSubscription service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginCreateSubscription(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void*                       a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_CreateSubscriptionRequest* pRequest = NULL;
    OpcUa_CreateSubscriptionResponse pResponse;
    OpcUa_CreateSubscriptionResponse_Initialize(&pResponse);
    OpcUa_ServerApi_PfnCreateSubscription* pfnInvoke = NULL;

    if(a_hEndpoint != NULL && a_hContext != NULL
       && a_ppRequest != NULL
       && a_pRequestType != NULL){
        status = STATUS_OK;
    }
    
    if(a_pRequestType->TypeId != OpcUaId_CreateSubscriptionRequest){
        status = OpcUa_BadInvalidArgument;
    }

    if(status == STATUS_OK){
        pRequest = (OpcUa_CreateSubscriptionRequest*)a_ppRequest;
    }

    if(status == STATUS_OK){
        /* get the function that implements the service call. */
        status = SOPC_Endpoint_GetServiceFunction(a_hEndpoint, a_hContext,
                                                  (SOPC_InvokeService**)&pfnInvoke);
    }

    if(status == STATUS_OK){
    /* invoke the service */
        status = pfnInvoke(
            a_hEndpoint,
            a_hContext,
            &pRequest->RequestHeader,
            pRequest->RequestedPublishingInterval,
            pRequest->RequestedLifetimeCount,
            pRequest->RequestedMaxKeepAliveCount,
            pRequest->MaxNotificationsPerPublish,
            pRequest->PublishingEnabled,
            pRequest->Priority,
            &pResponse.ResponseHeader,
            &pResponse.SubscriptionId,
            &pResponse.RevisedPublishingInterval,
            &pResponse.RevisedLifetimeCount,
            &pResponse.RevisedMaxKeepAliveCount);
    }
    
    if (status != STATUS_OK)
    {
        SOPC_StatusCode faultStatus;
        OpcUa_ServiceFault faultObj;
        OpcUa_ServiceFault_Initialize(&faultObj);
        /* create a fault */
        faultStatus = SOPC_ServerApi_CreateFault(&pRequest->RequestHeader,
                                                 status,
                                                 &pResponse.ResponseHeader.ServiceDiagnostics,
                                                 &pResponse.ResponseHeader.NoOfStringTable,
                                                 &pResponse.ResponseHeader.StringTable,
                                                 &faultObj);
        
        if(faultStatus == STATUS_OK){
            /* send the response */
            faultStatus = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                                     &OpcUa_ServiceFault_EncodeableType, &pResponse,
                                                     &a_hContext);
        }
    }

    if(status == STATUS_OK){
        /* send the response */
        status = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                            &OpcUa_CreateSubscriptionResponse_EncodeableType, &pResponse,
                                            &a_hContext);
    }
    return status;
}

/*============================================================================
 * The service dispatch information CreateSubscription service.
 *===========================================================================*/
struct SOPC_ServiceType OpcUa_CreateSubscription_ServiceType =
{
    OpcUaId_CreateSubscriptionRequest,
    &OpcUa_CreateSubscriptionResponse_EncodeableType,
    OpcUa_Server_BeginCreateSubscription,
    (SOPC_InvokeService*)OpcUa_ServerApi_CreateSubscription
};
#endif

#ifndef OPCUA_EXCLUDE_ModifySubscription
/*============================================================================
 * A pointer to a function that implements the ModifySubscription service.
 *===========================================================================*/
typedef SOPC_StatusCode (OpcUa_ServerApi_PfnModifySubscription)(
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
 * A stub method which implements the ModifySubscription service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_ModifySubscription(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    const OpcUa_RequestHeader*  a_pRequestHeader,
    uint32_t                    a_nSubscriptionId,
    double                      a_nRequestedPublishingInterval,
    uint32_t                    a_nRequestedLifetimeCount,
    uint32_t                    a_nRequestedMaxKeepAliveCount,
    uint32_t                    a_nMaxNotificationsPerPublish,
    SOPC_Byte                   a_nPriority,
    OpcUa_ResponseHeader*       a_pResponseHeader,
    double*                     a_pRevisedPublishingInterval,
    uint32_t*                   a_pRevisedLifetimeCount,
    uint32_t*                   a_pRevisedMaxKeepAliveCount)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;

    /* validate arguments. */
    if(a_hEndpoint != NULL && a_hContext != NULL){
        if( a_pRequestHeader != NULL
           &&  a_pResponseHeader != NULL
           &&  a_pRevisedPublishingInterval != NULL
           &&  a_pRevisedLifetimeCount != NULL
           &&  a_pRevisedMaxKeepAliveCount != NULL)
            status = STATUS_OK;
        (void) a_nSubscriptionId;
        (void) a_nRequestedPublishingInterval;
        (void) a_nRequestedLifetimeCount;
        (void) a_nRequestedMaxKeepAliveCount;
        (void) a_nMaxNotificationsPerPublish;
        (void) a_nPriority;

    }

    status = OpcUa_BadNotImplemented;

    return status;
}

/*============================================================================
 * Begins processing of a ModifySubscription service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginModifySubscription(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void*                       a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_ModifySubscriptionRequest* pRequest = NULL;
    OpcUa_ModifySubscriptionResponse pResponse;
    OpcUa_ModifySubscriptionResponse_Initialize(&pResponse);
    OpcUa_ServerApi_PfnModifySubscription* pfnInvoke = NULL;

    if(a_hEndpoint != NULL && a_hContext != NULL
       && a_ppRequest != NULL
       && a_pRequestType != NULL){
        status = STATUS_OK;
    }
    
    if(a_pRequestType->TypeId != OpcUaId_ModifySubscriptionRequest){
        status = OpcUa_BadInvalidArgument;
    }

    if(status == STATUS_OK){
        pRequest = (OpcUa_ModifySubscriptionRequest*)a_ppRequest;
    }

    if(status == STATUS_OK){
        /* get the function that implements the service call. */
        status = SOPC_Endpoint_GetServiceFunction(a_hEndpoint, a_hContext,
                                                  (SOPC_InvokeService**)&pfnInvoke);
    }

    if(status == STATUS_OK){
    /* invoke the service */
        status = pfnInvoke(
            a_hEndpoint,
            a_hContext,
            &pRequest->RequestHeader,
            pRequest->SubscriptionId,
            pRequest->RequestedPublishingInterval,
            pRequest->RequestedLifetimeCount,
            pRequest->RequestedMaxKeepAliveCount,
            pRequest->MaxNotificationsPerPublish,
            pRequest->Priority,
            &pResponse.ResponseHeader,
            &pResponse.RevisedPublishingInterval,
            &pResponse.RevisedLifetimeCount,
            &pResponse.RevisedMaxKeepAliveCount);
    }
    
    if (status != STATUS_OK)
    {
        SOPC_StatusCode faultStatus;
        OpcUa_ServiceFault faultObj;
        OpcUa_ServiceFault_Initialize(&faultObj);
        /* create a fault */
        faultStatus = SOPC_ServerApi_CreateFault(&pRequest->RequestHeader,
                                                 status,
                                                 &pResponse.ResponseHeader.ServiceDiagnostics,
                                                 &pResponse.ResponseHeader.NoOfStringTable,
                                                 &pResponse.ResponseHeader.StringTable,
                                                 &faultObj);
        
        if(faultStatus == STATUS_OK){
            /* send the response */
            faultStatus = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                                     &OpcUa_ServiceFault_EncodeableType, &pResponse,
                                                     &a_hContext);
        }
    }

    if(status == STATUS_OK){
        /* send the response */
        status = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                            &OpcUa_ModifySubscriptionResponse_EncodeableType, &pResponse,
                                            &a_hContext);
    }
    return status;
}

/*============================================================================
 * The service dispatch information ModifySubscription service.
 *===========================================================================*/
struct SOPC_ServiceType OpcUa_ModifySubscription_ServiceType =
{
    OpcUaId_ModifySubscriptionRequest,
    &OpcUa_ModifySubscriptionResponse_EncodeableType,
    OpcUa_Server_BeginModifySubscription,
    (SOPC_InvokeService*)OpcUa_ServerApi_ModifySubscription
};
#endif

#ifndef OPCUA_EXCLUDE_SetPublishingMode
/*============================================================================
 * A pointer to a function that implements the SetPublishingMode service.
 *===========================================================================*/
typedef SOPC_StatusCode (OpcUa_ServerApi_PfnSetPublishingMode)(
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
 * A stub method which implements the SetPublishingMode service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_SetPublishingMode(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    const OpcUa_RequestHeader*  a_pRequestHeader,
    SOPC_Boolean                a_bPublishingEnabled,
    int32_t                     a_nNoOfSubscriptionIds,
    const uint32_t*             a_pSubscriptionIds,
    OpcUa_ResponseHeader*       a_pResponseHeader,
    int32_t*                    a_pNoOfResults,
    SOPC_StatusCode**           a_pResults,
    int32_t*                    a_pNoOfDiagnosticInfos,
    SOPC_DiagnosticInfo**       a_pDiagnosticInfos)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;

    /* validate arguments. */
    if(a_hEndpoint != NULL && a_hContext != NULL){
        if( a_pRequestHeader != NULL
           &&  (a_pSubscriptionIds != NULL || a_nNoOfSubscriptionIds <= 0)
           &&  a_pResponseHeader != NULL
           &&  a_pNoOfResults != NULL
           &&  a_pResults != NULL
           &&  a_pNoOfDiagnosticInfos != NULL
           &&  a_pDiagnosticInfos != NULL)
            status = STATUS_OK;
        (void) a_bPublishingEnabled;

    }

    status = OpcUa_BadNotImplemented;

    return status;
}

/*============================================================================
 * Begins processing of a SetPublishingMode service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginSetPublishingMode(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void*                       a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_SetPublishingModeRequest* pRequest = NULL;
    OpcUa_SetPublishingModeResponse pResponse;
    OpcUa_SetPublishingModeResponse_Initialize(&pResponse);
    OpcUa_ServerApi_PfnSetPublishingMode* pfnInvoke = NULL;

    if(a_hEndpoint != NULL && a_hContext != NULL
       && a_ppRequest != NULL
       && a_pRequestType != NULL){
        status = STATUS_OK;
    }
    
    if(a_pRequestType->TypeId != OpcUaId_SetPublishingModeRequest){
        status = OpcUa_BadInvalidArgument;
    }

    if(status == STATUS_OK){
        pRequest = (OpcUa_SetPublishingModeRequest*)a_ppRequest;
    }

    if(status == STATUS_OK){
        /* get the function that implements the service call. */
        status = SOPC_Endpoint_GetServiceFunction(a_hEndpoint, a_hContext,
                                                  (SOPC_InvokeService**)&pfnInvoke);
    }

    if(status == STATUS_OK){
    /* invoke the service */
        status = pfnInvoke(
            a_hEndpoint,
            a_hContext,
            &pRequest->RequestHeader,
            pRequest->PublishingEnabled,
            pRequest->NoOfSubscriptionIds,
            pRequest->SubscriptionIds,
            &pResponse.ResponseHeader,
            &pResponse.NoOfResults,
            &pResponse.Results,
            &pResponse.NoOfDiagnosticInfos,
            &pResponse.DiagnosticInfos);
    }
    
    if (status != STATUS_OK)
    {
        SOPC_StatusCode faultStatus;
        OpcUa_ServiceFault faultObj;
        OpcUa_ServiceFault_Initialize(&faultObj);
        /* create a fault */
        faultStatus = SOPC_ServerApi_CreateFault(&pRequest->RequestHeader,
                                                 status,
                                                 &pResponse.ResponseHeader.ServiceDiagnostics,
                                                 &pResponse.ResponseHeader.NoOfStringTable,
                                                 &pResponse.ResponseHeader.StringTable,
                                                 &faultObj);
        
        if(faultStatus == STATUS_OK){
            /* send the response */
            faultStatus = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                                     &OpcUa_ServiceFault_EncodeableType, &pResponse,
                                                     &a_hContext);
        }
    }

    if(status == STATUS_OK){
        /* send the response */
        status = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                            &OpcUa_SetPublishingModeResponse_EncodeableType, &pResponse,
                                            &a_hContext);
    }
    return status;
}

/*============================================================================
 * The service dispatch information SetPublishingMode service.
 *===========================================================================*/
struct SOPC_ServiceType OpcUa_SetPublishingMode_ServiceType =
{
    OpcUaId_SetPublishingModeRequest,
    &OpcUa_SetPublishingModeResponse_EncodeableType,
    OpcUa_Server_BeginSetPublishingMode,
    (SOPC_InvokeService*)OpcUa_ServerApi_SetPublishingMode
};
#endif

#ifndef OPCUA_EXCLUDE_Publish
/*============================================================================
 * A pointer to a function that implements the Publish service.
 *===========================================================================*/
typedef SOPC_StatusCode (OpcUa_ServerApi_PfnPublish)(
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
 * A stub method which implements the Publish service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_Publish(
    SOPC_Endpoint                            a_hEndpoint,
    struct SOPC_RequestContext*              a_hContext,
    const OpcUa_RequestHeader*               a_pRequestHeader,
    int32_t                                  a_nNoOfSubscriptionAcknowledgements,
    const OpcUa_SubscriptionAcknowledgement* a_pSubscriptionAcknowledgements,
    OpcUa_ResponseHeader*                    a_pResponseHeader,
    uint32_t*                                a_pSubscriptionId,
    int32_t*                                 a_pNoOfAvailableSequenceNumbers,
    uint32_t**                               a_pAvailableSequenceNumbers,
    SOPC_Boolean*                            a_pMoreNotifications,
    OpcUa_NotificationMessage*               a_pNotificationMessage,
    int32_t*                                 a_pNoOfResults,
    SOPC_StatusCode**                        a_pResults,
    int32_t*                                 a_pNoOfDiagnosticInfos,
    SOPC_DiagnosticInfo**                    a_pDiagnosticInfos)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;

    /* validate arguments. */
    if(a_hEndpoint != NULL && a_hContext != NULL){
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

    }

    status = OpcUa_BadNotImplemented;

    return status;
}

/*============================================================================
 * Begins processing of a Publish service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginPublish(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void*                       a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_PublishRequest* pRequest = NULL;
    OpcUa_PublishResponse pResponse;
    OpcUa_PublishResponse_Initialize(&pResponse);
    OpcUa_ServerApi_PfnPublish* pfnInvoke = NULL;

    if(a_hEndpoint != NULL && a_hContext != NULL
       && a_ppRequest != NULL
       && a_pRequestType != NULL){
        status = STATUS_OK;
    }
    
    if(a_pRequestType->TypeId != OpcUaId_PublishRequest){
        status = OpcUa_BadInvalidArgument;
    }

    if(status == STATUS_OK){
        pRequest = (OpcUa_PublishRequest*)a_ppRequest;
    }

    if(status == STATUS_OK){
        /* get the function that implements the service call. */
        status = SOPC_Endpoint_GetServiceFunction(a_hEndpoint, a_hContext,
                                                  (SOPC_InvokeService**)&pfnInvoke);
    }

    if(status == STATUS_OK){
    /* invoke the service */
        status = pfnInvoke(
            a_hEndpoint,
            a_hContext,
            &pRequest->RequestHeader,
            pRequest->NoOfSubscriptionAcknowledgements,
            pRequest->SubscriptionAcknowledgements,
            &pResponse.ResponseHeader,
            &pResponse.SubscriptionId,
            &pResponse.NoOfAvailableSequenceNumbers,
            &pResponse.AvailableSequenceNumbers,
            &pResponse.MoreNotifications,
            &pResponse.NotificationMessage,
            &pResponse.NoOfResults,
            &pResponse.Results,
            &pResponse.NoOfDiagnosticInfos,
            &pResponse.DiagnosticInfos);
    }
    
    if (status != STATUS_OK)
    {
        SOPC_StatusCode faultStatus;
        OpcUa_ServiceFault faultObj;
        OpcUa_ServiceFault_Initialize(&faultObj);
        /* create a fault */
        faultStatus = SOPC_ServerApi_CreateFault(&pRequest->RequestHeader,
                                                 status,
                                                 &pResponse.ResponseHeader.ServiceDiagnostics,
                                                 &pResponse.ResponseHeader.NoOfStringTable,
                                                 &pResponse.ResponseHeader.StringTable,
                                                 &faultObj);
        
        if(faultStatus == STATUS_OK){
            /* send the response */
            faultStatus = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                                     &OpcUa_ServiceFault_EncodeableType, &pResponse,
                                                     &a_hContext);
        }
    }

    if(status == STATUS_OK){
        /* send the response */
        status = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                            &OpcUa_PublishResponse_EncodeableType, &pResponse,
                                            &a_hContext);
    }
    return status;
}

/*============================================================================
 * The service dispatch information Publish service.
 *===========================================================================*/
struct SOPC_ServiceType OpcUa_Publish_ServiceType =
{
    OpcUaId_PublishRequest,
    &OpcUa_PublishResponse_EncodeableType,
    OpcUa_Server_BeginPublish,
    (SOPC_InvokeService*)OpcUa_ServerApi_Publish
};
#endif

#ifndef OPCUA_EXCLUDE_Republish
/*============================================================================
 * A pointer to a function that implements the Republish service.
 *===========================================================================*/
typedef SOPC_StatusCode (OpcUa_ServerApi_PfnRepublish)(
    SOPC_Endpoint               hEndpoint,
    struct SOPC_RequestContext* hContext,
    const OpcUa_RequestHeader*  pRequestHeader,
    uint32_t                    nSubscriptionId,
    uint32_t                    nRetransmitSequenceNumber,
    OpcUa_ResponseHeader*       pResponseHeader,
    OpcUa_NotificationMessage*  pNotificationMessage);

/*============================================================================
 * A stub method which implements the Republish service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_Republish(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    const OpcUa_RequestHeader*  a_pRequestHeader,
    uint32_t                    a_nSubscriptionId,
    uint32_t                    a_nRetransmitSequenceNumber,
    OpcUa_ResponseHeader*       a_pResponseHeader,
    OpcUa_NotificationMessage*  a_pNotificationMessage)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;

    /* validate arguments. */
    if(a_hEndpoint != NULL && a_hContext != NULL){
        if( a_pRequestHeader != NULL
           &&  a_pResponseHeader != NULL
           &&  a_pNotificationMessage != NULL)
            status = STATUS_OK;
        (void) a_nSubscriptionId;
        (void) a_nRetransmitSequenceNumber;

    }

    status = OpcUa_BadNotImplemented;

    return status;
}

/*============================================================================
 * Begins processing of a Republish service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginRepublish(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void*                       a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_RepublishRequest* pRequest = NULL;
    OpcUa_RepublishResponse pResponse;
    OpcUa_RepublishResponse_Initialize(&pResponse);
    OpcUa_ServerApi_PfnRepublish* pfnInvoke = NULL;

    if(a_hEndpoint != NULL && a_hContext != NULL
       && a_ppRequest != NULL
       && a_pRequestType != NULL){
        status = STATUS_OK;
    }
    
    if(a_pRequestType->TypeId != OpcUaId_RepublishRequest){
        status = OpcUa_BadInvalidArgument;
    }

    if(status == STATUS_OK){
        pRequest = (OpcUa_RepublishRequest*)a_ppRequest;
    }

    if(status == STATUS_OK){
        /* get the function that implements the service call. */
        status = SOPC_Endpoint_GetServiceFunction(a_hEndpoint, a_hContext,
                                                  (SOPC_InvokeService**)&pfnInvoke);
    }

    if(status == STATUS_OK){
    /* invoke the service */
        status = pfnInvoke(
            a_hEndpoint,
            a_hContext,
            &pRequest->RequestHeader,
            pRequest->SubscriptionId,
            pRequest->RetransmitSequenceNumber,
            &pResponse.ResponseHeader,
            &pResponse.NotificationMessage);
    }
    
    if (status != STATUS_OK)
    {
        SOPC_StatusCode faultStatus;
        OpcUa_ServiceFault faultObj;
        OpcUa_ServiceFault_Initialize(&faultObj);
        /* create a fault */
        faultStatus = SOPC_ServerApi_CreateFault(&pRequest->RequestHeader,
                                                 status,
                                                 &pResponse.ResponseHeader.ServiceDiagnostics,
                                                 &pResponse.ResponseHeader.NoOfStringTable,
                                                 &pResponse.ResponseHeader.StringTable,
                                                 &faultObj);
        
        if(faultStatus == STATUS_OK){
            /* send the response */
            faultStatus = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                                     &OpcUa_ServiceFault_EncodeableType, &pResponse,
                                                     &a_hContext);
        }
    }

    if(status == STATUS_OK){
        /* send the response */
        status = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                            &OpcUa_RepublishResponse_EncodeableType, &pResponse,
                                            &a_hContext);
    }
    return status;
}

/*============================================================================
 * The service dispatch information Republish service.
 *===========================================================================*/
struct SOPC_ServiceType OpcUa_Republish_ServiceType =
{
    OpcUaId_RepublishRequest,
    &OpcUa_RepublishResponse_EncodeableType,
    OpcUa_Server_BeginRepublish,
    (SOPC_InvokeService*)OpcUa_ServerApi_Republish
};
#endif

#ifndef OPCUA_EXCLUDE_TransferSubscriptions
/*============================================================================
 * A pointer to a function that implements the TransferSubscriptions service.
 *===========================================================================*/
typedef SOPC_StatusCode (OpcUa_ServerApi_PfnTransferSubscriptions)(
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
 * A stub method which implements the TransferSubscriptions service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_TransferSubscriptions(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    const OpcUa_RequestHeader*  a_pRequestHeader,
    int32_t                     a_nNoOfSubscriptionIds,
    const uint32_t*             a_pSubscriptionIds,
    SOPC_Boolean                a_bSendInitialValues,
    OpcUa_ResponseHeader*       a_pResponseHeader,
    int32_t*                    a_pNoOfResults,
    OpcUa_TransferResult**      a_pResults,
    int32_t*                    a_pNoOfDiagnosticInfos,
    SOPC_DiagnosticInfo**       a_pDiagnosticInfos)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;

    /* validate arguments. */
    if(a_hEndpoint != NULL && a_hContext != NULL){
        if( a_pRequestHeader != NULL
           &&  (a_pSubscriptionIds != NULL || a_nNoOfSubscriptionIds <= 0)
           &&  a_pResponseHeader != NULL
           &&  a_pNoOfResults != NULL
           &&  a_pResults != NULL
           &&  a_pNoOfDiagnosticInfos != NULL
           &&  a_pDiagnosticInfos != NULL)
            status = STATUS_OK;
        (void) a_bSendInitialValues;

    }

    status = OpcUa_BadNotImplemented;

    return status;
}

/*============================================================================
 * Begins processing of a TransferSubscriptions service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginTransferSubscriptions(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void*                       a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_TransferSubscriptionsRequest* pRequest = NULL;
    OpcUa_TransferSubscriptionsResponse pResponse;
    OpcUa_TransferSubscriptionsResponse_Initialize(&pResponse);
    OpcUa_ServerApi_PfnTransferSubscriptions* pfnInvoke = NULL;

    if(a_hEndpoint != NULL && a_hContext != NULL
       && a_ppRequest != NULL
       && a_pRequestType != NULL){
        status = STATUS_OK;
    }
    
    if(a_pRequestType->TypeId != OpcUaId_TransferSubscriptionsRequest){
        status = OpcUa_BadInvalidArgument;
    }

    if(status == STATUS_OK){
        pRequest = (OpcUa_TransferSubscriptionsRequest*)a_ppRequest;
    }

    if(status == STATUS_OK){
        /* get the function that implements the service call. */
        status = SOPC_Endpoint_GetServiceFunction(a_hEndpoint, a_hContext,
                                                  (SOPC_InvokeService**)&pfnInvoke);
    }

    if(status == STATUS_OK){
    /* invoke the service */
        status = pfnInvoke(
            a_hEndpoint,
            a_hContext,
            &pRequest->RequestHeader,
            pRequest->NoOfSubscriptionIds,
            pRequest->SubscriptionIds,
            pRequest->SendInitialValues,
            &pResponse.ResponseHeader,
            &pResponse.NoOfResults,
            &pResponse.Results,
            &pResponse.NoOfDiagnosticInfos,
            &pResponse.DiagnosticInfos);
    }
    
    if (status != STATUS_OK)
    {
        SOPC_StatusCode faultStatus;
        OpcUa_ServiceFault faultObj;
        OpcUa_ServiceFault_Initialize(&faultObj);
        /* create a fault */
        faultStatus = SOPC_ServerApi_CreateFault(&pRequest->RequestHeader,
                                                 status,
                                                 &pResponse.ResponseHeader.ServiceDiagnostics,
                                                 &pResponse.ResponseHeader.NoOfStringTable,
                                                 &pResponse.ResponseHeader.StringTable,
                                                 &faultObj);
        
        if(faultStatus == STATUS_OK){
            /* send the response */
            faultStatus = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                                     &OpcUa_ServiceFault_EncodeableType, &pResponse,
                                                     &a_hContext);
        }
    }

    if(status == STATUS_OK){
        /* send the response */
        status = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                            &OpcUa_TransferSubscriptionsResponse_EncodeableType, &pResponse,
                                            &a_hContext);
    }
    return status;
}

/*============================================================================
 * The service dispatch information TransferSubscriptions service.
 *===========================================================================*/
struct SOPC_ServiceType OpcUa_TransferSubscriptions_ServiceType =
{
    OpcUaId_TransferSubscriptionsRequest,
    &OpcUa_TransferSubscriptionsResponse_EncodeableType,
    OpcUa_Server_BeginTransferSubscriptions,
    (SOPC_InvokeService*)OpcUa_ServerApi_TransferSubscriptions
};
#endif

#ifndef OPCUA_EXCLUDE_DeleteSubscriptions
/*============================================================================
 * A pointer to a function that implements the DeleteSubscriptions service.
 *===========================================================================*/
typedef SOPC_StatusCode (OpcUa_ServerApi_PfnDeleteSubscriptions)(
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
 * A stub method which implements the DeleteSubscriptions service.
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerApi_DeleteSubscriptions(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    const OpcUa_RequestHeader*  a_pRequestHeader,
    int32_t                     a_nNoOfSubscriptionIds,
    const uint32_t*             a_pSubscriptionIds,
    OpcUa_ResponseHeader*       a_pResponseHeader,
    int32_t*                    a_pNoOfResults,
    SOPC_StatusCode**           a_pResults,
    int32_t*                    a_pNoOfDiagnosticInfos,
    SOPC_DiagnosticInfo**       a_pDiagnosticInfos)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;

    /* validate arguments. */
    if(a_hEndpoint != NULL && a_hContext != NULL){
        if( a_pRequestHeader != NULL
           &&  (a_pSubscriptionIds != NULL || a_nNoOfSubscriptionIds <= 0)
           &&  a_pResponseHeader != NULL
           &&  a_pNoOfResults != NULL
           &&  a_pResults != NULL
           &&  a_pNoOfDiagnosticInfos != NULL
           &&  a_pDiagnosticInfos != NULL)
            status = STATUS_OK;

    }

    status = OpcUa_BadNotImplemented;

    return status;
}

/*============================================================================
 * Begins processing of a DeleteSubscriptions service request.
 *===========================================================================*/
SOPC_StatusCode OpcUa_Server_BeginDeleteSubscriptions(
    SOPC_Endpoint               a_hEndpoint,
    struct SOPC_RequestContext* a_hContext,
    void*                       a_ppRequest,
    SOPC_EncodeableType*        a_pRequestType)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_DeleteSubscriptionsRequest* pRequest = NULL;
    OpcUa_DeleteSubscriptionsResponse pResponse;
    OpcUa_DeleteSubscriptionsResponse_Initialize(&pResponse);
    OpcUa_ServerApi_PfnDeleteSubscriptions* pfnInvoke = NULL;

    if(a_hEndpoint != NULL && a_hContext != NULL
       && a_ppRequest != NULL
       && a_pRequestType != NULL){
        status = STATUS_OK;
    }
    
    if(a_pRequestType->TypeId != OpcUaId_DeleteSubscriptionsRequest){
        status = OpcUa_BadInvalidArgument;
    }

    if(status == STATUS_OK){
        pRequest = (OpcUa_DeleteSubscriptionsRequest*)a_ppRequest;
    }

    if(status == STATUS_OK){
        /* get the function that implements the service call. */
        status = SOPC_Endpoint_GetServiceFunction(a_hEndpoint, a_hContext,
                                                  (SOPC_InvokeService**)&pfnInvoke);
    }

    if(status == STATUS_OK){
    /* invoke the service */
        status = pfnInvoke(
            a_hEndpoint,
            a_hContext,
            &pRequest->RequestHeader,
            pRequest->NoOfSubscriptionIds,
            pRequest->SubscriptionIds,
            &pResponse.ResponseHeader,
            &pResponse.NoOfResults,
            &pResponse.Results,
            &pResponse.NoOfDiagnosticInfos,
            &pResponse.DiagnosticInfos);
    }
    
    if (status != STATUS_OK)
    {
        SOPC_StatusCode faultStatus;
        OpcUa_ServiceFault faultObj;
        OpcUa_ServiceFault_Initialize(&faultObj);
        /* create a fault */
        faultStatus = SOPC_ServerApi_CreateFault(&pRequest->RequestHeader,
                                                 status,
                                                 &pResponse.ResponseHeader.ServiceDiagnostics,
                                                 &pResponse.ResponseHeader.NoOfStringTable,
                                                 &pResponse.ResponseHeader.StringTable,
                                                 &faultObj);
        
        if(faultStatus == STATUS_OK){
            /* send the response */
            faultStatus = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                                     &OpcUa_ServiceFault_EncodeableType, &pResponse,
                                                     &a_hContext);
        }
    }

    if(status == STATUS_OK){
        /* send the response */
        status = SOPC_Endpoint_SendResponse(a_hEndpoint, 
                                            &OpcUa_DeleteSubscriptionsResponse_EncodeableType, &pResponse,
                                            &a_hContext);
    }
    return status;
}

/*============================================================================
 * The service dispatch information DeleteSubscriptions service.
 *===========================================================================*/
struct SOPC_ServiceType OpcUa_DeleteSubscriptions_ServiceType =
{
    OpcUaId_DeleteSubscriptionsRequest,
    &OpcUa_DeleteSubscriptionsResponse_EncodeableType,
    OpcUa_Server_BeginDeleteSubscriptions,
    (SOPC_InvokeService*)OpcUa_ServerApi_DeleteSubscriptions
};
#endif

SOPC_StatusCode SOPC_ServerApi_CreateFault(OpcUa_RequestHeader* requestHeader,
                                           SOPC_StatusCode      status,
                                           SOPC_DiagnosticInfo* serviceDiagnostics,
                                           int32_t*             noOfStringTable,
                                           SOPC_String**        stringTable,
                                           OpcUa_ServiceFault*  faultObj){
    SOPC_StatusCode retStatus = STATUS_INVALID_PARAMETERS;
    if(faultObj != NULL && requestHeader != NULL){
        faultObj->ResponseHeader.RequestHandle = requestHeader->RequestHandle;
        SOPC_DateTime_Clear(&faultObj->ResponseHeader.Timestamp); // TODO: set current date
        faultObj->ResponseHeader.ServiceResult = status;
        faultObj->ResponseHeader.ServiceDiagnostics = *serviceDiagnostics;
        memset(serviceDiagnostics, 0, sizeof(SOPC_DiagnosticInfo)); // erase content since now managed in response
        faultObj->ResponseHeader.NoOfStringTable = *noOfStringTable;
        faultObj->ResponseHeader.StringTable = *stringTable;
        *noOfStringTable = 0; // erase content since now managed in response
        *stringTable = NULL; // erase content since now managed in response
        retStatus = STATUS_OK;
    }
    return retStatus;                                         
}

/*============================================================================
 * Table of standard services.
 *===========================================================================*/
SOPC_ServiceType* SOPC_SupportedServiceTypes[] =
{
    #ifndef OPCUA_EXCLUDE_FindServers
    &OpcUa_FindServers_ServiceType,
    #endif
    #ifndef OPCUA_EXCLUDE_FindServersOnNetwork
    &OpcUa_FindServersOnNetwork_ServiceType,
    #endif
    #ifndef OPCUA_EXCLUDE_GetEndpoints
    &OpcUa_GetEndpoints_ServiceType,
    #endif
    #ifndef OPCUA_EXCLUDE_RegisterServer
    &OpcUa_RegisterServer_ServiceType,
    #endif
    #ifndef OPCUA_EXCLUDE_RegisterServer2
    &OpcUa_RegisterServer2_ServiceType,
    #endif
    #ifndef OPCUA_EXCLUDE_CreateSession
    &OpcUa_CreateSession_ServiceType,
    #endif
    #ifndef OPCUA_EXCLUDE_ActivateSession
    &OpcUa_ActivateSession_ServiceType,
    #endif
    #ifndef OPCUA_EXCLUDE_CloseSession
    &OpcUa_CloseSession_ServiceType,
    #endif
    #ifndef OPCUA_EXCLUDE_Cancel
    &OpcUa_Cancel_ServiceType,
    #endif
    #ifndef OPCUA_EXCLUDE_AddNodes
    &OpcUa_AddNodes_ServiceType,
    #endif
    #ifndef OPCUA_EXCLUDE_AddReferences
    &OpcUa_AddReferences_ServiceType,
    #endif
    #ifndef OPCUA_EXCLUDE_DeleteNodes
    &OpcUa_DeleteNodes_ServiceType,
    #endif
    #ifndef OPCUA_EXCLUDE_DeleteReferences
    &OpcUa_DeleteReferences_ServiceType,
    #endif
    #ifndef OPCUA_EXCLUDE_Browse
    &OpcUa_Browse_ServiceType,
    #endif
    #ifndef OPCUA_EXCLUDE_BrowseNext
    &OpcUa_BrowseNext_ServiceType,
    #endif
    #ifndef OPCUA_EXCLUDE_TranslateBrowsePathsToNodeIds
    &OpcUa_TranslateBrowsePathsToNodeIds_ServiceType,
    #endif
    #ifndef OPCUA_EXCLUDE_RegisterNodes
    &OpcUa_RegisterNodes_ServiceType,
    #endif
    #ifndef OPCUA_EXCLUDE_UnregisterNodes
    &OpcUa_UnregisterNodes_ServiceType,
    #endif
    #ifndef OPCUA_EXCLUDE_QueryFirst
    &OpcUa_QueryFirst_ServiceType,
    #endif
    #ifndef OPCUA_EXCLUDE_QueryNext
    &OpcUa_QueryNext_ServiceType,
    #endif
    #ifndef OPCUA_EXCLUDE_Read
    &OpcUa_Read_ServiceType,
    #endif
    #ifndef OPCUA_EXCLUDE_HistoryRead
    &OpcUa_HistoryRead_ServiceType,
    #endif
    #ifndef OPCUA_EXCLUDE_Write
    &OpcUa_Write_ServiceType,
    #endif
    #ifndef OPCUA_EXCLUDE_HistoryUpdate
    &OpcUa_HistoryUpdate_ServiceType,
    #endif
    #ifndef OPCUA_EXCLUDE_Call
    &OpcUa_Call_ServiceType,
    #endif
    #ifndef OPCUA_EXCLUDE_CreateMonitoredItems
    &OpcUa_CreateMonitoredItems_ServiceType,
    #endif
    #ifndef OPCUA_EXCLUDE_ModifyMonitoredItems
    &OpcUa_ModifyMonitoredItems_ServiceType,
    #endif
    #ifndef OPCUA_EXCLUDE_SetMonitoringMode
    &OpcUa_SetMonitoringMode_ServiceType,
    #endif
    #ifndef OPCUA_EXCLUDE_SetTriggering
    &OpcUa_SetTriggering_ServiceType,
    #endif
    #ifndef OPCUA_EXCLUDE_DeleteMonitoredItems
    &OpcUa_DeleteMonitoredItems_ServiceType,
    #endif
    #ifndef OPCUA_EXCLUDE_CreateSubscription
    &OpcUa_CreateSubscription_ServiceType,
    #endif
    #ifndef OPCUA_EXCLUDE_ModifySubscription
    &OpcUa_ModifySubscription_ServiceType,
    #endif
    #ifndef OPCUA_EXCLUDE_SetPublishingMode
    &OpcUa_SetPublishingMode_ServiceType,
    #endif
    #ifndef OPCUA_EXCLUDE_Publish
    &OpcUa_Publish_ServiceType,
    #endif
    #ifndef OPCUA_EXCLUDE_Republish
    &OpcUa_Republish_ServiceType,
    #endif
    #ifndef OPCUA_EXCLUDE_TransferSubscriptions
    &OpcUa_TransferSubscriptions_ServiceType,
    #endif
    #ifndef OPCUA_EXCLUDE_DeleteSubscriptions
    &OpcUa_DeleteSubscriptions_ServiceType,
    #endif
    NULL
};

#endif /* OPCUA_HAVE_SERVERAPI */
/* This is the last line of an autogenerated file. */
