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

#include "sopc_encodeable.h"

#include <stdlib.h>
#include <string.h>

#include "sopc_namespace_table.h"

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
