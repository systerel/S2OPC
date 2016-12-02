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

#ifndef SOPC_WRAPPER_ENCODEABLEOBJECT_H_
#define SOPC_WRAPPER_ENCODEABLEOBJECT_H_

#include "sopc_encodeabletype.h"
#include "sopc_builtintypes.h"

SOPC_StatusCode OpcUa_EncodeableObject_Create(SOPC_EncodeableType* encTyp, void** encObject);
SOPC_StatusCode OpcUa_EncodeableObject_Delete(SOPC_EncodeableType* encTyp, void** encObject);
SOPC_StatusCode OpcUa_EncodeableObject_CreateExtension(SOPC_EncodeableType*  encTyp,
                                                       SOPC_ExtensionObject* extObj,
                                                       void**                encObject);

#endif /* SOPC_WRAPPER_ENCODEABLEOBJECT_H_ */
