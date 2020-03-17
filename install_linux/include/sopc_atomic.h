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
 *  \brief A platform independent API for atomic integer operations.
 */

#ifndef SOPC_ATOMIC_H_
#define SOPC_ATOMIC_H_

#include <stdint.h>

/**
 * \brief Returns the current value of an atomic integer variable.
 *
 * \param atomic  The address of the atomic variable.
 *
 * \return The current value of the variable.
 */
int32_t SOPC_Atomic_Int_Get(int32_t* atomic);

/**
 * \brief Sets the value of an atomic integer variable.
 *
 * \param atomic  The address of the atomic variable.
 * \param val     The new value of the variable.
 */
void SOPC_Atomic_Int_Set(int32_t* atomic, int32_t val);

/**
 * \brief Adds a value to an atomic integer variable.
 *
 * \param atomic  The address of the atomic variable.
 * \param val     The value to add to the variable.
 *
 * \return The \em old value (before the add) of the atomic variable.
 */
int32_t SOPC_Atomic_Int_Add(int32_t* atomic, int32_t val);

/**
 * \brief Returns the current value of an atomic pointer.
 *
 * \param atomic  The address of the atomic pointer.
 *
 * \return The current value of the pointer.
 */
void* SOPC_Atomic_Ptr_Get(void** atomic);

/**
 * \brief Sets the value of an atomic pointer variable.
 *
 * \param atomic  The address of the atomic pointer.
 * \param val     The new value of the pointer.
 */
void SOPC_Atomic_Ptr_Set(void** atomic, void* val);

#endif // SOPC_ATOMIC_H_
