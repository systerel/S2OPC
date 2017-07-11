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

#include "io_dispatch_mgr.h"

#include "sopc_stack_config.h"
#include "sopc_time.h"
#include "sopc_endpoint.h"
#include "opcua_identifiers.h"
#include "opcua_statuscodes.h"
#include "sopc_encodeable.h"

#include "config_toolkit.h"
#include "internal_msg.h"
#include "internal_channel_endpoint.h"
#include "sopc_toolkit_config.h"

#include "wrap_read.h"

#include "sopc_sc_events.h"

void Test_ComEvent_Fct(SOPC_App_Com_Event event,
                         void*              param,
                         SOPC_StatusCode    status){
printf("COM EVENT '%d' received\n", event);
}

int connectionClosed = FALSE;

int main(void)
{
  SOPC_StatusCode status = STATUS_OK;
  SOPC_Endpoint_Config epConfig;
  // Sleep timeout in milliseconds
  const uint32_t sleepTimeout = 500;
  // Loop timeout in milliseconds
  const uint32_t loopTimeout = 20000;
  // Counter to stop waiting on timeout
  uint32_t loopCpt = 0;

  // Secu policy configuration: empty
  SOPC_SecurityPolicy secuConfig[1];
  SOPC_String_Initialize(&secuConfig[0].securityPolicy);

  if(STATUS_OK == status){
    status = SOPC_String_AttachFromCstring(&secuConfig[0].securityPolicy,
                                           "http://opcfoundation.org/UA/SecurityPolicy#None");
    secuConfig[0].securityModes = SECURITY_MODE_NONE_MASK;
  }

  // Init unique endpoint structure
  epConfig.endpointURL = ENDPOINT_URL;
  epConfig.serverCertificate = NULL;
  epConfig.serverKey = NULL;
  epConfig.pki = NULL;
  epConfig.nbSecuConfigs = 1;
  epConfig.secuConfigurations = secuConfig;

  // Init stack configuration
  if(STATUS_OK == status){
    status = SOPC_Toolkit_Initialize(Test_ComEvent_Fct);
    if(STATUS_OK != status){
      printf("<Test_Server_Toolkit: Failed initializing\n");
    }else{
      printf("<Test_Server_Toolkit: initialized\n");
    }
  }


  if(STATUS_OK == status){
    status = SOPC_ToolkitConfig_AddEndpointConfig(1,
                                                  &epConfig);
    if(STATUS_OK == status){
      status = SOPC_Toolkit_Configured();
    }
    if(STATUS_OK != status){
      printf("<Test_Server_Toolkit: Failed to configure the endpoint\n");
    }else{
      printf("<Test_Server_Toolkit: Endpoint configured\n");
    }
  }

  if(STATUS_OK == status){
    status = SOPC_EventDispatcherManager_AddEvent(scEventDispatcherMgr,
                                                  EP_OPEN,
                                                  1,
                                                  &epConfig,
                                                  0,
                                                  "Endpoint opening !");
    if(STATUS_OK == status){
      printf("<Test_Server_Toolkit: Opening endpoint... \n");
    }else{
      printf("<Test_Server_Toolkit: Failed opening endpoint... \n");
    }
  }

  while (STATUS_OK == status && connectionClosed == FALSE && loopCpt * sleepTimeout <= loopTimeout)
    {
        loopCpt++;
    	// Retrieve received messages on socket
        SOPC_Sleep(sleepTimeout);
    }

  if(loopCpt * sleepTimeout > loopTimeout){
    status = OpcUa_BadTimeout;
  }

  address_space_bs__UNINITIALISATION();

  SOPC_EventDispatcherManager_AddEvent(scEventDispatcherMgr,
                                       EP_CLOSE,
                                       1,
                                       &epConfig,
                                       0,
                                       "Endpoint closing !");

  SOPC_Toolkit_Clear();

  if(STATUS_OK == status){
    printf("<Test_Server_Toolkit final result: OK\n");
  }else{
    printf("<Test_Server_Toolkit final result: NOK with status = '%X'\n", status);
  }

  return status;
}
