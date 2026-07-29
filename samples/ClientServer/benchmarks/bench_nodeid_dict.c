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
 * Benchmark NodeId dictionary lookup performance.
 *
 * Compares the hash used by SOPC_NodeId_Dict_Create with alternative hash functions on a
 * dictionary of 500k entries. Each candidate is measured both as a hash function (bucket
 * distribution) and as a lookup cost.
 *
 * Before timing, runs structural validation:
 * - all keys are distinct (NodeId equality)
 * - hash stability and equal-key consistency
 * - chi-square of the low bits, the quality criterion for a power-of-two bucket array
 * - probe-chain simulation with saturation detection
 * - hash-value collision count among distinct keys, a sanity check only: for 500k keys and an
 *   ideal 64-bit hash the expected number of colliding pairs is about 7e-9, so zero collisions
 *   is the expected outcome even for a poor hash and cannot be used to rank candidates
 * - reference hash on platform-independent canonical encoding
 *
 * The DJB variant walks the NodeId fields itself instead of calling SOPC_NodeId_Hash, so that
 * this baseline column keeps its meaning when the production hash changes. A dedicated variant
 * reports what SOPC_NodeId_Hash currently does.
 *
 * Lookup timings are repeated BENCH_REPETITIONS times; the median and the best run are both
 * reported, the best run being the least perturbed by scheduling noise.
 */

#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sopc_assert.h"
#include "sopc_builtintypes.h"
#include "sopc_dict.h"
#include "sopc_hash.h"
#include "sopc_mem_alloc.h"
#include "sopc_time_reference.h"

#define DEFAULT_DICT_SIZE 500000U
#define DEFAULT_LOOKUPS 10000000U
#define WARMUP_LOOKUPS 100000U
#define BENCH_REPETITIONS 5U

/* Chi-square classes for the low bits of the hash. With the default 500k keys this leaves about
 * 122 keys per class, well above the count needed for the statistic to be meaningful. A ratio
 * near 1 means the low bits are as uniform as a random mapping, a ratio well above 1 means the
 * bucket array is unevenly loaded. Values below 1 are better than random and harmless. */
#define CHI2_BUCKETS 4096U

typedef enum
{
    NODEID_PATTERN_STRING,
    NODEID_PATTERN_NUMERIC,
} nodeid_pattern_t;

typedef void NodeId_Hash_Fct(const SOPC_NodeId* nodeId, uint64_t* hash);

typedef struct
{
    const char* name;
    NodeId_Hash_Fct* hash;
    SOPC_Dict_KeyHash_Fct* hash_wrapper;
    bool include_in_perf;
} hash_variant_t;

typedef struct
{
    SOPC_NodeId** keys;
    size_t n_keys;
    size_t* lookup_indices;
    size_t n_lookups;
} bench_data_t;

typedef struct
{
    double elapsed_us;        /* median run */
    double ns_per_lookup;     /* median run */
    double ns_per_lookup_min; /* best run */
    double avg_probes;
    double max_probes;
} bench_result_t;

typedef struct
{
    size_t hash_collisions;
    size_t max_probe_chain;
    double chi2_ratio;
    bool probe_simulation_ok;
    bool hash_stable;
    bool equal_keys_consistent;
} hash_validation_t;

typedef struct
{
    uint64_t hash;
    size_t key_index;
} hash_entry_t;

/* FNV-1a 64-bit offset basis, the initial value expected by SOPC_FNV1aHash_Step. */
#define FNV1A_OFFSET_BASIS 14695981039346656037ULL

static void append_u16_be(uint8_t* buf, size_t* offset, uint16_t value)
{
    buf[(*offset)++] = (uint8_t)((value >> 8) & 0xFFU);
    buf[(*offset)++] = (uint8_t)(value & 0xFFU);
}

static void append_u32_be(uint8_t* buf, size_t* offset, uint32_t value)
{
    buf[(*offset)++] = (uint8_t)((value >> 24) & 0xFFU);
    buf[(*offset)++] = (uint8_t)((value >> 16) & 0xFFU);
    buf[(*offset)++] = (uint8_t)((value >> 8) & 0xFFU);
    buf[(*offset)++] = (uint8_t)(value & 0xFFU);
}

