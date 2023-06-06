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
    uint32_t typeId;
} SOPC_EncodeableType_UserTypeKey;

typedef struct
{
    SOPC_EncodeableType* encoder;
} SOPC_EncodeableType_UserTypeValue;

static uint64_t typeId_hash(const uintptr_t data)
{
    return ((const SOPC_EncodeableType_UserTypeKey*) data)->typeId;
}

static bool typeId_equal(const uintptr_t a, const uintptr_t b)
{
    return ((const SOPC_EncodeableType_UserTypeKey*) a)->typeId == ((const SOPC_EncodeableType_UserTypeKey*) b)->typeId;
}

static void type_free(uintptr_t data)
{
    SOPC_Free((void*) data);
}

static SOPC_ReturnStatus insertKeyInUserTypes(SOPC_EncodeableType* pEncoder, const uint32_t typeId)
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

    prevEncoder = SOPC_EncodeableType_GetEncodeableType(pEncoder->TypeId);
    if (prevEncoder != NULL)
    {
        // This TypeId is already used.
        return SOPC_STATUS_NOT_SUPPORTED;
    }

    if (g_UserEncodeableTypes == NULL)
    {
        // Create dictionnary
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
        result = insertKeyInUserTypes(pEncoder, pEncoder->TypeId);
    }
    if (result == SOPC_STATUS_OK)
    {
        result = insertKeyInUserTypes(pEncoder, pEncoder->BinaryEncodingTypeId);
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

    key.typeId = encoder->TypeId;

    prevEncoder = (SOPC_EncodeableType*) SOPC_Dict_GetKey(g_UserEncodeableTypes, (const uintptr_t) &key, NULL);
    if (prevEncoder == NULL)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_Dict_Remove(g_UserEncodeableTypes, (const uintptr_t) &key);
    key.typeId = encoder->BinaryEncodingTypeId;
    SOPC_Dict_Remove(g_UserEncodeableTypes, (const uintptr_t) &key);
    // Delete the dictionnay if empty
    if (SOPC_Dict_Size(g_UserEncodeableTypes) == 0)
    {
        SOPC_Dict_Delete(g_UserEncodeableTypes);
        g_UserEncodeableTypes = NULL;
    }
    return SOPC_STATUS_OK;
}

SOPC_EncodeableType* SOPC_EncodeableType_GetUserType(uint32_t typeId)
{
    SOPC_EncodeableType_UserTypeValue* pValue = NULL;
    SOPC_EncodeableType* result = NULL;
    bool found = false;
    if (g_UserEncodeableTypes != NULL)
    {
        // search in user defined encodeable types
        SOPC_EncodeableType_UserTypeKey key = {.typeId = typeId};
        pValue = (SOPC_EncodeableType_UserTypeValue*) SOPC_Dict_Get(g_UserEncodeableTypes, (uintptr_t) &key, &found);
        if (found && pValue != NULL)
        {
            result = pValue->encoder;
            SOPC_ASSERT(result != NULL);
        }
    }
    return result;
}

