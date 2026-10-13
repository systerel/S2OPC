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

#include "check_helpers.h"

#include <check.h>
#include <math.h>
#include <string.h>

#include "sopc_builtintypes.h"
#include "sopc_mem_alloc.h"
#include "sopc_numeric_range.h"
#include "sopc_singly_linked_list.h"

START_TEST(test_expanded_node_id_equal)
{
    SOPC_ExpandedNodeId a;
    SOPC_ExpandedNodeId b;
    SOPC_ExpandedNodeId_Initialize(&a);
    SOPC_ExpandedNodeId_Initialize(&b);
    a.NodeId.IdentifierType = SOPC_IdentifierType_Numeric;
    a.NodeId.Namespace = 1;
    a.NodeId.Data.Numeric = 42;
    a.ServerIndex = 7;
    ck_assert(SOPC_String_InitializeFromCString(&a.NamespaceUri, "urn:test") == SOPC_STATUS_OK);
    ck_assert(SOPC_ExpandedNodeId_Copy(&b, &a) == SOPC_STATUS_OK);

    ck_assert(SOPC_ExpandedNodeId_Equal(&a, &b));

    b.ServerIndex = 8;
    ck_assert(!SOPC_ExpandedNodeId_Equal(&a, &b));
    b.ServerIndex = 7;
    ck_assert(SOPC_ExpandedNodeId_Equal(&a, &b));

    b.NodeId.Data.Numeric = 43;
    ck_assert(!SOPC_ExpandedNodeId_Equal(&a, &b));
    b.NodeId.Data.Numeric = 42;

    SOPC_String_Clear(&b.NamespaceUri);
    ck_assert(SOPC_String_InitializeFromCString(&b.NamespaceUri, "urn:other") == SOPC_STATUS_OK);
    ck_assert(!SOPC_ExpandedNodeId_Equal(&a, &b));

    ck_assert(!SOPC_ExpandedNodeId_Equal(NULL, &b));
    ck_assert(!SOPC_ExpandedNodeId_Equal(&a, NULL));

    SOPC_ExpandedNodeId_Clear(&a);
    SOPC_ExpandedNodeId_Clear(&b);
}
END_TEST

START_TEST(test_qualified_name_equal)
{
    SOPC_QualifiedName a;
    SOPC_QualifiedName b;
    SOPC_QualifiedName_Initialize(&a);
    SOPC_QualifiedName_Initialize(&b);
    a.NamespaceIndex = 3;
    ck_assert(SOPC_String_InitializeFromCString(&a.Name, "MyName") == SOPC_STATUS_OK);
    ck_assert(SOPC_QualifiedName_Copy(&b, &a) == SOPC_STATUS_OK);

    ck_assert(SOPC_QualifiedName_Equal(&a, &b));

    b.NamespaceIndex = 4;
    ck_assert(!SOPC_QualifiedName_Equal(&a, &b));
    b.NamespaceIndex = 3;

    SOPC_String_Clear(&b.Name);
    ck_assert(SOPC_String_InitializeFromCString(&b.Name, "OtherName") == SOPC_STATUS_OK);
    ck_assert(!SOPC_QualifiedName_Equal(&a, &b));

    ck_assert(!SOPC_QualifiedName_Equal(NULL, &b));
    ck_assert(!SOPC_QualifiedName_Equal(&a, NULL));

    SOPC_QualifiedName_Clear(&a);
    SOPC_QualifiedName_Clear(&b);
}
END_TEST

START_TEST(test_xml_element_equal)
{
    SOPC_XmlElement a;
    SOPC_XmlElement b;
    SOPC_XmlElement_Initialize(&a);
    SOPC_XmlElement_Initialize(&b);
    ck_assert(SOPC_String_InitializeFromCString((SOPC_String*) &a, "<a/>") == SOPC_STATUS_OK);
    ck_assert(SOPC_XmlElement_Copy(&b, &a) == SOPC_STATUS_OK);

    ck_assert(SOPC_XmlElement_Equal(&a, &b));

    SOPC_XmlElement_Clear(&b);
    ck_assert(SOPC_String_InitializeFromCString((SOPC_String*) &b, "<b/>") == SOPC_STATUS_OK);
    ck_assert(!SOPC_XmlElement_Equal(&a, &b));

    SOPC_XmlElement_Clear(&a);
    SOPC_XmlElement_Clear(&b);
}
END_TEST

