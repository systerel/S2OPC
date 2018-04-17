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
 * \file sopc_hash.h
 * \brief Implementations of some hash functions.
 */

#ifndef SOPC_HASH_H_
#define SOPC_HASH_H_

#include <stddef.h>
#include <stdint.h>

/**
 * \brief Hashes some data using DJB hash.
 * \param data  The data to hash.
 * \param len   The length of the data, in bytes.
 * \return The resulting hash.
 */
uint64_t SOPC_DJBHash(const uint8_t* data, size_t len);

/**
 * \brief Appends some data to a DJB hash.
 * \param current  The current value of the hash.
 * \param data     The data to hash.
 * \param len      The length of the data, in bytes.
 * \return The resulting hash.
 *
 * This interface allows computing a hash over various pieces of data in several
 * calls.
 */
uint64_t SOPC_DJBHash_Step(uint64_t current, const uint8_t* data, size_t len);

#endif /* SOPC_HASH_H_ */
