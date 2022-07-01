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
typedef void SOPC_Dict_Free_Fct(void* data);

/**
 * \brief Type of hash functions.
 */
typedef uint64_t SOPC_Dict_KeyHash_Fct(const void* data);

/**
 * \brief Type of functions used when checking two keys for equality. The
 * function should return \c TRUE if and only if both keys are equal, \c FALSE
 * otherwise.
 */
typedef bool SOPC_Dict_KeyEqual_Fct(const void* a, const void* b);

/**
 * \brief Type of callback functions for \ref SOPC_Dict_ForEach. Both the key and
 * value belong to the dictionary and shall not be modified. The value of
 * \p user_data is set when calling \ref SOPC_Dict_ForEach.
 */
typedef void SOPC_Dict_ForEach_Fct(const void* key, const void* value, void* user_data);

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
 *
 * Removal of values is not supported by default. To enable removing values from
 * the dictionary, set a tombstone key using \ref SOPC_Dict_SetTombstoneKey.
 */
SOPC_Dict* SOPC_Dict_Create(void* empty_key,
                            SOPC_Dict_KeyHash_Fct* key_hash,
                            SOPC_Dict_KeyEqual_Fct* key_equal,
                            SOPC_Dict_Free_Fct* key_free,
                            SOPC_Dict_Free_Fct* value_free);

/**
 * \brief Deletes a dictionary.
 * \param d The dictionary.
 *
 * The keys and/or values will also be freed if the corresponding \c key_free
 * and/or \c value_free parameters were passed to \ref SOPC_Dict_Create , or set
 * via \ref SOPC_Dict_SetKeyFreeFunc and \ref SOPC_Dict_SetValueFreeFunc .
 */
void SOPC_Dict_Delete(SOPC_Dict* d);

/**
 * \brief Reserve space for a given number of items in a dictionary.
 * \param d        The dictionary.
 * \param n_items  The minimum capacity to ensure.
 * \return \c TRUE in case of success, or \c FALSE in case of memory allocation
 *         failure.
 */
bool SOPC_Dict_Reserve(SOPC_Dict* d, size_t n_items);

/**
 * \brief Set the key used to mark removed values.
 * \param d              The dictionary.
 * \param tombstone_key  The key used to mark removed values.
 *
 * When removing values from the dictionary, this key will be used to indicate
 * that the bucket is now empty. This means that after this function is called,
 * the tombstone key cannot be used for normal values anymore. If the tombstone
 * key is not set, removals of values is not supported by the dictionary.
 *
 * The tombstone key MUST be different from the empty key.
 *
 * As a safeguard, calling this function is only allowed when the dictionary is
 * completely empty (including tombstones), like right after its creation.
 */
void SOPC_Dict_SetTombstoneKey(SOPC_Dict* d, void* tombstone_key);

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

/**
 * \brief Removes a values from the dictionary.
 * \param d    The dictionary.
 * \param key  The key to remove.
 *
 * For removals to be supported, a tombstone key MUST have been set before using
 * \ref SOPC_Dict_SetTombstoneKey.
 */
void SOPC_Dict_Remove(SOPC_Dict* d, const void* key);

/**
 * \brief Retrieves the free function for this dictionary's keys.
 * \param d  The dictionary.
 * \return The function used to free the keys, or NULL if no such function was
 *         set.
 */
SOPC_Dict_Free_Fct* SOPC_Dict_GetKeyFreeFunc(const SOPC_Dict* d);

/**
 * \brief Sets the free function for this dictionary's keys.
 * \param d     The dictionary.
 * \param func  The function to use when freeing keys.
 */
void SOPC_Dict_SetKeyFreeFunc(SOPC_Dict* d, SOPC_Dict_Free_Fct* func);

/**
 * \brief Retrieves the free function for this dictionary's values.
 * \param d  The dictionary.
 * \return The function used to free the values, or NULL if no such function was
 *         set.
 */
SOPC_Dict_Free_Fct* SOPC_Dict_GetValueFreeFunc(const SOPC_Dict* d);

/**
 * \brief Sets the free function for this dictionary's values.
 * \param d     The dictionary.
 * \param func  The function to use when freeing values.
 */
void SOPC_Dict_SetValueFreeFunc(SOPC_Dict* d, SOPC_Dict_Free_Fct* func);

/**
 * \brief Returns the number of items in this dictionary.
 * \param d  The dictionary.
 * \return The number of items in the dictionary.
 */
size_t SOPC_Dict_Size(const SOPC_Dict* d);

/**
 * \brief Returns the number if items this dictionary can hold.
 * \param d  The dictionary.
 * \return The number of items the dictionary can hold.
 *
 * The dictionary will grow its capacity as needed when inserting new items, and
 * reduce it after enough items are removed.
 */
size_t SOPC_Dict_Capacity(const SOPC_Dict* d);

/**
 * \brief Iterates over the dictionary, calling the given function for each
 *        (key, value) pair.
 *
 * \param d         The dictionary.
 * \param func      The function to call on each (key, value) pair.
 * \param user_data A user chose pointer to pass as last parameter to the
 *                  callback function.
 *
 * The order of the iteration is implementation defined, and should not be relied
 * on.
 */
void SOPC_Dict_ForEach(SOPC_Dict* d, SOPC_Dict_ForEach_Fct* func, void* user_data);

#endif /* SOPC_DICT_H_ */
