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

#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "sopc_mem_alloc.h"
#include "util_variant.h"

#include "sopc_missing_c99.h"

SOPC_Variant* util_variant__new_Variant_from_NodeId(SOPC_NodeId* pnid)
{
    SOPC_Variant* pvar = SOPC_Malloc(sizeof(SOPC_Variant));

    if (NULL == pvar)
        return NULL;

    SOPC_Variant_Initialize(pvar);
    pvar->BuiltInTypeId = SOPC_NodeId_Id;
    pvar->ArrayType = SOPC_VariantArrayType_SingleValue;
    pvar->Value.NodeId = pnid;
    pvar->DoNotClear = true; // It is shallow copy of provided node

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

SOPC_Variant* util_variant__new_Variant_from_QualifiedName(SOPC_QualifiedName* qn)
{
    SOPC_Variant* pvar = SOPC_Malloc(sizeof(SOPC_Variant));

    if (NULL == pvar)
        return NULL;

    SOPC_Variant_Initialize(pvar);
    pvar->BuiltInTypeId = SOPC_QualifiedName_Id;
    pvar->ArrayType = SOPC_VariantArrayType_SingleValue;
    pvar->Value.Qname = qn;
    pvar->DoNotClear = true; // It is shallow copy of provided node

    return pvar;
}

SOPC_Variant* util_variant__new_Variant_from_LocalizedText(SOPC_LocalizedText* lt)
{
    SOPC_Variant* pvar = SOPC_Malloc(sizeof(SOPC_Variant));

    if (NULL == pvar)
        return NULL;

    SOPC_Variant_Initialize(pvar);
    pvar->BuiltInTypeId = SOPC_LocalizedText_Id;
    pvar->ArrayType = SOPC_VariantArrayType_SingleValue;
    pvar->Value.LocalizedText = lt;
    pvar->DoNotClear = true; // It is shallow copy of provided node

    return pvar;
}

SOPC_Variant* util_variant__set_PreferredLocalizedText_from_LocalizedText_Variant(SOPC_Variant** v,
                                                                                  char** preferredLocales)
{
    assert(NULL != v);
    SOPC_Variant* value = *v;

    assert(SOPC_LocalizedText_Id == value->BuiltInTypeId);
    assert(value->DoNotClear); // it was a shallow copy

    SOPC_Variant* result = NULL;

    if (SOPC_VariantArrayType_SingleValue == value->ArrayType)
    {
        SOPC_LocalizedText* newLt = SOPC_Malloc(sizeof(*newLt));
        SOPC_LocalizedText_Initialize(newLt);
        SOPC_ReturnStatus status =
            SOPC_LocalizedText_GetPreferredLocale(newLt, preferredLocales, value->Value.LocalizedText);
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Free(newLt);
        }
        else
        {
            value->Value.LocalizedText = newLt;
            value->DoNotClear = false; // it is not anymore a shallow copy
            result = value;
        }
    }
    else if (SOPC_VariantArrayType_Array == value->ArrayType || SOPC_VariantArrayType_Matrix == value->ArrayType)
    {
        // Create a deep copy that will allocate the same
        result = SOPC_Variant_Create();
        SOPC_ReturnStatus status = SOPC_Variant_Copy(result, value);

        if (SOPC_STATUS_OK == status)
        {
            int32_t length = -1;
            SOPC_LocalizedText* srcLtArray = NULL;
            SOPC_LocalizedText* destLtArray = NULL;
            if (SOPC_VariantArrayType_Matrix == value->ArrayType)
            {
                length = 1;
                // Compute total length of flattened matrix
                for (int32_t i = 0; i < value->Value.Matrix.Dimensions; i++)
                {
                    length *= value->Value.Matrix.ArrayDimensions[i];
                }
                srcLtArray = value->Value.Matrix.Content.LocalizedTextArr;
                destLtArray = result->Value.Matrix.Content.LocalizedTextArr;
            }
            else
            {
                length = value->Value.Array.Length;
                srcLtArray = value->Value.Array.Content.LocalizedTextArr;
                destLtArray = result->Value.Array.Content.LocalizedTextArr;
            }

            for (int32_t i = 0; SOPC_STATUS_OK == status && i < length; i++)
            {
                SOPC_LocalizedText_Clear(&destLtArray[i]); // Clear the initial copy and retrieved preferred locale
                status = SOPC_LocalizedText_GetPreferredLocale(&destLtArray[i], preferredLocales, &srcLtArray[i]);
            }
        }

        if (SOPC_STATUS_OK == status)
        {
            // Preferred localized text array created, remove source one
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
        assert(false);
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

SOPC_Variant* util_variant__new_Variant_from_Variant(SOPC_Variant* pvara)
{
    SOPC_ReturnStatus retStatus = SOPC_STATUS_OK;
    SOPC_Variant* pvar;
    if (NULL == pvara)
        return util_variant__new_Variant_from_Indet();

    pvar = SOPC_Malloc(sizeof(SOPC_Variant));

    if (NULL == pvar)
        return NULL;

    retStatus = SOPC_Variant_ShallowCopy(pvar, pvara);

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

SOPC_Variant* util_variant__new_Variant_from_ByteString(SOPC_ByteString buf)
{
    SOPC_Variant* pvar = SOPC_Malloc(sizeof(SOPC_Variant));

    if (NULL == pvar)
        return NULL;

    SOPC_Variant_Initialize(pvar);
    pvar->BuiltInTypeId = SOPC_ByteString_Id;
    pvar->ArrayType = SOPC_VariantArrayType_SingleValue;
    pvar->Value.Bstring = buf;
    pvar->DoNotClear = true; // It is shallow copy of provided node

    return pvar;
}

void util_variant__print_SOPC_Variant(SOPC_Variant* pvar)
{
    size_t i;
    uint8_t c;
    printf("Variant @%p", (void*) pvar);
    char* s = NULL;

    if (NULL == pvar)
        return;
    printf(":\n  TypeId %i: ", pvar->BuiltInTypeId);

    switch (pvar->BuiltInTypeId)
    {
    case SOPC_Null_Id:
        printf("Null\n");
        break;
    case SOPC_Boolean_Id:
        printf("Boolean\n  Value: %" PRIu8 "\n", pvar->Value.Boolean);
        break;
    case SOPC_Int32_Id:
        printf("Int32\n  Value: %" PRIi32 "\n", pvar->Value.Int32);
        break;
    case SOPC_UInt32_Id:
        printf("UInt32\n  Value: %" PRIu32 "\n", pvar->Value.Uint32);
        break;
    case SOPC_Int64_Id:
        printf("Int64\n  Value: %" PRIi64 "\n", pvar->Value.Int64);
        break;
    case SOPC_Float_Id:
        printf("Float\n  Value: %g\n", pvar->Value.Floatv);
        break;
    case SOPC_Double_Id:
        printf("Double\n  Value: %g\n", pvar->Value.Doublev);
        break;
    case SOPC_String_Id:
        printf("String\n  Value: \"%*.*s\"\n", pvar->Value.String.Length, pvar->Value.String.Length,
               pvar->Value.String.Data);
        break;
    case SOPC_ByteString_Id:
        printf("ByteString\n  Length: %" PRIi32 "\n  Value: \"", pvar->Value.Bstring.Length);
        /* Pretty print */
        for (i = 0; i < (size_t) pvar->Value.Bstring.Length; ++i)
        {
            c = pvar->Value.Bstring.Data[i];
            if (0x20 <= c && c < 0x80)
                /* Displayable ascii range */
                printf("%c", c);
            else
                /* Special char */
                printf("\\x%02" PRIX8, c);
        }
        printf("\"\n");
        break;
    case SOPC_XmlElement_Id:
        printf("XmlElement\n  Length: %" PRIi32 "\n  Value: \"", pvar->Value.XmlElt.Length);
        /* Pretty print */
        for (i = 0; i < (size_t) pvar->Value.XmlElt.Length; ++i)
        {
            c = pvar->Value.XmlElt.Data[i];
            if (0x20 <= c && c < 0x80)
                /* Displayable ascii range */
                printf("%c", c);
            else
                /* Special char */
                printf("\\x%02" PRIX8, c);
        }
        printf("\"\n");
        break;
    case SOPC_NodeId_Id:
        s = SOPC_NodeId_ToCString(pvar->Value.NodeId);
        printf("NodeId\n  Value: %s\n", s);
        SOPC_Free(s);
        s = NULL;
        break;
    case SOPC_StatusCode_Id:
        printf("StatusCode\n  Value: %" PRIu32 "\n", pvar->Value.Status);
        break;
    case SOPC_SByte_Id:
    case SOPC_Byte_Id:
    case SOPC_Int16_Id:
    case SOPC_UInt16_Id:
    case SOPC_UInt64_Id:
    case SOPC_DateTime_Id:
    case SOPC_Guid_Id:
    case SOPC_ExpandedNodeId_Id:
    case SOPC_QualifiedName_Id:
    case SOPC_LocalizedText_Id:
    case SOPC_ExtensionObject_Id:
    case SOPC_DataValue_Id:
    case SOPC_Variant_Id: /* This one does not have an implementation at all */
    case SOPC_DiagnosticInfo_Id:
    default:
        printf("Unknown\n");
        break;
    }
}
