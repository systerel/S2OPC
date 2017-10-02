/**
 *  \file sopc_endpoint.h
 *
 *  \brief High level API to manipulate an OPC-UA Endpoint and define OPC UA services for the endpoint
 */
/*
 *  Copyright (C) 2016 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SOPC_ENDPOINT_H_
#define SOPC_ENDPOINT_H_

#include "sopc_stack_csts.h"

#ifdef OPCUA_HAVE_SERVERAPI

#include "sopc_encodeabletype.h"
#include "sopc_types.h"
#include "key_manager.h"

/**
 *  \brief Endpoint type definition
 */
typedef void* SOPC_Endpoint;

/**
 *  \brief Request context type necessary to for sending service Response
 */
typedef struct SOPC_RequestContext SOPC_RequestContext;

/**
 *  \brief Endpoint serialization enumeration type
 *  Note: only binary serialization is available in INGPOCS
 */
typedef enum {
    SOPC_EndpointSerializer_Invalid = 0x00,
    SOPC_EndpointSerializer_Binary = 0x01
} SOPC_Endpoint_SerializerType;


#define SECURITY_MODE_NONE_MASK 0x01
#define SECURITY_MODE_SIGN_MASK 0x02
#define SECURITY_MODE_SIGNANDENCRYPT_MASK 0x04
#define SECURITY_MODE_ANY_MASK 0x07

/**
 *  \brief Definition of a security policy supported by endpoint
 */
typedef struct SOPC_SecurityPolicy
{
    SOPC_String securityPolicy; /**< Security policy URI supported */
    uint16_t    securityModes;  /**< Mask of security modes supported (use combination of SECURITY_MODE_*_MASK values) */
    void*       padding;        /**< Binary compatibility */
} SOPC_SecurityPolicy;

/**
 *  \brief Endpoint event enumeration type
 */
typedef enum SOPC_EndpointEvent
{
    SOPC_EndpointEvent_Invalid,
    SOPC_EndpointEvent_SecureChannelOpened,
    SOPC_EndpointEvent_SecureChannelClosed,
    SOPC_EndpointEvent_Renewed,
    SOPC_EndpointEvent_UnsupportedServiceRequested,
    SOPC_EndpointEvent_DecoderError,
    SOPC_EndpointEvent_EndpointClosed,
} SOPC_EndpointEvent;

/**
 *  \brief Endpoint asynchronous operation result event enumeration type
 */
typedef enum SOPC_EndpointEvent_AsyncOperationResult {
    SOPC_EndpointAsync_OpenResult,
    SOPC_EndpointAsync_SendResponseResult,
    SOPC_EndpointAsync_CloseResult
} SOPC_EndpointEvent_AsyncOperationResult;

/**
 *  \brief Endpoint asynchronous operation result callback type
 */
typedef void (SOPC_Endpoint_AsyncResult_CB) (SOPC_Endpoint                           endpoint,
                                             void*                                   cbData,
                                             SOPC_EndpointEvent_AsyncOperationResult cEvent,
                                             SOPC_StatusCode                         status);

/**
 *  \brief Endpoint event notification function callback type
 */
typedef SOPC_StatusCode (SOPC_EndpointEvent_CB) (SOPC_Endpoint             endpoint,
                                                 void*                     cbData,
                                                 SOPC_EndpointEvent        event,
                                                 SOPC_StatusCode           status,
                                                 uint32_t                  secureChannelId,
                                                 const Certificate*        clientCertificate,
                                                 const SOPC_String*        securityPolicy,
                                                 OpcUa_MessageSecurityMode securityMode);

/**
 *  \brief Endpoint invoke service treatment function type
 */
typedef SOPC_StatusCode (SOPC_InvokeService) (SOPC_Endpoint endpoint, ...);

/**
 *  \brief Endpoint invoke service and send response function type
 */
