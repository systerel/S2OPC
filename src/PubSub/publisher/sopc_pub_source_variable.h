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
#include "sopc_mutexes.h"
#include "sopc_pubsub_conf.h"
#include "sopc_types.h"

// Pubscheduler get var request
typedef struct SOPC_PubSheduler_GetVariableRequestContext
{
    SOPC_DataValue* ldv;             // Data values response
    SOPC_EventHandler* eventHandler; // Event handler where to send result
    int32_t NoOfNodesToRead;         // Size of ldv
    uintptr_t msgCtxt;               // context of request
    Condition cond;
    Mutex mut;
} SOPC_PubSheduler_GetVariableRequestContext;

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
typedef SOPC_DataValue* (*SOPC_GetSourceVariables_Func)(OpcUa_ReadValueId* nodesToRead, int32_t nbValues);

SOPC_PubSourceVariableConfig* SOPC_PubSourceVariableConfig_Create(SOPC_GetSourceVariables_Func callback);

void SOPC_PubSourceVariableConfig_Delete(SOPC_PubSourceVariableConfig* sourceConfig);

/**
 *  Function used by publisher scheduler to get source variables
 *
 * \return an array of DataValue of the size of the PublishedDataSet (number of fields) or NULL in case of error
 */

SOPC_DataValue* SOPC_PubSourceVariable_GetVariables(const SOPC_PubSourceVariableConfig* sourceConfig, //
                                                    const SOPC_PublishedDataSet* pubDataset);         //

#endif /* SOPC_PUB_SOURCE_VARIABLE_H_ */
