/*
 * ua_types.h
 *
 *  Created on: Jul 22, 2016
 *      Author: vincent
 */

#ifndef INGOPCS_SOPC_TYPES_H_
#define INGOPCS_SOPC_TYPES_H_

#include <stdint.h>

#include <ua_stack_csts.h>
#include <ua_encodeable.h>
#include "sopc_base_types.h"

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
    int32_t   Length;
    SOPC_Byte*  Data;
    uint8_t   ClearBytes; // flag indicating if bytes must be freed
} SOPC_ByteString;

// TODO: modify string representation for binary compatibility ?
// Check if internal representation used by SDK
typedef SOPC_ByteString SOPC_String;

typedef SOPC_ByteString SOPC_XmlElement;

typedef int64_t SOPC_DateTime;

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
        uint32_t      Numeric;
        SOPC_String     String;
        SOPC_Guid       Guid;
        SOPC_ByteString Bstring;
    } Data;
} SOPC_NodeId;

typedef struct SOPC_ExpandedNodeId {
    SOPC_NodeId NodeId;
    SOPC_String NamespaceUri;
    uint32_t  ServerIndex;
} SOPC_ExpandedNodeId;

typedef struct SOPC_DiagnosticInfo {
    int32_t                   SymbolicId;
    int32_t                   NamespaceUri;
    int32_t                   Locale;
    int32_t                   LocalizedText;
    SOPC_String                 AdditionalInfo;
    SOPC_StatusCode                InnerStatusCode;
    struct SOPC_DiagnosticInfo* InnerDiagnosticInfo;
} SOPC_DiagnosticInfo;

typedef struct SOPC_QualifiedName {
    uint16_t  NamespaceIndex;
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
    SOPC_NodeId                TypeId;
    SOPC_ExtObjectBodyEncoding Encoding;

    union {
        SOPC_ByteString Bstring;
        SOPC_XmlElement Xml;
        struct {
            void*              Value;
            SOPC_EncodeableType* ObjType;
        } Object;

    } Body;

    int32_t   Length;

} SOPC_ExtensionObject;

typedef enum SOPC_VariantArrayTypeFlag {
    SOPC_VariantArrayMatrixFlag = 64, // 2^6 => bit 6
    SOPC_VariantArrayValueFlag = 128 // 2^7 => bit 7
} SOPC_VariantArrayTypeFlag;

struct SOPC_DataValue;
struct SOPC_Variant;

typedef union SOPC_VariantArrayValue {
    SOPC_Boolean*          BooleanArr;
    SOPC_SByte*            SbyteArr;
    SOPC_Byte*             ByteArr;
    int16_t*             Int16Arr;
    uint16_t*            Uint16Arr;
    int32_t*             Int32Arr;
    uint32_t*            Uint32Arr;
    int64_t*             Int64Arr;
    uint64_t*            Uint64Arr;
    float*               FloatvArr;
    double*              DoublevArr;
    SOPC_String*           StringArr;
    SOPC_DateTime*         DateArr;
    SOPC_Guid*             GuidArr;
    SOPC_ByteString*       BstringArr;
    SOPC_XmlElement*       XmlEltArr;
    SOPC_NodeId*           NodeIdArr;
    SOPC_ExpandedNodeId*   ExpNodeIdArr;
    SOPC_StatusCode*          StatusArr;
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
        int16_t              Int16;
        uint16_t             Uint16;
        int32_t              Int32;
        uint32_t             Uint32;
        int64_t              Int64;
        uint64_t             Uint64;
        float                Floatv;
        double               Doublev;
        SOPC_String            String;
        SOPC_DateTime          Date;
        SOPC_Guid*             Guid;
        SOPC_ByteString        Bstring;
        SOPC_XmlElement        XmlElt;
        SOPC_NodeId*           NodeId;
        SOPC_ExpandedNodeId*   ExpNodeId;
        SOPC_StatusCode           Status;
        SOPC_QualifiedName*    Qname;
        SOPC_LocalizedText*    LocalizedText;
        SOPC_ExtensionObject*  ExtObject;
        struct SOPC_DataValue* DataValue;
        SOPC_DiagnosticInfo*   DiagInfo; // TODO: not present ?
        struct {
            int32_t              Length;
            SOPC_VariantArrayValue Content;
        } Array;
        struct {
            int32_t              Dimensions;
            int32_t*             ArrayDimensions; // Product of dimensions must be <= INT32_MAX ! (binary arrayLength valid for matrix too)
            SOPC_VariantArrayValue Content;
        } Matrix;

} SOPC_VariantValue;

typedef struct SOPC_Variant {
    SOPC_Byte         BuiltInTypeMask;
    SOPC_Byte         ArrayTypeMask;
    SOPC_Byte         Padding[2];
    SOPC_VariantValue Value;
} SOPC_Variant;

typedef struct SOPC_DataValue {
    SOPC_Variant  Value;
    SOPC_StatusCode  Status;
    SOPC_DateTime SourceTimestamp;
    SOPC_DateTime ServerTimestamp;
    uint16_t    SourcePicoSeconds;
    uint16_t    ServerPicoSeconds;
} SOPC_DataValue;

#define SECURITY_POLICY_NONE           "http://opcfoundation.org/UA/SecurityPolicy#None"
#define SECURITY_POLICY_BASIC128RSA15  "http://opcfoundation.org/UA/SecurityPolicy#Basic128Rsa15"
#define SECURITY_POLICY_BASIC256       "http://opcfoundation.org/UA/SecurityPolicy#Basic256"
#define SECURITY_POLICY_BASIC256SHA256 "http://opcfoundation.org/UA/SecurityPolicy#Basic256Sha256"