static SOPC_LocalizedText* make_localized_text(const char* locale, const char* text)
{
    SOPC_LocalizedText* lt = SOPC_Malloc(sizeof(SOPC_LocalizedText));
    ck_assert_ptr_nonnull(lt);
    SOPC_LocalizedText_Initialize(lt);
    ck_assert(SOPC_String_InitializeFromCString(&lt->defaultLocale, locale) == SOPC_STATUS_OK);
    ck_assert(SOPC_String_InitializeFromCString(&lt->defaultText, text) == SOPC_STATUS_OK);
    return lt;
}

START_TEST(test_localized_text_equal_default_fields)
{
    SOPC_LocalizedText a;
    SOPC_LocalizedText b;
    SOPC_LocalizedText_Initialize(&a);
    SOPC_LocalizedText_Initialize(&b);
    ck_assert(SOPC_String_InitializeFromCString(&a.defaultLocale, "en-US") == SOPC_STATUS_OK);
    ck_assert(SOPC_String_InitializeFromCString(&a.defaultText, "Hello") == SOPC_STATUS_OK);
    ck_assert(SOPC_LocalizedText_Copy(&b, &a) == SOPC_STATUS_OK);

    ck_assert(SOPC_LocalizedText_Equal(&a, &b));

    SOPC_String_Clear(&b.defaultText);
    ck_assert(SOPC_String_InitializeFromCString(&b.defaultText, "Bonjour") == SOPC_STATUS_OK);
    ck_assert(!SOPC_LocalizedText_Equal(&a, &b));
    SOPC_String_Clear(&b.defaultText);
    ck_assert(SOPC_String_InitializeFromCString(&b.defaultText, "Hello") == SOPC_STATUS_OK);
    ck_assert(SOPC_LocalizedText_Equal(&a, &b));

    SOPC_String_Clear(&b.defaultLocale);
    ck_assert(SOPC_String_InitializeFromCString(&b.defaultLocale, "fr-FR") == SOPC_STATUS_OK);
    ck_assert(!SOPC_LocalizedText_Equal(&a, &b));

    ck_assert(!SOPC_LocalizedText_Equal(NULL, &b));
    ck_assert(!SOPC_LocalizedText_Equal(&a, NULL));

    SOPC_LocalizedText_Clear(&a);
    SOPC_LocalizedText_Clear(&b);
}
END_TEST

START_TEST(test_localized_text_equal_null_vs_empty_list)
{
    // Parity with SOPC_LocalizedText_Compare's existing quirk: a NULL list and a non-NULL
    // list of length 0 compare equal.
    SOPC_LocalizedText a;
    SOPC_LocalizedText b;
    SOPC_LocalizedText_Initialize(&a);
    SOPC_LocalizedText_Initialize(&b);
    ck_assert(SOPC_String_InitializeFromCString(&a.defaultLocale, "en-US") == SOPC_STATUS_OK);
    ck_assert(SOPC_String_InitializeFromCString(&a.defaultText, "Hello") == SOPC_STATUS_OK);
    ck_assert(SOPC_LocalizedText_Copy(&b, &a) == SOPC_STATUS_OK);
    ck_assert_ptr_null(a.localizedTextList);

    b.localizedTextList = SOPC_SLinkedList_Create(0);
    ck_assert_ptr_nonnull(b.localizedTextList);

    ck_assert(SOPC_LocalizedText_Equal(&a, &b));
    ck_assert(SOPC_LocalizedText_Equal(&b, &a));

    SOPC_LocalizedText_Clear(&a);
    SOPC_LocalizedText_Clear(&b);
}
END_TEST

START_TEST(test_localized_text_equal_list_contents)
{
    SOPC_LocalizedText a;
    SOPC_LocalizedText b;
    SOPC_LocalizedText_Initialize(&a);
    SOPC_LocalizedText_Initialize(&b);
    ck_assert(SOPC_String_InitializeFromCString(&a.defaultLocale, "en-US") == SOPC_STATUS_OK);
    ck_assert(SOPC_String_InitializeFromCString(&a.defaultText, "Hello") == SOPC_STATUS_OK);
    ck_assert(SOPC_LocalizedText_Copy(&b, &a) == SOPC_STATUS_OK);

    a.localizedTextList = SOPC_SLinkedList_Create(0);
    b.localizedTextList = SOPC_SLinkedList_Create(0);
    ck_assert_ptr_nonnull(a.localizedTextList);
    ck_assert_ptr_nonnull(b.localizedTextList);

    ck_assert(0 !=
              SOPC_SLinkedList_Append(a.localizedTextList, 0, (uintptr_t) make_localized_text("fr-FR", "Bonjour")));
    ck_assert(0 !=
              SOPC_SLinkedList_Append(b.localizedTextList, 0, (uintptr_t) make_localized_text("fr-FR", "Bonjour")));
    ck_assert(SOPC_LocalizedText_Equal(&a, &b));

    // Differing-length lists are not equal
    ck_assert(0 != SOPC_SLinkedList_Append(a.localizedTextList, 0, (uintptr_t) make_localized_text("de-DE", "Hallo")));
    ck_assert(!SOPC_LocalizedText_Equal(&a, &b));
    ck_assert(!SOPC_LocalizedText_Equal(&b, &a));

    // Same length, differing nested text -> not equal
    ck_assert(0 != SOPC_SLinkedList_Append(b.localizedTextList, 0, (uintptr_t) make_localized_text("de-DE", "Hallo!")));
    ck_assert(!SOPC_LocalizedText_Equal(&a, &b));

    SOPC_LocalizedText_Clear(&a);
    SOPC_LocalizedText_Clear(&b);
}
END_TEST