SOPC_EncodeableType* SOPC_EncodeableType_GetEncodeableType(uint32_t typeId)
{
    SOPC_EncodeableType* current = NULL;
    SOPC_EncodeableType* result = NULL;
    uint32_t idx = 0;
    current = SOPC_KnownEncodeableTypes[idx];
    while (current != NULL && NULL == result)
    {
        if (typeId == current->TypeId || typeId == current->BinaryEncodingTypeId)
        {
            result = current;
        }
        if (NULL == result && idx < UINT32_MAX)
        {
            idx++;
            current = SOPC_KnownEncodeableTypes[idx];
        }
        else
        {
            current = NULL;
        }
    }
    if (result == NULL)
    {
        result = SOPC_EncodeableType_GetUserType(typeId);
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

static SOPC_EncodeableType* getKnownEncodeableType(const SOPC_EncodeableType_FieldDescriptor* desc)
{
    const uint32_t typeIndex = desc->typeIndex;
    SOPC_ASSERT(typeIndex < SOPC_TypeInternalIndex_SIZE &&
                "Field descriptor type index cannot be greater than SOPC_TypeInternalIndex_SIZE");
    return SOPC_KnownEncodeableTypes[typeIndex];
}

static size_t getAllocationSize(const SOPC_EncodeableType_FieldDescriptor* desc)
{
    if (desc->isBuiltIn)
    {
        return SOPC_BuiltInType_HandlingTable[desc->typeIndex].size;
    }
    return getKnownEncodeableType(desc)->AllocationSize;
}

static SOPC_EncodeableObject_PfnInitialize* getPfnInitialize(const SOPC_EncodeableType_FieldDescriptor* desc)
{
    if (desc->isBuiltIn)
    {
        return SOPC_BuiltInType_HandlingTable[desc->typeIndex].initialize;
    }
    return getKnownEncodeableType(desc)->Initialize;
}

static SOPC_EncodeableObject_PfnClear* getPfnClear(const SOPC_EncodeableType_FieldDescriptor* desc)
{
    if (desc->isBuiltIn)
    {
        return SOPC_BuiltInType_HandlingTable[desc->typeIndex].clear;
    }
    return getKnownEncodeableType(desc)->Clear;
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
    SOPC_EncodeableObject_Initialize(*(SOPC_EncodeableType* const*) srcValue, destValue);
    return SOPC_EncodeableObject_Copy(*(SOPC_EncodeableType* const*) srcValue, destValue, srcValue);
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
        return SOPC_EncodeableType_PfnCompare;
    }
}

static void** retrieveArrayAddressPtr(void* pValue, const SOPC_EncodeableType_FieldDescriptor* arrayDesc)
{
    /* Avoid "warning: cast increases required alignment of target type [-Wcast-align]"
     * There is no issue in this case since arrayDesc->offset has been computed using 'offsetof' operator and
     * actually contains a void* address (address of the allocated array).
     * Therefore casting this address into a (void**) is valid and cannot lead to lose information on the
     * address due to alignment normalization (e.g.: when casting a char* to a int*, char* might not be aligned on
     * multiple of 4 bytes and then its address can be normalized to comply to this alignment after the cast
     * operation | see EXP36-C SEI CERT rule).*/
    SOPC_GCC_DIAGNOSTIC_PUSH
    SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_ALIGN
    return (void**) ((char*) pValue + arrayDesc->offset);
    SOPC_GCC_DIAGNOSTIC_RESTORE
}

static const void* const* retrieveConstArrayAddressPtr(const void* pValue,
                                                       const SOPC_EncodeableType_FieldDescriptor* arrayDesc)
{
    /* See retrieveArrayAddressPtr comment */
    SOPC_GCC_DIAGNOSTIC_PUSH
    SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_ALIGN
    return (const void* const*) ((const char*) pValue + arrayDesc->offset);
    SOPC_GCC_DIAGNOSTIC_RESTORE
}

void SOPC_EncodeableObject_Initialize(SOPC_EncodeableType* type, void* pValue)
{
    SOPC_ASSERT(type != NULL);
    SOPC_ASSERT(pValue != NULL);

    // The first field of all non-builtin OPC UA type instances is its encodeable type
    *((SOPC_EncodeableType**) pValue) = type;

    for (int32_t i = 0; i < type->NoOfFields; ++i)
    {
        const SOPC_EncodeableType_FieldDescriptor* desc = &type->Fields[i];
        void* pField = (char*) pValue + desc->offset;
        SOPC_EncodeableObject_PfnInitialize* initFunction = NULL;

        if (desc->isArrayLength)
        {
            int32_t* pLength = NULL;
            const SOPC_EncodeableType_FieldDescriptor* arrayDesc = NULL;
            void** pArray = NULL;

            SOPC_ASSERT(desc->isBuiltIn);
            SOPC_ASSERT(desc->typeIndex == (uint32_t) SOPC_Int32_Id);
            pLength = pField;

            // Increment to obtain the array content field
            ++i;
            SOPC_ASSERT(i < type->NoOfFields);
            arrayDesc = &type->Fields[i];
            pArray = retrieveArrayAddressPtr(pValue, arrayDesc);
            initFunction = getPfnInitialize(arrayDesc);

            // Initialize array fields to 0, array is not allocated by init (unknown length)
            *pLength = 0;
            *pArray = NULL;
        }
        else
        {
            initFunction = getPfnInitialize(desc);
            initFunction(pField);
        }
    }
}

void SOPC_EncodeableObject_Clear(SOPC_EncodeableType* type, void* pValue)
{
    SOPC_ASSERT(type != NULL);
    if (NULL == pValue)
    {
        return;
    }

    for (int32_t i = 0; i < type->NoOfFields; ++i)
    {
        const SOPC_EncodeableType_FieldDescriptor* desc = &type->Fields[i];
        void* pField = (char*) pValue + desc->offset;
        SOPC_EncodeableObject_PfnClear* clearFunction = NULL;

        if (desc->isArrayLength)
        {
            int32_t* pLength = NULL;
            const SOPC_EncodeableType_FieldDescriptor* arrayDesc = NULL;
            void** pArray = NULL;
            size_t size = 0;

            SOPC_ASSERT(desc->isBuiltIn);
            SOPC_ASSERT(desc->typeIndex == (uint32_t) SOPC_Int32_Id);
            pLength = pField;

            // Increment to obtain the array content field
            ++i;
            SOPC_ASSERT(i < type->NoOfFields);
            arrayDesc = &type->Fields[i];
            pArray = retrieveArrayAddressPtr(pValue, arrayDesc);
            size = getAllocationSize(arrayDesc);
            clearFunction = getPfnClear(arrayDesc);

            SOPC_Clear_Array(pLength, pArray, size, clearFunction);
        }
        else
        {
            clearFunction = getPfnClear(desc);
            clearFunction(pField);
        }
    }
}

SOPC_ReturnStatus SOPC_EncodeableObject_Encode(SOPC_EncodeableType* type,
                                               const void* pValue,
                                               SOPC_Buffer* buf,
                                               uint32_t nestedStructLevel)
{
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;

    if (NULL == type || NULL == pValue || NULL == buf || *((SOPC_EncodeableType* const*) pValue) != type)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    else if (nestedStructLevel >= SOPC_Internal_Common_GetEncodingConstants()->max_nested_struct)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    nestedStructLevel++;
    status = SOPC_STATUS_OK;

    for (int32_t i = 0; SOPC_STATUS_OK == status && i < type->NoOfFields; ++i)
    {
        const SOPC_EncodeableType_FieldDescriptor* desc = &type->Fields[i];
        const void* pField = (const char*) pValue + desc->offset;

        if (!desc->isToEncode)
        {
            // Skip this field
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
            pLength = pField;

            // Increment to obtain the array content field
            ++i;
            SOPC_ASSERT(i < type->NoOfFields);
            arrayDesc = &type->Fields[i];
            pArray = retrieveConstArrayAddressPtr(pValue, arrayDesc);
            size = getAllocationSize(arrayDesc);
            encodeFunction = getPfnEncode(arrayDesc);

            status = SOPC_Write_Array(buf, pLength, pArray, size, encodeFunction, nestedStructLevel);
        }
        else
        {
            SOPC_EncodeableObject_PfnEncode* encodeFunction = getPfnEncode(desc);
            status = encodeFunction(pField, buf, nestedStructLevel);
        }
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

    nestedStructLevel++; // increment for future calls
    status = SOPC_STATUS_OK;

    if (SOPC_STATUS_OK == status)
    {
        SOPC_EncodeableObject_Initialize(type, pValue);
    }

    for (int32_t i = 0; SOPC_STATUS_OK == status && i < type->NoOfFields; ++i)
    {
        const SOPC_EncodeableType_FieldDescriptor* desc = &type->Fields[i];
        void* pField = (char*) pValue + desc->offset;
        SOPC_EncodeableObject_PfnDecode* decodeFunction = NULL;

        if (!desc->isToEncode)
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
            pLength = pField;

            // Increment to obtain the array content field
            ++i;
            SOPC_ASSERT(i < type->NoOfFields);
            arrayDesc = &type->Fields[i];
            pArray = retrieveArrayAddressPtr(pValue, arrayDesc);
            size = getAllocationSize(arrayDesc);
            decodeFunction = getPfnDecode(arrayDesc);
            initFunction = getPfnInitialize(arrayDesc);
            clearFunction = getPfnClear(arrayDesc);

            status = SOPC_Read_Array(buf, pLength, pArray, size, decodeFunction, initFunction, clearFunction,
                                     nestedStructLevel);
        }
        else
        {
            decodeFunction = getPfnDecode(desc);
            status = decodeFunction(pField, buf, nestedStructLevel);
        }
    }

    if (status != SOPC_STATUS_OK)
    {
        SOPC_EncodeableObject_Clear(type, pValue);
    }

    return status;
}

SOPC_ReturnStatus SOPC_EncodeableObject_Copy(SOPC_EncodeableType* type, void* destValue, const void* srcValue)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;

    if (NULL == type || NULL == destValue || NULL == srcValue || *((SOPC_EncodeableType* const*) srcValue) != type ||
        *((SOPC_EncodeableType* const*) destValue) != type)
    {
        return status;
    }

    status = SOPC_STATUS_OK;

    for (int32_t i = 0; SOPC_STATUS_OK == status && i < type->NoOfFields; ++i)
    {
        const SOPC_EncodeableType_FieldDescriptor* desc = &type->Fields[i];
        const void* pSrcField = (const char*) srcValue + desc->offset;
        void* pDestField = (char*) destValue + desc->offset;

        if (desc->isArrayLength)
        {
            const int32_t* pSrcLength = pSrcField;
            int32_t* pDestLength = pDestField;

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
            if (*pSrcLength > 0)
            {
                arrayDesc = &type->Fields[i];
                pArrayDest = retrieveArrayAddressPtr(destValue, arrayDesc);
                pArraySource = retrieveConstArrayAddressPtr(srcValue, arrayDesc);
                size = getAllocationSize(arrayDesc);
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
            } // else NULL array with 0 length
            if (SOPC_STATUS_OK == status)
            {
                // Set dest length
                *pDestLength = *pSrcLength;
            }
        }
        else
        {
            SOPC_EncodeableObject_PfnCopy* copyFunction = getPfnCopy(desc, false);
            status = copyFunction(pDestField, pSrcField);
        }
    }

    if (status != SOPC_STATUS_OK)
    {
        SOPC_EncodeableObject_Clear(type, destValue);
    }

    return status;
}

