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

#include "sopc_encodeable.h"

#include <stdlib.h>
#include <string.h>

SOPC_ReturnStatus SOPC_Encodeable_Create(SOPC_EncodeableType* encTyp, void** encObject)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (encTyp != NULL && encTyp->Initialize != NULL && encTyp->AllocationSize > 0 && encObject != NULL &&
        NULL == *encObject)
    {
        *encObject = malloc(encTyp->AllocationSize);
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
        free(*encObject);
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
            extObject->Encoding = SOPC_ExtObjBodyEncoding_Object;
            extObject->Body.Object.ObjType = encTyp;
            extObject->Body.Object.Value = *encObject;
        }
    }
    return status;
}
