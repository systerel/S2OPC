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
 * \file sopc_dict.h
 * \brief A dictionary implementation.
 */

#ifndef SOPC_DICT_H_
#define SOPC_DICT_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct _SOPC_Dict SOPC_Dict;

/**
 * \brief Type of functions used to free keys and values.
 */
typedef void (*SOPC_Dict_Free_Fct)(void* data);

/**
 * \brief Type of hash functions.
 */
typedef uint64_t (*SOPC_Dict_KeyHash_Fct)(const void* data);

/**
 * \brief Type of functions used when checking two keys for equality. The
 * function should return \c TRUE if and only if both keys are equal, \c FALSE
 * otherwise.
 */
typedef bool (*SOPC_Dict_KeyEqual_Fct)(const void* a, const void* b);

/**
 * \brief Creates a new, empty dictionary.
 *
 * \param empty_key   The key used to mark empty buckets. When using pointers as
 *                    keys (for example \c char* ), \c NULL is a good choice.
 * \param key_hash    A function to calculate the hash from a key.
 * \param key_equal   A function to compare two keys for equality.
 * \param key_free    A function to free the keys when the dictionary is freed.
 *                    Can be \c NULL if the keys should not be freed.
 * \param value_free  A function to free the values when the dictionary is freed.
 *                    Can be \c NULL if the values should not be freed.
 * \return The created dictionary in case of success, or \c NULL on memory
 *         allocation failure.
 */
SOPC_Dict* SOPC_Dict_Create(void* empty_key,
                            SOPC_Dict_KeyHash_Fct key_hash,
                            SOPC_Dict_KeyEqual_Fct key_equal,
                            SOPC_Dict_Free_Fct key_free,
                            SOPC_Dict_Free_Fct value_free);

/**
 * \brief Deletes a dictionary.
 * \param d The dictionary.
 *
 * The keys and/or values will also be freed if the corresponding \c key_free
 * and/or \c value_free parameters were passed to \ref SOPC_Dict_Create .
 */
void SOPC_Dict_Delete(SOPC_Dict* d);

/**
 * \brief Reserve space for a given number of items in a dictionary.
 * \param d        The dictionary.
 * \param n_items  The minimum capacity to ensure.
 * \return \c TRUE in case of success, or \c FALSE in case of memory allocation
 *         failure.
 */
bool SOPC_Dict_Reserve(SOPC_Dict* d, uint64_t n_items);

/**
 * \brief Inserts a new key and value in the dictionary.
 * \param d      The dictionary.
 * \param key    The key to insert.
 * \param value  The value to insert.
 * \return \c TRUE in case of success, or \c FALSE in case of memory allocation
 *         failure.
 *
 * The dictionary takes ownership of the key and value, those should not be
 * modified after insertion. If an item with the same key was already inserted,
 * it is overwritten.
 */
bool SOPC_Dict_Insert(SOPC_Dict* d, void* key, void* value);

/**
 * \brief Looks up the value associated with a key in the dictionary.
 * \param d      The dictionary.
 * \param key    The key to search for.
 * \param found  Out parameter, set to \c TRUE if the key is found. Can be
 *               \c NULL if the information is not required.
 * \return The associated value, or \c NULL if no matching key is found.
 *
 * The returned value belongs to the dictionary and should not be modified.
 */
void* SOPC_Dict_Get(const SOPC_Dict* d, const void* key, bool* found);

/**
 * \brief Looks up a given key in the dictionary.
 * \param d      The dictionary.
 * \param key    The key to search for.
 * \param found  Out parameter, set to \c TRUE if the key is found. Can be
 *               \c NULL if the information is not required.
 * \return The key stored in the dictionary, or \c NULL if no such key is found.
 *
 * The returned value belongs to the dictionary and should not be modified.
 *
 * This function is useful for example when "interning" values: one can look up
 * the stored copy of the key passed as a parameter and use it, freeing the
 * original.
 */
void* SOPC_Dict_GetKey(const SOPC_Dict* d, const void* key, bool* found);

#endif /* SOPC_DICT_H_ */
