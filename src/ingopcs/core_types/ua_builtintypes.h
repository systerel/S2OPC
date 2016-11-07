/*
 * ua_types.h
 *
 *  Created on: Jul 22, 2016
 *      Author: vincent
 */

#ifndef INGOPCS_UA_TYPES_H_
#define INGOPCS_UA_TYPES_H_

#include <stdint.h>

#include <ua_stack_csts.h>
#include <ua_base_types.h>
#include <ua_encodeable.h>

BEGIN_EXTERN_C

typedef enum UA_BuiltinId {
    UA_Null_Id            = 0,
    UA_Boolean_Id         = 1,
    UA_SByte_Id           = 2,
    UA_Byte_Id            = 3,
    UA_Int16_Id           = 4,
    UA_UInt16_Id          = 5,
    UA_Int32_Id           = 6,
    UA_UInt32_Id          = 7,
    UA_Int64_Id           = 8,
    UA_UInt64_Id          = 9,
    UA_Float_Id           = 10,
    UA_Double_Id          = 11,
    UA_String_Id          = 12,
    UA_DateTime_Id        = 13,
    UA_Guid_Id            = 14,
    UA_ByteString_Id      = 15,
    UA_XmlElement_Id      = 16,
    UA_NodeId_Id          = 17,
    UA_ExpandedNodeId_Id  = 18,
    UA_StatusCode_Id      = 19,
    UA_QualifiedName_Id   = 20,
    UA_LocalizedText_Id   = 21,
    UA_ExtensionObject_Id = 22,
    UA_DataValue_Id       = 23,
    UA_Variant_Id         = 24,
    UA_DiagnosticInfo_Id  = 25
} UA_BuiltinId;
#define UA_BUILTINID_MAX 25

typedef uint8_t UA_Byte;

typedef UA_Byte UA_Boolean;

typedef int8_t UA_SByte;

typedef struct UA_ByteString {
    int32_t   Length;
    UA_Byte*  Data;
    uint8_t   ClearBytes; // flag indicating if bytes must be freed
} UA_ByteString;

// TODO: modify string representation for binary compatibility ?
// Check if internal representation used by SDK
typedef UA_ByteString UA_String;

typedef UA_ByteString UA_XmlElement;

typedef int64_t UA_DateTime;

typedef struct UA_Guid {
    uint32_t Data1;
    uint16_t Data2;
    uint16_t Data3;
    UA_Byte  Data4[8];
} UA_Guid;

typedef enum UA_IdentifierType {
    IdentifierType_Numeric = 0x00,
    IdentifierType_String = 0x01,
    IdentifierType_Guid = 0x02,
    IdentifierType_ByteString = 0x03,
} UA_IdentifierType;

typedef struct UA_NodeId {
    uint16_t IdentifierType; // UA_IdentifierType
    uint16_t Namespace;

    union {
        uint32_t      Numeric;
        UA_String     String;
        UA_Guid       Guid;
        UA_ByteString Bstring;
    } Data;
} UA_NodeId;

typedef struct UA_ExpandedNodeId {
    UA_NodeId NodeId;
    UA_String NamespaceUri;
    uint32_t  ServerIndex;
} UA_ExpandedNodeId;

typedef struct UA_DiagnosticInfo {
    int32_t                   SymbolicId;
    int32_t                   NamespaceUri;
    int32_t                   Locale;
    int32_t                   LocalizedText;
    UA_String                 AdditionalInfo;
    StatusCode                InnerStatusCode;
    struct UA_DiagnosticInfo* InnerDiagnosticInfo;
} UA_DiagnosticInfo;

typedef struct UA_QualifiedName {
    uint16_t  NamespaceIndex;
    UA_Byte   Padding[2]; // For type binary compatibility
    UA_String Name;
} UA_QualifiedName;

typedef struct UA_LocalizedText {
    UA_String Locale;
    UA_String Text;
} UA_LocalizedText;

typedef enum UA_ExtObjectBodyEncoding {
    UA_ExtObjBodyEncoding_None = 0x00,
    UA_ExtObjBodyEncoding_ByteString = 0x01,
    UA_ExtObjBodyEncoding_XMLElement = 0x02,
    UA_ExtObjBodyEncoding_Object     = 0x03
} UA_ExtObjectBodyEncoding;

