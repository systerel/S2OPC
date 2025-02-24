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

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "constants_statuscodes_bs.h"
#include "opcua_statuscodes.h"
#include "sopc_assert.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "util_b2c.h"
#include "util_variant.h"

#include "sopc_missing_c99.h"

SOPC_Variant* util_variant__new_Variant_from_NodeId(SOPC_NodeId* pnid, bool deepCopy)
{
    SOPC_Variant* pvar = SOPC_Variant_Create();

    if (NULL == pvar)
    {
        return NULL;
    }

    pvar->BuiltInTypeId = SOPC_NodeId_Id;
    pvar->ArrayType = SOPC_VariantArrayType_SingleValue;
    if (deepCopy)
    {
        pvar->Value.NodeId = SOPC_Calloc(sizeof(*pvar->Value.NodeId), 1);
        SOPC_ReturnStatus status = (NULL != pvar->Value.NodeId ? SOPC_STATUS_OK : SOPC_STATUS_OUT_OF_MEMORY);
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_NodeId_Copy(pvar->Value.NodeId, pnid);
            if (SOPC_STATUS_OK != status)
            {
                SOPC_Free(pvar->Value.NodeId);
            }
        }
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Free(pvar);
            pvar = NULL;
        }
    }
    else
    {
        pvar->Value.NodeId = pnid;
        pvar->DoNotClear = true; // It is shallow copy of provided node
    }
    return pvar;
}

SOPC_Variant* util_variant__new_Variant_from_NodeClass(OpcUa_NodeClass ncl)
{
    SOPC_Variant* pvar = SOPC_Malloc(sizeof(SOPC_Variant));

    if (NULL == pvar)
        return NULL;

    SOPC_Variant_Initialize(pvar);
    pvar->BuiltInTypeId = SOPC_Int32_Id;
    pvar->ArrayType = SOPC_VariantArrayType_SingleValue;
    pvar->Value.Int32 = (int32_t) ncl;

    return pvar;
}

SOPC_Variant* util_variant__new_Variant_from_QualifiedName(SOPC_QualifiedName* qn, bool deepCopy)
{
    SOPC_Variant* pvar = SOPC_Variant_Create();
    if (NULL == pvar)
    {
        return NULL;
    }

    pvar->BuiltInTypeId = SOPC_QualifiedName_Id;
    pvar->ArrayType = SOPC_VariantArrayType_SingleValue;
    if (deepCopy)
    {
        pvar->Value.Qname = SOPC_Calloc(sizeof(*pvar->Value.Qname), 1);
        SOPC_ReturnStatus status = (NULL != pvar->Value.Qname ? SOPC_STATUS_OK : SOPC_STATUS_OUT_OF_MEMORY);
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_QualifiedName_Copy(pvar->Value.Qname, qn);
            if (SOPC_STATUS_OK != status)
            {
                SOPC_Free(pvar->Value.Qname);
            }
        }
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Free(pvar);
            pvar = NULL;
        }
    }
    else
    {
        pvar->Value.Qname = qn;
        pvar->DoNotClear = true; // It is shallow copy of provided node
    }

    return pvar;
}

SOPC_Variant* util_variant__new_Variant_from_LocalizedText(SOPC_LocalizedText* lt, bool deepCopy)
{
    SOPC_Variant* pvar = SOPC_Variant_Create();

    if (NULL == pvar)
    {
        return NULL;
    }

    pvar->BuiltInTypeId = SOPC_LocalizedText_Id;
    pvar->ArrayType = SOPC_VariantArrayType_SingleValue;
    if (deepCopy)
    {
        pvar->Value.LocalizedText = SOPC_Calloc(sizeof(*pvar->Value.LocalizedText), 1);
        SOPC_ReturnStatus status = (NULL != pvar->Value.LocalizedText ? SOPC_STATUS_OK : SOPC_STATUS_OUT_OF_MEMORY);
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_LocalizedText_Copy(pvar->Value.LocalizedText, lt);
            if (SOPC_STATUS_OK != status)
            {
                SOPC_Free(pvar->Value.LocalizedText);
            }
        }
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Free(pvar);
            pvar = NULL;
        }
    }
    else
    {
        pvar->Value.LocalizedText = lt;
        pvar->DoNotClear = true; // It is shallow copy of provided node
    }

    return pvar;
}

