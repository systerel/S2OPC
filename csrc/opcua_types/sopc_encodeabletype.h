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

/**
 *  \file sopc_encodeabletype.h
 *
 *  \brief Encodeable type and services
 */

#ifndef SOPC_ENCODEABLETYPE_H_
#define SOPC_ENCODEABLETYPE_H_

#include <stddef.h>
#include <stdint.h>

#include "sopc_buffer.h"
#include "sopc_toolkit_constants.h"

/**
 *  \brief Initialization function generic signature for an encodeable object
 */
typedef void(SOPC_EncodeableObject_PfnInitialize)(void* value);

/**
 *  \brief Clear function generic signature for an encodeable object
 */
typedef void(SOPC_EncodeableObject_PfnClear)(void* value);

/**
 *  \brief Get size function generic signature for an encodeable object
 *  Note: Unused in INGOPCS, NULL pointer may be provided instead of function pointer
 */
typedef void(SOPC_EncodeableObject_PfnGetSize)(void);

/**
 *  \brief Encoding function generic signature for an encodeable object
 */
typedef SOPC_ReturnStatus(SOPC_EncodeableObject_PfnEncode)(const void* value, SOPC_Buffer* msgBuffer);

/**
 *  \brief Decoding function generic signature for an encodeable object
 */
typedef SOPC_ReturnStatus(SOPC_EncodeableObject_PfnDecode)(void* value, SOPC_Buffer* msgBuffer);

/**
 *  \brief Encodeable object type structure definition. It provides all the services
 *  functions associated with the encodeable object for encoding needs.
 */
typedef struct SOPC_EncodeableType
{
    char* TypeName;
    uint32_t TypeId;
    uint32_t BinaryEncodingTypeId;
    uint32_t XmlEncodingTypeId;
    char* NamespaceUri;
    size_t AllocationSize;
    SOPC_EncodeableObject_PfnInitialize* Initialize;
    SOPC_EncodeableObject_PfnClear* Clear;
    SOPC_EncodeableObject_PfnGetSize* GetSize;
    SOPC_EncodeableObject_PfnEncode* Encode;
    SOPC_EncodeableObject_PfnDecode* Decode;
} SOPC_EncodeableType;

/**
 *  \brief          Retrieve a defined encodeable object type from the given encodeable types table
 *                  with the given namespace and type Id.
 *
 *  \param encTypesTable  Encodeable types table (NULL terminated) in which encodeable object type is searched for
 *  \param namespac       Namespace in which the given type Id is searched for
 *  \param typeId         Type identifier for which corresponding encodeable object type must be returned
 *  \return               The searched encodeable type or NULL if parameters are incorrect or type is not found
 */
SOPC_EncodeableType* SOPC_EncodeableType_GetEncodeableType(SOPC_EncodeableType** encTypesTable,
                                                           const char* namespac,
                                                           uint32_t typeId);

#endif /* SOPC_ENCODEABLETYPE_H_ */
