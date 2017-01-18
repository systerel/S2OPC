/*
 *  Copyright (C) 2016 Systerel and others.
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

#ifndef SOPC_BUILTINTYPES_H_
#define SOPC_BUILTINTYPES_H_

#include <stdint.h>

#include "sopc_base_types.h"
#include "sopc_encodeabletype.h"
#include "sopc_stack_csts.h"

BEGIN_EXTERN_C

typedef enum SOPC_BuiltinId {
    SOPC_Null_Id            = 0,
    SOPC_Boolean_Id         = 1,
    SOPC_SByte_Id           = 2,
    SOPC_Byte_Id            = 3,
    SOPC_Int16_Id           = 4,
    SOPC_UInt16_Id          = 5,
    SOPC_Int32_Id           = 6,
    SOPC_UInt32_Id          = 7,
    SOPC_Int64_Id           = 8,
    SOPC_UInt64_Id          = 9,
    SOPC_Float_Id           = 10,
    SOPC_Double_Id          = 11,
    SOPC_String_Id          = 12,
    SOPC_DateTime_Id        = 13,
    SOPC_Guid_Id            = 14,
    SOPC_ByteString_Id      = 15,
    SOPC_XmlElement_Id      = 16,
    SOPC_NodeId_Id          = 17,
    SOPC_ExpandedNodeId_Id  = 18,
    SOPC_StatusCode_Id      = 19,
    SOPC_QualifiedName_Id   = 20,
    SOPC_LocalizedText_Id   = 21,
    SOPC_ExtensionObject_Id = 22,
    SOPC_DataValue_Id       = 23,
    SOPC_Variant_Id         = 24,
    SOPC_DiagnosticInfo_Id  = 25
} SOPC_BuiltinId;
#define SOPC_BUILTINID_MAX 25

typedef uint8_t SOPC_Byte;

typedef SOPC_Byte SOPC_Boolean;

typedef int8_t SOPC_SByte;

typedef struct SOPC_ByteString {
    int32_t    Length;
    SOPC_Byte* Data;
} SOPC_ByteString;

typedef SOPC_ByteString SOPC_XmlElement;

typedef struct SOPC_String {
    int32_t    Length;
    uint8_t    DoNotClear; // flag indicating if bytes must be freed
    SOPC_Byte* Data;
} SOPC_String;

typedef struct SOPC_DateTime {
    uint32_t Low32;
    uint32_t High32;
} SOPC_DateTime;

typedef struct SOPC_Guid {
    uint32_t Data1;
    uint16_t Data2;
    uint16_t Data3;
    SOPC_Byte  Data4[8];
} SOPC_Guid;

typedef enum SOPC_IdentifierType {
    IdentifierType_Numeric = 0x00,
    IdentifierType_String = 0x01,
    IdentifierType_Guid = 0x02,
    IdentifierType_ByteString = 0x03,
} SOPC_IdentifierType;

typedef struct SOPC_NodeId {
    uint16_t IdentifierType; // SOPC_IdentifierType
    uint16_t Namespace;

    union {
        uint32_t        Numeric;
        SOPC_String     String;
        SOPC_Guid*      Guid;
        SOPC_ByteString Bstring;
    } Data;
} SOPC_NodeId;

typedef struct SOPC_ExpandedNodeId {
    SOPC_NodeId NodeId;
    SOPC_String NamespaceUri;
    uint32_t  ServerIndex;
} SOPC_ExpandedNodeId;

typedef struct SOPC_DiagnosticInfo {
    int32_t                     SymbolicId;
    int32_t                     NamespaceUri;
    int32_t                     Locale;
    int32_t                     LocalizedText;
    SOPC_String                 AdditionalInfo;
    SOPC_StatusCode             InnerStatusCode;
    struct SOPC_DiagnosticInfo* InnerDiagnosticInfo;
} SOPC_DiagnosticInfo;

typedef struct SOPC_QualifiedName {
    uint16_t    NamespaceIndex;
    SOPC_Byte   Padding[2]; // For type binary compatibility
    SOPC_String Name;
} SOPC_QualifiedName;

typedef struct SOPC_LocalizedText {
    SOPC_String Locale;
    SOPC_String Text;
} SOPC_LocalizedText;

typedef enum SOPC_ExtObjectBodyEncoding {
    SOPC_ExtObjBodyEncoding_None = 0x00,
    SOPC_ExtObjBodyEncoding_ByteString = 0x01,
    SOPC_ExtObjBodyEncoding_XMLElement = 0x02,
    SOPC_ExtObjBodyEncoding_Object     = 0x03
} SOPC_ExtObjectBodyEncoding;

typedef struct SOPC_ExtensionObject {
    SOPC_ExpandedNodeId        TypeId;
    SOPC_ExtObjectBodyEncoding Encoding;

    union {
        SOPC_ByteString Bstring;
        SOPC_XmlElement Xml;
        struct {
            void*                Value;
            SOPC_EncodeableType* ObjType;
        } Object;

    } Body;

    int32_t   Length;

} SOPC_ExtensionObject;

typedef enum SOPC_VariantArrayTypeFlag {
    SOPC_VariantArrayMatrixFlag = 64, // 2^6 => bit 6
    SOPC_VariantArrayValueFlag = 128 // 2^7 => bit 7
} SOPC_VariantArrayTypeFlag;

// Binary compatible types
typedef enum SOPC_VariantArrayType {
    SOPC_VariantArrayType_SingleValue = 0x0,
    SOPC_VariantArrayType_Array = 0x1,
    SOPC_VariantArrayType_Matrix = 0x2
} SOPC_VariantArrayType;

struct SOPC_DataValue;
struct SOPC_Variant;

typedef union SOPC_VariantArrayValue {
    SOPC_Boolean*          BooleanArr;
    SOPC_SByte*            SbyteArr;
    SOPC_Byte*             ByteArr;
    int16_t*               Int16Arr;
    uint16_t*              Uint16Arr;
    int32_t*               Int32Arr;
    uint32_t*              Uint32Arr;
    int64_t*               Int64Arr;
    uint64_t*              Uint64Arr;
    float*                 FloatvArr;
    double*                DoublevArr;
    SOPC_String*           StringArr;
    SOPC_DateTime*         DateArr;
    SOPC_Guid*             GuidArr;
    SOPC_ByteString*       BstringArr;
    SOPC_XmlElement*       XmlEltArr;
    SOPC_NodeId*           NodeIdArr;
    SOPC_ExpandedNodeId*   ExpNodeIdArr;
    SOPC_StatusCode*       StatusArr;
    SOPC_QualifiedName*    QnameArr;
    SOPC_LocalizedText*    LocalizedTextArr;
    SOPC_ExtensionObject*  ExtObjectArr;
    struct SOPC_DataValue* DataValueArr;
    struct SOPC_Variant*   VariantArr;
    SOPC_DiagnosticInfo*   DiagInfoArr; // TODO: not present ?
} SOPC_VariantArrayValue;

typedef union SOPC_VariantValue {
        SOPC_Boolean           Boolean;
        SOPC_SByte             Sbyte;
        SOPC_Byte              Byte;
        int16_t                Int16;
        uint16_t               Uint16;
        int32_t                Int32;
        uint32_t               Uint32;
        int64_t                Int64;
        uint64_t               Uint64;
        float                  Floatv;
        double                 Doublev;
        SOPC_String            String;
        SOPC_DateTime          Date;
        SOPC_Guid*             Guid;
        SOPC_ByteString        Bstring;
        SOPC_XmlElement        XmlElt;
        SOPC_NodeId*           NodeId;
        SOPC_ExpandedNodeId*   ExpNodeId;
        SOPC_StatusCode        Status;
        SOPC_QualifiedName*    Qname;
        SOPC_LocalizedText*    LocalizedText;
        SOPC_ExtensionObject*  ExtObject;
        struct SOPC_DataValue* DataValue;
        SOPC_DiagnosticInfo*   DiagInfo; // TODO: not present ?
        struct {
            int32_t                Length;
            SOPC_VariantArrayValue Content;
        } Array;
        struct {
            int32_t                Dimensions;
            int32_t*               ArrayDimensions; // Product of dimensions must be <= INT32_MAX ! (binary arrayLength valid for matrix too)
            SOPC_VariantArrayValue Content;
        } Matrix;

} SOPC_VariantValue;

typedef struct SOPC_Variant {
    SOPC_Byte         BuiltInTypeId;
    SOPC_Byte         ArrayType;
    SOPC_Byte         Padding[2];
    SOPC_VariantValue Value;
} SOPC_Variant;

typedef struct SOPC_DataValue {
    SOPC_Variant    Value;
    SOPC_StatusCode Status;
    SOPC_DateTime   SourceTimestamp;
    SOPC_DateTime   ServerTimestamp;
    uint16_t        SourcePicoSeconds;
    uint16_t        ServerPicoSeconds;
} SOPC_DataValue;

#define SECURITY_POLICY_NONE           "http://opcfoundation.org/UA/SecurityPolicy#None"
#define SECURITY_POLICY_BASIC128RSA15  "http://opcfoundation.org/UA/SecurityPolicy#Basic128Rsa15"
#define SECURITY_POLICY_BASIC256       "http://opcfoundation.org/UA/SecurityPolicy#Basic256"
#define SECURITY_POLICY_BASIC256SHA256 "http://opcfoundation.org/UA/SecurityPolicy#Basic256Sha256"

void SOPC_Boolean_Initialize(SOPC_Boolean* b);
void SOPC_Boolean_InitializeAux(void* value);
void SOPC_Boolean_Clear(SOPC_Boolean* b);
void SOPC_Boolean_ClearAux(void* value);

void SOPC_SByte_Initialize(SOPC_SByte* sbyte);
void SOPC_SByte_InitializeAux(void* value);
void SOPC_SByte_Clear(SOPC_SByte* sbyte);
void SOPC_SByte_ClearAux(void* value);

void SOPC_Byte_Initialize(SOPC_Byte* byte);
void SOPC_Byte_InitializeAux(void* value);
void SOPC_Byte_Clear(SOPC_Byte* byte);
void SOPC_Byte_ClearAux(void* value);

void SOPC_Int16_Initialize(int16_t* intv);
void SOPC_Int16_InitializeAux(void* value);
void SOPC_Int16_Clear(int16_t* intv);
void SOPC_Int16_ClearAux(void* value);

void SOPC_UInt16_Initialize(uint16_t* uint);
void SOPC_UInt16_InitializeAux(void* value);
void SOPC_UInt16_Clear(uint16_t* uint);
void SOPC_UInt16_ClearAux(void* value);

void SOPC_Int32_Initialize(int32_t* intv);
void SOPC_Int32_InitializeAux(void* value);
void SOPC_Int32_Clear(int32_t* intv);
void SOPC_Int32_ClearAux(void* value);

void SOPC_UInt32_Initialize(uint32_t* uint);
void SOPC_UInt32_InitializeAux(void* value);
void SOPC_UInt32_Clear(uint32_t* uint);
void SOPC_UInt32_ClearAux(void* value);

void SOPC_Int64_Initialize(int64_t* intv);
void SOPC_Int64_InitializeAux(void* value);
void SOPC_Int64_Clear(int64_t* intv);
void SOPC_Int64_ClearAux(void* value);

void SOPC_UInt64_Initialize(uint64_t* uint);
void SOPC_UInt64_InitializeAux(void* value);
void SOPC_UInt64_Clear(uint64_t* uint);
void SOPC_UInt64_ClearAux(void* value);

void SOPC_Float_Initialize(float* f);
void SOPC_Float_InitializeAux(void* value);
void SOPC_Float_Clear(float* f);
void SOPC_Float_ClearAux(void* value);

void SOPC_Double_Initialize(double* d);
void SOPC_Double_InitializeAux(void* value);
void SOPC_Double_Clear(double* d);
void SOPC_Double_ClearAux(void* value);

void SOPC_ByteString_Initialize(SOPC_ByteString* bstring);
void SOPC_ByteString_InitializeAux(void* value);
SOPC_ByteString* SOPC_ByteString_Create(void);
SOPC_StatusCode SOPC_ByteString_InitializeFixedSize(SOPC_ByteString* bstring, uint32_t size);
SOPC_StatusCode SOPC_ByteString_CopyFromBytes(SOPC_ByteString* dest, SOPC_Byte* bytes, int32_t length);
SOPC_StatusCode SOPC_ByteString_Copy(SOPC_ByteString* dest, const SOPC_ByteString* src);
void SOPC_ByteString_Clear(SOPC_ByteString* bstring);
void SOPC_ByteString_ClearAux(void* value);
void SOPC_ByteString_Delete(SOPC_ByteString* bstring);

SOPC_StatusCode SOPC_ByteString_Compare(const SOPC_ByteString* left,
                                   const SOPC_ByteString* right,
                                   int32_t*               comparison);

// Returns 0 if false
uint8_t SOPC_ByteString_Equal(const SOPC_ByteString* left,
                              const SOPC_ByteString* right);

void SOPC_String_Initialize(SOPC_String* string);
void SOPC_String_InitializeAux(void* value);
SOPC_String* SOPC_String_Create(void);
SOPC_StatusCode SOPC_String_CopyFromCString(SOPC_String* string, const char* cString);
SOPC_StatusCode SOPC_String_InitializeFromCString(SOPC_String* string, const char* cString);
char* SOPC_String_GetCString(const SOPC_String* string); // Copy
const char* SOPC_String_GetRawCString(const SOPC_String* string); // Pointer to string

SOPC_StatusCode SOPC_String_AttachFrom(SOPC_String* dest, SOPC_String* src);
SOPC_StatusCode SOPC_String_AttachFromCstring(SOPC_String* dest, char* src);

SOPC_StatusCode SOPC_String_Copy(SOPC_String* dest, const SOPC_String* src);
void SOPC_String_Clear(SOPC_String* bstring);
void SOPC_String_ClearAux(void* value);
void SOPC_String_Delete(SOPC_String* bstring);

SOPC_StatusCode SOPC_String_Compare(const SOPC_String* left,
                                    const SOPC_String* right,
                                    uint8_t            ignoreCase,
                                    int32_t*           comparison);

uint32_t SOPC_String_Equal(const SOPC_String* left,
                      const SOPC_String* right);

void SOPC_XmlElement_Initialize(SOPC_XmlElement* xmlElt);
void SOPC_XmlElement_InitializeAux(void* value);
void SOPC_XmlElement_Clear(SOPC_XmlElement* xmlElt);
void SOPC_XmlElement_ClearAux(void* value);

void SOPC_DateTime_Initialize(SOPC_DateTime* dateTime);
void SOPC_DateTime_InitializeAux(void* value);
void SOPC_DateTime_Clear(SOPC_DateTime* dateTime);
void SOPC_DateTime_ClearAux(void* value);
int64_t SOPC_DateTime_ToInt64(const SOPC_DateTime* dateTime);
void SOPC_DateTime_FromInt64(SOPC_DateTime* dateTime, int64_t date);

void SOPC_Guid_Initialize(SOPC_Guid* guid);
void SOPC_Guid_InitializeAux(void* value);
void SOPC_Guid_Clear(SOPC_Guid* guid);
void SOPC_Guid_ClearAux(void* value);

void SOPC_NodeId_Initialize(SOPC_NodeId* nodeId);
void SOPC_NodeId_InitializeAux(void* value);
void SOPC_NodeId_InitType(SOPC_NodeId* nodeId, SOPC_IdentifierType knownIdType);
void SOPC_NodeId_Clear(SOPC_NodeId* nodeId);
void SOPC_NodeId_ClearAux(void* value);

void SOPC_ExpandedNodeId_Initialize(SOPC_ExpandedNodeId* expNodeId);
void SOPC_ExpandedNodeId_InitializeAux(void* value);
void SOPC_ExpandedNodeId_Clear(SOPC_ExpandedNodeId* expNodeId);
void SOPC_ExpandedNodeId_ClearAux(void* value);

void SOPC_StatusCode_Initialize(SOPC_StatusCode* status);
void SOPC_StatusCode_InitializeAux(void* value);
void SOPC_StatusCode_Clear(SOPC_StatusCode* status);
void SOPC_StatusCode_ClearAux(void* value);

void SOPC_DiagnosticInfo_Initialize(SOPC_DiagnosticInfo* diagInfo);
void SOPC_DiagnosticInfo_InitializeAux(void* value);
void SOPC_DiagnosticInfo_Clear(SOPC_DiagnosticInfo* diagInfo);
void SOPC_DiagnosticInfo_ClearAux(void* value);

void SOPC_QualifiedName_Initialize(SOPC_QualifiedName* qname);
void SOPC_QualifiedName_InitializeAux(void* value);
void SOPC_QualifiedName_Clear(SOPC_QualifiedName* qname);
void SOPC_QualifiedName_ClearAux(void* value);

void SOPC_LocalizedText_Initialize(SOPC_LocalizedText* localizedText);
void SOPC_LocalizedText_InitializeAux(void* value);
void SOPC_LocalizedText_Clear(SOPC_LocalizedText* localizedText);
void SOPC_LocalizedText_ClearAux(void* value);

void SOPC_ExtensionObject_Initialize(SOPC_ExtensionObject* extObj);
void SOPC_ExtensionObject_InitializeAux(void* value);
void SOPC_ExtensionObject_Clear(SOPC_ExtensionObject* extObj);
void SOPC_ExtensionObject_ClearAux(void* value);

void SOPC_Variant_Initialize(SOPC_Variant* variant);
void SOPC_Variant_InitializeAux(void* value);
void SOPC_Variant_Clear(SOPC_Variant* variant);
void SOPC_Variant_ClearAux(void* value);

void SOPC_DataValue_Initialize(SOPC_DataValue* dataValue);
void SOPC_DataValue_InitializeAux(void* value);
void SOPC_DataValue_Clear(SOPC_DataValue* dataValue);
void SOPC_DataValue_ClearAux(void* value);

void SOPC_Initialize_Array(int32_t* noOfElts, void** eltsArray, size_t sizeOfElt,
                           SOPC_EncodeableObject_PfnInitialize* initFct);
void SOPC_Clear_Array(int32_t* noOfElts, void** eltsArray, size_t sizeOfElt,
                      SOPC_EncodeableObject_PfnClear* clearFct);

END_EXTERN_C

#endif /* SOPC_SOPC_BUILTINTYPES_H_ */