bool util_variant__copy_PreferredLocalizedText_from_LocalizedText_Variant(SOPC_Variant* dest,
                                                                          const SOPC_Variant* src,
                                                                          char** preferredLocales)
{
    SOPC_ASSERT(NULL != src);
    SOPC_ASSERT(NULL != dest);
    SOPC_ASSERT(NULL != preferredLocales);

    SOPC_ASSERT(SOPC_LocalizedText_Id == src->BuiltInTypeId);

    SOPC_Variant* srcCopy = SOPC_Variant_Create();
    if (NULL == srcCopy)
    {
        return false;
    }
    SOPC_ReturnStatus status = SOPC_Variant_ShallowCopy(srcCopy, src);

    if (SOPC_STATUS_OK == status)
    {
        bool success = false;
        srcCopy =
            util_variant__set_PreferredLocalizedText_from_LocalizedText_Variant(&srcCopy, preferredLocales, &success);
        status = (success ? SOPC_STATUS_OK : SOPC_STATUS_NOK);
    }
    if (SOPC_STATUS_OK == status)
    {
        *dest = *srcCopy;
    }
    SOPC_Free(srcCopy);
    return (SOPC_STATUS_OK == status);
}

SOPC_Variant* util_variant__set_PreferredLocalizedText_from_LocalizedText_Variant(SOPC_Variant** v,
                                                                                  char** preferredLocales,
                                                                                  bool* success)
{
    SOPC_ASSERT(NULL != v);
    SOPC_Variant* value = *v;
    SOPC_ASSERT(SOPC_LocalizedText_Id == value->BuiltInTypeId);

    bool res = false;
    // If the caller wants to know the final result.
    if (NULL != success)
    {
        *success = res;
    }

    // Create a variant that will be a deep copy of *v
    SOPC_Variant* result = SOPC_Variant_Create();
    if (NULL == result)
    {
        return value;
    }

    if (SOPC_VariantArrayType_SingleValue == value->ArrayType)
    {
        SOPC_LocalizedText* newLt = SOPC_Malloc(sizeof(*newLt));
        SOPC_LocalizedText_Initialize(newLt);
        SOPC_ReturnStatus status =
            SOPC_LocalizedText_GetPreferredLocale(newLt, preferredLocales, value->Value.LocalizedText);
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Free(newLt);
            SOPC_Variant_Clear(result);
            SOPC_Free(result);
            result = value;
        }
        else
        {
            result->ArrayType = SOPC_VariantArrayType_SingleValue;
            result->BuiltInTypeId = SOPC_LocalizedText_Id;
            result->Value.LocalizedText = newLt;
            res = true;
            SOPC_Variant_Clear(value);
            SOPC_Free(value);
            *v = NULL;
        }
    }
    else if (SOPC_VariantArrayType_Array == value->ArrayType || SOPC_VariantArrayType_Matrix == value->ArrayType)
    {
        SOPC_ReturnStatus status = SOPC_Variant_Copy(result, value);

        if (SOPC_STATUS_OK == status)
        {
            int32_t length = SOPC_Variant_GetArrayOrMatrixLength(value);
            const SOPC_LocalizedText* dest = NULL;
            const SOPC_LocalizedText* src = NULL;
            for (int32_t i = 0; SOPC_STATUS_OK == status && i < length; i++)
            {
                dest = SOPC_Variant_Get_ArrayValue(result, SOPC_LocalizedText_Id, i);
                src = SOPC_Variant_Get_ArrayValue(value, SOPC_LocalizedText_Id, i);
                SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
                SOPC_LocalizedText_Clear(
                    (SOPC_LocalizedText*) dest); // Clear the initial copy and retrieved preferred locale
                status = SOPC_LocalizedText_GetPreferredLocale((SOPC_LocalizedText*) dest, preferredLocales, src);
                SOPC_GCC_DIAGNOSTIC_RESTORE
            }
        }

        if (SOPC_STATUS_OK == status)
        {
            // Preferred localized text array created, remove source one
            res = true;
            SOPC_Variant_Clear(value);
            SOPC_Free(value);
            *v = NULL;
        }
        else
        {
            // Do not compute preferred localized texts and keep default ones
            SOPC_Variant_Clear(result);
            SOPC_Free(result);
            result = value;
        }
    }
    else
    {
        SOPC_ASSERT(false);
    }

    // Update final result status
    if (NULL != success)
    {
        *success = res;
    }

    return result;
}

