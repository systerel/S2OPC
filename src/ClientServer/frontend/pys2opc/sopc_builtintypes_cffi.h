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
 * This file is an excerpt from sopc_builtintypes.h.
 * It should not be included in a generic project.
 * See s2opc_headers.h
 */

typedef SOPC_ReturnStatus(SOPC_EncodeableObject_PfnCopy)(void* dest, const void* src);
typedef SOPC_ReturnStatus(SOPC_EncodeableObject_PfnComp)(const void* left, const void* right, int32_t* comp);

typedef enum SOPC_BuiltinId
{
    SOPC_Null_Id = 0,
    SOPC_Boolean_Id = 1,
    SOPC_SByte_Id = 2,
    SOPC_Byte_Id = 3,
    SOPC_Int16_Id = 4,
    SOPC_UInt16_Id = 5,
    SOPC_Int32_Id = 6,
    SOPC_UInt32_Id = 7,
    SOPC_Int64_Id = 8,
    SOPC_UInt64_Id = 9,
    SOPC_Float_Id = 10,
    SOPC_Double_Id = 11,
    SOPC_String_Id = 12,
    SOPC_DateTime_Id = 13,
    SOPC_Guid_Id = 14,
    SOPC_ByteString_Id = 15,
    SOPC_XmlElement_Id = 16,
    SOPC_NodeId_Id = 17,
    SOPC_ExpandedNodeId_Id = 18,
    SOPC_StatusCode_Id = 19,
    SOPC_QualifiedName_Id = 20,
    SOPC_LocalizedText_Id = 21,
    SOPC_ExtensionObject_Id = 22,
    SOPC_DataValue_Id = 23,
    SOPC_Variant_Id = 24,
    SOPC_DiagnosticInfo_Id = 25
} SOPC_BuiltinId;

typedef uint8_t SOPC_Byte;

typedef SOPC_Byte SOPC_Boolean;

typedef int8_t SOPC_SByte;

typedef struct SOPC_String
{
    int32_t Length;
    bool DoNotClear; // flag indicating if bytes must be freed
    SOPC_Byte* Data;
} SOPC_String;

typedef SOPC_String SOPC_XmlElement;
typedef SOPC_String SOPC_ByteString;

typedef int64_t SOPC_DateTime;

typedef struct SOPC_Guid
{
    uint32_t Data1;
    uint16_t Data2;
    uint16_t Data3;
    SOPC_Byte Data4[8];
} SOPC_Guid;

typedef enum SOPC_IdentifierType
{
    SOPC_IdentifierType_Numeric = 0x00,
    SOPC_IdentifierType_String = 0x01,
    SOPC_IdentifierType_Guid = 0x02,
    SOPC_IdentifierType_ByteString = 0x03,
} SOPC_IdentifierType;

typedef struct SOPC_NodeId
{
    SOPC_IdentifierType IdentifierType;
    uint16_t Namespace;

    union {
        uint32_t Numeric;
        SOPC_String String;
        SOPC_Guid* Guid;
        SOPC_ByteString Bstring;
    } Data;
} SOPC_NodeId;

typedef struct SOPC_ExpandedNodeId
{
    SOPC_NodeId NodeId;
    SOPC_String NamespaceUri;
    uint32_t ServerIndex;
} SOPC_ExpandedNodeId;

typedef struct SOPC_DiagnosticInfo
{
    int32_t SymbolicId;
    int32_t NamespaceUri;
    int32_t Locale;
    int32_t LocalizedText;
    SOPC_String AdditionalInfo;
    SOPC_StatusCode InnerStatusCode;
    struct SOPC_DiagnosticInfo* InnerDiagnosticInfo;
} SOPC_DiagnosticInfo;

typedef struct SOPC_QualifiedName
{
    uint16_t NamespaceIndex;
    SOPC_String Name;
} SOPC_QualifiedName;

typedef struct SOPC_LocalizedText
{
    SOPC_String defaultLocale;
    SOPC_String defaultText;

    SOPC_SLinkedList* localizedTextList; // If NULL => no other localized text defined
} SOPC_LocalizedText;

