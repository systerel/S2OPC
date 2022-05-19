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

#ifndef SOPC_SUB_TARGET_VARIABLE_H_
#define SOPC_SUB_TARGET_VARIABLE_H_

#include "sopc_dataset_ll_layer.h"
#include "sopc_pubsub_conf.h"
#include "sopc_sub_target_variable.h"
#include "sopc_types.h"

/* Configuration to provide as target variable config when starting subscriber */
typedef struct _SOPC_SubTargetVariableConfig SOPC_SubTargetVariableConfig;

/**
 * \brief The subscriber scheduler calls this callback cyclically to pass the values received by the subscriber.
 *
 * \note Ownership of the WriteValue array and its elements is transferred to the callback code which must free them.
 *
 * \note This function is called once per configured DataSet, with all the values of the DataSet in a single call.
 *
 * \return   true if processed, false otherwise (array must still be freed) */
typedef bool SOPC_SetTargetVariables_Func(OpcUa_WriteValue* nodesToWrite, int32_t nbValues);

/* If callback NULL, creation succeeds and SetVariables will only check input parameters on call */
SOPC_SubTargetVariableConfig* SOPC_SubTargetVariableConfig_Create(SOPC_SetTargetVariables_Func* callback);

void SOPC_SubTargetVariableConfig_Delete(SOPC_SubTargetVariableConfig* targetConfig);

/* Function used by subscriber scheduler to set target variables */
bool SOPC_SubTargetVariable_SetVariables(SOPC_SubTargetVariableConfig* targetConfig,
                                         const SOPC_DataSetReader* reader,
                                         const SOPC_Dataset_LL_DataSetMessage* dsm);

#endif /* SOPC_SUB_TARGET_VARIABLE_H_ */