static size_t nodeid_canonical_size(const SOPC_NodeId* nodeId)
{
    size_t size = 1 + 2;

    switch (nodeId->IdentifierType)
    {
    case SOPC_IdentifierType_Numeric:
        size += 4;
        break;
    case SOPC_IdentifierType_ByteString:
    case SOPC_IdentifierType_String:
        size += 4 + (size_t) nodeId->Data.String.Length;
        break;
    case SOPC_IdentifierType_Guid:
        size += 16;
        break;
    default:
        size = 0;
        break;
    }

    return size;
}

static bool nodeid_canonical_encode(const SOPC_NodeId* nodeId, uint8_t* buf, size_t buf_cap, size_t* out_len)
{
    if (NULL == nodeId || NULL == buf || NULL == out_len)
    {
        return false;
    }

    const size_t needed = nodeid_canonical_size(nodeId);
    if (needed == 0 || needed > buf_cap)
    {
        return false;
    }

    size_t offset = 0;
    buf[offset++] = (uint8_t) nodeId->IdentifierType;
    append_u16_be(buf, &offset, nodeId->Namespace);

    switch (nodeId->IdentifierType)
    {
    case SOPC_IdentifierType_Numeric:
        append_u32_be(buf, &offset, nodeId->Data.Numeric);
        break;
    case SOPC_IdentifierType_ByteString:
    case SOPC_IdentifierType_String:
        append_u32_be(buf, &offset, (uint32_t) nodeId->Data.String.Length);
        if (nodeId->Data.String.Length > 0)
        {
            memcpy(&buf[offset], nodeId->Data.String.Data, (size_t) nodeId->Data.String.Length);
            offset += (size_t) nodeId->Data.String.Length;
        }
        break;
    case SOPC_IdentifierType_Guid:
        if (nodeId->Data.Guid == NULL)
        {
            memset(&buf[offset], 0, 16);
        }
        else
        {
            append_u32_be(buf, &offset, nodeId->Data.Guid->Data1);
            append_u16_be(buf, &offset, nodeId->Data.Guid->Data2);
            append_u16_be(buf, &offset, nodeId->Data.Guid->Data3);
            memcpy(&buf[offset], nodeId->Data.Guid->Data4, 8);
            offset += 8;
        }
        break;
    default:
        return false;
    }

    *out_len = offset;
    return true;
}

static bool nodeid_hash_from_canonical(const SOPC_NodeId* nodeId,
                                       uint64_t initial,
                                       uint64_t (*hash_bytes)(uint64_t, const uint8_t*, size_t),
                                       uint64_t* hash)
{
    const size_t cap = nodeid_canonical_size(nodeId);
    uint8_t* buf = SOPC_Malloc(cap);

    if (buf == NULL)
    {
        return false;
    }

    size_t len = 0;
    bool ok = nodeid_canonical_encode(nodeId, buf, cap, &len);
    if (ok)
    {
        *hash = hash_bytes(initial, buf, len);
    }

    SOPC_Free(buf);
    return ok;
}

static void hash_nodeid_fields(const SOPC_NodeId* nodeId,
                               uint64_t initial,
                               uint64_t (*step)(uint64_t, const uint8_t*, size_t),
                               uint64_t* hash)
{
    uint64_t h = initial;

    h = step(h, (const uint8_t*) &nodeId->IdentifierType, sizeof(SOPC_IdentifierType));
    h = step(h, (const uint8_t*) &nodeId->Namespace, sizeof(uint16_t));

    switch (nodeId->IdentifierType)
    {
    case SOPC_IdentifierType_Numeric:
        h = step(h, (const uint8_t*) &nodeId->Data.Numeric, sizeof(uint32_t));
        break;
    case SOPC_IdentifierType_ByteString:
    case SOPC_IdentifierType_String:
        if (nodeId->Data.String.Length > 0)
        {
            h = step(h, nodeId->Data.String.Data, (size_t) nodeId->Data.String.Length);
        }
        break;
    case SOPC_IdentifierType_Guid:
        if (nodeId->Data.Guid != NULL)
        {
            h = step(h, (const uint8_t*) nodeId->Data.Guid, sizeof(SOPC_Guid));
        }
        break;
    default:
        SOPC_ASSERT(false && "Unknown IdentifierType");
    }

    *hash = h;
}

/* Field-wise DJB, deliberately not delegating to SOPC_NodeId_Hash: this column must stay the DJB
 * baseline whatever the production hash becomes. 5381 is the DJB offset basis used by
 * SOPC_DJBHash. */
static void nodeid_hash_djb(const SOPC_NodeId* nodeId, uint64_t* hash)
{
    hash_nodeid_fields(nodeId, 5381, &SOPC_DJBHash_Step, hash);
}

