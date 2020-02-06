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

#include "sopc_addspace_loader.h"

extern const bool sopc_embedded_is_const_addspace;

extern SOPC_AddressSpace_Node SOPC_Embedded_AddressSpace_Nodes[];
extern const uint32_t SOPC_Embedded_AddressSpace_nNodes;

extern SOPC_Variant SOPC_Embedded_VariableVariant[];
extern const uint32_t SOPC_Embedded_VariableVariant_nb;

#define SOPC_AccessLevelMask_StatusWrite (uint8_t) 32    // bit5
#define SOPC_AccessLevelMask_TimestampWrite (uint8_t) 64 // bit6

static bool checkAccessLevelsForbidsStatusAndTs(void)
{
    bool result = true;
    for (uint32_t i = 0; i < SOPC_Embedded_AddressSpace_nNodes; i++)
    {
        SOPC_AddressSpace_Node* node = &SOPC_Embedded_AddressSpace_Nodes[i];
        if (OpcUa_NodeClass_Variable == node->node_class)
        {
            // Check AccessLevel does not allow to write Status or SourceTimestamp
            result = result && ((SOPC_AccessLevelMask_StatusWrite | SOPC_AccessLevelMask_TimestampWrite) &
                                node->data.variable.AccessLevel) == 0;
        }
    }
    return result;
}

SOPC_AddressSpace* SOPC_Embedded_AddressSpace_Load(void)
{
    SOPC_AddressSpace* space = NULL;

    if (sopc_embedded_is_const_addspace)
    {
        if (checkAccessLevelsForbidsStatusAndTs())
        {
            space = SOPC_AddressSpace_CreateReadOnlyNodes(
                SOPC_Embedded_AddressSpace_nNodes, SOPC_Embedded_AddressSpace_Nodes, SOPC_Embedded_VariableVariant_nb,
                SOPC_Embedded_VariableVariant);
        }
    }
    else
    {
        space = SOPC_AddressSpace_Create(false);

        if (space == NULL)
        {
            return NULL;
        }

        for (uint32_t i = 0; i < SOPC_Embedded_AddressSpace_nNodes; ++i)
        {
            SOPC_AddressSpace_Node* node = &SOPC_Embedded_AddressSpace_Nodes[i];

            if (SOPC_AddressSpace_Append(space, node) != SOPC_STATUS_OK)
            {
                SOPC_AddressSpace_Delete(space);
                return NULL;
            }
        }
    }

    return space;
}
