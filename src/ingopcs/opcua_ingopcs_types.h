/*
 * opcua_types.h
 *
 *  Created on: Jul 22, 2016
 *      Author: vincent
 */

#ifndef INGOPCS_OPCUA_TYPES_H_
#define INGOPCS_OPCUA_TYPES_H_

#ifndef NULL
#define NULL 0
#endif

#include <stdint.h>

typedef uint8_t UA_Byte;

typedef struct UA_Byte_String {
    int32_t   length;
    UA_Byte*  characters;
} UA_Byte_String;

typedef UA_Byte_String UA_String;

#define STATUS_OK 0x0
#define STATUS_NOK 0x1
#define STATUS_INVALID_PARAMETERS 0x2
#define STATUS_INVALID_STATE 0x3
typedef uint32_t StatusCode;

UA_Byte_String* Create_Byte_String(void);
UA_Byte_String* Create_Byte_String_Copy(UA_Byte_String* src);
void Delete_Byte_String (UA_Byte_String* bstring);

UA_String* Create_String_From_CString(char* cString);


UA_String* Create_String(void);
UA_String* Create_String_Copy(UA_String* src);
void Delete_String (UA_String* bstring);


#endif /* INGOPCS_OPCUA_TYPES_H_ */