/* What SOPC_NodeId_Dict_Create actually uses today. */
static void nodeid_hash_production(const SOPC_NodeId* nodeId, uint64_t* hash)
{
    SOPC_NodeId_Hash(nodeId, hash);
}

static void nodeid_hash_fnv1a(const SOPC_NodeId* nodeId, uint64_t* hash)
{
    hash_nodeid_fields(nodeId, FNV1A_OFFSET_BASIS, &SOPC_FNV1aHash_Step, hash);
}

static void nodeid_hash_canonical_fnv1a(const SOPC_NodeId* nodeId, uint64_t* hash)
{
    bool ok = nodeid_hash_from_canonical(nodeId, FNV1A_OFFSET_BASIS, &SOPC_FNV1aHash_Step, hash);
    SOPC_ASSERT(ok);
}

static uint64_t mix64(uint64_t x)
{
    x ^= x >> 33;
    x *= 0xff51afd7ed558ccdULL;
    x ^= x >> 33;
    x *= 0xc4ceb9fe1a85ec53ULL;
    x ^= x >> 33;
    return x;
}

static void nodeid_hash_numeric_mix(const SOPC_NodeId* nodeId, uint64_t* hash)
{
    if (nodeId->IdentifierType == SOPC_IdentifierType_Numeric)
    {
        uint64_t h = ((uint64_t) nodeId->Namespace << 32) | (uint64_t) nodeId->Data.Numeric;
        h ^= ((uint64_t) nodeId->IdentifierType << 48);
        *hash = mix64(h);
        return;
    }

    nodeid_hash_fnv1a(nodeId, hash);
}

static uint64_t make_hash_wrapper_djb(const uintptr_t id)
{
    uint64_t hash = 0;
    nodeid_hash_djb((const SOPC_NodeId*) id, &hash);
    return hash;
}

static uint64_t make_hash_wrapper_production(const uintptr_t id)
{
    uint64_t hash = 0;
    nodeid_hash_production((const SOPC_NodeId*) id, &hash);
    return hash;
}

static uint64_t make_hash_wrapper_fnv1a(const uintptr_t id)
{
    uint64_t hash = 0;
    nodeid_hash_fnv1a((const SOPC_NodeId*) id, &hash);
    return hash;
}

static uint64_t make_hash_wrapper_numeric_mix(const uintptr_t id)
{
    uint64_t hash = 0;
    nodeid_hash_numeric_mix((const SOPC_NodeId*) id, &hash);
    return hash;
}

static uint64_t make_hash_wrapper_canonical_fnv1a(const uintptr_t id)
{
    uint64_t hash = 0;
    nodeid_hash_canonical_fnv1a((const SOPC_NodeId*) id, &hash);
    return hash;
}

static bool nodeid_equal(const uintptr_t a, const uintptr_t b)
{
    return SOPC_NodeId_Equal((const SOPC_NodeId*) a, (const SOPC_NodeId*) b);
}

static int hash_entry_compare(const void* left, const void* right)
{
    const hash_entry_t* a = (const hash_entry_t*) left;
    const hash_entry_t* b = (const hash_entry_t*) right;

    if (a->hash < b->hash)
    {
        return -1;
    }
    if (a->hash > b->hash)
    {
        return 1;
    }
    if (a->key_index < b->key_index)
    {
        return -1;
    }
    if (a->key_index > b->key_index)
    {
        return 1;
    }
    return 0;
}

static bool verify_keys_distinct(const bench_data_t* data)
{
    SOPC_Dict* dict = SOPC_NodeId_Dict_Create(false, NULL);
    if (dict == NULL)
    {
        return false;
    }

    if (!SOPC_Dict_Reserve(dict, data->n_keys))
    {
        SOPC_Dict_Delete(dict);
        return false;
    }

    for (size_t i = 0; i < data->n_keys; ++i)
    {
        if (!SOPC_Dict_Insert(dict, (uintptr_t) data->keys[i], (uintptr_t)(i + 1)))
        {
            SOPC_Dict_Delete(dict);
            return false;
        }
    }

    const bool distinct = SOPC_Dict_Size(dict) == data->n_keys;
    SOPC_Dict_Delete(dict);
    return distinct;
}

static bool verify_hash_stability(NodeId_Hash_Fct* hash_fct, const SOPC_NodeId* key)
{
    uint64_t h1 = 0;
    uint64_t h2 = 0;
    hash_fct(key, &h1);
    hash_fct(key, &h2);
    return h1 == h2;
}

