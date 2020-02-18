/*
 * Licensed to Systerel under one or more contributor license
 * agreements. See the NOTICE file distributed with this work
 * for additional information regarding copyright ownership.
 * Systerel licenses this file to you under the Apache
 * License, Version 2.0 (the "License"); you may not use this
 * file except in compliance with the License. You may obtain
 * a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#ifndef SOPC_PUB_SOURCE_VARIABLE_H_
#define SOPC_PUB_SOURCE_VARIABLE_H_

#include "sopc_event_handler.h"
#include "sopc_pubsub_conf.h"
#include "sopc_types.h"

// Pubscheduler get var request
typedef struct TSOPC_PubSheduler_GetVariableRequestContext
{
    SOPC_DataValue* ldv;             // Data values response
    SOPC_EventHandler* eventHandler; // Event handler where to send result
    int32_t NoOfNodesToRead;         // Size of ldv
    uintptr_t msgCtxt;               // context of request
} SOPC_PubSheduler_GetVariableRequestContext;

// Status of a request for a message context
typedef enum ESOPC_PubSheduler_GetVariableRequestStatus
{
    SOPC_PUBSCHEDULER_STATUS_REQUEST_READY = 0,
    SOPC_PUBSCHEDULER_STATUS_REQUEST_BUSY = 1
} SOPC_PubSheduler_GetVariableRequestStatus;

// Pubscheduler events
typedef enum ESOPC_PubScheduler_Event
{
    SOPC_PUBSCHEDULER_EVENT_PUBLISH_REQUEST = 0,
    SOPC_PUBSCHEDULER_EVENT_PUBLISH_RESPONSE = 1
} SOPC_PubScheduler_Event;

/**
 * Configuration to provide as source variable config when starting publisher
 * */
typedef struct _SOPC_PubSourceVariableConfig SOPC_PubSourceVariableConfig;

/**
 *  Callback function called by publisher on publishingInterval to send a request on date variable values.
 *
 * Note: ownership of the ReadValue array is transfered to the callback code
 *
 * \return  status code indicating the success or failure of request sending
 */
typedef SOPC_ReturnStatus (*SOPC_GetSourceVariables_Func)(OpcUa_ReadValueId* nodesToRead,
                                                                 int32_t nbValues);

/**
 *  Callback function called by publisher on publishingInterval to send a request on date variable values.
 *
 * Note: ownership of the ReadValue array is transfered to the callback code
 *
 * \return  status code indicating the success or failure of request sending
 */
typedef SOPC_ReturnStatus (*SOPC_GetSourceVariablesRequest_Func)(SOPC_EventHandler* eventHandler,
                                                                 uintptr_t msgCtxt,
                                                                 OpcUa_ReadValueId* nodesToRead,
                                                                 int32_t nbValues);
/**
 *  Callback function called by publisher on publishingInterval to retrieve up to date variable values.
 *
 * Note: ownership of the return Data Values is transfered to the caller.
 *
 * \return  the array of \p nbValues read values or NULL otherwise
 */
typedef SOPC_DataValue* (*SOPC_GetSourceVariablesResponse_Func)(
    SOPC_PubSheduler_GetVariableRequestContext* requestContext);

SOPC_PubSourceVariableConfig* SOPC_PubSourceVariableConfig_Create(
    SOPC_GetSourceVariablesRequest_Func callbackRequest,    //
    SOPC_GetSourceVariablesResponse_Func callbackResponse); //

void SOPC_PubSourceVariableConfig_Delete(SOPC_PubSourceVariableConfig* sourceConfig);

/**
 *  Function used by publisher scheduler to request source variables:
 *
 * \return  status code indicating the success or failure of request sending
 */

SOPC_ReturnStatus SOPC_PubSourceVariable_GetVariables(SOPC_EventHandler* eventHandler,
                                                      uintptr_t msgCtxt,
                                                      const SOPC_PubSourceVariableConfig* sourceConfig,
                                                      const SOPC_PublishedDataSet* pubDataset);

/**
 *  Function used by publisher scheduler to get source variables from response:
 *
 * Note: ownership of the Data Values is transfered to the caller
 *
 *   \return  an array of DataValue of the size of the PublishedDataSet (number of fields) or NULL in case of error
 */

SOPC_DataValue* SOPC_PubSourceVariable_GetVariablesResponse(SOPC_PubSheduler_GetVariableRequestContext* requestContext,
                                                            const SOPC_PubSourceVariableConfig* sourceConfig);

#endif /* SOPC_PUB_SOURCE_VARIABLE_H_ */