SOPC_ReturnStatus SOPC_EncodeableObject_Compare(SOPC_EncodeableType* type,
                                                const void* leftValue,
                                                const void* rightValue,
                                                int32_t* comp)
{
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    int32_t resultComp = 0;

    if (NULL == type || NULL == leftValue || NULL == rightValue ||
        *((SOPC_EncodeableType* const*) rightValue) != type || *((SOPC_EncodeableType* const*) leftValue) != type ||
        NULL == comp)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    status = SOPC_STATUS_OK;

    for (int32_t i = 0; SOPC_STATUS_OK == status && i < type->NoOfFields; ++i)
    {
        const SOPC_EncodeableType_FieldDescriptor* desc = &type->Fields[i];
        const void* pRightField = (const char*) rightValue + desc->offset;
        const void* pLeftField = (const char*) leftValue + desc->offset;

        if (desc->isArrayLength)
        {
            const int32_t* pLeftLength = pLeftField;
            const int32_t* pRightLength = pRightField;

            const SOPC_EncodeableType_FieldDescriptor* arrayDesc = NULL;
            const void* const* pArrayLeft = NULL;
            const void* const* pArrayRight = NULL;
            size_t size = 0;
            SOPC_EncodeableObject_PfnComp* compFunction = NULL;

            SOPC_ASSERT(desc->isBuiltIn);
            SOPC_ASSERT(desc->typeIndex == (uint32_t) SOPC_Int32_Id);

            // Increment to obtain the array content field
            ++i;
            SOPC_ASSERT(i < type->NoOfFields);
            if (pLeftLength < pRightLength)
            {
                resultComp = -1;
            }
            else if (pLeftLength > pRightLength)
            {
                resultComp = 1;
            }
            else if (*pLeftLength > 0)
            {
                arrayDesc = &type->Fields[i];
                pArrayLeft = retrieveConstArrayAddressPtr(leftValue, arrayDesc);
                pArrayRight = retrieveConstArrayAddressPtr(rightValue, arrayDesc);
                size = getAllocationSize(arrayDesc);
                compFunction = getPfnCompare(arrayDesc);

                status = SOPC_Comp_Array(*pLeftLength, *pArrayLeft, *pArrayRight, size, compFunction, &resultComp);
            } // else both have length == 0
        }
        else
        {
            SOPC_EncodeableObject_PfnComp* compFunction = getPfnCompare(desc);
            status = compFunction(pLeftField, pRightField, &resultComp);
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        *comp = resultComp;
    }

    return status;
}
