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

#include "sopc_encodeabletype.h"

#include <string.h>

#include "opcua_identifiers.h"
#include "sopc_assert.h"
#include "sopc_builtintypes.h"
#include "sopc_common_constants.h"
#include "sopc_encoder.h"
#include "sopc_helper_string.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_types.h"

const char* nullType = "NULL";
const char* noNameType = "NoName";

SOPC_Dict* g_UserEncodeableTypes = NULL;

typedef struct
{
    uint16_t nsIndex;
    uint32_t typeId;
} SOPC_EncodeableType_UserTypeKey;

typedef struct
{
    SOPC_EncodeableType* encoder;
} SOPC_EncodeableType_UserTypeValue;

static SOPC_ReturnStatus SOPC_EncodeableObject_InternalInitialize(SOPC_EncodeableType* type, void* pValue);

static uint64_t typeId_hash(const uintptr_t data)
{
    return ((uint64_t)((const SOPC_EncodeableType_UserTypeKey*) data)->typeId) +
           ((uint64_t)(((const SOPC_EncodeableType_UserTypeKey*) data)->nsIndex) << 32);
}

static bool typeId_equal(const uintptr_t a, const uintptr_t b)
{
    return ((const SOPC_EncodeableType_UserTypeKey*) a)->nsIndex ==
               ((const SOPC_EncodeableType_UserTypeKey*) b)->nsIndex &&
           ((const SOPC_EncodeableType_UserTypeKey*) a)->typeId == ((const SOPC_EncodeableType_UserTypeKey*) b)->typeId;
}

static void type_free(uintptr_t data)
{
    SOPC_Free((void*) data);
}

static SOPC_ReturnStatus insertKeyInUserTypes(SOPC_EncodeableType* pEncoder,
                                              const uint16_t nsIndex,
                                              const uint32_t typeId)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    SOPC_EncodeableType_UserTypeKey* pKey = NULL;
    SOPC_EncodeableType_UserTypeValue* pValue = NULL;
    bool inserted = false;

    pKey = (SOPC_EncodeableType_UserTypeKey*) SOPC_Malloc(sizeof(*pKey));
    if (pKey == NULL)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }
    pValue = (SOPC_EncodeableType_UserTypeValue*) SOPC_Malloc(sizeof(*pValue));
    if (pValue == NULL)
    {
        SOPC_Free(pKey);
        return SOPC_STATUS_OUT_OF_MEMORY;
    }
    pKey->nsIndex = nsIndex;
    pKey->typeId = typeId;
    pValue->encoder = pEncoder;
    inserted = SOPC_Dict_Insert(g_UserEncodeableTypes, (uintptr_t) pKey, (uintptr_t) pValue);

    if (!inserted)
    {
        SOPC_Free(pKey);
        SOPC_Free(pValue);
        result = SOPC_STATUS_NOK;
    }

    return result;
}

SOPC_ReturnStatus SOPC_EncodeableType_AddUserType(SOPC_EncodeableType* pEncoder)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    SOPC_EncodeableType* prevEncoder = NULL;

    if (NULL == pEncoder)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ASSERT(NULL == pEncoder->NamespaceUri && "EncType Namespace URI translation unsupported");
    prevEncoder = SOPC_EncodeableType_GetEncodeableType(pEncoder->NamespaceIndex, pEncoder->TypeId);
    if (prevEncoder != NULL)
    {
        // This TypeId is already used.
        return SOPC_STATUS_NOT_SUPPORTED;
    }

    if (g_UserEncodeableTypes == NULL)
    {
        // Create dictionary
        g_UserEncodeableTypes = SOPC_Dict_Create(0, typeId_hash, typeId_equal, type_free, type_free);
        if (g_UserEncodeableTypes == NULL)
        {
            result = SOPC_STATUS_OUT_OF_MEMORY;
        }
        else
        {
            SOPC_Dict_SetTombstoneKey(g_UserEncodeableTypes, (uintptr_t) 0xFFFFFFFE);
        }
    }

    // Note : Values are released when removed from Dict objects, so that allocations have to be done
    // for each value inserted

    // insert both TypeId and BinaryEncodingTypeId keys to allow decoder to be correctly identified
    if (result == SOPC_STATUS_OK)
    {
        result = insertKeyInUserTypes(pEncoder, pEncoder->NamespaceIndex, pEncoder->TypeId);
    }
    if (result == SOPC_STATUS_OK)
    {
        result = insertKeyInUserTypes(pEncoder, pEncoder->NamespaceIndex, pEncoder->BinaryEncodingTypeId);
    }
    return result;
}

SOPC_ReturnStatus SOPC_EncodeableType_RemoveUserType(SOPC_EncodeableType* encoder)
{
    SOPC_EncodeableType_UserTypeKey key = {0};
    SOPC_EncodeableType* prevEncoder = NULL;
    if (encoder == NULL)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (g_UserEncodeableTypes == NULL)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ASSERT(NULL == encoder->NamespaceUri && "EncType Namespace URI translation unsupported");
    key.nsIndex = encoder->NamespaceIndex;
    key.typeId = encoder->TypeId;

    prevEncoder = (SOPC_EncodeableType*) SOPC_Dict_GetKey(g_UserEncodeableTypes, (const uintptr_t) &key, NULL);
    if (prevEncoder == NULL)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_Dict_Remove(g_UserEncodeableTypes, (const uintptr_t) &key);
    key.typeId = encoder->BinaryEncodingTypeId;
    SOPC_Dict_Remove(g_UserEncodeableTypes, (const uintptr_t) &key);
    // Delete the dictionary if empty
    if (SOPC_Dict_Size(g_UserEncodeableTypes) == 0)
    {
        SOPC_Dict_Delete(g_UserEncodeableTypes);
        g_UserEncodeableTypes = NULL;
    }
    return SOPC_STATUS_OK;
}

static void SOPC_EncodeableType_RemoveAllUserTypes_ForEach(const uintptr_t key, uintptr_t value, uintptr_t user_data)
{
    SOPC_UNUSED_ARG(value);
    SOPC_SLinkedList* toRemoveList = (SOPC_SLinkedList*) user_data;
    uintptr_t res = SOPC_SLinkedList_Append(toRemoveList, 0, key);
    SOPC_UNUSED_RESULT(res);
}

void SOPC_EncodeableType_RemoveAllUserTypes(void)
{
    SOPC_SLinkedList* toRemoveList = SOPC_SLinkedList_Create(0);
    if (NULL == toRemoveList)
    {
        return;
    }
    SOPC_Dict_ForEach(g_UserEncodeableTypes, SOPC_EncodeableType_RemoveAllUserTypes_ForEach, (uintptr_t) toRemoveList);
    SOPC_SLinkedListIterator toRemoveIt = SOPC_SLinkedList_GetIterator(toRemoveList);
    while (SOPC_SLinkedList_HasNext(&toRemoveIt))
    {
        SOPC_Dict_Remove(g_UserEncodeableTypes, SOPC_SLinkedList_Next(&toRemoveIt));
    }
    SOPC_SLinkedList_Delete(toRemoveList);

    SOPC_ASSERT(SOPC_Dict_Size(g_UserEncodeableTypes) == 0);
    SOPC_Dict_Delete(g_UserEncodeableTypes);
    g_UserEncodeableTypes = NULL;
}