static bool verify_equal_keys_same_hash(NodeId_Hash_Fct* hash_fct, const SOPC_NodeId* key)
{
    SOPC_NodeId copy;
    SOPC_NodeId_Initialize(&copy);
    if (SOPC_NodeId_Copy(&copy, key) != SOPC_STATUS_OK)
    {
        return false;
    }

    uint64_t h_key = 0;
    uint64_t h_copy = 0;
    hash_fct(key, &h_key);
    hash_fct(&copy, &h_copy);
    SOPC_NodeId_Clear(&copy);
    return h_key == h_copy;
}

static bool count_hash_collisions(const bench_data_t* data, NodeId_Hash_Fct* hash_fct, size_t* out_collisions)
{
    hash_entry_t* entries = SOPC_Calloc(data->n_keys, sizeof(hash_entry_t));
    if (entries == NULL)
    {
        return false;
    }

    for (size_t i = 0; i < data->n_keys; ++i)
    {
        entries[i].key_index = i;
        hash_fct(data->keys[i], &entries[i].hash);
    }

    qsort(entries, data->n_keys, sizeof(hash_entry_t), hash_entry_compare);

    size_t collisions = 0;
    for (size_t i = 1; i < data->n_keys; ++i)
    {
        if (entries[i].hash != entries[i - 1].hash)
        {
            continue;
        }

        if (!nodeid_equal((uintptr_t) data->keys[entries[i].key_index],
                          (uintptr_t) data->keys[entries[i - 1].key_index]))
        {
            collisions++;
        }
    }

    SOPC_Free(entries);
    *out_collisions = collisions;
    return true;
}

/**
 * \brief Chi-square of the low CHI2_BUCKETS bits, divided by the degrees of freedom.
 *
 * The dictionary indexes buckets with (hash + f(i)) & (size - 1), so only the low bits of the
 * hash ever matter. This is the distribution quality criterion; the 64-bit collision count is
 * not.
 */
static bool low_bits_chi2_ratio(const bench_data_t* data, NodeId_Hash_Fct* hash_fct, double* out_ratio)
{
    uint32_t* counts = SOPC_Calloc(CHI2_BUCKETS, sizeof(uint32_t));

    if (counts == NULL)
    {
        return false;
    }

    for (size_t i = 0; i < data->n_keys; ++i)
    {
        uint64_t hash = 0;
        hash_fct(data->keys[i], &hash);
        counts[hash & (CHI2_BUCKETS - 1U)]++;
    }

    const double expected = (double) data->n_keys / (double) CHI2_BUCKETS;
    double chi2 = 0.0;
    for (size_t b = 0; b < CHI2_BUCKETS; ++b)
    {
        const double delta = (double) counts[b] - expected;
        chi2 += (delta * delta) / expected;
    }

    SOPC_Free(counts);
    *out_ratio = chi2 / (double) (CHI2_BUCKETS - 1U);
    return true;
}

static bool validate_hash_variant(const bench_data_t* data, NodeId_Hash_Fct* hash_fct, hash_validation_t* validation)
{
    memset(validation, 0, sizeof(*validation));
    validation->probe_simulation_ok = true;
    validation->hash_stable = true;
    validation->equal_keys_consistent = true;

    if (!count_hash_collisions(data, hash_fct, &validation->hash_collisions))
    {
        return false;
    }

    if (!low_bits_chi2_ratio(data, hash_fct, &validation->chi2_ratio))
    {
        return false;
    }

    for (size_t i = 0; i < data->n_keys; ++i)
    {
        if (!verify_hash_stability(hash_fct, data->keys[i]))
        {
            validation->hash_stable = false;
            break;
        }
    }

    for (size_t i = 0; i < data->n_keys && validation->equal_keys_consistent; ++i)
    {
        if (!verify_equal_keys_same_hash(hash_fct, data->keys[i]))
        {
            validation->equal_keys_consistent = false;
        }
    }

    return true;
}

static SOPC_NodeId* make_nodeid(nodeid_pattern_t pattern, size_t index)
{
    char buf[64];
    int n = 0;

    if (pattern == NODEID_PATTERN_STRING)
    {
        n = snprintf(buf, sizeof(buf), "ns=1;s=Objects.%zu", index);
    }
    else
    {
        n = snprintf(buf, sizeof(buf), "ns=1;i=%zu", index);
    }

    if (n <= 0 || ((size_t) n) >= sizeof(buf))
    {
        return NULL;
    }

    return SOPC_NodeId_FromCString(buf);
}

