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

typedef uint32_t StatusCode;

#endif /* INGOPCS_OPCUA_TYPES_H_ */
