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
 *  \brief EncodeableType and services on encodeable object
 *
 *  An ::SOPC_EncodeableType is the description uniquely identified OPC UA type which is composed of an ordered list
 * of type fields which can be either ::SOPC_EncodeableType or OPC UA built-in types (see sopc_builtintypes.h).
 *
 * An instance of an ::SOPC_EncodeableType type is a C structure which contains as first field a pointer to its
 * ::SOPC_EncodeableType and then the fields of expected types described by its ::SOPC_EncodeableType
 * (see sopc_types.h, e.g. ::OpcUa_ReadRequest is the C structure instance of encodeable type
 * ::OpcUa_ReadRequest_EncodeableType).
 *
 * An encodeable object is either an instance of an OPC UA built-in type or an instance of an ::SOPC_EncodeableType.
 * Each encodeable object has several services functions:
 * - Initialize the encodeable object (C structure or simple C type)
 * - Encode the encodeable object into a byte buffer
 * - Decode a byte buffer into the encodeable object
 * - Clear the encodeable object
 *
 * Encodeable object types are defined in sopc_builtintypes.h for built-in types and  sopc_types.h for
 * internally defined ::SOPC_EncodeableType types.
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
 *  \brief Encoding function generic signature for an encodeable object
 *  *
 *  \param value             The encodeable object instance to encode into \p buffer.
 *                           it might be either of encodeable type object with ::SOPC_EncodeableType* as first field
 *                           or a built-in type object.
 *  \param buffer            The buffer in which the encodeable object will be encoded
 *  \param nestedStructLevel The number of structure levels encoded until then
 *
 *  \return                  ::SOPC_STATUS_OK in case of success, the appropriate error status otherwise.
 */
typedef SOPC_ReturnStatus(SOPC_EncodeableObject_PfnEncode)(const void* value,
                                                           SOPC_Buffer* buffer,
                                                           uint32_t nestedStructLevel);

/**
 *  \brief Decoding function generic signature for an encodeable object
 *
 *  \param value             The encodeable object instance in which \p buffer will be decoded,
 *                           it might be either of encodeable type object with ::SOPC_EncodeableType* as first field
 *                           or a built-in type object.
 *  \param buffer            The buffer to decode to fill the encodeable object content
 *  \param nestedStructLevel The number of structure levels decoded until then
 *
 *  \return                  ::SOPC_STATUS_OK in case of success, the appropriate error status otherwise.
 */
typedef SOPC_ReturnStatus(SOPC_EncodeableObject_PfnDecode)(void* value,
                                                           SOPC_Buffer* msgBuffer,
                                                           uint32_t nestedStructLevel);

/**
 *  \brief Copy function generic signature for an encodeable object
 *
 *  \param dest  The encodeable object instance in which copy will be done
 *               It shall be either an encodeable type object with ::SOPC_EncodeableType* as first field
 *               or a built-in type object.
 *  \param src   The encodeable object instance copied into \p dest.
 *               It shall be either an encodeable type object with ::SOPC_EncodeableType* as first field
 *               or a built-in type object.
 *  \return      ::SOPC_STATUS_OK in case of success, the appropriate error status otherwise.
 */
typedef SOPC_ReturnStatus(SOPC_EncodeableObject_PfnCopy)(void* dest, const void* src);

/**
 *  \brief Compare function generic signature for an encodeable object
 *
 *  \param left      The left operand encodeable object instance to be compared.
 *                   It shall be either an encodeable type object with ::SOPC_EncodeableType* as first field
 *                   or a built-in type object.
 *  \param right     The right operand encodeable object instance to be compared.
 *                   It shall be either an encodeable type object with ::SOPC_EncodeableType* as first field
 *                   or a built-in type object.
 * \param[out] comp  Pointer to an integer that will store the comparison result when returned status is
 *                   SOPC_STATUS_OK. In this latter case:
 *                   - \p comp &lt; 0 if \p left  &lt;  \p right,
 *                   - \p comp &gt; 0 if \p left &gt; \p right and
 *                   - \p comp == 0 if \p left == \p right.
 *  \return          ::SOPC_STATUS_OK in case of success, the appropriate error status otherwise.
 */
typedef SOPC_ReturnStatus(SOPC_EncodeableObject_PfnComp)(const void* left, const void* right, int32_t* comp);

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
 * The \c typeIndex field indicates the index of type in internal types array.
 * It shall be a valid value of ::SOPC_TypeInternalIndex enum type.
 * Note: fields can only be of internal defined types for user-defined types.
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
 *  \brief          Retrieve a user-defined encodeable type with the given type Id.
 *
 *  \param typeId         Type identifier for which corresponding encodeable type must be returned
 *  \return               The searched encodeable type or NULL if parameters are incorrect or type is not found
 */
SOPC_EncodeableType* SOPC_EncodeableType_GetUserType(uint32_t typeId);

/**
 *  \brief          Retrieve a defined encodeable type with the given type Id.
 *                  It can be a internal defined type or user-defined type.
 *
 *  \param typeId         Type identifier for which corresponding encodeable type must be returned
 *  \return               The searched encodeable type or NULL if parameters are incorrect or type is not found
 */
SOPC_EncodeableType* SOPC_EncodeableType_GetEncodeableType(uint32_t typeId);

/**
 *  \brief          Get the name of the given encodeable type
 *
 *  \param encType  The encodeable type for which name is requested
 *  \return         The name of the encodeable type as a C string
 */
const char* SOPC_EncodeableType_GetName(SOPC_EncodeableType* encType);