SOPC_ReturnStatus SOPC_EncodeableType_RegisterTypesArray(size_t nsTypesArrayLen, SOPC_EncodeableType** nsTypesArray)
{
    if (0 == nsTypesArrayLen || NULL == nsTypesArray)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_ReturnStatus localStatus = SOPC_STATUS_OK;
    SOPC_EncodeableType* nsType = NULL;
    const uint16_t nsIndex = nsTypesArray[0]->NamespaceIndex;
    for (size_t i = 0; i < nsTypesArrayLen; i++)
    {
        nsType = nsTypesArray[i];
        SOPC_ASSERT(nsIndex == nsType->NamespaceIndex);
        localStatus = SOPC_EncodeableType_AddUserType(nsType);
        // Keep NOK status if already NOK
        status = (SOPC_STATUS_OK == status ? localStatus : status);
    }
    /* Trick to allow to be able to find the NS type array without TypeId known:
     * use 0 to store first type if not already used TypeId, otherwise it is already ok.*/
    nsType = SOPC_EncodeableType_GetUserType(nsIndex, 0);
    if (NULL == nsType)
    {
        localStatus = insertKeyInUserTypes(nsTypesArray[0], nsIndex, 0);
    }

    return status;
}

SOPC_ReturnStatus SOPC_EncodeableType_UnRegisterTypesArray(size_t nsTypesArrayLen, SOPC_EncodeableType** nsTypesArray)
{
    if (0 == nsTypesArrayLen || NULL == nsTypesArray)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_ReturnStatus localStatus = SOPC_STATUS_OK;
    SOPC_EncodeableType* nsType = NULL;
    const uint16_t nsIndex = nsTypesArray[0]->NamespaceIndex;
    for (size_t i = 0; i < nsTypesArrayLen; i++)
    {
        nsType = nsTypesArray[i];
        SOPC_ASSERT(nsIndex == nsType->NamespaceIndex);
        localStatus = SOPC_EncodeableType_RemoveUserType(nsType);
        // Keep NOK status if already NOK
        status = (SOPC_STATUS_OK == status ? localStatus : status);
    }
    /* Because of the trick in RegisterTypesArray, we have to create a key for a possibly virtual typeId 0 */
    SOPC_EncodeableType_UserTypeKey key = {0};
    key.nsIndex = nsIndex;
    key.typeId = 0;

    nsType = (SOPC_EncodeableType*) SOPC_Dict_GetKey(g_UserEncodeableTypes, (const uintptr_t) &key, NULL);
    if (NULL != nsType)
    {
        SOPC_Dict_Remove(g_UserEncodeableTypes, (const uintptr_t) &key);
    }
    return status;
}

SOPC_EncodeableType* SOPC_EncodeableType_GetUserType(uint16_t nsIndex, uint32_t typeId)
{
    SOPC_EncodeableType_UserTypeValue* pValue = NULL;
    SOPC_EncodeableType* result = NULL;
    bool found = false;
    if (g_UserEncodeableTypes != NULL)
    {
        // search in user defined encodeable types
        SOPC_EncodeableType_UserTypeKey key = {.nsIndex = nsIndex, .typeId = typeId};
        pValue = (SOPC_EncodeableType_UserTypeValue*) SOPC_Dict_Get(g_UserEncodeableTypes, (uintptr_t) &key, &found);
        if (found && pValue != NULL)
        {
            result = pValue->encoder;
            SOPC_ASSERT(result != NULL);
        }
    }
    return result;
}

SOPC_EncodeableType* SOPC_EncodeableType_GetEncodeableType(uint16_t nsIndex, uint32_t typeId)
{
    SOPC_EncodeableType* current = NULL;
    SOPC_EncodeableType* result = NULL;
    uint32_t idx = 0;
    if (OPCUA_NAMESPACE_INDEX == nsIndex)
    {
        current = sopc_KnownEncodeableTypes[idx];
        while (current != NULL && NULL == result)
        {
            if (typeId == current->TypeId || typeId == current->BinaryEncodingTypeId)
            {
                result = current;
            }
            if (NULL == result && idx < UINT32_MAX)
            {
                idx++;
                current = sopc_KnownEncodeableTypes[idx];
            }
            else
            {
                current = NULL;
            }
        }
    }
    if (result == NULL)
    {
        result = SOPC_EncodeableType_GetUserType(nsIndex, typeId);
    }
    return result;
}

const char* SOPC_EncodeableType_GetName(SOPC_EncodeableType* encType)
{
    const char* result = NULL;
    if (NULL == encType)
    {
        result = nullType;
    }
    else if (NULL == encType->TypeName)
    {
        result = noNameType;
    }
    else
    {
        result = encType->TypeName;
    }
    return result;
}

SOPC_EncodeableType_StructureType SOPC_EncodeableType_GetStructureType(SOPC_EncodeableType* encType)
{
    SOPC_ASSERT(encType != NULL && "EncType parameter is NULL");
    return encType->StructType;
}

static inline bool checkEncodeableTypeDescIsValid(const SOPC_EncodeableType_FieldDescriptor* desc)
{
    if (desc->isBuiltIn || desc->isSameNs)
    {
        return true;
    }
    else
    {
        if (OPCUA_NAMESPACE_INDEX == desc->nsIndex && desc->typeIndex < SOPC_TypeInternalIndex_SIZE)
        {
            return true;
        }
        else
        {
            /* We are not able to check for the max size here */
            return (NULL != SOPC_EncodeableType_GetUserType(desc->nsIndex, 0));
        }
    }
    return false;
}

static SOPC_EncodeableType* getKnownEncodeableType(SOPC_EncodeableType* encType,
                                                   const SOPC_EncodeableType_FieldDescriptor* desc)
{
    SOPC_EncodeableType* encTypeZeroId = NULL;
    if (desc->isSameNs)
    {
        SOPC_ASSERT(desc->typeIndex < SOPC_TypeInternalIndex_SIZE &&
                    "Field descriptor type index cannot be greater than SOPC_TypeInternalIndex_SIZE");
        return encType->namespaceTypesArray[desc->typeIndex];
    }
    else
    {
        if (OPCUA_NAMESPACE_INDEX == desc->nsIndex && desc->typeIndex < SOPC_TypeInternalIndex_SIZE)
        {
            /* Well known encodeable type array */
            return sopc_KnownEncodeableTypes[desc->typeIndex];
        }
        else
        {
            /* Use a trick to retrieve the encodeable type array:
             * TypeId 0 is always added to access the array of the namespace */
            encTypeZeroId = SOPC_EncodeableType_GetUserType(desc->nsIndex, 0);
            SOPC_ASSERT(NULL != encTypeZeroId);
            return encTypeZeroId->namespaceTypesArray[desc->typeIndex];
        }
    }
}

static size_t getAllocationSize(SOPC_EncodeableType* encType, const SOPC_EncodeableType_FieldDescriptor* desc)
{
    if (desc->isBuiltIn)
    {
        return SOPC_BuiltInType_HandlingTable[desc->typeIndex].size;
    }
    return getKnownEncodeableType(encType, desc)->AllocationSize;
}

static SOPC_EncodeableObject_PfnInitialize* getPfnInitialize(SOPC_EncodeableType* encType,
                                                             const SOPC_EncodeableType_FieldDescriptor* desc)
{
    if (desc->isBuiltIn)
    {
        return SOPC_BuiltInType_HandlingTable[desc->typeIndex].initialize;
    }
    return getKnownEncodeableType(encType, desc)->Initialize;
}

static SOPC_EncodeableObject_PfnClear* getPfnClear(SOPC_EncodeableType* encType,
                                                   const SOPC_EncodeableType_FieldDescriptor* desc)
{
    if (desc->isBuiltIn)
    {
        return SOPC_BuiltInType_HandlingTable[desc->typeIndex].clear;
    }
    return getKnownEncodeableType(encType, desc)->Clear;
}

