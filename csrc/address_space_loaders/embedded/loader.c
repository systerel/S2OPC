/*
 *  Copyright (C) 2018 Systerel and others.
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

#include "loader.h"

extern SOPC_AddressSpace_Item SOPC_Embedded_AddressSpace_Items[];
extern uint32_t SOPC_Embedded_AddressSpace_nItems;

SOPC_AddressSpace* SOPC_Embedded_AddressSpace_Load(void)
{
    SOPC_AddressSpace* space = SOPC_AddressSpace_Create(false);

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
