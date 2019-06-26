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

#include "loader.h"

extern SOPC_AddressSpace_Item SOPC_Embedded_AddressSpace_Items[];
extern uint32_t SOPC_Embedded_AddressSpace_nItems;

extern SOPC_Variant SOPC_Embedded_VariableVariant[];
extern uint32_t SOPC_Embedded_VariableVariant_nb;

SOPC_AddressSpace* SOPC_Embedded_AddressSpace_Load(void)
{
    SOPC_AddressSpace* space =
        SOPC_AddressSpace_CreateReadOnlyItems(SOPC_Embedded_VariableVariant_nb, SOPC_Embedded_VariableVariant);

    if (space == NULL)
    {
        return NULL;
    }

    for (uint32_t i = 0; i < SOPC_Embedded_AddressSpace_nItems; ++i)
    {
        SOPC_AddressSpace_Item* item = &SOPC_Embedded_AddressSpace_Items[i];

        if (SOPC_AddressSpace_Append(space, item) != SOPC_STATUS_OK)
        {
            SOPC_AddressSpace_Delete(space);
            return NULL;
        }
    }

    return space;
}