typedef enum SOPC_ExtObjectBodyEncoding
{
    SOPC_ExtObjBodyEncoding_None = 0x00,
    SOPC_ExtObjBodyEncoding_ByteString = 0x01,
    SOPC_ExtObjBodyEncoding_XMLElement = 0x02,
    SOPC_ExtObjBodyEncoding_Object = 0x03
} SOPC_ExtObjectBodyEncoding;

typedef struct SOPC_ExtensionObject
{
    SOPC_ExpandedNodeId TypeId;
    SOPC_ExtObjectBodyEncoding Encoding;

    union {
        SOPC_ByteString Bstring;
        SOPC_XmlElement Xml;
        struct
        {
            void* Value;
            SOPC_EncodeableType* ObjType;
        } Object;

    } Body;

    int32_t Length;

} SOPC_ExtensionObject;

typedef enum SOPC_VariantArrayTypeFlag
{
    SOPC_VariantArrayValueFlag = 128,    // 2^7 => bit 7
    SOPC_VariantArrayDimensionsFlag = 64 // 2^6 => bit 6
} SOPC_VariantArrayTypeFlag;

// Binary compatible types
typedef enum SOPC_VariantArrayType
{
    SOPC_VariantArrayType_SingleValue = 0x0,
    SOPC_VariantArrayType_Array = 0x1,
    SOPC_VariantArrayType_Matrix = 0x2
} SOPC_VariantArrayType;

struct SOPC_DataValue;
struct SOPC_Variant;

typedef union SOPC_VariantArrayValue {
    SOPC_Boolean* BooleanArr;
    SOPC_SByte* SbyteArr;
    SOPC_Byte* ByteArr;
    int16_t* Int16Arr;
    uint16_t* Uint16Arr;
    int32_t* Int32Arr;
    uint32_t* Uint32Arr;
    int64_t* Int64Arr;
    uint64_t* Uint64Arr;
    float* FloatvArr;
    double* DoublevArr;
    SOPC_String* StringArr;
    SOPC_DateTime* DateArr;
    SOPC_Guid* GuidArr;
    SOPC_ByteString* BstringArr;
    SOPC_XmlElement* XmlEltArr;
    SOPC_NodeId* NodeIdArr;
    SOPC_ExpandedNodeId* ExpNodeIdArr;
    SOPC_StatusCode* StatusArr;
    SOPC_QualifiedName* QnameArr;
    SOPC_LocalizedText* LocalizedTextArr;
    SOPC_ExtensionObject* ExtObjectArr;
    struct SOPC_DataValue* DataValueArr;
    struct SOPC_Variant* VariantArr;
    SOPC_DiagnosticInfo* DiagInfoArr; // TODO: not present ?
} SOPC_VariantArrayValue;

typedef union SOPC_VariantValue {
    SOPC_Boolean Boolean;
    SOPC_SByte Sbyte;
    SOPC_Byte Byte;
    int16_t Int16;
    uint16_t Uint16;
    int32_t Int32;
    uint32_t Uint32;
    int64_t Int64;
    uint64_t Uint64;
    float Floatv;
    double Doublev;
    SOPC_String String;
    SOPC_DateTime Date;
    SOPC_Guid* Guid;
    SOPC_ByteString Bstring;
    SOPC_XmlElement XmlElt;
    SOPC_NodeId* NodeId;
    SOPC_ExpandedNodeId* ExpNodeId;
    SOPC_StatusCode Status;
    SOPC_QualifiedName* Qname;
    SOPC_LocalizedText* LocalizedText;
    SOPC_ExtensionObject* ExtObject;
    struct SOPC_DataValue* DataValue;
    SOPC_DiagnosticInfo* DiagInfo; // TODO: not present ?
    struct
    {
        int32_t Length;
        SOPC_VariantArrayValue Content;
    } Array;
    struct
    {
        int32_t Dimensions;
        int32_t*
            ArrayDimensions; // Product of dimensions must be <= INT32_MAX ! (binary arrayLength valid for matrix too)
        SOPC_VariantArrayValue Content;
    } Matrix;

} SOPC_VariantValue;