/**
 * \brief Initialize an encodeable object of the given encodeable type.
 *
 * \param type   The encodeable type of the object instance to initialize.
 * \param pValue An object instance of the appropriate encodeable type.
 *               It shall at least have allocation size described in the encodeable type
 *               and is expected to be the C structure corresponding to an instance of the encodeable type
 *               (The first field of the structure shall be a ::SOPC_EncodeableType*
 *                which value will be set to \p type.
 *                The following fields shall have types described by \p type)
 */
void SOPC_EncodeableObject_Initialize(SOPC_EncodeableType* type, void* pValue);

/**
 * \brief Clear an encodeable object of the given encodeable type.
 *
 * \param type   The encodeable type of the object instance to clear.
 * \param pValue An object instance of the appropriate encodeable type.
 *               It shall at least have allocation size described in the encodeable type
 *               and is expected to be the C structure corresponding to an instance of the encodeable type
 *               (The first field of the structure shall be a ::SOPC_EncodeableType*
 *                which value shall be \p type.
 *                The following fields shall have types described by \p type)
 */
void SOPC_EncodeableObject_Clear(SOPC_EncodeableType* type, void* pValue);

/**
 * \brief Encode an encodeable object of the given encodeable type into a bytes buffer.
 *
 * \param type               The encodeable type of the object instance to encode.
 * \param pValue             The object instance of the appropriate encodeable type to encode.
 *                           It shall at least have allocation size described in the encodeable type
 *                           and shall be the C structure corresponding to an instance of the encodeable type
 *                           (The first field of the structure shall be a ::SOPC_EncodeableType*
 *                            which value shall be \p type. The following fields shall have types described by \p type)
 *
 * \param buf               The buffer in which the encodeable object will be encoded
 * \param nestedStructLevel The number of structure levels encoded until then.
 *                          Value 0 shall be used for first call.
 *
 *  \return                 A status code indicating the result of operation
 */
SOPC_ReturnStatus SOPC_EncodeableObject_Encode(SOPC_EncodeableType* type,
                                               const void* pValue,
                                               SOPC_Buffer* buf,
                                               uint32_t nestedStructLevel);

/**
 * \brief Decode an encodeable object of the given encodeable type from a bytes buffer.
 *
 * \param type               The encodeable type of the object instance to decode.
 * \param pValue             An initialized object instance of the appropriate encodeable type to decode.
 *                           It shall at least have allocation size described in the encodeable type
 *                           and shall be the C structure corresponding to an instance of the encodeable type
 *                           (The first field of the structure shall be a ::SOPC_EncodeableType*
 *                            which value shall be \p type. The following fields shall have types described by \p type)
 *
 * \param buf               The buffer to decode to fill the encodeable object content
 * \param nestedStructLevel The number of structure levels decoded until then
 *                          Value 0 shall be used for first call.
 *
 *  \return                 A status code indicating the result of operation
 */
SOPC_ReturnStatus SOPC_EncodeableObject_Decode(SOPC_EncodeableType* type,
                                               void* pValue,
                                               SOPC_Buffer* buf,
                                               uint32_t nestedStructLevel);

/**
 * \brief Copy an encodeable object of the given encodeable type.
 *
 * \param type       The encodeableType of \p destValue and \p srcValue.
 * \param destValue  The destination encodeable object instance of the given encodeable type
 *                   in which the content copy will be done.
 *                   It shall have been initialized ::SOPC_EncodeableObject_Initialize and
 *                   cleared ::SOPC_EncodeableObject_Clear if necessary with \p type as encodeable type.
 *
 * \param srcValue   The source encodeable object instance of the given encodeable type
 *                   from which the content copy will be done.
 *                   It shall at least have allocation size described in the encodeable type
 *                   and shall be the C structure corresponding to an instance of the encodeable type
 *                   (The first field of the structure shall be a ::SOPC_EncodeableType*
 *                    which value shall be \p type. The following fields shall have types described by \p type)
 *
 * \return           SOPC_STATUS_OK in case of success, reason of failure otherwise.
 */
SOPC_ReturnStatus SOPC_EncodeableObject_Copy(SOPC_EncodeableType* type, void* destValue, const void* srcValue);

/**
 * \brief Compare 2 encodeable objects of the given encodeable type.
 *
 * \param type         The encodeableType of \p leftValue and \p rightValue.
 * \param leftValue    The left encodeable object of the given encodeable type operand to compare.
 *                     It shall at least have allocation size described in the encodeable type
 *                     and shall be the C structure corresponding to an instance of the encodeable type
 *                     (The first field of the structure shall be a ::SOPC_EncodeableType*
 *                     which value shall be \p type. The following fields shall have types described by \p type)
 * \param rightValue   The right encodeable object of the given encodeable type operand to compare.
 *                     It shall at least have allocation size described in the encodeable type
 *                     and shall be the C structure corresponding to an instance of the encodeable type
 *                     (The first field of the structure shall be a ::SOPC_EncodeableType*
 *                     which value shall be \p type. The following fields shall have types described by \p type)
 * \param[out] comp    Pointer to an integer that will store the comparison result when returned status is
 *                     SOPC_STATUS_OK. In this latter case:
 *                     - \p comp &lt; 0 if \p leftValue  &lt;  \p rightValue,
 *                     - \p comp &gt; 0 if \p leftValue &gt; \p rightValue and
 *                     - \p comp == 0 if \p leftValue == \p rightValue.
 *
 * \return             SOPC_STATUS_OK in case of success, reason of failure otherwise.
 */
SOPC_ReturnStatus SOPC_EncodeableObject_Compare(SOPC_EncodeableType* type,
                                                const void* leftValue,
                                                const void* rightValue,
                                                int32_t* comp);

#endif /* SOPC_ENCODEABLETYPE_H_ */
