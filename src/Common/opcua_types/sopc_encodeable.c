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
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (encTyp != NULL && encTyp->Initialize != NULL && encTyp->AllocationSize > 0 && encObject != NULL &&
        NULL == *encObject)
    {
        *encObject = SOPC_Malloc(encTyp->AllocationSize);
        if (*encObject != NULL)
        {
            encTyp->Initialize(*encObject);
            status = SOPC_STATUS_OK;
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
    }
    return status;
}

SOPC_ReturnStatus SOPC_Encodeable_Delete(SOPC_EncodeableType* encTyp, void** encObject)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (encTyp != NULL && encTyp->Clear != NULL && encObject != NULL && *encObject != NULL)
    {
        encTyp->Clear(*encObject);
        SOPC_Free(*encObject);
        *encObject = NULL;
        status = SOPC_STATUS_OK;
    }
    return status;
}

SOPC_ReturnStatus SOPC_Encodeable_CreateExtension(SOPC_ExtensionObject* extObject,
                                                  SOPC_EncodeableType* encTyp,
                                                  void** encObject)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (extObject != NULL && extObject->Encoding == SOPC_ExtObjBodyEncoding_None)
    {
        status = SOPC_Encodeable_Create(encTyp, encObject);
        if (SOPC_STATUS_OK == status)
        {
            SOPC_ExpandedNodeId_Initialize(&extObject->TypeId);
            /* extObject->TypeId.NamespaceUri is left empty, as it is the case for default OPC-UA types */
            extObject->TypeId.NodeId.IdentifierType = SOPC_IdentifierType_Numeric;
            extObject->TypeId.NodeId.Namespace = OPCUA_NAMESPACE_INDEX;
            extObject->TypeId.NodeId.Data.Numeric = encTyp->BinaryEncodingTypeId;
            extObject->Encoding = SOPC_ExtObjBodyEncoding_Object;
            extObject->Body.Object.ObjType = encTyp;
            extObject->Body.Object.Value = *encObject;
        }
        else
        {
            SOPC_ReturnStatus deleteStatus = SOPC_Encodeable_Delete(encTyp, encObject);
            SOPC_ASSERT(SOPC_STATUS_OK == deleteStatus);
        }
    }
    return status;
}

SOPC_ReturnStatus SOPC_Encodeable_Move(void* destObj, void* srcObj)
{
    if (NULL == destObj || NULL == srcObj || destObj == srcObj ||
        *(SOPC_EncodeableType**) destObj != *(SOPC_EncodeableType**) srcObj)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_EncodeableType* encType = *(SOPC_EncodeableType**) srcObj;

    memcpy(destObj, srcObj, encType->AllocationSize);
    SOPC_EncodeableObject_Initialize(encType, srcObj);

    return SOPC_STATUS_OK;
}