START_TEST(test_diagnostic_info_equal_scalar_fields)
{
    SOPC_DiagnosticInfo a;
    SOPC_DiagnosticInfo b;
    SOPC_DiagnosticInfo_Initialize(&a);
    SOPC_DiagnosticInfo_Initialize(&b);
    a.SymbolicId = 1;
    a.NamespaceUri = 2;
    a.Locale = 3;
    a.LocalizedText = 4;
    ck_assert(SOPC_String_InitializeFromCString(&a.AdditionalInfo, "info") == SOPC_STATUS_OK);
    ck_assert(SOPC_DiagnosticInfo_Copy(&b, &a) == SOPC_STATUS_OK);

    ck_assert(SOPC_DiagnosticInfo_Equal(&a, &b));

    b.SymbolicId = 99;
    ck_assert(!SOPC_DiagnosticInfo_Equal(&a, &b));
    b.SymbolicId = 1;

    b.NamespaceUri = 99;
    ck_assert(!SOPC_DiagnosticInfo_Equal(&a, &b));
    b.NamespaceUri = 2;
    ck_assert(SOPC_DiagnosticInfo_Equal(&a, &b));

    b.Locale = 99;
    ck_assert(!SOPC_DiagnosticInfo_Equal(&a, &b));
    b.Locale = 3;
    ck_assert(SOPC_DiagnosticInfo_Equal(&a, &b));

    b.LocalizedText = 99;
    ck_assert(!SOPC_DiagnosticInfo_Equal(&a, &b));
    b.LocalizedText = 4;
    ck_assert(SOPC_DiagnosticInfo_Equal(&a, &b));

    SOPC_String_Clear(&b.AdditionalInfo);
    ck_assert(SOPC_String_InitializeFromCString(&b.AdditionalInfo, "other") == SOPC_STATUS_OK);
    ck_assert(!SOPC_DiagnosticInfo_Equal(&a, &b));

    ck_assert(!SOPC_DiagnosticInfo_Equal(NULL, &b));
    ck_assert(!SOPC_DiagnosticInfo_Equal(&a, NULL));

    SOPC_DiagnosticInfo_Clear(&a);
    SOPC_DiagnosticInfo_Clear(&b);
}
END_TEST

START_TEST(test_diagnostic_info_equal_inner_status_code_ignored)
{
    // Locks in the same (pre-existing) omission as SOPC_DiagnosticInfo_Compare: InnerStatusCode
    // is not part of the equality, on purpose (parity, not a fix).
    SOPC_DiagnosticInfo a;
    SOPC_DiagnosticInfo b;
    SOPC_DiagnosticInfo_Initialize(&a);
    SOPC_DiagnosticInfo_Initialize(&b);
    ck_assert(SOPC_DiagnosticInfo_Copy(&b, &a) == SOPC_STATUS_OK);

    a.InnerStatusCode = 0x00000000;
    b.InnerStatusCode = 0x80000000; // Bad status
    ck_assert(SOPC_DiagnosticInfo_Equal(&a, &b));

    int32_t cmp = -1;
    ck_assert(SOPC_DiagnosticInfo_Compare(&a, &b, &cmp) == SOPC_STATUS_OK);
    ck_assert_int_eq(0, cmp); // Same pre-existing behavior on the Compare side

    SOPC_DiagnosticInfo_Clear(&a);
    SOPC_DiagnosticInfo_Clear(&b);
}
END_TEST

