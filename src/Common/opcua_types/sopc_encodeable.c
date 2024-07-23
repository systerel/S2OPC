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

#include <string.h>

#include "opcua_identifiers.h"
#include "sopc_assert.h"
#include "sopc_encodeable.h"
#include "sopc_mem_alloc.h"

SOPC_ReturnStatus SOPC_Encodeable_Create(SOPC_EncodeableType* encTyp, void** encObject)
{
    return SOPC_EncodeableObject_Create(encTyp, encObject);
}

SOPC_ReturnStatus SOPC_Encodeable_Delete(SOPC_EncodeableType* encTyp, void** encObject)
{
    return SOPC_EncodeableObject_Delete(encTyp, encObject);
}

SOPC_ReturnStatus SOPC_Encodeable_CreateExtension(SOPC_ExtensionObject* extObject,
                                                  SOPC_EncodeableType* encTyp,
                                                  void** encObject)
{
    return SOPC_ExtensionObject_CreateObject(extObject, encTyp, encObject);
}

SOPC_ReturnStatus SOPC_Encodeable_Move(void* destObj, void* srcObj)
{
    return SOPC_EncodeableObject_Move(destObj, srcObj);
}