typedef struct UA_ExtensionObject {
    UA_NodeId                TypeId;
    UA_ExtObjectBodyEncoding Encoding;

    union {
        UA_ByteString Bstring;
        UA_XmlElement Xml;
        struct {
            void*              Value;
            UA_EncodeableType* ObjType;
        } Object;

    } Body;

    int32_t   Length;

} UA_ExtensionObject;

typedef enum UA_VariantArrayTypeFlag {
    UA_VariantArrayMatrixFlag = 64, // 2^6 => bit 6
    UA_VariantArrayValueFlag = 128 // 2^7 => bit 7
} UA_VariantArrayTypeFlag;

struct UA_DataValue;
struct UA_Variant;

typedef union UA_VariantArrayValue {
    UA_Boolean*          BooleanArr;
    UA_SByte*            SbyteArr;
    UA_Byte*             ByteArr;
    int16_t*             Int16Arr;
    uint16_t*            Uint16Arr;
    int32_t*             Int32Arr;
    uint32_t*            Uint32Arr;
    int64_t*             Int64Arr;
    uint64_t*            Uint64Arr;
    float*               FloatvArr;
    double*              DoublevArr;
    UA_String*           StringArr;
    UA_DateTime*         DateArr;
    UA_Guid*             GuidArr;
    UA_ByteString*       BstringArr;
    UA_XmlElement*       XmlEltArr;
    UA_NodeId*           NodeIdArr;
    UA_ExpandedNodeId*   ExpNodeIdArr;
    StatusCode*          StatusArr;
    UA_QualifiedName*    QnameArr;
    UA_LocalizedText*    LocalizedTextArr;
    UA_ExtensionObject*  ExtObjectArr;
    struct UA_DataValue* DataValueArr;
    struct UA_Variant*   VariantArr;
    UA_DiagnosticInfo*   DiagInfoArr; // TODO: not present ?
} UA_VariantArrayValue;

typedef union UA_VariantValue {
        UA_Boolean           Boolean;
        UA_SByte             Sbyte;
        UA_Byte              Byte;
        int16_t              Int16;
        uint16_t             Uint16;
        int32_t              Int32;
        uint32_t             Uint32;
        int64_t              Int64;
        uint64_t             Uint64;
        float                Floatv;
        double               Doublev;
        UA_String            String;
        UA_DateTime          Date;
        UA_Guid*             Guid;
        UA_ByteString        Bstring;
        UA_XmlElement        XmlElt;
        UA_NodeId*           NodeId;
        UA_ExpandedNodeId*   ExpNodeId;
        StatusCode           Status;
        UA_QualifiedName*    Qname;
        UA_LocalizedText*    LocalizedText;
        UA_ExtensionObject*  ExtObject;
        struct UA_DataValue* DataValue;
        UA_DiagnosticInfo*   DiagInfo; // TODO: not present ?
        struct {
            int32_t              Length;
            UA_VariantArrayValue Content;
        } Array;
        struct {
            int32_t              Dimensions;
            int32_t*             ArrayDimensions; // Product of dimensions must be <= INT32_MAX ! (binary arrayLength valid for matrix too)
            UA_VariantArrayValue Content;
        } Matrix;

} UA_VariantValue;

typedef struct UA_Variant {
    UA_Byte         BuiltInTypeMask;
    UA_Byte         ArrayTypeMask;
    UA_Byte         Padding[2];
    UA_VariantValue Value;
} UA_Variant;

typedef struct UA_DataValue {
    UA_Variant  Value;
    StatusCode  Status;
    UA_DateTime SourceTimestamp;
    UA_DateTime ServerTimestamp;
    uint16_t    SourcePicoSeconds;
    uint16_t    ServerPicoSeconds;
} UA_DataValue;

#define SECURITY_POLICY_NONE           "http://opcfoundation.org/UA/SecurityPolicy#None"
#define SECURITY_POLICY_BASIC128RSA15  "http://opcfoundation.org/UA/SecurityPolicy#Basic128Rsa15"
#define SECURITY_POLICY_BASIC256       "http://opcfoundation.org/UA/SecurityPolicy#Basic256"
#define SECURITY_POLICY_BASIC256SHA256 "http://opcfoundation.org/UA/SecurityPolicy#Basic256Sha256"

