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

#ifndef SOPC_ENCODEABLE_H_
#define SOPC_ENCODEABLE_H_

#include <stdint.h>

#include "sopc_base_types.h"

BEGIN_EXTERN_C

struct SOPC_MsgBuffer;

typedef void (SOPC_EncodeableObject_PfnInitialize) (void* value);
typedef void (SOPC_EncodeableObject_PfnClear) (void* value);
typedef void (SOPC_EncodeableObject_PfnGetSize) (void); // Deactivated
// TODO: replace by status code => need to be separated from builtin types to avoid mutal dep
typedef uint32_t (SOPC_EncodeableObject_PfnEncode) (void* value, struct SOPC_MsgBuffer* msgBuffer);
typedef uint32_t (SOPC_EncodeableObject_PfnDecode) (void* value, struct SOPC_MsgBuffer* msgBuffer);

typedef struct SOPC_EncodeableType {
    char*                                TypeName;
    uint32_t                             TypeId;
    uint32_t                             BinaryEncodingTypeId;
    uint32_t                             XmlEncodingTypeId;
    char*                                NamespaceUri;
    uint32_t                             AllocationSize;
    SOPC_EncodeableObject_PfnInitialize* Initialize;
    SOPC_EncodeableObject_PfnClear*      Clear;
    SOPC_EncodeableObject_PfnGetSize*    GetSize;
    SOPC_EncodeableObject_PfnEncode*     Encode;
    SOPC_EncodeableObject_PfnDecode*     Decode;
} SOPC_EncodeableType;


SOPC_EncodeableType* EncodeableType_GetEncodeableType(SOPC_EncodeableType** encTypesTable, // null terminated enctype* list
                                                      const char*           namespac,
                                                      uint32_t              typeId);

END_EXTERN_C

#endif /* SOPC_ENCODEABLE_H_ */
