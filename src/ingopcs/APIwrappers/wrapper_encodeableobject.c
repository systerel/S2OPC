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

#include "wrapper_encodeableobject.h"
#include "sopc_encodeable.h"

SOPC_StatusCode OpcUa_EncodeableObject_Create(SOPC_EncodeableType* encTyp, void** encObject){
    return SOPC_Encodeable_Create(encTyp, encObject);
}

SOPC_StatusCode OpcUa_EncodeableObject_Delete(SOPC_EncodeableType* encTyp, void** encObject){
    return SOPC_Encodeable_Delete(encTyp, encObject);
}

SOPC_StatusCode OpcUa_EncodeableObject_CreateExtension(SOPC_EncodeableType*  encTyp,
                                                       SOPC_ExtensionObject* extObj,
                                                       void**                encObject){
    return SOPC_Encodeable_CreateExtension(extObj, encTyp, encObject);
}
