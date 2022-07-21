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
#include "sopc_enums.h"

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
 *  Note: Unused in S2OPC, NULL pointer may be provided instead of function pointer
 */
typedef void(SOPC_EncodeableObject_PfnGetSize)(void);

/**
 *  \brief Encoding function generic signature for an encodeable object
 */
typedef SOPC_ReturnStatus(SOPC_EncodeableObject_PfnEncode)(const void* value,
                                                           SOPC_Buffer* msgBuffer,
                                                           uint32_t nestedStructLevel);

/**
 *  \brief Decoding function generic signature for an encodeable object
 */
typedef SOPC_ReturnStatus(SOPC_EncodeableObject_PfnDecode)(void* value,
                                                           SOPC_Buffer* msgBuffer,
                                                           uint32_t nestedStructLevel);

/*
 * \brief Description of an encodeable type field.
 *
 * This structure has been designed to be very compact and fit into 8 bytes.
 * The \c isBuiltIn field indicates whether the field type is built-in or
 * defined in the address space.  In the first case, the type index is a member
 * of the \c SOPC_BuiltinId enumeration.  In the second case, it is a member of
 * the \c SOPC_TypeInternalIndex enumeration.
 *
 * The \c isArrayLength field indicates whether this field contains the length
 * of an array.  When true, the field must be of built-in type \c Int32, and the
 * array is described by the next field.
 *
 * The \c isToEncode field indicates whether this field shall be encoded and
 * decoded.  When false, the field is only initialized and cleared.
 *
 * Finally, the \c offset field gives the offset in bytes of the field in the
 * object structure.
 */
typedef struct SOPC_EncodeableType_FieldDescriptor
{
    bool isBuiltIn : 1;
    bool isArrayLength : 1;
    bool isToEncode : 1;
    uint32_t typeIndex : 29;
    uint32_t offset;
} SOPC_EncodeableType_FieldDescriptor;

/**
 *  \brief Encodeable object type structure definition. It provides all the services
 *  functions associated with the encodeable object for encoding needs.
 */
typedef const struct SOPC_EncodeableType_Struct
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
    int32_t NoOfFields;
    const SOPC_EncodeableType_FieldDescriptor* Fields;
} SOPC_EncodeableType;

/**
 * \brief       Registers a user-defined encodeable type.
 *              further calls to SOPC_EncodeableType_GetEncodeableType
 *              will successfully identify the registered encodeable type
 *
 * \note        All registered encoders must be freed by
 *              SOPC_EncodeableType_RemoveUserType
 *
 *  \param encoder        The encoder definition to register
 *  \return               A status code indicating the result of operation
 */
SOPC_ReturnStatus SOPC_EncodeableType_AddUserType(SOPC_EncodeableType* encoder);

/**
 * \brief       Removes a user-defined encodeable type previously created by
 *                      SOPC_EncodeableType_AddUserType
 *
 *  \param encoder        The encoder definition to register
 *  \return               A status code indicating the result of operation
 */
SOPC_ReturnStatus SOPC_EncodeableType_RemoveUserType(SOPC_EncodeableType* encoder);

/**
 *  \brief          Retrieve a defined encodeable object type with the given type Id.
 *
 *  \param typeId         Type identifier for which corresponding encodeable object type must be returned
 *  \return               The searched encodeable type or NULL if parameters are incorrect or type is not found
 */
SOPC_EncodeableType* SOPC_EncodeableType_GetEncodeableType(uint32_t typeId);

const char* SOPC_EncodeableType_GetName(SOPC_EncodeableType* encType);

/**
 * \brief Initialize an encodeable object.
 *
 * The \c pValue parameter shall correspond to an object of the appropriate
 * type.
 */
void SOPC_EncodeableObject_Initialize(SOPC_EncodeableType* type, void* pValue);

/**
 * \brief Clear an encodeable object.
 *
 * The \c pValue parameter shall correspond to an object of the appropriate
 * type.
 */
void SOPC_EncodeableObject_Clear(SOPC_EncodeableType* type, void* pValue);

/**
 * \brief Encode an encodeable object.
 *
 * The \c pValue parameter shall correspond to an object of the appropriate
 * type.
 */
SOPC_ReturnStatus SOPC_EncodeableObject_Encode(SOPC_EncodeableType* type,
                                               const void* pValue,
                                               SOPC_Buffer* buf,
                                               uint32_t nestedStructLevel);

/**
 * \brief Decode an encodeable object.
 *
 * The \c pValue parameter shall correspond to an object of the appropriate
 * type.
 */
SOPC_ReturnStatus SOPC_EncodeableObject_Decode(SOPC_EncodeableType* type,
                                               void* pValue,
                                               SOPC_Buffer* buf,
                                               uint32_t nestedStructLevel);

#endif /* SOPC_ENCODEABLETYPE_H_ */