typedef struct SOPC_Variant
{
    bool DoNotClear; // flag indicating if variant content must be freed
    SOPC_BuiltinId BuiltInTypeId;
    SOPC_VariantArrayType ArrayType;
    SOPC_VariantValue Value;
} SOPC_Variant;

typedef struct SOPC_DataValue
{
    SOPC_Variant Value;
    SOPC_StatusCode Status;
    SOPC_DateTime SourceTimestamp; // 100 nanoseconds
    SOPC_DateTime ServerTimestamp; // 100 nanoseconds
    uint16_t SourcePicoSeconds;    // 10 picoseconds
    uint16_t ServerPicoSeconds;    // 10 picoseconds
} SOPC_DataValue;

void SOPC_Boolean_Initialize(SOPC_Boolean* b);
void SOPC_Boolean_InitializeAux(void* value);
SOPC_ReturnStatus SOPC_Boolean_CopyAux(void* dest, const void* src);
SOPC_ReturnStatus SOPC_Boolean_CompareAux(const void* left, const void* right, int32_t* comparison);
void SOPC_Boolean_Clear(SOPC_Boolean* b);
void SOPC_Boolean_ClearAux(void* value);

void SOPC_SByte_Initialize(SOPC_SByte* sbyte);
void SOPC_SByte_InitializeAux(void* value);
SOPC_ReturnStatus SOPC_SByte_CopyAux(void* dest, const void* src);
SOPC_ReturnStatus SOPC_SByte_CompareAux(const void* left, const void* right, int32_t* comparison);
void SOPC_SByte_Clear(SOPC_SByte* sbyte);
void SOPC_SByte_ClearAux(void* value);

void SOPC_Byte_Initialize(SOPC_Byte* byte);
void SOPC_Byte_InitializeAux(void* value);
SOPC_ReturnStatus SOPC_Byte_CopyAux(void* dest, const void* src);
SOPC_ReturnStatus SOPC_Byte_CompareAux(const void* left, const void* right, int32_t* comparison);
void SOPC_Byte_Clear(SOPC_Byte* byte);
void SOPC_Byte_ClearAux(void* value);

void SOPC_Int16_Initialize(int16_t* intv);
void SOPC_Int16_InitializeAux(void* value);
SOPC_ReturnStatus SOPC_Int16_CopyAux(void* dest, const void* src);
SOPC_ReturnStatus SOPC_Int16_CompareAux(const void* left, const void* right, int32_t* comparison);
void SOPC_Int16_Clear(int16_t* intv);
void SOPC_Int16_ClearAux(void* value);

void SOPC_UInt16_Initialize(uint16_t* uint);
void SOPC_UInt16_InitializeAux(void* value);
SOPC_ReturnStatus SOPC_UInt16_CopyAux(void* dest, const void* src);
SOPC_ReturnStatus SOPC_UInt16_CompareAux(const void* left, const void* right, int32_t* comparison);
void SOPC_UInt16_Clear(uint16_t* uint);
void SOPC_UInt16_ClearAux(void* value);

void SOPC_Int32_Initialize(int32_t* intv);
void SOPC_Int32_InitializeAux(void* value);
SOPC_ReturnStatus SOPC_Int32_CopyAux(void* dest, const void* src);
SOPC_ReturnStatus SOPC_Int32_CompareAux(const void* left, const void* right, int32_t* comparison);
void SOPC_Int32_Clear(int32_t* intv);
void SOPC_Int32_ClearAux(void* value);

void SOPC_UInt32_Initialize(uint32_t* uint);
void SOPC_UInt32_InitializeAux(void* value);
SOPC_ReturnStatus SOPC_UInt32_CopyAux(void* dest, const void* src);
SOPC_ReturnStatus SOPC_UInt32_CompareAux(const void* left, const void* right, int32_t* comparison);
void SOPC_UInt32_Clear(uint32_t* uint);
void SOPC_UInt32_ClearAux(void* value);

