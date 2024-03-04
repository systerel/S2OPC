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

// TODO: this structure is taken from the sample/pubsub_server,
//  where it is used to handle asynchronous OPC UA local read request.
//  Either put it back there or document it properly.
// Pubscheduler get var request
typedef struct SOPC_PubSheduler_GetVariableRequestContext
{
    SOPC_DataValue* ldv;             // Data values response
    SOPC_EventHandler* eventHandler; // Event handler where to send result
    int32_t NoOfNodesToRead;         // Size of ldv
    uintptr_t msgCtxt;               // context of request
    SOPC_Condition cond;
    SOPC_Mutex mut;
} SOPC_PubSheduler_GetVariableRequestContext;

/**
 * Configuration to provide as source variable config when starting publisher
 */
typedef struct SOPC_PubSourceVariableConfig SOPC_PubSourceVariableConfig;

/**
 * Data transfered by scheduler to user callback
 */
typedef struct SOPC_SourceVariableCtx SOPC_SourceVariableCtx;

/**
 * \brief The publisher calls this callback cyclically to get the values to publish.
 *
 * Given the \p nodesToRead, it should produce an array of DataValues of length \p nbValues.
 *
 * \note Ownership of the returned DataValue array and its elements is transferred to the publisher library.
 *
 * \return  An array of DataValue of length \p nbValues or NULL in case of error
 */
typedef SOPC_DataValue* SOPC_GetSourceVariables_Func(const OpcUa_ReadValueId* nodesToRead, const int32_t nbValues);

SOPC_PubSourceVariableConfig* SOPC_PubSourceVariableConfig_Create(SOPC_GetSourceVariables_Func* callback);

void SOPC_PubSourceVariableConfig_Delete(SOPC_PubSourceVariableConfig* sourceConfig);

/**
 *  Function used by publisher scheduler to get source variables
 *
 * \return an array of DataValue of the size of the PublishedDataSet (number of fields) or NULL in case of error
 */

SOPC_DataValue* SOPC_PubSourceVariable_GetVariables(const SOPC_PubSourceVariableConfig* sourceConfig,
                                                    const SOPC_SourceVariableCtx* sourceVariable);

/**
 * @brief Create and Initialize Source Variable context for the user to get source variables
 *
 * @param pubDataset published dataset of the dataSetMessage which field we update
 *
 * @return An initialize source variable context and NULL in case of error
 */
SOPC_SourceVariableCtx* SOPC_PubSourceVariable_SourceVariablesCtx_Create(const SOPC_PublishedDataSet* pubDataset);

/* Delete SourceVariableCtx */
void SOPC_PubSourceVariable_SourceVariableCtx_Delete(SOPC_SourceVariableCtx** pubSourceVariable);

#endif /* SOPC_PUB_SOURCE_VARIABLE_H_ */