static SOPC_ReturnStatus SOPC_EncodeableType_PfnEncode(const void* value,
                                                       SOPC_Buffer* msgBuffer,
                                                       uint32_t nestedStructLevel)
{
    // note: the first field of an object instance for an encodeable type is always a reference to its encodeable type
    return SOPC_EncodeableObject_Encode(*(SOPC_EncodeableType* const*) value, value, msgBuffer, nestedStructLevel);
}

static SOPC_EncodeableObject_PfnEncode* getPfnEncode(const SOPC_EncodeableType_FieldDescriptor* desc)
{
    if (desc->isBuiltIn)
    {
        return SOPC_BuiltInType_EncodingTable[desc->typeIndex].encode;
    }
    else
    {
        return &SOPC_EncodeableType_PfnEncode;
    }
}

static SOPC_ReturnStatus SOPC_EncodeableType_PfnDecode(void* value, SOPC_Buffer* msgBuffer, uint32_t nestedStructLevel)
{
    // note: the first field of an object instance for an encodeable type is always a reference to its encodeable type
    return SOPC_EncodeableObject_Decode(*(SOPC_EncodeableType**) value, value, msgBuffer, nestedStructLevel);
}

static SOPC_EncodeableObject_PfnDecode* getPfnDecode(const SOPC_EncodeableType_FieldDescriptor* desc)
{
    if (desc->isBuiltIn)
    {
        return SOPC_BuiltInType_EncodingTable[desc->typeIndex].decode;
    }
    else
    {
        return &SOPC_EncodeableType_PfnDecode;
    }
}

static SOPC_ReturnStatus SOPC_EncodeableType_PfnCopyArray(void* destValue, const void* srcValue)
{
    // When copying array, values in an array were only allocated before this call
    // We need at least the encodeableType field to be initialized for generic copy function to work
    SOPC_ReturnStatus status =
        SOPC_EncodeableObject_InternalInitialize(*(SOPC_EncodeableType* const*) srcValue, destValue);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_EncodeableObject_Copy(*(SOPC_EncodeableType* const*) srcValue, destValue, srcValue);
    }
    return status;
}

static SOPC_ReturnStatus SOPC_EncodeableType_PfnCopy(void* destValue, const void* srcValue)
{
    return SOPC_EncodeableObject_Copy(*(SOPC_EncodeableType* const*) srcValue, destValue, srcValue);
}

static SOPC_EncodeableObject_PfnCopy* getPfnCopy(const SOPC_EncodeableType_FieldDescriptor* desc, bool isArray)
{
    if (desc->isBuiltIn)
    {
        return SOPC_BuiltInType_HandlingTable[desc->typeIndex].copy;
    }
    else if (isArray)
    {
        return &SOPC_EncodeableType_PfnCopyArray;
    }
    else
    {
        return &SOPC_EncodeableType_PfnCopy;
    }
}

static SOPC_ReturnStatus SOPC_EncodeableType_PfnCompare(const void* leftValue, const void* rightValue, int32_t* comp)
{
    return SOPC_EncodeableObject_Compare(*(SOPC_EncodeableType* const*) leftValue, leftValue, rightValue, comp);
}

static SOPC_EncodeableObject_PfnComp* getPfnCompare(const SOPC_EncodeableType_FieldDescriptor* desc)
{
    if (desc->isBuiltIn)
    {
        return SOPC_BuiltInType_HandlingTable[desc->typeIndex].compare;
    }
    else
    {
        return &SOPC_EncodeableType_PfnCompare;
    }
}

static void** retrieveAddressPtr(void* pValue, const SOPC_EncodeableType_FieldDescriptor* desc)
{
    /* Avoid "warning: cast increases required alignment of target type [-Wcast-align]"
     * There is no issue in this case since desc->offset has been computed using 'offsetof' operator and
     * actually contains a void* address (address of the allocated array).
     * Therefore casting this address into a (void**) is valid and cannot lead to lose information on the
     * address due to alignment normalization (e.g.: when casting a char* to a int*, char* might not be aligned on
     * multiple of 4 bytes and then its address can be normalized to comply to this alignment after the cast
     * operation | see EXP36-C SEI CERT rule).*/
    SOPC_GCC_DIAGNOSTIC_PUSH
    SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_ALIGN
    return (void**) ((char*) pValue + desc->offset);
    SOPC_GCC_DIAGNOSTIC_RESTORE
}

static const void* const* retrieveConstAddressPtr(const void* pValue, const SOPC_EncodeableType_FieldDescriptor* desc)
{
    /* See retrieveAddressPtr comment */
    SOPC_GCC_DIAGNOSTIC_PUSH
    SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_ALIGN
    return (const void* const*) ((const char*) pValue + desc->offset);
    SOPC_GCC_DIAGNOSTIC_RESTORE
}

/**
 * \brief Initialize ONLY the selected union field
 */
static SOPC_ReturnStatus EncodeableObject_InternalInitializeUnionField(SOPC_EncodeableType* type,
                                                                       void* pValue,
                                                                       const int32_t indexSwitchField)
{
    // Check the index of selected field
    if (indexSwitchField >= type->NoOfFields || indexSwitchField <= 0)
    {
        return SOPC_STATUS_ENCODING_ERROR;
    }
    const SOPC_EncodeableType_FieldDescriptor* desc = &type->Fields[indexSwitchField];
    bool validDesc = checkEncodeableTypeDescIsValid(desc);
    if (!validDesc)
    {
        return SOPC_STATUS_NOT_SUPPORTED;
    }
    void* pField = (char*) pValue + desc->offset;
    SOPC_EncodeableObject_PfnInitialize* initFunction = getPfnInitialize(type, desc);
    initFunction(pField);
    return SOPC_STATUS_OK;
}

/**
 * \brief Retrieve the size (max) of the union structure and initialize it to 0
 */
static SOPC_ReturnStatus SOPC_EncodeableObject_InternalInitializeUnion(SOPC_EncodeableType* type, void* pValue)
{
    const SOPC_EncodeableType_FieldDescriptor* descSwitchField = &type->Fields[0];
    bool validDesc = checkEncodeableTypeDescIsValid(descSwitchField);
    if (!validDesc)
    {
        return SOPC_STATUS_NOT_SUPPORTED;
    }

    void* pSwitchField = (char*) pValue + descSwitchField->offset;
    size_t unionStructSize = type->AllocationSize - sizeof(SOPC_EncodeableType*);
    memset(pSwitchField, 0, unionStructSize);
    return SOPC_STATUS_OK;
}