typedef enum SOPC_SecurityPolicy {
    Security_Policy_None = 0,
    Security_Policy_Basic128Rsa15 = 1,
    Security_Policy_Basic256 = 2,
    Security_Policy_Basic256Sha256 = 3,
} SOPC_SecurityPolicy;

void Boolean_Initialize(SOPC_Boolean* b);
void Boolean_Clear(SOPC_Boolean* b);

void SByte_Initialize(SOPC_SByte* sbyte);
void SByte_Clear(SOPC_SByte* sbyte);

void Byte_Initialize(SOPC_Byte* byte);
void Byte_Clear(SOPC_Byte* byte);

void Int16_Initialize(int16_t* intv);
void Int16_Clear(int16_t* intv);

void UInt16_Initialize(uint16_t* uint);
void UInt16_Clear(uint16_t* uint);

void Int32_Initialize(int32_t* intv);
void Int32_Clear(int32_t* intv);

void UInt32_Initialize(uint32_t* uint);
void UInt32_Clear(uint32_t* uint);

void Int64_Initialize(int64_t* intv);
void Int64_Clear(int64_t* intv);

void UInt64_Initialize(uint64_t* uint);
void UInt64_Clear(uint64_t* uint);

void Float_Initialize(float* f);
void Float_Clear(float* f);

void Double_Initialize(double* d);
void Double_Clear(double* d);

void ByteString_Initialize(SOPC_ByteString* bstring);
SOPC_ByteString* ByteString_Create(void);
SOPC_StatusCode ByteString_InitializeFixedSize(SOPC_ByteString* bstring, uint32_t size);
SOPC_StatusCode ByteString_AttachFromBytes(SOPC_ByteString* dest, SOPC_Byte* bytes, int32_t length);
SOPC_StatusCode ByteString_AttachFrom(SOPC_ByteString* dest, SOPC_ByteString* src);
SOPC_StatusCode ByteString_Copy(SOPC_ByteString* dest, const SOPC_ByteString* src);
void ByteString_Clear(SOPC_ByteString* bstring);
void ByteString_Delete(SOPC_ByteString* bstring);

SOPC_StatusCode ByteString_Compare(const SOPC_ByteString* left,
                              const SOPC_ByteString* right,
                              int32_t*             comparison);

// Returns 0 if false
uint32_t ByteString_Equal(const SOPC_ByteString* left,
                          const SOPC_ByteString* right);

void String_Initialize(SOPC_String* string);
SOPC_String* String_Create(void);
SOPC_StatusCode String_CopyFromCString(SOPC_String* string, const char* cString);
SOPC_StatusCode String_InitializeFromCString(SOPC_String* string, const char* cString);
char* String_GetCString(const SOPC_String* string); // Copy
const char* String_GetRawCString(const SOPC_String* string); // Pointer to string

SOPC_StatusCode String_AttachFrom(SOPC_String* dest, SOPC_String* src);
SOPC_StatusCode String_AttachFromCstring(SOPC_String* dest, char* src);

SOPC_StatusCode String_Copy(SOPC_String* dest, const SOPC_String* src);
void String_Clear(SOPC_String* bstring);
void String_Delete(SOPC_String* bstring);

SOPC_StatusCode String_Compare(const SOPC_String* left,
                          const SOPC_String* right,
                          int32_t*         comparison);

uint32_t String_Equal(const SOPC_String* left,
                      const SOPC_String* right);

void XmlElement_Initialize(SOPC_XmlElement* xmlElt);
void XmlElement_Clear(SOPC_XmlElement* xmlElt);

void DateTime_Initialize(SOPC_DateTime* dateTime);
void DateTime_Clear(SOPC_DateTime* dateTime);

void Guid_Initialize(SOPC_Guid* guid);
void Guid_Clear(SOPC_Guid* guid);

void NodeId_Initialize(SOPC_NodeId* nodeId);
void NodeId_InitType(SOPC_NodeId* nodeId, SOPC_IdentifierType knownIdType);
void NodeId_Clear(SOPC_NodeId* nodeId);

void ExpandedNodeId_Initialize(SOPC_ExpandedNodeId* expNodeId);
void ExpandedNodeId_Clear(SOPC_ExpandedNodeId* expNodeId);

void StatusCode_Initialize(SOPC_StatusCode* status);
void StatusCode_Clear(SOPC_StatusCode* status);

void DiagnosticInfo_Initialize(SOPC_DiagnosticInfo* diagInfo);
void DiagnosticInfo_Clear(SOPC_DiagnosticInfo* diagInfo);

void QualifiedName_Initialize(SOPC_QualifiedName* qname);
void QualifiedName_Clear(SOPC_QualifiedName* qname);

void LocalizedText_Initialize(SOPC_LocalizedText* localizedText);
void LocalizedText_Clear(SOPC_LocalizedText* localizedText);

void ExtensionObject_Initialize(SOPC_ExtensionObject* extObj);
void ExtensionObject_Clear(SOPC_ExtensionObject* extObj);

void Variant_Initialize(SOPC_Variant* variant);
void Variant_Clear(SOPC_Variant* variant);

void DataValue_Initialize(SOPC_DataValue* dataValue);
void DataValue_Clear(SOPC_DataValue* dataValue);

END_EXTERN_C

#endif /* INGOPCS_SOPC_TYPES_H_ */