typedef SOPC_StatusCode (SOPC_BeginInvokeService) (SOPC_Endpoint        endpoint,
                                                   SOPC_RequestContext* requestContext,
                                                   void**               a_ppRequest,
                                                   SOPC_EncodeableType* a_pRequestType);

/**
 *  \brief Service treatment type structure definition. An instance define the way to respond to a service request.
 */
typedef struct SOPC_ServiceType {
    uint32_t                 RequestTypeId;      /**< OPC UA service request type Id */
    SOPC_EncodeableType*     ResponseEncType;    /**< Service response encodeable type */
    SOPC_BeginInvokeService* BeginInvokeService; /**< Service treatment and response sending function */
    SOPC_InvokeService*      InvokeService;      /**< maximum size (allocated bytes) */
} SOPC_ServiceType;

/**
 *  \brief Create a new endpoint by initializing the provided endpoint and setting the given services treatments.
 *
 *  \param endpoint    The endpoint to initialize and to configure with the given services
 *  \param serialType  The channel serialization type to send data
 *                     (only ChannelSerializer_Binary is valid)
 *  \param services    A NULL terminated array of SOPC_ServiceType pointers.
 *                     If NULL is provided the default services treatments are used
 *                     (see opcua_serverapi module: response to any service request is then a Service Fault response)
 *
 *  \return            STATUS_OK if endpoint is correctly initialized, STATUS_NOK otherwise
 *                     (NULL pointer, invalid serialization value)
 */
SOPC_StatusCode SOPC_Endpoint_Create(SOPC_Endpoint*               endpoint,
                                     SOPC_Endpoint_SerializerType serialType,
                                     SOPC_ServiceType**           services); // Null terminated table

/**
 *  \brief Register action to open the endpoint and listen for connections from clients (TCP listener),
 *         then wait for TCP UA connection (TCP UA Hello reception and TCP UA Ack response)
 *         and secure channel establishement
 *         (TCP UA OpenSecureChannel request reception and send TCP UA OpenSecureChannel response)
 *
 *  \param endpoint           The channel to connect
 *  \param endpointURL        Endpoint address of the connection point
 *  \param callback           Endpoint events callback function to be called
 *  \param callbackData       Data to be provided to the endpoint event callback function on call
 *  \param serverCertificate  Server certificate to use for responding to clients connections (or NULL for None security mode)
 *  \param serverKey          Server private key to use for responding to clients connections (or NULL for None security mode)
 *  \param pki                The Public Key Infrastructure to use for validating certificates (or NULL for None security mode)
 *  \param nbSecuConfigs      Number of security policy supported and presents in secuConfigurations
 *  \param secuConfigurations Array of security policies supported with nbSecuConfigs length
 *  \param asyncCb            Asynchronous open result callback function called on endpoint opening failure or success
 *  \param asyncCbData        Asynchronous open result callback data provided to callback function on call
 *
 *  \return                   STATUS_OK if endpoint opening action registration succeeded, STATUS_NOK otherwise
 *
 */
SOPC_StatusCode SOPC_Endpoint_AsyncOpen(SOPC_Endpoint                 endpoint,
                                        char*                         endpointURL,
                                        SOPC_EndpointEvent_CB*        callback,
                                        void*                         callbackData,
                                        Certificate*                  serverCertificate,
                                        AsymmetricKey*                serverKey,
                                        PKIProvider*                  pki,
                                        uint8_t                       nbSecuConfigs,
                                        SOPC_SecurityPolicy*          secuConfigurations,
                                        SOPC_Endpoint_AsyncResult_CB* asyncCb,
                                        void*                         asyncCbData);