static SOPC_ReturnStatus SOPC_EncodeableObject_InternalInitialize(SOPC_EncodeableType* type, void* pValue)
{
    SOPC_ASSERT(type != NULL);
    SOPC_ASSERT(pValue != NULL);

    // The first field of all non-builtin OPC UA type instances is its encodeable type
    *((SOPC_EncodeableType**) pValue) = type;

    /* Initialize Union */
    if (SOPC_STRUCT_TYPE_UNION == type->StructType)
    {
        // Initialize the entire union structure to 0 (=> SwitchField = 0, Value = 0)
        return SOPC_EncodeableObject_InternalInitializeUnion(type, pValue);
    }

    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    for (int32_t i = 0; i < type->NoOfFields && SOPC_STATUS_OK == status; ++i)
    {
        const SOPC_EncodeableType_FieldDescriptor* desc = &type->Fields[i];
        bool validDesc = checkEncodeableTypeDescIsValid(desc);
        if (!validDesc)
        {
            status = SOPC_STATUS_NOT_SUPPORTED;
        }
        else
        {
            void* pField = (char*) pValue + desc->offset;
            void** pFieldPointer = retrieveAddressPtr(pValue, desc);
            SOPC_EncodeableObject_PfnInitialize* initFunction = NULL;

            if (desc->isArrayLength)
            {
                int32_t* pLength = NULL;
                const SOPC_EncodeableType_FieldDescriptor* arrayDesc = NULL;
                void** pArray = NULL;

                SOPC_ASSERT(desc->isBuiltIn);
                SOPC_ASSERT(desc->typeIndex == (uint32_t) SOPC_Int32_Id);

                // Increment to obtain the array content field
                ++i;
                SOPC_ASSERT(i < type->NoOfFields);
                arrayDesc = &type->Fields[i];
                pArray = retrieveAddressPtr(pValue, arrayDesc);
                // initFunction is not used since we initialize to empty/unallocated array
                // initFunction = getPfnInitialize(type, arrayDesc);

                // Initialize array fields to 0 (or NULL), array is not allocated by init (unknown length)
                if (desc->isOptional) // Optional Array
                {
                    *pFieldPointer = NULL;
                }
                else
                {
                    pLength = pField;
                    *pLength = 0;
                }
                *pArray = NULL;
            }
            else if (desc->isOptional) // Optional field: pointer to the optional value
            {
                *pFieldPointer = NULL;
            }
            else
            {
                initFunction = getPfnInitialize(type, desc);
                initFunction(pField);
            }
        }
    }
    return status;
}

void SOPC_EncodeableObject_Initialize(SOPC_EncodeableType* type, void* pValue)
{
    SOPC_ReturnStatus status = SOPC_EncodeableObject_InternalInitialize(type, pValue);
    SOPC_UNUSED_RESULT(status);
    return;
}

static void SOPC_EncodeableObject_ClearUnion(SOPC_EncodeableType* type, void* pValue)
{
    // 1. Decode the switch field
    const SOPC_EncodeableType_FieldDescriptor* descSwitchField = &type->Fields[0];
    bool validDesc = checkEncodeableTypeDescIsValid(descSwitchField);
    if (!validDesc)
    {
        return;
    }
    void* pSwitchField = (char*) pValue + descSwitchField->offset;
    const int32_t indexSwitchField = (const int32_t) * (uint32_t*) pSwitchField;
    // Check the index of selected field
    if (indexSwitchField >= type->NoOfFields || indexSwitchField <= 0)
    {
        return;
    }

    // 2. Clear the selected union field
    const SOPC_EncodeableType_FieldDescriptor* desc = &type->Fields[indexSwitchField];
    validDesc = checkEncodeableTypeDescIsValid(desc);
    if (!validDesc)
    {
        return;
    }
    void* pField = (char*) pValue + desc->offset;
    SOPC_EncodeableObject_PfnClear* clearFunction = getPfnClear(type, desc);
    clearFunction(pField);

    // 3. Clear the switch field
    SOPC_UInt32_ClearAux(pSwitchField);
}

void SOPC_EncodeableObject_Clear(SOPC_EncodeableType* type, void* pValue)
{
    SOPC_ASSERT(type != NULL);
    if (NULL == pValue)
    {
        return;
    }

    /* Clear Union*/
    if (SOPC_STRUCT_TYPE_UNION == type->StructType)
    {
        SOPC_EncodeableObject_ClearUnion(type, pValue);
        return;
    }

    for (int32_t i = 0; i < type->NoOfFields; ++i)
    {
        const SOPC_EncodeableType_FieldDescriptor* desc = &type->Fields[i];
        bool validDesc = checkEncodeableTypeDescIsValid(desc);
        if (!validDesc)
        {
            return;
        }
        void* pField = (char*) pValue + desc->offset;
        void** pFieldPointer = retrieveAddressPtr(pValue, desc);

        SOPC_EncodeableObject_PfnClear* clearFunction = NULL;

        if (desc->isArrayLength)
        {
            int32_t* pLength = NULL;
            const SOPC_EncodeableType_FieldDescriptor* arrayDesc = NULL;
            void** pArray = NULL;
            size_t size = 0;

            SOPC_ASSERT(desc->isBuiltIn);
            SOPC_ASSERT(desc->typeIndex == (uint32_t) SOPC_Int32_Id);
            if (desc->isOptional) // Optional Array
            {
                pLength = *pFieldPointer;
            }
            else
            {
                pLength = pField;
            }

            // Increment to obtain the array content field
            ++i;
            SOPC_ASSERT(i < type->NoOfFields);
            // Avoid case where Optional Array is not available (: pLength == NULL)
            if (pLength != NULL)
            {
                arrayDesc = &type->Fields[i];
                pArray = retrieveAddressPtr(pValue, arrayDesc);
                size = getAllocationSize(type, arrayDesc);
                clearFunction = getPfnClear(type, arrayDesc);
                SOPC_Clear_Array(pLength, pArray, size, clearFunction);
                if (desc->isOptional) // Optional Array
                {
                    SOPC_Free(*pFieldPointer);
                    *pFieldPointer = NULL;
                }
            }
        }
        else if (desc->isOptional) // Optional field
        {
            if (*pFieldPointer != NULL) // Optional field available
            {
                // Clear value
                clearFunction = getPfnClear(type, desc);
                clearFunction(*pFieldPointer);
                // Free pointer
                SOPC_Free(*pFieldPointer);
                *pFieldPointer = NULL;
            }
        }
        else
        {
            clearFunction = getPfnClear(type, desc);
            clearFunction(pField);
        }
    }
}

SOPC_ReturnStatus SOPC_EncodeableObject_Create(SOPC_EncodeableType* encTyp, void** encObject)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (encTyp != NULL && encTyp->Initialize != NULL && encTyp->AllocationSize > 0 && encObject != NULL &&
        NULL == *encObject)
    {
        *encObject = SOPC_Malloc(encTyp->AllocationSize);
        if (*encObject != NULL)
        {
            status = SOPC_EncodeableObject_InternalInitialize(encTyp, *encObject);
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
    }
    return status;
}

SOPC_ReturnStatus SOPC_EncodeableObject_Delete(SOPC_EncodeableType* encTyp, void** encObject)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (encTyp != NULL && encTyp->Clear != NULL && encObject != NULL && *encObject != NULL &&
        encTyp == *(SOPC_EncodeableType**) *encObject)
    {
        SOPC_EncodeableObject_Clear(encTyp, *encObject);
        SOPC_Free(*encObject);
        *encObject = NULL;
        status = SOPC_STATUS_OK;
    }
    return status;
}

/**
 * \brief Encode the mask for option fields ( \p encodingMask ).
 *        If a field is optional and allocated (!= NULL),
 *        the field is available and must be encoded.
 *        Refer to OPC UA part 6, 5.2.7.
 */
