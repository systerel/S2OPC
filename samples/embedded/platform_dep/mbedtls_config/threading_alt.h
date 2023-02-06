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

#ifndef __THREADING_ALT_H__
#define __THREADING_ALT_H__

#ifndef __INT32_MAX__
#include <toolchain/xcc_missing_defs.h>
#endif

#include "sopc_mutexes.h"
#include "sopc_threads.h"

typedef Mutex mbedtls_threading_mutex_t;

void tls_threading_initialize(void);

#endif /* __THREADING_ALT_H__ */
