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