void SOPC_Int64_Initialize(int64_t* intv);
void SOPC_Int64_InitializeAux(void* value);
SOPC_ReturnStatus SOPC_Int64_CopyAux(void* dest, const void* src);
SOPC_ReturnStatus SOPC_Int64_CompareAux(const void* left, const void* right, int32_t* comparison);
void SOPC_Int64_Clear(int64_t* intv);
void SOPC_Int64_ClearAux(void* value);

void SOPC_UInt64_Initialize(uint64_t* uint);
void SOPC_UInt64_InitializeAux(void* value);
SOPC_ReturnStatus SOPC_UInt64_CopyAux(void* dest, const void* src);
SOPC_ReturnStatus SOPC_UInt64_CompareAux(const void* left, const void* right, int32_t* comparison);
void SOPC_UInt64_Clear(uint64_t* uint);
void SOPC_UInt64_ClearAux(void* value);

void SOPC_Float_Initialize(float* f);
void SOPC_Float_InitializeAux(void* value);
SOPC_ReturnStatus SOPC_Float_CopyAux(void* dest, const void* src);
SOPC_ReturnStatus SOPC_Float_CompareAux(const void* left, const void* right, int32_t* comparison);
void SOPC_Float_Clear(float* f);
void SOPC_Float_ClearAux(void* value);

void SOPC_Double_Initialize(double* d);
void SOPC_Double_InitializeAux(void* value);
SOPC_ReturnStatus SOPC_Double_CopyAux(void* dest, const void* src);
SOPC_ReturnStatus SOPC_Double_CompareAux(const void* left, const void* right, int32_t* comparison);
void SOPC_Double_Clear(double* d);
void SOPC_Double_ClearAux(void* value);

void SOPC_ByteString_Initialize(SOPC_ByteString* bstring);
void SOPC_ByteString_InitializeAux(void* value);
SOPC_ByteString* SOPC_ByteString_Create(void);
SOPC_ReturnStatus SOPC_ByteString_InitializeFixedSize(SOPC_ByteString* bstring, uint32_t size);
SOPC_ReturnStatus SOPC_ByteString_CopyFromBytes(SOPC_ByteString* dest, const SOPC_Byte* bytes, int32_t length);
SOPC_ReturnStatus SOPC_ByteString_Copy(SOPC_ByteString* dest, const SOPC_ByteString* src);
SOPC_ReturnStatus SOPC_ByteString_CopyAux(void* dest, const void* src);
void SOPC_ByteString_Clear(SOPC_ByteString* bstring);
void SOPC_ByteString_ClearAux(void* value);
void SOPC_ByteString_Delete(SOPC_ByteString* bstring);

SOPC_ReturnStatus SOPC_ByteString_Compare(const SOPC_ByteString* left,
                                          const SOPC_ByteString* right,
                                          int32_t* comparison);
SOPC_ReturnStatus SOPC_ByteString_CompareAux(const void* left, const void* right, int32_t* comparison);

bool SOPC_ByteString_Equal(const SOPC_ByteString* left, const SOPC_ByteString* right);

void SOPC_String_Initialize(SOPC_String* string);
void SOPC_String_InitializeAux(void* value);
SOPC_String* SOPC_String_Create(void);
SOPC_ReturnStatus SOPC_String_CopyFromCString(SOPC_String* string, const char* cString);
SOPC_ReturnStatus SOPC_String_CopyAux(void* dest, const void* src);
SOPC_ReturnStatus SOPC_String_InitializeFromCString(SOPC_String* string, const char* cString);
char* SOPC_String_GetCString(const SOPC_String* string);          // Copy
const char* SOPC_String_GetRawCString(const SOPC_String* string); // Pointer to string

SOPC_ReturnStatus SOPC_String_AttachFrom(SOPC_String* dest, SOPC_String* src);
SOPC_ReturnStatus SOPC_String_AttachFromCstring(SOPC_String* dest, char* src);

SOPC_ReturnStatus SOPC_String_Copy(SOPC_String* dest, const SOPC_String* src);
void SOPC_String_Clear(SOPC_String* bstring);
void SOPC_String_ClearAux(void* value);
void SOPC_String_Delete(SOPC_String* bstring);

