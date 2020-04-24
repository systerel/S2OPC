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
 * This file is used to expand headers because pycparser (hence CFFI) does not
 * support pre-processor directives.
 *
 * This requires to avoid std includes,
 * but also to redefine (at least by name) some of the required values.
 */

/* Avoid these includes (TODO: this is not portable) */
//#define _ASSERT_H_DECLS
//#define _FLOAT_H___
//#define _INTTYPES_H
//#define _GCC_LIMITS_H_
//#define _PTHREAD_H
//#define _STDARG_H
//#define _STDBOOL_H
//#define _STDDEF_H
//#define _STDIO_H
//#define _STDLIB_H
//#define _STDINT_H
//#define _STRING_H
//#define _TIME_H

/* We also need to define some of the numerical constants from these includes */
#define INT32_MAX   (2147483647)
#define UINT32_MAX  (4294967295U)
#define UINTPTR_MAX (18446744073709551615UL)
#define SIZE_MAX    (18446744073709551615UL)

/* Variadic macro used in logger */
#define __attribute__(...)

/* Now the includes of the project */
#include "sopc_user_app_itf.h"
#include "sopc_log_manager.h"
//#include "sopc_version.h"
//#include "toolkit_helpers.h"
#define SKIP_S2OPC_DEFINITIONS
#include "libs2opc_client.h"
