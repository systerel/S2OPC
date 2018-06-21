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
 * \brief Helpers for tests.
 *
 */

#ifndef SOPC_HEXLIFY_H_
#define SOPC_HEXLIFY_H_

#include <stddef.h> // size_t

int hexlify(const unsigned char* src, char* dst, size_t n);
int unhexlify(const char* src, unsigned char* dst, size_t n);

#endif /* SOPC_HEXLIFY_H_ */