/**
 *  \brief Open the endpoint and listen for connections from clients (TCP listener),
 *         then wait for TCP UA connection (TCP UA Hello reception and TCP UA Ack response)
 *         and secure channel establishement
 *         (TCP UA OpenSecureChannel request reception and send TCP UA OpenSecureChannel response)
 *         Note: in single threaded mode, it is necessary to call the socket manager loop to receive messages.
 *
 *  \param endpoint           The channel to connect
 *  \param endpointURL        Endpoint address of the connection point
 *  \param callback           Endpoint events callback function to be called
 *  \param callbackData       Data to be provided to the endpoint event callback function on call
 *  \param serverCertificate  Server certificate to use for responding to clients connections (or NULL for None security mode)
 *  \param serverKey          Server private key to use for responding to clients connections (or NULL for None security mode)
 *  \param pki                The Public Key Infrastructure to use for validating certificates (or NULL for None security mode)
 *  \param nbSecuConfigs      Number of security policy supported and presents in secuConfigurations
 *  \param secuConfigurations Array of security policies supported with nbSecuConfigs length
 *
 *  \return                   STATUS_OK if endpoint opening succeeded, STATUS_NOK otherwise
 *
 */
SOPC_StatusCode SOPC_Endpoint_Open(SOPC_Endpoint          endpoint,
                                   char*                  endpointURL,
                                   SOPC_EndpointEvent_CB* callback,
                                   void*                  callbackData,
                                   Certificate*           serverCertificate,
                                   AsymmetricKey*         serverKey,
                                   PKIProvider*           pki,
                                   uint8_t                nbSecuConfigs,
                                   SOPC_SecurityPolicy*   secuConfigurations);

/**
 *  \brief Instantiate a response and set response type for the given request context
 *
 *  \param endpoint       The endpoint which sends a response
 *  \param context        The request context to use for response elements instantiation
 *  \param responseType   The encodeable type of the response to send
 *  \param response       The instance of response message to send
 *
 *  \return               STATUS_OK if response elements instantiation succeeded, STATUS_NOK otherwise
 */
SOPC_StatusCode SOPC_Endpoint_CreateResponse(SOPC_Endpoint         endpoint,
                                             SOPC_RequestContext*  context,
                                             void**                response,
                                             SOPC_EncodeableType** responseType);

/**
 *  \brief Send a service response message to the client which sent a request
 *
 *  \param endpoint       The endpoint which sends a response
 *  \param responseType   The encodeable type of the response to send
 *  \param response       The instance of response message to send
 *  \param requestContext The request context to use for sending the response
 *                        (provided by the call to SOPC_BeginInvokeService function instance)
 *
 *  \return               STATUS_OK if response sending is in progression, STATUS_NOK otherwise
 */
SOPC_StatusCode SOPC_Endpoint_SendResponse(SOPC_Endpoint         endpoint,
                                           SOPC_EncodeableType*  responseType,
                                           void*                 response,
                                           SOPC_RequestContext** requestContext);

/**
 *  \brief Cancel sending response to client. Must be called before call to SOPC_Endpoint_EndSendResponse.
 *  Note: only used to free the request context correctly.
 *
 *  \param endpoint       The endpoint which aborts a response
 *  \param errorCode      The error that caused the abort response
 *  \param reason         The string reason indicating the abort response cause
 *  \param requestContext The request context of the response to abort
 *                        (provided by the call to SOPC_BeginInvokeService function instance)
 *
 *  \return            STATUS_OK if abort was successful, STATUS_NOK otherwise
 */
SOPC_StatusCode SOPC_Endpoint_CancelSendResponse(SOPC_Endpoint         endpoint,
                                                 SOPC_StatusCode       errorCode,
                                                 SOPC_String*          reason,
                                                 SOPC_RequestContext** requestContext);

/**
 *  \brief Start action to close the endpoint
 *
 *  \param endpoint    The endpoint to close
 *  \param asyncCb     Asynchronous close result callback function called on endpoint closing failure or success
 *  \param asyncCbData Asynchronous close result callback data provided to callback function on call
 *
 *  \return            STATUS_OK if endpoint action to close was recorded correctly, STATUS_NOK otherwise
 */
SOPC_StatusCode SOPC_Endpoint_AsyncClose(SOPC_Endpoint endpoint,
                                         SOPC_Endpoint_AsyncResult_CB* asyncCb,
                                         void*                         asyncCbData);

