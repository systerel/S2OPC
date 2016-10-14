/*
 * platform_deps.h
 *
 *  Created on: Aug 2, 2016
 *      Author: vincent
 */

#ifndef INGOPCS_PLATFORM_DEPS_H_
#define INGOPCS_PLATFORM_DEPS_H_

#include <stdint.h>

#define OPCUA_BEGIN_EXTERN_C
#define OPCUA_END_EXTERN_C

typedef enum {
    P_Endianess_Undefined,
    P_Endianess_LittleEndian,
    P_Endianess_BigEndian
} Endianess;

// Undefined before call to initialize
extern Endianess endianess;
extern Endianess floatEndianess;

void InitPlatformDependencies(void);

#endif /* INGOPCS_PLATFORM_DEPS_H_ */