START_TEST(test_diagnostic_info_equal_inner_diagnostic_info)
{
    SOPC_DiagnosticInfo a;
    SOPC_DiagnosticInfo b;
    SOPC_DiagnosticInfo_Initialize(&a);
    SOPC_DiagnosticInfo_Initialize(&b);
    ck_assert(SOPC_DiagnosticInfo_Copy(&b, &a) == SOPC_STATUS_OK);
    ck_assert(SOPC_DiagnosticInfo_Equal(&a, &b)); // both NULL InnerDiagnosticInfo

    a.InnerDiagnosticInfo = SOPC_Malloc(sizeof(SOPC_DiagnosticInfo));
    ck_assert_ptr_nonnull(a.InnerDiagnosticInfo);
    SOPC_DiagnosticInfo_Initialize(a.InnerDiagnosticInfo);
    a.InnerDiagnosticInfo->SymbolicId = 5;

    // Asymmetric NULL -> not equal
    ck_assert(!SOPC_DiagnosticInfo_Equal(&a, &b));

    b.InnerDiagnosticInfo = SOPC_Malloc(sizeof(SOPC_DiagnosticInfo));
    ck_assert_ptr_nonnull(b.InnerDiagnosticInfo);
    SOPC_DiagnosticInfo_Initialize(b.InnerDiagnosticInfo);
    b.InnerDiagnosticInfo->SymbolicId = 5;
    ck_assert(SOPC_DiagnosticInfo_Equal(&a, &b));

    b.InnerDiagnosticInfo->SymbolicId = 6;
    ck_assert(!SOPC_DiagnosticInfo_Equal(&a, &b));

    SOPC_DiagnosticInfo_Clear(&a); // recursively clears+frees InnerDiagnosticInfo
    SOPC_DiagnosticInfo_Clear(&b);
}
END_TEST

static SOPC_Variant* make_scalar_variant(SOPC_BuiltinId type)
{
    SOPC_Variant* v = SOPC_Variant_Create();
    ck_assert_ptr_nonnull(v);
    v->BuiltInTypeId = type;
    v->ArrayType = SOPC_VariantArrayType_SingleValue;
    return v;
}

START_TEST(test_variant_equal_scalar_numeric_and_string_types)
{
    SOPC_Variant* a = make_scalar_variant(SOPC_UInt32_Id);
    SOPC_Variant* b = make_scalar_variant(SOPC_UInt32_Id);
    a->Value.Uint32 = 42;
    b->Value.Uint32 = 42;
    ck_assert(SOPC_Variant_Equal(a, b));
    b->Value.Uint32 = 43;
    ck_assert(!SOPC_Variant_Equal(a, b));
    SOPC_Variant_Delete(a);
    SOPC_Variant_Delete(b);

    a = make_scalar_variant(SOPC_String_Id);
    b = make_scalar_variant(SOPC_String_Id);
    ck_assert(SOPC_String_InitializeFromCString(&a->Value.String, "hello") == SOPC_STATUS_OK);
    ck_assert(SOPC_String_InitializeFromCString(&b->Value.String, "hello") == SOPC_STATUS_OK);
    ck_assert(SOPC_Variant_Equal(a, b));
    SOPC_String_Clear(&b->Value.String);
    ck_assert(SOPC_String_InitializeFromCString(&b->Value.String, "world") == SOPC_STATUS_OK);
    ck_assert(!SOPC_Variant_Equal(a, b));
    SOPC_Variant_Delete(a);
    SOPC_Variant_Delete(b);

    // Different BuiltInTypeId -> never equal
    a = make_scalar_variant(SOPC_Int32_Id);
    b = make_scalar_variant(SOPC_UInt32_Id);
    a->Value.Int32 = 1;
    b->Value.Uint32 = 1;
    ck_assert(!SOPC_Variant_Equal(a, b));
    SOPC_Variant_Delete(a);
    SOPC_Variant_Delete(b);
}
END_TEST

START_TEST(test_variant_equal_float_double_nan_parity_with_compare)
{
    SOPC_Variant* a = make_scalar_variant(SOPC_Double_Id);
    SOPC_Variant* b = make_scalar_variant(SOPC_Double_Id);
    double nan_val = nan("");
    a->Value.Doublev = nan_val;
    b->Value.Doublev = 3.0;

    bool equal = SOPC_Variant_Equal(a, b);
    int32_t cmp = -1;
    ck_assert(SOPC_Variant_Compare(a, b, &cmp) == SOPC_STATUS_OK);
    ck_assert(equal == (0 == cmp)); // same NaN-vs-value parity as Compare

    b->Value.Doublev = nan_val;
    equal = SOPC_Variant_Equal(a, b);
    ck_assert(SOPC_Variant_Compare(a, b, &cmp) == SOPC_STATUS_OK);
    ck_assert(equal == (0 == cmp)); // same NaN-vs-NaN parity as Compare

    SOPC_Variant_Delete(a);
    SOPC_Variant_Delete(b);
}
END_TEST