/**
 *  \brief Close the endpoint. The endpoint closes the active connections and does not listen
 *         for new connections anymore.
 *
 *  \param endpoint    The endpoint to close
 *
 *  \return            STATUS_OK if endpoint was closed correctly, STATUS_NOK otherwise
 */
SOPC_StatusCode SOPC_Endpoint_Close(SOPC_Endpoint endpoint);


/**
 *  \brief Start action to close and deallocate the endpoint
 *
 *  \param endpoint    The endpoint to close and deallocate
 *
 *  \return            STATUS_OK if endpoint action to close and deallocate was recorded correctly, STATUS_NOK otherwise
 */
SOPC_StatusCode SOPC_Endpoint_AsyncDelete(SOPC_Endpoint endpoint);

/**
 *  \brief Close and deallocate the endpoint.
 *
 *  \param endpoint    The endpoint to close and deallocate
 *
 *  \return            STATUS_OK if endpoint was closed and deallocated correctly, STATUS_NOK otherwise
 */
SOPC_StatusCode SOPC_Endpoint_Delete(SOPC_Endpoint* endpoint);

/**
 *  \brief Returns the callback data provided on endpoint open function call
 *
 *  \param endpoint    The endpoint for which callback data is requested
 *
 *  \return            The callback data provided on endpoint open function call, NULL otherwise
 */
void* SOPC_Endpoint_GetCallbackData(SOPC_Endpoint endpoint);

/**
 *  \brief Return the invoke service function for the given endpoint and request context
 *
 *  \param endpoint        The endpoint on which service is called
 *  \param requestContext  The request context to use for returning the invoke service function
 *                         (provided by the call to SOPC_BeginInvokeService function instance)
 *  \param serviceFunction The returned service function in case of success
 *
 *  \return                STATUS_OK if service function was returned correctly, STATUS_NOK otherwise
 */
SOPC_StatusCode SOPC_Endpoint_GetServiceFunction(SOPC_Endpoint        endpoint,
                                                 SOPC_RequestContext* requestContext,
                                                 SOPC_InvokeService** serviceFunction);

/**
 *  \brief Return the secure channel Id given the request context
 *
 *  \param context         The request context to use for returning the secure channel Id
 *                         (provided by the call to SOPC_BeginInvokeService function instance)
 *  \param secureChannelId The returned secure channel id in case of success
 *
 *  \return                STATUS_OK if secure channel id was returned correctly, STATUS_NOK in case of invalid context
 */
SOPC_StatusCode SOPC_Endpoint_GetContextSecureChannelId(SOPC_RequestContext* context,
                                                        uint32_t*            secureChannelId);

/**
 *  \brief Return the security policy and mode given the request context
 *
 *  \param context         The request context to use for returning the security policy and mode
 *                         (provided by the call to SOPC_BeginInvokeService function instance)
 *  \param securityPolicy  The returned security policy in case of success
 *  \param securityMode    The returned security mode in case of success
 *
 *  \return                STATUS_OK if security policy and mode was returned correctly, STATUS_NOK in case of invalid context
 */
SOPC_StatusCode SOPC_Endpoint_GetContextSecureChannelSecurityPolicy(SOPC_RequestContext*       context,
                                                                    SOPC_String*               securityPolicy,
                                                                    OpcUa_MessageSecurityMode* securityMode);

/**
 *  \brief Return the reponse encodeable type given the request context
 *
 *  \param context         The request context to use for returning the response encodeable type
 *                         (provided by the call to SOPC_BeginInvokeService function instance)
 *  \param respType        The returned response encodeable type in case of success
 *
 *  \return                STATUS_OK if response encodeable type was returned correctly, STATUS_NOK in case of invalid context
 */
SOPC_StatusCode SOPC_Endpoint_GetContextResponseType(SOPC_RequestContext*  context,
                                                     SOPC_EncodeableType** respType);

#endif /* SERVER API */

#endif /* SOPC_ENDPOINT_H_ */
