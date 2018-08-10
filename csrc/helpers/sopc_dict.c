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

#include <assert.h>
#include <stdlib.h>

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
    size_t size;     // Always a power of two
    size_t sizemask; // sizemask == (size - 1), used to replace (hash % size) by (hash & sizemask)
    size_t n_items;
    void* empty_key;
    SOPC_Dict_KeyHash_Fct hash_func;
    SOPC_Dict_KeyEqual_Fct equal_func;
    SOPC_Dict_Free_Fct key_free;
    SOPC_Dict_Free_Fct value_free;
};

static void set_empty_keys(SOPC_DictBucket* buckets, size_t n_buckets, void* empty_key)
{
    for (size_t i = 0; i < n_buckets; ++i)
    {
        buckets[i].key = empty_key;
    }
}

static void free_bucket(SOPC_DictBucket* b, SOPC_Dict_Free_Fct key_free, SOPC_Dict_Free_Fct value_free)
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
        if (b->key == d->empty_key)
        {
            b->key = key;
            b->value = value;
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
    SOPC_DictBucket* buckets = calloc(size, sizeof(SOPC_DictBucket));

    if (buckets == NULL)
    {
        return false;
    }

    if (d->empty_key != NULL)
    {
        set_empty_keys(buckets, size, d->empty_key);
    }

    size_t old_size = d->size;
    SOPC_DictBucket* old_buckets = d->buckets;

    d->buckets = buckets;
    d->size = size;
    d->sizemask = sizemask;

    bool ok = true;

    for (size_t i = 0; i < old_size; ++i)
    {
        if (old_buckets[i].key == d->empty_key)
        {
            continue;
        }

        SOPC_DictBucket* b = &old_buckets[i];
        uint64_t hash = d->hash_func(b->key);

        if (!insert_item(d, hash, b->key, b->value, false))
        {
            ok = false;
            break;
        }
    }

    if (ok)
    {
        free(old_buckets);
    }
    else
    {
        d->buckets = old_buckets;
        d->size = old_size;
        d->sizemask = old_size - 1;
    }

    return ok;
}

SOPC_Dict* SOPC_Dict_Create(void* empty_key,
                            SOPC_Dict_KeyHash_Fct key_hash,
                            SOPC_Dict_KeyEqual_Fct key_equal,
                            SOPC_Dict_Free_Fct key_free,
                            SOPC_Dict_Free_Fct value_free)
{
    static const size_t DICT_INITIAL_SIZE = 16;

    SOPC_Dict* d = calloc(1, sizeof(SOPC_Dict));

    if (d == NULL)
    {
        return NULL;
    }

    d->size = DICT_INITIAL_SIZE;
    d->sizemask = d->size - 1;

    d->buckets = calloc(d->size, sizeof(SOPC_DictBucket));

    bool ok = d->buckets != NULL;

    if (!ok)
    {
        SOPC_Dict_Delete(d);
        return NULL;
    }

    d->empty_key = empty_key;
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
                if (d->buckets[i].key != d->empty_key)
                {
                    free_bucket(&d->buckets[i], d->key_free, d->value_free);
                }
            }

            free(d->buckets);
        }

        free(d);
    }
}

bool SOPC_Dict_Reserve(SOPC_Dict* d, size_t n_items)
{
    assert(d != NULL);

    size_t new_size = d->size;

    while (new_size < n_items)
    {
        new_size *= 2;
    }

    return dict_resize(d, new_size);
}

bool SOPC_Dict_Insert(SOPC_Dict* d, void* key, void* value)
{
    assert(d != NULL);
    assert(key != d->empty_key);

    if ((d->n_items >= (d->size / 2)) && !dict_resize(d, 2 * d->size))
    {
        return false;
    }

    uint64_t hash = d->hash_func(key);

    if (insert_item(d, hash, key, value, true))
    {
        d->n_items++;
        return true;
    }
    else
    {
        return false;
    }
}

static void* get_internal(const SOPC_Dict* d, const void* key, bool* found, void** dict_key)
{
    uint64_t hash = d->hash_func(key);
    void* value = NULL;

    if (found != NULL)
    {
        *found = false;
    }

    if (dict_key != NULL)
    {
        *dict_key = NULL;
    }

    for (size_t i = 0; i < d->size; ++i)
    {
        uint64_t idx = HASH_I(hash, i) & d->sizemask;

        if (d->buckets[idx].key == d->empty_key)
        {
            break;
        }

        if (d->equal_func(key, d->buckets[idx].key))
        {
            value = d->buckets[idx].value;

            if (found != NULL)
            {
                *found = true;
            }

            if (dict_key != NULL)
            {
                *dict_key = d->buckets[idx].key;
            }
        }
    }

    return value;
}

void* SOPC_Dict_Get(const SOPC_Dict* d, const void* key, bool* found)
{
    assert(d != NULL);
    return get_internal(d, key, found, NULL);
}

void* SOPC_Dict_GetKey(const SOPC_Dict* d, const void* key, bool* found)
{
    assert(d != NULL);

    void* dict_key;
    bool _found;

    get_internal(d, key, &_found, &dict_key);

    if (found != NULL)
    {
        *found = _found;
    }

    return dict_key;
}

SOPC_Dict_Free_Fct SOPC_Dict_GetKeyFreeFunc(const SOPC_Dict* d)
{
    assert(d != NULL);
    return d->key_free;
}

void SOPC_Dict_SetKeyFreeFunc(SOPC_Dict* d, SOPC_Dict_Free_Fct func)
{
    assert(d != NULL);
    d->key_free = func;
}

SOPC_Dict_Free_Fct SOPC_Dict_GetValueFreeFunc(const SOPC_Dict* d)
{
    assert(d != NULL);
    return d->value_free;
}

void SOPC_Dict_SetValueFreeFunc(SOPC_Dict* d, SOPC_Dict_Free_Fct func)
{
    assert(d != NULL);
    d->value_free = func;
}