START_TEST(test_variant_equal_guid)
{
    SOPC_Variant* a = make_scalar_variant(SOPC_Guid_Id);
    SOPC_Variant* b = make_scalar_variant(SOPC_Guid_Id);
    a->Value.Guid = SOPC_Malloc(sizeof(SOPC_Guid));
    b->Value.Guid = SOPC_Malloc(sizeof(SOPC_Guid));
    ck_assert_ptr_nonnull(a->Value.Guid);
    ck_assert_ptr_nonnull(b->Value.Guid);
    SOPC_Guid_Initialize(a->Value.Guid);
    a->Value.Guid->Data1 = 0x12345678;
    a->Value.Guid->Data2 = 0xABCD;
    a->Value.Guid->Data3 = 0xEF01;
    memset(a->Value.Guid->Data4, 0x42, sizeof(a->Value.Guid->Data4));
    ck_assert(SOPC_Guid_Copy(b->Value.Guid, a->Value.Guid) == SOPC_STATUS_OK);

    ck_assert(SOPC_Variant_Equal(a, b));
    b->Value.Guid->Data4[7] = 0x43;
    ck_assert(!SOPC_Variant_Equal(a, b));

    SOPC_Variant_Delete(a);
    SOPC_Variant_Delete(b);
}
END_TEST

START_TEST(test_variant_equal_extension_object_fallback)
{
    SOPC_Variant* a = make_scalar_variant(SOPC_ExtensionObject_Id);
    SOPC_Variant* b = make_scalar_variant(SOPC_ExtensionObject_Id);
    a->Value.ExtObject = SOPC_Malloc(sizeof(SOPC_ExtensionObject));
    b->Value.ExtObject = SOPC_Malloc(sizeof(SOPC_ExtensionObject));
    ck_assert_ptr_nonnull(a->Value.ExtObject);
    ck_assert_ptr_nonnull(b->Value.ExtObject);
    SOPC_ExtensionObject_Initialize(a->Value.ExtObject);
    SOPC_ExtensionObject_Initialize(b->Value.ExtObject);
    a->Value.ExtObject->Encoding = SOPC_ExtObjBodyEncoding_ByteString;
    b->Value.ExtObject->Encoding = SOPC_ExtObjBodyEncoding_ByteString;
    ck_assert(SOPC_String_InitializeFromCString((SOPC_String*) &a->Value.ExtObject->Body.Bstring, "payload") ==
              SOPC_STATUS_OK);
    ck_assert(SOPC_ByteString_Copy(&b->Value.ExtObject->Body.Bstring, &a->Value.ExtObject->Body.Bstring) ==
              SOPC_STATUS_OK);

    ck_assert(SOPC_Variant_Equal(a, b));

    SOPC_ByteString_Clear(&b->Value.ExtObject->Body.Bstring);
    ck_assert(SOPC_String_InitializeFromCString((SOPC_String*) &b->Value.ExtObject->Body.Bstring, "other") ==
              SOPC_STATUS_OK);
    ck_assert(!SOPC_Variant_Equal(a, b));

    SOPC_Variant_Delete(a);
    SOPC_Variant_Delete(b);
}
END_TEST