SOPC_ReturnStatus SOPC_String_Compare(const SOPC_String* left,
                                      const SOPC_String* right,
                                      bool ignoreCase,
                                      int32_t* comparison);
SOPC_ReturnStatus SOPC_String_CompareAux(const void* left, const void* right, int32_t* comparison);

bool SOPC_String_Equal(const SOPC_String* left, const SOPC_String* right);

void SOPC_XmlElement_Initialize(SOPC_XmlElement* xmlElt);
void SOPC_XmlElement_InitializeAux(void* value);
SOPC_ReturnStatus SOPC_XmlElement_Copy(SOPC_XmlElement* dest, const SOPC_XmlElement* src);
SOPC_ReturnStatus SOPC_XmlElement_CopyAux(void* dest, const void* src);
SOPC_ReturnStatus SOPC_XmlElement_Compare(const SOPC_XmlElement* left,
                                          const SOPC_XmlElement* right,
                                          int32_t* comparison);
SOPC_ReturnStatus SOPC_XmlElement_CompareAux(const void* left, const void* right, int32_t* comparison);
void SOPC_XmlElement_Clear(SOPC_XmlElement* xmlElt);
void SOPC_XmlElement_ClearAux(void* value);

void SOPC_DateTime_Initialize(SOPC_DateTime* dateTime);
void SOPC_DateTime_InitializeAux(void* value);
SOPC_ReturnStatus SOPC_DateTime_CopyAux(void* dest, const void* src);
SOPC_ReturnStatus SOPC_DateTime_Compare(const SOPC_DateTime* left, const SOPC_DateTime* right, int32_t* comparison);
SOPC_ReturnStatus SOPC_DateTime_CompareAux(const void* left, const void* right, int32_t* comparison);
void SOPC_DateTime_Clear(SOPC_DateTime* dateTime);
void SOPC_DateTime_ClearAux(void* value);

void SOPC_Guid_Initialize(SOPC_Guid* guid);
void SOPC_Guid_InitializeAux(void* value);
SOPC_ReturnStatus SOPC_Guid_FromCString(SOPC_Guid* guid, const char* str, size_t len);
SOPC_ReturnStatus SOPC_Guid_Copy(SOPC_Guid* dest, const SOPC_Guid* src);
SOPC_ReturnStatus SOPC_Guid_CompareAux(const void* left, const void* right, int32_t* comparison);
SOPC_ReturnStatus SOPC_Guid_CopyAux(void* dest, const void* src);
void SOPC_Guid_Clear(SOPC_Guid* guid);
void SOPC_Guid_ClearAux(void* value);

void SOPC_NodeId_Initialize(SOPC_NodeId* nodeId);
void SOPC_NodeId_InitializeAux(void* value);
SOPC_ReturnStatus SOPC_NodeId_Copy(SOPC_NodeId* dest, const SOPC_NodeId* src);
SOPC_ReturnStatus SOPC_NodeId_CopyAux(void* dest, const void* src);
void SOPC_NodeId_Clear(SOPC_NodeId* nodeId);
void SOPC_NodeId_ClearAux(void* value);

SOPC_ReturnStatus SOPC_NodeId_Compare(const SOPC_NodeId* left, const SOPC_NodeId* right, int32_t* comparison);
SOPC_ReturnStatus SOPC_NodeId_CompareAux(const void* left, const void* right, int32_t* comparison);

void SOPC_NodeId_Hash(const SOPC_NodeId* nodeId, uint64_t* hash);

char* SOPC_NodeId_ToCString(const SOPC_NodeId* nodeId);
SOPC_NodeId* SOPC_NodeId_FromCString(const char* cString, int32_t len);

// SOPC_Dict* SOPC_NodeId_Dict_Create(bool free_keys, SOPC_Dict_Free_Fct value_free);

void SOPC_ExpandedNodeId_Initialize(SOPC_ExpandedNodeId* expNodeId);
void SOPC_ExpandedNodeId_InitializeAux(void* value);
SOPC_ReturnStatus SOPC_ExpandedNodeId_Copy(SOPC_ExpandedNodeId* dest, const SOPC_ExpandedNodeId* src);
SOPC_ReturnStatus SOPC_ExpandedNodeId_CopyAux(void* dest, const void* src);
SOPC_ReturnStatus SOPC_ExpandedNodeId_Compare(const SOPC_ExpandedNodeId* left,
                                              const SOPC_ExpandedNodeId* right,
                                              int32_t* comparison);
