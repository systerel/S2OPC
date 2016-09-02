/*
 * ua_types.h
 *
 *  Created on: Jul 22, 2016
 *      Author: vincent
 */

#ifndef INGOPCS_UA_TYPES_H_
#define INGOPCS_UA_TYPES_H_

#include <stdint.h>

#define UA_NULL ((void *)0)

typedef enum {
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

typedef uint8_t UA_Byte;

typedef UA_Byte UA_Boolean;
#define UA_FALSE 0

typedef int8_t UA_SByte;

typedef struct {
    int32_t   length;
    UA_Byte*  characters;
} UA_ByteString;

// TODO: modify string representation for binary compatibility ?
// Check if internal representation used by SDK
typedef UA_ByteString UA_String;

typedef UA_ByteString UA_XmlElement;

typedef int64_t UA_DateTime;

typedef struct {
    uint32_t data1;
    uint16_t data2;
    uint16_t data3;
    UA_Byte  data4[8];
} UA_Guid;

typedef enum {
    IdentifierType_Numeric = 0x00,
    IdentifierType_String = 0x01,
    IdentifierType_Guid = 0x02,
    IdentifierType_ByteString = 0x03,
    IdentifierType_Undefined = 0xFF
} UA_IdentifierType;

typedef struct {
    uint16_t identifierType; // UA_IdentifierType
    uint16_t namespace;

    union {
        uint32_t      numeric;
        UA_String     string;
        UA_Guid       guid;
        UA_ByteString bstring;
    };
} UA_NodeId;

typedef struct {
    UA_NodeId nodeId;
    UA_String namespaceUri;
    uint32_t  serverIndex;
} UA_ExpandedNodeId;

typedef uint32_t StatusCode;
#define STATUS_OK 0x0 // TODO: change values
#define STATUS_OK_INCOMPLETE 0x00000001
#define STATUS_NOK 0x80000000//0x10000000
#define STATUS_INVALID_PARAMETERS 0x80760001//0x20000000
#define STATUS_INVALID_STATE 0x80760002//0x30000000
#define STATUS_INVALID_RCV_PARAMETER 0x80000003//0x40000000

typedef struct UA_DiagnosticInfo {
    int32_t                   symbolicId;
    int32_t                   namespaceUri;
    int32_t                   locale;
    int32_t                   localizedText;
    UA_String                 additionalInfo;
    StatusCode                innerStatusCode;
    struct UA_DiagnosticInfo* innerDiagnosticInfo;
} UA_DiagnosticInfo;

typedef struct {
    uint16_t  namespaceIndex;
    UA_Byte   padding[2]; // For type binary compatibility
    UA_String name;
} UA_QualifiedName;

typedef struct {
    UA_String locale;
    UA_String text;
} UA_LocalizedText;

// TODO: TBD
typedef struct {} UA_EncodeableType;

typedef enum {
    UA_ExtObjBodyEncoding_None = 0x00,
    UA_ExtObjBodyEncoding_ByteString = 0x01,
    UA_ExtObjBodyEncoding_XMLElement = 0x02,
    UA_ExtObjBodyEncoding_Object     = 0x03
} UA_ExtObjectBodyEncoding;

typedef struct {
    UA_NodeId                typeId;
    UA_ExtObjectBodyEncoding encoding;

    union {
        UA_ByteString bstring;
        UA_XmlElement xml;
        struct {
            void*              value;
            UA_EncodeableType* objType;
        } object;

    } body;

    int32_t   length;

} UA_ExtensionObject;

typedef enum {
    UA_VariantArrayMatrixFlag = 64, // 2^6 => bit 6
    UA_VariantArrayValueFlag = 128 // 2^7 => bit 7
} UA_VariantArrayTypeFlag;

struct UA_DataValue;
struct UA_Variant;

typedef union {
    UA_Boolean*          booleanArr;
    UA_SByte*            sbyteArr;
    UA_Byte*             byteArr;
    int16_t*             int16Arr;
    uint16_t*            uint16Arr;
    int32_t*             int32Arr;
    uint32_t*            uint32Arr;
    int64_t*             int64Arr;
    uint64_t*            uint64Arr;
    float*               floatvArr;
    double*              doublevArr;
    UA_String*           stringArr;
    UA_DateTime*         dateArr;
    UA_Guid*             guidArr;
    UA_ByteString*       bstringArr;
    UA_XmlElement*       xmlEltArr;
    UA_NodeId*           nodeIdArr;
    UA_ExpandedNodeId*   expNodeIdArr;
    StatusCode*          statusArr;
    UA_QualifiedName*    qnameArr;
    UA_LocalizedText*    localizedTextArr;
    UA_ExtensionObject*  extObjectArr;
    struct UA_DataValue* dataValueArr;
    struct UA_Variant*   variantArr;
    UA_DiagnosticInfo*   diagInfoArr; // TODO: not present ?
} UA_VariantArrayValue;

typedef union {
        UA_Boolean           boolean;
        UA_SByte             sbyte;
        UA_Byte              byte;
        int16_t              int16;
        uint16_t             uint16;
        int32_t              int32;
        uint32_t             uint32;
        int64_t              int64;
        uint64_t             uint64;
        float                floatv;
        double               doublev;
        UA_String            string;
        UA_DateTime          date;
        UA_Guid*             guid;
        UA_ByteString        bstring;
        UA_XmlElement        xmlElt;
        UA_NodeId*           nodeId;
        UA_ExpandedNodeId*   expNodeId;
        StatusCode           status;
        UA_QualifiedName*    qname;
        UA_LocalizedText*    localizedText;
        UA_ExtensionObject*  extObject;
        struct UA_DataValue* dataValue;
        UA_DiagnosticInfo*   diagInfo; // TODO: not present ?
        struct {
            int32_t              length;
            UA_VariantArrayValue content;
        } array;
        struct {
            int32_t              dimensions;
            int32_t*             arrayDimensions; // Product of dimensions must be <= INT32_MAX ! (binary arrayLength valid for matrix too)
            UA_VariantArrayValue content;
        } matrix;

} UA_VariantValue;

typedef struct UA_Variant {
    UA_Byte         builtInTypeMask;
    UA_Byte         arrayTypeMask;
    UA_Byte         padding[2];
    UA_VariantValue value;
} UA_Variant;

typedef struct UA_DataValue {
    UA_Variant  value;
    StatusCode  status;
    UA_DateTime sourceTimestamp;
    UA_DateTime serverTimestamp;
    uint16_t    sourcePicoSeconds;
    uint16_t    serverPicoSeconds;
} UA_DataValue;

#define SECURITY_POLICY_NONE           "http://opcfoundation.org/UA/SecurityPolicy#None"
#define SECURITY_POLICY_BASIC128RSA15  "http://opcfoundation.org/UA/SecurityPolicy#Basic128Rsa15"
#define SECURITY_POLICY_BASIC256       "http://opcfoundation.org/UA/SecurityPolicy#Basic256"
#define SECURITY_POLICY_BASIC256SHA256 "http://opcfoundation.org/UA/SecurityPolicy#Basic256Sha256"

typedef enum {
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
UA_ByteString* ByteString_CreateFixedSize(uint32_t size);
UA_ByteString* ByteString_Copy(UA_ByteString* src);
void ByteString_Clear(UA_ByteString* bstring);
StatusCode ByteString_Compare(UA_ByteString* left,
                              UA_ByteString* right,
                              uint32_t*  comparison);
uint32_t ByteString_Equal(UA_ByteString* left,
                          UA_ByteString* right);

void String_Initialize(UA_String* string);
UA_String* String_Create(void);

UA_String* String_CreateFromCString(char* cString);
char* String_GetCString(UA_String* string);

UA_String* String_Copy(UA_String* src);
void String_Clear(UA_String* bstring);

StatusCode String_Compare(UA_String* left,
                          UA_String* right,
                          uint32_t*  comparison);

uint32_t String_Equal(UA_String* left,
                      UA_String* right);

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


#endif /* INGOPCS_UA_TYPES_H_ */
