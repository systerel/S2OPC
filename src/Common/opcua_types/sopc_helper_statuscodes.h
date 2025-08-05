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

#ifndef SOPC_HELPER_STATUSCODES_H_
#define SOPC_HELPER_STATUSCODES_H_

#include "sopc_builtintypes.h"

/**
 * \brief Convert a status code to its string representation
 *
 * \param status  The status code to convert
 * \return A const string pointer containing the name of the status code constant,
 *         or a generic message if the status code is not recognized.
 *         The returned string is valid for the lifetime of the program.
 */
const char* SOPC_StatusCodeToString(SOPC_StatusCode status);

/**
 * \brief Convert a status code to its string representation (allocating memory)
 *
 * \param status  The status code to convert
 * \return An allocated string containing the name of the status code constant,
 *         or the hexadecimal representation if the status code is not recognized.
 *         The caller is responsible for freeing the returned string with SOPC_Free().
 *         Returns NULL if memory allocation fails.
 */
char* SOPC_StatusCodeToStringAlloc(SOPC_StatusCode status);

#endif /* SOPC_HELPER_STATUSCODES_H_ */
