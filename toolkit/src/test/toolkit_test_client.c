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

#include "b2c.h"

#include "session_header_init.h"
#include "io_dispatch_mgr.h"
#include "constants_bs.h"

#include "address_space_impl.h"
#include "test_results.h"
#include "util_b2c.h"

#include "sopc_toolkit_config.h"
#include "sopc_time.h"
#include "sopc_types.h"
#include "opcua_statuscodes.h"
#include "config_toolkit.h"
#include "crypto_profiles.h"

#include "wrap_read.h"
#include "testlib_write.h"

#include "test_results.h"
#include "testlib_read_response.h"

#include "sopc_sc_events.h"

static uint32_t cptReadResps = 0;

void Test_ComEvent_Fct(SOPC_App_Com_Event event,
                         void*              param,
                         SOPC_StatusCode    status){
 if(event == SE_RCV_SESSION_RESPONSE){
   SOPC_Toolkit_Msg* msg = (SOPC_Toolkit_Msg*) param;
   if(msg->encType == &OpcUa_ReadResponse_EncodeableType){
       printf(">>Test_Client_Toolkit: received ReadResponse \n");
       OpcUa_ReadResponse* readResp = (OpcUa_ReadResponse*) msg->msg;
       cptReadResps++;
       if(cptReadResps <= 1){
           test_results_set_service_result(test_read_request_response(readResp,
                                                                      status,
                                                                      0)
                                           ? (!FALSE):FALSE);
       }else{
           // Second read response is to test write effect (through read result)
           test_results_set_service_result(
               tlibw_verify_response_remote(test_results_get_WriteRequest(), readResp));
       }
   }else if(msg->encType == &OpcUa_WriteResponse_EncodeableType){
       printf(">>Test_Client_Toolkit: received WriteResponse \n");
       OpcUa_WriteResponse* writeResp = (OpcUa_WriteResponse*) msg->msg;
       test_results_set_service_result(
           tlibw_verify_response(test_results_get_WriteRequest(), writeResp));
   }
 }
}

/* Function to build the read service request message */
SOPC_Toolkit_Msg* getReadRequest_message(){
  SOPC_Toolkit_Msg *pMsg = (SOPC_Toolkit_Msg *)malloc(sizeof(SOPC_Toolkit_Msg));
  if(NULL == pMsg)
    return NULL;
  pMsg->msg = read_new_read_request();
  if(NULL == pMsg->msg)
    return NULL;
  pMsg->encType = &OpcUa_ReadRequest_EncodeableType;
  pMsg->respEncType = &OpcUa_ReadResponse_EncodeableType;
  pMsg->isRequest = (!FALSE);
  return pMsg;
}

/* Function to build the verification read request */
SOPC_Toolkit_Msg* getReadRequest_verif_message() {
  SOPC_Toolkit_Msg *pMsg = (SOPC_Toolkit_Msg *)malloc(sizeof(SOPC_Toolkit_Msg));
  if(NULL == pMsg)
    return NULL;
  pMsg->msg = tlibw_new_ReadRequest_check();
  if(NULL == pMsg->msg)
    return NULL;
  pMsg->encType = &OpcUa_ReadRequest_EncodeableType;
  pMsg->respEncType = &OpcUa_ReadResponse_EncodeableType;
  pMsg->isRequest = (!FALSE);
  return pMsg;
}

SOPC_SecureChannel_Config scConfig = {
    .isClientSc = !FALSE,
    .url= ENDPOINT_URL,
    .crt_cli = NULL,
    .key_priv_cli = NULL,
    .crt_srv = NULL,
    .pki = NULL,
    .reqSecuPolicyUri = SecurityPolicy_None_URI,
    .requestedLifetime = 5,
    .msgSecurityMode = OpcUa_MessageSecurityMode_None
};

