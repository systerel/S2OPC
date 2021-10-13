/*
 * Licensed to Systerel under one or more contributor license
 * agreements. See the NOTICE file distributed with this work
 * for additional information regarding copyright ownership.
 * Systerel licenses this file to you under the Apache
 * License, Version 2.0 (the "License"); you may not use this
 * file except in compliance with the License. You may obtain
 * a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

/**
 * This file is an excerpt from stdint.h.
 * It has two objectives: define symbols required by cffi,
 * provide some types required by Visual Studio.
 *
 * It should not be included in a generic project.
 * See s2opc_headers.h
 *
 * It is not possible to include stdint with Visual Studio, as it contains some __compiler directives,
 * and it is not possible to include stdint on some gcc versions/architectures, as it contains __extension__ directives,
 * which are not compatible with pycparser, which is used by cffi.
 * Hence we must redefine manually the stdints under Visual Studio or gcc that use them.
 *
 * Note that part of this code is legacy, as more recent versions of pycffi understand ?intX_t types.
 *
 * The compiler compatibility is arbitrary selective:
 * it is IMPORTANT to double check the defines before adding another compiler/architecture.
 * Otherwise you risk nasty side-effects when calling the S2OPC library with the wrong integers sizes...
 */

/* First, we have to include these information only when preparing the header for cffi,
 * not when compiling the generated _pys2opc.c module.
 * To do so, S2OPC_PYEXPANSION will be defined when expanding this header.
 */

#ifdef S2OPC_PYEXPANSION

#if defined(_M_X64) && defined(_MSC_VER) && _MSC_VER >= 1900 && _MSC_VER < 1920

typedef signed char int8_t;
typedef short int16_t;
typedef int int32_t;
typedef long long int64_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

typedef signed char int_least8_t;
typedef short int_least16_t;
typedef int int_least32_t;
typedef long long int_least64_t;
typedef unsigned char uint_least8_t;
typedef unsigned short uint_least16_t;
typedef unsigned int uint_least32_t;
typedef unsigned long long uint_least64_t;

typedef signed char int_fast8_t;
typedef int int_fast16_t;
typedef int int_fast32_t;
typedef long long int_fast64_t;
typedef unsigned char uint_fast8_t;
typedef unsigned int uint_fast16_t;
typedef unsigned int uint_fast32_t;
typedef unsigned long long uint_fast64_t;

typedef long long intmax_t;
typedef unsigned long long uintmax_t;

/* These constants are modified:
 * they have a size suffix that is unknown to pycparser: i8, ... u128 */
#define INT8_MIN (-127 - 1)
#define INT16_MIN (-32767 - 1)
#define INT32_MIN (-2147483647 - 1)
#define INT64_MIN (-9223372036854775807LL - 1)
#define INT8_MAX 127
#define INT16_MAX 32767
#define INT32_MAX 2147483647
#define INT64_MAX 9223372036854775807LL
#define UINT8_MAX 0xff
#define UINT16_MAX 0xffff
#define UINT32_MAX 0xffffffffU
#define UINT64_MAX 0xffffffffffffffffULL

#define INT_LEAST8_MIN INT8_MIN
#define INT_LEAST16_MIN INT16_MIN
#define INT_LEAST32_MIN INT32_MIN
#define INT_LEAST64_MIN INT64_MIN
#define INT_LEAST8_MAX INT8_MAX
#define INT_LEAST16_MAX INT16_MAX
#define INT_LEAST32_MAX INT32_MAX
#define INT_LEAST64_MAX INT64_MAX
#define UINT_LEAST8_MAX UINT8_MAX
#define UINT_LEAST16_MAX UINT16_MAX
#define UINT_LEAST32_MAX UINT32_MAX
#define UINT_LEAST64_MAX UINT64_MAX

#define INT_FAST8_MIN INT8_MIN
#define INT_FAST16_MIN INT32_MIN
#define INT_FAST32_MIN INT32_MIN
#define INT_FAST64_MIN INT64_MIN
#define INT_FAST8_MAX INT8_MAX
#define INT_FAST16_MAX INT32_MAX
#define INT_FAST32_MAX INT32_MAX
#define INT_FAST64_MAX INT64_MAX
#define UINT_FAST8_MAX UINT8_MAX
#define UINT_FAST16_MAX UINT32_MAX
#define UINT_FAST32_MAX UINT32_MAX
#define UINT_FAST64_MAX UINT64_MAX

#ifdef _WIN64
#define INTPTR_MIN INT64_MIN
#define INTPTR_MAX INT64_MAX
#define UINTPTR_MAX UINT64_MAX
#else
#define INTPTR_MIN INT32_MIN
#define INTPTR_MAX INT32_MAX
#define UINTPTR_MAX UINT32_MAX
#endif

#define INTMAX_MIN INT64_MIN
#define INTMAX_MAX INT64_MAX
#define UINTMAX_MAX UINT64_MAX

#define PTRDIFF_MIN INTPTR_MIN
#define PTRDIFF_MAX INTPTR_MAX

#ifndef SIZE_MAX
#define SIZE_MAX UINTPTR_MAX
#endif

#define SIG_ATOMIC_MIN INT32_MIN
#define SIG_ATOMIC_MAX INT32_MAX

#define WCHAR_MIN 0x0000
#define WCHAR_MAX 0xffff

#define WINT_MIN 0x0000
#define WINT_MAX 0xffff

#define INT8_C(x) (x)
#define INT16_C(x) (x)
#define INT32_C(x) (x)
#define INT64_C(x) (x##LL)

#define UINT8_C(x) (x)
#define UINT16_C(x) (x)
#define UINT32_C(x) (x##U)
#define UINT64_C(x) (x##ULL)

#define INTMAX_C(x) INT64_C(x)
#define UINTMAX_C(x) UINT64_C(x)

/* Some other may want to include stdint, prevent them */
#define RC_INVOKED

/* And please don't include <vcruntime.h> */
#define _VCRUNTIME_H

/* This may not belong to stdint.h */
typedef uint64_t uintptr_t;
typedef uint64_t size_t;
typedef int64_t ptrdiff_t;
typedef int64_t intptr_t;

#else /* Otherwise we're most likely under gcc or clang */

/* Use these defines to prevent re-inclusion of stdint.h which, by cascading effects,
 * makes cffi think it does not know ?intX_t types anymore under certain architectures */
/* Another way to support this would be to update _cffi headers and duplicate headers that import stdint,
 * such as sopc_buffer.h */
#define _STDINT_H
#define _BITS_TYPES_H

/* Even though ?intX_t types are defined and understood by pycffi, it lacks the constant definitions.
 * In particular, INT32_MAX is used to force enum sizes across S2OPC. */
#define INT8_MIN (-127 - 1)
#define INT16_MIN (-32767 - 1)
#define INT32_MIN (-2147483647 - 1)
#define INT64_MIN (-9223372036854775807LL - 1)
#define INT8_MAX 127
#define INT16_MAX 32767
#define INT32_MAX 2147483647
#define INT64_MAX 9223372036854775807LL
#define UINT8_MAX 0xff
#define UINT16_MAX 0xffff
#define UINT32_MAX 0xffffffffU
#define UINT64_MAX 0xffffffffffffffffULL

#endif /* MSVC */

#endif /* S2OPC_PYEXPANSION */