static bool build_keys(bench_data_t* data, nodeid_pattern_t pattern, size_t dict_size)
{
    data->keys = SOPC_Calloc(dict_size, sizeof(SOPC_NodeId*));
    if (data->keys == NULL)
    {
        return false;
    }
    data->n_keys = dict_size;

    for (size_t i = 0; i < dict_size; ++i)
    {
        data->keys[i] = make_nodeid(pattern, i);
        if (data->keys[i] == NULL)
        {
            return false;
        }
    }

    return true;
}

static bool build_lookup_indices(bench_data_t* data, size_t n_lookups)
{
    data->lookup_indices = SOPC_Calloc(n_lookups, sizeof(size_t));
    if (data->lookup_indices == NULL)
    {
        return false;
    }
    data->n_lookups = n_lookups;

    for (size_t i = 0; i < n_lookups; ++i)
    {
        if ((i % 3U) == 0U)
        {
            data->lookup_indices[i] = i % data->n_keys;
        }
        else if ((i % 3U) == 1U)
        {
            data->lookup_indices[i] = (i * 997U) % data->n_keys;
        }
        else
        {
            data->lookup_indices[i] = (i * 7919U + 104729U) % data->n_keys;
        }
    }

    return true;
}

static bool populate_dict(SOPC_Dict* dict, const bench_data_t* data)
{
    if (!SOPC_Dict_Reserve(dict, data->n_keys))
    {
        return false;
    }

    for (size_t i = 0; i < data->n_keys; ++i)
    {
        if (!SOPC_Dict_Insert(dict, (uintptr_t) data->keys[i], (uintptr_t)(i + 1)))
        {
            return false;
        }
    }

    return SOPC_Dict_Size(dict) == data->n_keys;
}

static size_t minimum_dict_size(size_t n_items)
{
    size_t size = 16;

    while (size < (2 * n_items))
    {
        size *= 2;
    }

    return size;
}

static bool compute_probe_stats(const bench_data_t* data,
                                NodeId_Hash_Fct* hash_fct,
                                double* avg_probes,
                                double* max_probes,
                                size_t* max_probe_chain,
                                bool* simulation_ok)
{
    const size_t table_size = minimum_dict_size(data->n_keys);
    const size_t sizemask = table_size - 1;
    bool* occupied = SOPC_Calloc(table_size, sizeof(bool));

    if (occupied == NULL)
    {
        return false;
    }

    double probe_sum = 0.0;
    size_t max_probe = 0;
    bool ok = true;

    for (size_t k = 0; k < data->n_keys; ++k)
    {
        uint64_t hash = 0;
        hash_fct(data->keys[k], &hash);

        bool inserted = false;
        for (size_t i = 0; i < table_size; ++i)
        {
            size_t idx = (size_t) SOPC_DICT_HASH(hash, i) & sizemask;

            if (!occupied[idx])
            {
                occupied[idx] = true;
                probe_sum += (double) (i + 1);
                if ((i + 1) > max_probe)
                {
                    max_probe = i + 1;
                }
                inserted = true;
                break;
            }
        }

        if (!inserted)
        {
            ok = false;
            break;
        }
    }

    if (ok)
    {
        *avg_probes = probe_sum / (double) data->n_keys;
        *max_probes = (double) max_probe;
    }
    *max_probe_chain = max_probe;
    *simulation_ok = ok;

    SOPC_Free(occupied);
    return true;
}

/** \brief Runs the warmup then the timed lookup loop once, and returns the elapsed time in us. */
static double run_lookup_bench(const SOPC_Dict* dict, const bench_data_t* data, size_t n_lookups)
{
    for (size_t i = 0; i < WARMUP_LOOKUPS; ++i)
    {
        size_t idx = data->lookup_indices[i % data->n_lookups];
        SOPC_NodeId* key = data->keys[idx];
        bool found = false;
        (void) SOPC_Dict_Get(dict, (uintptr_t) key, &found);
        SOPC_ASSERT(found);
    }

    SOPC_HighRes_TimeReference* start = SOPC_HighRes_TimeReference_Create();
    SOPC_ASSERT(start != NULL);

    for (size_t i = 0; i < n_lookups; ++i)
    {
        size_t idx = data->lookup_indices[i];
        SOPC_NodeId* key = data->keys[idx];
        bool found = false;
        uintptr_t value = SOPC_Dict_Get(dict, (uintptr_t) key, &found);
        SOPC_ASSERT(found);
        SOPC_ASSERT(value == (uintptr_t)(idx + 1));
    }

    const double elapsed_us = (double) SOPC_HighRes_TimeReference_DeltaUs(start, NULL);

    SOPC_HighRes_TimeReference_Delete(&start);
    return elapsed_us;
}