static SOPC_ReturnStatus EncodeableObject_EncodeMaskOptFields(SOPC_EncodeableType* type,
                                                              const void* pValue,
                                                              SOPC_Buffer* buf,
                                                              uint32_t nestedStructLevel,
                                                              uint32_t* encodingMask)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    uint8_t indexOptional = 0;
    // 1st iteration to rebuild the mask for optional fields
    for (int32_t i = 0; SOPC_STATUS_OK == status && i < type->NoOfFields; ++i)
    {
        const SOPC_EncodeableType_FieldDescriptor* desc = &type->Fields[i];
        bool validDesc = checkEncodeableTypeDescIsValid(desc);
        if (!validDesc)
        {
            status = SOPC_STATUS_NOT_SUPPORTED;
        }
        else
        {
            const void* const* pFieldPointer = retrieveConstAddressPtr(pValue, desc);
            if (desc->isOptional) // Is an optional field
            {
                if (*pFieldPointer != NULL) // Is an optional field available
                {
                    *encodingMask = *encodingMask + (0x1u << indexOptional);
                }
                if (desc->isArrayLength)
                {
                    // Jump over the Array field (Arraylength field also marks for Array field).
                    i++;
                }
                indexOptional++;
            }
        }
    }
    // Encode the mask of optional fields
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_UInt32_Write(encodingMask, buf, nestedStructLevel);
    }
    return status;
}

static SOPC_ReturnStatus EncodeableObject_EncodeUnion(SOPC_EncodeableType* type,
                                                      const void* pValue,
                                                      SOPC_Buffer* buf,
                                                      uint32_t nestedStructLevel)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    const void* pSwitchField = NULL;
    const uint32_t prevLength = buf->length;
    const uint32_t prevPosition = buf->position;

    // 1. Encode the switch field
    const SOPC_EncodeableType_FieldDescriptor* desc = &type->Fields[0];
    bool validDesc = checkEncodeableTypeDescIsValid(desc);
    if (!validDesc)
    {
        status = SOPC_STATUS_NOT_SUPPORTED;
    }
    else
    {
        pSwitchField = (const char*) pValue + desc->offset;
        SOPC_UInt32_Write((const uint32_t*) pSwitchField, buf, nestedStructLevel);
    }

    // 2. Encode the selected union field
    if (SOPC_STATUS_OK == status)
    {
        const int32_t indexSwitchField = (const int32_t) * (const uint32_t*) pSwitchField;
        // Check and Set the index of selected field
        if (indexSwitchField >= type->NoOfFields)
        {
            status = SOPC_STATUS_ENCODING_ERROR;
        }
        if (SOPC_STATUS_OK == status && indexSwitchField > 0)
        {
            desc = &type->Fields[indexSwitchField];
            validDesc = checkEncodeableTypeDescIsValid(desc);
            if (!validDesc)
            {
                status = SOPC_STATUS_NOT_SUPPORTED;
            }
            else
            {
                const void* pField = (const char*) pValue + desc->offset;
                SOPC_EncodeableObject_PfnEncode* encodeFunction = getPfnEncode(desc);
                status = encodeFunction(pField, buf, nestedStructLevel);
            }
        }
    }
    if (SOPC_STATUS_OK != status)
    {
        // Restore initial position and length buffer states (ignore possibly written data)
        buf->length = prevLength;
        buf->position = prevPosition;
    }
    return status;
}

SOPC_ReturnStatus SOPC_EncodeableObject_Encode(SOPC_EncodeableType* type,
                                               const void* pValue,
                                               SOPC_Buffer* buf,
                                               uint32_t nestedStructLevel)
{
    if (NULL == type || NULL == pValue || NULL == buf || *((SOPC_EncodeableType* const*) pValue) != type)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    else if (nestedStructLevel >= SOPC_Internal_Common_GetEncodingConstants()->max_nested_struct)
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    nestedStructLevel++;

    /* Encode Union */
    if (SOPC_STRUCT_TYPE_UNION == type->StructType)
    {
        return EncodeableObject_EncodeUnion(type, pValue, buf, nestedStructLevel);
    }

    const uint32_t prevLength = buf->length;
    const uint32_t prevPosition = buf->position;
    uint32_t encodingMask = 0; // For OptFields struct
    uint8_t indexOptional = 0; // For OptFields struct
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    /* Encode Classic/OptFields */
    if (SOPC_STATUS_OK == status && SOPC_STRUCT_TYPE_OPT_FIELDS == type->StructType)
    {
        // If OptFields struct: first find available optional fields and encode mask of optional fields.
        status = EncodeableObject_EncodeMaskOptFields(type, pValue, buf, nestedStructLevel, &encodingMask);
    }
    for (int32_t i = 0; SOPC_STATUS_OK == status && i < type->NoOfFields; ++i)
    {
        const SOPC_EncodeableType_FieldDescriptor* desc = &type->Fields[i];
        bool validDesc = checkEncodeableTypeDescIsValid(desc);
        if (!validDesc)
        {
            status = SOPC_STATUS_NOT_SUPPORTED;
        }
        else // Encode Field
        {
            const void* pField = (const char*) pValue + desc->offset;
            const void* const* pFieldPointer = retrieveConstAddressPtr(pValue, desc);
            bool isOptAndNotAvailable = (desc->isOptional && ((0x1u << indexOptional) & ~encodingMask));
            indexOptional = desc->isOptional ? (uint8_t)(indexOptional + 1u) : indexOptional;

            if (!desc->isToEncode || isOptAndNotAvailable)
            {
                // Skip this field
                if (desc->isArrayLength)
                {
                    // Skip also array field
                    i++;
                }
            }
            else if (desc->isArrayLength)
            {
                const int32_t* pLength = NULL;
                const SOPC_EncodeableType_FieldDescriptor* arrayDesc = NULL;
                const void* const* pArray = NULL;
                size_t size = 0;
                SOPC_EncodeableObject_PfnEncode* encodeFunction = NULL;

                SOPC_ASSERT(desc->isBuiltIn);
                SOPC_ASSERT(desc->typeIndex == (uint32_t) SOPC_Int32_Id);
                if (desc->isOptional) // Optional Array => ArrayLength is a pointer
                {
                    pLength = *pFieldPointer;
                }
                else
                {
                    pLength = pField;
                }

                // Increment to obtain the array content field
                ++i;
                SOPC_ASSERT(i < type->NoOfFields);
                // pLength == NULL guarded by isOptAndNotAvailable condition
                SOPC_ASSERT(pLength != NULL);
                arrayDesc = &type->Fields[i];
                pArray = retrieveConstAddressPtr(pValue, arrayDesc);
                size = getAllocationSize(type, arrayDesc);
                encodeFunction = getPfnEncode(arrayDesc);
                status = SOPC_Write_Array(buf, pLength, pArray, size, encodeFunction, nestedStructLevel);
            }
            else if (desc->isOptional) // Optional field available
            {
                SOPC_EncodeableObject_PfnEncode* encodeFunction = getPfnEncode(desc);
                status = encodeFunction(*pFieldPointer, buf, nestedStructLevel);
            }
            else // Classic field
            {
                SOPC_EncodeableObject_PfnEncode* encodeFunction = getPfnEncode(desc);
                status = encodeFunction(pField, buf, nestedStructLevel);
            }
        }
    }
    if (SOPC_STATUS_OK != status)
    {
        // Restore initial position and length buffer states (ignore possibly written data)
        buf->length = prevLength;
        buf->position = prevPosition;
    }
    return status;
}

/**
 * \brief Retrieve the number of optional fields on a encodeable \p type.
 */
static uint32_t EncodeableObject_RetrieveNumberOfOptionalField(SOPC_EncodeableType* type)
{
    uint32_t nbOfOptField = 0;
    bool validDesc = true;
    for (int32_t i = 0; validDesc && i < type->NoOfFields; ++i)
    {
        const SOPC_EncodeableType_FieldDescriptor* desc = &type->Fields[i];
        validDesc = checkEncodeableTypeDescIsValid(desc);
        if (desc->isOptional) // Optional field
        {
            nbOfOptField++;
        }
        if (desc->isArrayLength)
        {
            // Skip the array field (already marked with the array length field).
            i++;
        }
    }
    return validDesc ? nbOfOptField : 0;
}

