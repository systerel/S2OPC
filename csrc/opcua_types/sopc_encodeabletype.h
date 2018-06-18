/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
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
 *  \brief          Retrieve a defined encodeable object type with the given type Id.
 *
 *  \param typeId         Type identifier for which corresponding encodeable object type must be returned
 *  \return               The searched encodeable type or NULL if parameters are incorrect or type is not found
 */
SOPC_EncodeableType* SOPC_EncodeableType_GetEncodeableType(uint32_t typeId);

const char* SOPC_EncodeableType_GetName(SOPC_EncodeableType* encType);

#endif /* SOPC_ENCODEABLETYPE_H_ */
