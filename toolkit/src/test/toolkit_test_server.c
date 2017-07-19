/*
 *  Copyright (C) 2017 Systerel and others.
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

#include <stdio.h>
#include <stdlib.h>

#include "session_header_init.h"
#include "io_dispatch_mgr.h"

#include "sopc_stack_config.h"
#include "sopc_run.h"
#include "sopc_endpoint.h"
#include "opcua_identifiers.h"
#include "opcua_statuscodes.h"
#include "sopc_encodeable.h"

#include "config_toolkit.h"
#include "internal_msg.h"
#include "internal_channel_endpoint.h"

#include "wrap_read.h"


static Internal_Channel_Or_Endpoint endpoint_and_context;

// indicates if a connection was closed (after being opened)
static int connectionClosed = 0;

/* Function provided to the communication Stack to redirect a received service message to the Toolkit */
SOPC_StatusCode Toolkit_BeginService(SOPC_Endpoint               a_hEndpoint,
                                     struct SOPC_RequestContext* a_hContext,
                                     void**                      a_ppRequest,
                                     SOPC_EncodeableType*        a_pRequestType){
  if(a_hEndpoint == endpoint_and_context.endpoint){
    // context to keep for the response
    // TODO: to be kept associated with the request handle for async services
    endpoint_and_context.context = a_hContext;
    message__message* msg = malloc(sizeof(message__message));
    if(NULL != msg){
      msg->msg = *a_ppRequest;
      msg->encType = a_pRequestType;
      msg->respEncType = NULL; // used only on client side for invoking service
      msg->isRequest = (!FALSE);

      io_dispatch_mgr__receive_msg((constants__t_channel_i) &endpoint_and_context,
                                   (constants__t_msg_i) msg);
      free(msg);
    }else{
      printf("Toolkit_BeginService: out of memory \n");
      exit(1);
    }
  }else{
    printf("Toolkit_BeginService: invalid endpoint \n");
    exit(1);
  }
  SOPC_Encodeable_Delete(a_pRequestType, a_ppRequest);

  return STATUS_OK;
}

/* Create session service provided to the Stack */
struct SOPC_ServiceType Toolkit_CreateSession_ServiceType =
{
    OpcUaId_CreateSessionRequest,
    &OpcUa_CreateSessionResponse_EncodeableType,
    Toolkit_BeginService,
    NULL
};

/* Activate session service provided to the Stack */
struct SOPC_ServiceType Toolkit_ActivateSession_ServiceType =
{
    OpcUaId_ActivateSessionRequest,
    &OpcUa_ActivateSessionResponse_EncodeableType,
   Toolkit_BeginService,
    NULL
};

/* Close session service provided to the Stack */
struct SOPC_ServiceType Toolkit_CloseSession_ServiceType =
{
    OpcUaId_CloseSessionRequest,
    &OpcUa_CloseSessionResponse_EncodeableType,
   Toolkit_BeginService,
    NULL
};

/* Read service provided to the Stack */
struct SOPC_ServiceType Toolkit_Read_ServiceType =
{
    OpcUaId_ReadRequest,
    &OpcUa_ReadResponse_EncodeableType,
    Toolkit_BeginService,
    NULL
};

/* Write service provided to the Stack */
struct SOPC_ServiceType Toolkit_Write_ServiceType =
{
    OpcUaId_WriteRequest,
    &OpcUa_WriteResponse_EncodeableType,
    Toolkit_BeginService,
    NULL
};

/* List of services provided to the Stack */
SOPC_ServiceType* Toolkit_SupportedServiceTypes[] =
{
    &Toolkit_CreateSession_ServiceType,
    &Toolkit_ActivateSession_ServiceType,
    &Toolkit_CloseSession_ServiceType,
    &Toolkit_Read_ServiceType,
    &Toolkit_Write_ServiceType,
    NULL
};