static int double_compare(const void* left, const void* right)
{
    const double a = *(const double*) left;
    const double b = *(const double*) right;

    if (a < b)
    {
        return -1;
    }
    if (a > b)
    {
        return 1;
    }
    return 0;
}

/**
 * \brief Repeats the lookup loop and keeps the median and the best run.
 *
 * A single run is not reproducible enough to compare hash functions: the median rejects outliers
 * caused by scheduling, and the best run is the closest to the hash cost proper.
 */
static bench_result_t run_lookup_bench_repeated(const SOPC_Dict* dict, const bench_data_t* data, size_t n_lookups)
{
    bench_result_t result = {0};
    double elapsed[BENCH_REPETITIONS];

    for (size_t r = 0; r < BENCH_REPETITIONS; ++r)
    {
        elapsed[r] = run_lookup_bench(dict, data, n_lookups);
    }

    qsort(elapsed, BENCH_REPETITIONS, sizeof(double), double_compare);

    result.elapsed_us = elapsed[BENCH_REPETITIONS / 2];
    result.ns_per_lookup = (result.elapsed_us * 1000.0) / (double) n_lookups;
    result.ns_per_lookup_min = (elapsed[0] * 1000.0) / (double) n_lookups;
    return result;
}

static void clear_bench_data(bench_data_t* data)
{
    if (data->keys != NULL)
    {
        for (size_t i = 0; i < data->n_keys; ++i)
        {
            if (data->keys[i] != NULL)
            {
                SOPC_NodeId_Clear(data->keys[i]);
                SOPC_Free(data->keys[i]);
            }
        }
        SOPC_Free(data->keys);
    }

    SOPC_Free(data->lookup_indices);
    memset(data, 0, sizeof(*data));
}

static void usage(const char* prog)
{
    printf(
        "Usage: %s [OPTIONS]\n\n"
        "Benchmark NodeId dictionary lookup performance.\n\n"
        "Options:\n"
        "  -n SIZE     Dictionary size (default: %u)\n"
        "  -l COUNT    Number of lookups per hash variant (default: %u)\n"
        "  -p PATTERN  NodeId pattern: string or numeric (default: string)\n"
        "  -h          Show this help\n",
        prog, DEFAULT_DICT_SIZE, DEFAULT_LOOKUPS);
}

static bool parse_size_arg(const char* arg, size_t* out)
{
    char* end = NULL;
    errno = 0;
    unsigned long long value = strtoull(arg, &end, 10);

    if (arg[0] == '\0' || end == NULL || *end != '\0' || errno != 0 || value == 0 || value > SIZE_MAX)
    {
        return false;
    }

    *out = (size_t) value;
    return true;
}

static bool run_validation_pass(const bench_data_t* data,
                                const hash_variant_t* variants,
                                size_t n_variants,
                                hash_validation_t* validations)
{
    printf("Validation\n");
    printf("----------\n");

    if (!verify_keys_distinct(data))
    {
        fprintf(stderr, "ERROR: benchmark key set contains duplicate NodeIds.\n");
        return false;
    }
    printf("Distinct NodeIds in key set: OK\n");

    printf("%-28s %10s %12s %10s %10s %10s\n", "Hash function", "chi2/dof", "Collisions*", "Stable", "Equal OK",
           "Probe sim");
    printf("%-28s %10s %12s %10s %10s %10s\n", "----------------------------", "----------", "------------",
           "----------", "----------", "----------");

    bool all_ok = true;

    for (size_t v = 0; v < n_variants; ++v)
    {
        if (!validate_hash_variant(data, variants[v].hash, &validations[v]))
        {
            fprintf(stderr, "ERROR: validation failed for %s.\n", variants[v].name);
            return false;
        }

        double avg_probes = 0.0;
        double max_probes = 0.0;
        size_t max_chain = 0;
        bool probe_ok = false;
        if (!compute_probe_stats(data, variants[v].hash, &avg_probes, &max_probes, &max_chain, &probe_ok))
        {
            fprintf(stderr, "ERROR: probe simulation failed for %s.\n", variants[v].name);
            return false;
        }

        validations[v].max_probe_chain = max_chain;
        validations[v].probe_simulation_ok = probe_ok;

        if (!validations[v].hash_stable || !validations[v].equal_keys_consistent || !probe_ok)
        {
            all_ok = false;
        }

        printf("%-28s %10.4f %12zu %10s %10s %10s\n", variants[v].name, validations[v].chi2_ratio,
               validations[v].hash_collisions, validations[v].hash_stable ? "OK" : "FAIL",
               validations[v].equal_keys_consistent ? "OK" : "FAIL", probe_ok ? "OK" : "SATURATED");
    }

    printf("\nchi2/dof over %u classes of low hash bits: 1.0 means as uniform as random, above 1\n", CHI2_BUCKETS);
    printf("means an unevenly loaded bucket array. This is the distribution criterion.\n");
    printf("(*) Collisions counts equal 64-bit hashes among distinct keys. It is a sanity check\n");
    printf("only: 0 is the expected result for any non-broken hash and cannot rank candidates.\n");
    printf("Reference hash uses platform-independent canonical encoding\n");
    printf("(type u8, namespace u16 BE, payload with fixed-endian fields).\n\n");

    if (!all_ok)
    {
        fprintf(stderr, "ERROR: at least one hash variant failed structural validation.\n");
        return false;
    }

    return true;
}

