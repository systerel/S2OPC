/**
 * OPC Foundation OPC UA Safety Mapper
 *
 * \file
 * \author
 *    Copyright 2021 (c) ifak e.V.Magdeburg
 *    Copyright 2021 (c) Tianzhe Yu
 *    Copyright 2021 (c) Matthias Riedl
 *    Copyright 2021 (c) Elke Hintze
 *
 * \brief Standard types of UA Safety Mapper implementation
 *
 * \date      2021-05-14
 * \revision  0.2
 * \status    in work
 *
 * Defines std types of UAM implementation.
 *
 * Safety-Related: no
 */

#ifndef __UAM_STDTYPES_H
#define __UAM_STDTYPES_H
#ifdef __cplusplus
extern "C"
{
#endif
#include "uas_stdtypes.h"
#include "uas_type.h"
#ifdef __cplusplus
}
#endif

#ifdef _WIN32

typedef __int8 int8;
typedef __int16 int16;
typedef __int32 int32;
typedef __int64 int64;

typedef unsigned __int8 uint8;
typedef unsigned __int16 uint16;
typedef unsigned __int32 uint32;
typedef unsigned __int64 uint64;

#elif defined(__GNUC__)

#include <stdint.h>

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

#else
#error unknown target
#endif

typedef float float32;
typedef double float64;

#endif // __UAM_STDTYPES_H