SOPC_Variant* util_variant__new_Variant_from_Indet(void)
{
    SOPC_Variant* pvar = SOPC_Malloc(sizeof(SOPC_Variant));

    if (NULL == pvar)
        return NULL;

    SOPC_Variant_Initialize(pvar);
    pvar->BuiltInTypeId = SOPC_Null_Id;
    pvar->ArrayType = SOPC_VariantArrayType_SingleValue;

    return pvar;
}

SOPC_Variant* util_variant__new_Variant_from_Variant(const SOPC_Variant* pvara, bool deepCopy)
{
    SOPC_ReturnStatus retStatus = SOPC_STATUS_OK;
    SOPC_Variant* pvar;
    if (NULL == pvara)
        return util_variant__new_Variant_from_Indet();

    pvar = SOPC_Variant_Create();

    if (NULL == pvar)
        return NULL;

    if (deepCopy)
    {
        retStatus = SOPC_Variant_Copy(pvar, pvara);
    }
    else
    {
        retStatus = SOPC_Variant_ShallowCopy(pvar, pvara);
    }

    if (retStatus != SOPC_STATUS_OK)
    {
        if (pvar != NULL)
        {
            SOPC_Free(pvar);
        }
        pvar = NULL;
    }

    return pvar;
}

SOPC_Variant* util_variant__new_Variant_from_Bool(bool b)
{
    SOPC_Variant* pvar = SOPC_Malloc(sizeof(SOPC_Variant));

    if (NULL != pvar)
    {
        SOPC_Variant_Initialize(pvar);
        pvar->BuiltInTypeId = SOPC_Boolean_Id;
        pvar->ArrayType = SOPC_VariantArrayType_SingleValue;
        pvar->Value.Byte = b;
    }

    return pvar;
}

SOPC_Variant* util_variant__new_Variant_from_Byte(uint8_t i)
{
    SOPC_Variant* pvar = SOPC_Malloc(sizeof(SOPC_Variant));

    if (NULL != pvar)
    {
        SOPC_Variant_Initialize(pvar);
        pvar->BuiltInTypeId = SOPC_Byte_Id;
        pvar->ArrayType = SOPC_VariantArrayType_SingleValue;
        pvar->Value.Byte = i;
    }

    return pvar;
}

SOPC_Variant* util_variant__new_Variant_from_uint32(uint32_t i)
{
    SOPC_Variant* pvar = SOPC_Malloc(sizeof(SOPC_Variant));

    if (NULL == pvar)
        return NULL;

    SOPC_Variant_Initialize(pvar);
    pvar->BuiltInTypeId = SOPC_UInt32_Id;
    pvar->ArrayType = SOPC_VariantArrayType_SingleValue;
    pvar->Value.Uint32 = i;

    return pvar;
}

SOPC_Variant* util_variant__new_Variant_from_int64(int64_t i)
{
    SOPC_Variant* pvar = SOPC_Malloc(sizeof(SOPC_Variant));

    if (NULL == pvar)
        return NULL;

    SOPC_Variant_Initialize(pvar);
    pvar->BuiltInTypeId = SOPC_Int64_Id;
    pvar->ArrayType = SOPC_VariantArrayType_SingleValue;
    pvar->Value.Int64 = i;

    return pvar;
}

SOPC_Variant* util_variant__new_Variant_from_int32(int32_t i)
{
    SOPC_Variant* pvar = SOPC_Malloc(sizeof(SOPC_Variant));

    if (NULL == pvar)
        return NULL;

    SOPC_Variant_Initialize(pvar);
    pvar->BuiltInTypeId = SOPC_Int32_Id;
    pvar->ArrayType = SOPC_VariantArrayType_SingleValue;
    pvar->Value.Int32 = i;

    return pvar;
}

SOPC_Variant* util_variant__new_Variant_from_double(double f)
{
    SOPC_Variant* pvar = SOPC_Malloc(sizeof(SOPC_Variant));

    if (NULL == pvar)
        return NULL;

    SOPC_Variant_Initialize(pvar);
    pvar->BuiltInTypeId = SOPC_Double_Id;
    pvar->ArrayType = SOPC_VariantArrayType_SingleValue;
    pvar->Value.Doublev = f;

    return pvar;
}