int main(int argc, char** argv)
{
    size_t dict_size = DEFAULT_DICT_SIZE;
    size_t n_lookups = DEFAULT_LOOKUPS;
    nodeid_pattern_t pattern = NODEID_PATTERN_STRING;

    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
        {
            usage(argv[0]);
            return 0;
        }
        else if (strcmp(argv[i], "-n") == 0)
        {
            if (i + 1 >= argc || !parse_size_arg(argv[++i], &dict_size))
            {
                fprintf(stderr, "Invalid dictionary size.\n");
                return 1;
            }
        }
        else if (strcmp(argv[i], "-l") == 0)
        {
            if (i + 1 >= argc || !parse_size_arg(argv[++i], &n_lookups))
            {
                fprintf(stderr, "Invalid lookup count.\n");
                return 1;
            }
        }
        else if (strcmp(argv[i], "-p") == 0)
        {
            if (i + 1 >= argc)
            {
                fprintf(stderr, "Missing pattern argument.\n");
                return 1;
            }
            ++i;
            if (strcmp(argv[i], "string") == 0)
            {
                pattern = NODEID_PATTERN_STRING;
            }
            else if (strcmp(argv[i], "numeric") == 0)
            {
                pattern = NODEID_PATTERN_NUMERIC;
            }
            else
            {
                fprintf(stderr, "Unknown pattern: %s\n", argv[i]);
                return 1;
            }
        }
        else
        {
            fprintf(stderr, "Unknown argument: %s\n", argv[i]);
            usage(argv[0]);
            return 1;
        }
    }

    const hash_variant_t variants[] = {
        {"DJB (baseline)", nodeid_hash_djb, make_hash_wrapper_djb, true},
        {"SOPC_NodeId_Hash (prod.)", nodeid_hash_production, make_hash_wrapper_production, true},
        {"FNV-1a 64-bit (native)", nodeid_hash_fnv1a, make_hash_wrapper_fnv1a, true},
        {"FNV-1a (canonical ref.)", nodeid_hash_canonical_fnv1a, make_hash_wrapper_canonical_fnv1a, false},
        {"Numeric mix64 fast-path", nodeid_hash_numeric_mix, make_hash_wrapper_numeric_mix, true},
    };

    const size_t n_variants = sizeof(variants) / sizeof(variants[0]);

    bench_data_t data = {0};
    if (!build_keys(&data, pattern, dict_size) || !build_lookup_indices(&data, n_lookups))
    {
        clear_bench_data(&data);
        fprintf(stderr, "Failed to allocate benchmark data.\n");
        return 1;
    }

    printf("NodeId dictionary lookup benchmark\n");
    printf("================================\n");
    printf("Dictionary size:     %zu\n", dict_size);
    printf("Lookups per variant: %zu\n", n_lookups);
    printf("NodeId pattern:      %s\n\n", pattern == NODEID_PATTERN_STRING ? "string" : "numeric");

    hash_validation_t* validations = SOPC_Calloc(n_variants, sizeof(hash_validation_t));
    if (validations == NULL)
    {
        clear_bench_data(&data);
        fprintf(stderr, "Failed to allocate validation results.\n");
        return 1;
    }

    if (!run_validation_pass(&data, variants, n_variants, validations))
    {
        SOPC_Free(validations);
        clear_bench_data(&data);
        return 1;
    }

    bench_result_t* results = SOPC_Calloc(n_variants, sizeof(bench_result_t));
    if (results == NULL)
    {
        SOPC_Free(validations);
        clear_bench_data(&data);
        fprintf(stderr, "Failed to allocate results.\n");
        return 1;
    }

    size_t perf_index = 0;
    size_t* perf_variant_ids = SOPC_Calloc(n_variants, sizeof(size_t));
    if (perf_variant_ids == NULL)
    {
        SOPC_Free(validations);
        clear_bench_data(&data);
        fprintf(stderr, "Failed to allocate perf variant map.\n");
        return 1;
    }

    for (size_t v = 0; v < n_variants; ++v)
    {
        if (!variants[v].include_in_perf)
        {
            continue;
        }

        perf_variant_ids[perf_index] = v;
        SOPC_Dict* dict = SOPC_Dict_Create(0, variants[v].hash_wrapper, &nodeid_equal, NULL, NULL);
        if (dict == NULL || !populate_dict(dict, &data))
        {
            fprintf(stderr, "Failed to build dictionary for %s.\n", variants[v].name);
            SOPC_Dict_Delete(dict);
            SOPC_Free(perf_variant_ids);
            SOPC_Free(results);
            SOPC_Free(validations);
            clear_bench_data(&data);
            return 1;
        }

        double avg_probes = 0.0;
        double max_probes = 0.0;
        size_t max_chain = 0;
        bool probe_ok = false;
        if (!compute_probe_stats(&data, variants[v].hash, &avg_probes, &max_probes, &max_chain, &probe_ok) || !probe_ok)
        {
            fprintf(stderr, "Failed probe stats for %s.\n", variants[v].name);
            SOPC_Dict_Delete(dict);
            SOPC_Free(perf_variant_ids);
            SOPC_Free(results);
            SOPC_Free(validations);
            clear_bench_data(&data);
            return 1;
        }

        printf("Running %s (%u repetitions) ...\n", variants[v].name, BENCH_REPETITIONS);
        results[perf_index] = run_lookup_bench_repeated(dict, &data, n_lookups);
        results[perf_index].avg_probes = avg_probes;
        results[perf_index].max_probes = max_probes;

        SOPC_Dict_Delete(dict);
        perf_index++;
    }

    const size_t n_perf = perf_index;
    clear_bench_data(&data);

    printf("\nPerformance\n");
    printf("-----------\n");
    printf("%-28s %14s %14s %12s %12s %10s\n", "Hash function", "ns/lookup(med)", "ns/lookup(min)", "Avg probes",
           "Max probes", "Speedup");
    printf("%-28s %14s %14s %12s %12s %10s\n", "----------------------------", "------------", "------------",
           "----------", "----------", "--------");

    double baseline_ns = (n_perf > 0) ? results[0].ns_per_lookup : 0.0;
    for (size_t i = 0; i < n_perf; ++i)
    {
        const size_t v = perf_variant_ids[i];
        double speedup = (baseline_ns > 0.0) ? baseline_ns / results[i].ns_per_lookup : 0.0;
        printf("%-28s %14.2f %14.2f %12.2f %12.0f %9.2fx\n", variants[v].name, results[i].ns_per_lookup,
               results[i].ns_per_lookup_min, results[i].avg_probes, results[i].max_probes, speedup);
    }

    printf("\nSpeedup is relative to the DJB baseline (median run).\n");
    printf("Canonical FNV-1a is validation-only (portable reference, not timed).\n");

    size_t best_perf = 0;
    for (size_t i = 1; i < n_perf; ++i)
    {
        if (results[i].ns_per_lookup < results[best_perf].ns_per_lookup)
        {
            best_perf = i;
        }
    }

    if (best_perf != 0 && baseline_ns > 0.0)
    {
        printf("Best timed alternative: %s (%.2fx faster, %.2f ns/lookup vs %.2f ns/lookup).\n",
               variants[perf_variant_ids[best_perf]].name, baseline_ns / results[best_perf].ns_per_lookup,
               results[best_perf].ns_per_lookup, baseline_ns);
    }
    else if (baseline_ns > 0.0)
    {
        printf("The DJB baseline is the fastest timed variant in this run.\n");
    }

    SOPC_Free(perf_variant_ids);
    SOPC_Free(validations);
    SOPC_Free(results);
    return 0;
}
