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
 * @brief
 * Implementation of SOPC threads in scope of native PIKEOS personnality
 */

#ifndef SOPC_PIKEOS_P_THREADS_H_
#define SOPC_PIKEOS_P_THREADS_H_

#include <p4.h>
#include <p4ext_threads.h>

#include "p_sopc_synchro.h"
#include "sopc_enums.h"

typedef struct T_THREAD_WKS tThreadWks; // Thread workspace
typedef tThreadWks* SOPC_Thread;             // Handle workspace

#endif // SOPC_PIKEOS_P_THREADS_H_