/**
 * \brief Allocate optional fields available (= defined by encoding mask).
 *        Refer to OPC UA part 6, 5.2.7.
 */
static SOPC_ReturnStatus EncodeableObject_AllocateOptFields(SOPC_EncodeableType* type,
                                                            void* pValue,
                                                            SOPC_Buffer* buf,
                                                            uint32_t nestedStructLevel)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    uint32_t encodingMask = 0;
    uint8_t indexOptional = 0;

    // First, decode the encoding mask.
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_UInt32_Read(&encodingMask, buf, nestedStructLevel);
        if (SOPC_STATUS_OK == status)
        {
            // Check if unassigned bits of the encoding mask are not 0.
            uint32_t nbOfOptField = EncodeableObject_RetrieveNumberOfOptionalField(type);
            uint32_t encodingMaskMax = ((0x1u << nbOfOptField) - 1); // 2^nbOfOptField - 1
            if (encodingMask > encodingMaskMax)
            {
                SOPC_Logger_TraceError(
                    SOPC_LOG_MODULE_COMMON,
                    "Decode Optional Fields structure: Unassigned bits of the EncodingMask are not 0 (Type: %s). "
                    "Expected max EncodingMask value: 0x%08" PRIX32 ", effective EncodingMask value: 0x%08" PRIX32 "",
                    type->TypeName, encodingMaskMax, encodingMask);
                status = SOPC_STATUS_ENCODING_ERROR;
            }
        }
    }

    // Allocation of available optional fields.
    for (int32_t i = 0; SOPC_STATUS_OK == status && i < type->NoOfFields; ++i)
    {
        const SOPC_EncodeableType_FieldDescriptor* desc = &type->Fields[i];
        SOPC_EncodeableObject_PfnInitialize* initFunction = NULL;
        bool validDesc = checkEncodeableTypeDescIsValid(desc);
        if (!validDesc)
        {
            status = SOPC_STATUS_NOT_SUPPORTED;
        }
        else
        {
            if (desc->isOptional) // Optional field
            {
                if ((0x1u << indexOptional) & encodingMask) // Optional field available
                {
                    // Allocate and Initialize the field pointer
                    size_t size = getAllocationSize(type, desc);
                    void** pFieldPointer = retrieveAddressPtr(pValue, desc);
                    *pFieldPointer = SOPC_Calloc(1, size);
                    initFunction = getPfnInitialize(type, desc);
                    initFunction(*pFieldPointer);
                }
                if (desc->isArrayLength)
                {
                    // Skip the array field (already marked with the array length field).
                    i++;
                }
                indexOptional++;
            }
        }
    }
    return status;
}

static SOPC_ReturnStatus EncodeableObject_DecodeUnion(SOPC_EncodeableType* type,
                                                      void* pValue,
                                                      SOPC_Buffer* buf,
                                                      uint32_t nestedStructLevel)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    void* pSwitchField = NULL;

    // 1. Decode the switch field
    const SOPC_EncodeableType_FieldDescriptor* desc = &type->Fields[0];
    bool validDesc = checkEncodeableTypeDescIsValid(desc);
    if (!validDesc)
    {
        status = SOPC_STATUS_NOT_SUPPORTED;
    }
    else
    {
        pSwitchField = (char*) pValue + desc->offset;
        SOPC_UInt32_Read((uint32_t*) pSwitchField, buf, nestedStructLevel);
    }

    // 2. Decode the selected union field
    if (SOPC_STATUS_OK == status)
    {
        const int32_t indexSwitchField = (const int32_t) * (uint32_t*) pSwitchField;
        // Initialize the selected union field before decoding it.
        status = EncodeableObject_InternalInitializeUnionField(type, pValue, indexSwitchField);
        if (SOPC_STATUS_OK == status)
        {
            // Decode selected union field
            desc = &type->Fields[indexSwitchField];
            validDesc = checkEncodeableTypeDescIsValid(desc);
            if (!validDesc)
            {
                status = SOPC_STATUS_NOT_SUPPORTED;
            }
            else
            {
                void* pField = (char*) pValue + desc->offset;
                SOPC_EncodeableObject_PfnDecode* decodeFunction = getPfnDecode(desc);
                status = decodeFunction(pField, buf, nestedStructLevel);
            }
        }
    }
    if (status != SOPC_STATUS_OK)
    {
        SOPC_EncodeableObject_Clear(type, pValue);
    }
    return status;
}

SOPC_ReturnStatus SOPC_EncodeableObject_Decode(SOPC_EncodeableType* type,
                                               void* pValue,
                                               SOPC_Buffer* buf,
                                               uint32_t nestedStructLevel)
{
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    if (NULL == type || NULL == pValue || NULL == buf)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    else if (nestedStructLevel >= SOPC_Internal_Common_GetEncodingConstants()->max_nested_struct)
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    else
    {
        status = SOPC_EncodeableObject_InternalInitialize(type, pValue);
    }
    nestedStructLevel++; // increment for future calls

    /* Decode Union */
    if (SOPC_STATUS_OK == status && SOPC_STRUCT_TYPE_UNION == type->StructType)
    {
        return EncodeableObject_DecodeUnion(type, pValue, buf, nestedStructLevel);
    }

    /* Decode Classic/OptFields */
    if (SOPC_STATUS_OK == status && SOPC_STRUCT_TYPE_OPT_FIELDS == type->StructType)
    {
        // If OptFields struct: first allocate the available fields,
        // then fill them in during the rest of the decoding process.
        status = EncodeableObject_AllocateOptFields(type, pValue, buf, nestedStructLevel);
    }
    for (int32_t i = 0; SOPC_STATUS_OK == status && i < type->NoOfFields; ++i)
    {
        const SOPC_EncodeableType_FieldDescriptor* desc = &type->Fields[i];
        bool validDesc = checkEncodeableTypeDescIsValid(desc);
        if (!validDesc)
        {
            status = SOPC_STATUS_NOT_SUPPORTED;
        }
        else
        {
            void* pField = (char*) pValue + desc->offset;
            void** pFieldPointer = retrieveAddressPtr(pValue, desc);
            SOPC_EncodeableObject_PfnDecode* decodeFunction = NULL;

            // Not to encode or optional field not available
            if (!desc->isToEncode || (desc->isOptional && *pFieldPointer == NULL))
            {
                // Skip this field
            }
            else if (desc->isArrayLength)
            {
                int32_t* pLength = NULL;
                const SOPC_EncodeableType_FieldDescriptor* arrayDesc = NULL;
                void** pArray = NULL;
                size_t size = 0;
                SOPC_EncodeableObject_PfnInitialize* initFunction = NULL;
                SOPC_EncodeableObject_PfnClear* clearFunction = NULL;

                SOPC_ASSERT(desc->isBuiltIn);
                SOPC_ASSERT(desc->typeIndex == (uint32_t) SOPC_Int32_Id);
                if (desc->isOptional) // Optional Array => ArrayLength is a pointer
                {
                    pLength = *pFieldPointer;
                }
                else
                {
                    pLength = pField;
                }

                // Increment to obtain the array content field
                ++i;
                SOPC_ASSERT(i < type->NoOfFields);
                // pLength == NULL guarded by if(desc->isOptional && *pFieldPointer == NULL) + if(desc->isOptional)
                SOPC_ASSERT(pLength != NULL);
                arrayDesc = &type->Fields[i];
                pArray = retrieveAddressPtr(pValue, arrayDesc);
                size = getAllocationSize(type, arrayDesc);
                decodeFunction = getPfnDecode(arrayDesc);
                initFunction = getPfnInitialize(type, arrayDesc);
                clearFunction = getPfnClear(type, arrayDesc);
                status = SOPC_Read_Array(buf, pLength, pArray, size, decodeFunction, initFunction, clearFunction,
                                         nestedStructLevel);
            }
            else if (desc->isOptional) // Optional field
            {
                decodeFunction = getPfnDecode(desc);
                status = decodeFunction(*pFieldPointer, buf, nestedStructLevel);
            }
            else
            {
                decodeFunction = getPfnDecode(desc);
                status = decodeFunction(pField, buf, nestedStructLevel);
            }
        }
    }
    if (status != SOPC_STATUS_OK)
    {
        SOPC_EncodeableObject_Clear(type, pValue);
    }
    return status;
}

