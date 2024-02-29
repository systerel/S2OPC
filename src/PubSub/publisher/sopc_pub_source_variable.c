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

#include <stdbool.h>

#include "sopc_assert.h"
#include "sopc_mem_alloc.h"
#include "sopc_pub_source_variable.h"
#include "sopc_pubsub_helpers.h"

struct SOPC_PubSourceVariableConfig
{
    SOPC_GetSourceVariables_Func* callback;
};

SOPC_PubSourceVariableConfig* SOPC_PubSourceVariableConfig_Create(SOPC_GetSourceVariables_Func* callback)
{
    SOPC_PubSourceVariableConfig* sourceConfig = NULL;
    if (NULL != callback)
    {
        sourceConfig = SOPC_Calloc(1, sizeof(*sourceConfig));
    }
    if (NULL != sourceConfig)
    {
        sourceConfig->callback = callback;
    }
    return sourceConfig;
}

void SOPC_PubSourceVariableConfig_Delete(SOPC_PubSourceVariableConfig* sourceConfig)
{
    SOPC_Free(sourceConfig);
}

SOPC_DataValue* SOPC_PubSourceVariable_GetVariables(const SOPC_PubSourceVariableConfig* sourceConfig, //.
                                                    const OpcUa_ReadValueId* readValues,
                                                    const int32_t nbValues) //
{
    if (NULL == sourceConfig || NULL == readValues)
    {
        return NULL;
    }
    return sourceConfig->callback(readValues, nbValues);
}
