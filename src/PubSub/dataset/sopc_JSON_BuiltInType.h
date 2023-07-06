/*
 * sopc_JSON_BuiltInType.h
 *
 *  Created on: 6 juil. 2023
 *      Author: sebastien
 */

#ifndef SRC_PUBSUB_DATASET_SOPC_JSON_BUILTINTYPE_H_
#define SRC_PUBSUB_DATASET_SOPC_JSON_BUILTINTYPE_H_

#include <inttypes.h>
#include "stdio.h"
#include "string.h"

#include "sopc_enums.h"
#include "sopc_builtintypes.h"
#include "sopc_macros.h"

SOPC_ReturnStatus SOPC_JSON_BuiltInType_Boolean		(bool value, char * buffer, size_t sizeBuffer);
SOPC_ReturnStatus SOPC_JSON_BuiltInType_Int16		(int value, char * buffer, size_t sizeBuffer);
SOPC_ReturnStatus SOPC_JSON_BuiltInType_UInt16		(uint16_t value, char * buffer, size_t sizeBuffer);
SOPC_ReturnStatus SOPC_JSON_BuiltInType_Int32		(long value, char * buffer, size_t sizeBuffer);
SOPC_ReturnStatus SOPC_JSON_BuiltInType_UInt32		(uint32_t value, char * buffer, size_t sizeBuffer);
SOPC_ReturnStatus SOPC_JSON_BuiltInType_Int64		(long long value, char * buffer, size_t sizeBuffer);
SOPC_ReturnStatus SOPC_JSON_BuiltInType_UInt64		(uint64_t value, char * buffer, size_t sizeBuffer);
SOPC_ReturnStatus SOPC_JSON_BuiltInType_Float		(float value, char * buffer, size_t sizeBuffer);
SOPC_ReturnStatus SOPC_JSON_BuiltInType_Double		(double value, char * buffer, size_t sizeBuffer);
SOPC_ReturnStatus SOPC_JSON_BuiltInType_String		(char * value, char * buffer, size_t sizeBuffer);


#endif /* SRC_PUBSUB_DATASET_SOPC_JSON_BUILTINTYPE_H_ */
