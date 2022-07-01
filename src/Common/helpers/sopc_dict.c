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

#include "sopc_dict.h"
#include "sopc_mem_alloc.h"

#include <assert.h>

#define HASH_I(hash, i) (hash + (i / 2) + (i * i / 2))

/* This is a dictionary implemented using quadratic probing (see
 * https://en.wikipedia.org/wiki/Linear_probing) for resolving key conflicts.
 *
 * We distinguish between empty and non-empty buckets using a special value for
 * the key. Removal of items can be implemented by stealing a second value from
 * the keyspace and using it as a tombstone.
 */

typedef struct _SOPC_DictBucket
{
    void* key;
    void* value;
} SOPC_DictBucket;

struct _SOPC_Dict
{
    SOPC_DictBucket* buckets;
    size_t size;     // Total number of buckets, always a power of two
    size_t sizemask; // sizemask == (size - 1), used to replace (hash % size) by (hash & sizemask)
    size_t n_items;  // Number of buckets holding a "real" value (not empty, not tombstone)
    size_t n_busy;   // Number of buckets where the key is not the empty key
    void* empty_key;
    void* tombstone_key;
    SOPC_Dict_KeyHash_Fct* hash_func;
    SOPC_Dict_KeyEqual_Fct* equal_func;
    SOPC_Dict_Free_Fct* key_free;
    SOPC_Dict_Free_Fct* value_free;
};

static const size_t DICT_INITIAL_SIZE = 16;

// Shrink the table if the number of items is below SHRINK_FACTOR * (number of buckets)
// We never shrink below DICT_INITIAL_SIZE.
static const double SHRINK_FACTOR = 0.4;

static void set_empty_keys(SOPC_DictBucket* buckets, size_t n_buckets, void* empty_key)
{
    for (size_t i = 0; i < n_buckets; ++i)
    {
        buckets[i].key = empty_key;
    }
}

static void free_bucket(SOPC_DictBucket* b, SOPC_Dict_Free_Fct* key_free, SOPC_Dict_Free_Fct* value_free)
{
    if (key_free != NULL)
    {
        key_free(b->key);
    }

    if (value_free != NULL)
    {
        value_free(b->value);
    }
}

static bool insert_item(SOPC_Dict* d, uint64_t hash, void* key, void* value, bool overwrite)
{
    for (size_t i = 0; i < d->size; ++i)
    {
        size_t idx = (size_t) HASH_I(hash, i) & d->sizemask;
        SOPC_DictBucket* b = &d->buckets[idx];

        // Normal insert
        if (b->key == d->empty_key || b->key == d->tombstone_key)
        {
            b->key = key;
            b->value = value;
            d->n_items++;
            d->n_busy++;
            return true;
        }

        // Overwriting of existing value
        if (overwrite && d->equal_func(key, b->key))
        {
            free_bucket(b, d->key_free, d->value_free);

            b->key = key;
            b->value = value;

            return true;
        }
    }

    assert(false && "Cannot find a free bucket?!");
    return false;
}

static bool dict_resize(SOPC_Dict* d, size_t size)
{
    size_t sizemask = size - 1;
    assert((size & sizemask) == 0); // Ensure we have a power of two

    SOPC_DictBucket* buckets = SOPC_Calloc(size, sizeof(SOPC_DictBucket));

    if (buckets == NULL)
    {
        return false;
    }

    if (d->empty_key != NULL)
    {
        set_empty_keys(buckets, size, d->empty_key);
    }

    SOPC_Dict dict_backup = *d;

    d->n_busy = 0;
    d->n_items = 0;
    d->buckets = buckets;
    d->size = size;
    d->sizemask = sizemask;

    bool ok = true;

    for (size_t i = 0; i < dict_backup.size; ++i)
    {
        SOPC_DictBucket* b = &dict_backup.buckets[i];

        if ((b->key == d->empty_key) || (b->key == d->tombstone_key))
        {
            continue;
        }

        uint64_t hash = d->hash_func(b->key);

        if (!insert_item(d, hash, b->key, b->value, false))
        {
            ok = false;
            break;
        }
    }

    if (ok)
    {
        SOPC_Free(dict_backup.buckets);
    }
    else
    {
        *d = dict_backup;
    }

    return ok;
}

SOPC_Dict* SOPC_Dict_Create(void* empty_key,
                            SOPC_Dict_KeyHash_Fct* key_hash,
                            SOPC_Dict_KeyEqual_Fct* key_equal,
                            SOPC_Dict_Free_Fct* key_free,
                            SOPC_Dict_Free_Fct* value_free)
{
    SOPC_Dict* d = SOPC_Calloc(1, sizeof(SOPC_Dict));

    if (d == NULL)
    {
        return NULL;
    }

    d->size = DICT_INITIAL_SIZE;
    d->sizemask = d->size - 1;

    d->buckets = SOPC_Calloc(d->size, sizeof(SOPC_DictBucket));

    bool ok = d->buckets != NULL;

    if (!ok)
    {
        SOPC_Dict_Delete(d);
        return NULL;
    }

    d->empty_key = empty_key;
    d->tombstone_key = empty_key;
    d->hash_func = key_hash;
    d->equal_func = key_equal;
    d->key_free = key_free;
    d->value_free = value_free;

    // We only go touch the memory if the empty key is not 0, since the memory
    // for our buckets is always alloced with calloc.
    if (d->empty_key != NULL)
    {
        set_empty_keys(d->buckets, d->size, d->empty_key);
    }

    return d;
}

