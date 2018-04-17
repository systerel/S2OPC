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
    uint64_t size;     // Always a power of two
    uint64_t sizemask; // sizemask == (size - 1), used to replace (hash % size) by (hash & sizemask)
    uint64_t n_items;
    void* empty_key;
    SOPC_Dict_KeyHash_Fct hash_func;
    SOPC_Dict_KeyEqual_Fct equal_func;
    SOPC_Dict_Free_Fct key_free;
    SOPC_Dict_Free_Fct value_free;
};

static void set_empty_keys(SOPC_DictBucket* buckets, uint64_t n_buckets, void* empty_key)
{
    for (uint64_t i = 0; i < n_buckets; ++i)
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
    for (uint64_t i = 0; i < d->size; ++i)
    {
        uint64_t idx = HASH_I(hash, i) & d->sizemask;
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

static bool dict_resize(SOPC_Dict* d, uint64_t size)
{
    uint64_t sizemask = size - 1;
    SOPC_DictBucket* buckets = calloc(size, sizeof(SOPC_DictBucket));

    if (buckets == NULL)
    {
        return false;
    }

    if (d->empty_key != NULL)
    {
        set_empty_keys(buckets, size, d->empty_key);
    }

    uint64_t old_size = d->size;
    SOPC_DictBucket* old_buckets = d->buckets;

    d->buckets = buckets;
    d->size = size;
    d->sizemask = sizemask;

    bool ok = true;

    for (uint64_t i = 0; i < old_size; ++i)
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
    static const uint64_t DICT_INITIAL_SIZE = 16;

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
            for (uint64_t i = 0; i < d->size; ++i)
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

bool SOPC_Dict_Reserve(SOPC_Dict* d, uint64_t n_items)
{
    uint64_t new_size = d->size;

    while (new_size < n_items)
    {
        new_size *= 2;
    }

    return dict_resize(d, new_size);
}

bool SOPC_Dict_Insert(SOPC_Dict* d, void* key, void* value)
{
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

void* SOPC_Dict_Get(const SOPC_Dict* d, const void* key, bool* found)
{
    uint64_t hash = d->hash_func(key);
    void* value = NULL;

    if (found != NULL)
    {
        *found = false;
    }

    for (uint64_t i = 0; i < d->size; ++i)
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
        }
    }

    return value;
}
