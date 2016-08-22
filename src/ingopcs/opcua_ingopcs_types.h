/*
 * opcua_types.h
 *
 *  Created on: Jul 22, 2016
 *      Author: vincent
 */

#ifndef INGOPCS_OPCUA_TYPES_H_
#define INGOPCS_OPCUA_TYPES_H_

#define UA_NULL ((void *)0)

#include <stdint.h>

typedef uint8_t UA_Byte;

typedef struct UA_Byte_String {
    int32_t   length;
    UA_Byte*  characters;
} UA_Byte_String;

typedef UA_Byte_String UA_String;

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

typedef enum Security_Policy {
    Security_Policy_None = 0,
    Security_Policy_Basic128Rsa15 = 1,
    Security_Policy_Basic256 = 2,
    Security_Policy_Basic256Sha256 = 3,
} Security_Policy;

UA_Byte_String* Create_Byte_String(void);
UA_Byte_String* Create_Byte_String_Fixed_Size(uint32_t size);
UA_Byte_String* Create_Byte_String_Copy(UA_Byte_String* src);
void Delete_Byte_String(UA_Byte_String* bstring);
StatusCode Compare_Byte_Strings(UA_String* left,
                                UA_String* right,
                                uint32_t*  comparison);

UA_String* Create_String_From_CString(char* cString);
char* Create_CString_From_String(UA_String* string);

UA_String* Create_String(void);
UA_String* Create_String_Copy(UA_String* src);
void Delete_String(UA_String* bstring);
StatusCode Compare_Strings(UA_String* left,
                           UA_String* right,
                           uint32_t*  comparison);

int32_t little_endian (void);


#endif /* INGOPCS_OPCUA_TYPES_H_ */