START_TEST(test_variant_equal_datavalue_scalar_and_recursive_value)
{
    SOPC_Variant* a = make_scalar_variant(SOPC_DataValue_Id);
    SOPC_Variant* b = make_scalar_variant(SOPC_DataValue_Id);
    a->Value.DataValue = SOPC_Malloc(sizeof(SOPC_DataValue));
    b->Value.DataValue = SOPC_Malloc(sizeof(SOPC_DataValue));
    ck_assert_ptr_nonnull(a->Value.DataValue);
    ck_assert_ptr_nonnull(b->Value.DataValue);
    SOPC_DataValue_Initialize(a->Value.DataValue);
    a->Value.DataValue->Status = 0;
    a->Value.DataValue->Value.BuiltInTypeId = SOPC_Int32_Id;
    a->Value.DataValue->Value.ArrayType = SOPC_VariantArrayType_SingleValue;
    a->Value.DataValue->Value.Value.Int32 = 7;
    ck_assert(SOPC_DataValue_Copy(b->Value.DataValue, a->Value.DataValue) == SOPC_STATUS_OK);

    ck_assert(SOPC_Variant_Equal(a, b));
    ck_assert(SOPC_DataValue_Equal(a->Value.DataValue, b->Value.DataValue));

    b->Value.DataValue->Status = 1;
    ck_assert(!SOPC_DataValue_Equal(a->Value.DataValue, b->Value.DataValue));
    b->Value.DataValue->Status = 0;
    ck_assert(SOPC_DataValue_Equal(a->Value.DataValue, b->Value.DataValue));

    b->Value.DataValue->ServerTimestamp = 12345;
    ck_assert(!SOPC_DataValue_Equal(a->Value.DataValue, b->Value.DataValue));
    b->Value.DataValue->ServerTimestamp = a->Value.DataValue->ServerTimestamp;
    ck_assert(SOPC_DataValue_Equal(a->Value.DataValue, b->Value.DataValue));

    b->Value.DataValue->ServerPicoSeconds = 99;
    ck_assert(!SOPC_DataValue_Equal(a->Value.DataValue, b->Value.DataValue));
    b->Value.DataValue->ServerPicoSeconds = a->Value.DataValue->ServerPicoSeconds;
    ck_assert(SOPC_DataValue_Equal(a->Value.DataValue, b->Value.DataValue));

    b->Value.DataValue->SourceTimestamp = 12345;
    ck_assert(!SOPC_DataValue_Equal(a->Value.DataValue, b->Value.DataValue));
    b->Value.DataValue->SourceTimestamp = a->Value.DataValue->SourceTimestamp;
    ck_assert(SOPC_DataValue_Equal(a->Value.DataValue, b->Value.DataValue));

    b->Value.DataValue->SourcePicoSeconds = 99;
    ck_assert(!SOPC_DataValue_Equal(a->Value.DataValue, b->Value.DataValue));
    b->Value.DataValue->SourcePicoSeconds = a->Value.DataValue->SourcePicoSeconds;
    ck_assert(SOPC_DataValue_Equal(a->Value.DataValue, b->Value.DataValue));

    b->Value.DataValue->Value.Value.Int32 = 8;
    ck_assert(!SOPC_Variant_Equal(a, b)); // change is inside the recursive Value field

    SOPC_Variant_Delete(a);
    SOPC_Variant_Delete(b);
}
END_TEST

START_TEST(test_variant_equal_edge_cases)
{
    SOPC_Variant* a = make_scalar_variant(SOPC_Int32_Id);
    a->Value.Int32 = 1;
    ck_assert(!SOPC_Variant_Equal(NULL, a));
    ck_assert(!SOPC_Variant_Equal(a, NULL));

    SOPC_Variant* v = make_scalar_variant(SOPC_Variant_Id); // forbidden as a scalar value (Part 6)
    SOPC_Variant* w = make_scalar_variant(SOPC_Variant_Id);
    ck_assert(!SOPC_Variant_Equal(v, w));

    SOPC_Variant_Delete(a);
    SOPC_Variant_Delete(v);
    SOPC_Variant_Delete(w);
}
END_TEST

static SOPC_Variant* make_array_variant(SOPC_BuiltinId type, int32_t length)
{
    SOPC_Variant* v = SOPC_Variant_Create();
    ck_assert_ptr_nonnull(v);
    ck_assert(SOPC_Variant_Initialize_Array(v, type, length));
    return v;
}

START_TEST(test_variant_equal_range_array_and_getrange_failure)
{
    SOPC_Variant* a = make_array_variant(SOPC_Int32_Id, 5);
    SOPC_Variant* b = make_array_variant(SOPC_Int32_Id, 5);
    for (int32_t i = 0; i < 5; i++)
    {
        a->Value.Array.Content.Int32Arr[i] = i;
        b->Value.Array.Content.Int32Arr[i] = i;
    }

    SOPC_NumericRange* range = NULL;
    ck_assert(SOPC_NumericRange_Parse("1:2", &range) == SOPC_STATUS_OK);
    ck_assert_ptr_nonnull(range);

    ck_assert(SOPC_Variant_EqualRange(a, b, range));

    b->Value.Array.Content.Int32Arr[1] = 99; // inside the [1:2] sub-range
    ck_assert(!SOPC_Variant_EqualRange(a, b, range));

    b->Value.Array.Content.Int32Arr[1] = 1;
    b->Value.Array.Content.Int32Arr[4] = 99; // outside the [1:2] sub-range
    ck_assert(SOPC_Variant_EqualRange(a, b, range));

    SOPC_NumericRange_Delete(range);

    // A 2-dimension range applied to a 1-dimension (non-string, non-Matrix) array is invalid ->
    // SOPC_Variant_GetRange fails -> SOPC_Variant_EqualRange is false
    SOPC_NumericRange* badRange = NULL;
    ck_assert(SOPC_NumericRange_Parse("0:1,0:1", &badRange) == SOPC_STATUS_OK);
    ck_assert_ptr_nonnull(badRange);
    ck_assert(!SOPC_Variant_EqualRange(a, b, badRange));
    SOPC_NumericRange_Delete(badRange);

    // range == NULL delegates to SOPC_Variant_Equal
    ck_assert(SOPC_Variant_EqualRange(a, a, NULL));

    SOPC_Variant_Delete(a);
    SOPC_Variant_Delete(b);
}
END_TEST

