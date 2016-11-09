/*
 *  Copyright (C) 2016 Systerel and others.
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

#include "sopc_encodeable.h"

#include <string.h>

#include "sopc_builtintypes.h"

SOPC_EncodeableType* EncodeableType_GetEncodeableType(SOPC_EncodeableType** encTypesTable,
                                                    const char*         namespace,
                                                    uint32_t            typeId){
    SOPC_EncodeableType* current = NULL;
    SOPC_EncodeableType* result = NULL;
    uint32_t idx = 0;
    if(encTypesTable != NULL){
        current = encTypesTable[idx];
        while(current != NULL && result == NULL){
            if(typeId == result->TypeId || typeId == result->BinaryEncodingTypeId){
                // || typeId = result->xmlTypeId => should not be the case since we use UA binary !
                //TODO: max namespace or string size ?
                if(strncmp(namespace, current->NamespaceUri, INT32_MAX) == 0){ // Use maximum SOPC_String size representation size
                    result = current;
                }
            }else if(idx < UINT32_MAX){
                idx++;
                current = encTypesTable[idx];
            }else{
                current = NULL;
            }
        }
    }
    return result;
}