SOPC_StatusCode TestServer_EndpointEvent_Callback(SOPC_Endpoint             endpoint,
                                                  void*                     cbData,
                                                  SOPC_EndpointEvent        event,
                                                  SOPC_StatusCode           status,
                                                  uint32_t                  secureChannelId,
                                                  const Certificate*        clientCertificate,
                                                  const SOPC_String*        securityPolicy,
                                                  OpcUa_MessageSecurityMode securityMode){
    (void) endpoint;
    (void) cbData;
    (void) secureChannelId;
    (void) clientCertificate;
    (void) securityPolicy;
    (void) securityMode;
    char* cevent = NULL;
    switch(event){
        case SOPC_EndpointEvent_Invalid:
            cevent = "SOPC_EndpointEvent_Invalid";
            break;
        case SOPC_EndpointEvent_SecureChannelOpened:
            cevent = "SOPC_EndpointEvent_SecureChannelOpened";
            break;
        case SOPC_EndpointEvent_SecureChannelClosed:
            cevent = "SOPC_EndpointEvent_SecureChannelClosed";
            break;
        case SOPC_EndpointEvent_Renewed:
            cevent = "SOPC_EndpointEvent_Renewed";
            break;
        case SOPC_EndpointEvent_UnsupportedServiceRequested:
            cevent = "SOPC_EndpointEvent_UnsupportedServiceRequested";
            break;
        case SOPC_EndpointEvent_DecoderError:
            cevent = "SOPC_EndpointEvent_Renewed";
            break;
        case SOPC_EndpointEvent_EndpointClosed:
            cevent = "SOPC_EndpointEvent_UnsupportedServiceRequested";
            break;
    }
    printf("<Test_Server_Toolkit: Endpoint CALLBACK called with event '%s' and status '%x' !\n", cevent, status);
    if (event == SOPC_EndpointEvent_SecureChannelClosed){
      connectionClosed = 1;
    }
    else if (event == SOPC_EndpointEvent_SecureChannelOpened){
      printf("<Test_Server_Toolkit: connection established on endpoint \n");
    }
    return STATUS_OK;
}


int main(void)
{
  SOPC_StatusCode status = STATUS_OK;

  // Sleep timeout in milliseconds
  const uint32_t maxSleepTimeout = 500;
  // Loop timeout in milliseconds
  const uint32_t maxLoopTimeout = 20000;
  // Counter to stop waiting on timeout
  uint32_t loopCpt = 0;

  /* Init B model */
  INITIALISATION();

  // Init unique endpoint structure
  endpoint_and_context.isChannel = FALSE;
  endpoint_and_context.channel = NULL;
  endpoint_and_context.endpoint = NULL;
  endpoint_and_context.context = NULL;

  // Secu policy configuration: empty
  SOPC_SecurityPolicy secuConfig[1];
  SOPC_String_Initialize(&secuConfig[0].securityPolicy);

  if(STATUS_OK == status){
    status = SOPC_String_AttachFromCstring(&secuConfig[0].securityPolicy,
                                           "http://opcfoundation.org/UA/SecurityPolicy#None");
    secuConfig[0].securityModes = SECURITY_MODE_NONE_MASK;
  }

  // Init stack configuration
  if(STATUS_OK == status){
    status = StackConfiguration_Initialize();
    if(STATUS_OK != status){
      printf("<Test_Server_Toolkit: Failed initializing stack\n");
    }else{
      printf("<Test_Server_Toolkit: Stack initialized\n");
    }
  }

  if(STATUS_OK == status){
    status = SOPC_Endpoint_Create (&endpoint_and_context.endpoint,
                                   SOPC_EndpointSerializer_Binary,
                                   Toolkit_SupportedServiceTypes); //Services
    if(STATUS_OK != status){
      printf("<Test_Server_Toolkit: Failed to create the endpoint\n");
    }else{
      printf("<Test_Server_Toolkit: Creating endpoint with success\n");
    }
  }

  if(STATUS_OK == status){
        status = SOPC_Endpoint_Open(endpoint_and_context.endpoint,        // Endpoint
                                    ENDPOINT_URL,                       // URL
                                    TestServer_EndpointEvent_Callback, // Endpoint Callback
                                    NULL,                    // Endpoint Callback Data
                                    NULL,                           // Server Certificate
                                    NULL,                          // Private Key
                                    NULL,                               // PKI Config
                                    1,  // NoOf SecurityPolicies
                                    secuConfig);                       // SecurityPolicies
        if(STATUS_OK != status){
            printf("<Test_Server_Toolkit: Failed to open the endpoint\n");
        }else{
            printf("<Test_Server_Toolkit: Opening endpoint with success\n");
        }
    }

  while (STATUS_OK == status && connectionClosed == FALSE && loopCpt * maxSleepTimeout <= maxLoopTimeout)
    {
        loopCpt++;
    	// Retrieve received messages on socket
        status = SOPC_TreatReceivedMessages(maxSleepTimeout);
    }

  if(loopCpt * maxSleepTimeout > maxLoopTimeout){
    status = OpcUa_BadTimeout;
  }

  address_space_bs__UNINITIALISATION();

  SOPC_Endpoint_Delete(&endpoint_and_context.endpoint);
  StackConfiguration_Clear();

  if(STATUS_OK == status){
    printf("<Test_Server_Toolkit final result: OK\n");
  }else{
    printf("<Test_Server_Toolkit final result: NOK with status = '%d'\n", status);
  }

  return status;
}
