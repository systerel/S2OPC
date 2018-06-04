/*
 *  Copyright (C) 2018 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