SOPC_ReturnStatus SOPC_ExpandedNodeId_CompareAux(const void* left, const void* right, int32_t* comparison);
void SOPC_ExpandedNodeId_Clear(SOPC_ExpandedNodeId* expNodeId);
void SOPC_ExpandedNodeId_ClearAux(void* value);

void SOPC_StatusCode_Initialize(SOPC_StatusCode* status);
void SOPC_StatusCode_InitializeAux(void* value);
SOPC_ReturnStatus SOPC_StatusCode_CopyAux(void* dest, const void* src);
SOPC_ReturnStatus SOPC_StatusCode_CompareAux(const void* left, const void* right, int32_t* comparison);
void SOPC_StatusCode_Clear(SOPC_StatusCode* status);
void SOPC_StatusCode_ClearAux(void* value);

void SOPC_DiagnosticInfo_Initialize(SOPC_DiagnosticInfo* diagInfo);
void SOPC_DiagnosticInfo_InitializeAux(void* value);
SOPC_ReturnStatus SOPC_DiagnosticInfo_Copy(SOPC_DiagnosticInfo* dest, const SOPC_DiagnosticInfo* src);
SOPC_ReturnStatus SOPC_DiagnosticInfo_CopyAux(void* dest, const void* src);
SOPC_ReturnStatus SOPC_DiagnosticInfo_Compare(const SOPC_DiagnosticInfo* left,
                                              const SOPC_DiagnosticInfo* right,
                                              int32_t* comparison);
SOPC_ReturnStatus SOPC_DiagnosticInfo_CompareAux(const void* left, const void* right, int32_t* comparison);
void SOPC_DiagnosticInfo_Clear(SOPC_DiagnosticInfo* diagInfo);
void SOPC_DiagnosticInfo_ClearAux(void* value);

void SOPC_QualifiedName_Initialize(SOPC_QualifiedName* qname);
void SOPC_QualifiedName_InitializeAux(void* value);
SOPC_ReturnStatus SOPC_QualifiedName_Copy(SOPC_QualifiedName* dest, const SOPC_QualifiedName* src);
SOPC_ReturnStatus SOPC_QualifiedName_CopyAux(void* dest, const void* src);
SOPC_ReturnStatus SOPC_QualifiedName_Compare(const SOPC_QualifiedName* left,
                                             const SOPC_QualifiedName* right,
                                             int32_t* comparison);
SOPC_ReturnStatus SOPC_QualifiedName_CompareAux(const void* left, const void* right, int32_t* comparison);
void SOPC_QualifiedName_Clear(SOPC_QualifiedName* qname);
void SOPC_QualifiedName_ClearAux(void* value);
SOPC_ReturnStatus SOPC_QualifiedName_ParseCString(SOPC_QualifiedName* qname, const char* str);

void SOPC_LocalizedText_Initialize(SOPC_LocalizedText* localizedText);
void SOPC_LocalizedText_InitializeAux(void* value);
SOPC_ReturnStatus SOPC_LocalizedText_Copy(SOPC_LocalizedText* dest, const SOPC_LocalizedText* src);
SOPC_ReturnStatus SOPC_LocalizedText_CopyAux(void* dest, const void* src);
SOPC_ReturnStatus SOPC_LocalizedText_Compare(const SOPC_LocalizedText* left,
                                             const SOPC_LocalizedText* right,
                                             int32_t* comparison);
SOPC_ReturnStatus SOPC_LocalizedText_CompareAux(const void* left, const void* right, int32_t* comparison);
void SOPC_LocalizedText_Clear(SOPC_LocalizedText* localizedText);
void SOPC_LocalizedText_ClearAux(void* value);

