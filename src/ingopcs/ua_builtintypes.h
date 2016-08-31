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

typedef uint8_t UA_Byte;

typedef UA_Byte UA_Boolean;
#define UA_FALSE 0

typedef struct UA_ByteString {
    int32_t   length;
    UA_Byte*  characters;
} UA_ByteString;

typedef UA_ByteString UA_String;

typedef UA_ByteString UA_XmlElement;

typedef int64_t UA_DateTime;

typedef struct UA_Guid {
    uint32_t data1;
    uint16_t data2;
    uint16_t data3;
    UA_Byte  data4[8];
} UA_Guid;

typedef enum UA_IdentifierType {
    IdentifierType_Numeric = 0x00,
    IdentifierType_String = 0x01,
    IdentifierType_Guid = 0x02,
    IdentifierType_ByteString = 0x03,
    IdentifierType_Undefined = 0xFF
} UA_IdentifierType;

typedef struct UA_NodeId {
    uint16_t namespace;
    uint16_t identifierType; // UA_IdentifierType

    union {
        uint32_t      numeric;
        UA_String     string;
        UA_Guid       guid;
        UA_ByteString bstring;
    };
} UA_NodeId;

typedef struct UA_ExpandedNodeId {
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

void Byte_Initialize(UA_Byte* byte);
void Byte_Clear(UA_Byte* byte);

void Boolean_Initialize(UA_Boolean* b);
void Boolean_Clear(UA_Boolean* b);

void UInt32_Initialize(uint32_t* uint);
void UInt32_Clear(uint32_t* uint);

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

#endif /* INGOPCS_UA_TYPES_H_ */