static SOPC_ReturnStatus EncodeableObject_CopyUnion(SOPC_EncodeableType* type, void* destValue, const void* srcValue)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    const void* pSrcSwitchField = NULL;

    // Copy switchField
    const SOPC_EncodeableType_FieldDescriptor* desc = &type->Fields[0];
    bool validDesc = checkEncodeableTypeDescIsValid(desc);
    if (!validDesc)
    {
        status = SOPC_STATUS_NOT_SUPPORTED;
    }
    else
    {
        pSrcSwitchField = (const char*) srcValue + desc->offset;
        void* pDestSwitchField = (char*) destValue + desc->offset;
        SOPC_EncodeableObject_PfnCopy* copyFunction = getPfnCopy(desc, false);
        status = copyFunction(pDestSwitchField, pSrcSwitchField);
    }

    // Copy selected union field
    if (SOPC_STATUS_OK == status)
    {
        const int32_t indexSwitchField = (const int32_t) * (const uint32_t*) pSrcSwitchField;
        // Check and Set the index of selected field
        if (indexSwitchField >= type->NoOfFields)
        {
            status = SOPC_STATUS_ENCODING_ERROR;
        }
        if (SOPC_STATUS_OK == status && indexSwitchField > 0)
        {
            desc = &type->Fields[indexSwitchField];
            validDesc = checkEncodeableTypeDescIsValid(desc);
            if (!validDesc)
            {
                status = SOPC_STATUS_NOT_SUPPORTED;
            }
            else
            {
                const void* pSrcField = (const char*) srcValue + desc->offset;
                void* pDestField = (char*) destValue + desc->offset;
                SOPC_EncodeableObject_PfnCopy* copyFunction = getPfnCopy(desc, false);
                status = copyFunction(pDestField, pSrcField);
            }
        }
    }
    if (status != SOPC_STATUS_OK)
    {
        SOPC_EncodeableObject_Clear(type, destValue);
    }
    return status;
}

SOPC_ReturnStatus SOPC_EncodeableObject_Copy(SOPC_EncodeableType* type, void* destValue, const void* srcValue)
{
    if (NULL == type || NULL == destValue || NULL == srcValue || *((SOPC_EncodeableType* const*) srcValue) != type ||
        *((SOPC_EncodeableType* const*) destValue) != type)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    /* Copy Union */
    if (SOPC_STRUCT_TYPE_UNION == type->StructType)
    {
        return EncodeableObject_CopyUnion(type, destValue, srcValue);
    }

    /* Copy Classic/OptFields */
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    for (int32_t i = 0; SOPC_STATUS_OK == status && i < type->NoOfFields; ++i)
    {
        const SOPC_EncodeableType_FieldDescriptor* desc = &type->Fields[i];
        bool validDesc = checkEncodeableTypeDescIsValid(desc);
        if (!validDesc)
        {
            status = SOPC_STATUS_NOT_SUPPORTED;
        }
        else
        {
            const void* pSrcField = (const char*) srcValue + desc->offset;
            const void* const* pSrcFieldPointer = retrieveConstAddressPtr(srcValue, desc);
            void* pDestField = (char*) destValue + desc->offset;
            void** pDestFieldPointer = retrieveAddressPtr(destValue, desc);

            if (desc->isArrayLength)
            {
                const SOPC_EncodeableType_FieldDescriptor* arrayDesc = NULL;
                void** pArrayDest = NULL;
                const void* const* pArraySource = NULL;
                size_t size = 0;
                SOPC_EncodeableObject_PfnCopy* copyFunction = NULL;

                SOPC_ASSERT(desc->isBuiltIn);
                SOPC_ASSERT(desc->typeIndex == (uint32_t) SOPC_Int32_Id);

                // Increment to obtain the array content field
                ++i;
                SOPC_ASSERT(i < type->NoOfFields);

                const int32_t* pSrcLength = NULL;
                int32_t* pDestLength = NULL;
                if (desc->isOptional) // Optional Array
                {
                    pSrcLength = *pSrcFieldPointer;
                    pDestLength = *pDestFieldPointer;
                }
                else
                {
                    pSrcLength = pSrcField;
                    pDestLength = pDestField;
                }
                // Avoid case where Optional Array is not available (: pSrcLength == NULL)
                if (pSrcLength != NULL && *pSrcLength > 0)
                {
                    arrayDesc = &type->Fields[i];
                    pArrayDest = retrieveAddressPtr(destValue, arrayDesc);
                    pArraySource = retrieveConstAddressPtr(srcValue, arrayDesc);
                    size = getAllocationSize(type, arrayDesc);
                    copyFunction = getPfnCopy(arrayDesc, true);

                    // Allocate array of source length with source elements size
                    // Note: overwrite previous pointer if dest was not cleared
                    *pArrayDest = SOPC_Calloc((size_t) *pSrcLength, size);
                    if (NULL != *pArrayDest)
                    {
                        status = SOPC_Copy_Array(*pSrcLength, *pArrayDest, *pArraySource, size, copyFunction);
                    }
                    else
                    {
                        status = SOPC_STATUS_OUT_OF_MEMORY;
                    }
                } // else NULL array with 0 length (or null length when optional array not available)
                // Avoid case where Optional Array is not available (: pSrcLength == NULL)
                if (SOPC_STATUS_OK == status && pSrcLength != NULL)
                {
                    // Copy dest length
                    if (desc->isOptional) // Optional array length
                    {
                        size_t sizeArrayLength = getAllocationSize(type, desc);
                        *pDestFieldPointer = SOPC_Calloc(1, sizeArrayLength);
                        void* destFieldPointer = *pDestFieldPointer;
                        const void* srcFieldPointer = *pSrcFieldPointer;
                        copyFunction = getPfnCopy(desc, false);
                        status = copyFunction(destFieldPointer, srcFieldPointer);
                    }
                    else
                    {
                        *pDestLength = *pSrcLength;
                    }
                }
            }
            else if (desc->isOptional) // optional field available
            {
                if (*pSrcFieldPointer != NULL)
                {
                    size_t size = getAllocationSize(type, desc);
                    *pDestFieldPointer = SOPC_Calloc(1, size);
                    void* destFieldPointer = *pDestFieldPointer;
                    const void* srcFieldPointer = *pSrcFieldPointer;
                    SOPC_EncodeableObject_PfnInitialize* initFunction = getPfnInitialize(type, desc);
                    initFunction(destFieldPointer);
                    SOPC_EncodeableObject_PfnCopy* copyFunction = getPfnCopy(desc, false);
                    status = copyFunction(destFieldPointer, srcFieldPointer);
                }
            }
            else
            {
                SOPC_EncodeableObject_PfnCopy* copyFunction = getPfnCopy(desc, false);
                status = copyFunction(pDestField, pSrcField);
            }
        }
    }

    if (status != SOPC_STATUS_OK)
    {
        SOPC_EncodeableObject_Clear(type, destValue);
    }

    return status;
}

