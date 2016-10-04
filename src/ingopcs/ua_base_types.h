/*
 * basic_types.h
 *
 *  Created on: Oct 4, 2016
 *      Author: vincent
 */

#ifndef INGOPCS_BASIC_TYPES_H_
#define INGOPCS_BASIC_TYPES_H_

#include <stdint.h>
#include <stddef.h>

typedef uint32_t StatusCode;
#define STATUS_OK 0x0 // TODO: change values
#define STATUS_OK_INCOMPLETE 0x00000001
#define STATUS_NOK 0x80000000//0x10000000
#define STATUS_INVALID_PARAMETERS 0x80760001//0x20000000
#define STATUS_INVALID_STATE 0x80760002//0x30000000
#define STATUS_INVALID_RCV_PARAMETER 0x80000003//0x40000000
#define UA_NULL NULL

#endif /* INGOPCS_BASIC_TYPES_H_ */