void SOPC_ExtensionObject_Initialize(SOPC_ExtensionObject* extObj);
void SOPC_ExtensionObject_InitializeAux(void* value);
SOPC_ReturnStatus SOPC_ExtensionObject_Copy(SOPC_ExtensionObject* dest, const SOPC_ExtensionObject* src);
SOPC_ReturnStatus SOPC_ExtensionObject_CopyAux(void* dest, const void* src);
SOPC_ReturnStatus SOPC_ExtensionObject_Compare(const SOPC_ExtensionObject* left,
                                               const SOPC_ExtensionObject* right,
                                               int32_t* comparison);
SOPC_ReturnStatus SOPC_ExtensionObject_CompareAux(const void* left, const void* right, int32_t* comparison);
void SOPC_ExtensionObject_Clear(SOPC_ExtensionObject* extObj);
void SOPC_ExtensionObject_ClearAux(void* value);

SOPC_Variant* SOPC_Variant_Create(void);
void SOPC_Variant_Initialize(SOPC_Variant* variant);
void SOPC_Variant_InitializeAux(void* value);
SOPC_ReturnStatus SOPC_Variant_Copy(SOPC_Variant* dest, const SOPC_Variant* src);
SOPC_ReturnStatus SOPC_Variant_CopyAux(void* dest, const void* src);
// SOPC_ReturnStatus SOPC_Variant_HasRange(const SOPC_Variant* variant, const SOPC_NumericRange* range, bool*
// has_range); SOPC_ReturnStatus SOPC_Variant_GetRange(SOPC_Variant* dst, const SOPC_Variant* src, const
// SOPC_NumericRange* range); SOPC_ReturnStatus SOPC_Variant_SetRange(SOPC_Variant* variant, const SOPC_Variant* src,
// const SOPC_NumericRange* range);

SOPC_ReturnStatus SOPC_Variant_ShallowCopy(SOPC_Variant* dst, const SOPC_Variant* src);
void SOPC_Variant_Move(SOPC_Variant* dest, SOPC_Variant* src);

SOPC_ReturnStatus SOPC_Variant_Compare(const SOPC_Variant* left, const SOPC_Variant* right, int32_t* comparison);
SOPC_ReturnStatus SOPC_Variant_CompareAux(const void* left, const void* right, int32_t* comparison);
void SOPC_Variant_Clear(SOPC_Variant* variant);
void SOPC_Variant_ClearAux(void* value);
void SOPC_Variant_Delete(SOPC_Variant* variant);

void SOPC_DataValue_Initialize(SOPC_DataValue* dataValue);
void SOPC_DataValue_InitializeAux(void* value);
SOPC_ReturnStatus SOPC_DataValue_Copy(SOPC_DataValue* dest, const SOPC_DataValue* src);
SOPC_ReturnStatus SOPC_DataValue_CopyAux(void* dest, const void* src);
SOPC_ReturnStatus SOPC_DataValue_Compare(const SOPC_DataValue* left, const SOPC_DataValue* right, int32_t* comparison);
SOPC_ReturnStatus SOPC_DataValue_CompareAux(const void* left, const void* right, int32_t* comparison);
void SOPC_DataValue_Clear(SOPC_DataValue* dataValue);
void SOPC_DataValue_ClearAux(void* value);

void SOPC_Initialize_Array(int32_t* noOfElts,
                           void** eltsArray,
                           size_t sizeOfElt,
                           SOPC_EncodeableObject_PfnInitialize* initFct);
SOPC_ReturnStatus SOPC_Op_Array(int32_t noOfElts,
                                void* eltsArrayLeft,
                                void* eltsArrayRight,
                                size_t sizeOfElt,
                                SOPC_EncodeableObject_PfnCopy* opFct);
SOPC_ReturnStatus SOPC_Comp_Array(int32_t noOfElts,
                                  void* eltsArrayLeft,
                                  void* eltsArrayRight,
                                  size_t sizeOfElt,
                                  SOPC_EncodeableObject_PfnComp* compFct,
                                  int32_t* comparison);
void SOPC_Clear_Array(int32_t* noOfElts, void** eltsArray, size_t sizeOfElt, SOPC_EncodeableObject_PfnClear* clearFct);