typedef enum UA_SecurityPolicy {
    Security_Policy_None = 0,
    Security_Policy_Basic128Rsa15 = 1,
    Security_Policy_Basic256 = 2,
    Security_Policy_Basic256Sha256 = 3,
} UA_SecurityPolicy;

void Boolean_Initialize(UA_Boolean* b);
void Boolean_Clear(UA_Boolean* b);

void SByte_Initialize(UA_SByte* sbyte);
void SByte_Clear(UA_SByte* sbyte);

void Byte_Initialize(UA_Byte* byte);
void Byte_Clear(UA_Byte* byte);

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

void ByteString_Initialize(UA_ByteString* bstring);
UA_ByteString* ByteString_Create(void);
StatusCode ByteString_InitializeFixedSize(UA_ByteString* bstring, uint32_t size);
StatusCode ByteString_AttachFromBytes(UA_ByteString* dest, UA_Byte* bytes, int32_t length);
StatusCode ByteString_AttachFrom(UA_ByteString* dest, UA_ByteString* src);
StatusCode ByteString_Copy(UA_ByteString* dest, const UA_ByteString* src);
void ByteString_Clear(UA_ByteString* bstring);
void ByteString_Delete(UA_ByteString* bstring);

StatusCode ByteString_Compare(const UA_ByteString* left,
                              const UA_ByteString* right,
                              int32_t*             comparison);

// Returns 0 if false
uint32_t ByteString_Equal(const UA_ByteString* left,
                          const UA_ByteString* right);

void String_Initialize(UA_String* string);
UA_String* String_Create(void);
StatusCode String_CopyFromCString(UA_String* string, const char* cString);
StatusCode String_InitializeFromCString(UA_String* string, const char* cString);
char* String_GetCString(const UA_String* string); // Copy
const char* String_GetRawCString(const UA_String* string); // Pointer to string

StatusCode String_AttachFrom(UA_String* dest, UA_String* src);
StatusCode String_AttachFromCstring(UA_String* dest, char* src);

StatusCode String_Copy(UA_String* dest, const UA_String* src);
void String_Clear(UA_String* bstring);
void String_Delete(UA_String* bstring);

StatusCode String_Compare(const UA_String* left,
                          const UA_String* right,
                          int32_t*         comparison);

uint32_t String_Equal(const UA_String* left,
                      const UA_String* right);

void XmlElement_Initialize(UA_XmlElement* xmlElt);
void XmlElement_Clear(UA_XmlElement* xmlElt);

void DateTime_Initialize(UA_DateTime* dateTime);
void DateTime_Clear(UA_DateTime* dateTime);

void Guid_Initialize(UA_Guid* guid);
void Guid_Clear(UA_Guid* guid);

void NodeId_Initialize(UA_NodeId* nodeId);
void NodeId_InitType(UA_NodeId* nodeId, UA_IdentifierType knownIdType);
void NodeId_Clear(UA_NodeId* nodeId);

void ExpandedNodeId_Initialize(UA_ExpandedNodeId* expNodeId);
void ExpandedNodeId_Clear(UA_ExpandedNodeId* expNodeId);

void StatusCode_Initialize(StatusCode* status);
void StatusCode_Clear(StatusCode* status);

void DiagnosticInfo_Initialize(UA_DiagnosticInfo* diagInfo);
void DiagnosticInfo_Clear(UA_DiagnosticInfo* diagInfo);

void QualifiedName_Initialize(UA_QualifiedName* qname);
void QualifiedName_Clear(UA_QualifiedName* qname);

void LocalizedText_Initialize(UA_LocalizedText* localizedText);
void LocalizedText_Clear(UA_LocalizedText* localizedText);

void ExtensionObject_Initialize(UA_ExtensionObject* extObj);
void ExtensionObject_Clear(UA_ExtensionObject* extObj);

void Variant_Initialize(UA_Variant* variant);
void Variant_Clear(UA_Variant* variant);

void DataValue_Initialize(UA_DataValue* dataValue);
void DataValue_Clear(UA_DataValue* dataValue);

END_EXTERN_C

#endif /* INGOPCS_UA_TYPES_H_ */
