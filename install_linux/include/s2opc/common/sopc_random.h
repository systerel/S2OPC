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
 * \file sopc_random.h
 *
 * \brief A platform independent API to retrieve random data from platform.
 *
 * \note Only implemented for Linux.
 */

#ifndef SOPC_RANDOM_H_
#define SOPC_RANDOM_H_

#include "sopc_buffer.h"

/**
 * \brief Retrieve random data and put it in a buffer.
 *
 * \param buffer The buffer which will receive the data.
 *               Needs to be allocated (with SOPC_Buffer_Create()) before calling the function.
 *
 * \param length Length of the random data wanted.
 *
 * \return \ref SOPC_STATUS_OK in case of success, \ref SOPC_STATUS_NOK in case of error.
 */
SOPC_ReturnStatus SOPC_GetRandom(SOPC_Buffer* buffer, uint32_t length);

#endif /* SOPC_RANDOM_H_ */