void SOPC_Dict_Delete(SOPC_Dict* d)
{
    if (d != NULL)
    {
        // buckets can be NULL if called from SOPC_Dict_Create
        if (d->buckets != NULL)
        {
            for (size_t i = 0; i < d->size; ++i)
            {
                SOPC_DictBucket* bucket = &d->buckets[i];

                if ((bucket->key != d->empty_key) && (bucket->key != d->tombstone_key))
                {
                    free_bucket(bucket, d->key_free, d->value_free);
                }
            }

            SOPC_Free(d->buckets);
        }

        SOPC_Free(d);
    }
}

// Compute the minimum dictionary size (a power of two) given an initial size
// and a number of items to store.
// The computed value is such that dictionary occupation stays under 50%.
static size_t minimum_dict_size(size_t start_size, size_t n_items)
{
    assert((start_size & (start_size - 1)) == 0);

    size_t size = start_size;

    while (size < (2 * n_items))
    {
        size *= 2;
    }

    return size;
}

bool SOPC_Dict_Reserve(SOPC_Dict* d, size_t n_items)
{
    assert(d != NULL);
    return dict_resize(d, minimum_dict_size(d->size, n_items));
}

void SOPC_Dict_SetTombstoneKey(SOPC_Dict* d, void* tombstone_key)
{
    assert(d != NULL);
    assert(d->empty_key != tombstone_key);
    assert(d->n_busy == 0);
    d->tombstone_key = tombstone_key;
}

// Delta is 1 when adding, 0 when removing
static bool maybe_resize(SOPC_Dict* d, uint8_t delta)
{
    const size_t shrink_limit = (size_t)(SHRINK_FACTOR * ((double) d->size));
    size_t target_size = d->size;

    if (((delta > 0) && ((d->n_busy + delta) > (d->size / 2))) || ((delta == 0) && (d->n_items < shrink_limit)))
    {
        // One of the two cases:
        // - Overpopulation when adding items
        // - Underpopulation while removing items
        //
        // Compute the required number of buckets after eliminating tombstones
        // and resize.

        // Ensure the occupation will be under 50%
        target_size = minimum_dict_size(DICT_INITIAL_SIZE, d->n_items + delta);
    }

    return (d->size == target_size) ? true : dict_resize(d, target_size);
}

bool SOPC_Dict_Insert(SOPC_Dict* d, void* key, void* value)
{
    assert(d != NULL);
    assert(key != d->empty_key);
    assert(key != d->tombstone_key);

    if (!maybe_resize(d, 1))
    {
        return false;
    }

    uint64_t hash = d->hash_func(key);

    return insert_item(d, hash, key, value, true);
}

static SOPC_DictBucket* get_internal(const SOPC_Dict* d, const void* key)
{
    assert(key != d->empty_key);
    assert(key != d->tombstone_key);

    uint64_t hash = d->hash_func(key);

    for (size_t i = 0; i < d->size; ++i)
    {
        uint64_t idx = HASH_I(hash, i) & d->sizemask;
        const void* bucket_key = d->buckets[idx].key;

        if (bucket_key == d->empty_key)
        {
            break;
        }

        // If removals are not supported, we have empty_key == tombstone_key, so
        // this if never matches.
        if (bucket_key == d->tombstone_key)
        {
            continue;
        }

        if (d->equal_func(key, bucket_key))
        {
            return &d->buckets[idx];
        }
    }

    return NULL;
}

void* SOPC_Dict_Get(const SOPC_Dict* d, const void* key, bool* found)
{
    assert(d != NULL);
    SOPC_DictBucket* bucket = get_internal(d, key);

    if (found != NULL)
    {
        *found = (bucket != NULL);
    }

    return (bucket != NULL) ? bucket->value : NULL;
}

void* SOPC_Dict_GetKey(const SOPC_Dict* d, const void* key, bool* found)
{
    assert(d != NULL);
    SOPC_DictBucket* bucket = get_internal(d, key);

    if (found != NULL)
    {
        *found = (bucket != NULL);
    }

    return (bucket != NULL) ? bucket->key : NULL;
}

void SOPC_Dict_Remove(SOPC_Dict* d, const void* key)
{
    assert(d != NULL);

    // Check that a tombstone key has been defined
    assert(d->empty_key != d->tombstone_key);

    SOPC_DictBucket* bucket = get_internal(d, key);

    if (bucket == NULL)
    {
        return;
    }

    free_bucket(bucket, d->key_free, d->value_free);
    bucket->key = d->tombstone_key;
    bucket->value = NULL;
    --d->n_items;

    // We can ignore failures here, worst case we fail to compact and will try
    // later.
    maybe_resize(d, 0);
}

SOPC_Dict_Free_Fct* SOPC_Dict_GetKeyFreeFunc(const SOPC_Dict* d)
{
    assert(d != NULL);
    return d->key_free;
}

void SOPC_Dict_SetKeyFreeFunc(SOPC_Dict* d, SOPC_Dict_Free_Fct* func)
{
    assert(d != NULL);
    d->key_free = func;
}

SOPC_Dict_Free_Fct* SOPC_Dict_GetValueFreeFunc(const SOPC_Dict* d)
{
    assert(d != NULL);
    return d->value_free;
}

void SOPC_Dict_SetValueFreeFunc(SOPC_Dict* d, SOPC_Dict_Free_Fct* func)
{
    assert(d != NULL);
    d->value_free = func;
}

size_t SOPC_Dict_Size(const SOPC_Dict* d)
{
    return d->n_items;
}

size_t SOPC_Dict_Capacity(const SOPC_Dict* d)
{
    return d->size / 2;
}

void SOPC_Dict_ForEach(SOPC_Dict* d, SOPC_Dict_ForEach_Fct* func, void* user_data)
{
    for (size_t i = 0; i < d->size; ++i)
    {
        if (d->buckets[i].key != d->empty_key)
        {
            func(d->buckets[i].key, d->buckets[i].value, user_data);
        }
    }
}
