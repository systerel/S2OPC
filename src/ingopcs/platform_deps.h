/*
 * platform_deps.h
 *
 *  Created on: Aug 2, 2016
 *      Author: vincent
 */

#ifndef INGOPCS_PLATFORM_DEPS_H_
#define INGOPCS_PLATFORM_DEPS_H_

#include <stdint.h>

// Invalid before call to initialize
uint32_t isLittleEndian;

uint32_t Initialize(void);

#endif /* INGOPCS_PLATFORM_DEPS_H_ */