START_TEST(test_variant_equal_pod_array_memcmp_path)
{
    // UInt32 arrays are contiguous memory block (Plain Old Data): use memcmp path for SOPC_Variant_Equal
    SOPC_Variant* a = make_array_variant(SOPC_UInt32_Id, 5);
    SOPC_Variant* b = make_array_variant(SOPC_UInt32_Id, 5);
    for (int32_t i = 0; i < 5; i++)
    {
        a->Value.Array.Content.Uint32Arr[i] = (uint32_t) i;
        b->Value.Array.Content.Uint32Arr[i] = (uint32_t) i;
    }
    ck_assert(SOPC_Variant_Equal(a, b));

    // Differing at the first element
    b->Value.Array.Content.Uint32Arr[0] = 99;
    ck_assert(!SOPC_Variant_Equal(a, b));
    b->Value.Array.Content.Uint32Arr[0] = 0;

    // Differing at the last element
    b->Value.Array.Content.Uint32Arr[4] = 99;
    ck_assert(!SOPC_Variant_Equal(a, b));

    SOPC_Variant_Delete(a);
    SOPC_Variant_Delete(b);

    // Differing length
    a = make_array_variant(SOPC_UInt32_Id, 3);
    b = make_array_variant(SOPC_UInt32_Id, 4);
    ck_assert(!SOPC_Variant_Equal(a, b));
    SOPC_Variant_Delete(a);
    SOPC_Variant_Delete(b);
}
END_TEST

START_TEST(test_variant_equal_guid_array_memcmp_path)
{
    SOPC_Variant* a = make_array_variant(SOPC_Guid_Id, 2);
    SOPC_Variant* b = make_array_variant(SOPC_Guid_Id, 2);
    for (int32_t i = 0; i < 2; i++)
    {
        SOPC_Guid_Initialize(&a->Value.Array.Content.GuidArr[i]);
        a->Value.Array.Content.GuidArr[i].Data1 = (uint32_t) i;
        SOPC_Guid_Initialize(&b->Value.Array.Content.GuidArr[i]);
        b->Value.Array.Content.GuidArr[i].Data1 = (uint32_t) i;
    }
    ck_assert(SOPC_Variant_Equal(a, b));
    b->Value.Array.Content.GuidArr[1].Data1 = 99;
    ck_assert(!SOPC_Variant_Equal(a, b));
    SOPC_Variant_Delete(a);
    SOPC_Variant_Delete(b);
}
END_TEST

START_TEST(test_variant_equal_non_pod_array_still_correct)
{
    // String arrays are not contiguous memory block (Plain Old Data): do not use memcmp path for SOPC_Variant_Equal
    SOPC_Variant* a = make_array_variant(SOPC_String_Id, 2);
    SOPC_Variant* b = make_array_variant(SOPC_String_Id, 2);
    ck_assert(SOPC_String_InitializeFromCString(&a->Value.Array.Content.StringArr[0], "one") == SOPC_STATUS_OK);
    ck_assert(SOPC_String_InitializeFromCString(&a->Value.Array.Content.StringArr[1], "two") == SOPC_STATUS_OK);
    ck_assert(SOPC_String_InitializeFromCString(&b->Value.Array.Content.StringArr[0], "one") == SOPC_STATUS_OK);
    ck_assert(SOPC_String_InitializeFromCString(&b->Value.Array.Content.StringArr[1], "two") == SOPC_STATUS_OK);
    ck_assert(SOPC_Variant_Equal(a, b));

    SOPC_String_Clear(&b->Value.Array.Content.StringArr[1]);
    ck_assert(SOPC_String_InitializeFromCString(&b->Value.Array.Content.StringArr[1], "other") == SOPC_STATUS_OK);
    ck_assert(!SOPC_Variant_Equal(a, b));

    SOPC_Variant_Delete(a);
    SOPC_Variant_Delete(b);
}
END_TEST