SOPC_ReturnStatus SOPC_EncodeableObject_Move(void* destObj, void* srcObj)
{
    if (NULL == destObj || NULL == srcObj || destObj == srcObj ||
        *(SOPC_EncodeableType**) destObj != *(SOPC_EncodeableType**) srcObj)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_EncodeableType* encType = *(SOPC_EncodeableType**) srcObj;

    memcpy(destObj, srcObj, encType->AllocationSize);
    return SOPC_EncodeableObject_InternalInitialize(encType, srcObj);
}

static SOPC_ReturnStatus EncodeableObject_CompareUnion(SOPC_EncodeableType* type,
                                                       const void* leftValue,
                                                       const void* rightValue,
                                                       int32_t* comp)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    int32_t resultComp = 0;
    const void* pLeftSwitchField = NULL;

    // Compare switchField
    const SOPC_EncodeableType_FieldDescriptor* desc = &type->Fields[0];
    bool validDesc = checkEncodeableTypeDescIsValid(desc);
    if (!validDesc)
    {
        status = SOPC_STATUS_NOT_SUPPORTED;
    }
    else
    {
        pLeftSwitchField = (const char*) leftValue + desc->offset;
        const void* pRightSwitchField = (const char*) rightValue + desc->offset;
        SOPC_EncodeableObject_PfnComp* compFunction = getPfnCompare(desc);
        status = compFunction(pLeftSwitchField, pRightSwitchField, &resultComp);
    }

    // Compare selected union field
    if (SOPC_STATUS_OK == status && 0 == resultComp)
    {
        const int32_t indexSwitchField = (const int32_t) * (const uint32_t*) pLeftSwitchField;
        // Check and Set the index of selected field
        if (indexSwitchField >= type->NoOfFields)
        {
            status = SOPC_STATUS_ENCODING_ERROR;
        }
        if (SOPC_STATUS_OK == status && indexSwitchField > 0)
        {
            desc = &type->Fields[indexSwitchField];
            validDesc = checkEncodeableTypeDescIsValid(desc);
            if (!validDesc)
            {
                status = SOPC_STATUS_NOT_SUPPORTED;
            }
            else
            {
                const void* pLeftField = (const char*) leftValue + desc->offset;
                const void* pRightField = (const char*) rightValue + desc->offset;
                SOPC_EncodeableObject_PfnComp* compFunction = getPfnCompare(desc);
                status = compFunction(pLeftField, pRightField, &resultComp);
            }
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        *comp = resultComp;
    }
    return status;
}

SOPC_ReturnStatus SOPC_EncodeableObject_Compare(SOPC_EncodeableType* type,
                                                const void* leftValue,
                                                const void* rightValue,
                                                int32_t* comp)
{
    if (NULL == type || NULL == leftValue || NULL == rightValue ||
        *((SOPC_EncodeableType* const*) rightValue) != type || *((SOPC_EncodeableType* const*) leftValue) != type ||
        NULL == comp)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    /* Compare Union*/
    if (SOPC_STRUCT_TYPE_UNION == type->StructType)
    {
        return EncodeableObject_CompareUnion(type, leftValue, rightValue, comp);
    }

    /* Compare Classic/OptFields */
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    int32_t resultComp = 0;
    for (int32_t i = 0; SOPC_STATUS_OK == status && 0 == resultComp && i < type->NoOfFields; ++i)
    {
        const SOPC_EncodeableType_FieldDescriptor* desc = &type->Fields[i];
        bool validDesc = checkEncodeableTypeDescIsValid(desc);
        if (!validDesc)
        {
            status = SOPC_STATUS_NOT_SUPPORTED;
        }
        else
        {
            const void* pRightField = (const char*) rightValue + desc->offset;
            const void* const* pRightFieldPointer = retrieveConstAddressPtr(rightValue, desc);
            const void* pLeftField = (const char*) leftValue + desc->offset;
            const void* const* pLeftFieldPointer = retrieveConstAddressPtr(leftValue, desc);
            if (desc->isArrayLength)
            {
                const SOPC_EncodeableType_FieldDescriptor* arrayDesc = NULL;
                const void* const* pArrayLeft = NULL;
                const void* const* pArrayRight = NULL;
                size_t size = 0;
                SOPC_EncodeableObject_PfnComp* compFunction = NULL;

                SOPC_ASSERT(desc->isBuiltIn);
                SOPC_ASSERT(desc->typeIndex == (uint32_t) SOPC_Int32_Id);

                const int32_t* pLeftLength = NULL;
                const int32_t* pRightLength = NULL;
                if (desc->isOptional) // Optional Array
                {
                    pLeftLength = *pLeftFieldPointer;
                    pRightLength = *pRightFieldPointer;
                }
                else
                {
                    pLeftLength = pLeftField;
                    pRightLength = pRightField;
                }

                // Increment to obtain the array content field
                ++i;
                SOPC_ASSERT(i < type->NoOfFields);
                // Both optional arrays are not available
                if (NULL == pLeftLength && NULL == pRightLength)
                {
                    // -> comp OK
                    resultComp = 0;
                }
                // One of optional arrays is not available
                else if (NULL == pLeftLength || NULL == pRightLength)
                {
                    resultComp = pLeftLength != NULL ? 1 : -1;
                }
                /* Both optional array are available or classic array cases */
                // Compare length
                else if (*pLeftLength < *pRightLength)
                {
                    resultComp = -1;
                }
                else if (*pLeftLength > *pRightLength)
                {
                    resultComp = 1;
                }
                // Compare array
                else if (*pLeftLength > 0)
                {
                    arrayDesc = &type->Fields[i];
                    pArrayLeft = retrieveConstAddressPtr(leftValue, arrayDesc);
                    pArrayRight = retrieveConstAddressPtr(rightValue, arrayDesc);
                    size = getAllocationSize(type, arrayDesc);
                    compFunction = getPfnCompare(arrayDesc);
                    status = SOPC_Comp_Array(*pLeftLength, *pArrayLeft, *pArrayRight, size, compFunction, &resultComp);
                } // else both have length == 0
            }
            else if (desc->isOptional) // Optional field
            {
                // Both field are available
                if (*pLeftFieldPointer != NULL && *pRightFieldPointer != NULL)
                {
                    const void* leftFieldPointer = *pLeftFieldPointer;
                    const void* rightFieldPointer = *pRightFieldPointer;
                    SOPC_EncodeableObject_PfnComp* compFunction = getPfnCompare(desc);
                    status = compFunction(leftFieldPointer, rightFieldPointer, &resultComp);
                }
                // Left available, right not available
                else if (*pLeftFieldPointer != NULL && *pRightFieldPointer == NULL)
                {
                    resultComp = 1;
                }
                // Left not available, right available
                else if (*pLeftFieldPointer == NULL && *pRightFieldPointer != NULL)
                {
                    resultComp = -1;
                }
                // Both are not available
                else
                {
                    resultComp = 0;
                }
            }
            else
            {
                SOPC_EncodeableObject_PfnComp* compFunction = getPfnCompare(desc);
                status = compFunction(pLeftField, pRightField, &resultComp);
            }
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        *comp = resultComp;
    }
    return status;
}