SOPC_Variant* util_variant__new_Variant_from_ByteString(SOPC_ByteString* bs, bool deepCopy)
{
    SOPC_ASSERT(NULL != bs);
    SOPC_Variant* pvar = SOPC_Variant_Create();
    if (NULL == pvar)
    {
        return NULL;
    }

    pvar->BuiltInTypeId = SOPC_ByteString_Id;
    pvar->ArrayType = SOPC_VariantArrayType_SingleValue;
    if (deepCopy)
    {
        SOPC_ByteString_Initialize(&pvar->Value.Bstring);
        SOPC_ReturnStatus status = SOPC_ByteString_Copy(&pvar->Value.Bstring, bs);
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Free(pvar);
            pvar = NULL;
        }
    }
    else
    {
        pvar->Value.Bstring = *bs;
        pvar->DoNotClear = true; // It is shallow copy of provided node
    }

    return pvar;
}

SOPC_Variant* util_variant__new_Variant_from_ExtensionObject(SOPC_ExtensionObject* extObj, bool deepCopy)
{
    SOPC_Variant* pvar = SOPC_Variant_Create();

    if (NULL == pvar)
        return NULL;

    pvar->BuiltInTypeId = SOPC_ExtensionObject_Id;
    pvar->ArrayType = SOPC_VariantArrayType_SingleValue;
    if (deepCopy)
    {
        pvar->Value.ExtObject = SOPC_Calloc(sizeof(*pvar->Value.ExtObject), 1);
        SOPC_ReturnStatus status = (NULL != pvar->Value.ExtObject ? SOPC_STATUS_OK : SOPC_STATUS_OUT_OF_MEMORY);
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_ExtensionObject_Copy(pvar->Value.ExtObject, extObj);
            if (SOPC_STATUS_OK != status)
            {
                SOPC_Free(pvar->Value.ExtObject);
            }
        }
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Free(pvar);
            pvar = NULL;
        }
    }
    else
    {
        pvar->Value.ExtObject = extObj;
        pvar->DoNotClear = true; // It is shallow copy of provided node
    }

    return pvar;
}

SOPC_ReturnStatus util_variant__copy_and_apply_locales_and_index_range(SOPC_Variant* destVal,
                                                                       const SOPC_Variant* source,
                                                                       char** preferredLocalesIds,
                                                                       const SOPC_NumericRange* indexRange)
{
    SOPC_ASSERT(NULL != destVal);
    SOPC_ASSERT(NULL != source);
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_Variant ltEventVar;
    SOPC_Variant_Initialize(&ltEventVar);
    SOPC_StatusCode valueStatus = SOPC_GoodGenericStatus;

    /* Set the preferred locale in case of LT value */
    if (SOPC_LocalizedText_Id == source->BuiltInTypeId && NULL != preferredLocalesIds)
    {
        bool res = util_variant__copy_PreferredLocalizedText_from_LocalizedText_Variant(&ltEventVar, source,
                                                                                        preferredLocalesIds);
        if (!res)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
            valueStatus = OpcUa_BadOutOfMemory;
        }
        source = &ltEventVar;
    }

    /* IndexRange filtering */
    constants_statuscodes_bs__t_StatusCode_i readSC = constants_statuscodes_bs__c_StatusCode_indet;
    if (SOPC_STATUS_OK == status)
    {
        if (NULL != indexRange)
        {
            readSC = util_read_value_indexed_helper(destVal, source, indexRange);
            if (constants_statuscodes_bs__e_sc_ok != readSC)
            {
                /* note: index range syntax error shall have occurred on createMI but the specification state any error
                 * might be returned as value:
                 * "Servers should return all other errors as CreateMonitoredItems results
                 *  but all possible errors are allowed to be returned in the Publish response."
                 */
                util_status_code__B_to_C(readSC, &valueStatus);
                status = SOPC_STATUS_NOK;
            }
        }
        else
        {
            status = SOPC_Variant_Copy(destVal, source);
            if (SOPC_STATUS_OK != status)
            {
                SOPC_ASSERT(SOPC_STATUS_OUT_OF_MEMORY == status);
                valueStatus = OpcUa_BadOutOfMemory;
            }
        }
    }

    SOPC_Variant_Clear(&ltEventVar);
    if (SOPC_STATUS_OK != status)
    {
        SOPC_ASSERT(!SOPC_IsGoodStatus(valueStatus));
        SOPC_Variant_Clear(destVal);
        destVal->BuiltInTypeId = SOPC_StatusCode_Id;
        destVal->Value.Status = valueStatus;
    }
    return status;
}
