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
 *  \file sopc_macros.h
 *
 *  \brief Macros used by S2OPC
 */

#ifndef SOPC_MACROS_H_
#define SOPC_MACROS_H_

#if defined(__GNUC__) && (__GNUC__ > 4)
#define SOPC_GCC_DIAGNOSTIC_PUSH _Pragma("GCC diagnostic push")
#define SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST _Pragma("GCC diagnostic ignored \"-Wcast-qual\"")
#define SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_ALIGN _Pragma("GCC diagnostic ignored \"-Wcast-align\"")
#define SOPC_GCC_DIAGNOSTIC_RESTORE _Pragma("GCC diagnostic pop")
#else
#define SOPC_GCC_DIAGNOSTIC_PUSH
#define SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
#define SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_ALIGN
#define SOPC_GCC_DIAGNOSTIC_RESTORE
#endif

#if defined(__GNUC__) && (__GNUC__ > 4) && !defined(__clang__)
#define SOPC_GCC_DIAGNOSTIC_IGNORE_DISCARD_QUALIFIER _Pragma("GCC diagnostic ignored \"-Wdiscarded-qualifiers\"")
#elif defined(__clang__)
#define SOPC_GCC_DIAGNOSTIC_IGNORE_DISCARD_QUALIFIER \
    _Pragma("GCC diagnostic ignored \"-Wincompatible-pointer-types-discards-qualifiers\"")
#else
#define SOPC_GCC_DIAGNOSTIC_IGNORE_DISCARD_QUALIFIER
#endif

// This macro is used to avoid warning about unused argument :
#define SOPC_UNUSED_ARG(arg) (void) (arg)

// This macro is used to avoid warning about unused return function :
#define SOPC_UNUSED_RESULT(arg) (void) (arg)

#endif // SOPC_MACROS_H_
