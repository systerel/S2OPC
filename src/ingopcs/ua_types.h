/*
 * ua_types.h
 *
 *  Created on: Jul 22, 2016
 *      Author: vincent
 */

#ifndef INGOPCS_UA_TYPES_H_
#define INGOPCS_UA_TYPES_H_

#define UA_NULL ((void *)0)

#include <stdint.h>

typedef uint8_t UA_Byte;

typedef struct UA_ByteString {
    int32_t   length;
    UA_Byte*  characters;
} UA_ByteString;

typedef UA_ByteString UA_String;

typedef uint32_t UA_Boolean;
#define UA_FALSE 0

#define STATUS_OK 0x0 // TODO: change values
#define STATUS_OK_INCOMPLETE 0x00000001
#define STATUS_NOK 0x80000000//0x10000000
#define STATUS_INVALID_PARAMETERS 0x80760001//0x20000000
#define STATUS_INVALID_STATE 0x80760002//0x30000000
#define STATUS_INVALID_RCV_PARAMETER 0x80000003//0x40000000
typedef uint32_t StatusCode;


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

UA_ByteString* ByteString_Create(void);
UA_ByteString* ByteString_CreateFixedSize(uint32_t size);
UA_ByteString* ByteString_Copy(UA_ByteString* src);
void ByteString_Delete(UA_ByteString* bstring);
StatusCode ByteString_Compare(UA_ByteString* left,
                              UA_ByteString* right,
                              uint32_t*  comparison);
uint32_t ByteString_Equal(UA_ByteString* left,
                          UA_ByteString* right);

UA_String* String_CreateFromCString(char* cString);
char* String_GetCString(UA_String* string);

UA_String* String_Create(void);
UA_String* String_Copy(UA_String* src);
void String_Delete(UA_String* bstring);
StatusCode String_Compare(UA_String* left,
                          UA_String* right,
                          uint32_t*  comparison);
uint32_t String_Equal(UA_String* left,
                      UA_String* right);

#endif /* INGOPCS_UA_TYPES_H_ */
