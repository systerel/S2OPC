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
 *  \file
 *
 *  \brief A platform independent API to use threads
 */

#ifndef SOPC_THREADS_H_
#define SOPC_THREADS_H_

// Import Mutex type from platform dependent code
#include "p_threads.h"

/**
 *  \brief Function to create a thread
 *
 *  \param thread      Return parameter for the created thread
 *  \param startFct    Function called at thread start
 *  \param startArgs   Arguments of the start function
 *  \param taskName    Name of the created thread
 *
 *  \return            SOPC_STATUS_OK if operation succeeded, SOPC_STATUS_INVALID_PARAMETERS
 *      or SOPC_STATUS_NOK otherwise.
 */
SOPC_ReturnStatus SOPC_Thread_Create(Thread* thread, void* (*startFct)(void*), void* startArgs, const char* taskName);

/**
 *  \brief Function to wait for a thread to terminate
 *
 *  \param thread   Thread to wait for
 *
 */
SOPC_ReturnStatus SOPC_Thread_Join(Thread thread);

#endif /* SOPC_THREADS_H_ */
