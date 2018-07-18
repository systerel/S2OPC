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

/** \file
 *
 * \brief Contains the constants used by the Tookit. Those constants are not intended to be modified.
 *
 */

#ifndef SOPC_TOOLKIT_CONSTANTS_H_
#define SOPC_TOOLKIT_CONSTANTS_H_

#include "sopc_toolkit_config_constants.h"

/* Check configured constants values */
#include "sopc_config_constants_check.h"

/** @brief Version of the toolkit */
#define SOPC_TOOLKIT_VERSION_MAJOR 0
#define SOPC_TOOLKIT_VERSION_MEDIUM 5
#define SOPC_TOOLKIT_VERSION_MINOR 0

#define SOPC_TOOLKIT_VERSION "0.5.0*"

/* SOPC return statuses */
typedef enum SOPC_ReturnStatus
{
    SOPC_STATUS_OK = 0,
    SOPC_STATUS_NOK,
    SOPC_STATUS_INVALID_PARAMETERS,
    SOPC_STATUS_INVALID_STATE,
    SOPC_STATUS_ENCODING_ERROR,
    SOPC_STATUS_WOULD_BLOCK,
    SOPC_STATUS_TIMEOUT,
    SOPC_STATUS_OUT_OF_MEMORY,
    SOPC_STATUS_CLOSED,
    SOPC_STATUS_NOT_SUPPORTED
} SOPC_ReturnStatus;

/**
 * \brief the toolkit provide and use monotonic clock for time references (used for timers)
 * Note: it is possible to set the clock as non monotonic defining variable on configuration. Otherwise default value is
 * true.
 */
#ifndef SOPC_MONOTONIC_CLOCK
#define SOPC_MONOTONIC_CLOCK true
#endif

#if defined(__GNUC__) && (__GNUC__ > 4) && !defined(__clang__)
#define SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST _Pragma("GCC diagnostic ignored \"-Wcast-qual\"");
#define SOPC_GCC_DIAGNOSTIC_RESTORE _Pragma("GCC diagnostic pop")
#else
#define SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
#define SOPC_GCC_DIAGNOSTIC_RESTORE
#endif

#endif /* SOPC_TOOLKIT_CONSTANTS_H_ */
