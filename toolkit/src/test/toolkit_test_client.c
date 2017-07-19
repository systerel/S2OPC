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
#include "internal_msg.h"
#include "test_results.h"
#include "util_b2c.h"

#include "sopc_stack_config.h"
#include "sopc_base_types.h"
#include "sopc_types.h"
#include "opcua_statuscodes.h"
#include "sopc_run.h"

#include "wrap_read.h"
#include "testlib_write.h"


/* Function to build the read service request message */
message__message* getReadRequest_message(){
  message__message *pMsg = (message__message *)malloc(sizeof(message__message));
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

/* Functions to build a write service request message, and its verification read request */
message__message *getWriteRequest_message() {
  message__message *pMsg = (message__message *)malloc(sizeof(message__message));
  if(NULL == pMsg)
    return NULL;
  pMsg->msg = tlibw_new_WriteRequest();
  if(NULL == pMsg->msg)
    return NULL;
  pMsg->encType = &OpcUa_WriteRequest_EncodeableType;
  pMsg->respEncType = &OpcUa_WriteResponse_EncodeableType;
  pMsg->isRequest = (!FALSE);
  return pMsg;
}
message__message* getReadRequest_verif_message() {
  message__message *pMsg = (message__message *)malloc(sizeof(message__message));
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

int main(void){

  // Sleep timeout in milliseconds
  const uint32_t maxSleepTimeout = 500;
  // Loop timeout in milliseconds
  const uint32_t maxLoopTimeout = 20000;
  // Counter to stop waiting on timeout
  uint32_t loopCpt = 0;

  const constants__t_endpoint_i endpoint = constants__c_endpoint_indet;
  constants__t_session_i session = constants__c_session_indet;
  /* Note: in current version of toolkit user == 1 is considered as anonymous user */
  constants__t_user_i user = 1;
  constants__t_sessionState session_state = constants__e_session_closed;

  SOPC_StatusCode status = STATUS_OK;
  constants__t_StatusCode_i sCode = constants__e_sc_ok;

  OpcUa_WriteRequest *pWriteReq = NULL;

  /* Init B model */
  INITIALISATION();

  /* Init stack configuration */
  if(STATUS_OK == status){
    status = StackConfiguration_Initialize();
    if(STATUS_OK != status){
      printf(">>Test_Client_Toolkit: Failed initializing stack\n");
    }else{
      printf(">>Test_Client_Toolkit: Stack initialized\n");
    }
  }

  /* Create a session (and underlying secure channel if necessary).

     Note: in current version endpoint is managed through ENDPOINT_URL
     variable and parameter has no effect.
  */
  if(STATUS_OK == status){
    io_dispatch_mgr__create_session(endpoint,
                                    &session);

    /* Check a new session was created */
    if (session == constants__c_session_indet){
      status = STATUS_NOK;
    }
  }

  /* Wait until session is in created state or timeout */
  loopCpt = 0;
  while(STATUS_OK == status &&
        session_state != constants__e_session_created &&
        loopCpt * maxSleepTimeout <= maxLoopTimeout){
    loopCpt++;
    // Retrieve received messages on socket
    status = SOPC_TreatReceivedMessages(maxSleepTimeout);
    io_dispatch_mgr__get_session_state_or_closed(session,
                                                 &session_state);
  };

  if(loopCpt * maxSleepTimeout > maxLoopTimeout){
    status = OpcUa_BadTimeout;
  }

  if(STATUS_OK == status &&
     constants__e_session_created == session_state){
    /* Activate session with anonymous user */
    io_dispatch_mgr__activate_session(session,
                                      user,
                                      &sCode);
    if(constants__e_sc_ok == sCode){
        printf(">>Test_Client_Toolkit: Activating session: OK\n");
    }else{
      util_status_code__B_to_C(sCode,
                               &status);
      printf(">>Test_Client_Toolkit: Activating session: statusCode = '%d' | NOK\n", status);
    }
  }

  /* Wait until session is activated or timeout */
  loopCpt = 0;
  while(STATUS_OK == status &&
        session_state != constants__e_session_userActivated &&
        loopCpt * maxSleepTimeout <= maxLoopTimeout){
    loopCpt++;
    // Retrieve received messages on socket
    status = SOPC_TreatReceivedMessages(maxSleepTimeout);
    io_dispatch_mgr__get_session_state_or_closed(session,
                                                 &session_state);
  }

  if(loopCpt * maxSleepTimeout > maxLoopTimeout){
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
    /* Create a service request message and send it through session (read service) */
    message__message *pMsgRead = getReadRequest_message();
    io_dispatch_mgr__send_service_request_msg(session,
                                              pMsgRead,
                                              &sCode);

    free(((OpcUa_ReadRequest *)pMsgRead->msg)->NodesToRead);
    free(pMsgRead->msg);
    free(pMsgRead);

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
        loopCpt * maxSleepTimeout <= maxLoopTimeout){
    loopCpt++;
    status = SOPC_TreatReceivedMessages(100);
  }

  if(loopCpt * maxSleepTimeout > maxLoopTimeout){
    status = OpcUa_BadTimeout;
  }

  if(STATUS_OK == status)
  {
    // Reset expected result
    test_results_set_service_result(FALSE);
    /* Sends a WriteRequest */
    message__message *pMsgWrite = getWriteRequest_message();
    pWriteReq = (OpcUa_WriteRequest *) pMsgWrite->msg;
    test_results_set_WriteRequest(pWriteReq);
    io_dispatch_mgr__send_service_request_msg(session, pMsgWrite, &sCode);

    /* Do not free the WriteRequest now, it is used by test_results_* */
    free(pMsgWrite);

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
        loopCpt * maxSleepTimeout <= maxLoopTimeout){
    loopCpt++;
    status = SOPC_TreatReceivedMessages(100);
  }

  if(loopCpt * maxSleepTimeout > maxLoopTimeout){
    status = OpcUa_BadTimeout;
  }

  if(STATUS_OK == status)
  {
    // Reset expected result
    test_results_set_service_result(FALSE);
    /* Sends another ReadRequest, to verify that the AddS has changed */
    /* The callback will call the verification */
    message__message *pMsgRead = getReadRequest_verif_message();
    io_dispatch_mgr__send_service_request_msg(session,
                                              pMsgRead,
                                              &sCode);

    free(((OpcUa_ReadRequest *)pMsgRead->msg)->NodesToRead);
    free(pMsgRead->msg);
    free(pMsgRead);

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
        loopCpt * maxSleepTimeout <= maxLoopTimeout){
    loopCpt++;
    status = SOPC_TreatReceivedMessages(100);
  }

  if(loopCpt * maxSleepTimeout > maxLoopTimeout){
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
    status = SOPC_TreatReceivedMessages(100);
    io_dispatch_mgr__get_session_state_or_closed(session,
                                                 &session_state);
  }while(STATUS_OK == status &&
         constants__e_session_closed != session_state &&
         loopCpt * maxSleepTimeout <= maxLoopTimeout);

  io_dispatch_mgr__close_all_active_connections();

  address_space_bs__UNINITIALISATION();

  StackConfiguration_Clear();

  if(STATUS_OK == status && test_results_get_service_result() == (!FALSE)){
    printf(">>Test_Client_Toolkit: read request received ! \n");
    printf(">>Test_Client_Toolkit final result: OK\n");
    return 0;
  }else{
    printf(">>Test_Client_Toolkit: read request not received or BAD status (%d) ! \n", status);
    printf(">>Test_Client_Toolkit final result: NOK\n");
    return status;
  }

}