START_TEST(test_variant_equal_matrix)
{
    SOPC_Variant* a = SOPC_Variant_Create();
    SOPC_Variant* b = SOPC_Variant_Create();
    ck_assert_ptr_nonnull(a);
    ck_assert_ptr_nonnull(b);
    a->BuiltInTypeId = SOPC_Int32_Id;
    a->ArrayType = SOPC_VariantArrayType_Matrix;
    a->Value.Matrix.Dimensions = 2;
    a->Value.Matrix.ArrayDimensions = SOPC_Malloc(2 * sizeof(int32_t));
    ck_assert_ptr_nonnull(a->Value.Matrix.ArrayDimensions);
    a->Value.Matrix.ArrayDimensions[0] = 2;
    a->Value.Matrix.ArrayDimensions[1] = 3;
    a->Value.Matrix.Content.Int32Arr = SOPC_Malloc(6 * sizeof(int32_t));
    ck_assert_ptr_nonnull(a->Value.Matrix.Content.Int32Arr);
    for (int32_t i = 0; i < 6; i++)
    {
        a->Value.Matrix.Content.Int32Arr[i] = i;
    }
    ck_assert(SOPC_Variant_Copy(b, a) == SOPC_STATUS_OK);

    ck_assert(SOPC_Variant_Equal(a, b));

    b->Value.Matrix.Content.Int32Arr[5] = 99;
    ck_assert(!SOPC_Variant_Equal(a, b));
    b->Value.Matrix.Content.Int32Arr[5] = 5;

    b->Value.Matrix.ArrayDimensions[1] = 4; // dimension mismatch -> not equal, no OOB read
    b->Value.Matrix.Content.Int32Arr =
        SOPC_Realloc(b->Value.Matrix.Content.Int32Arr, 6 * sizeof(int32_t), 8 * sizeof(int32_t));
    ck_assert_ptr_nonnull(b->Value.Matrix.Content.Int32Arr);
    ck_assert(!SOPC_Variant_Equal(a, b));

    SOPC_Variant_Delete(a);
    SOPC_Variant_Delete(b);
}
END_TEST

Suite* tests_make_suite_variant_equal(void)
{
    Suite* s;
    TCase* tc_leaf_types;
    TCase* tc_localized_text;
    TCase* tc_diagnostic_info;

    s = suite_create("SOPC_Variant_Equal tests");
    tc_leaf_types = tcase_create("Leaf structured types");

    tcase_add_test(tc_leaf_types, test_expanded_node_id_equal);
    tcase_add_test(tc_leaf_types, test_qualified_name_equal);
    tcase_add_test(tc_leaf_types, test_xml_element_equal);
    suite_add_tcase(s, tc_leaf_types);

    tc_localized_text = tcase_create("LocalizedText");
    tcase_add_test(tc_localized_text, test_localized_text_equal_default_fields);
    tcase_add_test(tc_localized_text, test_localized_text_equal_null_vs_empty_list);
    tcase_add_test(tc_localized_text, test_localized_text_equal_list_contents);
    suite_add_tcase(s, tc_localized_text);

    tc_diagnostic_info = tcase_create("DiagnosticInfo");
    tcase_add_test(tc_diagnostic_info, test_diagnostic_info_equal_scalar_fields);
    tcase_add_test(tc_diagnostic_info, test_diagnostic_info_equal_inner_status_code_ignored);
    tcase_add_test(tc_diagnostic_info, test_diagnostic_info_equal_inner_diagnostic_info);
    suite_add_tcase(s, tc_diagnostic_info);

    TCase* tc_variant_scalar = tcase_create("Variant scalar dispatch");
    tcase_add_test(tc_variant_scalar, test_variant_equal_scalar_numeric_and_string_types);
    tcase_add_test(tc_variant_scalar, test_variant_equal_float_double_nan_parity_with_compare);
    tcase_add_test(tc_variant_scalar, test_variant_equal_guid);
    tcase_add_test(tc_variant_scalar, test_variant_equal_extension_object_fallback);
    tcase_add_test(tc_variant_scalar, test_variant_equal_datavalue_scalar_and_recursive_value);
    tcase_add_test(tc_variant_scalar, test_variant_equal_edge_cases);
    tcase_add_test(tc_variant_scalar, test_variant_equal_range_array_and_getrange_failure);
    suite_add_tcase(s, tc_variant_scalar);

    TCase* tc_variant_array = tcase_create("Variant array/matrix dispatch");
    tcase_add_test(tc_variant_array, test_variant_equal_pod_array_memcmp_path);
    tcase_add_test(tc_variant_array, test_variant_equal_guid_array_memcmp_path);
    tcase_add_test(tc_variant_array, test_variant_equal_non_pod_array_still_correct);
    tcase_add_test(tc_variant_array, test_variant_equal_matrix);
    suite_add_tcase(s, tc_variant_array);

    return s;
}