int main(void){

  // Sleep timeout in milliseconds
  const uint32_t sleepTimeout = 500;
  // Loop timeout in milliseconds
  const uint32_t loopTimeout = 20000;
  // Counter to stop waiting on timeout
  uint32_t loopCpt = 0;

  constants__t_channel_config_idx_i channel_config_idx = constants__c_channel_config_idx_indet;
  constants__t_session_i session = constants__c_session_indet;
  /* Note: in current version of toolkit user == 1 is considered as anonymous user */
  constants__t_user_i user = 1;
  constants__t_sessionState session_state = constants__e_session_closed;

  SOPC_StatusCode status = STATUS_OK;
  constants__t_StatusCode_i sCode = constants__e_sc_ok;

  OpcUa_WriteRequest *pWriteReq = NULL;

  /* Init stack configuration */
  if(STATUS_OK == status){
    status = SOPC_Toolkit_Initialize(Test_ComEvent_Fct);
    if(STATUS_OK != status){
      printf(">>Test_Client_Toolkit: Failed initializing\n");
    }else{
      printf(">>Test_Client_Toolkit: Stack initialized\n");
    }
  }

  if(STATUS_OK == status){
    channel_config_idx = 1;
    status = SOPC_ToolkitConfig_AddSecureChannelConfig(channel_config_idx,
                                                       &scConfig);
    if(STATUS_OK == status){
      status = SOPC_Toolkit_Configured();
    }
    if(STATUS_OK != status){
      printf(">>Test_Client_Toolkit: Failed to configure the secure channel\n");
    }else{
      printf(">>Test_Client_Toolkit: Client configured\n");
    }
  }

  /* Create a session (and underlying secure channel if necessary).

     Note: in current version endpoint is managed through ENDPOINT_URL
     variable and parameter has no effect.
  */
  if(STATUS_OK == status){
    io_dispatch_mgr__activate_new_session(channel_config_idx,
                                          user,
                                          &session);
    if(session != constants__c_session_indet){
        printf(">>Test_Client_Toolkit: Creating/Activating session: OK\n");
    }else{
        status = STATUS_NOK;
        printf(">>Test_Client_Toolkit: Creating/Activating session: statusCode = '%X' | NOK\n", status);
    }
  }

  /* Wait until session is activated or timeout */
  loopCpt = 0;
  while(STATUS_OK == status &&
        session_state != constants__e_session_userActivated &&
        loopCpt * sleepTimeout <= loopTimeout){
    loopCpt++;
    // Retrieve received messages on socket
    SOPC_Sleep(sleepTimeout);
    io_dispatch_mgr__get_session_state_or_closed(session,
                                                 &session_state);
  }

  if(loopCpt * sleepTimeout > loopTimeout){
    status = OpcUa_BadTimeout;
  }

  if(STATUS_OK == status){
    if(constants__e_session_userActivated == session_state){
      printf(">>Test_Client_Toolkit: Session activated: OK'\n");
    }else{
      printf(">>Test_Client_Toolkit: Session activated: NOK'\n");
    }
  }

  if(STATUS_OK == status){
    /* Create a service request message and send it through session (read service)*/
    SOPC_Toolkit_Msg *pMsgRead = getReadRequest_message();
    // msg freed when sent
    io_dispatch_mgr__send_service_request_msg(session,
                                              pMsgRead,
                                              &sCode);

    if(constants__e_sc_ok == sCode){
      printf(">>Test_Client_Toolkit: read request sending: OK'\n");
    }else{
      util_status_code__B_to_C(sCode,
                               &status);
      printf(">>Test_Client_Toolkit: read request sending: statusCode = '%X' | NOK\n", status);
    }
  }

  /* Wait until service response is received */
  loopCpt = 0;
  while(STATUS_OK == status &&
        test_results_get_service_result() == FALSE &&
        loopCpt * sleepTimeout <= loopTimeout){
    loopCpt++;
    SOPC_Sleep(100);
  }

  if(loopCpt * sleepTimeout > loopTimeout){
    status = OpcUa_BadTimeout;
  }

  if(STATUS_OK == status)
  {
    // Reset expected result
    test_results_set_service_result(FALSE);
    /* Sends a WriteRequest */
    pWriteReq = tlibw_new_WriteRequest();
    SOPC_Toolkit_Msg *pMsgWrite = tlibw_new_message_WriteRequest(pWriteReq);
    pWriteReq = (OpcUa_WriteRequest *) pMsgWrite->msg;
    test_results_set_WriteRequest(pWriteReq);
    // msg freed when sent
    io_dispatch_mgr__send_service_request_msg(session, pMsgWrite, &sCode);

    /* Same data must be provided to verify result, since request will be freed on sending allocate a new (same content) */
    pWriteReq = tlibw_new_WriteRequest();
    test_results_set_WriteRequest(pWriteReq);


    if(constants__e_sc_ok == sCode){
      printf(">>Test_Client_Toolkit: write request sending: OK\n");
    }else{
      util_status_code__B_to_C(sCode,
                               &status);
      printf(">>Test_Client_Toolkit: write request sending: statusCode = '%d' | NOK\n", status);
    }
  }

  /* Wait until service response is received */
  loopCpt = 0;
  while(STATUS_OK == status &&
        test_results_get_service_result() == FALSE &&
        loopCpt * sleepTimeout <= loopTimeout){
    loopCpt++;
    SOPC_Sleep(100);
  }

  if(loopCpt * sleepTimeout > loopTimeout){
    status = OpcUa_BadTimeout;
  }

  if(STATUS_OK == status)
  {
    // Reset expected result
    test_results_set_service_result(FALSE);
    /* Sends another ReadRequest, to verify that the AddS has changed */
    /* The callback will call the verification */
    SOPC_Toolkit_Msg *pMsgRead = getReadRequest_verif_message();
    // msg freed when sent
    io_dispatch_mgr__send_service_request_msg(session,
                                              pMsgRead,
                                              &sCode);

    if(constants__e_sc_ok == sCode){
      printf(">>Test_Client_Toolkit: read request sending: OK'\n");
    }else{
      util_status_code__B_to_C(sCode,
                               &status);
      printf(">>Test_Client_Toolkit: read request sending: statusCode = '%d' | NOK\n", status);
    }
  }

  /* Wait until service response is received */
  loopCpt = 0;
  while(STATUS_OK == status &&
        test_results_get_service_result() == FALSE &&
        loopCpt * sleepTimeout <= loopTimeout){
    loopCpt++;
    SOPC_Sleep(100);
  }

  if(loopCpt * sleepTimeout > loopTimeout){
    status = OpcUa_BadTimeout;
  }

  /* Now the request can be freed */
  test_results_set_WriteRequest(NULL);
  tlibw_free_WriteRequest((OpcUa_WriteRequest **) &pWriteReq);

  /* Close the session */
  if(constants__c_session_indet != session){
    io_dispatch_mgr__close_session(session,
                                   &sCode);
    if(STATUS_OK == status){
      util_status_code__B_to_C(sCode,
                               &status);
    }
  }

  /* Wait until session is closed or timeout */
  loopCpt = 0;
  do{
    loopCpt++;
    SOPC_Sleep(100);
    io_dispatch_mgr__get_session_state_or_closed(session,
                                                 &session_state);
  }while(STATUS_OK == status &&
         constants__e_session_closed != session_state &&
         loopCpt * sleepTimeout <= loopTimeout);

  address_space_bs__UNINITIALISATION();

  SOPC_Toolkit_Clear();

  if(STATUS_OK == status && test_results_get_service_result() == (!FALSE)){
    printf(">>Test_Client_Toolkit: read request received ! \n");
    printf(">>Test_Client_Toolkit final result: OK\n");
    return 0;
  }else{
    printf(">>Test_Client_Toolkit: read request not received or BAD status (%X) ! \n", status);
    printf(">>Test_Client_Toolkit final result: NOK\n");
    return status;
  }

}

