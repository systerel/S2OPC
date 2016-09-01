/*
 * platform_deps.h
 *
 *  Created on: Aug 2, 2016
 *      Author: vincent
 */

#ifndef INGOPCS_PLATFORM_DEPS_H_
#define INGOPCS_PLATFORM_DEPS_H_

#include <stdint.h>

typedef enum {
    Endianess_Undefined,
    Endianess_LittleEndian,
    Endianess_BigEndian
} Endianess;

// Undefined before call to initialize
extern Endianess endianess;
extern Endianess floatEndianess;

void InitPlatformDependencies(void);

#endif /* INGOPCS_PLATFORM_DEPS_H_ */
