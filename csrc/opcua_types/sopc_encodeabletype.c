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

#include "sopc_encodeabletype.h"

#include <string.h>

#include "sopc_helper_string.h"
#include "sopc_builtintypes.h"
#include "sopc_namespace_table.h"

SOPC_EncodeableType* SOPC_EncodeableType_GetEncodeableType(SOPC_EncodeableType** encTypesTable,
                                                           const char*           namespace,
                                                           uint32_t              typeId){
    SOPC_EncodeableType* current = NULL;
    const char* currentNs = NULL;
    SOPC_EncodeableType* result = NULL;
    uint32_t idx = 0;
    if(encTypesTable != NULL){
        current = encTypesTable[idx];
        while(current != NULL && result == NULL){
            if(typeId == current->TypeId || typeId == current->BinaryEncodingTypeId){
                // || typeId = current->xmlTypeId => should not be the case since we use UA binary !
                if(current->NamespaceUri == NULL && namespace == NULL){
                    // Default namespace for both
                    result = current;
                }else{
                    if(namespace == NULL){
                        namespace = OPCUA_NAMESPACE_NAME;
                    }
                    if(current->NamespaceUri == NULL){
                        // It is considered as default namespace:
                        currentNs = OPCUA_NAMESPACE_NAME;
                    }else{
                        currentNs = current->NamespaceUri;
                    }
                    if(SOPC_strncmp_ignore_case(namespace, currentNs, strlen(namespace)+1) == 0){
                        result = current;
                    }
                }
            }
            if(result == NULL && idx < UINT32_MAX){
                idx++;
                current = encTypesTable[idx];
            }else{
                current = NULL;
            }
        }
    }
    return result;
}
